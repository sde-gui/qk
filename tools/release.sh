#! /bin/sh

tmpdir=`mktemp --directory --tmpdir medit-release.XXXXXX`
mkdir $tmpdir/files

if [ ! -d $1 ]; then
  echo "Usage: $0 <sourcedir>"
  exit 1
fi

srcdir=`dirname $1`
srcdir=`cd $srcdir && pwd`

if [ "`cd $srcdir && hg st`" ]; then
  echo "uncommitted changes, aborting"
  exit 1
fi

do_or_die() {
  echo "$@"
  "$@" || exit 1
}

set_title() {
  echo -n "\033]0;$1\007"
}

prepare() {
  set_title "medit-release prepare"
  do_or_die rm -fr $tmpdir/medit
  do_or_die cd $tmpdir
  do_or_die hg clone $srcdir medit
  do_or_die cd medit
  set_title "medit-release autogen.sh"
  do_or_die ./autogen.sh
}

check_unix() {
  set_title "medit-release unix build"
  do_or_die mkdir $tmpdir/build-unix
  do_or_die cd $tmpdir/build-unix
  do_or_die $tmpdir/medit/configure --enable-dev-mode --enable-silent-rules
  do_or_die make
  set_title "medit-release unix fullcheck"
  do_or_die make fullcheck
  do_or_die mv medit-*.tar.bz2 $tmpdir/files/
}

check_windows() {
  set_title "medit-release windows build"
  do_or_die mkdir $tmpdir/build-windows
  do_or_die cd $tmpdir/build-windows
  do_or_die $tmpdir/medit/plat/win32/mingw-configure
  do_or_die make
  set_title "medit-release windows installer"
  do_or_die make installer
  do_or_die mv medit-*.exe $tmpdir/files/
}

prepare
check_unix
check_windows

echo "============================================================="
echo " Done."
echo "============================================================="
