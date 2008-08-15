import moo
import traceback
import sys
import os
import re

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
