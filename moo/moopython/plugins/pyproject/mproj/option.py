class Attribute(object):
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

    def default(self):
        return self.__default


class _DictMeta(type):
    def __install_attribute(cls, name):
        def get_prop(obj):
            if not hasattr(obj, 'get_' + name):
                return getattr(obj, '_' + name)
            else:
                return getattr(obj, 'get_' + name)(obj)
        def set_prop(obj, val):
            if not hasattr(obj, 'set_' + name):
                setattr(obj, '_' + name, val)
            else:
                getattr(obj, 'set_' + name)(obj, val)
        setattr(cls, name, property(get_prop, set_prop))

    def __init__(cls, name, bases, dic):
        super(_DictMeta, cls).__init__(name, bases, dic)

        attributes = {}

        if dic.has_key('__attributes__'):
            src = dic['__attributes__']
            for a in src:
                _DictMeta.__install_attribute(cls, a)
                attributes[a] = Attribute(a, src[a])

        for b in bases:
            if hasattr(b, '__attributes__'):
                parent_attrs = getattr(b, '__attributes__')
                for a in parent_attrs:
                    if not attributes.has_key(a):
                        attributes[a] = parent_attrs[a]

        setattr(cls, '__attributes__', attributes)

class Dict(object):
    __metaclass__ = _DictMeta
    __attributes__ = {'fuck' : [str, '1'],
                      'crap' : [str, '2']}

    def __init__(self):
        attrs = getattr(type(self), '__attributes__')
        for a in attrs:
            setattr(self, a, attrs[a].default())


if __name__ == '__main__':
    d = Dict()
    print d.fuck
    print d.crap
