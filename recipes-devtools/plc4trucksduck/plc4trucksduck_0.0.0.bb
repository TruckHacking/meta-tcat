DESCRIPTION = "PLC4TRUCKSDUCK service for TCAT"
LICENSE = "MIT"

SRC_URI += " file://plc-dev"

TARGET_DIR = "/opt/tcat/programs/plc-dev"

inherit systemd
SYSTEMD_SERVICE:${PN} = "plc4trucksduck.service"
SYSTEMD_AUTO_ENABLE = "enable"

DEPENDS += "ti-cgt-pru-native dtc-native"

do_compile() {
    # compile C host tools
    ${CC} ${CFLAGS} ${LDFLAGS} ${WORKDIR}/plc-dev/plc4trucksduck/src/arm/plc4trucksduck_host.c -o ${WORKDIR}/plc-dev/plc4trucksduck/src/arm/plc4trucksduck_host_c -lpthread
    ${CC} ${CFLAGS} ${LDFLAGS} ${WORKDIR}/plc-dev/plc4trucksduck/src/arm/j1708send.c -o ${WORKDIR}/plc-dev/plc4trucksduck/src/arm/j1708send_c
    ${CC} ${CFLAGS} ${LDFLAGS} ${WORKDIR}/plc-dev/plc4trucksduck/src/arm/j1708dump.c -o ${WORKDIR}/plc-dev/plc4trucksduck/src/arm/j1708dump_c

    # compile PRU firmware
    make -C ${WORKDIR}/plc-dev/plc4trucksduck PRU_CGT=${STAGING_DATADIR_NATIVE}/ti/cgt-pru CLPRU=${STAGING_DATADIR_NATIVE}/ti/cgt-pru/bin/clpru
}

do_install(){

    # make a backup of development environment
    install -d ${D}${TARGET_DIR}
    cp -r ${WORKDIR}/plc-dev/* ${D}${TARGET_DIR}

    # install program directories
    install -d ${D}/usr/bin
    install -d ${D}/usr/lib/firmware

    # firmware (using newly compiled outputs)
    cp ${WORKDIR}/plc-dev/plc4trucksduck/src/pru/generated/plc4trucksduck.out ${D}/usr/lib/firmware/am335x-pru0-fw
    cp ${WORKDIR}/plc-dev/plc4trucksduck/src/pru/generated/plc4trucksduck.out ${D}/usr/lib/firmware/am335x-pru0-fw-default
    cp ${WORKDIR}/plc-dev/plc4trucksduck/src/pru/generated/plc4trucksduck_bitbang.out ${D}/usr/lib/firmware/am335x-pru0-fw-bitbang
    cp ${WORKDIR}/plc-dev/plc4trucksduck/src/pru/generated/plc4trucksduck_j1708.out ${D}/usr/lib/firmware/am335x-pru0-fw-j1708

    # user space code (has to be root to access PRU)
    install -d ${D}/usr/sbin
    install -m 0755 ${WORKDIR}/plc-dev/plc4trucksduck/src/arm/set-tcat-pru ${D}/usr/sbin/
    install -m 0755 ${WORKDIR}/plc-dev/plc4trucksduck/src/arm/plc4trucksduck_host_c ${D}/usr/bin/plc4trucksduck_host
    
    install -m 0755 ${WORKDIR}/plc-dev/plc4trucksduck/src/arm/plc4trucksduck_host ${D}/usr/bin/plc4trucksduck_host.py
    
    # client tools
    install -m 0755 ${WORKDIR}/plc-dev/plc4trucksduck/src/arm/j1708send_c ${D}/usr/bin/j1708send
    install -m 0755 ${WORKDIR}/plc-dev/plc4trucksduck/src/arm/j1708dump_c ${D}/usr/bin/j1708dump
    ln -s j1708send ${D}/usr/bin/j2497send
    ln -s j1708dump ${D}/usr/bin/j2497dump

    # services
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/plc-dev/plc4trucksduck/src/arm/plc4trucksduck.service ${D}${systemd_system_unitdir}/

    # just in case the user wants to force stop the PRU
    install -m 0755 ${WORKDIR}/plc-dev/plc4trucksduck/stop_remoteproc ${D}/usr/bin
}

FILES:${PN} += "${TARGET_DIR} \
                /usr/sbin/* \
                /usr/lib/*"
FILES:${PN} += "${systemd_system_unitdir}/*.service"

# only one can be enabled at a time with PRU resources available
# _hard requirement_ on 6.6.32 due to pre-compiled pru binary
RDEPENDS:${PN} += "kernel-base (= 6.6.32) bash python3-core python3"
# this package has firmware blobs so we skip the QA consistency check
INSANE_SKIP:${PN} += "arch buildpaths file-rdeps"
