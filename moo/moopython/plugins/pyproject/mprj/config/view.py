if __name__ == '__main__':
    import sys
    import os.path
    dir = os.path.dirname(__file__)
    sys.path.insert(0, os.path.join(dir, '../..'))

""" configview.py: TreeView column and cell renderers for settings """

import gtk
import gobject
import pango

from moo.utils import _

import mprj.config


""" TreeView column containing settings """
class Column(gtk.TreeViewColumn):
    def __init__(self, *args, **kwargs):
        gtk.TreeViewColumn.__init__(self, *args, **kwargs)

        self.__column = 0
        self.__cells = {}

    def __add_group(self, group):
        for item in group:
            if isinstance(item, mprj.config.Group):
                self.__add_group(item)
            else:
                self.__create_cell(item)

    def set_group(self, group):
        self.__add_group(group)

    def __create_cell(self, item):
        if not item.get_visible():
            return
        cell_types = item.get_cell_types()
        key = '+'.join([str(ct) for ct in cell_types])
        cells = self.__cells.get(key, [])
        if not self.__cells.has_key(key):
            for ct in cell_types:
                cell = gobject.new(ct, xalign=0)
                cell._set_column(self.__column)
                self.pack_start(cell)
                self.set_cell_data_func(cell, cell.cell_data_func, self.__column)
                cells.append(cell)
            self.__cells[key] = cells
        item.set_config_cells(cells)

    def set_model(self, model):
        for k in self.__cells:
            for c in self.__cells[k]:
                c._set_model(model)


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


class DictView(gtk.TreeView):
    def __init__(self):
        gtk.TreeView.__init__(self)

        self.dct = None

        self.column_name = gtk.TreeViewColumn(_('Name'))
        self.column_name.set_resizable(True)
        self.cell_name = gtk.CellRendererText()
        self.column_name.pack_start(self.cell_name, False)
        self.append_column(self.column_name)
        self.column_name.set_cell_data_func(self.cell_name, self.name_data_func)
        self.cell_name.set_property('editable', True)
        self.cell_name.connect('edited', self.name_edited)
        self.cell_name.connect('editing-started', self.editing_started, 0)

        self.column_value = gtk.TreeViewColumn(_('Value'))
        self.column_value.set_resizable(True)
        self.cell_value = gtk.CellRendererText()
        self.column_value.pack_start(self.cell_value, False)
        self.append_column(self.column_value)
        self.column_value.set_cell_data_func(self.cell_value, self.value_data_func)
        self.cell_value.set_property('editable', True)
        self.cell_value.connect('edited', self.value_edited)
        self.cell_value.connect('editing-started', self.editing_started, 1)

    def set_dict(self, dct):
        self.dct = dct
        store = gtk.ListStore(object)
        self.set_model(store)

        keys = dct.keys()
        keys.sort()
        for key in keys:
            store.append([[key, dct[key]]])

        store.append([[None, None]])

    def name_data_func(self, column, cell, model, iter):
        data = model.get_value(iter, 0)
        if data[0] is None:
            cell.set_property('text', _('new...'))
            cell.set_property('style', pango.STYLE_ITALIC)
#             cell.set_property('foreground', 'grey')
        else:
            cell.set_property('text', data[0])
            cell.set_property('style', pango.STYLE_NORMAL)
#             cell.set_property('foreground', 'black')

    def editing_started(self, cell, entry, path, ind):
        model = self.get_model()
        iter = model.get_iter(path)
        data = model.get_value(iter, 0)
        if data[ind] is None:
            entry.set_text('')

    def name_edited(self, cell, path, text):
        if not text:
            return
        model = self.get_model()
        iter = model.get_iter(path)
        if iter is None:
            return

        data = model.get_value(iter, 0)
        old_key = data[0]
        old_val = data[1]
        new_key = text

        if old_key == new_key:
            return
        if self.dct.has_key(new_key):
            raise KeyError()

        if old_val is None:
            old_val = ''
        self.dct[new_key] = old_val

        if old_key is not None:
            del self.dct[old_key]

        new_data = [new_key, self.dct[new_key]]
        model.set_value(iter, 0, new_data)

        if old_key is None:
            model.append([[None, None]])

    def value_data_func(self, column, cell, model, iter):
        data = model.get_value(iter, 0)
        if data[1] is None:
            cell.set_property('text', _('click to edit...'))
            cell.set_property('style', pango.STYLE_ITALIC)
#             cell.set_property('foreground', 'grey')
        else:
            cell.set_property('text', data[1])
            cell.set_property('style', pango.STYLE_NORMAL)
#             cell.set_property('foreground', 'black')

    def value_edited(self, cell, path, text):
        model = self.get_model()
        iter = model.get_iter(path)
        if iter is None:
            return
        data = model.get_value(iter, 0)
        if data[1] == text:
            return
        if data[0] is not None:
            self.dct[data[0]] = text
            data = [data[0], self.dct[data[0]]]
        else:
            if not text:
                text = None
            data = [data[0], text]
        model.set_value(iter, 0, data)

    def delete_activated(self, item, path):
        model = self.get_model()
        iter = model.get_iter(path)
        if not iter:
            return
        data = model.get_value(iter, 0)
        del self.dct[data[0]]
        model.remove(iter)

    def do_button_press_event(self, event):
        if event.button != 3:
            return gtk.TreeView.do_button_press_event(self, event)
        pos = self.get_path_at_pos(int(event.x), int(event.y))
        if pos is None:
            return gtk.TreeView.do_button_press_event(self, event)
        model = self.get_model()
        if model is None:
            return gtk.TreeView.do_button_press_event(self, event)
        iter = model.get_iter(pos[0])
        data = model.get_value(iter, 0)
        if data[0] is None:
            return gtk.TreeView.do_button_press_event(self, event)
        self.get_selection().select_iter(iter)
        menu = gtk.Menu()
        item = gtk.MenuItem(_("Delete"), False)
        item.show()
        item.connect('activate', self.delete_activated, pos[0])
        menu.add(item)
        menu.popup(None, None, None, event.button, event.time)

    def apply(self):
        pass


class GroupView(gtk.TreeView):
    def __init__(self):
        gtk.TreeView.__init__(self)

        self.items = None

        self.column_name = gtk.TreeViewColumn()
        self.column_name.set_resizable(True)
        self.cell_name = gtk.CellRendererText()
        self.column_name.pack_start(self.cell_name, False)
        self.append_column(self.column_name)
        self.column_name.set_cell_data_func(self.cell_name, self.name_data_func)

        self.column_value = Column()
        self.column_value.set_resizable(True)
        self.append_column(self.column_value)

        self.hidden_column = gtk.TreeViewColumn()
        self.hidden_column.set_visible(False)
        self.append_column(self.hidden_column)

    def set_group(self, group):
        self.column_value.set_group(group)
        self.__set_items(group.items())

    def __set_items(self, items):
        need_tree = False
        for i in items:
            if i.get_visible() and isinstance(i, mprj.config.Group):
                need_tree = True
                break

        self.items = items
        store = gtk.TreeStore(object)
        self.set_model(store)

        if need_tree:
            self.set_expander_column(self.column_name)
        else:
            self.set_expander_column(self.hidden_column)

        def append(store, items, parent):
            for item in items:
                iter = (parent and model.get_iter(parent)) or None
                iter = store.append(iter, [item])
                if isinstance(item, mprj.config.Group):
                    new_parent = store.get_path(iter)
                    append(store, item.items(), new_parent)
        append(store, items, None)
        self.column_value.set_model(store)

    def name_data_func(self, column, cell, model, iter):
        item = model.get_value(iter, 0)
        cell.set_property('text', item.get_name())

#     def value_data_func(self, column, cell, model, iter):
#         item = model.get_value(iter, 0)
#         if isinstance(item, mprj.config.Group):
#             cell.set_property('text', None)
#             cell.set_property('editable', False)
#         else:
#             cell.set_property('text', item.get_value())
#             cell.set_property('editable', True)

#     def value_edited(self, cell, path, text):
#         model = self.get_model()
#         iter = model.get_iter(path)
#         if iter is None:
#             return
#         item = model.get_value(iter, 0)
#         item.set_string(text)

    def apply(self):
        pass


class Entry(gtk.Entry):
    def __init__(self):
        gtk.Entry.__init__(self)
        self.setting = None

    def set_setting(self, setting):
        self.setting = setting
        if setting.get_value() is not None:
            self.set_text(setting.get_value())

    def apply(self):
        if self.setting is not None:
            self.setting.set_string(self.get_text())


gobject.type_register(Column)
gobject.type_register(View)
gobject.type_register(DictView)
gobject.type_register(GroupView)
gobject.type_register(Entry)


""" _CellMeta: metaclass for all settings cell renderers """
def _cell_data_func(dummy, column, cell, model, iter, index):
    item = model.get_value(iter, index)
    if not cell in item.get_config_cells():
        cell.set_property('visible', False)
    else:
        cell.set_property('visible', True)
        cell.set_data(item)

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


def CellTextN(idx):
    """ CellText: text renderer """
    class CellTextN(gtk.CellRendererText):
        __metaclass__ = _CellMeta

        def set_data(self, data):
            self.set_property('text', data.get_value()[idx])
            self.set_property('editable', data.get_editable())

        def do_edited(self, path, text):
            model = self.__model
            setting = model.get_value(model.get_iter(path), self.__column)
            value = list(setting.get_value())
            value[idx] = text
            setting.set_value(value)

    return CellTextN


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
