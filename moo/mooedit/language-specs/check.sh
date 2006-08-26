#!/bin/sh

langs="c.lang cpp.lang def.lang html.lang \
       javascript.lang latex.lang xml.lang \
       yacc.lang"

styles="gvim.styles kde.styles"

if [ $1 ]; then
    langs=$*
    styles=
fi

for file in $langs; do
    if ! xmllint --relaxng language2.rng --noout $file ; then
	exit 1
    fi
done

for file in $styles; do
    if ! xmllint --relaxng styles.rng --noout $file ; then
	exit 1
    fi
done
