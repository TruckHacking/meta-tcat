FILESEXTRAPATHS:prepend := "${THISDIR}:"

SRC_URI:append = " file://0001-Fix-dtype-mismatch-on-32-bit-systems.patch"

do_install:append() {
    rm -rf ${D}${PYTHON_SITEPACKAGES_DIR}/pandas/tests
}
