SUMMARY = "Python libs and scripts for pretty-printing J1939 logs"
DESCRIPTION = "A J1939 log pretty-printer and associated python libraries to decode SAE J1939 CAN messages."
HOMEPAGE = "https://github.com/nmfta-repo/pretty_j1939"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=86d3f3a95c324c9479bd8986968f4327"
SRC_URI = "git://github.com/nmfta-repo/pretty_j1939.git;protocol=https;branch=master"
SRCREV = "7778c342f8429c9a74b1efefaf0b704d5351f4b6"
S = "${WORKDIR}/git"
inherit python_setuptools_build_meta
RDEPENDS:${PN} = "python3-asteval python3-defusedxml python3-unidecode python3-xlrd python3-openpyxl python3-rich python3-bitstring python3-can"

FILES:${PN} += "${bindir}/pretty_j1939"
