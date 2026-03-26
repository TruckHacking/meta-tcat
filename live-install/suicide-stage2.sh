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
        # We NO LONGER modify fstab. The .wic image defaults to mmcblk0p1/p2, 
        # which is correct when the eMMC boots without an SD card.
        
        # 2. Restore SSH keys & machine-id
        echo "[Stage3] Restoring SSH keys and machine-id..."
        if [ -d "/identity_backup" ]; then
            cp -p /identity_backup/ssh_host_* "$NEW_ROOT/etc/ssh/" 2>/dev/null || true
            cp -p /identity_backup/machine-id "$NEW_ROOT/etc/" 2>/dev/null || true
        fi
        rm -f "$NEW_ROOT/etc/ssh/ssh.regenerate" || true

        # 3. Rename Host and Create User
        if [ -f /identity_backup/hostname ]; then
            OLD_HOSTNAME=$(cat /identity_backup/hostname | tr -d '[:space:]')
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

            if [ -s /identity_backup/passwd.nmfta ]; then
                # Restore nmfta user from identity_backup
                cat /identity_backup/passwd.nmfta >> "$NEW_ROOT/etc/passwd" || true
                cat /identity_backup/shadow.nmfta >> "$NEW_ROOT/etc/shadow" || true
                cat /identity_backup/group.nmfta >> "$NEW_ROOT/etc/group" || true
                
                # Make sure nmfta is in the sudo and tcat-ops groups in the new rootfs
                if ! grep -q "^tcat-ops:" "$NEW_ROOT/etc/group"; then
                    echo "tcat-ops:x:992:" >> "$NEW_ROOT/etc/group"
                fi
                for grp in sudo tcat-ops; do
                    if grep -q "^${grp}:" "$NEW_ROOT/etc/group"; then
                        sed -i "/^${grp}:/ s/\$/nmfta,/" "$NEW_ROOT/etc/group"
                        # clean up double commas if it was already there or empty
                        sed -i 's/:,/:/g; s/,,/,/g' "$NEW_ROOT/etc/group"
                    fi
                done
                
                # Lock root account just in case
                sed -i 's/^root:[^:]*:/root:!:/' "$NEW_ROOT/etc/shadow"
                
                # Ensure home directory exists and restore ssh keys
                mkdir -p "$NEW_ROOT/home/nmfta/.ssh"
                if [ -d "/identity_backup/ssh_keys" ] && [ "$(ls -A /identity_backup/ssh_keys 2>/dev/null)" ]; then
                    cp -rp /identity_backup/ssh_keys/* "$NEW_ROOT/home/nmfta/.ssh/" 2>/dev/null || true
                fi
                
                # Fix ownership of home directory
                NMFTA_UID=$(cat /identity_backup/passwd.nmfta | cut -d: -f3)
                NMFTA_GID=$(cat /identity_backup/passwd.nmfta | cut -d: -f4)
                if [ -n "$NMFTA_UID" ] && [ -n "$NMFTA_GID" ]; then
                    chown -R "$NMFTA_UID:$NMFTA_GID" "$NEW_ROOT/home/nmfta" 2>/dev/null || true
                fi
            else
                echo "[Stage3] Warning: nmfta user not found in identity_backup. Cannot restore."
            fi
        fi

        # 4. Update Device Tree Overlays and extlinux.conf
        echo "[Stage3] Updating Device Tree Overlays and Boot Config..."
        mkdir -p "$NEW_ROOT/boot/p1"
        if mount "$PART1" "$NEW_ROOT/boot/p1"; then
            # We avoid chrooting to run update-overlays due to host kernel incompatibilities.
            # Instead, we create a one-shot systemd service to run it on the first boot.
            if [ -x "$NEW_ROOT/usr/bin/update-overlays" ] || [ -x "$NEW_ROOT/bin/update-overlays" ]; then
                if grep -q "FDTOVERLAYS" "$NEW_ROOT/boot/p1/extlinux/extlinux.conf" 2>/dev/null; then
                    echo "[Stage3] Device Tree Overlays appear to be already configured. Skipping update-overlays."
                else
                    echo "[Stage3] Scheduling update-overlays for first boot..."
                    cat << 'EOF' > "$NEW_ROOT/etc/systemd/system/first-boot-update-overlays.service"
[Unit]
Description=Run update-overlays on first boot
After=multi-user.target

[Service]
Type=oneshot
ExecStart=/usr/bin/update-overlays
ExecStartPost=/bin/rm -f /etc/systemd/system/first-boot-update-overlays.service
ExecStartPost=/bin/rm -f /etc/systemd/system/multi-user.target.wants/first-boot-update-overlays.service
ExecStartPost=/bin/systemctl reboot
RemainAfterExit=yes

[Install]
WantedBy=multi-user.target
EOF
                    mkdir -p "$NEW_ROOT/etc/systemd/system/multi-user.target.wants"
                    ln -sf "/etc/systemd/system/first-boot-update-overlays.service" "$NEW_ROOT/etc/systemd/system/multi-user.target.wants/first-boot-update-overlays.service"
                fi
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
