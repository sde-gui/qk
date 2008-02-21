#! /bin/sh

srcdir=/home/muntyan/projects/moo
builddir=/home/muntyan/projects/moo/build/mingw

GTK_PREFIX=${GTK_PREFIX:-/usr/local/win/gtk}
GTK_BIN_DIR=$GTK_PREFIX/bin

cp $builddir/moo/.libs/libmoo.dll $GTK_BIN_DIR || exit 1
ln -s $srcdir/tests/data $GTK_BIN_DIR/test-data || exit 1

cd $GTK_BIN_DIR

wine $builddir/tests/.libs/run-tests.exe

rm libmoo.dll test-data
