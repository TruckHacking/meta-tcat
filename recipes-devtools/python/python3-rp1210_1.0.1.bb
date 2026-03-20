SUMMARY = "RP1210 package"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"
SRC_URI = "https://files.pythonhosted.org/packages/53/45/04c714c517dd486add79a9d27b234eb9f741498137d42117c32be2ca6282/rp1210-1.0.1.tar.gz"
SRC_URI[sha256sum] = "e30198b0768d87aa52689c011f4cefaee5ab13f2ce5d23c4a27d0e3e37496e0a"
S = "${WORKDIR}/rp1210-${PV}"
inherit python_setuptools_build_meta
