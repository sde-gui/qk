if [ x$2 = x ]; then
    echo usage: "$0 <var_name> <file_name>"
    exit 1
fi

echo "static const char *$1 = \"\""
sed 's/\\/\\\\/g' | sed 's/"/\\"/g' $2 | sed 's/^\(.*\)$/"\1\\n"/'
echo ";"
