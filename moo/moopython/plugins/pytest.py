#
#  pytest.py
#
#  Copyright (C) 2004-2010 by Yevgen Muntyan <emuntyan@sourceforge.net>
#
#  This file is part of medit.  medit is free software; you can
#  redistribute it and/or modify it under the terms of the
#  GNU Lesser General Public License as published by the
#  Free Software Foundation; either version 2.1 of the License,
#  or (at your option) any later version.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with medit.  If not, see <http://www.gnu.org/licenses/>.
#

import moo
import gobject

PLUGIN_ID = "TestPythonPlugin"

class Action(moo.Action):
    def __init__(self):
        moo.Action.__init__(self)
        self.set_property("label", "AnAction")
        #print "__init__"

    def do_check_state(self):
        #print "check_state"
        return True

    def do_activate(self):
        #print "activate: doc is", self.doc
        pass


class Plugin(moo.Plugin):

    def do_init(self):
        #print "do_init"
        return True

    def do_deinit(self):
        #print "do_deinit"
        pass

    def do_attach_win(self, window):
        #print "do_attach_win"
        pass
    def do_detach_win(self, window):
        #print "do_detach_win"
        pass
    def do_attach_doc(self, doc, window):
        #print "do_attach_doc"
        pass
    def do_detach_doc(self, doc, window):
        #print "do_detach_doc"
        pass


if moo.module_check_version(2, 0):
    gobject.type_register(Action)
    gobject.type_register(Plugin)

    info = moo.PluginInfo("RealPythonPlugin", "Real Python Plugin",
                          description="Plugin", author="Uknown",
                          version="3.1415926")
    moo.plugin_register(Plugin, info)
