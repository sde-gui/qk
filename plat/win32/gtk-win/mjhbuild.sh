#! /bin/sh

# Usage: mjhbuild.sh [--debug|--release] <jhbuild arguments...>

if [ -z "$mgwconfig" ]; then
  export mgwdir=`dirname $0`
  . $mgwdir/config.sh || exit 1
fi

export _GTKWINDIR=`cd $mgwdir && pwd`
export _GTKWINCONFIG=$mgwconfig
export _GTKWINBUILDROOT=$mgwbuildroot

# JHBUILD=$mgwjhbuilddir/bin/jhbuild
JHBUILD=jhbuild

chmod a-w $mgwdir/jhbuildrc/glib-win32.cache

echo $JHBUILD -f $mgwdir_s/jhbuildrc/jhbuildrc "$@"
exec $JHBUILD -f $mgwdir_s/jhbuildrc/jhbuildrc "$@"
