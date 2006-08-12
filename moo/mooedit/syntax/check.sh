#!/bin/sh

langs="c.lang desktop.lang diff.lang latex.lang \
perl.lang misc.lang make.lang ms.lang python.lang \
po.lang python-console.lang sh.lang xml.lang \
sci.lang gap.lang scheme.lang texinfo.lang \
changelog.lang diff.lang gnuplot.lang"

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
