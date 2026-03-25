FILESEXTRAPATHS:prepend := "${THISDIR}:"

SRC_URI += "file://0002-fix-traitlets-warn-stacklevel.patch"

RDEPENDS:${PN} += "python3-jupyter-server"
