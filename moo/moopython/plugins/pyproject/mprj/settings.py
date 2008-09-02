""" settings.py: basic Setting subclasses """

from mprj.config import Setting, Group, Dict, Item
from mprj.config.view import *
from mprj.config._xml import XMLItem, XMLGroup

def _cmp_nodes(n1, n2):
    return (n1.name < n2.name and -1) or (n1.name > n2.name and 1) or 0

class String(Setting):
    __item_cell_types__ = CellText

    def transform_value(self, value):
        if value is None or isinstance(value, str):
            return value
        if isinstance(value, unicode):
            return str(value)
        raise TypeError('value %s of type %s is invalid for %s', value, type(value), self)

    def get_string(self):
        return self.get_value()
    def set_string(self, text):
        self.set_value(text)

Filename = String

class Command(Setting):
    __item_cell_types__ = [CellTextN(1), CellTextN(0)]

    def copy_from(self, other):
        return Setting.copy_from(self, other)

    def set_value(self, value):
        if len(value) != 2:
            raise ValueError("invalid Command value %s" % (value,))
        return Setting.set_value(self, value)

    def load(self, node):
        cmd = node.get_child('cmd').get()
        working_dir = node.get_child('working_dir').get()
        return self.set_value([working_dir, cmd])

    def save(self):
        if not self.is_default():
            value = self.get_value()
            items = [XMLItem('working_dir', value[0]), XMLItem('cmd', value[1])]
            return [XMLGroup(self.get_id(), items)]
        else:
            return []

class Bool(Setting):
    __item_data_type__ = bool
    __item_cell_types__ = CellToggle

    def get_bool(self):
        return self.get_value()

    def set_string(self, text):
        self.set_value(bool(text))

class Int(Setting):
    __item_data_type__ = int
    __item_cell_types__ = CellText

    def get_int(self):
        return self.get_value()
    def check_value(self, value):
        try:
            value = int(value)
            return True
        except:
            return False
    def set_string(self, text):
        self.set_value(int(text))

if __name__ == '__main__':
    import gtk
    import gobject
    window = gtk.Window()
    window.set_size_request(300,200)
    window.connect('destroy', gtk.main_quit)

    group = Group.create_instance('ddd')
    s = String.create_instance('blah', value='111')
    group.add_item(s)
    group.blah = '8'
    print s.get_value()
    print s is group.blah
    group.add_item(String.create_instance('foo', value='foofoofoofoofoofoofoofoo'))
    group.add_item(Bool.create_instance('fff', value=True))

    view = View(group)
    window.add(view)
    window.show_all()
    gtk.main()
