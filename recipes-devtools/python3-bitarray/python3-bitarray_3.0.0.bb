SUMMARY = "efficient arrays of booleans -- C extension"
DESCRIPTION = "A high-level Python efficient arrays of booleans -- C extension"
HOMEPAGE = "https://github.com/ilanschnell/bitarray"
LICENSE = "PSF-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=550ddd3aeb756937d5d7c4aa364a0ba4"

SRC_URI[sha256sum] = "a2083dc20f0d828a7cdf7a16b20dae56aab0f43dc4f347a3b3039f6577992b03"

inherit setuptools3 pypi

BBCLASSEXTEND = "native nativesdk"