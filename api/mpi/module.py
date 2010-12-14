import re
import xml.etree.ElementTree as _etree

class Doc(object):
    def __init__(self, text):
        object.__init__(self)
        self.text = text

    @staticmethod
    def from_xml(elm):
        return Doc(elm.text)

def _set_unique_attribute(obj, attr, value):
    if getattr(obj, attr) is not None:
        raise RuntimeError("duplicated attribute '%s'" % (attr,))
    setattr(obj, attr, value)

def _set_unique_attribute_bool(obj, attr, value):
    if value.lower() in ('0', 'false', 'no'):
        value = False
    elif value.lower() in ('1', 'true', 'yes'):
        value = True
    else:
        raise RuntimeError("bad value '%s' for boolean attribute '%s'" % (value, attr))
    _set_unique_attribute(obj, attr, value)

class _XmlObject(object):
    def __init__(self):
        object.__init__(self)
        self.doc = None
        self.annotations = {}

    @classmethod
    def from_xml(cls, elm, *args):
        obj = cls()
        obj._parse_xml(elm, *args)
        return obj

    def _parse_xml_element(self, elm):
        if elm.tag == 'doc':
            _set_unique_attribute(self, 'doc', Doc.from_xml(elm))
        else:
            raise RuntimeError('unknown element %s' % (elm.tag,))

    def _parse_attribute(self, attr, value):
        if attr.find('.') >= 0:
            self.annotations[attr] = value
            return True
        else:
            return False

    def _parse_xml(self, elm, *args):
        for attr, value in elm.items():
            if not self._parse_attribute(attr, value):
                raise RuntimeError('unknown attribute %s' % (attr,))
        for child in elm.getchildren():
            self._parse_xml_element(child)

class _ParamBase(_XmlObject):
    def __init__(self):
        _XmlObject.__init__(self)
        self.type = None
        self.transfer_mode = None

    def _parse_attribute(self, attr, value):
        if attr in ('type', 'transfer_mode'):
            _set_unique_attribute(self, attr, value)
        else:
            return _XmlObject._parse_attribute(self, attr, value)
        return True

class Param(_ParamBase):
    def __init__(self):
        _ParamBase.__init__(self)
        self.name = None
        self.default_value = None
        self.allow_none = None

    def _parse_attribute(self, attr, value):
        if attr in ('default_value', 'name'):
            _set_unique_attribute(self, attr, value)
        elif attr == 'allow_none':
            _set_unique_attribute_bool(self, attr, value)
        else:
            return _ParamBase._parse_attribute(self, attr, value)
        return True

class Retval(_ParamBase):
    def __init__(self):
        _ParamBase.__init__(self)

class _FunctionBase(_XmlObject):
    def __init__(self):
        _XmlObject.__init__(self)
        self.name = None
        self.c_name = None
        self.retval = None
        self.params = []

    def _parse_attribute(self, attr, value):
        if attr in ('c_name', 'name'):
            _set_unique_attribute(self, attr, value)
        else:
            return _XmlObject._parse_attribute(self, attr, value)
        return True

    def _parse_xml_element(self, elm):
        if elm.tag == 'retval':
            _set_unique_attribute(self, 'retval', Retval.from_xml(elm))
        elif elm.tag == 'param':
            self.params.append(Param.from_xml(elm))
        else:
            _XmlObject._parse_xml_element(self, elm)

    def _parse_xml(self, elm, *args):
        _XmlObject._parse_xml(self, elm, *args)
        if not self.name:
            raise RuntimeError('function name missing')
        if not self.c_name:
            raise RuntimeError('function c_name missing')

class Function(_FunctionBase):
    def __init__(self):
        _FunctionBase.__init__(self)

class Constructor(_FunctionBase):
    def __init__(self):
        _FunctionBase.__init__(self)

class Method(_FunctionBase):
    def __init__(self):
        _FunctionBase.__init__(self)

class VMethod(_FunctionBase):
    def __init__(self):
        _FunctionBase.__init__(self)
        self.c_name = "fake"

class Type(_XmlObject):
    def __init__(self):
        _XmlObject.__init__(self)
        self.name = None
        self.c_name = None
        self.gtype_id = None

class BasicType(Type):
    def __init__(self, name):
        Type.__init__(self)
        self.name = name

class ArrayType(Type):
    def __init__(self, elm_type):
        Type.__init__(self)
        self.elm_type = elm_type
        self.name = '%sArray*' % elm_type.name
        self.c_name = '%sArray*' % elm_type.name

class GErrorReturnType(Type):
    def __init__(self):
        Type.__init__(self)
        self.name = 'GError**'

class GTypedType(Type):
    def __init__(self):
        Type.__init__(self)
        self.short_name = None

    def _parse_attribute(self, attr, value):
        if attr in ('short_name', 'name', 'gtype_id'):
            _set_unique_attribute(self, attr, value)
        else:
            return Type._parse_attribute(self, attr, value)
        return True

    def _parse_xml(self, elm, *args):
        Type._parse_xml(self, elm, *args)
        if self.name is None:
            raise RuntimeError('class name missing')
        if self.short_name is None:
            raise RuntimeError('class short name missing')
        if self.gtype_id is None:
            raise RuntimeError('class gtype missing')

class Enum(GTypedType):
    def __init__(self):
        GTypedType.__init__(self)

class Flags(GTypedType):
    def __init__(self):
        GTypedType.__init__(self)

class InstanceType(GTypedType):
    def __init__(self):
        GTypedType.__init__(self)
        self.constructor = None
        self.methods = []
        self.__method_hash = {}

    def _parse_xml_element(self, elm):
        if elm.tag == 'method':
            meth = Method.from_xml(elm, self)
            assert not meth.name in self.__method_hash
            self.__method_hash[meth.name] = meth
            self.methods.append(meth)
        elif elm.tag == 'constructor':
            assert not self.constructor
            self.constructor = Constructor.from_xml(elm)
        else:
            GTypedType._parse_xml_element(self, elm)

class Pointer(InstanceType):
    def __init__(self):
        InstanceType.__init__(self)

class Boxed(InstanceType):
    def __init__(self):
        InstanceType.__init__(self)

class Class(InstanceType):
    def __init__(self):
        InstanceType.__init__(self)
        self.parent = None
        self.vmethods = []
        self.constructable = None
        self.__vmethod_hash = {}

    def _parse_attribute(self, attr, value):
        if attr in ('parent'):
            _set_unique_attribute(self, attr, value)
        elif attr in ('constructable'):
            _set_unique_attribute_bool(self, attr, value)
        else:
            return InstanceType._parse_attribute(self, attr, value)
        return True

    def _parse_xml_element(self, elm):
        if elm.tag == 'virtual':
            meth = VMethod.from_xml(elm, self)
            assert not meth.name in self.__vmethod_hash
            self.__vmethod_hash[meth.name] = meth
            self.vmethods.append(meth)
        else:
            InstanceType._parse_xml_element(self, elm)

    def _parse_xml(self, elm, *args):
        InstanceType._parse_xml(self, elm, *args)
        if self.parent is None:
            raise RuntimeError('class parent name missing')
        if self.constructable and self.constructor:
            raise RuntimeError('both constructor and constructable attributes present')

class Module(object):
    def __init__(self):
        object.__init__(self)
        self.name = None
        self.__classes = []
        self.__boxed = []
        self.__pointers = []
        self.__enums = []
        self.__class_hash = {}
        self.__functions = []
        self.__function_hash = {}
        self.__import_modules = []
        self.__parsing_done = False
        self.__types = {}

    def __add_type(self, typ):
        if typ.name in self.__class_hash:
            raise RuntimeError('duplicated type %s' % typ.name)
        self.__class_hash[typ.name] = typ

    def __finish_type(self, typ):
        if isinstance(typ, Type):
            return typ
        if typ in self.__types:
            return self.__types[typ]
        if typ == 'GError**':
            return GErrorReturnType()
        m = re.match(r'([\w\d_]+)\*', typ)
        if m:
            name = m.group(1)
            if name in self.__types:
                obj_type = self.__types[name]
                if isinstance(obj_type, InstanceType):
                    return obj_type
        m = re.match(r'Array<([\w\d_]+)>\*', typ)
        if m:
            elm_type = self.__finish_type(m.group(1))
            return ArrayType(elm_type)
        m = re.match(r'([\w\d_]+)Array\*', typ)
        if m:
            elm_type = self.__finish_type(m.group(1))
            return ArrayType(elm_type)
        return BasicType(typ)

    def __finish_parsing_method(self, meth, typ):
        for p in meth.params:
            p.type = self.__finish_type(p.type)
        if meth.retval:
            meth.retval.type = self.__finish_type(meth.retval.type)

    def __finish_parsing_type(self, typ):
        if hasattr(typ, 'constructor') and typ.constructor is not None:
            self.__finish_parsing_method(typ.constructor, typ)
        for meth in typ.methods:
            self.__finish_parsing_method(meth, typ)
        if hasattr(typ, 'vmethods'):
            for meth in typ.vmethods:
                self.__finish_parsing_method(meth, typ)

    def __finish_parsing(self):
        if self.__parsing_done:
            return

        for typ in self.__classes + self.__boxed + self.__pointers + self.__enums:
            self.__types[typ.name] = typ

        for module in self.__import_modules:
            for typ in module.get_classes() + module.get_boxed() + \
                       module.get_pointers() + module.get_enums():
                self.__types[typ.name] = typ

        for typ in self.__classes + self.__boxed + self.__pointers:
            self.__finish_parsing_type(typ)

        for func in self.__functions:
            self.__finish_parsing_method(func, None)

        self.__parsing_done = True

    def get_classes(self):
        self.__finish_parsing()
        return list(self.__classes)

    def get_boxed(self):
        self.__finish_parsing()
        return list(self.__boxed)

    def get_pointers(self):
        self.__finish_parsing()
        return list(self.__pointers)

    def get_enums(self):
        self.__finish_parsing()
        return list(self.__enums)

    def get_functions(self):
        self.__finish_parsing()
        return list(self.__functions)

    def __parse_module_entry(self, elm):
        if elm.tag == 'class':
            cls = Class.from_xml(elm)
            self.__add_type(cls)
            self.__classes.append(cls)
        elif elm.tag == 'boxed':
            cls = Boxed.from_xml(elm)
            self.__add_type(cls)
            self.__boxed.append(cls)
        elif elm.tag == 'pointer':
            cls = Pointer.from_xml(elm)
            self.__add_type(cls)
            self.__pointers.append(cls)
        elif elm.tag == 'enum':
            enum = Enum.from_xml(elm)
            self.__add_type(enum)
            self.__enums.append(enum)
        elif elm.tag == 'flags':
            enum = Flags.from_xml(elm)
            self.__add_type(enum)
            self.__enums.append(enum)
        elif elm.tag == 'function':
            func = Function.from_xml(elm)
            assert not func.name in self.__function_hash
            self.__function_hash[func.name] = func
            self.__functions.append(func)
        else:
            raise RuntimeError('unknown element %s' % (elm.tag,))

    def import_module(self, mod):
        self.__import_modules.append(mod)

    @staticmethod
    def from_xml(filename):
        mod = Module()
        xml = _etree.ElementTree()
        xml.parse(filename)
        root = xml.getroot()
        assert root.tag == 'module'
        mod.name = root.get('name')
        assert mod.name is not None
        for elm in root.getchildren():
            mod.__parse_module_entry(elm)
        return mod
