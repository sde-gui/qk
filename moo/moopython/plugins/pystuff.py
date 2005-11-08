import moo
import gtk
import pango

try:
    import pyconsole
    have_pyconsole = True
except ImportError:
    have_pyconsole = False

PLUGIN_ID = "PyStuff"

class Plugin(object):
    def get_info(self):
        return {
            "id" : PLUGIN_ID,
            "name" : "Python Stuff",
            "description" : "Python stuff",
            "author" : "Yevgen Muntyan <muntyan@math.tamu.edu>",
            "version" : "3.1415926",
            "enabled" : True,
            "visible" : True
        }

    def init(self):
        editor = moo.edit.editor_instance()
        xml = editor.get_ui_xml()
        moo.utils.window_class_add_action (moo.edit.EditWindow, "ShowLogWindow",
                                           name="Show Log Window",
                                           label="Show Log Window",
                                           callback=self.show_log_window)
        self.ui_merge_id = xml.new_merge_id()
        xml.add_item(self.ui_merge_id, "Editor/Menubar/Tools",
                     "ShowLogWindow", "ShowLogWindow", -1)

        if have_pyconsole:
            moo.utils.window_class_add_action (moo.edit.EditWindow, "ShowPythonConsole",
                                               name="Show Python Console",
                                               label="Show Python Console",
                                               callback=self.show_console)
            xml.add_item(self.ui_merge_id, "Editor/Menubar/Tools",
                        "ShowPythonConsole", "ShowPythonConsole", -1)
        return True

    def deinit(self):
        editor = moo.edit.editor_instance()
        xml = editor.get_ui_xml()
        moo.utils.window_class_remove_action(moo.edit.EditWindow, "ShowLogWindow");
        moo.utils.window_class_remove_action(moo.edit.EditWindow, "ShowPythonConsole");
        xml.remove_ui(self.ui_merge_id)

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
        swin.set_size_request(400,300)
        window.show_all()

moo.edit.plugin_register(Plugin)
