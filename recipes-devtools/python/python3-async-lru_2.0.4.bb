inherit pypi setuptools3

SUMMARY = "Simple LRU cache for asyncio"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=637551ffe345b083f0847cebe17a634d"

SRC_URI[sha256sum] = "b8a59a5df60805ff63220b2a0c5b5393da5521b113cd5465a44eb037d81a5627"

PYPI_PACKAGE = "async-lru"

RDEPENDS:${PN} += " \
    ${PYTHON_PN}-typing-extensions \
"

BBCLASSEXTEND = "native"
