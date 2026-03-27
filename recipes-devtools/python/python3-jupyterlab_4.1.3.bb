inherit pypi python_hatchling

SUMMARY = "JupyterLab computational environment"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=3833c7b0556c7185b65b20af0b237965"

SRC_URI[sha256sum] = "b85bd8766f995d23461e1f68a0cbc688d23e0af2b6f42a7768fc7b1826b2ec39"

PYPI_PACKAGE = "jupyterlab"

do_configure:prepend() {
    sed -i '/\[tool.hatch.build.hooks.jupyter-builder\]/,/install-pre-commit-hook = true/d' ${S}/pyproject.toml
    sed -i '/\[tool.hatch.build.hooks.jupyter-builder.editable-build-kwargs\]/,/npm = \["node", "jupyterlab\/staging\/yarn.js"\]/d' ${S}/pyproject.toml
    sed -i '/\[tool.hatch.build.hooks.jupyter-builder.build-kwargs\]/,/npm = \["node", "yarn.js"\]/d' ${S}/pyproject.toml
}

DEPENDS += " \
    ${PYTHON_PN}-hatch-jupyter-builder-native \
"

RDEPENDS:${PN} += " \
    ${PYTHON_PN}-async-lru \
    ${PYTHON_PN}-httpx \
    ${PYTHON_PN}-ipykernel \
    ${PYTHON_PN}-jinja2 \
    ${PYTHON_PN}-jupyter-core \
    ${PYTHON_PN}-jupyter-server \
    ${PYTHON_PN}-jupyter-lsp \
    ${PYTHON_PN}-jupyterlab-server \
    ${PYTHON_PN}-notebook-shim \
    ${PYTHON_PN}-packaging \
    ${PYTHON_PN}-tornado \
    ${PYTHON_PN}-traitlets \
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
