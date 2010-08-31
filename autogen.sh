[ -z "$ACLOCAL" ] && ACLOCAL=aclocal
[ -z "$AUTOCONF" ] && AUTOCONF=autoconf
[ -z "$AUTOHEADER" ] && AUTOHEADER=autoheader
[ -z "$AUTOMAKE" ] && AUTOMAKE=automake

workingdir=`pwd`
srcdir=`dirname "$0"`
srcdir=`cd "$srcdir" && pwd`

cd "$srcdir"

# autoreconf --verbose --install --force || exit $!

run_cmd() {
    echo "$@"
    "$@" || exit $!
}

run_cmd libtoolize --copy --force

run_cmd intltoolize --copy --force
echo "sed -e s/@GETTEXT_PACKAGE@/moo-gsv/ po/Makefile.in.in > po-gsv/Makefile.in.in"
sed -e s/@GETTEXT_PACKAGE@/@GETTEXT_PACKAGE@-gsv/ po/Makefile.in.in > po-gsv/Makefile.in.in || exit $!

run_cmd $ACLOCAL --force -I m4 $ACLOCAL_FLAGS
run_cmd $AUTOCONF --force
run_cmd $AUTOHEADER --force
run_cmd $AUTOMAKE --add-missing --copy --force-missing
