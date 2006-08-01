import moo
import sys
import traceback
import gobject
from mproj.manager import Manager

_PLUGIN_ID = "ProjectManager"

class _Plugin(moo.edit.Plugin):
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


if __name__.startswith("moo_plugin_") and moo.edit.module_check_version(2, 0):
    #gobject.type_register(_Plugin)
    info = moo.edit.PluginInfo(_PLUGIN_ID, "Project Manager",
                               description="Project Manager",
                               author="Yevgen Muntyan <muntyan@math.tamu.edu>",
                               version="3.1415926")
    params = moo.edit.PluginParams(False, False)
    moo.edit.plugin_register(_Plugin, info, params)
