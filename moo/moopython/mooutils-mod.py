"""moo.utils module"""
import _moo_utils as _utils
from _moo_utils import *


class _ActionFactory(object):
    def __init__(self, action_id, **kwargs):
        object.__init__(self)
        self.id = action_id
        self.kwargs = kwargs

    def __call__(self, window):
        self.window = window
        action = Action()
        action.set_property("id", self.id)

        for key in self.kwargs.keys():
            if key == "callback":
                action.connect("activate", self.action_activate)
            else:
                action.set_property(key, self.kwargs[key])

        return action

    def action_activate(self, action):
        self.kwargs["callback"](self.window)

def window_class_add_action(klass, action_id, **kwargs):
    _utils._window_class_add_action(klass, action_id, _ActionFactory(action_id, **kwargs))
