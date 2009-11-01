import sys
import re
import getopt

import scmexpr

class Type(object):
    def __init__(self, name):
        object.__init__(self)
        self.__name = name

    def _get_name(self):
        return self.__name
    def __get_name(self):
        return self._get_name()
    name = property(__get_name)

    def _get_elementary(self):
        return False
    def __get_elementary(self):
        return self._get_elementary()
    elementary = property(__get_elementary)

    def _get_cpp_name(self):
        return self.name
    def __get_cpp_name(self):
        return self._get_cpp_name()
    cpp_name = property(__get_cpp_name)

    def __str__(self):
        return '<type %s>' % (self.name,)

    def resolve(self, api):
        pass

class BuiltinType(Type):
    def __init__(self, name, elementary=True, cpp_name=None):
        Type.__init__(self, name)
        self.__elementary = elementary
        self.__cpp_name = cpp_name or name
        print self.__cpp_name

    def __str__(self):
        return '<builtin type %s>' % (self.name,)

    def _get_elementary(self):
        return self.__elementary

    def _get_cpp_name(self):
        return self.__cpp_name

class ListType(Type):
    def __init__(self, elmType):
        if isinstance(elmType, Type):
            Type.__init__(self, 'List<%s>' % (elmType.name,))
        else:
            Type.__init__(self, 'List<%s>' % (elmType.name,))
        self.elmType = elmType

    def __str__(self):
        return '<list of %s>' % (self.elmType.name,)

    def resolve(self, api):
        self.elmType = api.get_type(self.elmType)

    def _get_cpp_name(self):
        return 'List<%s>' % (self.elmType.cpp_name,)

class Property(object):
    def __init__(self, name, *args):
        object.__init__(self)
        self.__name = name
        self.__read_only = True
        self.__of_type = None
        if name.startswith('is') and name[2:] and name[2].upper() == name[2]:
            self.__setter = 'set' + name[2:]
            self.__getter = name
        else:
            self.__setter = 'set' + name[0].upper() + name[1:]
            self.__getter = 'get' + name[0].upper() + name[1:]
        for a in args:
            if a[0] == 'read-only':
                self.__read_only = True
            elif a[0] == 'read-write':
                self.__read_only = False
            elif a[0] == 'of-type':
                self.__of_type = a[1]
            elif a[0] == 'set':
                self.__setter = a[1]
            elif a[0] == 'get':
                self.__getter = a[1]
            else:
                raise RuntimeError('unknown: %s' % (a,))
        if self.__of_type is None:
            raise RuntimeError('missing type in property %s: %s' % (name, args))

    def _get_setter(self):
        return self.__setter
    def __get_setter(self):
        return self._get_setter()
    setter = property(__get_setter)

    def _get_getter(self):
        return self.__getter
    def __get_getter(self):
        return self._get_getter()
    getter = property(__get_getter)

    def _get_name(self):
        return self.__name
    def __get_name(self):
        return self._get_name()
    name = property(__get_name)

    def _get_read_only(self):
        return self.__read_only
    def __get_read_only(self):
        return self._get_read_only()
    read_only = property(__get_read_only)

    def _get_of_type(self):
        return self.__of_type
    def __get_of_type(self):
        return self._get_of_type()
    of_type = property(__get_of_type)

    def resolve(self, api):
        self.__of_type = api.get_type(self.__of_type)

class DicList(object):
    def __init__(self):
        object.__init__(self)
        self.__list = []
        self.__dic = {}

    def has_key(self, key):
        return self.__dic.has_key(key)

    def __setitem__(self, key, val):
        assert not self.__dic.has_key(key)
        self.__dic[key] = val
        self.__list.append(val)

    def __getitem__(self, key):
        return self.__dic[key]

    def __iter__(self):
        return self.__list.__iter__()

class Class(Type):
    def __init__(self, name):
        Type.__init__(self, name)
        self.__parent = None
        self.props = DicList()

    def __str__(self):
        return '<class %s : %s>' % (self.name, self.__parent and self.__parent.name or self.__parent)

    def _get_parent(self):
        return self.__parent
    def __get_parent(self):
        return self._get_parent()
    def _set_parent(self, parent):
        self.__parent = parent
    def __set_parent(self, parent):
        self._set_parent(parent)
    parent = property(__get_parent, __set_parent)

    def resolve(self, api):
        if self.__parent is not None:
            self.__parent = api.get_type(self.__parent)
        for prop in self.props:
            prop.resolve(api)

class Api(object):
    def __init__(self):
        object.__init__(self)
        self.types = DicList()
        self.add_type(BuiltinType('bool'))
        self.add_type(BuiltinType('string', elementary=False, cpp_name='String'))
        self.add_type(BuiltinType('_ObjectImpl'))

    def add_type(self, typ):
        if self.types.has_key(typ.name):
            raise RuntimeError('type %s already exists' % (typ.name,))
        self.types[typ.name] = typ

    def resolve(self):
        for typ in self.types:
            typ.resolve(self)

    def get_type(self, typ):
        assert isinstance(typ, Type) or isinstance(typ, str)
        if isinstance(typ, Type):
            return typ
        if self.types.has_key(typ):
            return self.types[typ]
        m = re.match('List<(.*)>', typ)
        if m:
            elm_type = self.get_type(m.group(1))
            list_type = ListType(elm_type)
            self.types[typ] = list_type
            return list_type
        raise RuntimeError('unknown type %s' % (typ,))

    def __str__(self):
        return '\n'.join(map(lambda name: str(self.types[name]), self.types.keys()))

class ApiParser(scmexpr.Parser):
    def __init__(self):
        scmexpr.Parser.__init__(self, None)
        self.api = Api()

    def unknown(self, tup):
        print >> sys.stderr, "unknown: ", tup
        sys.exit(1)

    def parse(self, filename):
        self.startParsing(filename)

    def define_class(self, name, *args):
        cls = Class(name)
        self.api.add_type(cls)
        for a in args:
            if a[0] == 'define-property':
                p = Property(a[1], *(a[2:]))
                if cls.props.has_key(p.name):
                    raise RuntimeError('property %s already exists in class %s' % (p.name, cls.name))
                cls.props[p.name] = p
            elif a[0] == 'subclass-of':
                if cls.parent is not None:
                    raise RuntimeError('parent already exists in class %s' % (cls.name,))
                cls.parent = a[1]
            else:
                raise RuntimeError('unknown: %s' % (a,))

cppdecl_start = """\
#ifndef MEDIT_API_H
#define MEDIT_API_H

#ifdef __cplusplus

#include "momobject.h"

namespace mom {

"""

cppdecl_end = """\
} // namespace mom

#endif /* __cplusplus */

#endif /* MEDIT_API_H */
"""

class CppDeclGenerator(object):
    def __init__(self, api):
        object.__init__(self)
        self.api = api

    def write_decl(self, out):
        out.write(cppdecl_start)
        classes = filter(lambda typ: isinstance(typ, Class), self.api.types)
        for cls in classes:
            print >> out, 'class %s;' % (cls.cpp_name)
        print >> out, ''
        for cls in classes:
            self.__write_decl_cls(cls, out)
        out.write(cppdecl_end)

    def __format_in_prop_arg(self, prop):
        if prop.of_type.elementary:
            return '%s' % (prop.of_type.cpp_name,)
        else:
            return 'const %s&' % (prop.of_type.cpp_name,)

    def __format_out_prop_arg(self, prop):
        return '%s&' % (prop.of_type.cpp_name,)

    def __write_decl_cls(self, cls, out):
        print >> out, 'class ' + cls.name + (cls.parent and ' : public %s' % (cls.parent.name,) or '')
        print >> out, '{'
        print >> out, '    MOM_DECLARE_OBJECT_CLASS(%s)' % (cls.name,)
        print >> out, 'public:'
        for prop in cls.props:
            getter_name = '_mom_' + cls.name + '_' + prop.getter
            setter_name = '_mom_' + cls.name + '_' + prop.setter
            print >> out, '    Result ' + prop.getter + '(' + self.__format_out_prop_arg(prop) + ') const;'
            if not prop.read_only:
                print >> out, '    Result ' + prop.setter + '(' + self.__format_in_prop_arg(prop) + ');'
        print >> out, '};'
        print >> out, ''

cppimpl_start = """\
#include "medit-api-impl.h"

namespace mom {
namespace impl {

"""

cppimpl_end = """\
} // namespace impl
} // namespace mom
"""

class CppImplGenerator(object):
    def __init__(self, api):
        object.__init__(self)
        self.api = api

    def write_impl(self, out):
        out.write(cppimpl_start)
        classes = filter(lambda typ: isinstance(typ, Class), self.api.types)
        for cls in classes:
            self.__write_impl_cls(cls, out)
        out.write(cppimpl_end)

    def __format_in_prop_arg(self, prop):
        if prop.of_type.elementary:
            return '%s' % (prop.of_type.cpp_name,)
        else:
            return 'const %s&' % (prop.of_type.cpp_name,)

    def __format_out_prop_arg(self, prop):
        return '%s&' % (prop.of_type.cpp_name,)

    def __write_impl_cls(self, cls, out):
        return
        print >> out, 'class ' + cls.name + (cls.parent and ' : public %s' % (cls.parent.name,) or '')
        print >> out, '{'
        print >> out, 'public:'
        for prop in cls.props:
            getter_name = '_mom_' + cls.name + '_' + prop.getter
            setter_name = '_mom_' + cls.name + '_' + prop.setter
            print >> out, '    Result ' + prop.getter + '(' + self.__format_out_prop_arg(prop) + ') const;'
            if not prop.read_only:
                print >> out, '    Result ' + prop.setter + '(' + self.__format_in_prop_arg(prop) + ');'
        print >> out, '};'
        print >> out, ''

def main(argv):
    out_decl_filename = None
    out_impl_filename = None
    defs_filename = None
    mode = None
    template = None
    opts, args = getopt.getopt(argv[1:], "", ["output-decl=", "output-impl=", "defs=", "mode=", "template="])
    for opt, arg in opts:
        if opt in ('--output-decl'):
            out_decl_filename = arg
        elif opt in ('--output-impl'):
            out_impl_filename = arg
        elif opt in ('--defs'):
            defs_filename = arg
        elif opt in ('--mode'):
            mode = arg
    if args:
        print >> sys.stderr, "bad args"
        return 1

    if not defs_filename:
        print >> sys.stderr, "defs file missing"
        return 1

    p = ApiParser()
    p.parse(defs_filename)
    p.api.resolve()

    jobs = []
    if mode == 'cpp':
        gen = CppDeclGenerator(p.api)
        jobs.append((gen.write_decl, out_decl_filename))
        gen = CppImplGenerator(p.api)
        jobs.append((gen.write_impl, out_impl_filename))
    elif mode:
        print >> sys.stderr, "unknown mode %s" % (mode,)
        return 1

    for j in jobs:
        func = j[0]
        out_filename = j[1]
        out_file = sys.stdout
        close_out_file = False
        if out_filename:
            out_file = open(out_filename, 'w')
            close_out_file = True
        func(out_file)
        if close_out_file:
            out_file.close()

    return 0

if __name__ == '__main__':
    sys.exit(main(sys.argv))
