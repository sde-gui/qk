__all__ = ['Item']

from moo.utils import _


def create_instance(descr, *args, **kwargs):
#     print "create_instance: %s, %s, %s" % (descr, args, kwargs)

    if isinstance(descr, Item):
        return descr

    item_type = None

    if isinstance(descr, type):
        if issubclass(descr, Item):
            item_type = descr
        else:
            raise TypeError('invalid argument for create_instance: %s' % (descr,))
    else:
        if not issubclass(descr[0], Item):
            raise TypeError('invalid argument for create_instance: %s' % (descr,))
        item_type = descr[0]
        if not args or len(args) > 1:
            raise TypeError('invalid arguments for create_instance: %s, %s, %s' % (descr, args, kwargs))
        if kwargs:
            raise TypeError('invalid arguments for create_instance: %s, %s, %s' % (descr, args, kwargs))
        if len(descr) > 2:
            raise TypeError('invalid arguments for create_instance: %s, %s, %s' % (descr, args, kwargs))
        if descr[1:]:
            kwargs = descr[1]
        else:
            kwargs = {}

    return item_type.create_instance(*args, **kwargs)


class _ItemMeta(type):
    # Override __call__ so that syntax FooSetting(foo=bar)
    # returns [FooSetting, {'foo' : bar}]. It makes it possible
    # to use nicer {'id' : FooSetting(default=1)} syntax in
    # Group __items__ attribute.
    # If do_create_instance keyword argument is True, then
    # actually do create an instance.
    def __call__(self, *args, **kwargs):
        if kwargs.has_key('do_create_instance') and \
           kwargs['do_create_instance'] is True:
            del kwargs['do_create_instance']
            obj = type.__call__(self, *args, **kwargs)
            kwargs['do_create_instance'] = True
            return obj
        else:
            dct = {}
            for k in kwargs:
                dct[k] = kwargs[k]
            return [self, dct]

    def __init__(cls, name, bases, dic):
        super(_ItemMeta, cls).__init__(name, bases, dic)

        # Add create_instace class method.
        if not dic.has_key('create_instance'):
            def create_instance(self, *args, **kwargs):
                kwargs['do_create_instance'] = True
                obj = self(*args, **kwargs)
                del kwargs['do_create_instance']
                return obj
            setattr(cls, 'create_instance', classmethod(create_instance))

        # check basic methods
        if not hasattr(cls, '__no_item_methods__') and not dic.has_key('__no_item_methods__'):
            if dic.has_key('__init__') or dic.has_key('set_value'):
                if not dic.has_key('copy_from'):
                    raise RuntimeError('Class %s does not implement copy_from()' % (cls,))
                if not dic.has_key('load'):
                    raise RuntimeError('Class %s does not implement load()' % (cls,))
                if not dic.has_key('save'):
                    raise RuntimeError('Class %s does not implement save()' % (cls,))
        else:
            def notimplemented(*args, **kwargs):
                raise NotImplementedError('Class %s does not implement save()' % (cls,))
            if not dic.has_key('copy_from'):
                setattr(cls, 'copy_from', notimplemented)
            if not dic.has_key('load'):
                setattr(cls, 'load', notimplemented)
            if not dic.has_key('save'):
                setattr(cls, 'save', notimplemented)



class Item(object):
    __metaclass__ = _ItemMeta

    def __init__(self, id, name=None, description=None, visible=True):
        object.__init__(self)
        self.__id = id

        if name:
            self.__name = name or _(id)
        elif hasattr(type(self), '__item_name__'):
            self.__name = getattr(type(self), '__item_name__')
        else:
            self.__name = _(id)

        if description:
            self.__description = description
        elif hasattr(type(self), '__item_description__'):
            self.__description = getattr(type(self), '__item_description__')
        else:
            self.__description = self.__name

        if hasattr(type(self), '__item_visible__'):
            self.__visible = getattr(type(self), '__item_visible__')
        else:
            self.__visible = visible

    def set_name(self, name): self.__name = name
    def set_description(self, description): self.__description = description
    def set_visible(self, visible): self.__visible = visible

    def get_id(self): return self.__id
    def get_name(self): return self.__name
    def get_description(self): return self.__description
    def get_visible(self): return self.__visible

    def load(self, node): raise NotImplementedError()
    def save(self): raise NotImplementedError()
    def get_value(self): raise NotImplementedError()
    def set_value(self): raise NotImplementedError()

    def copy_from(self, other):
        self.__name = other.__name
        self.__description = other.__description
        self.__visible = other.__visible
        return False

    def copy(self):
        copy = create_instance(type(self), self.get_id())
        copy.copy_from(self)
        return copy

    def __str__(self):
        return '<%s %s>' % (type(self), self.get_id())
    def __repr__(self):
        return self.__str__()
