/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *   moogrep.c
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

#include "mooedit/mooplugin.h"
#include "mooedit/plugins/moogrep-glade.h"
#include "mooedit/plugins/mooeditplugins.h"
#include "mooedit/moofileview/moofileentry.h"
#include "mooedit/moopaneview.h"
#include "mooui/moouiobject.h"
#include "mooutils/moostock.h"
#include <glade/glade.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

#define GREP_PLUGIN_ID "grep"


typedef struct {
    GtkWidget *dialog;
    GladeXML *xml;
    MooFileEntryCompletion *completion;
    MooEditWindow *window;
    MooPaneView *output;
    GtkTextTag *line_number_tag;
    GtkTextTag *match_tag;
    GtkTextTag *file_tag;
    GtkTextTag *error_tag;
    char *current_file;
    gboolean running;
    int exit_status;
    GPid pid;
    int stdout;
    int stderr;
    GIOChannel *stdout_io;
    GIOChannel *stderr_io;
    guint child_watch;
    guint stdout_watch;
    guint stderr_watch;
} WindowStuff;

typedef struct {
    char *filename;
    int line;
} FileLinePair;

static WindowStuff *window_stuff_new        (MooEditWindow  *window);
static void         window_stuff_free       (WindowStuff    *stuff);

static void         grep_plugin_attach      (MooEditWindow  *window);
static void         grep_plugin_detach      (MooEditWindow  *window);

static void         do_find                 (MooEditWindow  *window,
                                             WindowStuff    *stuff);
static void         create_dialog           (MooEditWindow  *window,
                                             WindowStuff    *stuff);
static void         init_dialog             (MooEditWindow  *window,
                                             WindowStuff    *stuff);
static void         execute_find            (const char     *pattern,
                                             const char     *glob,
                                             const char     *dir,
                                             const char     *skip_dirs,
                                             gboolean        case_sensitive,
                                             WindowStuff    *stuff);
static void         stop_find               (WindowStuff    *stuff);
static gboolean     output_click            (WindowStuff    *stuff,
                                             FileLinePair   *line_data);
static void         check_find_stop         (WindowStuff    *stuff);


static void
find_in_files_cb (MooEditWindow *window)
{
    WindowStuff *stuff;
    int response;

    stuff = moo_plugin_get_window_data (GREP_PLUGIN_ID, window);
    g_return_if_fail (stuff != NULL);

    if (!stuff->dialog)
    {
        create_dialog (window, stuff);
        g_return_if_fail (stuff->dialog != NULL);
    }

    init_dialog (window, stuff);

    response = gtk_dialog_run (GTK_DIALOG (stuff->dialog));
    gtk_widget_hide (stuff->dialog);

    if (response == GTK_RESPONSE_OK)
        do_find (window, stuff);
}


static void
grep_plugin_attach (MooEditWindow *window)
{
    GtkWidget *swin;
    MooPaneLabel *label;
    WindowStuff *stuff = window_stuff_new (window);

    label = moo_pane_label_new (MOO_STOCK_GREP, NULL, NULL, "Find in Files");
    stuff->output = g_object_new (MOO_TYPE_PANE_VIEW,
                                  "highlight-current-line", TRUE,
                                  NULL);

    g_signal_connect_swapped (stuff->output, "click",
                              G_CALLBACK (output_click), stuff);

    stuff->line_number_tag = moo_pane_view_create_tag (stuff->output, NULL,
                                                       "weight", PANGO_WEIGHT_BOLD,
                                                       NULL);
    stuff->match_tag = moo_pane_view_create_tag (stuff->output, NULL,
                                                 "foreground", "blue",
                                                 NULL);
    stuff->file_tag = moo_pane_view_create_tag (stuff->output, NULL,
                                                "foreground", "green",
                                                NULL);
    stuff->error_tag = moo_pane_view_create_tag (stuff->output, NULL,
                                                 "foreground", "red",
                                                 NULL);

    swin = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (swin),
                                         GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swin),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    gtk_container_add (GTK_CONTAINER (swin), GTK_WIDGET (stuff->output));
    gtk_widget_show_all (swin);

    moo_edit_window_add_pane (window, GREP_PLUGIN_ID,
                              swin, label, MOO_PANE_POS_BOTTOM);

    moo_plugin_set_window_data (GREP_PLUGIN_ID, window, stuff,
                                (GDestroyNotify) window_stuff_free);
}


static WindowStuff*
window_stuff_new (MooEditWindow  *window)
{
    WindowStuff *stuff = g_new0 (WindowStuff, 1);
    stuff->window = window;
    return stuff;
}


static void
window_stuff_free (WindowStuff *stuff)
{
    if (stuff)
    {
        if (stuff->dialog)
            gtk_widget_destroy (stuff->dialog);
        if (stuff->xml)
            g_object_unref (stuff->xml);
        if (stuff->completion)
            g_object_unref (stuff->completion);
        g_free (stuff->current_file);
        g_free (stuff);
    }
}


static gboolean
grep_plugin_init (void)
{
    GObjectClass *klass = g_type_class_ref (MOO_TYPE_EDIT_WINDOW);
    g_return_val_if_fail (klass != NULL, FALSE);

    moo_ui_object_class_new_action (klass,
                                    "id", "FindInFiles",
                                    "name", "Find In Files",
                                    "label", "Find In Files",
                                    "tooltip", "Find In Files",
                                    "icon-stock-id", MOO_STOCK_GREP,
                                    "closure::callback", find_in_files_cb,
                                    NULL);

    g_type_class_unref (klass);
    return TRUE;
}


static void
grep_plugin_deinit (void)
{
    /* XXX remove action */
}


gboolean
moo_grep_init (void)
{
    MooPluginParams params = { TRUE };
    MooPluginPrefsParams prefs_params;

    MooPluginInfo info = {
        MOO_PLUGIN_CURRENT_VERSION,

        GREP_PLUGIN_ID,
        GREP_PLUGIN_ID,
        GREP_PLUGIN_ID,
        "Yevgen Muntyan <muntyan@tamu.edu>",
        MOO_VERSION,

        (MooPluginInitFunc) grep_plugin_init,
        (MooPluginDeinitFunc) grep_plugin_deinit,
        (MooPluginWindowAttachFunc) grep_plugin_attach,
        (MooPluginWindowDetachFunc) grep_plugin_detach,

        &params,
        &prefs_params
    };

    return moo_plugin_register (&info, NULL);
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
create_dialog (MooEditWindow  *window,
               WindowStuff    *stuff)
{
    GtkWidget *dir_entry, *pattern_entry;

    stuff->xml = glade_xml_new_from_buffer (MOO_GREP_GLADE_XML,
                                            strlen (MOO_GREP_GLADE_XML),
                                            NULL, NULL);
    g_return_if_fail (stuff->xml != NULL);

    stuff->dialog = glade_xml_get_widget (stuff->xml, "dialog");
    g_return_if_fail (stuff->dialog != NULL);

    gtk_dialog_set_default_response (GTK_DIALOG (stuff->dialog),
                                     GTK_RESPONSE_OK);
    gtk_dialog_set_response_sensitive (GTK_DIALOG (stuff->dialog),
                                       GTK_RESPONSE_OK, FALSE);
    gtk_window_set_transient_for (GTK_WINDOW (stuff->dialog),
                                  GTK_WINDOW (window));

    g_signal_connect (stuff->dialog, "delete-event",
                      G_CALLBACK (gtk_widget_hide_on_delete), NULL);

    pattern_entry = glade_xml_get_widget (stuff->xml, "pattern_entry");
    g_signal_connect (pattern_entry, "changed",
                      G_CALLBACK (pattern_entry_changed), stuff->dialog);

    dir_entry = glade_xml_get_widget (stuff->xml, "dir_entry");
    g_return_if_fail (dir_entry != NULL);
    stuff->completion = g_object_new (MOO_TYPE_FILE_ENTRY_COMPLETION,
                                      "directories-only", TRUE,
                                      "case-sensitive", TRUE,
                                      "show-hidden", FALSE,
                                      NULL);
    moo_file_entry_completion_set_entry (stuff->completion, GTK_ENTRY (dir_entry));
}


static void
init_dialog (MooEditWindow *window,
             WindowStuff   *stuff)
{
    MooEdit *doc;
    GtkWidget *dir_entry, *pattern_entry, *glob_entry;

    dir_entry = glade_xml_get_widget (stuff->xml, "dir_entry");
    pattern_entry = glade_xml_get_widget (stuff->xml, "pattern_entry");
    glob_entry = glade_xml_get_widget (stuff->xml, "glob_entry");

    doc = moo_edit_window_get_active_doc (window);

    if (doc)
    {
        char *sel = moo_edit_get_selection (doc);
        if (sel && !strchr (sel, '\n'))
            gtk_entry_set_text (GTK_ENTRY (pattern_entry), sel);
        g_free (sel);
    }

    if (!gtk_entry_get_text(GTK_ENTRY (dir_entry))[0])
    {
        if (doc && moo_edit_get_filename (doc))
        {
            char *dir = g_path_get_dirname (moo_edit_get_filename (doc));
            moo_file_entry_completion_set_path (stuff->completion, dir);
            g_free (dir);
        }
        else
        {
            moo_file_entry_completion_set_path (stuff->completion, g_get_home_dir ());
        }
    }

    if (!gtk_entry_get_text(GTK_ENTRY (glob_entry))[0])
        gtk_entry_set_text (GTK_ENTRY (glob_entry), "*");

    gtk_widget_grab_focus (pattern_entry);
}


static void
do_find (MooEditWindow  *window,
         WindowStuff    *stuff)
{
    GtkWidget *pane;
    GtkWidget *dir_entry, *pattern_entry, *glob_entry;
    GtkWidget *skip_entry, *case_sensitive_button;
    const char *dir_utf8, *pattern, *glob, *skip;
    gboolean case_sensitive;
    char *dir;

    pane = moo_edit_window_get_pane (window, GREP_PLUGIN_ID);
    g_return_if_fail (pane != NULL);

    dir_entry = glade_xml_get_widget (stuff->xml, "dir_entry");
    pattern_entry = glade_xml_get_widget (stuff->xml, "pattern_entry");
    glob_entry = glade_xml_get_widget (stuff->xml, "glob_entry");
    skip_entry = glade_xml_get_widget (stuff->xml, "skip_entry");
    skip_entry = glade_xml_get_widget (stuff->xml, "skip_entry");
    case_sensitive_button = glade_xml_get_widget (stuff->xml, "case_sensitive_button");

    dir_utf8 = gtk_entry_get_text (GTK_ENTRY (dir_entry));
    dir = g_filename_from_utf8 (dir_utf8, -1, NULL, NULL, NULL);
    g_return_if_fail (dir != NULL);

    pattern = gtk_entry_get_text (GTK_ENTRY (pattern_entry));
    glob = gtk_entry_get_text (GTK_ENTRY (glob_entry));
    skip = gtk_entry_get_text (GTK_ENTRY (skip_entry));
    case_sensitive = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (case_sensitive_button));

    moo_pane_view_clear (stuff->output);
    moo_big_paned_present_pane (window->paned, pane);

    execute_find (pattern, glob, dir, skip,
                  case_sensitive, stuff);
}


static void
command_exit (GPid            pid,
              gint            status,
              WindowStuff    *stuff)
{
    g_return_if_fail (pid == stuff->pid);

    stuff->child_watch = 0;

    g_return_if_fail (WIFEXITED (status));

    stuff->exit_status = WEXITSTATUS (status);

    g_spawn_close_pid (stuff->pid);
    stuff->pid = 0;

    check_find_stop (stuff);
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


static void
process_line (WindowStuff *stuff,
              const char  *line,
              gboolean     stderr)
{
    char *filename = NULL;
    char *number = NULL;
    const char *colon, *p;
    int view_line;
    int line_no;
    guint64 line_no_64;

    if (stderr)
    {
        moo_pane_view_write_line (stuff->output, line, -1,
                                  stuff->error_tag);
        return;
    }

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
        view_line = moo_pane_view_write_line (stuff->output,
                                              filename, -1,
                                              stuff->file_tag);
        moo_pane_view_set_line_data (stuff->output, view_line,
                                     file_line_pair_new (filename, -1),
                                     (GDestroyNotify) file_line_pair_free);
    }
    else
    {
        g_free (filename);
    }

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

    view_line = moo_pane_view_start_line (stuff->output);
    moo_pane_view_write (stuff->output, number, -1,
                         stuff->line_number_tag);
    moo_pane_view_write (stuff->output, ": ", -1, NULL);
    moo_pane_view_write (stuff->output, p, -1, stuff->match_tag);
    moo_pane_view_end_line (stuff->output);

    moo_pane_view_set_line_data (stuff->output, view_line,
                                 file_line_pair_new (stuff->current_file, line_no),
                                 (GDestroyNotify) file_line_pair_free);

    g_free (number);
    return;

parse_error:
    g_warning ("%s: could not parse line '%s'",
               G_STRLOC, line);
    moo_pane_view_write_line (stuff->output, line, -1, NULL);
    g_free (filename);
    g_free (number);
}


static gboolean
command_out_or_err (GIOChannel     *channel,
                    GIOCondition    condition,
                    gboolean        stdout,
                    WindowStuff    *stuff)
{
    char *line;
    GError *error = NULL;
    GIOStatus status;

    status = g_io_channel_read_line (channel, &line, NULL, NULL, &error);

    if (line)
    {
        process_line (stuff, line, !stdout);
        g_free (line);
    }

    if (error)
    {
        g_warning ("%s: %s", G_STRLOC, error->message);
        g_error_free (error);
        return FALSE;
    }

    if (condition & (G_IO_ERR | G_IO_HUP))
        return FALSE;

    if (status == G_IO_STATUS_EOF)
        return FALSE;

    return TRUE;
}

static gboolean
command_out (GIOChannel     *channel,
             GIOCondition    condition,
             WindowStuff    *stuff)
{
    return command_out_or_err (channel, condition, TRUE, stuff);
}


static gboolean
command_err (GIOChannel     *channel,
             GIOCondition    condition,
             WindowStuff    *stuff)
{
    return command_out_or_err (channel, condition, FALSE, stuff);
}

static void
try_channel_leftover (WindowStuff *stuff,
                      GIOChannel  *channel,
                      gboolean     stdout)
{
    char *text;

    g_io_channel_read_to_end (channel, &text, NULL, NULL);

    if (text)
    {
        char **lines, **p;
        g_strdelimit (text, "\r", '\n');
        lines = g_strsplit (text, "\n", 0);

        if (lines)
        {
            for (p = lines; *p != NULL; p++)
                if (**p)
                    process_line (stuff, *p, !stdout);
        }

        g_strfreev (lines);
        g_free (text);
    }
}


static void
stdout_watch_removed (WindowStuff *stuff)
{
    if (stuff->stdout_io)
    {
        try_channel_leftover (stuff, stuff->stdout_io, TRUE);
        g_io_channel_unref (stuff->stdout_io);
    }

    stuff->stdout_io = NULL;
    stuff->stdout_watch = 0;
    check_find_stop (stuff);
}

static void
stderr_watch_removed (WindowStuff *stuff)
{
    if (stuff->stderr_io)
    {
        try_channel_leftover (stuff, stuff->stderr_io, FALSE);
        g_io_channel_unref (stuff->stderr_io);
    }

    stuff->stderr_io = NULL;
    stuff->stderr_watch = 0;
    check_find_stop (stuff);
}

static void
execute_find (const char     *pattern,
              const char     *glob,
              const char     *dir,
              const char     *skip_dirs,
              gboolean        case_sensitive,
              WindowStuff    *stuff)
{
    GError *error = NULL;
    char **argv = NULL;
    GString *command = NULL;
    char **globs = NULL;

    g_return_if_fail (stuff->output != NULL);
    g_return_if_fail (pattern && pattern[0]);

    if (stuff->running)
        return;

    g_free (stuff->current_file);
    stuff->current_file = NULL;

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

    if (skip_dirs)
    {
        globs = g_strsplit (skip_dirs, ";", 0);

        if (globs)
        {
            char **p;
            for (p = globs; *p != NULL; p++)
                g_string_append_printf (command, " | grep -v \"%s/\"", *p);
        }

        g_strfreev (globs);
    }

    g_string_append_printf (command, " | xargs egrep -H -n %s-e '%s'",
                            !case_sensitive ? "-i " : "", pattern);

    moo_pane_view_write_line (stuff->output, command->str, -1, NULL);

    argv = g_new (char*, 4);
    argv[0] = g_strdup ("/bin/sh");
    argv[1] = g_strdup ("-c");
    argv[2] = g_string_free (command, FALSE);
    argv[3] = NULL;

    g_spawn_async_with_pipes (NULL,
                              argv, NULL,
                              G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD,
                              NULL, NULL,
                              &stuff->pid,
                              NULL,
                              &stuff->stdout,
                              &stuff->stderr,
                              &error);

    if (error)
    {
        moo_pane_view_write_line (stuff->output, error->message, -1,
                                  stuff->error_tag);
        g_error_free (error);
        goto out;
    }

    stuff->running = TRUE;

    stuff->child_watch =
            g_child_watch_add (stuff->pid,
                               (GChildWatchFunc) command_exit,
                               stuff);

    stuff->stdout_io = g_io_channel_unix_new (stuff->stdout);
    g_io_channel_set_encoding (stuff->stdout_io, NULL, NULL);
    g_io_channel_set_buffered (stuff->stdout_io, TRUE);
    g_io_channel_set_flags (stuff->stdout_io, G_IO_FLAG_NONBLOCK, NULL);
    g_io_channel_set_close_on_unref (stuff->stdout_io, TRUE);
    stuff->stdout_watch =
            g_io_add_watch_full (stuff->stdout_io,
                                 G_PRIORITY_DEFAULT_IDLE,
                                 G_IO_IN | G_IO_PRI | G_IO_ERR | G_IO_HUP,
                                 (GIOFunc) command_out, stuff,
                                 (GDestroyNotify) stdout_watch_removed);

    stuff->stderr_io = g_io_channel_unix_new (stuff->stderr);
    g_io_channel_set_encoding (stuff->stderr_io, NULL, NULL);
    g_io_channel_set_buffered (stuff->stderr_io, TRUE);
    g_io_channel_set_flags (stuff->stderr_io, G_IO_FLAG_NONBLOCK, NULL);
    g_io_channel_set_close_on_unref (stuff->stderr_io, TRUE);
    stuff->stderr_watch =
            g_io_add_watch_full (stuff->stderr_io,
                                 G_PRIORITY_DEFAULT_IDLE,
                                 G_IO_IN | G_IO_PRI | G_IO_ERR | G_IO_HUP,
                                 (GIOFunc) command_err, stuff,
                                 (GDestroyNotify) stderr_watch_removed);

out:
    g_strfreev (argv);
}


static void
grep_plugin_detach (MooEditWindow *window)
{
    WindowStuff *stuff = moo_plugin_get_window_data (GREP_PLUGIN_ID, window);
    g_return_if_fail (stuff != NULL);
    stop_find (stuff);
}


static void
stop_find (WindowStuff *stuff)
{
    if (!stuff->running)
        return;

    stuff->running = FALSE;

    if (stuff->child_watch)
        g_source_remove (stuff->child_watch);
    if (stuff->stdout_watch)
        g_source_remove (stuff->stdout_watch);
    if (stuff->stderr_watch)
        g_source_remove (stuff->stderr_watch);

    if (stuff->stdout_io)
        g_io_channel_unref (stuff->stdout_io);
    if (stuff->stderr_io)
        g_io_channel_unref (stuff->stderr_io);

    if (stuff->pid)
    {
        kill (stuff->pid, SIGTERM);
        g_spawn_close_pid (stuff->pid);
    }

    g_free (stuff->current_file);
    stuff->current_file = NULL;
    stuff->pid = 0;
    stuff->stdout = -1;
    stuff->stderr = -1;
    stuff->child_watch = 0;
    stuff->stdout_watch = 0;
    stuff->stderr_watch = 0;
    stuff->stdout_io = NULL;
    stuff->stderr_io = NULL;
}


static void
check_find_stop (WindowStuff *stuff)
{
    if (!stuff->running)
        return;

    if (!stuff->child_watch && !stuff->stdout_watch && !stuff->stderr_watch)
    {
        if (!stuff->exit_status)
        {
            moo_pane_view_write_line (stuff->output,
                                      "*** Success ***", -1,
                                      NULL);
        }
        else
        {
            char *msg = g_strdup_printf ("Command failed with status %d",
                                         stuff->exit_status);
            moo_pane_view_write_line (stuff->output,
                                      msg, -1, stuff->error_tag);
            g_free (msg);
        }

        stop_find (stuff);
    }
}


static gboolean
output_click (WindowStuff    *stuff,
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
    g_return_val_if_fail (doc != NULL, FALSE);

    moo_editor_set_active_doc (editor, doc);
    gtk_widget_grab_focus (GTK_WIDGET (doc));

    if (line_data->line >= 0)
        moo_edit_move_cursor (doc, line_data->line, -1);

    return TRUE;
}
