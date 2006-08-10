import gobject
import gtk
import pango
import moo
from moo.utils import _

import mprj.config


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

        self.column_value = gtk.TreeViewColumn()
        self.column_value.set_resizable(True)
        self.cell_value = gtk.CellRendererText()
        self.column_value.pack_start(self.cell_value, False)
        self.append_column(self.column_value)
        self.column_value.set_cell_data_func(self.cell_value, self.value_data_func)
        self.cell_value.connect('edited', self.value_edited)

        self.hidden_column = gtk.TreeViewColumn()
        self.hidden_column.set_visible(False)
        self.append_column(self.hidden_column)

    def set_group(self, group):
        self.set_items(group.items())

    def set_items(self, items):
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

    def name_data_func(self, column, cell, model, iter):
        item = model.get_value(iter, 0)
        cell.set_property('text', item.get_name())

    def value_data_func(self, column, cell, model, iter):
        item = model.get_value(iter, 0)
        if isinstance(item, mprj.config.Group):
            cell.set_property('text', None)
            cell.set_property('editable', False)
        else:
            cell.set_property('text', item.get_value())
            cell.set_property('editable', True)

    def value_edited(self, cell, path, text):
        model = self.get_model()
        iter = model.get_iter(path)
        if iter is None:
            return
        item = model.get_value(iter, 0)
        item.set_string(text)

#     def delete_activated(self, item, path):
#         model = self.get_model()
#         iter = model.get_iter(path)
#         if not iter:
#             return
#         data = model.get_value(iter, 0)
#         del self.dct[data[0]]
#         model.remove(iter)

#     def do_button_press_event(self, event):
#         if event.button != 3:
#             return gtk.TreeView.do_button_press_event(self, event)
#         pos = self.get_path_at_pos(int(event.x), int(event.y))
#         if pos is None:
#             return gtk.TreeView.do_button_press_event(self, event)
#         model = self.get_model()
#         if model is None:
#             return gtk.TreeView.do_button_press_event(self, event)
#         iter = model.get_iter(pos[0])
#         data = model.get_value(iter, 0)
#         if data[0] is None:
#             return gtk.TreeView.do_button_press_event(self, event)
#         self.get_selection().select_iter(iter)
#         menu = gtk.Menu()
#         item = gtk.MenuItem(_("Delete"), False)
#         item.show()
#         item.connect('activate', self.delete_activated, pos[0])
#         menu.add(item)
#         menu.popup(None, None, None, event.button, event.time)

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


# class BrowseButton(gtk.Button):
#     def connect(self, entry, topdir):
#         self.entry = entry
#
#     def do_clicked(self):
#         start_dir=self.entry.get_text()
#         if not start_dir:
#             start_dir = None
#         if start_dir and not os.path.isabs(start_dir):
#             start_dir = os.path.join()
#
#         path = moo.utils.file_dialog(moo.utils.FILE_DIALOG_OPEN_DIR,
#                                      parent=self,
#                                      start_dir=self.entry.get_text())
#         if path:
#             self.entry.set_text(path)


gobject.type_register(GroupView)
gobject.type_register(DictView)
gobject.type_register(Entry)
# gobject.type_register(BrowseButton)


if __name__ == '__main__':
    DictView()
