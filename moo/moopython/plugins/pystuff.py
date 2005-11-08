import moo
import gtk

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
        moo.utils.window_class_add_action (moo.edit.EditWindow, "ShowPythonConsole",
                                           name="Show Python Console",
                                           label="Show Python Console",
                                           callback=self.show_console)
        self.ui_merge_id = xml.new_merge_id()
        xml.add_item(self.ui_merge_id, "Editor/Menubar/Tools",
                     "ShowPythonConsole", "ShowPythonConsole", -1)
        return True

    def deinit(self):
        editor = moo.edit.editor_instance()
        xml = editor.get_ui_xml()
        moo.utils.window_class_remove_action(moo.edit.EditWindow, "ShowPythonConsole");
        xml.remove_ui(self.ui_merge_id)

    def show_console(self, window):
        moo.app.get_instance().show_python_console()


moo.edit.plugin_register(Plugin)
