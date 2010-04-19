#! /bin/sh

export mgwdir=`dirname $0`
. $mgwdir/config.sh || exit 1

export _GTKWINDIR=`cd $mgwdir && pwd`
export _GTKWINCONFIG=$mgwconfig

echo $mgwdir_s/jhbuild/bin/jhbuild -f $mgwdir_s/jhbuildrc/jhbuildrc "$@"
exec $mgwdir_s/jhbuild/bin/jhbuild -f $mgwdir_s/jhbuildrc/jhbuildrc "$@"
