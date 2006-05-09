"""moo.utils module"""
import _moo_utils as _utils
import gtk as _gtk
from _moo_utils import *


class ActionFactory(object):
    def __init__(self, action_id, **kwargs):
        object.__init__(self)
        self.id = action_id
        self.kwargs = kwargs

    def __call__(self, window):
        self.window = window
        action = _gtk.Action(self.id, None, None, None)
        self.set_action_properties(action)
        return action

    def set_action_properties(self, action):
        for key in self.kwargs.keys():
            if key == "callback":
                action.connect("activate", self.action_activate)
            elif key == "display_name":
                action_set_display_name(action, self.kwargs[key])
            elif key == "accel":
                action_set_default_accel(action, self.kwargs[key])
            elif key == "no_accel":
                action_set_no_accel(action, self.kwargs[key])
            else:
                action.set_property(key, self.kwargs[key])

    def action_activate(self, action):
        self.kwargs["callback"](self.window)


def window_class_add_action(klass, action_id, **kwargs):
    if kwargs.has_key("factory"):
        _utils._window_class_add_action(klass, action_id, kwargs["factory"])
    else:
        _utils._window_class_add_action(klass, action_id, ActionFactory(action_id, **kwargs))
