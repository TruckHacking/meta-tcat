SUMMARY = "Controller Area Network (CAN) interface module for Python"
DESCRIPTION = "python-can provides Controller Area Network (CAN) support for Python developers; providing common abstractions to different hardware devices, and a suite of utilities for sending and receiving messages on a CAN bus."
HOMEPAGE = "https://github.com/hardbyte/python-can"
LICENSE = "LGPL-3.0-only"
LIC_FILES_CHKSUM = "file://LICENSE.txt;md5=e6a600fd5e1d9cbde2d983680233ad02"

SRC_URI = "https://pypi.io/packages/source/p/python-can/python_can-${PV}.tar.gz"
SRC_URI[sha256sum] = "290fea135d04b8504ebff33889cc6d301e2181a54099116609f940825ffe5005"

S = "${WORKDIR}/python_can-${PV}"
inherit python_setuptools_build_meta
DEPENDS += "python3-setuptools-scm-native"
RDEPENDS:${PN} = "python3-msgpack python3-wrapt"

do_compile:prepend() {
    sed -i -e 's/license = "LGPL-3.0-only"/license = { text = "LGPL-3.0-only" }/g' ${S}/pyproject.toml || true
    sed -i -e 's/setuptools *>= *[0-9.]*/setuptools/g' ${S}/pyproject.toml || true
    sed -i -e 's/setuptools_scm *>= *[0-9.]*/setuptools_scm/g' ${S}/pyproject.toml || true
}

FILES:${PN} += "${bindir}/can_logconvert ${bindir}/can_logger ${bindir}/can_player ${bindir}/can_viewer ${bindir}/can_bridge"