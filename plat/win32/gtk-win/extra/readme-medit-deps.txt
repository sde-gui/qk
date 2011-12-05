These are sources of the binaries built and distributed with medit. Most of the tarballs
(except those named medit-*.tar.bz2) are stock source tarballs distributed by their project
authors/owners. They are collected here to satisfy requirements of GPL and LGPL, and to
ensure that they are present for as long as they are needed to build medit (non-gnome
source tarballs do disappear from their websites sometimes).

Pygtk, pygobject, and pycairo are special: when they are built, sources are downloaded
from Mercurial repositories at https://bitbucket.org/muntyan. Tarballs present here
are clones of corresponding Mercurial reporsitories.

gtk-win.tar.bz2 contains scripts used to build all these projects, see
readme-win32-build.txt inside.

To build medit using these tarballs (as opposed to letting jhbuild go and download
everything from internet), place the stock tarballs into:

<gtk-win-build>/tarballs/

and unpack medit-py*.tar.bz2 tarballs into

<gtk-win-build>/release/source/

Here <gtk-win-build> is the root of medit jhbuild environment, see readme-win32-build.txt.
