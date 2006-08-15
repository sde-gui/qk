/*
 *   tests/medit.c
 *
 *   Copyright (C) 2004-2006 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#include <config.h>
#include "mooedit/mooeditor.h"
#include "mooedit/mooplugin.h"
#include "mooedit/mooeditprefs.h"
#include "mooedit/plugins/mooeditplugins.h"
#include "mooutils/mooprefsdialog.h"
#include "medit-ui.h"


static void init_actions (void);

int main (int argc, char **argv)
{
    char *rcfile, *user_dir;
    MooEditor *editor;
    MooEditWindow *win;
    MooUIXML *xml;
    MooLangTable *lang_table;

    gtk_init (&argc, &argv);
//     gdk_window_set_debug_updates (TRUE);

    rcfile = g_build_filename (g_get_home_dir (), ".meditrc", NULL);
    moo_prefs_load (rcfile, NULL);

    init_actions ();

    xml = moo_ui_xml_new ();
    moo_ui_xml_add_ui_from_string (xml, MEDIT_UI, -1);

    editor = moo_editor_instance ();
    moo_editor_set_ui_xml (editor, xml);
    lang_table = moo_editor_get_lang_table (editor);
    user_dir = g_build_filename (g_get_home_dir (), ".medit", "syntax", NULL);
    moo_lang_table_add_dir (lang_table, MOO_TEXT_LANG_FILES_DIR);
    moo_lang_table_add_dir (lang_table, user_dir);
    moo_lang_table_read_dirs (lang_table);
    g_free (user_dir);

    g_signal_connect (editor, "all-windows-closed", G_CALLBACK (gtk_main_quit), NULL);

    moo_plugin_init_builtin ();

    user_dir = g_build_filename (g_get_home_dir (), ".medit", "plugins", NULL);
    moo_plugin_read_dir (MOO_PLUGINS_DIR);
    moo_plugin_read_dir (user_dir);
    g_free (user_dir);

    win = moo_editor_new_window (editor);

    gtk_main ();

    moo_prefs_save (rcfile, NULL);

    g_object_unref (editor);
    g_free (rcfile);

    return 0;
}


static void show_preferences (MooEditWindow *window)
{
    GtkWidget *dialog = moo_prefs_dialog_new ("Preferences");

    moo_prefs_dialog_append_page (MOO_PREFS_DIALOG (dialog),
                                  moo_edit_prefs_page_new (moo_editor_instance()));
    _moo_plugin_attach_prefs (dialog);

    moo_prefs_dialog_run (MOO_PREFS_DIALOG (dialog), GTK_WIDGET (window));
}


static void quit (MooEditWindow *window)
{
    moo_editor_close_all (moo_edit_window_get_editor (window));
}

static void about (void)
{
}


static void init_actions (void)
{
    MooWindowClass *klass = g_type_class_ref (MOO_TYPE_EDIT_WINDOW);

    g_return_if_fail (klass != NULL);

    moo_window_class_new_action (klass, "Preferences", NULL,
                                 "display-name", "Preferences",
                                 "label", "Preferences",
                                 "tooltip", "Preferences",
                                 "stock-id", GTK_STOCK_PREFERENCES,
                                 "closure-callback", show_preferences,
                                 NULL);
    moo_window_class_new_action (klass, "Quit", NULL,
                                 "display-name", "Quit",
                                 "label", "_Quit",
                                 "tooltip", "Quit",
                                 "stock-id", GTK_STOCK_QUIT,
                                 "closure-callback", quit,
                                 NULL);
    moo_window_class_new_action (klass, "About", NULL,
                                 "display-name", "About",
                                 "label", "About",
                                 "tooltip", "About",
                                 "stock-id", GTK_STOCK_ABOUT,
                                 "closure-callback", about,
                                 NULL);

    g_type_class_unref (klass);
}
