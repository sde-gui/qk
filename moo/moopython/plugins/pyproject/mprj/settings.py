""" settings.py: basic Setting subclasses """

from mprj.config import Setting, Group, Dict, Item
from mprj.config.view import *
from mprj.config._xml import XMLItem, XMLGroup

def _cmp_nodes(n1, n2):
    return (n1.name < n2.name and -1) or (n1.name > n2.name and 1) or 0

class String(Setting):
    def get_cell_type(self):
        return CellText
    def get_string(self):
        return self.get_value()
    def set_string(self, text):
        self.set_value(text)

class Bool(Setting):
    def get_cell_type(self):
        return CellToggle
    def get_bool(self):
        return self.get_value()
    def set_string(self, text):
        self.set_value(bool(text))

class Int(Setting):
    def get_cell_type(self):
        return CellText
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
    group.blah = 8
    print s.get_value()
    print s is group.blah
    group.add_item(String.create_instance('foo', value='foofoofoofoofoofoofoofoo'))
    group.add_item(Bool.create_instance('fff', value=True))

    view = View(group)
    window.add(view)
    window.show_all()
    gtk.main()
