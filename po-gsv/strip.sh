#! /bin/sh

rm -fr fake_input po-stripped
mkdir fake_input po-stripped

input_files=`cat POTFILES.in`

mv POTFILES.in POTFILES.in.orig

for file in $input_files; do
  if (echo $file | grep .lang$ > /dev/null); then
    sed -r -e 's/<((style|string|(line|block)-comment|keyword-list|pattern-item) .*)_name\s*=\s*"/<\1name="/' ../$file > fake_input/`basename $file`
  else
    cp ../$file fake_input/
  fi
  echo po-gsv/fake_input/`basename $file` >> POTFILES.in
done

./pot

sed -r 's/^#[:,.].*//' medit-1-gsv.pot | sed '$!N; /^\(.*\)\n\1$/!P; D' > medit-1-gsv.pot.tmp
mv medit-1-gsv.pot.tmp medit-1-gsv.pot

for lang in `sed 's/#.*//' LINGUAS`; do
  cp $lang.po $lang.po.bak
  ./dist $lang
  sed '/#, fuzzy/,+3 d' $lang.po | sed '/#~ /,+2 d' > $lang.po.tmp
  python msgs.py $lang.po.tmp > po-stripped/$lang.po
  rm $lang.po.tmp
  mv $lang.po.bak $lang.po
done

mv POTFILES.in.orig POTFILES.in

rm -fr fake_input
