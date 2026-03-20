SUMMARY = "candleLight USB CAN interface support for python-can"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"
SRC_URI = "https://files.pythonhosted.org/packages/c5/16/afc489f64a7128dda1eb880ba245a7eb63c3ff9c336641d5550df7b501eb/python_can_candle-1.2.4.tar.gz"
SRC_URI[sha256sum] = "f5a1d016579045e596931fd5377102816d9f8e891586e07ac19458588f8a80f2"
S = "${WORKDIR}/python_can_candle-${PV}"
inherit python_setuptools_build_meta
