"""mproj.config - medit projects configuration"""

from mproj.configxml import XML, File
from mproj.configxml import Text as TextNode
from mproj.configxml import Dir as DirNode


class _Attribute(object):
    def __new__(cls, name, data):
        if isinstance(data, _Attribute):
            return data
        else:
            return super(_Attribute, cls).__new__(cls, name, data)

    def __init__(self, name, data):
        object.__init__(self)

        self.name = name
        self.__default = None

        if isinstance(data, type):
            self.type = data
        else:
            self.type = data[0]
            self.__default = data[1]

        if hasattr(self.type, '__default__'):
            self.__default = self.type.__default__

    def load(self, node):
        if node is None:
            return self.default()

        if hasattr(self.type, 'load'):
            d = self.type()
            d.load(node)
            return d

        s = node.get()

        if self.type is int:
            assert s is not None
            return int(s)
        if self.type is str:
            if s is None:
                return None
            else:
                return str(s)
        if self.type is unicode:
            return s
        raise RuntimeError()

    def default(self):
        return self.copy(self.__default)

    def copy(self, val):
        if val is None:
            return None
        if hasattr(val, 'copy'):
            return val.copy()
        if hasattr(self.type, 'copy'):
            return self.type.copy(val)
        if self.type is int:
            return int(val)
        if self.type is str:
            return str(val)
        if self.type is unicode:
            return unicode(val)
        raise RuntimeError()

    def save(self, val):
        name = self.name
        if not name:
            name = val.name
        if val is None:
            return []
        if hasattr(val, 'save'):
            return val.save(name)
        if self.__default == val:
            return []
        if self.type is int or self.type is str or self.type is unicode:
            return [TextNode(name, unicode(val))]
        raise NotImplementedError("Don't know how to save %s" % (val,))


class _DictMeta(type):
    def __init__(cls, name, bases, dic):
        super(_DictMeta, cls).__init__(name, bases, dic)

        attributes = {}

        if dic.has_key('__attributes__'):
            src = dic['__attributes__']
            for a in src:
                attributes[a] = _Attribute(a, src[a])

        for b in bases:
            if hasattr(b, '__attributes__'):
                parent_attrs = getattr(b, '__attributes__')
                for a in parent_attrs:
                    if not attributes.has_key(a):
                        attributes[a] = parent_attrs[a]

        setattr(cls, '__attributes__', attributes)


def _cmp_nodes(n1, n2):
    return (n1.name < n2.name and -1) or (n1.name > n2.name and 1) or 0


class Dict(object):
    __metaclass__ = _DictMeta

    def __init__(self):
        attrs = getattr(type(self), '__attributes__')
        for a in attrs:
            setattr(self, a, attrs[a].default())

    def load(self, node):
        attrs = getattr(type(self), '__attributes__')
        for c in node.children():
            if attrs.has_key(c.name):
                a = attrs[c.name]
                setattr(self, c.name, a.load(c))

    def copy(self):
        Type = type(self)
        copy = Type()
        attrs = getattr(Type, '__attributes__')
        for a in attrs:
            setattr(copy, a, attrs[a].copy(getattr(self, a)))
        return copy

    def save(self, name):
        children = []
        attrs = getattr(type(self), '__attributes__')

        for n in attrs:
            if not hasattr(self, n):
                continue
            val = getattr(self, n)
            children += attrs[n].save(val)

        if not children:
            return []

#         print 'Dict.save children: ', children
#         print 'Dict.save: ', DirNode(name, children)
        children.sort(_cmp_nodes)
        return [DirNode(name, children)]


    def __repr__(self):
        attrs = getattr(type(self), '__attributes__')
        dic = {}
        for a in attrs:
            if hasattr(self, a):
                val = getattr(self, a)
                if val is not None:
                    dic[a] = repr(val)
        return repr(dic)


def List(data = str):
    class List(list):
        elm_data = _Attribute(None, data)

        def load(self, node):
            Type = type(self)
            attr = getattr(Type, 'elm_data')
            for c in node.children():
                self.append(attr.load(c))

        def copy(self):
            Type = type(self)
            attr = getattr(Type, 'elm_data')
            copy = Type()
            for elm in self:
                copy.append(attr.copy(elm))
            return copy

        def save(self, name):
            t = type(self)
            attr = getattr(t, 'elm_data')
            nodes = []

            for c in self:
                nodes += attr.save(c)

            if nodes:
                return [DirNode(name, nodes)]
            else:
                return []

    return List


class Config(Dict):
    def __init__(self, file):
        Dict.__init__(self)
        self.file = file
        self.name = file.name
        self.type = file.project_type
        self.version = file.version

    def load(self):
        Dict.load(self, self.file.root)

    def get_xml(self):
        xml = Dict.save(self, 'medit-project')[0]
        xml.set_attr('name', self.name)
        xml.set_attr('type', self.type)
        xml.set_attr('version', self.version)
        return xml


class StringDict(dict):
    def load(self, node):
        for c in node.children():
            self[c.name] = c.get()
    def save(self, name):
        nodes = []
        for k in self:
            val = self[k]
            if val is not None:
                nodes.append(TextNode(k, val))
        if nodes:
            nodes.sort(_cmp_nodes)
            return [DirNode(name, nodes)]
        else:
            return []
    def copy(self):
        copy = StringDict()
        for key in self:
            copy[key] = self[key]
        return copy

if __name__ == '__main__':
    f = File("""<medit-project version="1.0" name="Foo" type="Simple">
                  <variables>
                    <foo>bar</foo>
                  </variables>
                  <project_dir>.</project_dir>
                  <stuff>
                    <kff>ddd</kff>
                  </stuff>
                </medit-project>""")

    class D(Dict):
        __attributes__ = {'kff' : str,
                          'blah' : int}

    class C(Config):
        __attributes__ = {'variables' : StringDict,
                          'project_dir' : str,
                          'stuff' : D,
                          'somestuff' : D}

    c = C(f)
    c.load()
    print c.variables
    print c.project_dir
    print c.stuff
    print f
    print c
    print c.get_xml()
