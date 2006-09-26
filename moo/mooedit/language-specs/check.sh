#!/bin/sh
# "./check.sh somelang.lang" will validate somelang.lang file.
# "./check.sh" without arguments will validate lang and styles files
# specified here.

langs="c.lang cpp.lang changelog.lang def.lang \
       html.lang javascript.lang latex.lang \
       m4.lang makefile.lang xml.lang yacc.lang \
       sh.lang python.lang"

styles="gvim.styles kate.styles"

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
