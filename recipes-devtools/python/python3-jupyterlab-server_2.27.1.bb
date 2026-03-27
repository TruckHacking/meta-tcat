inherit pypi python_hatchling

SUMMARY = "A set of server components for JupyterLab and JupyterLab like applications."
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=16b24abb4aef09551533365c88c785cf"

SRC_URI[sha256sum] = "097b5ac709b676c7284ac9c5e373f11930a561f52cd5a86e4fc7e5a9c8a8631d"

PYPI_PACKAGE = "jupyterlab_server"

RDEPENDS:${PN} += " \
    ${PYTHON_PN}-babel \
    ${PYTHON_PN}-jinja2 \
    ${PYTHON_PN}-json5 \
    ${PYTHON_PN}-jsonschema \
    ${PYTHON_PN}-jupyter-server \
    ${PYTHON_PN}-packaging \
    ${PYTHON_PN}-requests \
"

BBCLASSEXTEND = "native"
