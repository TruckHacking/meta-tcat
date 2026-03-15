SUMMARY = "Enable and configure systemd-resolved (LLMNR) at boot"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"
PR = "r2"

SRC_URI = "file://resolved.conf"

S = "${WORKDIR}"

do_install() {
    install -d ${D}${sysconfdir}/systemd/resolved.conf.d
    install -m 0644 ${WORKDIR}/resolved.conf ${D}${sysconfdir}/systemd/resolved.conf.d/llmnr.conf
}

FILES:${PN} += "${sysconfdir}/systemd/resolved.conf.d/llmnr.conf"

# Ensure nss-resolve is available
RDEPENDS:${PN} += "systemd"
