define=false

while true; do
  case "$1" in
    --define)
      define=true
      shift
      ;;
    *)
      break
      ;;
  esac
done

if [ $# != 2 ]; then
    echo "usage: $0 [--define] <var_name> <file_name>" > /dev/stderr
    exit 1
fi

VARNAME="$1"
INPUT="$2"

echo "/* -*- C -*- */"

if $define; then
  echo "#define $VARNAME \\"
  sed 's/"/\\"/g' "$INPUT" | sed 's/^\(.*\)$/"\1\\n"\\/' || exit $?
  echo "\"\""
else
  echo "static const char $VARNAME [] = \"\""
  sed 's/"/\\"/g' "$INPUT" | sed 's/^\(.*\)$/"\1\\n"/' || exit $?
  echo ";"
fi
