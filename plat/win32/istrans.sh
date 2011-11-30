islangsdir="$HOME/.wine/drive_c/Program Files/Inno Setup 5/Languages/"

mkdir /tmp/isfiles || exit 1
cd /tmp/isfiles

wget `echo '
http://www.jrsoftware.org/files/istrans/Basque-1/Basque-1-5.1.11.isl
http://www.jrsoftware.org/files/istrans/Catalan-4/Catalan-4-5.1.11.isl
http://www.jrsoftware.org/files/istrans/Czech-5/Czech-5-5.1.11.isl
http://www.jrsoftware.org/files/istrans/Danish-4/Danish-4-5.1.11.isl
http://www.jrsoftware.org/files/istrans/Dutch-8/Dutch-8-5.1.11.isl
http://www.jrsoftware.org/files/istrans/Finnish/Finnish-5.1.11.isl
http://www.jrsoftware.org/files/istrans/French-15/French-15-5.1.11.isl
http://www.jrsoftware.org/files/istrans/German-2/German-2-5.1.11.isl
http://www.jrsoftware.org/files/istrans/Hebrew-3/Hebrew-3-5.1.11.isl
http://www.jrsoftware.org/files/istrans/Hungarian/Hungarian-5.1.11.isl
http://www.jrsoftware.org/files/istrans/Italian-14/Italian-14-5.1.11.isl
http://www.jrsoftware.org/files/istrans/Japanese-5/Japanese-5-5.1.11.isl
http://www.jrsoftware.org/files/istrans/Norwegian/Norwegian-5.1.11.isl
http://www.jrsoftware.org/files/istrans/Polish-8/Polish-8-5.1.11.isl
http://www.jrsoftware.org/files/istrans/PortugueseBr-16/BrazilianPortuguese-16-5.1.11.isl
http://www.jrsoftware.org/files/istrans/PortugueseStd-1/PortugueseStd-1-5.1.11.isl
http://www.jrsoftware.org/files/istrans/Russian-19/Russian-19-5.1.11.isl
http://www.jrsoftware.org/files/istrans/SerbianCyrillic-2/SerbianCyrillic-2-5.1.11.isl
http://www.jrsoftware.org/files/istrans/Serbian-6/Serbian-6-5.1.11.isl
http://www.jrsoftware.org/files/istrans/Slovak-6/Slovak-6-5.1.11.isl
http://www.jrsoftware.org/files/istrans/Slovenian-3/Slovenian-3-5.1.11.isl
http://www.jrsoftware.org/files/istrans/SpanishStd-5/SpanishStd-5-5.1.11.isl
http://www.jrsoftware.org/files/istrans/Ukrainian-7/Ukrainian-7-5.1.11.isl
http://www.jrsoftware.org/files/istrans/Albanian-2/Albanian-2-5.1.11.isl
http://www.jrsoftware.org/files/istrans/Armenian-1/Armenian-1-5.1.11.islu
'`

for file in *.isl *.islu; do
  mv $file `echo $file | sed -r 's/(\w+)-.*\.(\w+)/\1.\2/'` || exit 1
done

mv *.isl *.islu "$islangsdir" || exit 1
cd /tmp
rmdir isfiles
