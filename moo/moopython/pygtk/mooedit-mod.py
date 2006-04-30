"""moo.edit module

Contains text editor and stuff.

To get an instance of Editor, the object which manages document instances,
use editor_instance().
To create new or open existing document, use Editor.create_doc().
To save or close document use Edit.save() and Edit.close().
To find out status of document (unsaved, deleted from disk, etc.) use
Edit.get_status, it returns flags from EditStatus class.

Note that by default Editor shows alerts in various places, like when
user tries to close document with unsaved changes. To disable this,
use 'silent' property: editor.set_property("silent", True).
"""
import _moo_edit as _edit
from _moo_edit import *

def edit_class_add_action(cls, action_id, action_class):
    _edit._edit_class_add_action(cls, action_id, action_class)

class _UIInfo(object):
    def __init__(self, parent, action, name=None, index=-1):
        self.parent = parent
        self.action = action
        self.name = name or action
        self.index = index

class _WindowActionInfo(object):
    def __init__(self, window_type, id, **kwargs):
        self.window_type = window_type
        self.id = id
        self.props = kwargs

class _EditActionInfo(object):
    def __init__(self, id, action_type, edit_type = Edit):
        self.id = id
        self.action_type = action_type
        self.edit_type = edit_type

class Plugin(object):
    def __init__(self):
        object.__init__(self)

        self.info = {
            "id" : "Plugin",
            "name" : "Plugin",
            "description" : "Plugin",
            "author" : "Uknown",
            "version" : "3.1415926",
            "enabled" : True,
            "visible" : True
        }

        self._window_actions = []
        self._edit_actions = []
        self._ui_merge_id = 0
        self._ui = []

    def get_info(self):
        return self.info

    def init(self):
        import moo

        for a in self._window_actions:
            moo.utils.window_class_add_action(a.window_type, a.id, **a.props)

        for a in self._edit_actions:
            edit_class_add_action(a.edit_type, a.id, a.action_type)

        if self._ui:
            editor = editor_instance()
            xml = editor.get_ui_xml()
            self._ui_merge_id = xml.new_merge_id()
            for item in self._ui:
                xml.add_item(self._ui_merge_id, item.parent,
                             item.name, item.action, item.index)

        return True

    def deinit(self):
        import moo

        if self._ui_merge_id:
            editor = editor_instance()
            xml = editor.get_ui_xml()
            xml.remove_ui(self._ui_merge_id)
        self._ui_merge_id = 0

        for a in self._window_actions:
            moo.utils.window_class_remove_action(a.window_type, a.id)

        for a in self._edit_actions:
            edit_class_remove_action(a.edit_type, a.id)

    """ add_ui(self, parent, action, name=None, index=-1) -> None"""
    def add_ui(self, *args, **kwargs):
        self._ui.append(_UIInfo(*args, **kwargs))

    """ add_edit_action(self, id, action_type, edit_type = Edit) -> None"""
    def add_edit_action(self, *args, **kwargs):
        self._edit_actions.append(_EditActionInfo(*args, **kwargs))

    """ add_window_action(self, window_type, id, **kwargs) -> None"""
    def add_window_action(self, *args, **kwargs):
        self._window_actions.append(_WindowActionInfo(*args, **kwargs))
