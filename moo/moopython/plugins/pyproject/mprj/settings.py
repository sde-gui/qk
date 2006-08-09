""" settings.py: basic Setting subclasses """

from mprj.config import Setting, Group, List
from mprj.config._view import *
from mprj.config._xml import XMLItem, XMLGroup

def _cmp_nodes(n1, n2):
    return (n1.name < n2.name and -1) or (n1.name > n2.name and 1) or 0

class String(Setting):
    def get_cell_type(self):
        return mprj.configview.CellText
    def get_string(self):
        return self.get()
    def set_string(self, text):
        self.set(text)

class Bool(Setting):
    def get_cell_type(self):
        return mprj.configview.CellToggle
    def get_bool(self):
        return self.get()
    def set_string(self, text):
        self.set(bool(text))

class Int(Setting):
    def get_cell_type(self):
        return mprj.configview.CellText
    def get_int(self):
        return self.get()
    def check_value(self, value):
        try:
            value = int(value)
            return True
        except:
            return False
    def set_string(self, text):
        self.set(int(text))

class Dict(Setting, dict):
    def load(self, node):
        for c in node.children():
            self[c.name] = c.get()
    def save(self):
        nodes = []
        for k in self:
            val = self[k]
            if val is not None:
                nodes.append(XmlItem(k, val))
        if nodes:
            nodes.sort(_cmp_nodes)
            return [XMLGroup(self.get_id(), nodes)]
        else:
            return []
    def copy(self):
        copy = Dict()
        for key in self:
            copy[key] = self[key]
        return copy

if __name__ == '__main__':
    import gtk
    import gobject
    window = gtk.Window()
    window.set_size_request(300,200)
    window.connect('destroy', gtk.main_quit)

    group = mprj.config.Group('ddd')
    s = String('blah', value='111')
    group.add_item(s)
    group.blah = 8
    print s.get()
    print s is group.blah
    group.add_item(String('foo', value='foofoofoofoofoofoofoofoo'))
    group.add_item(Bool('fff', value=True))

    view = mprj.configview.View(group)
    window.add(view)
    window.show_all()
    gtk.main()
