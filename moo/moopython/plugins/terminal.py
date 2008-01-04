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
        if not self.terminal.child_alive():
            self.terminal.soft_reset()
            self.terminal.start_default_shell()

    def color_scheme_item_activated(self, item, color_scheme):
        self.terminal.set_colors(color_scheme.colors)
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

        self.terminal = moo.term.Term()
        self.terminal.connect("child-died", self.start)
        self.terminal.connect("populate-popup", self.terminal_populate_popup)
        self.start()

        cs_name = moo.utils.prefs_get_string('Plugins/Terminal/color_scheme')
        cs = find_color_scheme(cs_name)
        if cs:
            self.terminal.set_colors(cs.colors)

        swin = gtk.ScrolledWindow()
        swin.set_shadow_type(gtk.SHADOW_ETCHED_IN)
        swin.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        swin.add(self.terminal)
        swin.show_all()

        self.window.add_pane(TERMINAL_PLUGIN_ID, swin, label, moo.utils.PANE_POS_BOTTOM)

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
        import moo.term
        gobject.type_register(Plugin)
        gobject.type_register(WinPlugin)
        __plugin__ = Plugin
    except ImportError:
        pass
