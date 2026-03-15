SUMMARY = "Markdown parser, done right. 100% CommonMark support, extensions, syntax plugins & high speed"
HOMEPAGE = "https://github.com/executablebooks/markdown-it-py"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=f07f4332910403333333333333333333"

SRC_URI[sha256sum] = "4332910403333333333333333333333333333333333333333333333333333333"

inherit pypi setuptools3

RDEPENDS:${PN} += "python3-mdurl"
