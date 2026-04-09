DESCRIPTION = "The package provides SAE J1939 support for Python developers"
SECTION = "devel/python"
LICENSE = "MIT"

SRC_URI = "git://github.com/TruckHacking/py-hv-networks.git;protocol=https;rev=559d9e1dda2d21bfd7dec611f1e47a84e2fb449d;branch=master \
           file://0001-add-j2497-scripts.patch \
           file://0002-bitstring-compatibility.patch"

S = "${WORKDIR}/git"
inherit setuptools3

# TODO: test this
do_install(){
    # install scripts with .py extensions
    install -d ${D}${bindir}
    install -m 0755 ${S}/j1708dump.py ${D}${bindir}/j1708dump.py
    install -m 0755 ${S}/j1708send.py ${D}${bindir}/j1708send.py
    install -m 0755 ${S}/j2497dump.py ${D}${bindir}/j2497dump.py
    install -m 0755 ${S}/j2497send.py ${D}${bindir}/j2497send.py

    # install everything just in case
    install -d ${D}${PYTHON_SITEPACKAGES_DIR}/hv_networks
    install -m 0644 ${S}/hv_networks/* ${D}/${PYTHON_SITEPACKAGES_DIR}/hv_networks/
}

FILES:${PN} += "*"