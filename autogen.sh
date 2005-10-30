#! /bin/sh

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

echo "Generating configuration files..."

if test x$AUTOMAKE_VERSION != x; then
    am_version=-$AUTOMAKE_VERSION
fi
if test x$AUTOCONF_VERSION != x; then
    ac_version=-$AUTOCONF_VERSION
fi
if test x$LIBTOOL_VERSION != x; then
    lt_version=-$LIBTOOL_VERSION
fi

if test x$ACLOCAL = x; then
    export ACLOCAL=aclocal$am_version
fi
if test x$AUTOMAKE = x; then
    export AUTOMAKE=automake$am_version
fi

if test x$AUTOCONF = x; then
    export AUTOCONF=autoconf$ac_version
fi
if test x$AUTOHEADER = x; then
    export AUTOHEADER=autoheader$ac_version
fi
if test x$AUTOM4TE = x; then
    export AUTOM4TE=autom4te$ac_version
fi

if test x$LIBTOOL = x; then
    export LIBTOOL=libtool$lt_version
fi
if test x$LIBTOOLIZE = x; then
    export LIBTOOLIZE=libtoolize$lt_version
fi

if test -d $srcdir/m4 ; then
    aclocal_extra="-I $srcdir/m4"
fi

echo $LIBTOOLIZE --automake --copy      && \
$LIBTOOLIZE --automake --copy           && \
                                           \
echo $ACLOCAL $aclocal_extra            && \
$ACLOCAL $aclocal_extra                 && \
                                           \
echo $AUTOHEADER                        && \
$AUTOHEADER                             && \
                                           \
echo $AUTOMAKE --add-missing --copy     && \
$AUTOMAKE --add-missing --copy          && \
                                           \
echo $AUTOCONF                          && \
$AUTOCONF                               && \
                                           \
echo && echo "run './configure ; make ; make install'" && echo
