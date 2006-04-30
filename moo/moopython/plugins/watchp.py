import moo
import gtk
import pango
import re

PLUGIN_ID = "WatchP"
window = None
notify_id = 0

class Plugin(moo.edit.Plugin):
    def __init__(self):
        moo.edit.Plugin.__init__(self)

        self.info = {
            "id" : PLUGIN_ID,
            "name" : "WatchP",
            "description" : "WatchP",
            "author" : "Yevgen Muntyan <muntyan@math.tamu.edu>",
            "version" : "3.1415926",
            "enabled" : True,
            "visible" : True
        }

        self.add_window_action(moo.edit.EditWindow, "ShowPrefsWindow",
                               name="Show Prefs Window",
                               label="Show Prefs Window",
                               callback=show_window)
        self.add_ui("ToolsMenu", "ShowPrefsWindow")


class PrefsWatcher(gtk.TreeView):
    def __init__(self):
        global notify_id

        gtk.TreeView.__init__(self)

        model = gtk.ListStore(str, object)
        self.set_model(model)

        column = gtk.TreeViewColumn()
        self.append_column(column)

        cell = gtk.CellRendererText()
        column.pack_start(cell, False)
        column.set_attributes(cell, text=0)

        cell = gtk.CellRendererText()
        column.pack_start(cell, True)
        column.set_cell_data_func(cell, self.print_value)

        notify_id = moo.utils.prefs_notify_connect(".*", moo.utils.PREFS_MATCH_REGEX,
                                                   self.prefs_changed, model)

    def prefs_changed(self, key, value, model):
        model.append([key, value])

    def print_value(self, column, cell, model, iter):
        value = model.get_value(iter, 1)
        return cell.set_property('text', repr(value))


def show_window(*whatever):
    global window

    if not window:
        window = gtk.Window()
        window.connect("destroy", window_destroyed)
        swin = gtk.ScrolledWindow()
        pwatch = PrefsWatcher()
        window.add(swin)
        swin.add(pwatch)
        window.show_all()
        window.present()

    window.present()

def window_destroyed(*whatever):
    global window, notify_id
    notify_id = 0
    window = None

moo.edit.plugin_register(Plugin)
# kate: indent-width 4; space-indent on
