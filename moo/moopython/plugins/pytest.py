import moo
import gobject

UIInfo = moo.edit.Plugin.UIInfo

PLUGIN_ID = "TestPythonPlugin"

class Action(moo.edit.Action):
    def __init__(self):
        moo.edit.Action.__init__(self)
        self.set_property("label", "AnAction")
        print "__init__"

    def do_check_state(self):
        print "check_state"
        return True

class Plugin(moo.edit.Plugin):
    def __init__(self):
        moo.edit.Plugin.__init__(self)

        self.info = {
            "id" : PLUGIN_ID,
            "name" : "Test python plugin",
            "description" : "Test python plugin",
            "author" : "Yevgen Muntyan <muntyan@math.tamu.edu>",
            "version" : "3.1415926",
            "enabled" : True,
            "visible" : True,
            "langs" : "c",
        }

        moo.edit.edit_class_add_action(moo.edit.Edit, "AnAction", Action)
        self.ui.append(UIInfo("Editor/Popup", "AnAction"))

    def attach_doc(self, doc, window):
        print "attaching to", doc

    def detach_doc(self, doc, window):
        print "detaching from", doc

gobject.type_register(Action)
moo.edit.plugin_register(Plugin)
