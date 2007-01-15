import sys
import os
import re
import gtk
import moo
from moo.utils import _

if os.name == 'nt':
    PYTHON_COMMAND = '"' + sys.exec_prefix + '\\pythonw.exe" -u'
else:
    PYTHON_COMMAND = 'python -u'

PANE_ID = 'PythonOutput'

error_patterns = [
    [re.compile(r'\s*File\s*"([^"]+)",\s*line\s*(\d+).*'), 1, 2],
    [re.compile(r'\s*([^:]+):(\d+):.*'), 1, 2]
]

class FileLine(object):
    def __init__(self, filename, line):
        object.__init__(self)
        self.filename = filename
        self.line = line

class Runner(object):
    def __init__(self, window, python_command=PYTHON_COMMAND, pane_id=PANE_ID, pane_label=None):
        self.window = window
        self.python_command = python_command
        self.pane_id = pane_id
        self.pane_label = pane_label

    def __get_output(self):
        return self.window.get_pane(self.pane_id)
    def __ensure_output(self):
        pane = self.__get_output()
        if pane is None:
            label = self.pane_label or moo.utils.PaneLabel(icon_stock_id=moo.utils.STOCK_EXECUTE,
                                                           label=_("Python Output"))
            output = moo.edit.CmdView()
            output.set_property("highlight-current-line", True)
            output.connect("activate", self.__output_activate)
            output.connect("stderr-line", self.__stderr_line)

            pane = gtk.ScrolledWindow()
            pane.set_shadow_type(gtk.SHADOW_ETCHED_IN)
            pane.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
            pane.add(output)
            pane.show_all()

            pane.output = output
            self.window.add_pane(self.pane_id, pane, label, moo.utils.PANE_POS_BOTTOM)
            self.window.add_stop_client(output)
        return pane

    def run(self, filename=None, args_string=None, working_dir=None):
        pane = self.__get_output()

        if pane is not None and pane.output.running():
            return

        if filename is None:
            doc = self.window.get_active_doc()

            if not doc:
                return
            if not doc.get_filename() or doc.get_status() & moo.edit.EDIT_MODIFIED:
                if not doc.save():
                    return

            filename = doc.get_filename()

        pane = self.__ensure_output()
        pane.output.clear()
        self.window.paned.present_pane(pane)

        if working_dir is None:
            working_dir = os.path.dirname(filename)
        cmd_line = self.python_command + ' "%s"' % filename
        if args_string is not None:
            cmd_line += ' %s' % (args_string,)
        self.working_dir = working_dir
        pane.output.run_command(cmd_line, working_dir)

    def __output_activate(self, output, line):
        data = output.get_line_data(line)

        if not data:
            return False

        editor = moo.edit.editor_instance()

        filename = data.filename
        if not os.path.isabs(filename):
            filename = os.path.join(self.working_dir, filename)
        editor.open_file(None, output, filename)

        doc = editor.get_doc(filename)

        if not doc:
            return True

        editor.set_active_doc(doc)
        doc.grab_focus()

        if data.line >= 0:
            doc.move_cursor(data.line, -1, False, True)

        return True

    def __stderr_line(self, output, line):
        data = None

        for p in error_patterns:
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
