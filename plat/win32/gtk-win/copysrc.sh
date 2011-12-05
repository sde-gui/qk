#! /bin/sh

export mgwdir=`dirname $0`
. $mgwdir/config.sh || exit 1

jhtarballdir=$mgwbuildroot/tarballs
jhsourcedir=$mgwbuilddir/source
destdir=$mgwbuildroot/medit-deps-src

do_or_die() {
  echo "$@"
  "$@" || exit 1
}

cd $mgwbuildroot

if [ -e $destdir ]; then
  echo "Directory $destdir already exists"
  exit 1
fi

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
do_or_die rm -r $destdir/tmp
do_or_die cp -lfR $mgwdir $destdir/gtk-win
do_or_die mv $destdir/gtk-win/extra/readme-medit-deps.txt $destdir/readme.txt
do_or_die tar cjf gtk-win.tar.bz2 gtk-win
do_or_die rm -fr gtk-win

# cd $mgwbuildroot
# do_or_die zip -r src-$mgwconfig.zip src-$mgwconfig
