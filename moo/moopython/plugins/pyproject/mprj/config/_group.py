__all__ = ['Group', 'ValueNotSet']

from mprj.config._item import Item, _ItemMeta, create_instance
from mprj.config._xml import XMLGroup
from mprj.config._utils import dict_diff


class ValueNotSetType(object):
    def __new__(cls):
        if not hasattr(cls, 'instance'):
            instance = object.__new__(cls)
            setattr(cls, 'instance', instance)
        return getattr(cls, 'instance')
    def __nonzero__(self):
        return False

ValueNotSet = ValueNotSetType()


class _GroupMeta(_ItemMeta):
    def __init__(cls, name, bases, dic):
        super(_GroupMeta, cls).__init__(name, bases, dic)

        items = {}
        deleted = {}

        if dic.has_key('__items__'):
            src = dic['__items__']
            for a in src:
                if src[a] != 'delete':
                    items[a] = src[a]
                else:
                    deleted[a] = ''

        for b in bases:
            if hasattr(b, '__items__'):
                parent_items = getattr(b, '__items__')
                for i in parent_items:
                    if not items.has_key(i) and not deleted.has_key(i):
                        items[i] = parent_items[i]

        if items:
            setattr(cls, '__items__', items)

def _cmp_nodes(n1, n2):
    return (n1.name < n2.name and -1) or (n1.name > n2.name and 1) or 0

class Group(Item):
    __metaclass__ = _GroupMeta

    def __init__(self, id, items={}, not_set=False, **kwargs):
        Item.__init__(self, id, **kwargs)

        self.__items = []
        self.__items_dict = {}
        self.__not_set = not_set

        if items:
            for id in items:
                self.add_item(items[id], id)

        if hasattr(type(self), '__items__'):
            items = getattr(type(self), '__items__')
            for id in items:
                self.add_item(items[id], id)

    def __getattr__(self, attr):
        if self.has_item(attr):
            return self[attr].get_value()
        else:
            raise KeyError("no attribute '%s' in '%s'" % (attr, self))

    def __setattr__(self, name, value):
        if name.startswith('_'):
            Item.__setattr__(self, name, value)
        else:
            dic = self.__items_dict
            if dic.has_key(name):
                dic[name].set_value(value)
            else:
                Item.__setattr__(self, name, value)

    def __len__(self): return len(self.__items)
    def __nonzero__(self): return True
    def __getitem__(self, key): return self.__items_dict[key]

    def __setitem__(self, key, value):
        if self.__items_dict.has_key(key):
            return self.__items_dict[key].set(value)
        else:
            raise KeyError("no attribute '%s' in '%s'" % (key, self))

    def __iter__(self):
        return self.__items.__iter__()

    def has_item(self, name):
        return self.__items_dict.has_key(name)

    def copy_from(self, other):
        changed = Item.copy_from(self, other)
        first, common, second = dict_diff(self.__items_dict, other.__items_dict)
        if first or second:
            changed = True
        for id in first:
            self.remove_item(id)
        for id in common:
            changed = self[id].copy_from(other[id]) or changed
        for id in second:
            self.add_item(other[id].copy())
        return changed

    def get_value(self):
        if self.__not_set:
            return None
        else:
            return self

    def items(self): return self.__items
    def keys(self): return self.__items_dict.keys()

    def add_item(self, info, id=None):
        if id is None:
            id = info.get_id()
        if self.has_item(id):
            raise RuntimeError("item '%s' already exist in '%s'" % (id, self))
        item = create_instance(info, id)
        self.__items.append(item)
        self.__items_dict[id] = item

    def load(self, node):
        for c in node.children():
            if self.has_item(c.name):
                self[c.name].load(c)

    def save(self):
        children = []
        attrs = getattr(type(self), '__items__')

        for setting in self:
            children += setting.save()

        if not children:
            return []

        children.sort(_cmp_nodes)
        return [XMLGroup(self.get_id(), children)]

    def __eq__(self, other):
        if type(self) == type(other):
            for item in self:
                if item != other[item.get_id()]:
                    return False
            return True
        else:
            return False
    def __ne__(self, other):
        return not self.__eq__(other)
