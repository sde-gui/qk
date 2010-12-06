#
#  mprj/project.py
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

"""Project class - the base class for all project types.

All project types subclass (or are method-compatible with) Project.

When project is loaded, the following happens:
1) The project file is parsed and appropriate project type is found.
2) Config instance is created using the project type's 'config' attribute.
3) Instance of this type is created with the editor window and config
as constructor arguments.
3) If no exception raised so far, project's load() method is called without
arguments. It's responsible for initializing gui, loading session, etc.

Note, only __init__ method may (must in case of errors) raise exceptions.
Project *must* handle errors well, otherwise gui may be left in inconsistent
state. E.g. presenting a dialog saying "Could not load session file" is fine
(though annoying), but raising OSError on trying to read nonexistant file is
not.

The closing sequence is the following:
1) Project's close() method is called. It's responsible for asking user if he
wants to save his files, saving session, etc. If it returns False, it means
user changed his mind and project is not closed. Otherwise,
2) project's unload() method is called. It must remove actions it installed,
unload plugins it loaded, etc., i.e. revert the editor state to the original.

After unload() is called, the project instance is considered dead, and not used
anymore.
"""

import moo
import os.path


class Project(object):

    """Base project type.

    Project types must satisfy the following requirements:
        1) The class must have 'config' attribute, which is the type of
        config used in this project.
        2) __init__ must have the same signature: __init__(window, file).
        3) Implement all the public methods of Project class (or just subclass it).

    """

    def __init__(self, window, config, file):
        object.__init__(self)

        self.window = window
        self.config = config
        self.name = file.name

        self.filename = os.path.abspath(file.path)
        self.topdir = os.path.dirname(self.filename)

        editor = moo.editor_instance()
        xml = editor.get_ui_xml()
        if xml is not None:
            self.merge_id = xml.new_merge_id()
        self.actions = []
        self.panes = []

    def add_action(self, id, **kwargs):
        moo.window_class_add_action(moo.EditWindow, id, **kwargs)
        self.actions.append(id)

    def init_ui(self):
        pass

    def deinit_ui(self):
        editor = moo.editor_instance()
        xml = editor.get_ui_xml()
        xml.remove_ui(self.merge_id)
        for a in self.actions:
            moo.window_class_remove_action(moo.EditWindow, a)
        if self.window:
            for p in self.panes:
                self.window.remove_pane(p)

    def load(self):
        self.init_ui()

    def close(self):
        return True

    def unload(self):
        self.deinit_ui()
