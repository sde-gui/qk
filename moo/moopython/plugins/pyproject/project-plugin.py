#
#  project-plugin.py
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
import sys
import traceback
import gobject
from mprj.manager import Manager

class __plugin__(moo.Plugin):
    __gproperties__ = { 'project' : (str, 'project to open', 'project to open', None, gobject.PARAM_READWRITE) }

    def do_set_property(self, prop, value):
        self.project_to_open = value

    def do_init(self):
        project = None
        if hasattr(self, "project_to_open"):
            project = self.project_to_open
        self.mgr = Manager(project)
        return True

    def do_deinit(self):
        self.mgr.deinit()
        del self.mgr

    def do_attach_win(self, window):
        self.mgr.attach_win(window)

    def do_detach_win(self, window):
        self.mgr.detach_win(window)
