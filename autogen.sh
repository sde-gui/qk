[ -z "$ACLOCAL" ] && ACLOCAL=aclocal
[ -z "$AUTOCONF" ] && AUTOCONF=autoconf
[ -z "$AUTOHEADER" ] && AUTOHEADER=autoheader
[ -z "$AUTOMAKE" ] && AUTOMAKE=automake

workingdir=`pwd`
rel_srcdir=`dirname "$0"`
srcdir=`cd "$rel_srcdir" && pwd`

cd "$srcdir"

# autoreconf --verbose --install --force || exit $!

run_cmd() {
    echo "$@"
    "$@" || exit $!
}

# run_cmd libtoolize --copy --force

run_cmd $ACLOCAL --force -I m4 $ACLOCAL_FLAGS
run_cmd $AUTOCONF --force
run_cmd $AUTOHEADER --force
run_cmd $AUTOMAKE --add-missing --copy --force-missing

if [ "$1" ]; then
  cd $workingdir && run_cmd $rel_srcdir/configure --enable-dev-mode "$@"
else
  echo "Done. Run '$rel_srcdir/configure --enable-dev-mode' to configure"
fi
