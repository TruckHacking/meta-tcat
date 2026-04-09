DESCRIPTION = "Useful in interacting with trucks that use J1939"
SECTION = "devel/python"
LICENSE = "GPL-3.0-only"
LIC_FILES_CHKSUM = "file://${WORKDIR}/git/LICENSE.md;md5=1ebbd3e34237af26da5dc08a4e440464"

SRC_URI = "git://github.com/BenGardiner/TruckDevil.git;protocol=https;branch=ecu_disco_improv"

SRCREV = "a88239e8d846389e53e516fbadf04a6315e387f2"

S = "${WORKDIR}/git"

inherit setuptools3

RDEPENDS:${PN} += " \
    bash \
    python3 \
    python3-bitstring \
    python3-can \
    python3-core \
    python3-dill \
    python3-prompt-toolkit \
    python3-pyserial \
    python3-setuptools \
"
