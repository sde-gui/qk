import os
import moo
import gtk

CONSOLE_PLUGIN_ID = "Console"

class Plugin(moo.edit.Plugin):
    def __init__(self):
        moo.edit.Plugin.__init__(self)

        self.info = {
            "id" : CONSOLE_PLUGIN_ID,
            "name" : "Console",
            "description" : "Console",
            "author" : "Yevgen Muntyan <muntyan@math.tamu.edu>",
            "version" : "3.1415926",
            "enabled" : True,
            "visible" : True
        }

        self.add_window_action(moo.edit.EditWindow, "ShowConsole",
                               display_name="Show Console",
                               label="Show Console",
                               stock_id=moo.utils.STOCK_TERMINAL,
                               callback=self.show_console)
        self.add_ui("Editor/Menubar/View", "ShowConsole")

    def show_console(self, window):
        pane = window.get_pane(CONSOLE_PLUGIN_ID)
        window.paned.present_pane(pane)


class WinPlugin(object):
    def start(self, *whatever):
        if not self.terminal.child_alive():
            self.terminal.soft_reset()
            self.terminal.start_default_shell()

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


if os.name == 'posix':
    moo.edit.plugin_register(Plugin, WinPlugin)
