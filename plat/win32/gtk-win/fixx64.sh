#! /bin/bash

for f in `find . '(' -iname "*.[ch]" -o -iname "*.h.in" -o -iname "*.cpp" -o -iname "*.hpp" ')'`
do
  echo $f
  sed -i -r \
    -e 's/`.*/__escaped1__&__escaped1__/g' \
    -e 's|/\*.*|__escaped5__&__escaped5__|g' \
    -e 's/__escaped[0-9]__(.*)long\s+long(.*)__escaped[0-9]__/__escaped2__\1__LiteralLongLong__\2__escaped2__/g' \
    -e 's/__escaped[0-9]__(.*)long(.*)__escaped[0-9]__/__escaped2__\1__LiteralLong__\2__escaped2__/g' \
    -e 's/__escaped[0-9]__(.*)long(.*)__escaped[0-9]__/__escaped2__\1__LiteralLong__\2__escaped2__/g' \
    -e 's/((arbitrarily|too|the|how|few|potentially|to|very)\s+)long/__escaped3__\1__LiteralLong____escaped3__/g' \
    -e 's/long(-|\s+(as|double|option))/__escaped3____LiteralLong__\1__escaped3__/g' \
    -e 's/([^[:alnum:]_]|^)(long\s+long)([^[:alnum:]_]|$)/\1long\3/g' \
    -e 's/([^[:alnum:]_]|^)(long\s+int)([^[:alnum:]_]|$)/\1long\3/g' \
    -e 's/([^[:alnum:]_]|^)(unsigned\s+long)([^[:alnum:]_]|$)/\1__TypeUnsignedLongLong__\3/g' \
    -e 's/([^[:alnum:]_]|^)long([^[:alnum:]_]|$)/\1__TypeLongLong__\2/g' \
    -e 's/__LiteralLong__/long/g' \
    -e 's/__LiteralLongLong__/long long/g' \
    -e 's/__TypeLongLong__/long long/g' \
    -e 's/__TypeUnsignedLongLong__/unsigned long long/g' \
    -e 's/__escaped[0-9]__(.*)__escaped[0-9]__/\1/g' \
    -e 's/__escaped[0-9]__(.*)__escaped[0-9]__/\1/g' \
    -e 's/volatile long long started/volatile long started/g' \
    $f
    #
done
