FILESEXTRAPATHS:prepend := "${THISDIR}/files:"
SRC_URI += "file://0001-rtc.patch \
            file://0001-intc-plc.patch"
            
COMPATIBLE_MACHINE = "tcat"
