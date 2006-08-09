__all__ = ['Column', 'View', 'CellText', 'CellToggle']

""" configview.py: TreeView column and cell renderers for settings """

import gtk
import gobject
import mprj.config


""" TreeView column containing settings """
class Column(gtk.TreeViewColumn):
    def __init__(self, group, column, *args, **kwargs):
        gtk.TreeViewColumn.__init__(self, *args, **kwargs)

        self.__column = column
        self.__cells = {}

        self.__add_group(group)

    def __add_group(self, group):
        for item in group:
            if isinstance(item, mprj.config.Group):
                self.__add_group(item)
            else:
                self.__create_cell(item)

    def __create_cell(self, item):
        if not item.get_visible():
            return
        cell_type = item.get_cell_type()
        if self.__cells.has_key(cell_type):
            return
        cell = gobject.new(item.get_cell_type(), xalign=0)
        cell._set_column(self.__column)
        self.pack_start(cell)
        self.set_cell_data_func(cell, cell.cell_data_func, self.__column)
        self.__cells[cell_type] = cell

    def set_model(self, model):
        for k in self.__cells:
            self.__cells[k]._set_model(model)


""" TreeView containing settings """
class View(gtk.TreeView):
    def __init__(self, group):
        gtk.TreeView.__init__(self)
        self.set_property('headers_visible', False)

        self.__group = group.copy()
        self.__create_model()

        self.column_name = gtk.TreeViewColumn()
        cell = gtk.CellRendererText()
        self.column_name.pack_start(cell)
        self.column_name.set_cell_data_func(cell, self.__name_data_func)
        self.append_column(self.column_name)

        self.column_data = Column(self.__group, 0)
        self.column_data.set_model(self.get_model())
        self.append_column(self.column_data)

    def get_group(self):
        return self.__group

    def __create_model(self):
        need_tree = False
        for item in self.__group:
            if isinstance(item, mprj.config.Group):
                need_tree = True
                break
        if need_tree:
            model = self.__create_tree()
        else:
            model = self.__create_list()
        self.set_model(model)

    def __create_tree(self):
        tree = gtk.TreeStore(object)
        def append(model, group, parent):
            for item in self.__group:
                iter = (parent and model.get_iter(parent)) or None
                iter = model.append(iter, [item])
                if isinstance(item, mprj.Config.Group):
                    new_parent = model.get_path(iter)
                    append(model, item, new_parent)
        append(tree, self.__group, None)
        return tree

    def __create_list(self):
        list = gtk.ListStore(object)
        for item in self.__group:
            list.append([item])
        return list

    def __name_data_func(self, column, cell, model, iter):
        setting = model.get_value(iter, 0)
        cell.set_property('text', setting.get_name())

gobject.type_register(Column)
gobject.type_register(View)


""" _CellMeta: metaclass for all settings cell renderers """
def _cell_data_func(dummy, column, cell, model, iter, index):
    data = model.get_value(iter, index)
    if data.get_cell_type() != type(cell):
        cell.set_property('visible', False)
    else:
        cell.set_property('visible', True)
        cell.set_data(data)

def _set_private(clsname, obj, attr, value):
    setattr(obj, "_%s%s" % (clsname, attr), value)

class _CellMeta(gobject.GObjectMeta):
    def __init__(cls, name, bases, dic):
        super(_CellMeta, cls).__init__(name, bases, dic)

        if not dic.has_key('_set_column'):
            def set_column(cell, column):
                _set_private(name, cell, '__column', column)
            setattr(cls, '_set_column', set_column)

        if not dic.has_key('_set_model'):
            def set_model(cell, model):
                _set_private(name, cell, '__model', model)
            setattr(cls, '_set_model', set_model)

        if not dic.has_key('cell_data_func'):
            setattr(cls, 'cell_data_func', _cell_data_func)

        gobject.type_register(cls)


""" CellText: text renderer """
class CellText(gtk.CellRendererText):
    __metaclass__ = _CellMeta

    def set_data(self, data):
        self.set_property('text', data.get_string())
        self.set_property('editable', data.get_editable())

    def do_edited(self, path, text):
        model = self.__model
        setting = model.get_value(model.get_iter(path), self.__column)
        setting.set_string(text)


""" CellToggle: toggle renderer """
class CellToggle(gtk.CellRendererToggle):
    __metaclass__ = _CellMeta

    def set_data(self, data):
        self.set_property('active', data.get_bool())
        self.set_property('activatable', data.get_editable())

    def do_toggled(self, path):
        model = self.__model
        setting = model.get_value(model.get_iter(path), self.__column)
        setting.set_value(not setting.get_bool())
