import os
import moo
import gtk
import gobject

CONSOLE_PLUGIN_ID = "Console"

class Plugin(moo.edit.Plugin):
    def do_init(self):
        editor = moo.edit.editor_instance()
        xml = editor.get_ui_xml()
        self.ui_merge_id = xml.new_merge_id()

        self.set_win_plugin_type(WinPlugin)
        moo.utils.window_class_add_action(moo.edit.EditWindow, "ShowConsole",
                                          display_name="Show Console",
                                          label="Show Console",
                                          stock_id=moo.utils.STOCK_TERMINAL,
                                          callback=self.show_console)
        xml.add_item(self.ui_merge_id, "Editor/Menubar/View", action="ShowConsole")

        return True

    def do_deinit(self):
        editor = moo.edit.editor_instance()
        xml = editor.get_ui_xml()
        xml.remove_ui(self.ui_merge_id)
        moo.utils.window_class_remove_action(moo.edit.EditWindow, "ShowConsole")

    def show_console(self, window):
        pane = window.get_pane(CONSOLE_PLUGIN_ID)
        window.paned.present_pane(pane)


class WinPlugin(moo.edit.WinPlugin):
    def start(self, *whatever):
        if not self.terminal.child_alive():
            self.terminal.soft_reset()
            self.terminal.start_default_shell()

    def do_create(self):
        label = moo.utils.PaneLabel(icon_stock_id = moo.utils.STOCK_TERMINAL,
                                    label = "Console",
                                    window_title = "Console")

        self.terminal = moo.term.Term()
        self.terminal.connect("child_died", self.start)
        self.start()

        swin = gtk.ScrolledWindow()
        swin.set_shadow_type(gtk.SHADOW_ETCHED_IN)
        swin.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        swin.add(self.terminal)
        swin.show_all()

        self.window.add_pane(CONSOLE_PLUGIN_ID, swin, label, moo.utils.PANE_POS_BOTTOM)

        return True

    def do_destroy(self):
        self.window.remove_pane(CONSOLE_PLUGIN_ID)


if os.name == 'posix':
    gobject.type_register(Plugin)
    gobject.type_register(WinPlugin)
    __plugin__ = Plugin
else:
    __plugin__ = None
