#! /bin/sh

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

echo "Generating configuration files..."

echo "Adding libtools..."
libtoolize --automake --copy

echo "Building macros..."
aclocal

echo "Building headers..."
autoheader

echo "Building makefiles..."
automake --add-missing --copy

echo "Building configure..."
autoconf

echo
echo 'run "./configure ; make ; make install"'
echo
