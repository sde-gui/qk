import moo
import sys
import traceback
import gobject
from mprj.manager import Manager

class __plugin__(moo.edit.Plugin):
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
