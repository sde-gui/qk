__all__ = ['Group']

from mprj.config._item import Item, _ItemMeta, create_instance
from mprj.config._xml import XMLGroup


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

    def __init__(self, id, items={}, **kwargs):
        Item.__init__(self, id, **kwargs)

        self.__items = []
        self.__items_dict = {}

        if items:
            for id in items:
                self.add_item(id, items[id])

        if hasattr(type(self), '__items__'):
            items = getattr(type(self), '__items__')
            for id in items:
                self.add_item(id, items[id])

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
        Item.copy_from(self, other)
        for item in self:
            item.copy_from(other[item.get_id()])

    def get_value(self):
        return self

    def get_items(self):
        return self.__items

    def add_item(self, id, info):
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
