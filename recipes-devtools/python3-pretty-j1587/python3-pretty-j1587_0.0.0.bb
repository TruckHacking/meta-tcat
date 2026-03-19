DESCRIPTION = "This is a tool for getting detailed decodings of J1587/J1708 (and J2497) messages using the J1587 and J1708 specification PDFs as a reference"
SECTION = "devel/python"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${WORKDIR}/git/LICENSE;md5=dff86395128c2d1dffad00b26678dfca"

# Specify the source file location
# Using this specific fork until the changes are merged into the main branch
SRC_URI = "git://github.com/Spenc3rB/pretty_j1587;protocol=https;rev=ebd9c4d2377216919c85c45a3da2d168694c68f5;branch=master"

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "file://config.cfg"

S = "${WORKDIR}/git"

RDEPENDS:${PN} += "python3-core python3-py-hv-networks python3-platformdirs"

inherit python3native
DEPENDS += "python3-native"

do_install() {
    install -d ${D}/opt/tcat/programs/pretty_j1587
    install -d ${D}/usr/bin

    #rm -rf ${S}/fuzz*
    cp -r ${S}/* ${D}/opt/tcat/programs/pretty_j1587
    install -m 0644 ${WORKDIR}/config.cfg ${D}/opt/tcat/programs/pretty_j1587

    ln -s /opt/tcat/programs/pretty_j1587/pretty_j1587.py ${D}/usr/bin/pretty_j1587

    # Rewrite the first line for any .py under our installed directory if it references unversioned 'python'
    for f in ${D}/opt/tcat/programs/pretty_j1587/*.py; do
        # Check the first line for an unversioned python shebang
        if head -n 1 "$f" | grep -qiE '^#!.*\bpython([[:space:]]|$)'; then
	    bbwarn "patching python->python3 in $f"
            # Force Python 3 interpreter path
            sed -i '1s|^#!.*python\([[:space:]].*\)\?$|#!/usr/bin/python3|' "$f"
        fi
    done
}


# Ensure we have a native Python available in the task env
inherit python3native
DEPENDS += "python3-native"

FILES:${PN} += "/opt/tcat/programs/pretty_j1587"

inherit python3native
DEPENDS += "python3-native"

