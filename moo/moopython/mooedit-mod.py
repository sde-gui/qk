"""moo.edit module"""
from _moo_edit import *

class Plugin(object):
    class ActionInfo(object):
        def __init__(self, window_type, id, **kwargs):
            self.window_type = window_type
            self.id = id
            self.props = kwargs

    class UIInfo(object):
        def __init__(self, parent, action, name=None, index=-1):
            self.parent = parent
            self.action = action
            self.name = name or action
            self.index = index

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

        self.actions = []
        self.ui_merge_id = 0
        self.ui = []

    def get_info(self):
        return self.info

    def init(self):
        import moo

        for a in self.actions:
            moo.utils.window_class_add_action(a.window_type, a.id, **a.props)

        if self.ui:
            editor = editor_instance()
            xml = editor.get_ui_xml()
            self.ui_merge_id = xml.new_merge_id()
            for item in self.ui:
                xml.add_item(self.ui_merge_id, item.parent,
                             item.name, item.action, item.index)

        return True

    def deinit(self):
        import moo
        for a in self.actions:
            moo.utils.window_class_remove_action(a.window_type, a.id)
        if self.ui_merge_id:
            editor = editor_instance()
            xml = editor.get_ui_xml()
            xml.remove_ui(self.ui_merge_id)
        self.ui_merge_id = 0
