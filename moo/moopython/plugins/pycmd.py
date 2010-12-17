#
#  pycmd.py
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

import moo
import gobject
import gtk
from moo import _

class PyCmd(moo.Command):
    def __init__(self, code, options):
        moo.Command.__init__(self)
        if code and code[-1] != '\n' and code[-1] != '\r':
            self.code = code + '\n'
        else:
            self.code = code
        self.set_options(options)

    def do_run(self, ctx):
        dic = {}
        dic['doc'] = ctx.get_doc()
        dic['window'] = ctx.get_window()
        dic['buffer'] = ctx.get_doc() and ctx.get_doc().get_buffer()
        dic['editor'] = moo.editor_instance()
        dic['moo'] = moo

        buf = (ctx.get_doc() and ctx.get_doc().get_buffer()) or None

        if buf is not None:
            buf.begin_user_action()

        exc = None

        try:
            exec self.code in dic
        except Exception, e:
            exc = e

        if buf is not None:
            buf.end_user_action()

        if exc is not None:
            raise exc

class PyCmdFactory(moo.CommandFactory):
    def do_create_command(self, data, options):
        return PyCmd(data.get_code(), moo.parse_command_options(options))

    def do_create_widget(self):
        swin = gtk.ScrolledWindow()
        swin.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        swin.set_shadow_type(gtk.SHADOW_ETCHED_IN)
        textview = moo.TextView()
        swin.add(textview)
        swin.show_all()
        textview.set_font_from_string("Monospace")
        textview.set_lang_by_id("python")
        swin.textview = textview
        return swin

    def do_load_data(self, widget, data):
        code = data.get_code()
        if not code:
            code = ""
        elif not code.endswith("\n"):
            code = code + "\n"
        widget.textview.get_buffer().set_text(code)

    def do_save_data(self, widget, data):
        new_code = widget.textview.get_buffer().props.text or None
        old_code = data.get_code() or None
        if new_code != old_code:
            data.set_code(new_code)
            return True
        else:
            return False

gobject.type_register(PyCmd)
gobject.type_register(PyCmdFactory)
moo.command_factory_register("python", _("Python script"), PyCmdFactory(), None, ".py")
