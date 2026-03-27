inherit pypi python_hatchling

SUMMARY = "Jupyter core package"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=a5cc41e8bc83e8e689ce2c7bb0ceba14"

RDEPENDS:${PN} += " \
    ${PYTHON_PN}-platformdirs \
    ${PYTHON_PN}-traitlets \
"

PYPI_PACKAGE = "jupyter_core"

BBCLASSEXTEND = "native"

SRC_URI[sha256sum] = "aa5f8d32bbf6b431ac830496da7392035d6f61b4f54872f15c4bd2a9c3f536d9"

do_install:append() {
    # Make sure we use /usr/bin/env python
    for PYTHSCRIPT in `grep -rIl '^#!.*python' ${D}`; do
        sed -i -e '1s|^#!.*|#!/usr/bin/env ${PYTHON_PN}|' $PYTHSCRIPT
    done

    # these files will be installed by python-jupyter
    rm -f ${D}${PYTHON_SITEPACKAGES_DIR}/jupyter.py
    rm -f ${D}${PYTHON_SITEPACKAGES_DIR}/__pycache__/jupyter.cpython-*.pyc
}
