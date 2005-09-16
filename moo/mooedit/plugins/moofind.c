/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   moofind.c
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifndef MOO_VERSION
#define MOO_VERSION NULL
#endif

#include "mooedit/mooplugin-macro.h"
#include "mooedit/plugins/moofind-glade.h"
#include "mooedit/plugins/mooeditplugins.h"
#include "mooedit/moofileview/moofileentry.h"
#include "mooedit/moocmdview.h"
#include "mooui/moouiobject.h"
#include "mooutils/moostock.h"
#include "mooutils/mooglade.h"
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

#define FIND_PLUGIN_ID "find"

enum {
    CMD_GREP = 1,
    CMD_FIND
};

typedef struct {
    MooPlugin parent;
    guint ui_merge_id;
} FindPlugin;

typedef struct {
    MooWindowPlugin parent;

    GtkWidget *grep_dialog;
    MooGladeXML *grep_xml;
    MooFileEntryCompletion *grep_completion;
    char *current_file;

    GtkWidget *find_dialog;
    MooGladeXML *find_xml;
    MooFileEntryCompletion *find_completion;

    MooEditWindow *window;
    MooCmdView *output;
    GtkTextTag *line_number_tag;
    GtkTextTag *match_tag;
    GtkTextTag *file_tag;
    GtkTextTag *error_tag;
    GtkTextTag *message_tag;
    guint match_count;
    int cmd;
} FindWindowPlugin;

#define WindowStuff FindWindowPlugin


typedef struct {
    char *filename;
    int line;
} FileLinePair;

static void         do_grep                 (MooEditWindow  *window,
                                             WindowStuff    *stuff);
static void         create_grep_dialog      (MooEditWindow  *window,
                                             WindowStuff    *stuff);
static void         init_grep_dialog        (MooEditWindow  *window,
                                             WindowStuff    *stuff);
static void         execute_grep            (const char     *pattern,
                                             const char     *glob,
                                             const char     *dir,
                                             const char     *skip_files,
                                             gboolean        case_sensitive,
                                             WindowStuff    *stuff);

static void         do_find                 (MooEditWindow  *window,
                                             WindowStuff    *stuff);
static void         create_find_dialog      (MooEditWindow  *window,
                                             WindowStuff    *stuff);
static void         init_find_dialog        (MooEditWindow  *window,
                                             WindowStuff    *stuff);
static void         execute_find            (const char     *pattern,
                                             const char     *dir,
                                             const char     *skip_files,
                                             WindowStuff    *stuff);

static gboolean     output_activate         (WindowStuff    *stuff,
                                             FileLinePair   *line_data);
static gboolean     command_exit            (MooPaneView    *view,
                                             int             status,
                                             WindowStuff    *stuff);
static gboolean     process_line            (MooPaneView    *view,
                                             const char     *line,
                                             WindowStuff    *stuff);


static void
find_in_files_cb (MooEditWindow *window)
{
    WindowStuff *stuff;
    int response;

    stuff = moo_window_plugin_lookup (FIND_PLUGIN_ID, window);
    g_return_if_fail (stuff != NULL);

    if (!stuff->grep_dialog)
    {
        create_grep_dialog (window, stuff);
        g_return_if_fail (stuff->grep_dialog != NULL);
    }

    init_grep_dialog (window, stuff);

    response = gtk_dialog_run (GTK_DIALOG (stuff->grep_dialog));
    gtk_widget_hide (stuff->grep_dialog);

    if (response == GTK_RESPONSE_OK)
        do_grep (window, stuff);
}


static void
find_file_cb (MooEditWindow *window)
{
    WindowStuff *stuff;
    int response;

    stuff = moo_window_plugin_lookup (FIND_PLUGIN_ID, window);
    g_return_if_fail (stuff != NULL);

    if (!stuff->find_dialog)
    {
        create_find_dialog (window, stuff);
        g_return_if_fail (stuff->find_dialog != NULL);
    }

    init_find_dialog (window, stuff);

    response = gtk_dialog_run (GTK_DIALOG (stuff->find_dialog));
    gtk_widget_hide (stuff->find_dialog);

    if (response == GTK_RESPONSE_OK)
        do_find (window, stuff);
}


static void
find_window_plugin_create (WindowStuff *stuff)
{
    GtkWidget *swin;
    MooPaneLabel *label;
    MooEditWindow *window = MOO_WINDOW_PLUGIN(stuff)->window;

    stuff->window = window;

    label = moo_pane_label_new (MOO_STOCK_FIND_IN_FILES, NULL, NULL, "Find");
    stuff->output = g_object_new (MOO_TYPE_CMD_VIEW,
                                  "highlight-current-line", TRUE,
                                  NULL);

    g_signal_connect_swapped (stuff->output, "activate",
                              G_CALLBACK (output_activate), stuff);

    stuff->line_number_tag =
            moo_pane_view_create_tag (MOO_PANE_VIEW (stuff->output),
                                      NULL, "weight", PANGO_WEIGHT_BOLD, NULL);
    stuff->match_tag =
            moo_pane_view_create_tag (MOO_PANE_VIEW (stuff->output), NULL,
                                      "foreground", "blue", NULL);
    stuff->file_tag =
            moo_pane_view_create_tag (MOO_PANE_VIEW (stuff->output), NULL,
                                      "foreground", "green", NULL);
    stuff->error_tag =
            moo_text_view_lookup_tag (MOO_TEXT_VIEW (stuff->output), "error");
    stuff->message_tag =
            moo_text_view_lookup_tag (MOO_TEXT_VIEW (stuff->output), "message");

    g_signal_connect (stuff->output, "cmd-exit",
                      G_CALLBACK (command_exit), stuff);
    g_signal_connect (stuff->output, "stdout-line",
                      G_CALLBACK (process_line), stuff);

    swin = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (swin),
                                         GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swin),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    gtk_container_add (GTK_CONTAINER (swin), GTK_WIDGET (stuff->output));
    gtk_widget_show_all (swin);

    moo_edit_window_add_pane (window, FIND_PLUGIN_ID,
                              swin, label, MOO_PANE_POS_BOTTOM);
}


static gboolean
find_plugin_init (FindPlugin *plugin)
{
    GObjectClass *klass = g_type_class_ref (MOO_TYPE_EDIT_WINDOW);
    MooEditor *editor = moo_editor_instance ();
    MooUIXML *xml = moo_editor_get_ui_xml (editor);

    g_return_val_if_fail (klass != NULL, FALSE);
    g_return_val_if_fail (editor != NULL, FALSE);

    moo_ui_object_class_new_action (klass, "FindInFiles",
                                    "name", "Find In Files",
                                    "label", "Find In Files",
                                    "tooltip", "Find In Files",
                                    "icon-stock-id", MOO_STOCK_FIND_IN_FILES,
                                    "closure::callback", find_in_files_cb,
                                    NULL);

    moo_ui_object_class_new_action (klass, "FindFile",
                                    "name", "Find File",
                                    "label", "Find File",
                                    "tooltip", "Find File",
                                    "icon-stock-id", MOO_STOCK_FIND_FILE,
                                    "closure::callback", find_file_cb,
                                    NULL);

    plugin->ui_merge_id = moo_ui_xml_new_merge_id (xml);

    moo_ui_xml_add_item (xml, plugin->ui_merge_id,
                         "Editor/Menubar/Search",
                         "FindInFiles", "FindInFiles", -1);
    moo_ui_xml_add_item (xml, plugin->ui_merge_id,
                         "Editor/Menubar/Search",
                         "FindFile", "FindFile", -1);

    g_type_class_unref (klass);
    return TRUE;
}


static void
find_plugin_deinit (FindPlugin *plugin)
{
    GObjectClass *klass = g_type_class_ref (MOO_TYPE_EDIT_WINDOW);
    MooEditor *editor = moo_editor_instance ();
    MooUIXML *xml = moo_editor_get_ui_xml (editor);

    moo_ui_object_class_remove_action (klass, "FindInFiles");
    moo_ui_object_class_remove_action (klass, "FindFile");

    if (plugin->ui_merge_id)
        moo_ui_xml_remove_ui (xml, plugin->ui_merge_id);
    plugin->ui_merge_id = 0;

    g_type_class_unref (klass);
}


static void
pattern_entry_changed (GtkEntry  *entry,
                       GtkDialog *dialog)
{
    const char *text = gtk_entry_get_text (entry);
    gtk_dialog_set_response_sensitive (dialog, GTK_RESPONSE_OK,
                                       text[0] != 0);
}


static void
create_grep_dialog (MooEditWindow  *window,
                    WindowStuff    *stuff)
{
    GtkWidget *dir_entry, *pattern_entry, *glob_entry, *skip_entry;

    stuff->grep_xml = moo_glade_xml_new_from_buf (MOO_FIND_GLADE_XML, -1,
                                                  "grep_dialog", NULL);
    g_return_if_fail (stuff->grep_xml != NULL);

    stuff->grep_dialog = moo_glade_xml_get_widget (stuff->grep_xml, "grep_dialog");
    g_return_if_fail (stuff->grep_dialog != NULL);

    gtk_dialog_set_default_response (GTK_DIALOG (stuff->grep_dialog),
                                     GTK_RESPONSE_OK);
    gtk_dialog_set_response_sensitive (GTK_DIALOG (stuff->grep_dialog),
                                       GTK_RESPONSE_OK, FALSE);
    gtk_window_set_transient_for (GTK_WINDOW (stuff->grep_dialog),
                                  GTK_WINDOW (window));

    g_signal_connect (stuff->grep_dialog, "delete-event",
                      G_CALLBACK (gtk_widget_hide_on_delete), NULL);

    pattern_entry = moo_glade_xml_get_widget (stuff->grep_xml, "pattern_combo");
    pattern_entry = GTK_BIN(pattern_entry)->child;
    g_signal_connect (pattern_entry, "changed",
                      G_CALLBACK (pattern_entry_changed), stuff->grep_dialog);

    dir_entry = moo_glade_xml_get_widget (stuff->grep_xml, "dir_combo");
    dir_entry = GTK_BIN(dir_entry)->child;
    stuff->grep_completion = g_object_new (MOO_TYPE_FILE_ENTRY_COMPLETION,
                                           "directories-only", TRUE,
                                           "case-sensitive", TRUE,
                                           "show-hidden", FALSE,
                                           NULL);
    moo_file_entry_completion_set_entry (stuff->grep_completion, GTK_ENTRY (dir_entry));

    glob_entry = moo_glade_xml_get_widget (stuff->grep_xml, "glob_combo");
    glob_entry = GTK_BIN(glob_entry)->child;

    skip_entry = moo_glade_xml_get_widget (stuff->grep_xml, "skip_combo");
    skip_entry = GTK_BIN(skip_entry)->child;
}


static void
create_find_dialog (MooEditWindow  *window,
                    WindowStuff    *stuff)
{
    GtkWidget *dir_entry, *pattern_entry;

    stuff->find_xml = moo_glade_xml_new_from_buf (MOO_FIND_GLADE_XML, -1,
                                                  "find_dialog", NULL);
    g_return_if_fail (stuff->find_xml != NULL);

    stuff->find_dialog = moo_glade_xml_get_widget (stuff->find_xml, "find_dialog");
    g_return_if_fail (stuff->find_dialog != NULL);

    gtk_dialog_set_default_response (GTK_DIALOG (stuff->find_dialog),
                                     GTK_RESPONSE_OK);
    gtk_dialog_set_response_sensitive (GTK_DIALOG (stuff->find_dialog),
                                       GTK_RESPONSE_OK, FALSE);
    gtk_window_set_transient_for (GTK_WINDOW (stuff->find_dialog),
                                  GTK_WINDOW (window));

    g_signal_connect (stuff->find_dialog, "delete-event",
                      G_CALLBACK (gtk_widget_hide_on_delete), NULL);

    pattern_entry = moo_glade_xml_get_widget (stuff->find_xml, "pattern_combo");
    pattern_entry = GTK_BIN(pattern_entry)->child;
    g_signal_connect (pattern_entry, "changed",
                      G_CALLBACK (pattern_entry_changed), stuff->find_dialog);

    dir_entry = moo_glade_xml_get_widget (stuff->find_xml, "dir_combo");
    dir_entry = GTK_BIN(dir_entry)->child;
    stuff->find_completion = g_object_new (MOO_TYPE_FILE_ENTRY_COMPLETION,
                                           "directories-only", TRUE,
                                           "case-sensitive", TRUE,
                                           "show-hidden", FALSE,
                                           NULL);
    moo_file_entry_completion_set_entry (stuff->find_completion, GTK_ENTRY (dir_entry));
}


static void
init_grep_dialog (MooEditWindow *window,
                  WindowStuff   *stuff)
{
    MooEdit *doc;
    GtkWidget *dir_entry, *pattern_entry, *glob_entry;

    dir_entry = GTK_BIN (moo_glade_xml_get_widget (stuff->grep_xml, "dir_combo"))->child;
    pattern_entry = GTK_BIN (moo_glade_xml_get_widget (stuff->grep_xml, "pattern_combo"))->child;
    glob_entry = GTK_BIN (moo_glade_xml_get_widget (stuff->grep_xml, "glob_combo"))->child;

    doc = moo_edit_window_get_active_doc (window);

    if (doc)
    {
        char *sel = moo_text_view_get_selection (MOO_TEXT_VIEW (doc));
        if (sel && !strchr (sel, '\n'))
            gtk_entry_set_text (GTK_ENTRY (pattern_entry), sel);
        g_free (sel);
    }

    if (!gtk_entry_get_text(GTK_ENTRY (dir_entry))[0])
    {
        if (doc && moo_edit_get_filename (doc))
        {
            char *dir = g_path_get_dirname (moo_edit_get_filename (doc));
            moo_file_entry_completion_set_path (stuff->grep_completion, dir);
            g_free (dir);
        }
        else
        {
            moo_file_entry_completion_set_path (stuff->grep_completion, g_get_home_dir ());
        }
    }

    if (!gtk_entry_get_text(GTK_ENTRY (glob_entry))[0])
        gtk_entry_set_text (GTK_ENTRY (glob_entry), "*");

    gtk_widget_grab_focus (pattern_entry);
}


static void
init_find_dialog (G_GNUC_UNUSED MooEditWindow *window,
                  WindowStuff   *stuff)
{
    GtkWidget *dir_entry, *pattern_entry;

    dir_entry = GTK_BIN (moo_glade_xml_get_widget (stuff->find_xml, "dir_combo"))->child;
    pattern_entry = GTK_BIN (moo_glade_xml_get_widget (stuff->find_xml, "pattern_combo"))->child;

    if (!gtk_entry_get_text(GTK_ENTRY (dir_entry))[0])
        moo_file_entry_completion_set_path (stuff->find_completion, g_get_home_dir ());

    gtk_widget_grab_focus (pattern_entry);
}


static void
do_grep (MooEditWindow  *window,
         WindowStuff    *stuff)
{
    GtkWidget *pane;
    GtkWidget *dir_entry, *pattern_entry, *glob_entry;
    GtkWidget *skip_entry, *case_sensitive_button;
    const char *dir_utf8, *pattern, *glob, *skip;
    gboolean case_sensitive;
    char *dir;

    pane = moo_edit_window_get_pane (window, FIND_PLUGIN_ID);
    g_return_if_fail (pane != NULL);

    dir_entry = GTK_BIN (moo_glade_xml_get_widget (stuff->grep_xml, "dir_combo"))->child;
    pattern_entry = GTK_BIN (moo_glade_xml_get_widget (stuff->grep_xml, "pattern_combo"))->child;
    glob_entry = GTK_BIN (moo_glade_xml_get_widget (stuff->grep_xml, "glob_combo"))->child;
    skip_entry = GTK_BIN (moo_glade_xml_get_widget (stuff->grep_xml, "skip_combo"))->child;
    case_sensitive_button = moo_glade_xml_get_widget (stuff->grep_xml, "case_sensitive_button");

    dir_utf8 = gtk_entry_get_text (GTK_ENTRY (dir_entry));
    dir = g_filename_from_utf8 (dir_utf8, -1, NULL, NULL, NULL);
    g_return_if_fail (dir != NULL);

    pattern = gtk_entry_get_text (GTK_ENTRY (pattern_entry));
    glob = gtk_entry_get_text (GTK_ENTRY (glob_entry));
    skip = gtk_entry_get_text (GTK_ENTRY (skip_entry));
    case_sensitive = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (case_sensitive_button));

    moo_pane_view_clear (MOO_PANE_VIEW (stuff->output));
    moo_big_paned_present_pane (window->paned, pane);

    execute_grep (pattern, glob, dir, skip,
                  case_sensitive, stuff);
}


static void
do_find (MooEditWindow  *window,
         WindowStuff    *stuff)
{
    GtkWidget *pane;
    GtkWidget *dir_entry, *pattern_entry, *skip_entry;
    const char *dir_utf8, *pattern, *skip;
    char *dir;

    pane = moo_edit_window_get_pane (window, FIND_PLUGIN_ID);
    g_return_if_fail (pane != NULL);

    dir_entry = GTK_BIN (moo_glade_xml_get_widget (stuff->find_xml, "dir_combo"))->child;
    pattern_entry = GTK_BIN (moo_glade_xml_get_widget (stuff->find_xml, "pattern_combo"))->child;
    skip_entry = GTK_BIN (moo_glade_xml_get_widget (stuff->find_xml, "skip_combo"))->child;

    dir_utf8 = gtk_entry_get_text (GTK_ENTRY (dir_entry));
    dir = g_filename_from_utf8 (dir_utf8, -1, NULL, NULL, NULL);
    g_return_if_fail (dir != NULL);

    pattern = gtk_entry_get_text (GTK_ENTRY (pattern_entry));
    skip = gtk_entry_get_text (GTK_ENTRY (skip_entry));

    moo_pane_view_clear (MOO_PANE_VIEW (stuff->output));
    moo_big_paned_present_pane (window->paned, pane);

    execute_find (pattern, dir, skip, stuff);
}


static FileLinePair*
file_line_pair_new (const char *filename,
                    int         line)
{
    FileLinePair *pair = g_new (FileLinePair, 1);
    pair->filename = g_strdup (filename);
    pair->line = line;
    return pair;
}


static void
file_line_pair_free (FileLinePair *pair)
{
    if (pair)
    {
        g_free (pair->filename);
        g_free (pair);
    }
}


static gboolean
process_grep_line (MooPaneView *view,
                   const char  *line,
                   WindowStuff *stuff)
{
    char *filename = NULL;
    char *number = NULL;
    const char *colon, *p;
    int view_line;
    int line_no;
    guint64 line_no_64;

    /* 'Binary file blah matches' */
    if (g_str_has_prefix (line, "Binary file "))
        return FALSE;

    p = line;
    if (!(colon = strchr (p, ':')) || !colon[1])
        goto parse_error;

    filename = g_strndup (p, colon - p);
    p = colon + 1;

    if (!(colon = strchr (p, ':')) || !colon[1])
        goto parse_error;

    number = g_strndup (p, colon - p);
    p = colon + 1;

    if (!stuff->current_file || strcmp (stuff->current_file, filename))
    {
        g_free (stuff->current_file);
        stuff->current_file = filename;
        view_line = moo_pane_view_write_line (view, filename, -1,
                                              stuff->file_tag);
        moo_pane_view_set_line_data (view, view_line,
                                     file_line_pair_new (filename, -1),
                                     (GDestroyNotify) file_line_pair_free);
    }
    else
    {
        g_free (filename);
    }

    errno = 0;
    line_no_64 = g_ascii_strtoull (number, NULL, 0);

    if (errno)
    {
        g_warning ("%s: could not parse number '%s'",
                   G_STRLOC, number);
        line_no = -1;
    }
    else if (line_no_64 > G_MAXINT)
    {
        g_warning ("%s: number '%s' is too large",
                   G_STRLOC, number);
        line_no = -1;
    }
    else if (line_no_64 == 0)
    {
        g_warning ("%s: number '%s' is zero",
                   G_STRLOC, number);
        line_no = -1;
    }
    else
    {
        line_no = line_no_64 - 1;
    }

    view_line = moo_pane_view_start_line (view);
    moo_pane_view_write (view, number, -1, stuff->line_number_tag);
    moo_pane_view_write (view, ": ", -1, NULL);
    moo_pane_view_write (view, p, -1, stuff->match_tag);
    moo_pane_view_end_line (view);

    moo_pane_view_set_line_data (view, view_line,
                                 file_line_pair_new (stuff->current_file, line_no),
                                 (GDestroyNotify) file_line_pair_free);
    stuff->match_count++;

    g_free (number);
    return TRUE;

parse_error:
    g_warning ("%s: could not parse line '%s'",
               G_STRLOC, line);
    g_free (filename);
    g_free (number);
    return FALSE;
}


static gboolean
process_find_line (MooPaneView *view,
                   const char  *line,
                   WindowStuff *stuff)
{
    int view_line;

    view_line = moo_pane_view_write_line (view, line, -1, stuff->match_tag);
    moo_pane_view_set_line_data (view, view_line,
                                 file_line_pair_new (line, -1),
                                 (GDestroyNotify) file_line_pair_free);
    stuff->match_count++;

    return TRUE;
}


static gboolean
process_line (MooPaneView *view,
              const char  *line,
              WindowStuff *stuff)
{
    switch (stuff->cmd)
    {
        case CMD_GREP:
            return process_grep_line (view, line, stuff);
        case CMD_FIND:
            return process_find_line (view, line, stuff);
        default:
            g_return_val_if_reached (FALSE);
    }
}


static void
execute_grep (const char     *pattern,
              const char     *glob,
              const char     *dir,
              const char     *skip_files,
              gboolean        case_sensitive,
              WindowStuff    *stuff)
{
    GString *command = NULL;
    char **globs = NULL;

    g_return_if_fail (stuff->output != NULL);
    g_return_if_fail (pattern && pattern[0]);
    g_return_if_fail (dir && dir[0]);

    g_free (stuff->current_file);
    stuff->current_file = NULL;
    stuff->match_count = 0;

    command = g_string_new ("");
    g_string_printf (command, "find '%s'", dir);

    if (glob)
    {
        globs = g_strsplit (glob, ";", 0);

        if (globs)
        {
            char **p;
            gboolean first = TRUE;

            for (p = globs; *p != NULL; p++)
            {
                if (first)
                {
                    g_string_append_printf (command, " \\( -name \"%s\"", *p);
                    first = FALSE;
                }
                else
                {
                    g_string_append_printf (command, " -o -name \"%s\"", *p);
                }
            }

            g_string_append (command, " \\)");
        }

        g_strfreev (globs);
    }

    g_string_append_printf (command, " -print -follow");

    if (skip_files)
    {
        globs = g_strsplit (skip_files, ";", 0);

        if (globs)
        {
            char **p;
            for (p = globs; *p != NULL; p++)
                g_string_append_printf (command, " | grep -v \"%s\"", *p);
        }

        g_strfreev (globs);
    }

    g_string_append_printf (command, " | xargs egrep -H -n %s-e '%s'",
                            !case_sensitive ? "-i " : "", pattern);

    stuff->cmd = CMD_GREP;
    moo_cmd_view_run_command (stuff->output, command->str);
    g_string_free (command, TRUE);
}


static void
execute_find (const char     *pattern,
              const char     *dir,
              const char     *skip_files,
              WindowStuff    *stuff)
{
    GString *command = NULL;
    char **globs = NULL;
    char **p;

    g_return_if_fail (stuff->cmd == 0);
    g_return_if_fail (pattern && pattern[0]);
    g_return_if_fail (dir && dir[0]);

    g_free (stuff->current_file);
    stuff->current_file = NULL;
    stuff->match_count = 0;

    globs = g_strsplit (pattern, ";", 0);
    g_return_if_fail (globs != NULL);

    for (p = globs; *p != NULL; p++)
    {
        if (!command)
        {
            command = g_string_new ("");
            g_string_printf (command, "find '%s' \\( -name \"%s\"", dir, *p);
        }
        else
        {
            g_string_append_printf (command, " -o -name \"%s\"", *p);
        }
    }

    g_string_append (command, " \\)");
    g_strfreev (globs);

    g_string_append_printf (command, " -print");

    if (skip_files)
    {
        globs = g_strsplit (skip_files, ";", 0);

        if (globs)
        {
            char **p;
            for (p = globs; *p != NULL; p++)
                g_string_append_printf (command, " | grep -v \"%s\"", *p);
        }

        g_strfreev (globs);
    }

    stuff->cmd = CMD_FIND;
    moo_cmd_view_run_command (stuff->output, command->str);
    g_string_free (command, TRUE);
}


static void
find_window_plugin_destroy (WindowStuff *stuff)
{
    MooEditWindow *window = MOO_WINDOW_PLUGIN(stuff)->window;

    g_signal_handlers_disconnect_by_func (stuff->output,
                                          (gpointer) command_exit,
                                          stuff);
    g_signal_handlers_disconnect_by_func (stuff->output,
                                          (gpointer) process_line,
                                          stuff);
    moo_cmd_view_abort (stuff->output);

    if (stuff->grep_dialog)
        gtk_widget_destroy (stuff->grep_dialog);
    if (stuff->find_dialog)
        gtk_widget_destroy (stuff->find_dialog);
    if (stuff->grep_xml)
        moo_glade_xml_unref (stuff->grep_xml);
    if (stuff->find_xml)
        moo_glade_xml_unref (stuff->find_xml);
    if (stuff->grep_completion)
        g_object_unref (stuff->grep_completion);
    if (stuff->find_completion)
        g_object_unref (stuff->find_completion);
    g_free (stuff->current_file);

    moo_edit_window_remove_pane (window, FIND_PLUGIN_ID);
}


static gboolean
command_exit (MooPaneView *view,
              int          status,
              WindowStuff *stuff)
{
    int cmd = stuff->cmd;

    g_return_val_if_fail (cmd != 0, FALSE);

    stuff->cmd = 0;

    if (WIFEXITED (status))
    {
        char *msg = NULL;
        guint8 exit_code = WEXITSTATUS (status);

        /* xargs exits with code 123 if it's command exited with status 1-125*/
        if (cmd == CMD_GREP && (!exit_code || exit_code == 123))
            msg = g_strdup_printf ("*** %d matches found ***",
                                   stuff->match_count);
        else if (cmd == CMD_FIND && !exit_code)
            msg = g_strdup_printf ("*** %d files found ***",
                                   stuff->match_count);
        else
            return FALSE;

        moo_pane_view_write_line (view, msg, -1,
                                  stuff->message_tag);
        g_free (msg);
        return TRUE;
    }

    return FALSE;
}


static gboolean
output_activate (WindowStuff    *stuff,
                 FileLinePair   *line_data)
{
    MooEditor *editor;
    MooEdit *doc;

    if (!line_data)
        return FALSE;

    editor = moo_edit_window_get_editor (stuff->window);
    moo_editor_open_file (editor, stuff->window, NULL,
                          line_data->filename, NULL);

    doc = moo_editor_get_doc (editor, line_data->filename);

    if (!doc)
        return TRUE;

    moo_editor_set_active_doc (editor, doc);
    gtk_widget_grab_focus (GTK_WIDGET (doc));

    if (line_data->line >= 0)
        moo_text_view_move_cursor (MOO_TEXT_VIEW (doc),
                                   line_data->line, -1, TRUE);

    return TRUE;
}


MOO_WINDOW_PLUGIN_DEFINE (FindWindowPlugin, find_window_plugin,
                          find_window_plugin_create,
                          find_window_plugin_destroy);
MOO_PLUGIN_DEFINE_PARAMS (info, TRUE, FIND_PLUGIN_ID,
                          "Find", "Finds everything",
                          "Yevgen Muntyan <muntyan@tamu.edu>",
                          MOO_VERSION);
MOO_PLUGIN_DEFINE (FindPlugin, find_plugin,
                   find_plugin_init, find_plugin_deinit,
                   NULL, NULL, NULL, info,
                   find_window_plugin_get_type ());


gboolean
moo_find_init (void)
{
    return moo_plugin_register (find_plugin_get_type ());
}
