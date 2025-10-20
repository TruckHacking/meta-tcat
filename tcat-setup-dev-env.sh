#!/bin/bash

YOCTO_DIR="Yocto"
LOG_FILE="tcat-layers-setup.log"

exec > >(tee -a "$LOG_FILE") 2>&1

# Helper functions
function handle_error() {
    echo "Error on line $1"
    exit 1
}

SCARTHGAP_TAG="scarthgap-5.0.12"
function clone_and_checkout() {
    echo -e "\n==> Cloning and checking out layers... (please be patient as this takes some time)\n"

    # Clone and checkout poky if the directory does not exist or the directory is empty
    if [ ! -d "$FULL_YOCTO_DIR/" ] || [ -z "$(ls -A $FULL_YOCTO_DIR/)" ]; then
        git clone git://git.yoctoproject.org/poky.git "$FULL_YOCTO_DIR/" || handle_error $LINENO
    fi
    cd "$FULL_YOCTO_DIR/" || handle_error $LINENO
    git checkout ${SCARTHGAP_TAG} || handle_error $LINENO
    pwd || handle_error $LINENO

    # Clone and checkout meta-openembedded
    if [ ! -d "$FULL_YOCTO_DIR/meta-openembedded" ]; then
        git clone git://git.openembedded.org/meta-openembedded "$FULL_YOCTO_DIR/meta-openembedded" || handle_error $LINENO
    fi
    cd "$FULL_YOCTO_DIR/meta-openembedded" || handle_error $LINENO
    # scarthgap branch commit as of 20251011
    git checkout e621da9470 || handle_error $LINENO
    cd "$FULL_YOCTO_DIR" || handle_error $LINENO

    # Clone meta-python2
    if [ ! -d "$FULL_YOCTO_DIR/meta-python2" ]; then
        git clone git://git.openembedded.org/meta-python2 "$FULL_YOCTO_DIR/meta-python2" || handle_error $LINENO
    fi
    cd "$FULL_YOCTO_DIR/meta-python2" || handle_error $LINENO
    # main/master branch commit as of 20251011
    git checkout 1358cdb || handle_error $LINENO
    cd - || handle_error $LINENO

    # Clone meta-jupyter
    if [ ! -d "$FULL_YOCTO_DIR/meta-jupyter" ]; then
        git clone https://github.com/Xilinx/meta-jupyter "$FULL_YOCTO_DIR/meta-jupyter" || handle_error $LINENO
    fi
    cd "$FULL_YOCTO_DIR/meta-jupyter" || handle_error $LINENO
    # master/main branch as of 20251011
    git checkout d965100 || handle_error $LINENO
    cd - || handle_error $LINENO

    # Clone and checkout meta-arm
    if [ ! -d "$FULL_YOCTO_DIR/meta-arm" ]; then
        git clone git://git.yoctoproject.org/meta-arm "$FULL_YOCTO_DIR/meta-arm" || handle_error $LINENO
    fi
    cd "$FULL_YOCTO_DIR/meta-arm" || handle_error $LINENO
    # scarthgap branch commit as of 20251011
    git checkout 0f1e7bf9 || handle_error $LINENO
    cd "$FULL_YOCTO_DIR" || handle_error $LINENO

    # Clone and checkout meta-ti
    if [ ! -d "$FULL_YOCTO_DIR/meta-ti" ]; then
        git clone git://git.yoctoproject.org/meta-ti "$FULL_YOCTO_DIR/meta-ti" || handle_error $LINENO
    fi
    cd "$FULL_YOCTO_DIR/meta-ti" || handle_error $LINENO
    # last version before update to kernel 6.12
    git checkout 11.00.14 || handle_error $LINENO
    cd "$FULL_YOCTO_DIR" || handle_error $LINENO

    # Clone and checkout meta-tcat
    if [ ! -d "$FULL_YOCTO_DIR/meta-tcat" ]; then
        git clone https://github.com/TruckHacking/meta-tcat "$FULL_YOCTO_DIR/meta-tcat" || handle_error $LINENO
    fi
    cd "$FULL_YOCTO_DIR/meta-tcat" || handle_error $LINENO
    # TODO change to a tag
    git checkout scarthgap || handle_error $LINENO
    cd "$FULL_YOCTO_DIR" || handle_error $LINENO
}

echo -e "\n==>Installing necessary packages...\n"
# 1. setup build host
sudo apt update
sudo apt install -y gawk wget git diffstat unzip texinfo gcc build-essential chrpath socat cpio python3 python3-pip python3-pexpect xz-utils debianutils iputils-ping python3-git python3-jinja2 libsdl1.2-dev pylint xterm python3-subunit mesa-common-dev zstd liblz4-tool || handle_error $LINENO
sudo apt install -y libegl1-mesa || echo "Warning: Optional package 'libegl1-mesa' may need to be installed manually." # ubuntu 24 issue
sudo dpkg --add-architecture i386 # for ti-cgt-pru
sudo apt update
sudo apt install libc6:i386
echo -e "\n==> Package installation completed.\n"

# 2. Clone the Yocto layers
mkdir -p "$YOCTO_DIR" || handle_error $LINENO
FULL_YOCTO_DIR=$(cd "$YOCTO_DIR" && pwd) || handle_error $LINENO
echo -e "\n==> Yocto development directory located at: $FULL_YOCTO_DIR\n"
clone_and_checkout

# Add the meta-tcat conf.samples to their respective directories
echo -e "\n==> Copying layer configuration files from the meta-tcat repo to $FULL_YOCTO_DIR/build/conf\n"
mkdir -p "$FULL_YOCTO_DIR/build/conf" || handle_error $LINENO
cp "$FULL_YOCTO_DIR/meta-tcat/conf.samples/meta-python2-layer.conf.sample" "$FULL_YOCTO_DIR/meta-python2/conf/layer.conf" || handle_error $LINENO
cp "$FULL_YOCTO_DIR/meta-tcat/conf.samples/meta-jupyter-layer.conf.sample" "$FULL_YOCTO_DIR/meta-jupyter/conf/layer.conf" || handle_error $LINENO
cp "$FULL_YOCTO_DIR/meta-tcat/conf.samples/local.conf.sample" "$FULL_YOCTO_DIR/build/conf/local.conf" || handle_error $LINENO
cp "$FULL_YOCTO_DIR/meta-tcat/conf.samples/bblayers.conf.sample" "$FULL_YOCTO_DIR/build/conf/bblayers.conf" || handle_error $LINENO
cp "$FULL_YOCTO_DIR/meta-tcat/conf.samples/conf-notes.txt" "$FULL_YOCTO_DIR/build/conf/conf-notes.txt" || handle_error $LINENO
cp "$FULL_YOCTO_DIR/meta-tcat/conf.samples/conf-summary.txt.sample" "$FULL_YOCTO_DIR/build/conf/conf-summary.txt" || handle_error $LINENO
for file in $FULL_YOCTO_DIR/build/conf/*; do sed -i "s#\${FULL_YOCTO_DIR}#$(pwd)#g" "$file"; done
echo -e "\n==> Configuration files copied. Please run 'source oe-init-build-env' from WITHIN the 'Yocto' dir to run bitbake commands.\n"
