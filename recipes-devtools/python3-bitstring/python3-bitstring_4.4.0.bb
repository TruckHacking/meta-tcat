SUMMARY = "A Python module for the creation, manipulation and analysis of binary data."
DESCRIPTION = "bitstring is a pure Python module designed to help make the creation and analysis of binary data as simple and natural as possible."
HOMEPAGE = "https://github.com/scott-griffiths/bitstring"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=963a24c175e658fbf16a764135121ffa"

SRC_URI = "https://pypi.io/packages/source/b/bitstring/bitstring-${PV}.tar.gz"
SRC_URI[sha256sum] = "e682ac522bb63e041d16cbc9d0ca86a4f00194db16d0847c7efe066f836b2e37"

S = "${WORKDIR}/bitstring-${PV}"
inherit python_setuptools_build_meta
RDEPENDS:${PN} = "python3-bitarray"
