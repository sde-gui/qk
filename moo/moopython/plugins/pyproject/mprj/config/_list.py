__all__ = ['List']

from mprj.config._item import Item


def _load_instance(typ, info, node):
    if issubclass(typ, Item):
        obj = mprj._item.create_instance(info, node.name)
        obj.load(node)
    else:
        return typ(node.get())

def _save_instance(obj):
    if isinstance(obj, Item):
        return obj.save()
    elif obj is None:
        return XMLItem('item', None)
    else:
        return XMLItem('item', str(obj))

def _copy_instance(obj):
    if isinstance(obj, Item):
        return obj.copy()
    else:
        return type(obj)(obj)


def List(info = str):
    if isinstance(info, type):
        typ = info
        if issubclass(info, Item):
            pass
        else:
            pass
    else:
        typ = info[0]

    class List(Item):
        __elm_type__ = typ
        __elm_info__ = info

        def __init__(self, *args, **kwargs):
            Item.__init__(self, *args, **kwargs)
            self.__items = []

        def __len__(self): return len(self.__items)
        def __iter__(self): return self.__items.__iter__()

        def __getitem__(self, ind):
            item = self.__items[ind]
            if issubclass(List.__elm_type__, Item):
                return item.get_value()
            else:
                return item

        def __eq__(self, other):
            return type(self) == type(other) and \
                   self.get_id() == other.get_id() and \
                   self.__items == other.__items
        def __ne__(self, other):
            return not self.__eq__(other)

        def get_items(self):
            return self.__items

        def get_value(self):
            return self

        def append(self, item):
            if not isinstance(item, List.__elm_type__):
                raise ValueError()
            self.__items.append(item)

        def copy_from(self, other):
            self.__items = []
            for elm in other:
                self.append(_copy_instance(elm))

        def load(self, node):
            for c in node.children():
                self.append(_load_instance(List.__elm_type__, List.__elm_info__, c))

        def save(self):
            nodes = []
            for c in self:
                nodes += _save_instance(c)
            if nodes:
                return [XMLGroup(self.get_id(), nodes)]
            else:
                return []

    return List
