#! /bin/sh

builddir=`pwd`

rel_srcdir=`dirname $0`
test -z "$rel_srcdir" && rel_srcdir=.

srcdir=`cd $rel_srcdir && pwd`
echo "srcdir=" $srcdir

if test -d $rel_srcdir/m4 ; then
  aclocal_extra="-I m4"
elif test -d $rel_srcdir/moo/m4 ; then
  aclocal_extra="-I moo/m4"
fi

cd $srcdir
echo "Generating configuration files..."

ACLOCAL=${ACLOCAL:-aclocal-1.9}
ACLOCAL_FLAGS="$ACLOCAL_FLAGS $aclocal_extra"
AUTOMAKE=${AUTOMAKE:-automake-1.9}

LIBTOOLIZE=${LIBTOOLIZE:-libtoolize}

AUTOHEADER=${AUTOHEADER:-autoheader}
AUTOCONF=${AUTOCONF:-autoconf}

echo $LIBTOOLIZE --automake --copy
$LIBTOOLIZE --automake --copy || exit $?

echo glib-gettextize --force
glib-gettextize --force
echo intltoolize --automake --force
intltoolize --automake --force

echo $ACLOCAL $ACLOCAL_FLAGS
$ACLOCAL $ACLOCAL_FLAGS || exit $?

echo $AUTOHEADER
$AUTOHEADER || exit $?

echo $AUTOMAKE --add-missing --copy --foreign
$AUTOMAKE --add-missing --copy --foreign || exit $?

echo $AUTOCONF
$AUTOCONF || exit $?

if test -z $1; then
    echo
    echo "run '$rel_srcdir/configure ; make ; make install'"
    echo
else
    if test $1 = "do" -o $1 = "--do"; then
        shift
    fi
    cd $builddir
    $srcdir/configure $*
fi
