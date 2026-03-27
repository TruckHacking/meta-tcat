inherit pypi python_hatchling

SUMMARY = "The backend—i.e. core services, APIs, and REST endpoints—to Jupyter web applications."
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=083556a9912a35360dae8281fb57e886"

SRC_URI[sha256sum] = "659154cea512083434fd7c93b7fe0897af7a2fd0b9dd4749282b42eaac4ae677"

PYPI_PACKAGE = "jupyter_server"

do_configure:prepend() {
    sed -i '/\[tool.hatch.build.hooks.jupyter-builder\]/,/optional-editable-build = true/d' ${S}/pyproject.toml
    # Fix race condition when websocket is closed before handshake finishes
    sed -i 's/preferred_protocol = self.connection.kernel_ws_protocol/preferred_protocol = getattr(self.connection, "kernel_ws_protocol", None)/g' ${S}/jupyter_server/services/kernels/websocket.py
    # Fix noisy 404 tracebacks from API errors polling non-existent kernels
    sed -i 's/reply\["message"\], exc_info=True/reply\["message"\], exc_info=not (exc_info and isinstance(exc_info[1], HTTPError))/g' ${S}/jupyter_server/base/handlers.py
}

DEPENDS += " \
    ${PYTHON_PN}-hatch-jupyter-builder-native \
"

RDEPENDS:${PN} += " \
    ${PYTHON_PN}-anyio \
    ${PYTHON_PN}-argon2-cffi \
    ${PYTHON_PN}-jinja2 \
    ${PYTHON_PN}-jupyter-client \
    ${PYTHON_PN}-jupyter-core \
    ${PYTHON_PN}-jupyter-server-terminals \
    ${PYTHON_PN}-nbconvert \
    ${PYTHON_PN}-nbformat \
    ${PYTHON_PN}-packaging \
    ${PYTHON_PN}-prometheus-client \
    ${PYTHON_PN}-pyzmq \
    ${PYTHON_PN}-send2trash \
    ${PYTHON_PN}-terminado \
    ${PYTHON_PN}-tornado \
    ${PYTHON_PN}-traitlets \
    ${PYTHON_PN}-websocket-client \
    ${PYTHON_PN}-jupyter-events \
    ${PYTHON_PN}-overrides \
"

do_install:append() {
    # this files will be installed by ${PYTHON_PN}-notebook
    rm -f ${D}${bindir}/jupyter-bundlerextension

    if [ -d ${D}${prefix}/etc ]; then
        if [ -d ${D}${sysconfdir} ]; then
            cp -r ${D}${prefix}/etc/* ${D}${sysconfdir}/
            rm -rf ${D}${prefix}/etc
        else
            mv ${D}${prefix}/etc ${D}${sysconfdir}
        fi
    fi
}

FILES:${PN} += "${sysconfdir}/jupyter"

BBCLASSEXTEND = "native"
