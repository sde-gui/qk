#! /bin/sh

builddir=`pwd`

rel_srcdir=`dirname $0`
test -z "$rel_srcdir" && rel_srcdir=.

srcdir=`cd $rel_srcdir && pwd`
echo "srcdir=" $srcdir

if test -d $rel_srcdir/m4 ; then
    m4dir=`cd $rel_srcdir/m4 && pwd`
    aclocal_extra="-I $m4dir"
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
echo intltoolize --force --automake
intltoolize --force --automake

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
