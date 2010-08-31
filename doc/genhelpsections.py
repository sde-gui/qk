# genhelpsections.py help.html

import sys
import re

print '#ifndef HELP_SECTIONS_H'
print '#define HELP_SECTIONS_H'
print ''

for line in open(sys.argv[1]):
    m = re.search(r"##(.+)##", line)
    if m:
        section = m.group(1)
        if section.startswith("fake-"):
            section = section[5:]
        section_cap = section.replace('-', '_').upper()
        print '#define HELP_SECTION_%s "%s"' % (section_cap, section)

print ''
print '#endif /* HELP_SECTIONS_H */'
