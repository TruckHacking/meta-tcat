SUMMARY = "A syntax highlighting package written in Python"
DESCRIPTION = "Pygments is a Python library for rich text and beautiful formatting in the terminal."
HOMEPAGE = "https://pygments.org/"
LICENSE = "BSD-2-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=36a13c90514e2899f1eba7f41c3ee592"
SRC_URI = "https://pypi.io/packages/source/P/Pygments/pygments-${PV}.tar.gz"
SRC_URI[sha256sum] = "636cb2477cec7f8952536970bc533bc43743542f70392ae026374600add5b887"
S = "${WORKDIR}/pygments-${PV}"

inherit python_hatchling
RDEPENDS:${PN} = ""
BBCLASSEXTEND = "native nativesdk"