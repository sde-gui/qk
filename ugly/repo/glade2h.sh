#! /bin/sh

xml2h="`dirname $0`/xml2h.sh"

base=`basename "$1" .glade | sed -e "s/-/_/g"`
BASE=`echo $base | tr '[a-z]' '[A-Z]'`
echo "#ifndef ${BASE}_GLADE_H"
echo "#define ${BASE}_GLADE_H"
"$xml2h" "${base}_glade_xml" "$1" || exit 0
echo "#endif /* ${BASE}_GLADE_H */"
