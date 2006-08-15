"""moo.utils module"""
import __moo_utils__ as _utils
import gtk as _gtk
import gobject as _gobject
from __moo_utils__ import *

_ = gettext

def N_(string):
    return string

def _activate(action, callback, window):
    callback(window)

class ActionFactory(object):
    def __init__(self, action_id, **kwargs):
        object.__init__(self)
        self.id = action_id

        self.props = {}
        self.fake_props = {}

        for key in kwargs.keys():
            if key in ["callback"]:
                self.fake_props[key] = kwargs[key]
            elif kwargs[key] is not None or key not in ["stock_id"]:
                self.props[key] = kwargs[key]

    def __call__(self, window):
        self.window = window
        if _gtk.check_version(2, 8, 0):
            action = Action(self.id)
            self.set_props(action)
        else:
            action = _gobject.new(Action, name=self.id, **self.props)
        self.set_fake_props(action)
        return action

    def set_fake_props(self, action):
        for key in self.fake_props.keys():
            if key == "callback":
                action.connect("activate", _activate, self.fake_props[key], self.window)
            else:
                raise ValueError("unknown property " + key)

    def set_props(self, action):
        for key in self.props.keys():
            action.set_property(key, self.props[key])



def window_class_add_action(klass, action_id, group=None, **kwargs):
    if kwargs.has_key("factory"):
        _utils._window_class_add_action(klass, action_id, group, kwargs["factory"])
    else:
        _utils._window_class_add_action(klass, action_id, group, ActionFactory(action_id, **kwargs))
