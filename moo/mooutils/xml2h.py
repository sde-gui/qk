import sys

if not sys.argv[2:]:
    print 'usage: "%s <var_name> <file_name>"' % (sys.argv[0],)
    sys.exit(1)

VARNAME = sys.argv[1]
INPUT = sys.argv[2]

file = open(INPUT, "r")

sys.stdout.write('static const char %s[] = \"\"\n' % (VARNAME,))

for line in file:
    line = line[:-1]
    line = '"' + line.replace('"', '\\"') + '"\n'
    sys.stdout.write(line)

sys.stdout.write(';\n')
