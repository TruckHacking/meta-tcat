SUMMARY = "Automotive Scapy Playground"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=45b26d404801eadc33ba5b829b12d3d7"
SRCREV = "20b7ba97c7d7c1dc9ee350420d75bcabf9ea08ae"
SRC_URI = "git://github.com/BenGardiner/automotive_scapy_playground.git;protocol=https;branch=main \
           file://start-playground.sh"

S = "${WORKDIR}/git"

do_configure[noexec] = "1"
do_compile[noexec] = "1"

do_install() {
    install -d ${D}${datadir}/automotive_scapy_playground
    cp -r ${S}/*.ipynb ${D}${datadir}/automotive_scapy_playground/
    cp -r ${S}/*.pcapng ${D}${datadir}/automotive_scapy_playground/
    
    install -d ${D}${bindir}
    install -m 0755 ${WORKDIR}/start-playground.sh ${D}${bindir}/start-playground
}

RDEPENDS:${PN} = " \
    python3-scapy \
    python3-can \
    python3-pyserial \
    python3-python-can-candle \
    python3-rp1210 \
    python3-jupyter \
    python3-notebook \
    python3-ipython \
    python3-ipywidgets \
    python3-numpy \
    python3-pandas \
    python3-qgridnext \
    python3-z3-solver \
    python3-bokeh \
    python3-tabulate \
"
FILES:${PN} += "${datadir}/automotive_scapy_playground"
