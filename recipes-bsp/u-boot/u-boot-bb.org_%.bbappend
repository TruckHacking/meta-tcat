FILESEXTRAPATHS:prepend := "${THISDIR}/files:"
SRC_URI += " file://0001-BOTHBOOT.patch"

# Set the COMPATIBLE_MACHINE variable to include the TCAT
COMPATIBLE_MACHINE = "tcat"
