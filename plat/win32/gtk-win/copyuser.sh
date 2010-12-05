#! /bin/sh

export mgwdir=`dirname $0`
. $mgwdir/config.sh || exit 1

usage() {
  echo "Usage: copyuser.sh [--en|--min] src dest"
  exit 1
}

en_only=false
min=false

for arg; do
  case "$arg" in
    -h|--help)
      usage
      ;;
    --en)
      en_only=true
      shift
      ;;
    --min)
      min=true
      shift
      ;;
    -*)
      usage
      ;;
    *)
      break
      ;;
  esac
done

srcdir="$1"
dstdir="$2"

{ [ -n "$srcdir" ] && [ -n "$dstdir" ] ; } || usage
[ -e "$srcdir" ] || { echo "Directory '$srcdir' doesn't exist"; exit 1; }
[ -d "$srcdir" ] || { echo "'$srcdir' is not a directory"; exit 1; }

srcdir=`cd $srcdir && pwd`

mkdir -p "$dstdir" || exit 1
dstdir=`cd $dstdir && pwd`

do_or_die() {
  echo "$@"
  "$@" || exit 1
}

old_grep_bin_files='
grep.exe
intl.dll
'

new_grep_bin_files='
grep.exe
'

bin_files='
gspawn-win32-helper-console.exe
gspawn-win32-helper.exe
libatk-1.0-0.dll
libcairo-2.dll
libgdk_pixbuf-2.0-0.dll
libgdk-win32-2.0-0.dll
libgio-2.0-0.dll
libglib-2.0-0.dll
libgmodule-2.0-0.dll
libgobject-2.0-0.dll
libgthread-2.0-0.dll
libgtk-win32-2.0-0.dll
libintl-8.dll
libpango-1.0-0.dll
libpangocairo-1.0-0.dll
libxml2-2.dll
libpixman-1-0.dll
libpng14-14.dll
zlib1-m314.dll
libgailutil-18.dll
libpangowin32-1.0-0.dll
libiconv-2.dll
'

etc_files='
gtk-2.0/gtkrc
gtk-2.0/im-multipress.conf
'

lib_files='
gtk-2.0/2.10.0/engines/*.dll
gtk-2.0/modules/*.dll
'

share_files='
themes/*/gtk-2.0/gtkrc
themes/*/gtk-2.0-key/gtkrc
locale/locale.alias
'

lc_modules='
atk10
glib20
grep
gtk20
gtk20-properties
'

copy_files_from_dir() {
  subdir=$1
  shift
  cd $srcdir/$subdir || exit 1
  for f in $@; do
    subsubdir=`dirname $f`
    dstsubdir="$dstdir/$subdir/$subsubdir"
    if [ "$subsubdir" = "." ]; then
      dstsubdir="$dstdir/$subdir"
    fi
    if [ ! -d "$dstsubdir" ]; then
      mkdir -p "$dstsubdir" || exit 1
    fi
    echo " -- $dstsubdir/`basename $f`"
    cp -fl $f "$dstsubdir/" || exit 1
  done
}

copy_files() {
  copy_files_from_dir bin $bin_files
  copy_files_from_dir bin $old_grep_bin_files
#   copy_files_from_dir bin $new_grep_bin_files
  copy_files_from_dir etc $etc_files
  copy_files_from_dir lib $lib_files
  copy_files_from_dir share $share_files
}

copy_locale() {
  cd $srcdir/share/locale || exit 1
  for locale in *; do
    if [ -d $locale ]; then
      mkdir -p "$dstdir/share/locale/$locale/LC_MESSAGES" || exit 1
      for module in $lc_modules; do
	mo=$locale/LC_MESSAGES/$module.mo
	if [ -f $mo ]; then
	  echo " -- $dstdir/share/locale/$mo"
	  cp -fl $mo "$dstdir/share/locale/$mo" || exit 1
	fi
      done
    fi
  done
}

copy_icons() {
  do_or_die mkdir -p "$dstdir/share/icons"
  themes="hicolor"
  for theme in $themes; do
    do_or_die cp -flR "$srcdir/share/icons/$theme" "$dstdir/share/icons/"
    do_or_die rm -fr "$dstdir/share/icons/$theme/scalable"
    do_or_die gtk-update-icon-cache "$srcdir/share/icons/$theme"
  done

  (
    cd $dstdir/share/icons/hicolor
    for d in [0-9][0-9]x[0-9][0-9] [0-9][0-9][0-9]x[0-9][0-9][0-9] scalable; do
      do_or_die rm -fr $d
    done
  )
}

copy_mime() {
  do_or_die cp -R /usr/share/mime "$dstdir/share/mime-tmp"
  do_or_die update-mime-database "$dstdir/share/mime-tmp"
  do_or_die mkdir -p "$dstdir/share/mime"
  do_or_die mv "$dstdir/share/mime-tmp/mime.cache" "$dstdir/share/mime/"
  do_or_die rm -fr "$dstdir/share/mime-tmp"
}

copy_python() {
  :
}

copy_files
if ! $en_only; then
  copy_locale
fi
copy_icons
copy_mime

# -%- indent-width:2 -%-
