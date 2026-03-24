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

# Ensure TARGET_EMMC_DEV is set
if [ -z "$TARGET_EMMC_DEV" ]; then
    echo "[Stage2] ERROR: TARGET_EMMC_DEV not set. Cannot proceed." >&2
    exit 1
fi

if [ ! -b "$TARGET_EMMC_DEV" ]; then
    echo "[Stage2] ERROR: $TARGET_EMMC_DEV is not a block device." >&2
    exit 1
fi

echo "[Stage2] Starting Flash to $TARGET_EMMC_DEV..."
echo "[Stage2] NOTICE: This process typically takes 5 to 12 minutes. There will be no progress updates."

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

    echo "[Stage3] Refreshing partition table and block devices..."
    sync
    # Force kernel to reread partition table
    blockdev --rereadpt "$TARGET_EMMC_DEV" 2>/dev/null || true
    # Wait for devtmpfs to populate partition nodes
    sleep 5

    echo "[Stage3] Ensuring partition 1 is bootable (active)..."
    # Only toggle the bootable flag if it isn't already set.
    # We check if partition 1 has the '*' boot flag in fdisk -l output.
    if ! fdisk -l "$TARGET_EMMC_DEV" | grep "^${TARGET_EMMC_DEV}p1" | grep -q "\*"; then
        echo "[Stage3] Toggling bootable flag on partition 1..."
        printf "a\n1\nw\n" | fdisk "$TARGET_EMMC_DEV" >/dev/null 2>&1 || true
    else
        echo "[Stage3] Partition 1 is already bootable."
    fi

    echo "[Stage3] Running post-flash fixups..."
    
    # Identify the partitions on the newly flashed device
    PART1="${TARGET_EMMC_DEV}p1"
    PART2="${TARGET_EMMC_DEV}p2"
    
    # Create mount point
    NEW_ROOT="/mnt/newroot"
    mkdir -p "$NEW_ROOT"
    
    echo "[Stage3] Mounting new rootfs ($PART2)..."
    if mount "$PART2" "$NEW_ROOT"; then
        # Bind essential virtual filesystems for chroot
        mount --bind /proc "$NEW_ROOT/proc"
        mount --bind /sys "$NEW_ROOT/sys"
        mount --bind /dev "$NEW_ROOT/dev"

        # 1. Fix fstab
        # We explicitly set root to the actual partition we just flashed
        echo "[Stage3] Fixing fstab (setting root to $PART2)..."
        # Remove any existing mmcblk lines and replace with explicit ones
        sed -i '/mmcblk/d' "$NEW_ROOT/etc/fstab"
        echo "$PART1  /boot/p1  auto  defaults  0  2" >> "$NEW_ROOT/etc/fstab"
        echo "$PART2  /         auto  defaults  0  1" >> "$NEW_ROOT/etc/fstab"
        
        # 2. Restore SSH keys & machine-id
        echo "[Stage3] Restoring SSH keys and machine-id..."
        if [ -d "/identity_backup" ]; then
            cp -p /identity_backup/ssh_host_* "$NEW_ROOT/etc/ssh/" 2>/dev/null || true
            cp -p /identity_backup/machine-id "$NEW_ROOT/etc/" 2>/dev/null || true
        fi
        rm -f "$NEW_ROOT/etc/ssh/ssh.regenerate" || true

        # 3. Rename Host and Create User
        if [ -f /oldroot/etc/hostname ]; then
            OLD_HOSTNAME=$(cat /oldroot/etc/hostname | tr -d '[:space:]')
        else
            OLD_HOSTNAME="tcat"
        fi
        
        if [ -n "$OLD_HOSTNAME" ]; then
            echo "[Stage3] Restoring hostname ($OLD_HOSTNAME) and user 'nmfta'..."
            echo "$OLD_HOSTNAME" > "$NEW_ROOT/etc/hostname"
            
            # Fix hosts file
            sed -i "/127.0.1.1/d" "$NEW_ROOT/etc/hosts"
            echo "127.0.1.1 $OLD_HOSTNAME" >> "$NEW_ROOT/etc/hosts"
            
            if ! grep -q "127.0.0.1" "$NEW_ROOT/etc/hosts"; then
                echo "127.0.0.1 localhost" >> "$NEW_ROOT/etc/hosts"
            fi

            if [ -x "$NEW_ROOT/usr/sbin/useradd" ]; then
                chroot "$NEW_ROOT" /bin/bash -c "
                    if id 'nmfta' &>/dev/null; then
                        deluser nmfta
                    fi
                    useradd -m -d /home/nmfta -s /bin/bash nmfta
                    echo 'nmfta:$OLD_HOSTNAME' | chpasswd
                    usermod -aG sudo nmfta
                    passwd -l root
                    chown -R nmfta:nmfta /home/nmfta
                "
            fi
        fi

        # 4. Update Device Tree Overlays and extlinux.conf
        echo "[Stage3] Updating Device Tree Overlays and Boot Config..."
        mkdir -p "$NEW_ROOT/boot/p1"
        if mount "$PART1" "$NEW_ROOT/boot/p1"; then
            # Fix root device in extlinux.conf to match the flashed device
            EXT_CONF="$NEW_ROOT/boot/p1/extlinux/extlinux.conf"
            if [ -f "$EXT_CONF" ]; then
                echo "[Stage3] Setting root device in extlinux.conf to $PART2..."
                # Replace root=... with root=PART2
                sed -i "s|root=[^ ]*|root=$PART2|g" "$EXT_CONF"
            fi

            if [ -x "$NEW_ROOT/usr/bin/update-overlays" ] || [ -x "$NEW_ROOT/bin/update-overlays" ]; then
                chroot "$NEW_ROOT" /bin/bash -c "update-overlays"
            fi
            umount "$NEW_ROOT/boot/p1"
        fi

        umount "$NEW_ROOT/dev" || true
        umount "$NEW_ROOT/sys" || true
        umount "$NEW_ROOT/proc" || true
        umount "$NEW_ROOT"
        echo "[Stage3] Post-flash fixups COMPLETE."
    else
        echo "[Stage3] CRITICAL WARNING: Failed to mount $PART2 for post-flash fixups!" >&2
    fi

    echo "[Stage3] Syncing disk and flushing buffers..."
    sync
    blockdev --flushbufs "$TARGET_EMMC_DEV" 2>/dev/null || true
    echo "[Stage3] NOTICE: To complete the update, power must be disconnected and the supercap left to discharge."
    echo "[Stage3] NOTICE: Please wait until the power LED goes dark before reapplying power."
    echo "[Stage3] Rebooting in 3 seconds..."
    # Background the reboot so the script can exit cleanly and close the SSH session gracefully
    (sleep 3; reboot -f) &
    exit 0
else
    echo "[Stage2] CRITICAL ERROR: Write failed!" >&2
    # If we fail here, the system is likely corrupt.
    # We exit 1 to close the SSH session.
    exit 1
fi
