import os
import sys
import re

in_file = sys.argv[1]
classes = {}
singletons = []
gobjects = []
re_singleton = re.compile(r':\s*public\s+_Singleton\s*<\s*(\s*\w+\s*)\s*>')
re_gobject = re.compile(r':\s*public\s+_GObjectWrapper\s*<\s*(\w+)\s*,\s*\w+\s*>')
re_prop = re.compile(r'^\s*\bPROPERTY\s*\((\w+)\s*,\s*(\w+)\s*,\s*([\w-]+)\s*\)')
re_meth = re.compile(r'^\s*\bMETHOD\s*\((\w+)\s*,\s*(\w+)\s*\)')
re_ignore = re.compile(r'$^')

def get_class(class_name):
    cls = classes.get(class_name)
    if not cls:
        cls = [[], []]
        classes[class_name] = cls
    return cls

def add_property(class_name, prop_name, flags):
    cls = get_class(class_name)
    cls[0].append([prop_name, flags])

def add_method(class_name, meth_name):
    cls = get_class(class_name)
    cls[1].append(meth_name)

for line in open(in_file):
    if re_ignore.search(line):
        continue
    m = re_singleton.search(line)
    if m:
        singletons.append(m.group(1))
    m = re_gobject.search(line)
    if m:
        gobjects.append(m.group(1))
    m = re_prop.search(line)
    if m:
        add_property(m.group(1), m.group(2), m.group(3))
    m = re_meth.search(line)
    if m:
        add_method(m.group(1), m.group(2))

for class_name in singletons:
    sys.stdout.write('MOM_SINGLETON_DEFN(%s)\n' % class_name)
for class_name in gobjects:
    sys.stdout.write('MOM_GOBJECT_DEFN(%s)\n' % class_name)
sys.stdout.write('\n')

tmpl_start = """\
void %(Class)s::InitMetaObject(MetaObject &meta)
{
"""

tmpl_end = """\
}
"""

tmpl_prop = """\
    meta.add_property("%(property)s", %(getter)s, %(setter)s);
"""

tmpl_meth = """\
    meta.add_method("%(method)s", &%(Class)s::%(method)s);
"""

for class_name in sorted(classes.keys()):
    cls = classes[class_name]
    dic = dict(Class=class_name)
    sys.stdout.write(tmpl_start % dic)
    for prop_name, flags in cls[0]:
        readable = (flags in ('read', 'read-write'))
        writable = (flags in ('write', 'read-write'))
        assert readable or writable
        dic['property'] = prop_name
        dic['getter'] = readable and ('&%s::get_%s' % (class_name, prop_name)) or '(PropertyGetter) 0'
        dic['setter'] = writable and ('&%s::set_%s' % (class_name, prop_name)) or '(PropertySetter) 0'
        sys.stdout.write(tmpl_prop % dic)
    for meth_name in cls[1]:
        dic['method'] = meth_name
        sys.stdout.write(tmpl_meth % dic)
    sys.stdout.write(tmpl_end % dic)
    sys.stdout.write('\n')
