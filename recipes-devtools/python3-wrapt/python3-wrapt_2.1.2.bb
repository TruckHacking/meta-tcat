SUMMARY = "A Python module for decorators, wrappers and monkey patching"
DESCRIPTION = "The aim of the wrapt module is to provide a transparent object proxy for Python, which can be used as a basis for the construction of function wrappers and decorator functions."
HOMEPAGE = "https://github.com/GrahamDumpleton/wrapt"
LICENSE = "BSD-2-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=63a78af2900bfcc5ce482f3b8d445898"

SRC_URI = "https://pypi.io/packages/source/w/wrapt/wrapt-${PV}.tar.gz"
SRC_URI[sha256sum] = "3996a67eecc2c68fd47b4e3c564405a5777367adfd9b8abb58387b63ee83b21e"

S = "${WORKDIR}/wrapt-${PV}"
inherit python_setuptools_build_meta
RDEPENDS:${PN} = ""

do_compile:prepend() {
    sed -i -e 's/license = "BSD-2-Clause"/license = { text = "BSD-2-Clause" }/g' ${S}/pyproject.toml || true
    sed -i -e '/license-files/d' ${S}/pyproject.toml || true
}