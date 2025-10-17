FILESEXTRAPATHS:prepend := "${THISDIR}/files:"
SRC_URI += "file://BB-MCP251xFD-SPI.dts \
            file://BB-PLC4TRUCKSDUC-00A0.dts \
            file://BB-UART5-00A0.dts \
            file://BB-UTHP-DCAN.dts \
            file://BB-UTHP-GPIO.dts \
            file://BB-UTHP-LEDS.dts \
            file://update-overlays \
            file://Makefile \
            file://README.md \
            "

LICENSE = "MIT"
### REMEMBER TO dtc -O dtb -o <something>.dtbo -b 0 -@ <something>.dts ###
do_install() {
    install -d ${D}/boot/dtb/tcat
    install -d ${D}/boot/dts/tcat
    install -m 0644 ${WORKDIR}/*.dts ${D}/boot/dts/tcat
    # install makefile
    install -m 0644 ${WORKDIR}/Makefile ${D}/boot/dts/tcat
    # install README
    install -m 0644 ${WORKDIR}/README.md ${D}/boot/dts/tcat

    install -d ${D}/usr/bin
    install -m 0755 ${WORKDIR}/update-overlays ${D}/usr/bin
}

RDEPENDS:${PN} += "bash"

FILES:${PN} += "/boot/* /usr/bin/*"
