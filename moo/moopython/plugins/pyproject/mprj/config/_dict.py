__all__ = ['Dict']

from mprj.config._item import Item, create_instance
from mprj.config._xml import XMLGroup, XMLItem
from mprj.config._utils import dict_diff


def _load_instance(typ, node, id):
    if issubclass(typ, Item):
        obj = create_instance(typ, id)
        obj.load(node)
    else:
        val = node.get()
        # XXX
        if val is None and typ is str:
            obj = None
        else:
            obj = typ(val)
    return obj

def _create_node(elm_name, attr_name, name, string):
    if elm_name is not None:
        node = XMLItem(elm_name, string)
        node.set_attr(attr_name, name)
    else:
        node = XMLItem(name, string)
    return node

def _save_instance(elm_name, attr_name, name, obj):
    if isinstance(obj, Item):
        nodes = obj.save()
        if elm_name is not None:
            if len(nodes) != 1:
                raise NotImplementedError()
            nodes[0].name = elm_name
            nodes[0].set_attr(attr_name, name)
        return nodes
    if obj is None:
        return [_create_node(elm_name, attr_name, name, None)]
    else:
        return [_create_node(elm_name, attr_name, name, str(obj))]

def _copy_instance(obj):
    if isinstance(obj, Item):
        return obj.copy()
    else:
        return type(obj)(obj)

def _check_type(obj, elm_type):
    if obj is not None:
        return isinstance(obj, elm_type)
    if elm_type is str:
        return True
    return False

class DictBase(Item):
    pass

def Dict(typ, **kwargs):
    if not isinstance(typ, type):
        raise TypeError('argument %s is invalid for Dict()' % (typ,))

    attrs = {}
    for k in kwargs:
        attrs[k] = kwargs[k]

    class Dict(DictBase):
        __elm_type__ = typ
        __item_attributes__ = attrs

        def __init__(self, *args, **kwargs):
            Item.__init__(self, *args, **kwargs)
            self.__items = {}

            attrs = getattr(type(self), '__item_attributes__')

            if attrs.has_key('xml_elm_name'):
                self.__xml_elm_name = attrs['xml_elm_name']
            else:
                self.__xml_elm_name = None

            if self.__xml_elm_name is not None:
                if attrs.has_key('xml_attr_name'):
                    self.__xml_attr_name = attrs['xml_attr_name']
                else:
                    self.__xml_attr_name = 'name'

        def __len__(self): return len(self.__items)
        def __iter__(self): return self.__items.__iter__()
        def has_key(self, key): return self.__items.has_key(key)
        def __delitem__(self, key): del self.__items[key]

        def __getitem__(self, key):
            item = self.__items[key]
            if issubclass(Dict.__elm_type__, Item):
                return item.get_value()
            else:
                return item

        def __setitem__(self, key, value):
            if not _check_type(value, Dict.__elm_type__):
                print 'value: ', value
                print '__elm_type__: ', Dict.__elm_type__
                raise TypeError('value %s is invalid for %s' % (value, self))
            self.__items[key] = value

        def __eq__(self, other):
            return type(self) == type(other) and \
                   self.get_id() == other.get_id() and \
                   self.__items == other.__items
        def __ne__(self, other):
            return not self.__eq__(other)

        def get_value(self):
            return self

        def items(self): return self.__items.items()
        def keys(self): return self.__items.keys()

        def copy_from(self, other):
            changed = Item.copy_from(self, other)
            first, common, second = dict_diff(self.__items, other.__items)

            if first or second:
                changed = True
            for key in first:
                del self[key]
            for key in second:
                self[key] = _copy_instance(other.__items[key])

            if issubclass(Dict.__elm_type__, Item):
                for key in common:
                    changed = self.__items[key].copy_from(other.__items[key]) or changed
            else:
                for key in common:
                    old = self.__items[key]
                    new = other.__items[key]
                    if old != new:
                        self.__items[key] = new
                        changed = True

            return changed

        def rename(self, old_key, new_key):
            item = self[old_key]
            if self.has_key(new_key):
                raise KeyError('key %s already exists' % (new_key,))
            self.__items[new_key] = item
            del self.__items[old_key]
            assert item.get_id() == old_key
            item.set_id(new_key)

        def load(self, node):
            for c in node.children():
                if self.__xml_elm_name is not None:
                    if c.name == self.__xml_elm_name:
                        if not c.has_attr(self.__xml_attr_name):
                            raise RuntimeError("element '%s' doesn't have '%s' attribute" % \
                                                (c.name, self.__xml_attr_name))
                        key = c.get_attr(self.__xml_attr_name)
                        self[key] = _load_instance(Dict.__elm_type__, c, key)
                    else:
                        raise RuntimeError("unknown element '%s'" % (c.name,))
                else:
                    self[c.name] = _load_instance(Dict.__elm_type__, c, c.name)

    	def cmp_nodes(self, n1, n2):
	    return cmp(n1.name, n2.name) or \
		   cmp(n1.get_attr(self.__xml_attr_name), n2.get_attr(self.__xml_attr_name))
        def save(self):
            nodes = []
            for key in self:
                if self.__xml_elm_name is not None:
                    nodes += _save_instance(self.__xml_elm_name, self.__xml_attr_name, key, self[key])
                else:
                    nodes += _save_instance(None, None, key, self[key])
            if nodes:
		nodes.sort(self.cmp_nodes)
                return [XMLGroup(self.get_id(), nodes, sort=False)]
            else:
                return []

    return Dict
