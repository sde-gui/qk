#! /bin/sh

if [ -z "$test_release_tmpdir" ]; then
  tmpdir=`mktemp --directory --tmpdir=/var/tmp medit-release.XXXXXX`
  mkdir $tmpdir/files
else
  tmpdir=$test_release_tmpdir
fi

if [ ! -d "$1" ]; then
  echo "Usage: $0 <sourcedir>"
  exit 1
fi

srcdir=`cd $1 && pwd`
shift

if [ "`cd $srcdir && hg st | grep -v test-release.sh`" ]; then
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
  if [ ! -d $tmpdir/medit ]; then
    do_or_die cd $tmpdir
    do_or_die hg clone $srcdir medit
  fi
  if [ ! -e $tmpdir/medit/configure ]; then
    do_or_die cd $tmpdir/medit
    set_title "medit-release autogen.sh"
    do_or_die ./autogen.sh
  fi
}

check_unix() {
  prepare
  set_title "medit-release unix build"
  do_or_die mkdir $tmpdir/build-unix
  do_or_die cd $tmpdir/build-unix
  do_or_die $tmpdir/medit/configure --enable-dev-mode
  do_or_die make -j 3
  set_title "medit-release unix fullcheck"
  do_or_die make fullcheck
#   set_title "medit-release unix user-build"
#   do_or_die tar xjf medit-*.tar.bz2
#   do_or_die cd medit-*
#   do_or_die ./configure
#   do_or_die make -j 3
#   do_or_die make test
#   do_or_die cd ..
  do_or_die mv medit-*.tar.bz2 $tmpdir/files/
}

check_no_python() {
  prepare
  set_title "medit-release unix-no-python build"
  do_or_die mkdir $tmpdir/build-unix-no-python
  do_or_die cd $tmpdir/build-unix-no-python
  do_or_die $tmpdir/medit/configure --enable-dev-mode --without-python
  do_or_die make -j 3
  set_title "medit-release unix-no-python fullcheck"
  do_or_die make test
}

check_python() {
  prepare
  set_title "medit-release unix-python build"
  do_or_die mkdir $tmpdir/build-unix-python
  do_or_die cd $tmpdir/build-unix-python
  do_or_die $tmpdir/medit/configure --enable-dev-mode --enable-moo-module --enable-shared --disable-static
  do_or_die make -j 3
}

check_windows() {
  prepare
  set_title "medit-release windows build"
  do_or_die mkdir $tmpdir/build-windows
  do_or_die cd $tmpdir/build-windows
  do_or_die $tmpdir/medit/plat/win32/mingw-configure
  do_or_die make -j 3
  set_title "medit-release windows installer"
  do_or_die make installer
  do_or_die mv medit-*.exe $tmpdir/files/
}

check_all() {
  all_checks="unix no_python python windows"
  fail=false
  failed_checks=

  export test_release_tmpdir=$tmpdir

  for check in `echo $all_checks`; do
    if ! $0 $srcdir "$check"; then
      failed_checks="$failed_checks $check"
      fail=true
    fi
  done

  if $fail; then
    echo "FAILED"
    for check in `echo $failed_checks`; do
      echo "check_$check - FAIL"
    done
  else
    echo "SUCCESS, files are in $tmpdir/files/"
  fi
}

if [ -n "$1" ]; then
  "check_$1" $srcdir || exit 1
else
  check_all || exit 1
fi
