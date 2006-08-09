__all__ = ['Dict']

from mprj.config._item import Item


def _load_instance(typ, info, node):
    if issubclass(typ, Item):
        obj = mprj._item.create_instance(info, node.name)
        obj.load(node)
    else:
        return typ(node.get())

def _save_instance(name, obj):
    if isinstance(obj, Item):
        return obj.save()
    elif obj is None:
        return XMLItem(name, None)
    else:
        return XMLItem(name, str(obj))

def _copy_instance(obj):
    if isinstance(obj, Item):
        return obj.copy()
    else:
        return type(obj)(obj)


def Dict(info = str):
    if isinstance(info, type):
        typ = info
        if issubclass(info, Item):
            pass
        else:
            pass
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
            self.__items[key] = value

        def __eq__(self, other):
            return type(self) == type(other) and \
                   self.get_id() == other.get_id() and \
                   self.__items == other.__items
        def __ne__(self, other):
            return not self.__eq__(other)

        def get_value(self):
            return self

        def get_items(self):
            return self.__items.items()

        def copy_from(self, other):
            self.__items = {}
            for key in other:
                self[key] = _copy_instance(other[key])

        def load(self, node):
            print 'load'
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
