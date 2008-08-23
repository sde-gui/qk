import os
import moo
import gtk
import gobject
from moo.utils import _

TERMINAL_PLUGIN_ID = "Terminal"
moo.utils.prefs_new_key_string('Plugins/Terminal/color_scheme', None)

class Plugin(moo.edit.Plugin):
    def do_init(self):
        editor = moo.edit.editor_instance()
        xml = editor.get_ui_xml()

        if xml is None:
            return False

        self.set_win_plugin_type(WinPlugin)
        return True

    def do_deinit(self):
        pass

    def show_terminal(self, window):
        pane = window.get_pane(TERMINAL_PLUGIN_ID)
        window.paned.present_pane(pane)

class WinPlugin(moo.edit.WinPlugin):
    def start(self, *whatever):
#         if not self.terminal.child_alive():
            self.terminal.reset(True, True)
            # XXX
            self.terminal.fork_command("/bin/sh", ["/bin/sh"])

    def color_scheme_item_activated(self, item, color_scheme):
        color_scheme.set_on_terminal(self.terminal)
        if color_scheme.colors:
            moo.utils.prefs_set_string('Plugins/Terminal/color_scheme', color_scheme.name)
        else:
            moo.utils.prefs_set_string('Plugins/Terminal/color_scheme', None)

    def terminal_populate_popup(self, terminal, menu):
        item = gtk.MenuItem("Color Scheme")
        submenu = gtk.Menu()
        item.set_submenu(submenu)
        for cs in color_schemes:
            child = gtk.MenuItem(cs.name)
            submenu.append(child)
            child.connect('activate', self.color_scheme_item_activated, cs)
        item.show_all()
        menu.append(item)

    def do_create(self):
        label = moo.utils.PaneLabel(icon_name=moo.utils.STOCK_TERMINAL,
                                    label_text=_("Terminal"))

        self.terminal = vte.Terminal()
        self.terminal.set_scrollback_lines(1000000)
        self.terminal.connect("child-exited", self.start)
#         self.terminal.connect("populate-popup", self.terminal_populate_popup)
        self.start()

        cs_name = moo.utils.prefs_get_string('Plugins/Terminal/color_scheme')
        cs = find_color_scheme(cs_name)
        if cs:
            cs.set_on_terminal(self.terminal)

        hbox = gtk.HBox()
        hbox.pack_start(self.terminal)
        scrollbar = gtk.VScrollbar(self.terminal.get_adjustment())
        hbox.pack_start(scrollbar, False, False, 0)
        hbox.show_all()

        self.terminal.set_size(self.terminal.get_column_count(), 10)
        self.terminal.set_size_request(10, 10)

        self.window.add_pane(TERMINAL_PLUGIN_ID, hbox, label, moo.utils.PANE_POS_BOTTOM)

        return True

    def do_destroy(self):
        self.window.remove_pane(TERMINAL_PLUGIN_ID)


class ColorScheme(object):
    def __init__(self, name, colors):
        object.__init__(self)
        self.name = name
        if colors is None:
            self.colors = None
        else:
            self.colors = [gtk.gdk.color_parse(c) for c in colors]

    def set_on_terminal(self, term):
        if self.colors is not None:
            term.set_colors(self.colors[0], self.colors[1], self.colors[2:10])

color_schemes = [ColorScheme(cs[0], cs[1]) for cs in [
    # Color schemes shamelessly stolen from Konsole, the best terminal emulator out there
    ["Default", None],
    ["Black on Light Yellow",
        ['#000000', '#ffffdd', '#000000', '#b21818', '#18b218', '#b26818', '#1818b2', '#b218b2', '#18b2b2', '#b2b2b2',
         '#000000', '#ffffdd', '#686868', '#ff5454', '#54ff54', '#ffff54', '#5454ff', '#ff54ff', '#54ffff', '#ffffff']],
    ["Black on White",
        ['#000000', '#ffffff', '#000000', '#b21818', '#18b218', '#b26818', '#1818b2', '#b218b2', '#18b2b2', '#b2b2b2',
         '#000000', '#ffffff', '#686868', '#ff5454', '#54ff54', '#ffff54', '#5454ff', '#ff54ff', '#54ffff', '#ffffff']],
    ["Marble",
        ['#ffffff', '#000000', '#000000', '#b21818', '#18b218', '#b26818', '#1818b2', '#b218b2', '#18b2b2', '#b2b2b2',
         '#ffffff', '#000000', '#686868', '#ff5454', '#54ff54', '#ffff54', '#5454ff', '#ff54ff', '#54ffff', '#ffffff']],
    ["Green on Black",
        ['#18f018', '#000000', '#000000', '#b21818', '#18b218', '#b26818', '#1818b2', '#b218b2', '#18b2b2', '#b2b2b2',
         '#18f018', '#000000', '#686868', '#ff5454', '#54ff54', '#ffff54', '#5454ff', '#ff54ff', '#54ffff', '#ffffff']],
    ["Paper, Light",
        ['#000000', '#ffffff', '#000000', '#b21818', '#18b218', '#b26818', '#1818b2', '#b218b2', '#18b2b2', '#b2b2b2',
         '#000000', '#ffffff', '#686868', '#ff5454', '#54ff54', '#ffff54', '#5454ff', '#ff54ff', '#54ffff', '#ffffff']],
    ["Paper",
        ['#000000', '#ffffff', '#000000', '#b21818', '#18b218', '#b26818', '#1818b2', '#b218b2', '#18b2b2', '#b2b2b2',
         '#000000', '#ffffff', '#686868', '#ff5454', '#54ff54', '#ffff54', '#5454ff', '#ff54ff', '#54ffff', '#ffffff']],
    ["Linux Colors",
        ['#b2b2b2', '#000000', '#000000', '#b21818', '#18b218', '#b26818', '#1818b2', '#b218b2', '#18b2b2', '#b2b2b2',
         '#ffffff', '#686868', '#686868', '#ff5454', '#54ff54', '#ffff54', '#5454ff', '#ff54ff', '#54ffff', '#ffffff']],
    ["VIM Colors",
        ['#000000', '#ffffff', '#000000', '#c00000', '#008000', '#808000', '#0000c0', '#c000c0', '#008080', '#c0c0c0',
         '#4d4d4d', '#ffffff', '#808080', '#ff6060', '#00ff00', '#ffff00', '#8080ff', '#ff40ff', '#00ffff', '#ffffff']],
    ["White on Black",
        ['#ffffff', '#000000', '#000000', '#b21818', '#18b218', '#b26818', '#1818b2', '#b218b2', '#18b2b2', '#b2b2b2',
         '#ffffff', '#000000', '#686868', '#ff5454', '#54ff54', '#ffff54', '#5454ff', '#ff54ff', '#54ffff', '#ffffff']]
]]

def find_color_scheme(name):
    for cs in color_schemes:
        if cs.name == name:
            return cs


__plugin__ = None

if os.name == 'posix':
    try:
        import vte
        gobject.type_register(Plugin)
        gobject.type_register(WinPlugin)
        __plugin__ = Plugin
    except ImportError:
        pass
