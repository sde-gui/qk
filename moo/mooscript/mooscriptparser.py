#! /usr/bin/env python

import sys
import os
import xml.etree.cElementTree as etree

class Module(object):
    def __init__(self):
        object.__init__(self)
        self.classes = {}
        self.types = {}

class Type(object):
    def __init__(self, name):
        object.__init__(self)
        self.name = name

class BasicType(Type):
    def __init__(self, name):
        Type.__init__(self, name)

    def __str__(self):
        return '<type %s>' % (self.name,)

class Retval(object):
    def __init__(self, typ):
        object.__init__(self)
        self.type = typ

    def __str__(self):
        return '<retval %s>' % (self.type,)

class Param(object):
    def __init__(self, name, typ=None):
        object.__init__(self)
        self.name = name
        self.type = typ
        self.optional = False

    def __str__(self):
        return '<param %s>' % (self.name,)

class _MethodBase(object):
    def __init__(self, name):
        object.__init__(self)
        self.name = name
        self.params = []
        self.retval = None
        self.varargs = False
        self.kwargs = False

class Method(_MethodBase):
    def __init__(self, name):
        _MethodBase.__init__(self, name)
    def __str__(self):
        return '<method %s>' % (self.name,)

class Signal(_MethodBase):
    def __init__(self, name):
        _MethodBase.__init__(self, name)
    def __str__(self):
        return '<method %s>' % (self.name,)

class Class(Type):
    def __init__(self, name):
        Type.__init__(self, name)
        self.methods = {}
        self.signals = {}
        self.singleton = False
        self.gobject = None

    def __str__(self):
        return '<class %s>' % (self.name,)

def parse_bool(s):
    if s == '1':
        return True
    elif s == '0':
        return False
    else:
        raise RuntimeError("invalid value '%s' for boolean attribute" % (s,))

class Parser(object):
    def __init__(self):
        object.__init__(self)

    def parse(self, filename):
        xml = etree.parse(filename)
        root = xml.getroot()
        if root.tag != 'classes':
            raise RuntimeError("root element is not 'classes'")
        mod = Module()
        for elm in root:
            if elm.tag == 'class':
                cls = self.__parse_class(elm)
                if mod.classes.has_key(cls.name):
                   raise RuntimeError("duplicated class '%s'" % (cls.name,))
                mod.classes[cls.name] = cls
            else:
                raise RuntimeError("unknown element '%s'" % (elm.tag,))
        self.__check_types(mod)
        return mod

    def __check_type(self, typ, mod):
        if typ is None or isinstance(typ, Type):
            return typ
        elif isinstance(typ, str):
            real_type = mod.types.get(typ)
            if real_type is None:
                raise RuntimeError("unknown type '%s'" % (typ,))
            return real_type
        else:
            raise RuntimeError("oops: '%s'" % (typ,))

    def __check_types(self, mod):
        for t in ('bool', 'string', 'variant', 'list', 'int', 'index', 'arglist', 'argset'):
            mod.types[t] = BasicType(t)
        for name in mod.classes:
            cls = mod.classes[name]
            if mod.types.has_key(name):
                raise RuntimeError("duplicated type '%s'" % (name,))
            mod.types[name] = cls
        for name in mod.classes:
            cls = mod.classes[name]
            for m in cls.methods.values() + cls.signals.values():
                for p in m.params:
                    p.type = self.__check_type(p.type, mod)
                if m.retval:
                    m.retval.type = self.__check_type(m.retval.type, mod)

    def __parse_class(self, elm):
        cls = Class(elm.attrib['name'])
        cls.gobject = elm.get('gobject')
        if elm.get('singleton'):
            cls.singleton = parse_bool(elm.get('singleton'))
        for child in elm:
            if child.tag == 'method':
                meth = self.__parse_method(child)
                if cls.methods.has_key(meth.name):
                   raise RuntimeError("duplicated method '%s'" % (meth.name,))
                cls.methods[meth.name] = meth
            elif child.tag == 'signal':
                sig = self.__parse_signal(child)
                if cls.signals.has_key(sig.name):
                   raise RuntimeError("duplicated signal '%s'" % (sig.name,))
                cls.signals[sig.name] = sig
            else:
                raise RuntimeError("unknown element '%s'" % (elm.tag,))
        return cls

    def __parse_method_or_signal(self, elm, What):
        meth = What(elm.attrib['name'])
#         print '', meth
        retval = elm.get('retval')
        if retval:
            meth.retval = Retval(retval)
#             print ' ', meth.retval
        if elm.get('varargs') is not None:
            va = parse_bool(elm.get('varargs'))
            if va:
                meth.varargs = True
                meth.params.append(Param('args', 'arglist'))
            else:
                raise RuntimeError('oops')
        elif elm.get('kwargs') is not None:
            va = parse_bool(elm.get('kwargs'))
            if va:
                meth.kwargs = True
                meth.params.append(Param('args', 'argset'))
            else:
                raise RuntimeError('oops')
        elif elm.get('param-name') is not None:
            p = Param(elm.get('param-name'), elm.get('param-type'))
            if elm.get('param-optional') is not None:
                p.optional = parse_bool(elm.get('param-optional'))
            meth.params.append(p)
        else:
            for child in elm:
                if child.tag == 'param':
                    param = self.__parse_param(child)
                    meth.params.append(param)
                else:
                    raise RuntimeError("unknown element '%s'" % (elm.tag,))
        for p in meth.params:
            if p.type is None:
                raise RuntimeError("bad method specification: '%s'" % (meth.name,))
        return meth

    def __parse_method(self, elm):
        return self.__parse_method_or_signal(elm, Method)

    def __parse_signal(self, elm):
        return self.__parse_method_or_signal(elm, Signal)

    def __parse_param(self, elm):
        param = Param(elm.get('name'))
        param.type = elm.get('type')
        if elm.get('optional') is not None:
            param.optional = parse_bool(elm.get('optional'))
        return param

if __name__ == '__main__':
    p = Parser()
    p.parse('mom.xml')
