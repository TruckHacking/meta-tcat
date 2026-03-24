#!/bin/bash
set -euo pipefail
set -E

# --- Configuration and Defaults ---
TARGET_IP="192.168.7.2"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
IMAGE_FILE="${SCRIPT_DIR}/core-image-tcat.rootfs.wic.xz"
SSH_USER="nmfta"
# SSH Options: StrictHostKeyChecking=no to avoid prompts, ConnectTimeout for responsiveness
SSH_OPTS=(-o "StrictHostKeyChecking=no" -o "UserKnownHostsFile=/dev/null" -o "ConnectTimeout=10")
PAYLOAD_BASE="${SCRIPT_DIR}/suicide-payload-base.sh"
STAGE2_SCRIPT="${SCRIPT_DIR}/suicide-stage2.sh"
DRY_RUN=false
# Add a configurable timeout for monitoring
REBOOT_TIMEOUT_SECONDS=600

# --- State and Cleanup ---
# Use mktemp to create a temporary file safely
PAYLOAD_SCRIPT=$(mktemp) || { echo "Failed to create temp file"; exit 1; }
SSH_SOCKET=$(mktemp -u /tmp/ssh-ctrl-XXXXXX)
SSH_OPTS+=(-o "ControlMaster=auto" -o "ControlPath=$SSH_SOCKET" -o "ControlPersist=yes")

cleanup_host() {
    # Only remove if it exists to avoid errors in strict mode
    if [ -f "$PAYLOAD_SCRIPT" ]; then
        rm -f "$PAYLOAD_SCRIPT"
    fi
    if [ -S "$SSH_SOCKET" ]; then
        ssh -O exit -o "ControlPath=$SSH_SOCKET" "$SSH_USER@$TARGET_IP" 2>/dev/null || true
    fi
}
trap cleanup_host EXIT INT TERM

# --- Usage ---
usage() {
    echo "Usage: $0 [-i <image_file>] [-t <target_ip>] [-u <ssh_user>] [--reboot-timeout <seconds>] [--dry-run]"
    echo "  -i <image_file>    Path to the compressed image file (default: \"$IMAGE_FILE\")"
    echo "  -t <target_ip>     IP address of the BeagleBone Black (default: \"$TARGET_IP\")"
    echo "  -u <ssh_user>      SSH user (default: \"$SSH_USER\")"
    echo "  --reboot-timeout   Timeout in seconds to wait for reboot (default: $REBOOT_TIMEOUT_SECONDS)"
    echo "  --dry-run          Test transfer and setup but DO NOT write to flash or reboot"
    echo "  -h                 Show this help message"
    echo ""
    echo "WARNING: Monitoring after reboot assumes the target retains its IP address."
    exit 1
}

# --- Argument Parsing ---
while [[ $# -gt 0 ]]; do
    case "$1" in
        -i)
            [[ $# -lt 2 ]] && { echo "Error: -i requires an argument"; usage; }
            IMAGE_FILE="$2"
            shift 2
            ;;
        -t)
            [[ $# -lt 2 ]] && { echo "Error: -t requires an argument"; usage; }
            TARGET_IP="$2"
            shift 2
            ;;
        -u)
            [[ $# -lt 2 ]] && { echo "Error: -u requires an argument"; usage; }
            SSH_USER="$2"
            shift 2
            ;;
        --reboot-timeout)
            [[ $# -lt 2 ]] && { echo "Error: --reboot-timeout requires an argument"; usage; }
            if ! [[ "$2" =~ ^[0-9]+$ ]]; then
                echo "Error: Timeout must be a number." >&2; usage;
            fi
            REBOOT_TIMEOUT_SECONDS="$2"
            shift 2
            ;;
        --dry-run)
            DRY_RUN=true
            shift
            ;;
        -h|--help)
            usage
            ;;
        *)
            echo "Error: Unknown argument '$1'" >&2
            usage
            ;;
    esac
done

if [ "$DRY_RUN" = true ]; then
    echo "!!! DRY RUN MODE ENABLED !!!"
    echo "Transfer will be tested, but no permanent changes will be made."
fi

# --- Host-Side Pre-flight Checks ---
echo "--- Host-Side Checks ---"
# Defensive check for required files
for f in "$IMAGE_FILE" "$PAYLOAD_BASE" "$STAGE2_SCRIPT"; do
    if [ ! -f "$f" ]; then
        echo "Error: Required file '$f' not found." >&2
        exit 1
    fi
done

# Add 'ping' and 'date' to the list of required tools
for tool in ssh scp xz ping date; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "Error: Host tool '$tool' is required but not installed." >&2
        exit 1
    fi
done

echo "Running host tool feature checks..."
# Verify xz supports -lv for byte-count extraction
if ! xz -lv --help 2>&1 | grep -qi "\-v"; then
    echo "Error: Host 'xz' does not support the '-lv' flags for byte-count extraction." >&2
    exit 1
fi

echo "Calculating uncompressed image size..."
# xz -lv provides the byte count in parentheses for broader compatibility
if ! IMG_INFO=$(xz -lv "$IMAGE_FILE"); then
    echo "Error: Failed to read image info from '$IMAGE_FILE' (is it a valid .xz file?)." >&2
    exit 1
fi
# Extract byte count from the 'Uncompressed size:' line, e.g. "10.0 MiB (10,485,760 B)"
IMG_SIZE_BYTES=$(echo "$IMG_INFO" | grep "Uncompressed size:" | head -n 1 | awk -F'[()]' '{print $(NF-1)}' | tr -d ' ,B')
if ! [[ "$IMG_SIZE_BYTES" =~ ^[0-9]+$ ]]; then
    echo "Error: Could not determine uncompressed image size from xz metadata (tried xz -lv)." >&2
    exit 1
fi
echo "Image size: $((IMG_SIZE_BYTES / 1024 / 1024)) MB"

# --- Target-Side Pre-flight Checks ---
echo "--- Target-Side Checks ---"
echo "Establishing persistent SSH connection to target ($TARGET_IP)..."

# -M puts ssh in master mode for connection sharing.
# -f tells ssh to go to the background just before command execution.
# -N tells ssh not to execute a remote command.
if ! ssh -M -f -N "${SSH_OPTS[@]}" "$SSH_USER@$TARGET_IP"; then
    echo "Error: Cannot establish persistent SSH connection. Check IP, network, and firewall." >&2
    exit 1
fi

# --- Privilege Escalation ---
SUDO_CMD=""
if [ "$SSH_USER" != "root" ]; then
    echo "SSH user '$SSH_USER' is not root. Requesting sudo access on target to temporarily disable sudo password..."
    # We temporarily allow passwordless sudo for the user. 
    # This is safe because the OS is about to be completely overwritten in a few seconds.
    # We write to a sudoers.d drop-in, or append to /etc/sudoers if the dir doesn't exist.
    if ! ssh -t "${SSH_OPTS[@]}" "$SSH_USER@$TARGET_IP" "sudo sh -c 'mkdir -p /etc/sudoers.d && echo \"$SSH_USER ALL=(ALL) NOPASSWD: ALL\" > /etc/sudoers.d/suicide_update || echo \"$SSH_USER ALL=(ALL) NOPASSWD: ALL\" >> /etc/sudoers'"; then
        echo "Error: Failed to obtain sudo privileges on target." >&2
        exit 1
    fi
    SUDO_CMD="sudo "
fi

# Improved eMMC size check: search for a valid mmcblk device
EMMC_DEVICE_PATH=""
TARGET_EMMC_DEV=""
for dev in mmcblk1 mmcblk0; do
    # shellcheck disable=SC2029
    if ssh "${SSH_OPTS[@]}" "$SSH_USER@$TARGET_IP" "test -e /sys/class/block/$dev/size"; then
        EMMC_DEVICE_PATH="/sys/class/block/$dev"
        TARGET_EMMC_DEV="/dev/$dev"
        break
    fi
done

if [ -z "$EMMC_DEVICE_PATH" ]; then
    echo "Error: Could not find a valid eMMC device (mmcblk0 or mmcblk1) on target." >&2
    exit 1
fi
echo "Found eMMC device at $EMMC_DEVICE_PATH ($TARGET_EMMC_DEV)"

# Check eMMC size
# shellcheck disable=SC2029
if ! EMMC_SECTORS=$(ssh "${SSH_OPTS[@]}" "$SSH_USER@$TARGET_IP" "cat $EMMC_DEVICE_PATH/size"); then
    echo "Error: Could not read eMMC size on target." >&2
    exit 1
fi
EMMC_SECTORS=$(echo "$EMMC_SECTORS" | tr -d '[:space:]')

if ! [[ "$EMMC_SECTORS" =~ ^[0-9]+$ ]]; then
    echo "Error: Invalid eMMC sector count ('$EMMC_SECTORS') returned from target." >&2
    exit 1
fi

EMMC_SIZE_BYTES=$((EMMC_SECTORS * 512))
echo "Target eMMC size: $((EMMC_SIZE_BYTES / 1024 / 1024)) MB"

if [ "$IMG_SIZE_BYTES" -gt "$EMMC_SIZE_BYTES" ]; then
    echo "Error: Image size ($IMG_SIZE_BYTES bytes) exceeds eMMC capacity ($EMMC_SIZE_BYTES bytes)." >&2
    exit 1
fi

# Locate busybox on target
TARGET_BB=$(ssh "${SSH_OPTS[@]}" "$SSH_USER@$TARGET_IP" "command -v busybox")
if [ -z "$TARGET_BB" ]; then
    echo "Error: busybox not found on target." >&2
    exit 1
fi
echo "Found busybox at: $TARGET_BB"

# Locate xz on target (optional but recommended for integrity)
# shellcheck disable=SC2029
TARGET_XZ=$(ssh "${SSH_OPTS[@]}" "$SSH_USER@$TARGET_IP" "command -v xz || true")
if [ -n "$TARGET_XZ" ]; then
    echo "Found xz at $TARGET_XZ. Target-side decompression enabled."
else
    echo "xz not found on target. Using host-side decompression (verify checksum manually if needed)."
fi

# Check for ldd
if ! ssh "${SSH_OPTS[@]}" "$SSH_USER@$TARGET_IP" "command -v ldd >/dev/null"; then
    echo "Error: 'ldd' not found on target. Cannot resolve dependencies." >&2
    exit 1
fi

echo "Pre-flight checks passed. Generating payload..."

# --- Payload Generation ---
# We inject host-side variables at the top, then append the payload body
# from the external file.

cat << HEADER_EOF > "$PAYLOAD_SCRIPT"
#!/bin/sh
set -e
# --- Injected Configuration ---
TARGET_BB="$TARGET_BB"
TARGET_XZ="$TARGET_XZ"
TARGET_EMMC_DEV="$TARGET_EMMC_DEV"
IMG_SIZE_BYTES="$IMG_SIZE_BYTES"
DRY_RUN="$DRY_RUN"
RAMFS_DIR="/tmp/suicide_ramfs"
OLD_ROOT="/oldroot"
STAGE2_SCRIPT="/stage2.sh"
HEADER_EOF

cat "$PAYLOAD_BASE" >> "$PAYLOAD_SCRIPT"

# --- Execution ---
echo "--- Execution ---"
echo "Transferring scripts..."

# Transfer payload script
if ! scp "${SSH_OPTS[@]}" "$PAYLOAD_SCRIPT" "$SSH_USER@$TARGET_IP:/tmp/suicide_payload.sh"; then
    echo "Error: Failed to copy payload script to target." >&2
    exit 1
fi

# shellcheck disable=SC2029
if ! ssh "${SSH_OPTS[@]}" "$SSH_USER@$TARGET_IP" "chmod +x /tmp/suicide_payload.sh"; then
    echo "Error: Failed to make payload script executable." >&2
    exit 1
fi

# Transfer stage2 script separately to /tmp/suicide_stage2.sh
if ! scp "${SSH_OPTS[@]}" "$STAGE2_SCRIPT" "$SSH_USER@$TARGET_IP:/tmp/suicide_stage2.sh"; then
    echo "Error: Failed to copy stage2 script to target." >&2
    exit 1
fi

echo "----------------------------------------------------------------"
if [ "$DRY_RUN" = true ]; then
    echo "WARNING: DRY RUN MODE."
    echo "Testing data transfer only. No flash write."
else
    echo "WARNING: Starting irreversible update process."
    echo "You have 5 seconds to abort (Ctrl+C)."
    for i in 1 2 3 4 5; do
      echo $i
      sleep 1
    done
fi
echo "----------------------------------------------------------------"

echo "Streaming image to target... (This may take several minutes)"
# Decide whether to stream compressed or uncompressed
STREAM_CMD=""
if [ -n "$TARGET_XZ" ]; then
    # Stream compressed, let target decompress (verifies integrity)
    STREAM_CMD="cat \"$IMAGE_FILE\""
else
    # Stream uncompressed, decompressed on host
    STREAM_CMD="xz -d -c \"$IMAGE_FILE\""
fi

# Pipe the stream through SSH to the payload script.
# The payload script reads from stdin (via dd in stage 2).
# shellcheck disable=SC2029
if eval "$STREAM_CMD" | ssh "${SSH_OPTS[@]}" "$SSH_USER@$TARGET_IP" "${SUDO_CMD}/tmp/suicide_payload.sh"; then
    echo "Stream and remote execution finished successfully."
else
    echo "Error: The image stream failed. This could be due to:" >&2
    echo "  1. A corrupted image file ('$IMAGE_FILE')." >&2
    echo "  2. A network error during the SSH transfer." >&2
    echo "  3. An error in the remote payload script on the target." >&2
    exit 1
fi

# --- Post-Update Monitoring ---
if [ "$DRY_RUN" = true ]; then
    echo "Dry run completed. Skipping reboot monitoring."
    exit 0
fi

echo "--- Monitoring ---"
echo "Waiting for target to begin reboot process..."
# Give it a moment to actually shut down
sleep 15
START_TIME=$(date +%s)

echo "Polling for device at $TARGET_IP (timeout: ${REBOOT_TIMEOUT_SECONDS}s)..."
while true; do
    # More robust check: use SSH instead of ping. 
    # Disable ControlMaster here to prevent polling failures if the old socket is dead.
    if ssh "${SSH_OPTS[@]}" -o "ControlPath=none" -o "ConnectTimeout=2" "$SSH_USER@$TARGET_IP" "exit 0" >/dev/null 2>&1; then
        echo ""
        echo "SUCCESS: Device is back online and SSH is responsive!"
        break
    fi

    CURRENT_TIME=$(date +%s)
    ELAPSED=$((CURRENT_TIME - START_TIME))

    if [ "$ELAPSED" -gt "$REBOOT_TIMEOUT_SECONDS" ]; then
        echo ""
        echo "Error: Timed out waiting for device to reboot ($REBOOT_TIMEOUT_SECONDS seconds)." >&2
        echo "The device may have failed to update or has a different IP address." >&2
        exit 1
    fi

    echo -n "."
    sleep 2
done
