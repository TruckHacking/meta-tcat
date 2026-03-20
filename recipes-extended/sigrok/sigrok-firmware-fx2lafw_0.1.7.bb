SUMMARY = "Firmware for Cypress FX2 based logic analyzers"
HOMEPAGE = "http://sigrok.org/wiki/Fx2lafw"
LICENSE = "GPL-2.0-or-later"
LIC_FILES_CHKSUM = "file://COPYING;md5=751419260aa954499f7abaabaa882bbe \
                    file://COPYING.LESSER;md5=4fbd65380cdd255951079008b364516c"

SRC_URI = "https://sigrok.org/download/binary/${BPN}/${BPN}-bin-${PV}.tar.gz"
SRC_URI[sha256sum] = "c876fd075549e7783a6d5bfc8d99a695cfc583ddbcea0217d8e3f9351d1723af"

S = "${WORKDIR}/${PN}-bin-${PV}"

inherit allarch

do_install() {
    install -d ${D}${datadir}/sigrok-firmware
    install -m 0644 ${S}/*.fw ${D}${datadir}/sigrok-firmware/
}

FILES:${PN} = "${datadir}/sigrok-firmware"
