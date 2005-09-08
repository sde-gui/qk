/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *   cproject.c
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

#include "cproject.h"
#include "mooutils/moostock.h"
#include "mooutils/moodialogs.h"
#include "mooui/moouiobject.h"
#include "mooui/moomenuaction.h"
#include <string.h>
#include <sys/wait.h>


#define CPROJECT_PREFS_ROOT MOO_PLUGIN_PREFS_ROOT "/" CPROJECT_PLUGIN_ID

#define LAST_PROJECT_PREFS      CPROJECT_PREFS_ROOT "/last-project"
#define LOAD_LAST_PROJECT_PREFS CPROJECT_PREFS_ROOT "/load-last-project"


static void     new_doc                     (MooEditWindow  *window,
                                             MooEdit        *doc,
                                             CProjectPlugin *plugin);
static gboolean window_close                (CProjectPlugin *plugin);
static void     cproject_update_filelist    (CProjectPlugin *plugin);
static void     block_window_signals        (CProjectPlugin *plugin);
static void     unblock_window_signals      (CProjectPlugin *plugin);


static void
new_project_cb (G_GNUC_UNUSED MooEditWindow *window)
{
    CProjectPlugin *plugin = moo_plugin_get_data (CPROJECT_PLUGIN_ID);
    g_return_if_fail (plugin != NULL && plugin->window != NULL);
    cproject_new_project (plugin);
}


static void
open_project_cb (G_GNUC_UNUSED MooEditWindow *window)
{
    CProjectPlugin *plugin = moo_plugin_get_data (CPROJECT_PLUGIN_ID);
    g_return_if_fail (plugin != NULL && plugin->window != NULL);
    cproject_open_project (plugin, NULL);
}


static void
close_project_cb (G_GNUC_UNUSED MooEditWindow *window)
{
    CProjectPlugin *plugin = moo_plugin_get_data (CPROJECT_PLUGIN_ID);
    g_return_if_fail (plugin != NULL && plugin->window != NULL);
    cproject_close_project (plugin);
}


static void
project_options_cb (G_GNUC_UNUSED MooEditWindow *window)
{
    CProjectPlugin *plugin = moo_plugin_get_data (CPROJECT_PLUGIN_ID);
    g_return_if_fail (plugin != NULL && plugin->window != NULL);
    cproject_project_options (plugin);
}


static void
build_project_cb (G_GNUC_UNUSED MooEditWindow *window)
{
    CProjectPlugin *plugin = moo_plugin_get_data (CPROJECT_PLUGIN_ID);
    g_return_if_fail (plugin != NULL && plugin->window != NULL);
    cproject_build_project (plugin);
}


static void
compile_file_cb (G_GNUC_UNUSED MooEditWindow *window)
{
    CProjectPlugin *plugin = moo_plugin_get_data (CPROJECT_PLUGIN_ID);
    g_return_if_fail (plugin != NULL && plugin->window != NULL);
    cproject_compile_file (plugin);
}


static void
execute_cb (G_GNUC_UNUSED MooEditWindow *window)
{
    CProjectPlugin *plugin = moo_plugin_get_data (CPROJECT_PLUGIN_ID);
    g_return_if_fail (plugin != NULL && plugin->window != NULL);
    cproject_execute (plugin);
}


/* XXX */
static GtkMenuItem*
create_build_configuration_menu (MooEditWindow  *window,
                                 G_GNUC_UNUSED MooAction *action)
{
    CProjectPlugin *plugin;
    GtkWidget *item;

    plugin = moo_plugin_get_data (CPROJECT_PLUGIN_ID);
    g_return_val_if_fail (plugin != NULL, NULL);
    g_return_val_if_fail (!plugin->window || plugin->window == window, NULL);

    plugin->build_configuration_menu = gtk_menu_new ();
    gtk_widget_show (plugin->build_configuration_menu);
    item = gtk_menu_item_new_with_label ("Build Configuration");
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (item),
                               plugin->build_configuration_menu);
    return GTK_MENU_ITEM (item);
}


static void
recent_item_added (MooRecentMgr   *mgr,
                   MooAction      *action)
{
    moo_action_set_sensitive (action, moo_recent_mgr_get_num_items (mgr));
}


static void
open_recent_project (CProjectPlugin     *plugin,
                     MooEditFileInfo    *file)
{
    cproject_open_project (plugin, file->filename);
}


static GtkMenuItem*
create_recent_menu (MooEditWindow  *window,
                    MooAction      *action)
{
    CProjectPlugin *plugin;
    GtkMenuItem *item;

    plugin = moo_plugin_get_data (CPROJECT_PLUGIN_ID);
    g_return_val_if_fail (plugin != NULL, NULL);
    g_return_val_if_fail (!plugin->window || plugin->window == window, NULL);

    item = moo_recent_mgr_create_menu (plugin->recent_mgr, NULL, "Open Recent Project");
    moo_action_set_sensitive (action, moo_recent_mgr_get_num_items (plugin->recent_mgr));

    /* XXX */
    g_signal_connect (plugin->recent_mgr, "item-added",
                      G_CALLBACK (recent_item_added), action);

    return item;
}


gboolean
cproject_plugin_init (CProjectPlugin *plugin)
{
    GObjectClass *klass = g_type_class_ref (MOO_TYPE_EDIT_WINDOW);
    g_return_val_if_fail (klass != NULL, FALSE);

    moo_ui_object_class_new_action (klass,
                                    "id", "NewProject",
                                    "name", "New Project",
                                    "label", "New Project",
                                    "tooltip", "New Project",
                                    "icon-stock-id", MOO_STOCK_NEW_PROJECT,
                                    "closure::callback", new_project_cb,
                                    NULL);

    moo_ui_object_class_new_action (klass,
                                    "id", "OpenProject",
                                    "name", "Open Project",
                                    "label", "Open Project",
                                    "tooltip", "Open Project",
                                    "icon-stock-id", MOO_STOCK_OPEN_PROJECT,
                                    "closure::callback", open_project_cb,
                                    NULL);

    moo_ui_object_class_new_action (klass,
                                    "action-type::", MOO_TYPE_MENU_ACTION,
                                    "id", "OpenRecentProject",
                                    "create-menu-func", create_recent_menu,
                                    NULL);

    moo_ui_object_class_new_action (klass,
                                    "id", "CloseProject",
                                    "name", "Close Project",
                                    "label", "Close Project",
                                    "tooltip", "Close Project",
                                    "icon-stock-id", MOO_STOCK_CLOSE_PROJECT,
                                    "closure::callback", close_project_cb,
                                    NULL);

    moo_ui_object_class_new_action (klass,
                                    "id", "ProjectOptions",
                                    "name", "Project Options",
                                    "label", "Project Options",
                                    "tooltip", "Project Options",
                                    "icon-stock-id", MOO_STOCK_PROJECT_OPTIONS,
                                    "closure::callback", project_options_cb,
                                    NULL);

    moo_ui_object_class_new_action (klass,
                                    "action-type::", MOO_TYPE_MENU_ACTION,
                                    "id", "BuildConfiguration",
                                    "create-menu-func", create_build_configuration_menu,
                                    NULL);

    moo_ui_object_class_new_action (klass,
                                    "id", "BuildProject",
                                    "name", "Build Project",
                                    "label", "Build Project",
                                    "tooltip", "Build Project",
                                    "icon-stock-id", MOO_STOCK_BUILD,
                                    "closure::callback", build_project_cb,
                                    NULL);

    moo_ui_object_class_new_action (klass,
                                    "id", "CompileFile",
                                    "name", "Compile File",
                                    "label", "Compile File",
                                    "tooltip", "Compile File",
                                    "icon-stock-id", MOO_STOCK_COMPILE,
                                    "closure::callback", compile_file_cb,
                                    NULL);

    moo_ui_object_class_new_action (klass,
                                    "id", "Execute",
                                    "name", "Execute",
                                    "label", "Execute",
                                    "tooltip", "Execute",
                                    "icon-stock-id", MOO_STOCK_EXECUTE,
                                    "closure::callback", execute_cb,
                                    NULL);

    plugin->recent_mgr = moo_recent_mgr_new (CPROJECT_PLUGIN_ID);
    g_signal_connect_swapped (plugin->recent_mgr, "open_recent",
                              G_CALLBACK (open_recent_project), plugin);

    g_type_class_unref (klass);
    return TRUE;
}


void
cproject_plugin_deinit (G_GNUC_UNUSED CProjectPlugin *plugin)
{
}


void
cproject_plugin_attach (MooEditWindow              *window,
                        CProjectPlugin             *plugin)
{
    MooPaneLabel *label;
    GtkWidget *widget, *swin;

    g_return_if_fail (plugin->window == NULL);

    plugin->window = window;

    cproject_load_prefs (plugin);
    cproject_update_project_ui (plugin);

    g_signal_connect (plugin->window, "new_doc",
                      G_CALLBACK (new_doc), plugin);
    g_signal_connect_swapped (plugin->window, "close_doc_after",
                              G_CALLBACK (cproject_update_filelist), plugin);
    g_signal_connect_swapped (plugin->window, "close",
                              G_CALLBACK (window_close), plugin);


    swin = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swin),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (swin),
                                         GTK_SHADOW_ETCHED_IN);

    widget = moo_pane_view_new ();
    gtk_container_add (GTK_CONTAINER (swin), widget);
    gtk_widget_show_all (swin);

    label = moo_pane_label_new (GTK_STOCK_EXECUTE, NULL, NULL, "Build");

    moo_edit_window_add_pane (window, CPROJECT_PLUGIN_ID,
                              swin, label, MOO_PANE_POS_BOTTOM);

    plugin->output = MOO_PANE_VIEW (widget);
}


void
cproject_plugin_detach (MooEditWindow              *window,
                        CProjectPlugin             *plugin)
{
    g_return_if_fail (plugin->window == window);

    g_signal_handlers_disconnect_by_func (plugin->window,
                                          (gpointer) new_doc,
                                          plugin);
    g_signal_handlers_disconnect_by_func (plugin->window,
                                          (gpointer) cproject_update_filelist,
                                          plugin);

    plugin->window = NULL;
    plugin->output = NULL;
}


void
cproject_load_prefs (CProjectPlugin *plugin)
{
    const char *last_proj;

    moo_prefs_new_key_string (LAST_PROJECT_PREFS, NULL);
    moo_prefs_new_key_bool (LOAD_LAST_PROJECT_PREFS, TRUE);

    last_proj = moo_prefs_get_filename (LAST_PROJECT_PREFS);

    if (!last_proj || !moo_prefs_get_bool (LOAD_LAST_PROJECT_PREFS))
        return;

    cproject_open_project (plugin, last_proj);
}


void
cproject_save_prefs (CProjectPlugin *plugin)
{
    if (plugin->project)
        moo_prefs_set_filename (LAST_PROJECT_PREFS, plugin->project->file);
    else
        moo_prefs_set_filename (LAST_PROJECT_PREFS, NULL);
}


gboolean
cproject_close_project (CProjectPlugin *plugin)
{
    if (!plugin->project)
        return TRUE;

    cproject_update_filelist (plugin);
    project_save (plugin->project);

    project_free (plugin->project);
    plugin->project = NULL;
    cproject_update_project_ui (plugin);

    return TRUE;
}


void
cproject_new_project (CProjectPlugin *plugin)
{
    if (!cproject_close_project (plugin))
        return;
}


void
cproject_open_project (CProjectPlugin *plugin,
                       const char     *path)
{
    GSList *list;
    MooEditor *editor;
    MooEditFileInfo *info;

    if (!path)
        path = moo_file_dialog (GTK_WIDGET (plugin->window),
                                MOO_DIALOG_FILE_OPEN_EXISTING,
                                "Open Project",
                                NULL);

    if (!path)
        return;

    if (!cproject_close_project (plugin))
        return;

    g_return_if_fail (plugin->project == NULL);
    plugin->project = project_load (path);
    g_return_if_fail (plugin->project != NULL);

    block_window_signals (plugin);

    editor = moo_edit_window_get_editor (plugin->window);

    list = moo_edit_window_list_docs (plugin->window);

    if (list)
    {
        moo_editor_close_docs (editor, list);
        g_slist_free (list);
    }

    for (list = plugin->project->file_list; list != NULL; list = list->next)
        moo_editor_open_file (editor, plugin->window, NULL, list->data, NULL);

    unblock_window_signals (plugin);

    cproject_save_prefs (plugin);
    cproject_update_project_ui (plugin);

    info = moo_edit_file_info_new (path, NULL);
    moo_recent_mgr_add_recent (plugin->recent_mgr, info);
    moo_edit_file_info_free (info);
}


void
cproject_project_options (CProjectPlugin *plugin)
{
    g_return_if_fail (plugin->project != NULL);
}


void
cproject_compile_file (CProjectPlugin *plugin)
{
    g_return_if_fail (plugin->project != NULL);
}


void
cproject_execute (CProjectPlugin *plugin)
{
    g_return_if_fail (plugin->project != NULL);
}


void
cproject_update_project_ui (CProjectPlugin *plugin)
{
    MooActionGroup *actions;
    MooAction *build_configuration, *project_options;
    MooAction *close_project, *build_project, *execute;
    gboolean sensitive;

    g_return_if_fail (plugin->window != NULL);

    if (plugin->project)
        moo_edit_window_set_title_prefix (plugin->window,
                                          plugin->project->name);
    else
        moo_edit_window_set_title_prefix (plugin->window, NULL);

    actions = moo_ui_object_get_actions (MOO_UI_OBJECT (plugin->window));
    build_configuration = moo_action_group_get_action (actions, "BuildConfiguration");
    project_options = moo_action_group_get_action (actions, "ProjectOptions");
    close_project = moo_action_group_get_action (actions, "CloseProject");
    build_project = moo_action_group_get_action (actions, "BuildProject");
    execute = moo_action_group_get_action (actions, "Execute");

    sensitive = (plugin->project != NULL);

    moo_action_set_sensitive (build_configuration, sensitive);
    moo_action_set_sensitive (project_options, sensitive);
    moo_action_set_sensitive (close_project, sensitive);
    moo_action_set_sensitive (build_project, sensitive);
    moo_action_set_sensitive (execute, sensitive);

    cproject_update_file_ui (plugin);
}


void
cproject_update_file_ui (CProjectPlugin *plugin)
{
    MooEdit *doc;
    MooActionGroup *actions;
    MooAction *compile_file;
    gboolean sensitive;

    g_return_if_fail (plugin->window != NULL);

    doc = moo_edit_window_get_active_doc (plugin->window);

    actions = moo_ui_object_get_actions (MOO_UI_OBJECT (plugin->window));
    compile_file = moo_action_group_get_action (actions, "CompileFile");

    sensitive = (plugin->project != NULL && doc != NULL);

    moo_action_set_sensitive (compile_file, sensitive);
}


static void
new_doc (G_GNUC_UNUSED MooEditWindow *window,
         MooEdit        *doc,
         CProjectPlugin *plugin)
{
    g_signal_connect_swapped (doc, "filename-changed",
                              G_CALLBACK (cproject_update_filelist),
                              plugin);

    if (!plugin->project)
        return;

    cproject_update_filelist (plugin);
}


static void
cproject_update_filelist (CProjectPlugin *plugin)
{
    GSList *docs, *filenames = NULL, *l;

    if (!plugin->project)
        return;

    docs = moo_edit_window_list_docs (plugin->window);

    for (l = docs; l != NULL; l = l->next)
    {
        MooEdit *edit = l->data;
        const char *filename = moo_edit_get_filename (edit);
        if (filename)
            filenames = g_slist_prepend (filenames, (gpointer) filename);
    }

    filenames = g_slist_reverse (filenames);
    project_set_file_list (plugin->project, filenames);
    project_save (plugin->project);

    g_slist_free (filenames);
}


static void
block_window_signals (CProjectPlugin *plugin)
{
    g_signal_handlers_block_by_func (plugin->window,
                                     (gpointer) new_doc,
                                     plugin);
    g_signal_handlers_block_by_func (plugin->window,
                                     (gpointer) cproject_update_filelist,
                                     plugin);
}


static void
unblock_window_signals (CProjectPlugin *plugin)
{
    g_signal_handlers_unblock_by_func (plugin->window,
                                       (gpointer) new_doc,
                                       plugin);
    g_signal_handlers_unblock_by_func (plugin->window,
                                       (gpointer) cproject_update_filelist,
                                       plugin);
}


static gboolean
window_close (CProjectPlugin *plugin)
{
    if (!plugin->project)
        return FALSE;
    else
        return !cproject_close_project (plugin);
}


static void
command_exit (GPid            pid,
              gint            status,
              CProjectPlugin *plugin)
{
    g_return_if_fail (pid == plugin->command_pid);

    if (plugin->command_out_watch)
        g_source_remove (plugin->command_out_watch);
    if (plugin->command_err_watch)
        g_source_remove (plugin->command_err_watch);

    command_free (plugin->running);
    plugin->running = NULL;

    g_spawn_close_pid (pid);

    g_return_if_fail (WIFEXITED (status));

    if (!WEXITSTATUS (status))
    {
        moo_pane_view_write_raw (plugin->output, "*** Success ***", -1);
    }
    else
    {
        char *msg = g_strdup_printf ("Command failed with status %d",
                                     WEXITSTATUS (status));
        moo_pane_view_write_raw (plugin->output, msg, -1);
        g_free (msg);
    }
}


static gboolean
command_out_or_err (GIOChannel     *channel,
                    GIOCondition    condition,
                    gboolean        stdout,
                    CProjectPlugin *plugin)
{
    char *line = NULL;
    gsize length;
    GError *error = NULL;

    if (condition & (G_IO_ERR | G_IO_HUP))
        goto end;

    g_io_channel_read_line (channel, &line, &length,
                            NULL, &error);

    if (error)
    {
        g_warning ("%s: %s", G_STRLOC, error->message);
        g_error_free (error);
        goto end;
    }

    if (line)
    {
        moo_pane_view_write_raw (plugin->output, line, length);
        g_free (line);
    }

    return TRUE;

end:
    if (stdout)
        plugin->command_out_watch = 0;
    else
        plugin->command_err_watch = 0;
    return FALSE;
}

static gboolean
command_out (GIOChannel     *channel,
             GIOCondition    condition,
             CProjectPlugin *plugin)
{
    return command_out_or_err (channel, condition, TRUE, plugin);
}


static gboolean
command_err (GIOChannel     *channel,
             GIOCondition    condition,
             CProjectPlugin *plugin)
{
    return command_out_or_err (channel, condition, FALSE, plugin);
}


void
cproject_run_command (CProjectPlugin *plugin,
                      Command        *command)
{
    GError *error = NULL;
    GIOChannel *command_out_chan, *command_err_chan;

    g_return_if_fail (plugin->output != NULL);
    g_return_if_fail (command != NULL);

    if (plugin->running)
        return;

    g_spawn_async_with_pipes (command->working_dir,
                              command->argv, command->envp,
                              G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD,
                              NULL, NULL,
                              &plugin->command_pid,
                              NULL,
                              &plugin->command_stdout,
                              &plugin->command_stderr,
                              &error);

    if (error)
    {
        g_warning ("%s: %s", G_STRLOC, error->message);
        g_error_free (error);
        command_free (command);
        return;
    }

    plugin->running = command;

    plugin->command_watch =
            g_child_watch_add (plugin->command_pid,
                               (GChildWatchFunc) command_exit,
                               plugin);

    command_out_chan = g_io_channel_unix_new (plugin->command_stdout);
    g_io_channel_set_encoding (command_out_chan, NULL, NULL);
    g_io_channel_set_buffered (command_out_chan, TRUE);
    g_io_channel_set_flags (command_out_chan, G_IO_FLAG_NONBLOCK, NULL);
    plugin->command_out_watch =
            g_io_add_watch_full (command_out_chan,
                                 G_PRIORITY_DEFAULT_IDLE,
                                 G_IO_IN | G_IO_PRI,
                                 (GIOFunc) command_out, plugin, NULL);
    g_io_channel_unref (command_out_chan);

    command_err_chan = g_io_channel_unix_new (plugin->command_stderr);
    g_io_channel_set_encoding (command_err_chan, NULL, NULL);
    g_io_channel_set_buffered (command_err_chan, TRUE);
    g_io_channel_set_flags (command_err_chan, G_IO_FLAG_NONBLOCK, NULL);
    plugin->command_err_watch =
            g_io_add_watch_full (command_err_chan,
                                 G_PRIORITY_DEFAULT_IDLE,
                                 G_IO_IN | G_IO_PRI,
                                 (GIOFunc) command_err, plugin, NULL);
    g_io_channel_unref (command_err_chan);
}


void
cproject_build_project (CProjectPlugin *plugin)
{
    Command *command;

    g_return_if_fail (plugin->project != NULL);

    command = project_get_command (plugin->project,
                                   COMMAND_BUILD_PROJECT);
    g_return_if_fail (command != NULL);

    cproject_run_command (plugin, command);
}
