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
#include "mooui/moouiobject.h"
#include "editor-ui.h"

static void init_terminal (void);
static void init_editor (void);

int main (int argc, char *argv[])
{
    MooApp *app;
    MooUIXML *xml;
    MooEditor *editor;
    MooEditWindow *win;

    gtk_init (&argc, &argv);
//     gdk_window_set_debug_updates (TRUE);

    init_terminal ();
    init_editor ();

    app = MOO_APP (g_object_new (MOO_TYPE_APP,
                   "argv", argv,
                   "short-name", "medit",
                   "full-name", "MEdit",
                   "window-policy",
                   MOO_APP_MANY_EDITORS | MOO_APP_ONE_TERMINAL |
                           MOO_APP_QUIT_ON_CLOSE_ALL_WINDOWS,
                   "description", "MEdit is a text editor",
                   NULL));

    xml = moo_app_get_ui_xml (app);

    if (!moo_ui_xml_add_ui_from_string (xml, MEDIT_UI, -1, NULL))
        g_error ("%s", G_STRLOC);

    moo_app_init (app);

    editor = moo_app_get_editor (app);
    win = moo_editor_new_window (editor);

    if (argc > 1)
        moo_editor_open (editor, MOO_EDIT_WINDOW (win),
                         GTK_WIDGET (win), argv[1], NULL);

    return moo_app_run (app);
}


static void terminal_restart (MooTermWindow *win)
{
    MooTerm *term = moo_term_window_get_term (win);
    moo_term_kill_child (term);
    moo_term_start_default_profile (term, NULL);
}


static void init_terminal (void)
{
    GObjectClass *klass = g_type_class_ref (MOO_TYPE_TERM_WINDOW);

    g_return_if_fail (klass != NULL);

    moo_ui_object_class_new_action (klass,
                                    "id", "Restart",
                                    "name", "Restart",
                                    "label", "_Restart",
                                    "tooltip", "Restart",
                                    "icon-stock-id", GTK_STOCK_REFRESH,
                                    "closure::callback", terminal_restart,
                                    NULL);

    g_type_class_unref (klass);
}


static void editor_send_to_terminal (MooEditWindow *win)
{
    char *text;
    MooEdit *edit;

    edit = moo_edit_window_get_active_doc (win);

    text = moo_edit_get_selection (edit);
    if (!text)
        text = moo_edit_get_text (edit);

    if (text)
    {
        MooApp *app = moo_app_get_instance ();
        MooTermWindow *term_win = moo_app_get_terminal (app);
        MooTerm *term = moo_term_window_get_term (term_win);
        moo_term_feed_child (term, text, -1);
        g_free (text);
    }
}


static void init_editor (void)
{
    GObjectClass *klass = g_type_class_ref (MOO_TYPE_EDIT_WINDOW);

    g_return_if_fail (klass != NULL);

    moo_ui_object_class_new_action (klass,
                                    "id", "SendToTerminal",
                                    "name", "Send to Terminal",
                                    "label", "_Send to Terminal",
                                    "tooltip", "Send to Terminal",
                                    "icon-stock-id", GTK_STOCK_GOTO_BOTTOM,
                                    "closure::callback", editor_send_to_terminal,
                                    NULL);

    g_type_class_unref (klass);
}
