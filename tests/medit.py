#!/usr/bin/env python
#
#   medit.py
#
#   Copyright (C) 2004-2006 by Yevgen Muntyan <muntyan@math.tamu.edu>
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   See COPYING file that comes with this distribution.
#

import moo
import gobject
import gtk
import sys
import getopt


def usage():
    print "Usage: %s [OPTIONS] [FILES]" % (sys.argv[0],)
    print "Options:"
    print "  -u, --unique        Use running instance of application"
    print "  -n, --new-app       Run new instance of application"
    print "  -l, --log[=FILE]    Show debug output or write it to FILE"
    print "      --version       Display version information and exit"
    print "  -h, --help          Display this help text and exit"


def get_ui():
    file = open('/home/muntyan/projects/moo/tests/medit-ui.xml')
    ui = file.read()
    file.close()
    return ui

def main(argv):
    new_instance = True

    opts, args = getopt.getopt(sys.argv[1:], "unlh",
                               ["unique", "new-app", "log=", "version", "help"])

    for opt, arg in opts:
        if opt in ('-h', '--help'):
            usage()
            return 0
        elif opt in ('-v', '--version'):
            print "medit ", moo.version
            return 0
        elif opt in ('-l', '--log'):
            if arg:
                moo.utils.set_log_func_file(arg)
            else:
                moo.utils.set_log_func_window()
        elif opt in ('-u', '--unique'):
            new_instance = False
        elif opt in ('-n', '--new-instance'):
            new_instance = True

    app = gobject.new(moo.app.App,
                      argv=argv,
                      short_name="medit",
                      full_name="medit",
                      description="medit is a text editor",
                      website="http://mooedit.sourceforge.net/",
                      website_label="http://mooedit.sourceforge.net/",
                      default_ui=get_ui(),
                      logo=moo.utils.STOCK_MEDIT)

    if (not new_instance and app.send_files(files)) or not app.init():
        gtk.gdk.notify_startup_complete()
        del app
        return 0

    editor = app.get_editor()
    window = editor.new_window()

    if args:
        for f in args:
            editor.new_file(window, None, f)

    editor.connect("all-windows-closed", lambda e, a: a.quit())

    return app.run()


if __name__ == '__main__':
    sys.exit(main(sys.argv))
