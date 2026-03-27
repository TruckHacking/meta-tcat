FILESEXTRAPATHS:prepend := "${THISDIR}/python3-pandas:"

SRC_URI:append = " file://0001-Fix-dtype-mismatch-on-32-bit-systems.patch"
