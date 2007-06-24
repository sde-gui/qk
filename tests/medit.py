#!/usr/bin/env python
#
#   medit.py
#
#   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License as published by the Free Software Foundation; either
#   version 2.1 of the License, or (at your option) any later version.
#
#   See COPYING file that comes with this distribution.
#

import gtk
import moo
import gobject
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
                      default_ui=ui,
                      logo=moo.utils.STOCK_MEDIT)

    if (not new_instance and app.send_files(files)) or not app.init():
        gtk.gdk.notify_startup_complete()
        return 0

    editor = app.get_editor()
    window = editor.new_window()

    if args:
        for f in args:
            editor.new_file(window, None, f)

    editor.connect("all-windows-closed", lambda e, a: a.quit(), app)

    return app.run()


ui = """
<ui><!-- -*- Mode: XML; indent-tabs-mode: nil; c-basic-offset: 2 -*- -->

<object name="Editor">

  <widget name="Menubar">

    <item name="File" label="_File">
      <separator/>
      <item action="NewDoc"/>
      <item action="NewWindow"/>
      <separator/>
      <item action="Open"/>
      <item action="OpenRecent"/>
      <item action="Save"/>
      <item action="SaveAs"/>
      <separator/>
      <item action="Reload"/>
      <separator/>
      <item action="PrintOptions"/>
      <item action="PageSetup"/>
      <item action="Print"/>
      <separator/>
      <placeholder name="UserMenu"/>
      <separator/>
      <item action="Close"/>
      <item action="CloseAll"/>
      <separator/>
      <item action="Quit"/>
      <separator/>
    </item>

    <item name="Edit" label="_Edit">
      <separator/>
      <item action="Undo"/>
      <item action="Redo"/>
      <separator/>
      <item action="Cut"/>
      <item action="Copy"/>
      <item action="Paste"/>
      <item action="Delete"/>
      <separator/>
      <item action="SelectAll"/>
      <separator/>
      <item action="Indent"/>
      <item action="Unindent"/>
      <separator/>
      <item action="Comment"/>
      <item action="Uncomment"/>
      <separator/>
      <placeholder name="UserMenu"/>
      <separator/>
    </item>

    <item name="Search" label="_Search">
      <separator/>
      <item action="Find"/>
      <item action="FindNext"/>
      <item action="FindPrevious"/>
      <item action="Replace"/>
      <separator/>
      <item action="QuickSearch"/>
      <item action="FindCurrent"/>
      <item action="FindCurrentBack"/>
      <separator/>
      <item action="GoToLine"/>
      <separator/>
      <item action="NextPlaceholder"/>
      <item action="PrevPlaceholder"/>
      <separator/>
      <placeholder name="UserMenu"/>
      <separator/>
    </item>

    <item name="View" label="_View">
      <separator/>
      <item action="PreviousTab"/>
      <item action="NextTab"/>
      <separator/>
      <item action="LanguageMenu"/>
      <separator/>
      <!--
      <item action="ToggleBookmark"/>
      <item action="PreviousBookmark"/>
      <item action="NextBookmark"/>
      -->
      <separator/>
      <item name="Document" label="_Document">
        <item action="WrapText"/>
        <item action="LineNumbers"/>
      </item>
      <separator/>
      <item action="FocusDoc"/>
      <separator/>
      <placeholder name="UserMenu"/>
      <separator/>
    </item>

    <item name="Tools" label="_Tools">
      <separator/>
      <placeholder name="ToolsMenu"/>
      <separator/>
      <placeholder name="UserMenu"/>
      <separator/>
      <item action="StopJob"/>
      <separator/>
    </item>

    <item name="Settings" label="_Settings">
      <separator/>
      <item action="ShowToolbar"/>
      <item action="ToolbarStyle"/>
      <separator/>
      <item action="ConfigureShortcuts"/>
      <separator/>
      <item action="Preferences"/>
      <separator/>
    </item>

    <item name="Window" label="_Window">
      <separator/>
      <item action="NoDocuments"/>
      <placeholder name="DocList"/>
      <separator/>
    </item>

    <item name="Help" label="_Help">
      <separator/>
      <item action="About"/>
      <separator/>
    </item>

  </widget> <!-- Menubar -->

  <widget name="Toolbar">
    <separator/>
    <item action="NewDoc"/>
    <separator/>
    <item action="Open"/>
    <item action="Save"/>
    <item action="SaveAs"/>
    <separator/>
    <item action="Undo"/>
    <item action="Redo"/>
    <separator/>
    <item action="Cut"/>
    <item action="Copy"/>
    <item action="Paste"/>
    <separator/>

    <item action="Find"/>
    <item action="Replace"/>

    <separator/>
    <placeholder name="BuildToolbar">
      <item action="StopJob"/>
    </placeholder>
    <separator/>

  </widget> <!-- Toolbar -->

  <widget name="Popup">
    <separator/>
    <placeholder name="PopupStart"/>
    <separator/>
    <item action="Undo"/>
    <item action="Redo"/>
    <separator/>
    <item action="Cut"/>
    <item action="Copy"/>
    <item action="Paste"/>
    <separator/>
    <item action="SelectAll"/>
    <separator/>
    <item action="BookmarksMenu"/>
    <separator/>
    <placeholder name="PopupEnd"/>
    <separator/>
  </widget> <!-- Popup -->

</object> <!-- Editor -->

</ui>
"""


if __name__ == '__main__':
    sys.exit(main(sys.argv))
