SUMMARY = "Z3 solver (Fixed version to match requirements)"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"
SRC_URI = "https://files.pythonhosted.org/packages/3a/fd/e3f5850fd04480a942aca9f9f7520d3fa5b57731335c221a11f55bb6d91a/z3-solver-4.13.0.0.tar.gz"
SRC_URI[sha256sum] = "52588e92aec7cb338fd6288ce93758ae01770f62ca0c80e8f4f2b2333feaf51b"
S = "${WORKDIR}/z3-solver-${PV}"
inherit python_setuptools_build_meta
DEPENDS += "cmake-native python3-cmake-native"
