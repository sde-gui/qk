[ -z "$ACLOCAL" ] && ACLOCAL=aclocal
[ -z "$AUTOCONF" ] && AUTOCONF=autoconf
[ -z "$AUTOHEADER" ] && AUTOHEADER=autoheader
[ -z "$AUTOMAKE" ] && AUTOMAKE=automake

workingdir=`pwd`
rel_srcdir=`dirname "$0"`
srcdir=`cd "$rel_srcdir" && pwd`

cd "$srcdir"

run_cmd() {
    echo "$@"
    "$@" || exit $!
}

run_cmd libtoolize --copy --force

run_cmd $ACLOCAL --force -I m4 $ACLOCAL_FLAGS
run_cmd $AUTOCONF --force
run_cmd $AUTOHEADER --force
run_cmd $AUTOMAKE --add-missing --copy --force-missing

cd $workingdir

run_configure=true
configure_args="--enable-dev-mode --enable-silent-rules"
if [ "$1" ]; then
  :
else
  echo "Done. Run '$rel_srcdir/configure --enable-dev-mode' to configure and then 'make' to build"
  run_configure=false
fi

if $run_configure; then
  run_cmd $rel_srcdir/configure $configure_args "$@"
fi
