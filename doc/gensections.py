import os
import sys
import re

re_section = re.compile('<!-- moo-help-section: (\S+) -->')
sections = [
    [ 'PREFS_ACCELS', 'index.html' ],
    [ 'PREFS_DIALOG', 'index.html' ],
    [ 'PREFS_PLUGINS', 'index.html' ],
    [ 'DIALOG_REPLACE', 'index.html' ],
    [ 'DIALOG_FIND', 'index.html' ],
    [ 'PREFS_FILE_SELECTOR', 'index.html' ],
    [ 'FILE_SELECTOR', 'index.html' ],
    [ 'DIALOG_FIND_FILE', 'index.html' ],
    [ 'DIALOG_FIND_IN_FILES', 'index.html' ],
]

for f in sys.argv[1:]:
    if not os.path.basename(f) in ['help.html', 'script-lua.html', 'script-lua-gtk.html', 'script-python.html']:
        for line in open(f):
            m = re_section.search(line)
            if m:
                sections.append([m.group(1), f])
                break

print '#ifndef MOO_HELP_SECTIONS_H'
print '#define MOO_HELP_SECTIONS_H'
print ''
for sec, filename in sorted(sections):
    print '#define HELP_SECTION_%s "%s"' % (sec, os.path.basename(filename))
print ''
print '#endif /* MOO_HELP_SECTIONS_H */'
