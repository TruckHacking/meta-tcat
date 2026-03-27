SUMMARY = "An interactive grid for sorting, filtering, and editing DataFrames in Jupyter notebooks"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"
SRC_URI = "https://files.pythonhosted.org/packages/16/12/4379b367190c06526050d45e96c9747d8cf4fff08fc3b734be8e2c1870c1/qgridnext-2.0.4.tar.gz \
           file://0001-Fix-dtype-mismatch-on-32-bit-systems.patch"

SRC_URI[sha256sum] = "5312d688f133fb9622ccf819f5bef99d6df86a343122cc193c35801a732a5ccc"

S = "${WORKDIR}/qgridnext-${PV}"

inherit python_setuptools_build_meta

RDEPENDS:${PN} = " \
    python3-pandas \
    python3-ipywidgets \
    python3-numpy \
    python3-traitlets \
"

do_install:append() {
    if [ -d ${D}${prefix}/etc ]; then
        install -d ${D}${sysconfdir}
        mv ${D}${prefix}/etc/* ${D}${sysconfdir}/
        rm -rf ${D}${prefix}/etc
    fi
}

FILES:${PN} += " \
    ${datadir}/jupyter \
    ${sysconfdir}/jupyter \
"
