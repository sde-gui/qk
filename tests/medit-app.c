/*
 *   tests/editor.c.in
 *
 *   Copyright (C) 2004-2005 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#include "mooapp/mooapp.h"
#include "medit-ui.h"
#include <gtk/gtk.h>


int main (int argc, char *argv[])
{
    MooApp *app;
    MooUIXML *xml;
    MooEditor *editor;
    MooEditWindow *win;

    gtk_init (&argc, &argv);
//     gdk_window_set_debug_updates (TRUE);

    app = g_object_new (MOO_TYPE_APP,
                        "argv", argv,
                        "short-name", "medit",
                        "full-name", "medit",
                        "description", "medit is a text editor",
                        NULL);

    xml = moo_app_get_ui_xml (app);
    moo_ui_xml_add_ui_from_string (xml, MEDIT_UI, -1);

    moo_app_init (app);

    editor = moo_app_get_editor (app);

    win = moo_editor_new_window (editor);

    if (argc > 1)
        moo_editor_open_file (editor, MOO_EDIT_WINDOW (win),
                              GTK_WIDGET (win), argv[1], NULL);

    return moo_app_run (app);
}
