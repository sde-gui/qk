import os
import sys
import re

lines = []
re_doc = re.compile('^\s*///(\s+(.*))?$')

for f in sys.argv[1:]:
    for line in open(f):
        m = re_doc.search(line)
        if m:
            lines.append(m.group(2) or '')

print '\n'.join(lines)
