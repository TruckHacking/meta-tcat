inherit pypi python_hatchling

SUMMARY = "Jupyter Notebook - A web-based notebook environment for interactive computing"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=083556a9912a35360dae8281fb57e886"

SRC_URI[sha256sum] = "41fcebff44cf7bb9377180808bcbae066629b55d8c7722f1ebbe75ca44f9cfc1"

PYPI_PACKAGE = "notebook"

do_configure:prepend() {
    # Remove jupyter-builder hook and its dependency to avoid native dependency hell
    # as static assets are already in the sdist
    sed -i 's/requires = \["hatchling>=1.11", "jupyterlab>=4.1.1,<4.2"\]/requires = \["hatchling>=1.11"\]/' ${S}/pyproject.toml
    sed -i '/\[tool.hatch.build.hooks.jupyter-builder\]/,/install-pre-commit-hook = true/d' ${S}/pyproject.toml
}

DEPENDS += " \
    ${PYTHON_PN}-hatch-jupyter-builder-native \
"

RDEPENDS:${PN} += " \
    ${PYTHON_PN}-jupyter-server \
    ${PYTHON_PN}-jupyterlab \
    ${PYTHON_PN}-jupyterlab-server \
    ${PYTHON_PN}-notebook-shim \
    ${PYTHON_PN}-tornado \
"

do_install:append() {
    if [ -d ${D}${prefix}/etc ]; then
        if [ -d ${D}${sysconfdir} ]; then
            cp -r ${D}${prefix}/etc/* ${D}${sysconfdir}/
            rm -rf ${D}${prefix}/etc
        else
            mv ${D}${prefix}/etc ${D}${sysconfdir}
        fi
    fi
}

FILES:${PN} += " \
    ${sysconfdir}/jupyter \
    ${datadir}/jupyter \
    ${datadir}/icons \
"

BBCLASSEXTEND = "native"
