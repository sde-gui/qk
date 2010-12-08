#
#  mprj/utils.py
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
import traceback
import sys
import os
import gtk
import re
import tempfile
import shutil

def _expand(command, dic):
    if isinstance(command, str) or isinstance(command, unicode):
        working_dir = ''
        cmd = command
    else:
        working_dir = command[0]
        cmd = command[1]
    while 1:
        new_working_dir = re.sub(r'\$\(([a-zA-Z_]\w*)\)', r'%(\1)s', working_dir) % dic
        new_cmd = re.sub(r'\$\(([a-zA-Z_]\w*)\)', r'%(\1)s', cmd) % dic
        if new_working_dir == working_dir and new_cmd == cmd:
            return [new_working_dir, new_cmd]
        working_dir = new_working_dir
        cmd = new_cmd

def expand_command(command, vars, filename, top_srcdir, top_builddir=None):
    dic = get_file_paths(filename, top_srcdir, top_builddir)
    if vars:
        for v in vars:
            dic[v] = vars[v]
    return _expand(command, dic)

def get_file_paths(filename, top_srcdir, top_builddir=None):
    top_srcdir = os.path.abspath(top_srcdir)
    top_builddir = (top_builddir is None and top_srcdir) or \
                    os.path.abspath(top_builddir)

    if filename:
        filename = os.path.abspath(filename)
        basename = os.path.basename(filename)
        base, ext = os.path.splitext(basename)
        ext = ext[1:]

        srcdir = os.path.dirname(filename)

        if srcdir == top_srcdir:
            builddir = top_builddir
        elif os.path.commonprefix([top_srcdir, srcdir]) == top_srcdir:
            builddir = os.path.join(top_builddir, srcdir[len(top_srcdir) + 1:])
        else:
            print "can't get builddir for", filename
            builddir = ''
    else:
        filename, basename = '', ''
        base, ext = '', ''
        srcdir, builddir = '', ''

    return {
        'top_srcdir' : top_srcdir,
        'top_builddir' : top_builddir,
        'srcdir' : srcdir,
        'builddir' : builddir,
        'filename' : filename,
        'basename' : basename,
        'base' : base,
        'ext' : ext,
    }

def implement_me(window, what):
    moo.utils.warning_dialog(window, "IMPLEMENT ME", str(what))

def oops(window, error):
    moo.utils.error_dialog(window, "OOPS", format_error(error))

def print_error(error=None):
    print >> sys.stderr, format_error(error)

def format_error(error=None):
    if error:
        return str(error) + "\n" + \
            "".join(traceback.format_exception(*sys.exc_info()))
    else:
        return "".join(traceback.format_exception(*sys.exc_info()))


def entry_dialog(parent=None, label=None, entry_content=None, title=None):
    glade_file = os.path.join(os.path.dirname(__file__), "utils.glade")
    xml = moo.utils.glade_xml_new_from_file(glade_file, root='entry_dialog',
                                            domain=moo.utils.GETTEXT_PACKAGE)
    xml.w_entry_dialog.set_title(title)
    if parent:
        parent = parent.get_toplevel()
        xml.w_entry_dialog.set_transient_for(parent)
    if label:
        xml.w_label.set_text(label)
    else:
        xml.w_label.hide()
    if entry_content:
        xml.w_entry.set_text(entry_content)

    retval = None
    if xml.w_entry_dialog.run() == gtk.RESPONSE_OK:
        retval = xml.w_entry.get_text()

    xml.w_entry_dialog.destroy()
    return retval

def error_dialog(text, secondary_text=None, parent=None):
    moo.utils.error_dialog(parent, text, secondary_text)

def question_dialog(text, secondary_text=None, default_ok=False, parent=None):
    if parent:
        parent = parent.get_toplevel()
    dlg = gtk.MessageDialog(parent, 0, gtk.MESSAGE_QUESTION, gtk.BUTTONS_OK_CANCEL)
    dlg.set_property('text', text)
    dlg.set_property('secondary_text', secondary_text)
    if default_ok:
        dlg.set_default_response(gtk.RESPONSE_OK)
    else:
        dlg.set_default_response(gtk.RESPONSE_CANCEL)
    response = dlg.run()
    dlg.destroy()
    return response == gtk.RESPONSE_OK

def prefs_key(name):
    return 'MProject/' + name

def save_file(filename, contents):
    f = FileSaver(filename)
    f.write(contents)
    f.close()

class FileSaver(object):
    def __init__(self, filename):
        object.__init__(self)

        self.tmp_file = None
        self.tmp_name = None
        self.filename = None
        self.closed = True

        basename = os.path.basename(filename)
        dirname = os.path.dirname(filename)
        tmp_fd, tmp_name = tempfile.mkstemp('', '.' + basename + '-', dirname, True)
        self.tmp_file = os.fdopen(tmp_fd, 'w')
        self.tmp_name = tmp_name
        self.filename = filename
        self.closed = False

    def close(self):
        if not self.closed:
            self.closed = True
            try:
                self.tmp_file.close()
                shutil.move(self.tmp_name, self.filename)
            except Exception, e:
                self.__cleanup()
                raise e

    def __cleanup(self):
        if self.tmp_file:
            try:
                self.tmp_file.close()
            except:
                pass
        if self.tmp_name:
            try:
                os.remove(self.tmp_name)
            except:
                pass

    def flush(self):
        return self.tmp_file.flush()
    def fileno(self):
        return self.tmp_file.fileno()
    def isatty(self):
        return False
    def seek(self, offset, whence = 0):
        return self.tmp_file.seek(offset, whence)
    def tell(self):
        return self.tmp_file.tell()
    def truncate(self, size = 0):
        return self.tmp_file.truncate(size)
    def write(self, string):
        return self.tmp_file.write(string)
    def writelines(self, sequence):
        return self.tmp_file.writelines(sequence)

if __name__ == '__main__':
    print expand_command(['$(builddir)', 'make $(base).o'],
                         None,
                         '/blah/project/subdir/file.c',
                         '/blah/project',
                         '/blah/project/build')
    print expand_command(['$(top_builddir)', '$(make)'],
                         {'make' : 'make'},
                         None,
                         '/blah/project',
                         '/blah/project/build')
