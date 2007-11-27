import moo
import gobject
import gtk
from moo.utils import _

class PyCmd(moo.edit.Command):
    def __init__(self, code, options):
        moo.edit.Command.__init__(self)
        if code and code[-1] != '\n' and code[-1] != '\r':
            self.code = code + '\n'
        else:
            self.code = code
        self.set_options(options)

    def __set_variable(self, name, value, dic):
        dic[name] = value

    def do_run(self, ctx):
        dic = {}
        dic['doc'] = ctx.get_doc()
        dic['window'] = ctx.get_window()
        dic['buffer'] = ctx.get_doc() and ctx.get_doc().get_buffer()
        dic['editor'] = moo.edit.editor_instance()
        dic['moo'] = moo

        ctx.foreach(self.__set_variable, dic)

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

class PyCmdFactory(moo.edit.CommandFactory):
    def do_create_command(self, data, options):
        return PyCmd(data.get_code(), moo.edit.parse_command_options(options))

    def do_create_widget(self):
        swin = gtk.ScrolledWindow()
        swin.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        swin.set_shadow_type(gtk.SHADOW_ETCHED_IN)
        textview = moo.edit.TextView()
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
moo.edit.command_factory_register("python", _("Python script"), PyCmdFactory(), None, ".py")
