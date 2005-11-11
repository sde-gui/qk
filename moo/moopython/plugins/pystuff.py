import moo
import gtk
import pango

SHOW_LOG_WINDOW = False

try:
    import pyconsole
    have_pyconsole = True
except ImportError:
    have_pyconsole = False

PLUGIN_ID = "PyStuff"

class Plugin(moo.edit.Plugin):
    def __init__(self):
        moo.edit.Plugin.__init__(self)

        self.info = {
            "id" : PLUGIN_ID,
            "name" : "Python Stuff",
            "description" : "Python stuff",
            "author" : "Yevgen Muntyan <muntyan@math.tamu.edu>",
            "version" : "3.1415926",
            "enabled" : True,
            "visible" : True
        }

        if SHOW_LOG_WINDOW:
            a = moo.edit.Plugin.ActionInfo(moo.edit.EditWindow, "ShowLogWindow",
                                           name="Show Log Window",
                                           label="Show Log Window",
                                           callback=self.show_log_window)
            self.actions.append(a)
            self.ui.append(moo.edit.Plugin.UIInfo("Editor/Menubar/Tools", "ShowLogWindow"))

        if have_pyconsole:
            a = moo.edit.Plugin.ActionInfo(moo.edit.EditWindow, "ShowPythonConsole",
                                           name="Show Python Console",
                                           label="Show Python Console",
                                           callback=self.show_console)
            self.actions.append(a)
            self.ui.append(moo.edit.Plugin.UIInfo("Editor/Menubar/Tools", "ShowPythonConsole"))

    def show_log_window(self, window):
        moo.app.get_instance().show_python_console()

    def show_console(self, window):
        window = gtk.Window()
        swin = gtk.ScrolledWindow()
        swin.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_ALWAYS)
        window.add(swin)

        console_type = pyconsole.ConsoleType(moo.edit.TextView)
        console = console_type(use_rlcompleter=False)
        console.set_property("highlight-current-line", False)
        editor = moo.edit.editor_instance()
        mgr = editor.get_lang_mgr()
        lang = mgr.get_lang("PythonConsole")
        console.set_lang(lang)
        console.modify_font(pango.FontDescription("Courier New 11"))

        swin.add(console)
        window.set_default_size(400,300)
        window.show_all()

moo.edit.plugin_register(Plugin)
# kate: indent-width 4; space-indent on
