#! /bin/sh

export mgwdir=`dirname $0`
. $mgwdir/config.sh || exit 1

jhsourcedir=$mgwbuilddir/source
jhbuilddir=$mgwbuilddir/build
destdir=medit-deps-dbg-src-$mgwconfig-`date +%Y%m%d`

do_or_die() {
  echo "$@"
  "$@" || exit 1
}

do_or_die mkdir -p $mgwbuildroot/dist-$mgwconfig
cd $mgwbuildroot/dist-$mgwconfig

do_or_die rm -fr $destdir.zip $destdir
do_or_die mkdir -p $destdir
do_or_die cp -lfR $jhsourcedir/* $destdir/
do_or_die cp -lfR $jhbuilddir/* $destdir/

cd $destdir

{ find . -type d \( -name 'tests' \) -print0 | xargs -0 rm -r ; } || exit 1
{ find . -type d \( -name '.libs' -o -name '.deps' \) -print0 | xargs -0 rm -r ; } || exit 1
rm -rf */test */docs */doc */man */demos */examples */po */.hg */.git \
    gettext-*/gettext-tools/examples gettext-*/gettext-tools/man gettext-*/gettext-tools/po \
    gettext-*/gettext-tools/doc gettext-*/gettext-tools/gnulib-tests \
    */autom4te.cache libxml2-*/result || exit 1

find . -type f \( \
    -name '*.o' -o \
    -name '*.lo' -o \
    -name '*.Plo' -o \
    -name '*.Po' -o \
    -name '*.dll' -o \
    -name '*.a' -o \
    -name '*.la' -o \
    -name '*.html' -o \
    -name '*.sgml' -o \
    -name '*.xml' -o \
    -name '*.txt' -o \
    -name '*.po' -o \
    -name '*.gmo' -o \
    -name '*.in' -o \
    -name '*.m4' -o \
    -name '*.pc' -o \
    -name '*.spec' -o \
    -name '*.sh' -o \
    -name '*.guess' -o \
    -name '*.log' -o \
    -name '*.status' -o \
    -name '*.sub' -o \
    -name 'AUTHORS' -o \
    -name 'ChangeLog' -o \
    -name 'ChangeLog.*' -o \
    -name 'configure' -o \
    -name 'COPYING' -o \
    -name 'depcomp' -o \
    -name 'INSTALL' -o \
    -name 'install-sh' -o \
    -name 'libtool' -o \
    -name 'MAINTAINERS' -o \
    -name 'Makefile' -o \
    -name 'Makefile.*' -o \
    -name 'missing' -o \
    -name 'NEWS' -o \
    -name 'mkinstalldirs' -o \
    -name 'README' -o \
    -name 'stamp-h1' -o \
    -name '*.make' -o \
    -name '*.def' -o \
    -name '*.rc' -o \
    -name '*.symbols' -o \
    -name '*.png' -o \
    -name '*.override' -o \
    -name '*.defs' -o \
    -name '*.lai' \
\) -delete || exit 1

cd $mgwbuildroot/dist-$mgwconfig
do_or_die zip -r9 $destdir.zip $destdir
do_or_die rm -rf $destdir
