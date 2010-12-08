#
#  mprj/config/_item.py
#
#  Copyright (C) 2004-2010 by Yevgen Muntyan <emuntyan@sourceforge.net>
#
#  This file is part of medit.  medit is free software; you can
#  redistribute it and/or modify it under the terms of the
#  GNU Lesser General Public License as published by the
#  Free Software Foundation; either version 2.1 of the License,
#  or (at your option) any later version.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with medit.  If not, see <http://www.gnu.org/licenses/>.
#

__all__ = ['Item']

from moo import _


def create_instance(descr, *args, **kwargs):
    if isinstance(descr, Item):
        return descr

    if not isinstance(descr, type) or not issubclass(descr, Item):
        raise TypeError('invalid argument for create_instance: %s' % (descr,))

    return descr.create_instance(*args, **kwargs)


def _create_item_class(cls, dct):
    if not dct:
        return cls
    class ItemClass(cls):
        __item_attributes__ = dct
    return ItemClass

class _ItemMeta(type):
    # Override __call__ so that syntax FooSetting(foo=bar)
    # returns new class derived from FooSetting with __item_attributes__
    # set. It makes it possible to use nicer {'id' : FooSetting(default=1)}
    # syntax in Group __items__ attribute.
    # If _do_create_instance keyword argument is True, then
    # actually do create an instance.
    def __call__(self, *args, **kwargs):
        if kwargs.has_key('_do_create_instance') and \
           kwargs['_do_create_instance'] is True:
            del kwargs['_do_create_instance']
            obj = type.__call__(self, *args, **kwargs)
            kwargs['_do_create_instance'] = True
            return obj
        else:
            dct = {}
            for k in kwargs:
                dct[k] = kwargs[k]
            return _create_item_class(self, dct)

    def __init__(cls, name, bases, dic):
        super(_ItemMeta, cls).__init__(name, bases, dic)

        # Add create_instace class method.
        if not dic.has_key('create_instance'):
            def create_instance(self, *args, **kwargs):
                kwargs['_do_create_instance'] = True
                obj = self(*args, **kwargs)
                del kwargs['_do_create_instance']
                return obj
            setattr(cls, 'create_instance', classmethod(create_instance))

        meta_attrs = {}
        if dic.has_key('__class_attributes__'):
            meta_attrs = dic['__class_attributes__']
        for c in bases:
            if hasattr(c, '__class_attributes__'):
                m = getattr(c, '__class_attributes__')
                for k in m:
                    meta_attrs[k] = m[k]
        setattr(cls, '__class_attributes__', meta_attrs)

        attrs = {}
        if dic.has_key('__item_attributes__'):
            attrs = dic['__item_attributes__']
        for a in meta_attrs:
            if dic.has_key(a):
                attrs[meta_attrs[a]] = dic[a]
        for c in bases:
            if hasattr(c, '__item_attributes__'):
                d = getattr(c, '__item_attributes__')
                for k in d:
                    # XXX copy
                    if not attrs.has_key(k):
                        attrs[k] = d[k]
        setattr(cls, '__item_attributes__', attrs)

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
            def notimplemented(name):
                def notimplemented(*args, **kwargs):
                    raise NotImplementedError('Class %s does not implement %s()' % (cls, name))
                return notimplemented
#             if not dic.has_key('copy_from'):
#                 setattr(cls, 'copy_from', notimplemented('copy_from'))
            if not dic.has_key('load'):
                setattr(cls, 'load', notimplemented('load'))
            if not dic.has_key('save'):
                setattr(cls, 'save', notimplemented('save'))


class Item(object):
    __metaclass__ = _ItemMeta
    __class_attributes__ = {
        '__item_name__' : 'name',
        '__item_description__' : 'description',
        '__item_cell_types__' : 'cell_types',
        '__item_visible__' : 'visible',
    }

    def __init__(self, id, name=None, description=None, visible=None, cell_types=None):
        object.__init__(self)
        self.__id = id

        attrs = getattr(type(self), '__item_attributes__')

        if not cell_types and attrs.has_key('cell_types'):
            cell_types = attrs['cell_types']
            if not isinstance(cell_types, list) and \
               not isinstance(cell_types, tuple):
                    cell_types = [cell_types]
        if not cell_types:
            cell_types = []

        if name is None:
            if attrs.has_key('name'):
                name = attrs['name']
            else:
                name = _(id)

        if description is None:
            if attrs.has_key('description'):
                description = attrs['description']
            else:
                description = name

        if visible is None:
            if attrs.has_key('visible'):
                visible = attrs['visible']
            else:
                visible = True

        self.__config_cells = []
        self.__name = name
        self.__description = description
        self.__visible = visible
        self.__cell_types = cell_types

    def set_id(self, new_id): self.__id = new_id
    def set_name(self, name): self.__name = name
    def set_description(self, description): self.__description = description
    def set_visible(self, visible): self.__visible = visible
    def set_cell_types(self, cell_types): self.__cell_types = cell_types

    def get_id(self): return self.__id
    def get_name(self): return self.__name
    def get_description(self): return self.__description
    def get_visible(self): return self.__visible
    def get_cell_types(self): return self.__cell_types

    def set_config_cells(self, cells): self.__config_cells = cells
    def get_config_cells(self): return self.__config_cells

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

    def dump_xml(self):
        nodes = self.save()
        if not nodes:
            return ''
        if len(nodes) > 1:
            return '\n'.join([n.get_string() for n in nodes])
        else:
            return nodes[0].get_string()

    def __str__(self):
        return '<%s %s>' % (type(self), self.get_id())
    def __repr__(self):
        return self.__str__()
