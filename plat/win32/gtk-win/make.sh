#! /bin/sh

if [ "$1" = "--all" ]; then
  shift
  echo $0 --debug "$@"
  $0 --debug "$@" || exit 1
  echo $0 --release "$@"
  $0 --release "$@" || exit 1
  exit 0
fi

export mgwdir=`dirname $0`
. $mgwdir/config.sh || exit 1

echo make -f $mgwdir_s/Makefile "$@"
exec make -f $mgwdir_s/Makefile "$@"
