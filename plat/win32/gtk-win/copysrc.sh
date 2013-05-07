#! /bin/sh

export mgwdir=`dirname $0`
. $mgwdir/config.sh || exit 1

jhtarballdir=$mgwbuildroot/tarballs
jhsourcedir=$mgwbuilddir/source
destname=medit-deps-src-`date +%Y%m%d`
destdir=$mgwbuildroot/$destname

do_or_die() {
  echo "$@"
  "$@" || exit 1
}

cd $mgwbuildroot

do_or_die rm -fr $destdir $destdir.tar.bz2
do_or_die mkdir $destdir
do_or_die cp -lfR $jhtarballdir/* $destdir/

do_or_die mkdir $destdir/tmp
for proj in pycairo pygobject pygtk; do
    cd $destdir/tmp || exit 1
    hg clone $jhsourcedir/$proj
    tar cjf medit-$proj.tar.bz2 $proj
    mv medit-$proj.tar.bz2 ..
done
do_or_die cd $destdir
do_or_die rm -fr $destdir/tmp
do_or_die cp -lfR $mgwdir $destdir/gtk-win
do_or_die mv $destdir/gtk-win/extra/readme-medit-deps.txt $destdir/readme.txt

do_or_die cd $destdir/..
do_or_die tar cjf $destname.tar.bz2 $destname
do_or_die rm -fr $destname
