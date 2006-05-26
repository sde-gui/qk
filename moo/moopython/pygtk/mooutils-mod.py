"""moo.utils module"""
import _moo_utils as _utils
import gtk as _gtk
import gobject as _gobject
from _moo_utils import *


def _activate(action, callback, window):
    callback(window)

class ActionFactory(object):
    def __init__(self, action_id, **kwargs):
        object.__init__(self)
        self.id = action_id

        self.props = {}
        self.fake_props = {}

        for key in kwargs.keys():
            if key in ["callback", "display_name", "accel", "no_accel"]:
                self.fake_props[key] = kwargs[key]
            elif kwargs[key] is not None or key not in ["stock_id"]:
                self.props[key] = kwargs[key]

    def __call__(self, window):
        self.window = window
        if _gtk.check_version(2, 8, 0):
            action = _gtk.Action(self.id, None, None, None)
            self.set_props(action)
        else:
            action = _gobject.new(_gtk.Action, name=self.id, **self.props)
        self.set_fake_props(action)
        return action

    def set_fake_props(self, action):
        for key in self.fake_props.keys():
            if key == "callback":
                action.connect("activate", _activate, self.fake_props[key], self.window)
            elif key == "display_name":
                action_set_display_name(action, self.fake_props[key])
            elif key == "accel":
                action_set_default_accel(action, self.fake_props[key])
            elif key == "no_accel":
                action_set_no_accel(action, self.fake_props[key])
            else:
                raise ValueError("unknown property " + key)

    def set_props(self, action):
        for key in self.props.keys():
            action.set_property(key, self.props[key])



def window_class_add_action(klass, action_id, **kwargs):
    if kwargs.has_key("factory"):
        _utils._window_class_add_action(klass, action_id, kwargs["factory"])
    else:
        _utils._window_class_add_action(klass, action_id, ActionFactory(action_id, **kwargs))
