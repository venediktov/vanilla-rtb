# Maintainer: Travis Gockel <travis@gockelhut.com>
# mkaurball
pkgname=json-voorhees
pkgver=1.0.0
pkgrel=1
epoch=
pkgdesc="A modern JSON parsing library for C++"
arch=('x86_64')
url="https://github.com/tgockel/json-voorhees"
license=('Apache')
groups=()
depends=()
makedepends=('gcc>=4.9' 'boost>=1.52 cmake>=2.8')
checkdepends=('boost-libs>=1.48')
optdepends=()
provides=()
conflicts=()
replaces=()
backup=()
options=()
install=
changelog=
source=("https://github.com/tgockel/${pkgname}/archive/v${pkgver}.tar.gz")
noextract=()
md5sums=('b92c91e833c19bbd4e36ec759bb600b1') # makepkg -g 

build() {
	cd "${srcdir}/${pkgname}-${pkgver}"
	cmake . -DCMAKE_BUILD_TYPE=Release
	make
}

check() {
	cd "${srcdir}/${pkgname}-${pkgver}"
	./jsonv-tests
}

package() {
	cd "${srcdir}/${pkgname}-${pkgver}"
	make DESTDIR="${pkgdir}/" install
}
