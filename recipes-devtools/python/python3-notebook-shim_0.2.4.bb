inherit pypi python_hatchling

SUMMARY = "A shim layer for notebook traits and config"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=f91a22ac359078bf4380ccbace244c41"

SRC_URI[sha256sum] = "b4b2cfa1b65d98307ca24361f5b30fe785b53c3fd07b7a47e89acb5e6ac638cb"

PYPI_PACKAGE = "notebook_shim"

RDEPENDS:${PN} += " \
    ${PYTHON_PN}-jupyter-server \
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
