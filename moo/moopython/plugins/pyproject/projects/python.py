#
#  python.py
#
#  Copyright (C) 2004-2010 by Yevgen Muntyan <emuntyan@users.sourceforge.net>
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

if __name__ == '__main__':
    import sys
    import os.path
    dir = os.path.dirname(__file__)
    sys.path.insert(0, os.path.join(dir, '../..'))
    sys.path.insert(0, os.path.join(dir, '..'))

import gtk
import os.path
import gobject

import moo
from moo import _, N_

import medit
import mprj.utils
from mprj.simple import SimpleProject
from mprj.utils import print_error

import pyproj.config
from pyproj.config import PyConfig
from pyproj.optdialog import Dialog as OptionsDialog


_STOCK_EXECUTE = moo.STOCK_EXECUTE
_STOCK_PROJECT_OPTIONS = moo.STOCK_PROJECT_OPTIONS

_OUTPUT_PANE_ID   = "PythonOutput"
_CMD_EXECUTE      = 'execute'
_CMD_EXECUTE_FILE = 'execute_file'


class PyProject(SimpleProject):
    __config__ = PyConfig
    __factory_name__ = "Python"
    __factory_template__ = pyproj.config.factory_template

    class DoCmd(object):
        def __init__(self, proj, *args):
            object.__init__(self)
            self.proj = proj
            self.args = args
        def __call__(self, window):
            return self.proj.do_command(window, *self.args)

    def init_ui(self):
        SimpleProject.init_ui(self)

        self.panes.extend([_OUTPUT_PANE_ID])

        commands = [
            ["Execute", _("Execute Program"), _STOCK_EXECUTE, "F9", _CMD_EXECUTE],
            ["ExecuteFile", _("Execute File"), _STOCK_EXECUTE, "<shift>F9", _CMD_EXECUTE_FILE],
        ]

        for c in commands:
            self.add_action("PyProject" + c[0],
                            display_name=c[1], label=c[1],
                            stock_id=c[2], default_accel=c[3],
                            callback=PyProject.DoCmd(self, c[4]))

        editor = moo.Editor.instance()
        xml = editor.get_ui_xml()
        xml.insert_markup_after(self.merge_id, "Editor/Menubar",
                                "Project", """
                                <item name="Build" _label="%s">
                                  <item action="PyProjectExecute"/>
                                  <item action="PyProjectExecuteFile"/>
                                </item>
                                """ % (N_("_Build"),))
        xml.insert_markup(self.merge_id, "Editor/Toolbar/BuildToolbar",
                          0, """
                          <item action="PyProjectExecute"/>
                          <separator/>
                          """)

    def get_file_path(self, file):
        if os.path.exists(file):
            return file
        bd = self.config.get_build_dir(self.topdir)
        f = os.path.join(bd, file)
        if os.path.exists(f):
            return f
        f = os.path.join(self.topdir, file)
        if os.path.exists(f):
            return f
        return None

    def save_all(self, window):
        docs = window.list_docs()
        for d in docs:
            if d.get_filename() and d.get_status() & moo.EDIT_MODIFIED:
                d.save()

    def do_command(self, window, cmd):
        try:
            self.before_command(window, cmd) and \
            self.exec_command(window, cmd)   and \
            self.after_command(window, cmd)
        except Exception, e:
            mprj.utils.oops(window, e)


    def before_command(self, window, cmd):
        self.save_all(window)
        return True

    def after_command(self, window, cmd):
        return True

    def __cmd_execute(self, window):
        filename, args, working_dir = self.config.get_exe(self.topdir)
        r = medit.runpython.Runner(window, pane_id=_OUTPUT_PANE_ID)
        r.run(filename, args, working_dir)
        return True

    def __cmd_execute_file(self, window):
        r = medit.runpython.Runner(window, pane_id=_OUTPUT_PANE_ID)
        r.run()
        return True

    def exec_command(self, window, cmd):
        if cmd == _CMD_EXECUTE:
            return self.__cmd_execute(window)
        elif cmd == _CMD_EXECUTE_FILE:
            return self.__cmd_execute_file(window)
        else:
            mprj.utils.implement_me(window, "Command " + cmd)
            return False

    def create_options_dialog(self):
        return OptionsDialog(self)

__project__ = PyProject
__project_type__ = "Python"
__project_version__ = "2.0"
