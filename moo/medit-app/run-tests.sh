#! /bin/sh

[ -z "$1" ] && echo "Usage: run-tests.sh <medit-binary> [args...]" && exit 1

medit=$1
shift

echo $medit --run-unit-tests "$@"
exec $medit --run-unit-tests "$@"
