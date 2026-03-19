# TODO: update this with TCP enabled truckdevil
DESCRIPTION = "Useful in interacting with trucks that use J1939"
SECTION = "devel/python"
LICENSE = "GPL-3.0"
LIC_FILES_CHKSUM = "file://${WORKDIR}/git/LICENSE.md;md5=1ebbd3e34237af26da5dc08a4e440464"

# using the TCP version of TruckDevil
SRC_URI = "git://github.com/BenGardiner/TruckDevil.git;protocol=https;branch=pretty,tab-completion;name=pretty,tab_completion \
           file://truckdevil"

SRCREV_pretty = "47df0c16b3b70e098f39133bc84f6304acd1ff05"
SRCREV_tab_completion = "8c5e5a321e0b80b8099daa495b03fad4229e0fae"
SRCREV_FORMAT = "pretty_tab_completion"
SRCREV = "${SRCREV_pretty}"

S = "${WORKDIR}/git"

do_merge_branches() {
    cd ${S}
    git config user.email "yocto@example.com"
    git config user.name "Yocto Builder"
    git merge ${SRCREV_tab_completion} -m "Merge tab-completion into pretty"
}
addtask merge_branches after do_patch before do_configure

do_install() {
    install -d ${D}/opt/tcat/programs
    install -d ${D}/usr/bin

    cp -r ${S}/truckdevil ${D}/opt/tcat/programs

    # Make the script executable after we copied everything over
    chmod +x ${D}/opt/tcat/programs/truckdevil/truckdevil.py

    # Add the truckdevil wrapper
    install -m 0755 ${WORKDIR}/truckdevil ${D}/usr/bin/truckdevil
}

# Quickly edit the shebang line
do_compile() {
    sed -i '1i #!/usr/bin/python3' ${S}/truckdevil/truckdevil.py
}

RDEPENDS:${PN} += "python3-core python3 bash python3-can python3-dill python3-pyserial python3-setuptools"
FILES:${PN} += "/opt/tcat/programs/truckdevil \
                /usr/bin/truckdevil \
                "
