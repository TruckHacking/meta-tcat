SUMMARY = "MessagePack (de)serializer"
DESCRIPTION = "MessagePack is an efficient binary serialization format. It lets you exchange data among multiple languages like JSON. But it's faster and smaller."
HOMEPAGE = "https://github.com/msgpack/msgpack-python"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://COPYING;md5=cd9523181d9d4fbf7ffca52eaa2a5751"

SRC_URI = "https://pypi.io/packages/source/m/msgpack/msgpack-${PV}.tar.gz"
SRC_URI[sha256sum] = "3b60763c1373dd60f398488069bcdc703cd08a711477b5d480eecc9f9626f47e"

S = "${WORKDIR}/msgpack-${PV}"
inherit python_setuptools_build_meta
RDEPENDS:${PN} = ""

do_compile:prepend() {
    sed -i -e 's/license = "Apache-2.0"/license = { text = "Apache-2.0" }/g' ${S}/pyproject.toml || true
    sed -i -e 's/setuptools *>= *[0-9.]*/setuptools/g' ${S}/pyproject.toml || true
}