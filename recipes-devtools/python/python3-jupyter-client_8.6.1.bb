inherit pypi python_hatchling

SUMMARY = "Jupyter protocol implementation and client libraries"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=083556a9912a35360dae8281fb57e886"

SRC_URI[sha256sum] = "e842515e2bab8e19186d89fdfea7abd15e39dd581f94e399f00e2af5a1652d3f"
PYPI_PACKAGE = "jupyter_client"

RDEPENDS:${PN} += " \
    ${PYTHON_PN}-pyzmq \
    ${PYTHON_PN}-traitlets \
    ${PYTHON_PN}-jupyter-core \
    ${PYTHON_PN}-tornado \
    ${PYTHON_PN}-dateutil \
"

do_install:append () {
    # Make sure we use /usr/bin/env python
    for PYTHSCRIPT in `grep -rIl '^#!.*python' ${D}`; do
        sed -i -e '1s|^#!.*|#!/usr/bin/env ${PYTHON_PN}|' $PYTHSCRIPT
    done
}

BBCLASSEXTEND = "native"
