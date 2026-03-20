#!/bin/bash
# Script to copy current Yocto configuration back to meta-tcat/conf.samples
# This reverses the copying done by tcat-setup-dev-env.sh and replaces the
# absolute paths with ${FULL_YOCTO_DIR}.

# Determine the absolute path to the Yocto base directory
YOCTO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SAMPLES_DIR="$YOCTO_DIR/meta-tcat/conf.samples"

echo "Copying layer.conf for meta-python2..."
if [ -f "$YOCTO_DIR/meta-python2/conf/layer.conf" ]; then
    cp "$YOCTO_DIR/meta-python2/conf/layer.conf" "$SAMPLES_DIR/meta-python2-layer.conf.sample"
fi

echo "Copying layer.conf for meta-jupyter..."
if [ -f "$YOCTO_DIR/meta-jupyter/conf/layer.conf" ]; then
    cp "$YOCTO_DIR/meta-jupyter/conf/layer.conf" "$SAMPLES_DIR/meta-jupyter-layer.conf.sample"
fi

echo "Copying build configurations..."
if [ -f "$YOCTO_DIR/build/conf/local.conf" ]; then
    cp "$YOCTO_DIR/build/conf/local.conf" "$SAMPLES_DIR/local.conf.sample"
fi
if [ -f "$YOCTO_DIR/build/conf/bblayers.conf" ]; then
    cp "$YOCTO_DIR/build/conf/bblayers.conf" "$SAMPLES_DIR/bblayers.conf.sample"
fi
if [ -f "$YOCTO_DIR/build/conf/conf-notes.txt" ]; then
    cp "$YOCTO_DIR/build/conf/conf-notes.txt" "$SAMPLES_DIR/conf-notes.txt"
fi
if [ -f "$YOCTO_DIR/build/conf/conf-summary.txt" ]; then
    cp "$YOCTO_DIR/build/conf/conf-summary.txt" "$SAMPLES_DIR/conf-summary.txt.sample"
fi

echo "Restoring \${FULL_YOCTO_DIR} variable in sample files..."
# Replace the absolute path with the variable ${FULL_YOCTO_DIR}
for file in "$SAMPLES_DIR"/local.conf.sample "$SAMPLES_DIR"/bblayers.conf.sample "$SAMPLES_DIR"/conf-notes.txt "$SAMPLES_DIR"/conf-summary.txt.sample; do
    if [ -f "$file" ]; then
        sed -i "s#$YOCTO_DIR#\${FULL_YOCTO_DIR}#g" "$file"
    fi
done

echo "Done updating conf.samples in meta-tcat."