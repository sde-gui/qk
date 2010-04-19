#! /bin/sh

export mgwdir=`dirname $0`
. $mgwdir/config.sh || exit 1

do_or_die()
{
  echo "$@"
  "$@" || exit 1
}

if [ -d $mgwdir/jhbuild ]; then
  echo "Directory $mgwdir/jhbuild already exists"
  exit 1
fi

do_or_die git clone git://git.gnome.org/jhbuild
do_or_die cd $mgwdir_s/jhbuild
do_or_die patch -p 1 < ../jhbuild.patch
do_or_die make -f Makefile.plain install bindir=$mgwdir/jhbuild/bin datarootdir=$mgwdir/jhbuild/share DISABLE_GETTEXT=1
