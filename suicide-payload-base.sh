# shellcheck shell=sh

# --- Defensive Config ---
# Try to enable pipefail for safer pipe error handling
set -o pipefail 2>/dev/null || true

# Ignore SIGHUP to prevent immediate termination if SSH connection flickers,
# though the data pipe will likely break causing failure anyway.
trap '' HUP

# --- Error Handling ---
# shellcheck disable=SC2329,SC2317
cleanup() {
    # Attempt to cleanup RAMFS if we haven't pivoted yet
    # We check if RAMFS_DIR is still mounted at its original path
    if grep -q "$RAMFS_DIR" /proc/mounts 2>/dev/null; then
        echo "[Target] Cleaning up RAMFS mounts..."
        umount "$RAMFS_DIR" 2>/dev/null || true
    fi
    if [ -d "$RAMFS_DIR" ]; then
         rm -rf "$RAMFS_DIR"
    fi
}

# shellcheck disable=SC2329,SC2317
on_exit() {
    rc=$?
    # cleanup will run unless we explicitly removed the trap (after pivot)
    cleanup
    if [ "$rc" -ne 0 ]; then
        echo "[Target] FATAL ERROR: Script exited with error code $rc" >&2
    fi
}
trap on_exit EXIT
trap "exit 1" INT TERM

# --- Validation of Injected State ---
if [ -z "$TARGET_BB" ] || [ -z "$RAMFS_DIR" ] || [ -z "$STAGE2_SCRIPT" ] || [ -z "$TARGET_EMMC_DEV" ]; then
    echo "[Target] Error: Critical injected variables are missing." >&2
    exit 1
fi

# Enable SysRq for emergency unmount/reboot later
echo 1 > /proc/sys/kernel/sysrq 2>/dev/null || echo "[Target] Warning: Failed to enable SysRq."

# --- RAMFS Setup ---
echo "[Target] Setting up RAMFS at $RAMFS_DIR..."
mkdir -p "$RAMFS_DIR"
if ! mount -t tmpfs -o size=128M tmpfs "$RAMFS_DIR"; then
    echo "[Target] Error: Failed to mount tmpfs at $RAMFS_DIR" >&2
    exit 1
fi

echo "[Target] Copying busybox..."
mkdir -p "$RAMFS_DIR/bin"
# shellcheck disable=SC2153
if ! cp "$TARGET_BB" "$RAMFS_DIR/bin/busybox"; then
    echo "[Target] Error: Failed to copy busybox to RAMFS" >&2
    exit 1
fi
chmod +x "$RAMFS_DIR/bin/busybox"

echo "[Target] Installing busybox applets..."
# Create necessary directories
"$RAMFS_DIR/bin/busybox" mkdir -p "$RAMFS_DIR/lib" "$RAMFS_DIR/usr/lib" \
    "$RAMFS_DIR/proc" "$RAMFS_DIR/sys" "$RAMFS_DIR/dev" "$RAMFS_DIR/tmp" \
    "$RAMFS_DIR$OLD_ROOT"

"$RAMFS_DIR/bin/busybox" --install -s "$RAMFS_DIR/bin"

# If we have xz, copy it too
if [ -n "$TARGET_XZ" ]; then
    echo "[Target] Copying xz..."
    if ! cp "$TARGET_XZ" "$RAMFS_DIR/bin/xz"; then
         echo "[Target] Error: Failed to copy xz to RAMFS" >&2
         exit 1
    fi
    chmod +x "$RAMFS_DIR/bin/xz"
fi

echo "[Target] Resolving shared library dependencies..."
# Use ldd to find libraries. We preserve directory structure to be safe.
# Disable set -e temporarily for the pipe in case grep finds nothing (static binary)
set +e
LIBS=$(ldd "$TARGET_BB" 2>/dev/null | grep -o '/[^ ]*')
if [ -n "$TARGET_XZ" ]; then
    XZ_LIBS=$(ldd "$TARGET_XZ" 2>/dev/null | grep -o '/[^ ]*')
    LIBS="$LIBS
$XZ_LIBS"
fi
RET=$?
set -e

if [ $RET -eq 0 ] && [ -n "$LIBS" ]; then
    echo "$LIBS" | sort -u | while read -r lib; do
        # Some ldd lines might match but not be files (e.g. vdso), check existence
        if [ -f "$lib" ]; then
            dir=$(dirname "$lib")
            mkdir -p "$RAMFS_DIR$dir"
            # Use -L to dereference symlinks (copy the actual file content)
            cp -L "$lib" "$RAMFS_DIR$lib" || {
                echo "[Target] Warning: Failed to copy library $lib" >&2
            }
        fi
    done
fi

# Copy dynamic linker explicitly if not caught (e.g. ld-linux.so)
if ls /lib/ld-*.so.* >/dev/null 2>&1; then
    cp -L /lib/ld-*.so.* "$RAMFS_DIR/lib/"
fi

# Verify RAMFS environment
# We use full path to busybox sh to ensure it works
if ! chroot "$RAMFS_DIR" /bin/busybox sh -c "true"; then
    echo "[Target] Error: RAMFS environment is non-functional (missing libs?)." >&2
    exit 1
fi

if [ -n "$TARGET_XZ" ]; then
    if ! chroot "$RAMFS_DIR" /bin/xz --version >/dev/null 2>&1; then
        echo "[Target] Error: xz copied to RAMFS but not functional. Aborting." >&2
        exit 1
    fi
fi

# --- Stage 2 Script (Inner Payload) ---
# Copy the stage2 script to the RAMFS
if [ -f "/tmp/suicide_stage2.sh" ]; then
    cp "/tmp/suicide_stage2.sh" "$RAMFS_DIR$STAGE2_SCRIPT"
    chmod +x "$RAMFS_DIR$STAGE2_SCRIPT"
else
    echo "Error: /tmp/suicide_stage2.sh not found." >&2
    exit 1
fi

# --- Dry Run Check ---
if [ "$DRY_RUN" = "true" ]; then
    echo "[Target] DRY RUN: Consuming input stream to /dev/null..."
    # Consume exactly as much as sent? No, just until EOF.
    # Note: If target has xz, we are receiving compressed stream, need to decompress
    if [ -n "$TARGET_XZ" ]; then
        if xz -d -c >/dev/null; then
            echo "[Target] DRY RUN: Compressed transfer successful."
            exit 0
        else
            echo "[Target] DRY RUN: Compressed transfer FAILED (corrupt xz?)." >&2
            exit 1
        fi
    else
        if cat >/dev/null; then
            echo "[Target] DRY RUN: Uncompressed transfer successful."
            exit 0
        else
            echo "[Target] DRY RUN: Transfer FAILED." >&2
            exit 1
        fi
    fi
fi

# --- Pivot Root ---
echo "[Target] Preparing to pivot root..."

echo "[Target] Quieting system (stopping services)..."
# DO NOT kill dropbear/sshd as it holds the pipe for the image data
killall -q udevd systemd-journald syslogd rsyslogd dbus-daemon || true
sleep 2

echo "[Target] Syncing filesystem..."
sync

# Make root private to ensure pivot_root works if mounts are shared
mount --make-rprivate / 2>/dev/null || true

# Attempt to remount root RO to ensure consistency
echo "[Target] Attempting to remount / read-only..."
if ! mount -o remount,ro /; then
    echo "[Target] Warning: Could not remount / read-only. Unclean unmount likely." >&2
    echo "[Target] Forcing sync again..."
    sync
fi

# Bind mounts for stage 2
mount --bind /proc "$RAMFS_DIR/proc"
mount --bind /sys "$RAMFS_DIR/sys"
mount --bind /dev "$RAMFS_DIR/dev"

echo "[Target] Pivoting to RAMFS..."
cd "$RAMFS_DIR" || exit 1

if ! pivot_root . ".$OLD_ROOT"; then
    echo "[Target] Error: pivot_root failed." >&2
    exit 1
fi

# PIVOT SUCCESSFUL - Point of no return
# Disable cleanup trap because we are now in the new root
trap - EXIT INT TERM

echo "[Target] Executing Stage 2..."
# exec chroot replaces the current process with the stage 2 script inside the new root
# We need to pass variables to stage 2
export TARGET_XZ
export TARGET_EMMC_DEV
export IMG_SIZE_BYTES
# shellcheck disable=SC2093
exec chroot . "$STAGE2_SCRIPT"

# --- Failsafe ---
# If exec returns, it failed to execute the new process.
echo "[Target] CRITICAL ERROR: exec chroot failed." >&2
echo "[Target] The system is in a pivoted RAMFS state. Rebooting to restore original OS..." >&2
sleep 3
reboot -f
# Last ditch
echo b > /proc/sysrq-trigger
exit 1
