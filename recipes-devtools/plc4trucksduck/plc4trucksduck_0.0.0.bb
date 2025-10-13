DESCRIPTION = "PLC4TRUCKSDUCK installation for the UTHP"
LICENSE = "MIT"

SRC_URI += " file://plc-dev"

TARGET_DIR = "/opt/uthp/programs/plc-dev"

inherit systemd
SYSTEMD_SERVICE:${PN} = "plc4trucksduck.service"
SYSTEMD_AUTO_ENABLE = "enable"

do_install(){

    # make a backup of development environment
    install -d ${D}${TARGET_DIR}
    cp -r ${WORKDIR}/plc-dev/* ${D}${TARGET_DIR}

    # install program directories
    install -d ${D}/usr/bin
    install -d ${D}/usr/lib/firmware

    # firmware (needs to be copied becuase of objcopy errors in Yocto)
    cp ${WORKDIR}/plc-dev/plc4trucksduck/src/pru/generated/plc4trucksduck.out ${D}/usr/lib/firmware/am335x-pru0-fw
    cp ${WORKDIR}/plc-dev/plc4trucksduck/src/pru/generated/j17084truckduck.out ${D}/usr/lib/firmware/am335x-pru1-fw

    # user space code (has to be root to access PRU)
    install -m 0755 ${WORKDIR}/plc-dev/plc4trucksduck/src/arm/plc4trucksduck_host ${D}/usr/bin/
    install -m 0755 ${WORKDIR}/plc-dev/plc4trucksduck/src/arm/j17084truckduck_host ${D}/usr/bin/

    # services
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/plc-dev/plc4trucksduck/src/arm/j17084truckduck.service ${D}${systemd_system_unitdir}/
    install -m 0644 ${WORKDIR}/plc-dev/plc4trucksduck/src/arm/plc4trucksduck.service ${D}${systemd_system_unitdir}/

    # just in case the user wants to force stop the PRU
    install -m 0755 ${WORKDIR}/plc-dev/plc4trucksduck/stop_remoteproc ${D}/usr/bin
}

FILES:${PN} += "${TARGET_DIR} \
                /usr/lib/*"
FILES:${PN} += "${systemd_system_unitdir}/plc4trucksduck.service"

# only one can be enabled at a time with PRU resources available
RDEPENDS:${PN} += "python3-core python3 python bash"
# _hard requirement_ on 6.6.32 due to pre-compiled pru binary
RDEPENDS:${PN} += "kernel-base (= 6.6.32)"
# this package has firmware blobs so we skip the QA consistency check
INSANE_SKIP:${PN} += "arch"
