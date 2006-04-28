#! /bin/sh

builddir=`pwd`

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

if test -d $srcdir/m4 ; then
    m4dir=`cd $srcdir/m4 && pwd`
    aclocal_extra="-I $m4dir"
fi

cd $srcdir
echo "Generating configuration files..."

ACLOCAL=${ACLOCAL:-aclocal-1.9}
AUTOMAKE=${AUTOMAKE:-automake-1.9}

LIBTOOLIZE=${LIBTOOLIZE:-libtoolize}

AUTOHEADER=${AUTOHEADER:-autoheader}
AUTOCONF=${AUTOCONF:-autoconf}

echo $LIBTOOLIZE --automake --copy
$LIBTOOLIZE --automake --copy || exit $?

echo $ACLOCAL $ACLOCAL_FLAGS $aclocal_extra
$ACLOCAL $ACLOCAL_FLAGS $aclocal_extra || exit $?

echo $AUTOHEADER
$AUTOHEADER || exit $?

echo $AUTOMAKE --add-missing --copy --foreign
$AUTOMAKE --add-missing --copy --foreign || exit $?

echo $AUTOCONF
$AUTOCONF || exit $?

if test -z $1; then
    echo
    echo "run '$srcdir/configure ; make ; make install'"
    echo
else
    if test $1 = "do" -o $1 = "--do"; then
        shift
    fi
    cd $builddir
    $srcdir/configure $*
fi
