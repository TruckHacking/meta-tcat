#!/bin/sh
set -e

# We are now in the RAMFS root. /oldroot contains the old eMMC.

echo "[Stage2] Unmounting old root..."
# Try clean unmount first
if ! umount /oldroot 2>/dev/null; then
    echo "[Stage2] Clean unmount failed. Trying lazy unmount..."
    if ! umount -l /oldroot; then
        echo "[Stage2] Lazy unmount failed. Attempting SysRq emergency R/O remount..."
        # Magic SysRq key 'u' remounts all filesystems read-only immediately.
        echo u > /proc/sysrq-trigger || echo "Warning: SysRq trigger failed."
    fi
fi

# Ensure TARGET_EMMC_DEV is set, otherwise default to mmcblk1 (unsafe assumption, but needed as fallback?)
# Actually, we should have passed it from the payload script.
if [ -z "$TARGET_EMMC_DEV" ]; then
    echo "[Stage2] Warning: TARGET_EMMC_DEV not set. Defaulting to /dev/mmcblk1."
    TARGET_EMMC_DEV="/dev/mmcblk1"
fi

echo "[Stage2] Starting Flash to $TARGET_EMMC_DEV..."

# Construct the write command
if [ -n "$TARGET_XZ" ] && [ -x "/bin/xz" ]; then
    echo "[Stage2] Decompressing on-the-fly with xz..."
    WRITE_CMD="xz -d -c | dd of=$TARGET_EMMC_DEV bs=4M"
else
    echo "[Stage2] Writing raw stream..."
    WRITE_CMD="dd of=$TARGET_EMMC_DEV bs=4M"
fi

# Execute
if eval "$WRITE_CMD"; then
    echo "[Stage2] Flash COMPLETE."
    echo "[Stage2] Syncing disk..."
    sync
    echo "[Stage2] Rebooting in 3 seconds..."
    sleep 3
    reboot -f
else
    echo "[Stage2] CRITICAL ERROR: Write failed!" >&2
    # If we fail here, the system is likely corrupt.
    # We exit 1 to close the SSH session.
    exit 1
fi
