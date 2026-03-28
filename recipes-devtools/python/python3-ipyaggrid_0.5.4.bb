SUMMARY = "Jupyter widget for ag-grid"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=f0c5dc14e4c09d3ed786d6a30cfb0b71"

SRC_URI[sha256sum] = "e2e4c5902e97c9c24c688cb2176876fa60e62545605b5804c9f6b563655df029"

inherit pypi python_setuptools_build_meta

DEPENDS += " \
    python3-jupyter-packaging-native \
    python3-deprecation-native \
    python3-tomlkit-native \
"

do_configure:prepend() {
    sed -i 's/"jupyterlab>=3.0.0,==3.*", //' ${S}/pyproject.toml
    sed -i 's/"jupyter_packaging~=0.7.9"/"jupyter_packaging"/' ${S}/pyproject.toml
}

do_install:append() {
    if [ -d ${D}${prefix}/etc ]; then
        install -d ${D}${sysconfdir}
        mv ${D}${prefix}/etc/* ${D}${sysconfdir}/
        rm -rf ${D}${prefix}/etc
    fi
}

RDEPENDS:${PN} += " \
    python3-ipywidgets \
    python3-pandas \
    python3-simplejson \
"

FILES:${PN} += " \
    ${datadir}/jupyter \
    ${sysconfdir}/jupyter \
"
