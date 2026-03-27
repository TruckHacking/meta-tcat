inherit pypi python_hatchling

SUMMARY = "A Jupyter Server Extension Providing Terminals."
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=ee82bc15ab23966cc24cc4e361736bda"

SRC_URI[sha256sum] = "5ae0295167220e9ace0edcfdb212afd2b01ee8d179fe6f23c899590e9b8a5269"

PYPI_PACKAGE = "jupyter_server_terminals"

RDEPENDS:${PN} += " \
    ${PYTHON_PN}-terminado \
"

do_install:append() {
    if [ -d ${D}${prefix}/etc ]; then
        if [ -d ${D}${sysconfdir} ]; then
            cp -r ${D}${prefix}/etc/* ${D}${sysconfdir}/
            rm -rf ${D}${prefix}/etc
        else
            mv ${D}${prefix}/etc ${D}${sysconfdir}
        fi
    fi
}

FILES:${PN} += "${sysconfdir}/jupyter"

BBCLASSEXTEND = "native"
