# Maintainer: ANDRoid7890 <andrey.android7890@gmail.com>

pkgname=bootsplash-manager
pkgver=1.0
pkgrel=1
pkgdesc="Simple cli bootsplash manager"
url="https://github.com/ANDRoid7890/bootsplash-manager"
arch=(x86_64)
license=(GPL2)
depends=('qt5-base'
         'bootsplash-systemd')
makedepends=(cmake)

source=("CMakeLists.txt"
        "main.cpp"
        "org.manjaro.bootsplash-manager.policy")

md5sums=('c15c5c3dbb5d60868db08caaf63ff901'
         '8a8ef82d4ad27e743d38be739724a463'
         'bff11696553f09c850aee775a83ba4d3')

build() {
    cmake .
    make
}

package() {
    install -Dm755 bootsplash-manager "$pkgdir/usr/bin/bootsplash-manager"
    install -Dm644 org.manjaro.bootsplash-manager.policy "$pkgdir/usr/share/polkit-1/actions/org.manjaro.bootsplash-manager.policy"
}
