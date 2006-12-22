import os
import moo
import gtk
import gobject
from moo.utils import _

TERMINAL_PLUGIN_ID = "Terminal"

class Plugin(moo.edit.Plugin):
    def do_init(self):
        editor = moo.edit.editor_instance()
        xml = editor.get_ui_xml()

        if xml is None:
            return False

        self.ui_merge_id = xml.new_merge_id()

        self.set_win_plugin_type(WinPlugin)
        moo.utils.window_class_add_action(moo.edit.EditWindow, "ShowTerminal",
                                          display_name=_("Show Terminal"),
                                          label=_("Show Terminal"),
                                          stock_id=moo.utils.STOCK_TERMINAL,
                                          callback=self.show_terminal)
        xml.add_item(self.ui_merge_id, "Editor/Menubar/View", action="ShowTerminal")

        return True

    def do_deinit(self):
        editor = moo.edit.editor_instance()
        xml = editor.get_ui_xml()
        xml.remove_ui(self.ui_merge_id)
        moo.utils.window_class_remove_action(moo.edit.EditWindow, "ShowTerminal")

    def show_terminal(self, window):
        pane = window.get_pane(TERMINAL_PLUGIN_ID)
        window.paned.present_pane(pane)


class WinPlugin(moo.edit.WinPlugin):
    def start(self, *whatever):
        if not self.terminal.child_alive():
            self.terminal.soft_reset()
            self.terminal.start_default_shell()

    def do_create(self):
        label = moo.utils.PaneLabel(icon_stock_id = moo.utils.STOCK_TERMINAL,
                                    label=_("Terminal"),
                                    window_title=_("Terminal"))

        self.terminal = moo.term.Term()
        self.terminal.connect("child-died", self.start)
        self.start()

        swin = gtk.ScrolledWindow()
        swin.set_shadow_type(gtk.SHADOW_ETCHED_IN)
        swin.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        swin.add(self.terminal)
        swin.show_all()

        self.window.add_pane(TERMINAL_PLUGIN_ID, swin, label, moo.utils.PANE_POS_BOTTOM)

        return True

    def do_destroy(self):
        self.window.remove_pane(TERMINAL_PLUGIN_ID)


__plugin__ = None

if os.name == 'posix':
    try:
        import moo.term
        gobject.type_register(Plugin)
        gobject.type_register(WinPlugin)
        __plugin__ = Plugin
    except ImportError:
        pass
