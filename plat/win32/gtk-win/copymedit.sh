#! /bin/sh

export mgwdir=`dirname $0`
. $mgwdir/config.sh || exit 1

usage() {
  echo "Usage: copymedit.sh [--en] src dest"
  exit 1
}

en_only=false

for arg; do
  case "$arg" in
    -h|--help)
      usage
      ;;
    --en)
      en_only=true
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

bin_files='
medit.exe
'

lib_files='
medit-1
'

share_files='
doc/medit-1
medit-1
'

lc_modules='
medit-1
medit-1-gsv
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
    if [ -d $f ]; then
      cp -R $f "$dstsubdir/" || exit 1
    else
      cp -l $f "$dstsubdir/" || exit 1
    fi
  done
}

copy_files() {
  copy_files_from_dir bin $bin_files
  #copy_files_from_dir lib $lib_files
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
	  cp -l $mo "$dstdir/share/locale/$mo" || exit 1
	fi
      done
    fi
  done
}

copy_files
if ! $en_only; then
  copy_locale
fi

# -%- indent-width:2 -%-
