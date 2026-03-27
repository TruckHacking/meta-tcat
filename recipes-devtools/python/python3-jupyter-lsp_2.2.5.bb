inherit pypi setuptools3

SUMMARY = "Multi-Language Server WebSocket proxy for Jupyter Notebook/Lab server"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=7c4ff345ebb091fcbc5290142e324da5"

SRC_URI[sha256sum] = "793147a05ad446f809fd53ef1cd19a9f5256fd0a2d6b7ce943a982cb4f545001"

PYPI_PACKAGE = "jupyter-lsp"

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
