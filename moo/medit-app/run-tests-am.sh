#! /bin/sh
echo ./medit --ut --ut-coverage called-functions
./medit --ut --ut-coverage called-functions || exit $?
sort called-functions > called-functions.tmp || exit 1
mv called-functions.tmp called-functions || exit 1

moo_xml=$top_srcdir/api/moo.xml
gtk_xml=$top_srcdir/api/gtk.xml
[ -f $moo_xml ] || { echo "file $moo_xml doesn't exist"; exit 1; }
[ -f $gtk_xml ] || { echo "file $gtk_xml doesn't exist"; exit 1; }

$PYTHON $print_functions $moo_xml $gtk_xml > all-functions.tmp || exit 1
sort all-functions.tmp > all-functions.tmp2 || exit 1
mv all-functions.tmp2 all-functions || exit 1
rm -f all-functions.tmp all-functions.tmp2

comm -3 -2 all-functions called-functions > not-covered-functions

if [ -z "$IGNORE_COVERAGE" ] && [ -s not-covered-functions ]; then
  echo "*** Not all functions are covered, see file not-covered-functions"
  exit 1
else
  rm -f all-functions called-functions not-covered-functions
  exit 0
fi
