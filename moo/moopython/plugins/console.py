import moo
import gtk

CONSOLE_PLUGIN_ID = "Console"

class Plugin(object):
    def get_info(self):
        return {
            "id" : CONSOLE_PLUGIN_ID,
            "name" : "Console",
            "description" : "Console",
            "author" : "Yevgen Muntyan <muntyan@math.tamu.edu>",
            "version" : "3.1415926",
            "enabled" : True,
            "visible" : True
        }

    def init(self):
        editor = moo.edit.editor_instance()
        xml = editor.get_ui_xml()
        moo.utils.window_class_add_action (moo.edit.EditWindow, "ShowConsole",
                                           name="Show Console",
                                           label="Show Console",
                                           icon_stock_id=moo.utils.STOCK_TERMINAL,
                                           callback=self.show_console)
        self.ui_merge_id = xml.new_merge_id()
        xml.add_item(self.ui_merge_id, "Editor/Menubar/View",
                     "ShowConsole", "ShowConsole", -1)
        return True

    def deinit(self):
        editor = moo.edit.editor_instance()
        xml = editor.get_ui_xml()

        moo.utils.window_class_remove_action(moo.edit.EditWindow, "ShowConsole");

        if self.ui_merge_id:
            xml.remove_ui(self.ui_merge_id)
        self.ui_merge_id = 0

    def show_console(self, window):
        pane = window.get_pane(CONSOLE_PLUGIN_ID)
        window.paned.present_pane(pane)


class WinPlugin(object):
    def start(self, *whatever):
        if not self.terminal.child_alive():
            self.terminal.soft_reset()
            self.terminal.start_default_profile()

    def create(self, window):
        self.window = window

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

        window.add_pane(CONSOLE_PLUGIN_ID, swin, label, moo.utils.PANE_POS_BOTTOM)

        return True

    def destroy(self, window):
        window.remove_pane(CONSOLE_PLUGIN_ID)


moo.edit.plugin_register(Plugin, WinPlugin)
