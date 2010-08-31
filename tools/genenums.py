# foo/foo-enums-in.py foo/foo-enums stamp

import sys
import filecmp
import shutil
import os
import re

def copy_tmp_if_changed(tmp, dst):
    docopy = False
    try:
        docopy = not filecmp.cmp(tmp, dst)
    except:
        docopy = True

    if docopy:
        shutil.copyfile(tmp, dst)

    try:
        os.remove(tmp)
    except:
        pass

comp_name = sys.argv[1]
input_py = sys.argv[2]
output_h = sys.argv[3] + '.h'
output_c = sys.argv[3] + '.c'
output_h_tmp = output_h + '.tmp'
output_c_tmp = output_c + '.tmp'
stamp = sys.argv[4]

vrs = {}
execfile(input_py, vrs)

def parse_name(Name):
    Pieces = re.findall('[A-Z][a-z]+', Name)
    assert Name == ''.join(Pieces)
    PIECES = map(lambda s: s.upper(), Pieces)
    pieces = map(lambda s: s.lower(), Pieces)
    return { 'Name': Name, 'name': '_'.join(pieces),
             'Prefix': pieces[0], 'prefix': pieces[0].lower(), 'PREFIX': pieces[0].upper(),
             'short': '_'.join(pieces[1:]), 'SHORT': '_'.join(PIECES[1:]), }

def print_enum_h(name, vals, out):
    print >> out, 'typedef enum {'
    for i in range(len(vals)):
        v = vals[i]
        out.write('    %s' % (v[0],))
        if v[1:]:
            out.write(' = %s' % (v[1],))
        if i + 1 < len(vals):
            out.write(',')
        if v[2:]:
            out.write(' /* %s */' % (v[2],))
        out.write('\n')
    print >> out, '} %s;' % name
    print >> out, ''
    dic = parse_name(name)
    print >> out, 'GType %(name)s_get_type (void) G_GNUC_CONST;' % dic
    print >> out, '#define %(PREFIX)s_TYPE_%(SHORT)s (%(name)s_get_type())' % dic

def print_flags_h(name, vals, out):
    print_enum_h(name, vals, out)

## h

out = open(output_h_tmp, 'w')

print >> out, '#ifndef %s_ENUMS_H' % (comp_name.upper(),)
print >> out, '#define %s_ENUMS_H' % (comp_name.upper(),)
print >> out, ''
print >> out, '#include <glib-object.h>'
print >> out, ''
print >> out, 'G_BEGIN_DECLS'
print >> out, ''

for name in vrs.get('enums', {}):
    print_enum_h(name, vrs['enums'][name], out)
    print >> out, ''
for name in vrs.get('flags', {}):
    print_flags_h(name, vrs['flags'][name], out)
    print >> out, ''

print >> out, ''
print >> out, 'G_END_DECLS'
print >> out, ''
print >> out, '#endif /* %s_ENUMS_H */' % (comp_name.upper(),)

out.close()
copy_tmp_if_changed(output_h_tmp, output_h)


## c

def print_flags_or_enum_c(name, vals, out, enum):
    dic = parse_name(name)
    dic['Type'] = enum and 'Enum' or 'Flags'
    dic['type'] = enum and 'enum' or 'flags'
    out.write(
'''GType\n%(name)s_get_type (void)
{
    static GType etype;
    if (G_UNLIKELY (!etype))
    {
        static const G%(Type)sValue values[] = {
''' % dic)

    for v in vals:
        out.write('            { %s, (char*) "%s", (char*) "%s" },\n' % (v[0], v[0], v[0]))

    out.write('''            { 0, NULL, NULL }
        };
        etype = g_%(type)s_register_static ("%(Name)s", values);
    }
    return etype;
}\n
''' % dic)

def print_enum_c(name, vals, out):
    print_flags_or_enum_c(name, vals, out, True)

def print_flags_c(name, vals, out):
    print_flags_or_enum_c(name, vals, out, False)

out = open(output_c_tmp, 'w')

print >> out, '#include "%s"' % (output_h,)
print >> out, ''

for name in vrs.get('enums', {}):
    print_enum_c(name, vrs['enums'][name], out)
for name in vrs.get('flags', {}):
    print_flags_c(name, vrs['flags'][name], out)

print >> out, ''

out.close()
copy_tmp_if_changed(output_c_tmp, output_c)


## stamp

open(stamp, 'w').write('stamp\n')
