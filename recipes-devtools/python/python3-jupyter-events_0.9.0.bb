inherit pypi python_hatchling

SUMMARY = "Jupyter Event System library"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=469c300e985ff0b264c3ba21b4fae725"

SRC_URI[sha256sum] = "81ad2e4bc710881ec274d31c6c50669d71bbaa5dd9d01e600b56faa85700d399"

PYPI_PACKAGE = "jupyter_events"

RDEPENDS:${PN} += " \
    ${PYTHON_PN}-referencing \
    ${PYTHON_PN}-jsonschema \
    ${PYTHON_PN}-python-json-logger \
    ${PYTHON_PN}-pyyaml \
    ${PYTHON_PN}-traitlets \
    ${PYTHON_PN}-rfc3339-validator \
    ${PYTHON_PN}-rfc3986-validator \
"

BBCLASSEXTEND = "native"
