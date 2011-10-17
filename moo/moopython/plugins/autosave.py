#
#  autosave.py
#
#  Dumb autosave plugin. It will save every open document every three minutes.
#

import gtk
import gobject
import glib
import os

import moo

AUTOSAVE_INTERVAL = 3 * 60 # 3 minutes, in seconds

class DocPlugin(moo.DocPlugin):
    def do_create(self):
        self.id_timeout = 0
        self.id_callback = self.get_doc().connect('after-save', self.on_after_save)
        self.__add_timeout()
        return True

    def do_destroy(self):
        self.get_doc().disconnect(self.id_callback)
        self.id_callback = 0
        self.__remove_timeout()
  
    def __remove_timeout(self):
        if self.id_timeout != 0:
            glib.source_remove(self.id_timeout)
            self.id_timeout = 0

    def __add_timeout(self):
        self.id_timeout = glib.timeout_add_seconds(AUTOSAVE_INTERVAL, self.on_timeout)

    def on_after_save(self, doc):
        self.__remove_timeout()
        self.__add_timeout()

    def on_timeout(self):
        self.__remove_timeout()
        if self.get_doc().get_uri() and self.get_doc().save():
            self.__add_timeout()
        return False

class Plugin(moo.Plugin):
    def do_init(self):
        self.set_doc_plugin_type(DocPlugin)
        return True

    def do_deinit(self):
        pass

gobject.type_register(Plugin)
gobject.type_register(DocPlugin)
__plugin__ = Plugin
