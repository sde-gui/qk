import gtk
import moo

import mproj.utils
from mproj.simple import SimpleProject
from mproj.utils import print_error

from ltxproj.config import LatexConfig


_STOCK_LATEX    = moo.utils.STOCK_BUILD
_STOCK_VIEW_DVI = moo.utils.STOCK_COMPILE

_CMD_LATEX      = 'latex'
_CMD_VIEW_DVI   = 'viewdvi'


class LatexProject(SimpleProject):
    __config__ = LatexConfig

    class DoCmd(object):
        def __init__(self, obj, *args):
            object.__init__(self)
            self.obj = obj
            self.args = args
        def __call__(self, window):
            return self.obj.do_command(window, *self.args)

    def init_ui(self):
        SimpleProject.init_ui(self)

        commands = [
            ["Latex", "LaTeX", _STOCK_LATEX, "F8", _CMD_LATEX],
            ["ViewDvi", "View Dvi", _STOCK_VIEW_DVI, "F9", _CMD_VIEW_DVI],
        ]

        for c in commands:
            self.add_action("LatexProject" + c[0],
                            display_name=c[1], label=c[1],
                            stock_id=c[2], accel=c[3],
                            callback=LatexProject.DoCmd(self, c[4]))

        editor = moo.edit.editor_instance()
        xml = editor.get_ui_xml()
        xml.insert_markup_after(self.merge_id, "Editor/Menubar",
                                "Project", """
                                <item name="Latex" label="_LaTeX">
                                  <item action="LatexProjectLatex"/>
                                  <item action="LatexProjectViewDvi"/>
                                </item>
                                """)
        xml.insert_markup(self.merge_id, "Editor/Toolbar/BuildToolbar",
                          0, """
                          <item action="LatexProjectLatex"/>
                          <item action="LatexProjectViewDvi"/>
                          <separator/>
                          """)

    def save_all(self, window):
        docs = window.list_docs()
        for d in docs:
            if d.get_filename() and d.get_status() & moo.edit.EDIT_MODIFIED:
                d.save()

    def do_command(self, window, cmd):
        try:
            self.before_command(window, cmd) and \
            self.exec_command(window, cmd)   and \
            self.after_command(window, cmd)
        except Exception, e:
            mproj.utils.oops(window, e)


    def before_command(self, window, cmd):
        self.save_all(window)
        return True

    def after_command(self, window, cmd):
        return True

    def __cmd_simple(self, cmd, filename, window):
        try:
            command = self.config.get_command(cmd, filename, self.topdir)
        except Exception, e:
            print_error(e)
            return False
        output = self.window.get_output()
        output.clear()
        window.present_output()
        output.run_command(command)
        return True

    def exec_command(self, window, cmd):
        doc = self.window.get_active_doc()
        filename = doc and doc.get_filename()
        return self.__cmd_simple(cmd, filename, window)


__project__ = LatexProject
__project_type__ = "LaTeX"
__project_version__ = "1.0"
