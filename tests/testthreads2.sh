#! /bin/sh

rm -f testthreads2-log.txt
echo "Running ./testthreads2"
./testthreads2 || exit 1
[ `wc -l < testthreads2-log.txt` = 3300 ] || exit 1
