SUMMARY = "XML bomb protection for Python stdlib modules"
DESCRIPTION = "The defusedxml package contains several Python-only workarounds and fixes for denial-of-service and other vulnerabilities in Python's XML libraries."
HOMEPAGE = "https://github.com/tiran/defusedxml"
LICENSE = "PSF-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=056fea6a4b395a24d0d278bf5c80249e"

SRC_URI = "https://pypi.io/packages/source/d/defusedxml/defusedxml-${PV}.tar.gz"
SRC_URI[sha256sum] = "1bb3032db185915b62d7c6209c5a8792be6a32ab2fedacc84e01b52c51aa3e69"

S = "${WORKDIR}/defusedxml-${PV}"
inherit pypi setuptools3
RDEPENDS:${PN} = ""
