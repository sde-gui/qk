#! /bin/sh

builddir=`pwd`

rel_srcdir=`dirname $0`
test -z "$rel_srcdir" && rel_srcdir=.

srcdir=`cd $rel_srcdir && pwd`
echo "srcdir="$srcdir

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

echo $LIBTOOLIZE --automake --copy --force
$LIBTOOLIZE --automake --copy --force || exit $?

echo glib-gettextize --force --copy
glib-gettextize --force --copy || exit $?
echo intltoolize --automake --force --copy
intltoolize --automake --force --copy || exit $?
echo sed 's/@GETTEXT_PACKAGE@/@GETTEXT_PACKAGE@-gsv/' po/Makefile.in.in '>' po-gsv/Makefile.in.in
sed 's/@GETTEXT_PACKAGE@/@GETTEXT_PACKAGE@-gsv/' po/Makefile.in.in > po-gsv/Makefile.in.in || exit $?

echo $ACLOCAL $ACLOCAL_FLAGS
$ACLOCAL $ACLOCAL_FLAGS || exit $?

echo $AUTOHEADER
$AUTOHEADER || exit $?

echo $AUTOMAKE --add-missing --foreign --copy
$AUTOMAKE --add-missing --foreign --copy || exit $?

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
