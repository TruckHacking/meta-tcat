FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"
SRC_URI += "file://0001-NOROOTPASS.patch"

do_install:append() {
    # Ensure sshd listens on all addresses
    sed -i 's/^#\?ListenAddress 0.0.0.0/ListenAddress 0.0.0.0/' ${D}${sysconfdir}/ssh/sshd_config
    sed -i 's/^#\?ListenAddress ::/ListenAddress ::/' ${D}${sysconfdir}/ssh/sshd_config
    sed -i 's/^#\?AddressFamily .*/AddressFamily any/' ${D}${sysconfdir}/ssh/sshd_config
}
