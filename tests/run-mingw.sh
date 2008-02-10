#! /bin/sh

srcdir=/home/muntyan/projects/moo
builddir=/home/muntyan/projects/moo/build/mingw
gtkdir=/usr/local/win/gtk/bin

cp $builddir/moo/.libs/libmoo.dll $gtkdir || exit 1
ln -s $srcdir/tests/data $gtkdir/test-data || exit 1

cd $gtkdir

wine $builddir/tests/.libs/run-tests.exe

rm libmoo.dll test-data
