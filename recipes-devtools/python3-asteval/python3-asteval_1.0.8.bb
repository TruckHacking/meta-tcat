SUMMARY = "Safe, minimalistic evaluator of Python expressions"
DESCRIPTION = "asteval is a safe(ish) evaluator of Python expressions and statements using Python's ast module."
HOMEPAGE = "https://github.com/lmfit/asteval"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b601b0049906fef95afe91f690a555af"
SRC_URI = "https://pypi.io/packages/source/a/asteval/asteval-${PV}.tar.gz"
SRC_URI[sha256sum] = "7175134331726df0e1569f4ab5fa59266192cf1b365db0ff463c978842075cbb"
S = "${WORKDIR}/asteval-${PV}"
inherit pypi python_setuptools_build_meta
DEPENDS += "python3-setuptools-scm-native"
do_compile:prepend() {
    sed -i -e 's/license = "MIT"/license = { text = "MIT" }/g' ${S}/pyproject.toml || true
    sed -i -e '/license-files/d' ${S}/pyproject.toml || true
    sed -i -e 's/setuptools_scm *>= *[0-9.]*/setuptools_scm/g' ${S}/pyproject.toml || true
}
RDEPENDS:${PN} = ""
