SUMMARY = "Python libs and scripts for pretty-printing J1939 logs"
DESCRIPTION = "A J1939 log pretty-printer and associated python libraries to decode SAE J1939 CAN messages."
HOMEPAGE = "https://github.com/nmfta-repo/pretty_j1939"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=86d3f3a95c324c9479bd8986968f4327"
SRC_URI = "https://github.com/nmfta-repo/pretty_j1939/archive/refs/tags/v${PV}.tar.gz"
SRC_URI[sha256sum] = "31736cc5fc3a140f52fc8f9e1a22dcbddf3d24dbc19fa6482ab3072212f81e3f"
S = "${WORKDIR}/pretty_j1939-${PV}"
inherit python_setuptools_build_meta
RDEPENDS:${PN} = "python3-asteval python3-defusedxml python3-unidecode python3-xlrd python3-openpyxl python3-rich python3-bitstring python3-can"

FILES:${PN} += "${bindir}/pretty_j1939"
