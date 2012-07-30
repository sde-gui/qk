#! /bin/sh

export mgwdir=`dirname $0`
. $mgwdir/config.sh || exit 1

jhtargetdir=$mgwbuilddir/target
destdir=$mgwbuildroot/dist-$mgwconfig
tmpdir=$destdir/medit-deps-bin-tmp
suffix=`date +%Y%m%d`
tarball=medit-deps-bin-$mgwconfig-$suffix.zip

do_or_die() {
  echo "$@"
  "$@" || exit 1
}

do_or_die rm -fr $tmpdir $destdir/$tarball
do_or_die mkdir -p $tmpdir/gtk-win-build/$mgwconfig
do_or_die cp -lfR $jhtargetdir $tmpdir/gtk-win-build/$mgwconfig/
do_or_die cd $tmpdir
do_or_die zip -r9 $tarball gtk-win-build
do_or_die mv $tarball $destdir/
do_or_die cd $destdir
do_or_die rm -fr $tmpdir
