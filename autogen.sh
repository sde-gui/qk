#! /bin/sh

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

echo "Generating configuration files..."

if test -d $srcdir/m4 ; then
    aclocal_extra="-I $srcdir/m4"
fi

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

echo $AUTOMAKE --add-missing --copy
$AUTOMAKE --add-missing --copy || exit $?

echo $AUTOCONF
$AUTOCONF || exit $?

echo
echo "run './configure ; make ; make install'"
echo
