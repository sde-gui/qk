import moo
import gtk
import pango
import re
import sys
import os

if os.name == 'nt':
    PYTHON_COMMAND = '"' + sys.exec_prefix + '\\python.exe"'
else:
    PYTHON_COMMAND = 'python'

try:
    import pyconsole
    have_pyconsole = True
except ImportError:
    have_pyconsole = False

PLUGIN_ID = "Python"

class FileLine(object):
    def __init__(self, filename, line):
        object.__init__(self)
        self.filename = filename
        self.line = line

class Plugin(moo.edit.Plugin):
    def __init__(self):
        moo.edit.Plugin.__init__(self)

        self.info = {
            "id" : PLUGIN_ID,
            "name" : "Python",
            "description" : "Python support",
            "author" : "Yevgen Muntyan <muntyan@math.tamu.edu>",
            "version" : "3.1415926",
            "enabled" : True,
            "visible" : True
        }

        if have_pyconsole:
            self.add_window_action(moo.edit.EditWindow, "PythonConsole",
                                   display_name="Python Console",
                                   label="Python Console",
                                   callback=self.show_console)
            self.add_ui("ToolsMenu", "PythonConsole")

        """ Run file """
        self.patterns = [
            [re.compile(r'\s*File\s*"([^"]+)",\s*line\s*(\d+).*'), 1, 2],
            [re.compile(r'\s*([^:]+):(\d+):.*'), 1, 2]
        ]
        self.add_window_action(moo.edit.EditWindow, "RunFile",
                               display_name="Run File",
                               label="Run File",
                               stock_id=moo.utils.STOCK_EXECUTE,
                               accel="<shift>F9",
                               callback=self.run_file)
        moo.edit.window_set_action_langs("RunFile", "sensitive", ["python"])
        self.add_ui("ToolsMenu", "RunFile")

    def show_console(self, window):
        window = gtk.Window()
        swin = gtk.ScrolledWindow()
        swin.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_ALWAYS)
        window.add(swin)

        console_type = pyconsole.ConsoleType(moo.edit.TextView)
        console = console_type(use_rlcompleter=False, start_script=
                               "import moo\nimport gtk\n")
        console.set_property("highlight-current-line", False)
        editor = moo.edit.editor_instance()
        mgr = editor.get_lang_mgr()
        lang = mgr.get_lang("PythonConsole")
        console.set_lang(lang)
        console.modify_font(pango.FontDescription("Courier New 11"))

        swin.add(console)
        window.set_default_size(400,300)
        window.set_title("pythony")
        window.show_all()


    def ensure_output(self, window):
        pane = window.get_pane(PLUGIN_ID)
        if not pane:
            label = moo.utils.PaneLabel(icon_stock_id=moo.utils.STOCK_EXECUTE, label="Output")
            output = moo.edit.CmdView()
            output.set_property("highlight-current-line", True)
            output.connect("activate", self.output_activate)
            output.connect("stderr-line", self.stderr_line)

            pane = gtk.ScrolledWindow()
            pane.set_shadow_type(gtk.SHADOW_ETCHED_IN)
            pane.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
            pane.add(output)
            pane.show_all()

            pane.output = output
            window.add_pane(PLUGIN_ID, pane, label, moo.utils.PANE_POS_BOTTOM)
            window.add_stop_client(output)
        return pane

    def output_activate(self, output, line):
        data = output.get_line_data(line)

        if not data:
            return False

        editor = moo.edit.editor_instance()
        editor.open_file(None, output, data.filename)

        doc = editor.get_doc(data.filename)

        if not doc:
            return True

        editor.set_active_doc(doc)
        doc.grab_focus()

        if data.line > 0:
            doc.move_cursor(data.line, -1, False, True)

        return True

    def stderr_line(self, output, line):
        data = None

        for p in self.patterns:
            match = p[0].match(line)
            if match:
                data = FileLine(match.group(p[1]), int(match.group(p[2])) - 1)
                break

        if not data:
            return False

        line_no = output.start_line()
        output.write(line, -1, output.lookup_tag("error"))
        output.end_line()
        output.set_line_data(line_no, data)

        return True

    def run_file(self, window):
        doc = window.get_active_doc()
        if not doc or not doc.save():
            return
        pane = self.ensure_output(window)

        if pane.output.running():
            return

        pane.output.clear()
        window.paned.present_pane(pane)
        pane.output.run_command(PYTHON_COMMAND + ' "%s"' % doc.get_filename())

    def detach_win(self, window):
        window.remove_pane(PLUGIN_ID)

moo.edit.plugin_register(Plugin)
# kate: indent-width 4; space-indent on
