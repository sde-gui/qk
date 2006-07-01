#!/usr/bin/env python
import sys

if not sys.argv[2:]:
    print 'usage: %s <var_name> <file_name>' % (sys.argv[0],)
    sys.exit(1)

if sys.argv[1] == '--txt':
    line_term = '\\n'
    VARNAME, INPUT = sys.argv[2:]
else:
    line_term = ''
    VARNAME, INPUT = sys.argv[1:]

file = open(INPUT, "r")

sys.stdout.write('static const char %s[] = \"\"\n' % (VARNAME,))

for line in file:
    line = line[:-1]
    line = '"' + line.replace('"', '\\"') + line_term + '"\n'
    sys.stdout.write(line)

sys.stdout.write(';\n')
