SUMMARY = "Render rich text, tables, progress bars, syntax highlighting, markdown and more to the terminal"
HOMEPAGE = "https://github.com/Textualize/rich"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b1961a04351da6032ef695196328766c"

SRC_URI[sha256sum] = "4ed991305220a29ad3139f1b302c7798b5ccafd7ad81c20573e895dad66797ca"

inherit pypi setuptools3

RDEPENDS:${PN} += " \
    python3-pygments \
    python3-markdown-it-py \
"
