#
#  simple.py
#
#  Copyright (C) 2004-2010 by Yevgen Muntyan <emuntyan@users.sourceforge.net>
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
import gtk
import gobject

class Plugin(moo.Plugin):
    # this method is called when plugin is loaded, once per session
    # (or after user checks Enabled in Preferences dialog)
    def do_init(self):
        editor = moo.Editor.instance()
        xml = editor.get_ui_xml()
        self.ui_merge_id = xml.new_merge_id()
        # Create a new action associated with editor windows
        moo.window_class_add_action(moo.EditWindow,
                                    "AnAction",                   # unique action id
                                    display_name="Do Something",  # what user sees in Configure Shortcuts dialog
                                    label="Do Something",         # menu item label
                                    stock_id=gtk.STOCK_APPLY,     # stock icon
                                    callback=self.do_stuff        # the action callback
                                    )
        # and add it into the xml, so it's actually persent in menu
        xml.add_item(self.ui_merge_id, "Editor/Menubar/Tools", name="AnAction", action="AnAction", position=-1)

        return True

    # this method is called when plugin is unloaded (on program exit or when plugin is disabled)
    def do_deinit(self):
        editor = moo.Editor.instance()
        xml = editor.get_ui_xml()
        xml.remove_ui(self.ui_merge_id)
        moo.window_class_remove_action(moo.EditWindow, "AnAction")

    def do_stuff(self, window):
        doc = window.get_active_doc()
        if doc is not None:
            buf = doc.get_buffer()
            buf.insert_at_cursor("Hi there")


# Register the plugin type with pygtk
gobject.type_register(Plugin)
# __plugin__ variable is picked up by plugin loader, it must be the plugin type
__plugin__ = Plugin
