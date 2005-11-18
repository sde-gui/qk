#!/bin/sh

langs="c.lang diff.lang misc.lang make.lang python.lang python-console.lang sh.lang xml.lang sci.lang gap.lang scheme.lang"
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
