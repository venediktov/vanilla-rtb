# Maintainer: Travis Gockel <travis@gockelhut.com>
# mkaurball
_pkgbase=json-voorhees
pkgname=${_pkgbase}-git
pkgver=1.0.0.r7.g1810f53
pkgver() {
	cd "${srcdir}/${_pkgbase}"
	git describe --long | sed -r 's/([^-]*-g)/r\1/;s/-/./g' | cut -c2-
}
pkgrel=1
epoch=
pkgdesc="A modern JSON parsing library for C++"
arch=('x86_64')
url="https://github.com/tgockel/json-voorhees"
license=('Apache')
groups=()
depends=()
makedepends=('gcc>=4.9' 'boost-libs>=1.52 cmake>=2.8')
checkdepends=('boost>=1.48')
optdepends=()
provides=()
conflicts=()
replaces=()
backup=()
options=()
install=
changelog=
source=("${_pkgbase}::git+https://github.com/tgockel/${_pkgbase}.git#branch=v1.0")
noextract=()
md5sums=('SKIP')

build() {
	cd "${srcdir}/${_pkgbase}"
	cmake . -DCMAKE_BUILD_TYPE=Release -DJSONV_VERSION=${pkgver}
	make
}

check() {
	cd "${srcdir}/${_pkgbase}"
	./jsonv-tests
}

package() {
	cd "${srcdir}/${_pkgbase}"
	make install DESTDIR="${pkgdir}/"
}
