#! /bin/sh

run_cmd() {
echo "$@"
exec "$@"
}

export G_SLICE=always-malloc
args="--tool=memcheck --log-file=medit-valgrind.log --num-callers=10 --error-limit=no --gen-suppressions=all --leak-check=full --leak-resolution=med --show-reachable=no --freelist-vol=100000000"
# --track-fds=yes

def_supp_file=`dirname $0`/medit.supp
supp_file=/tmp/medit-valgrind.supp

if [ ! -f $supp_file -o $def_supp_file -nt $supp_file ]; then
    cp $def_supp_file $supp_file
    cat /usr/lib/valgrind/default.supp >> $supp_file
fi

[ -f $supp_file ] && args="$args --suppressions=$supp_file"

run_cmd valgrind $args "$@"
