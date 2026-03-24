#!/bin/bash
set -euo pipefail

# Directory of this script (meta-tcat/)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
META_TCAT_DIR="$SCRIPT_DIR"
YOCTO_DIR="$(dirname "$META_TCAT_DIR")"
LIVE_INSTALL_DIR="$SCRIPT_DIR/live-install"

# Read DISTRO_VERSION from TruckHackingOS.conf
DISTRO_CONF="$META_TCAT_DIR/conf/distro/TruckHackingOS.conf"
if [ ! -f "$DISTRO_CONF" ]; then
    echo "Error: Cannot find $DISTRO_CONF"
    exit 1
fi

DISTRO_VERSION=$(grep '^DISTRO_VERSION' "$DISTRO_CONF" | cut -d'"' -f2 || true)
if [ -z "$DISTRO_VERSION" ]; then
    echo "Error: Could not extract DISTRO_VERSION"
    exit 1
fi

# Get current meta-tcat commit
COMMIT=$(git -C "$META_TCAT_DIR" rev-parse --short HEAD 2>/dev/null || echo "unknown")

# Define package name
PKG_NAME="live-install-truckhackingos-tcat-${DISTRO_VERSION}-${COMMIT}"
TARBALL_NAME="${PKG_NAME}.tgz"

# Define target tarball location (meta-tcat/live-install/)
PACKAGE_OUTPUT_PATH="$LIVE_INSTALL_DIR/$TARBALL_NAME"

echo "Creating package: $TARBALL_NAME"

# Create a temporary staging directory
STAGING_DIR=$(mktemp -d)
trap 'rm -rf "$STAGING_DIR"' EXIT

PKG_DIR="$STAGING_DIR/$PKG_NAME"
mkdir -p "$PKG_DIR"

# Copy scripts
echo "Copying scripts..."
cp "$LIVE_INSTALL_DIR/update-beaglebone-suicide.sh" "$PKG_DIR/"
cp "$LIVE_INSTALL_DIR/suicide-payload-base.sh" "$PKG_DIR/"
cp "$LIVE_INSTALL_DIR/suicide-stage2.sh" "$PKG_DIR/"

# Make scripts executable
chmod +x "$PKG_DIR"/*.sh

# Locate and copy the image
IMAGE_SRC="$YOCTO_DIR/build/deploy-ti/images/tcat/core-image-tcat.rootfs.wic.xz"
if [ ! -f "$IMAGE_SRC" ]; then
    echo "Error: Image file not found at $IMAGE_SRC"
    echo "Please ensure you have built the core-image."
    exit 1
fi

echo "Copying image..."
cp "$IMAGE_SRC" "$PKG_DIR/"

# Create the tarball
echo "Creating tarball..."
tar -czf "$PACKAGE_OUTPUT_PATH" -C "$STAGING_DIR" "$PKG_NAME"

echo "Successfully created $PACKAGE_OUTPUT_PATH"
