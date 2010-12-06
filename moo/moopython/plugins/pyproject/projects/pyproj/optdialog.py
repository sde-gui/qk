#
#  pyproj/optdialog.py
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

if __name__ == '__main__':
    import sys
    import os.path
    dir = os.path.dirname(__file__)
    sys.path.insert(0, os.path.join(dir, '../../..'))
    sys.path.insert(0, os.path.join(dir, '../..'))
    sys.path.insert(0, os.path.join(dir, '..'))

import gobject
import os.path
import moo
from moo import _

import mprj.optdialog
from mprj.config.view import *
from pyproj.config import *

class RunOptionsPage(mprj.optdialog.ConfigPage):
    __label__ = _('Run options')
    __types__ = {'vars' : DictView,
                 'exe' : Entry,
                 'args' : Entry}

    def do_init(self):
        self.xml.w_vars.set_dict(self.config.run.vars)
        self.xml.w_exe.set_setting(self.config.run['exe'])
        self.xml.w_args.set_setting(self.config.run['args'])

class Dialog(moo.PrefsDialog):
    def __init__(self, project, title=_('Project Options')):
        moo.PrefsDialog.__init__(self, title)
        self.project = project
        self.config_copy = project.config.copy()
#         self.append_page(mprj.simple.ConfigPage(self.config_copy))
        glade_file = os.path.join(os.path.dirname(__file__), 'options.glade')
        self.append_page(RunOptionsPage('page_run', self.config_copy, glade_file))

    def do_apply(self):
        moo.PrefsDialog.do_apply(self)
        self.project.config.copy_from(self.config_copy)
        self.project.save_config()
#         print '============================='
#         print self.project.config.dump_xml()
#         print '============================='

gobject.type_register(RunOptionsPage)
gobject.type_register(Dialog)

if __name__ == '__main__':
    import gtk
    from pyproj.config import PyConfig, _sample_file
    from mprj.config import File
    from python import PyProject

    editor = moo.create_editor_instance()
    file = File(_sample_file, '/tmp/fake/file')
    config = PyConfig(file)
    project = PyProject(None, config, file)
    dialog = Dialog(project)
    dialog.connect('destroy', gtk.main_quit)
    dialog.run()
    gtk.main()
