#! /bin/sh

coverage=false
uninstalled=false

for arg; do
  case $arg in
    --installed)
      uninstalled=false
      ;;
    --uninstalled)
      uninstalled=true
      ;;
    --coverage)
      coverage=true
      ;;
    *)
      echo "Unknown option $arg"
      exit 1
      ;;
  esac
done

if $uninstalled; then
  if ! [ -e ./medit ]; then
    echo "file ./medit doesn't exist"
    exit 1
  fi
  medit_cmd_line="./medit --ut --ut-uninstalled"
else
  if [ -z $bindir ]; then
    medit=`which medit`
  else
    medit=$bindir/medit
  fi
  if ! [ -e $medit ]; then
    echo "file $medit doesn't exist"
    exit 1
  fi
  if [ ./medit -nt $medit ]; then
    echo "file ./medit is newer than '$medit', did you forget run make install?"
    exit 1
  fi
  medit_cmd_line="$medit --ut"
fi

if $coverage; then
  medit_cmd_line="$medit_cmd_line --ut-coverage called-functions"
fi

echo "$medit_cmd_line"
$medit_cmd_line || exit $?

if $coverage; then
  sort called-functions > called-functions.tmp || exit 1
  mv called-functions.tmp called-functions || exit 1

  moo_xml=$top_srcdir/api/moo.xml
  gtk_xml=$top_srcdir/api/gtk.xml
  [ -f $moo_xml ] || { echo "file $moo_xml doesn't exist"; exit 1; }
  [ -f $gtk_xml ] || { echo "file $gtk_xml doesn't exist"; exit 1; }

  $PYTHON $print_functions $moo_xml $gtk_xml > all-functions.tmp || exit 1
  sort all-functions.tmp > all-functions.tmp2 || exit 1
  mv all-functions.tmp2 all-functions || exit 1
  rm -f all-functions.tmp all-functions.tmp2

  comm -3 -2 all-functions called-functions > not-covered-functions

  if [ -z "$IGNORE_COVERAGE" ] && [ -s not-covered-functions ]; then
    echo "*** Not all functions are covered, see file not-covered-functions"
    exit 1
  else
    rm -f all-functions called-functions not-covered-functions
    exit 0
  fi
fi
