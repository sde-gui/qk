#!/bin/sh

langs="gap.lang c.lang diff.lang make.lang misc.lang python.lang sh.lang xml.lang"
styles="garnacho.styles gvim.styles"
files="$langs $styles"

if [ $1 ]; then
    files=$*
fi

for file in $files; do
    if ! xmllint --valid --noout $file ; then
	exit 1
    fi
done
