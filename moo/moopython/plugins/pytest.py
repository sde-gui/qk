import moo
import gobject

PLUGIN_ID = "TestPythonPlugin"

class Action(moo.edit.Action):
    def __init__(self):
        moo.edit.Action.__init__(self)
        self.set_property("label", "AnAction")
        #print "__init__"

    def do_check_state(self):
        #print "check_state"
        return True

    def do_activate(self):
        #print "activate: doc is", self.doc
        pass

class Plugin(moo.edit.Plugin):
    def __init__(self):
        moo.edit.Plugin.__init__(self)

        #print '__init__'

        self.info = {
            "id" : PLUGIN_ID,
            "name" : "Test python plugin",
            "description" : "Test python plugin",
            "author" : "Yevgen Muntyan <muntyan@math.tamu.edu>",
            "version" : "3.1415926",
            "enabled" : True,
            "visible" : True,
            #"langs" : "c",
        }

        self.add_edit_action("AnAction", Action)
        self.add_ui("Editor/Popup", "AnAction")

        #print '__init__ done'

    def attach_doc(self, doc, window):
        #print "attaching to", doc
        pass

    def detach_doc(self, doc, window):
        #print "detaching from", doc
        pass

    def attach_win(self, window):
        #print "attaching to", window
        pass

    def detach_win(self, window):
        #print "detaching from", window
        pass

gobject.type_register(Action)
moo.edit.plugin_register(Plugin)
