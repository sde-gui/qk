__all__ = ['Dict']

from mprj.config._item import Item, create_instance
from mprj.config._xml import XMLGroup, XMLItem


def _load_instance(typ, info, node):
    if issubclass(typ, Item):
        obj = create_instance(info, node.name)
        obj.load(node)
    else:
        obj = typ(node.get())
#     print "_load_instance: ", obj
    return obj

def _save_instance(name, obj):
    if isinstance(obj, Item):
        return obj.save()
    elif obj is None:
        return [XMLItem(name, None)]
    else:
        return [XMLItem(name, str(obj))]

def _copy_instance(obj):
    if isinstance(obj, Item):
        return obj.copy()
    else:
        return type(obj)(obj)


def Dict(info = str, **kwargs):
    if isinstance(info, type):
        typ = info
        info = [typ, kwargs]
    else:
        typ = info[0]

    class Dict(Item):
        __elm_type__ = typ
        __elm_info__ = info

        def __init__(self, *args, **kwargs):
            Item.__init__(self, *args, **kwargs)
            self.__items = {}

        def __len__(self): return len(self.__items)
        def __iter__(self): return self.__items.__iter__()

        def __getitem__(self, key):
            item = self.__items[key]
            if issubclass(Dict.__elm_type__, Item):
                return item.get_value()
            else:
                return item

        def __setitem__(self, key, value):
            if not isinstance(value, Dict.__elm_type__):
                raise TypeError('value %s is invalid for %s' % (value, self))
            if self.__items.has_key(key):
                raise RuntimeError('key %s already exists in %s' % (key, self))
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
            changed = Dict.copy_from(self, other)
            first, common, second = dict_diff(self.__items, other.__items)

            if first or second:
                changed = True
            for key in first:
                self.remove_item(key)
            for key in second:
                self.add_item(_copy_instance(other.__items[key]))

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

        def load(self, node):
#             print "Dict.load: %s, %s, %s" % (Dict.__elm_type__, Dict.__elm_info__, node)
            for c in node.children():
                self[c.name] = _load_instance(Dict.__elm_type__, Dict.__elm_info__, c)

        def save(self):
            nodes = []
            for key in self:
                nodes += _save_instance(key, self[key])
            if nodes:
                return [XMLGroup(self.get_id(), nodes)]
            else:
                return []

    return Dict
