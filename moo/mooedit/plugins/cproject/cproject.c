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
#include "mooutils/mooprefsdialog.h"
#include "mooutils/moomenuaction.h"
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
    CProjectPlugin *plugin = moo_plugin_lookup (CPROJECT_PLUGIN_ID);
    g_return_if_fail (plugin != NULL && plugin->window != NULL);
    cproject_new_project (plugin);
}


static void
open_project_cb (G_GNUC_UNUSED MooEditWindow *window)
{
    CProjectPlugin *plugin = moo_plugin_lookup (CPROJECT_PLUGIN_ID);
    g_return_if_fail (plugin != NULL && plugin->window != NULL);
    cproject_open_project (plugin, NULL);
}


static void
close_project_cb (G_GNUC_UNUSED MooEditWindow *window)
{
    CProjectPlugin *plugin = moo_plugin_lookup (CPROJECT_PLUGIN_ID);
    g_return_if_fail (plugin != NULL && plugin->window != NULL);
    cproject_close_project (plugin);
}


static void
project_options_cb (G_GNUC_UNUSED MooEditWindow *window)
{
    CProjectPlugin *plugin = moo_plugin_lookup (CPROJECT_PLUGIN_ID);
    g_return_if_fail (plugin != NULL && plugin->window != NULL);
    cproject_project_options (plugin);
}


static void
build_project_cb (G_GNUC_UNUSED MooEditWindow *window)
{
    CProjectPlugin *plugin = moo_plugin_lookup (CPROJECT_PLUGIN_ID);
    g_return_if_fail (plugin != NULL && plugin->window != NULL);
    cproject_build_project (plugin);
}


static void
compile_file_cb (G_GNUC_UNUSED MooEditWindow *window)
{
    CProjectPlugin *plugin = moo_plugin_lookup (CPROJECT_PLUGIN_ID);
    g_return_if_fail (plugin != NULL && plugin->window != NULL);
    cproject_compile_file (plugin);
}


static void
execute_cb (G_GNUC_UNUSED MooEditWindow *window)
{
    CProjectPlugin *plugin = moo_plugin_lookup (CPROJECT_PLUGIN_ID);
    g_return_if_fail (plugin != NULL && plugin->window != NULL);
    cproject_execute (plugin);
}


/* XXX */
static MooAction*
create_build_configuration_action (MooEditWindow  *window)
{
    CProjectPlugin *plugin;
    MooMenuMgr *mgr;
    MooAction *action;

    plugin = moo_plugin_lookup (CPROJECT_PLUGIN_ID);
    g_return_val_if_fail (plugin != NULL, NULL);
    g_return_val_if_fail (!plugin->window || plugin->window == window, NULL);

    action = moo_menu_action_new ("BuildConfiguration");
    mgr = moo_menu_action_get_mgr (MOO_MENU_ACTION (action));

    moo_menu_mgr_append (mgr, NULL, "BuildConfiguration",
                         "Build Configuration", 0,
                         NULL, NULL);

    return action;
}


static void
open_recent_project (CProjectPlugin     *plugin,
                     MooHistoryListItem *item)
{
    g_return_if_fail (item != NULL);
    cproject_open_project (plugin, item->data);
}


static MooAction*
create_recent_action (MooEditWindow  *window)
{
    CProjectPlugin *plugin;
    MooAction *action;
    MooMenuMgr *mgr;

    plugin = moo_plugin_lookup (CPROJECT_PLUGIN_ID);
    g_return_val_if_fail (plugin != NULL, NULL);
    g_return_val_if_fail (!plugin->window || plugin->window == window, NULL);

    action = moo_menu_action_new ("OpenRecentProject");
    mgr = moo_history_list_get_menu_mgr (plugin->recent_list);
    moo_menu_action_set_mgr (MOO_MENU_ACTION (action), mgr);
    moo_menu_action_set_menu_data (MOO_MENU_ACTION (action), window, TRUE);
    moo_menu_action_set_menu_label (MOO_MENU_ACTION (action), "Open Recent Project");

    moo_bind_bool_property (action, "sensitive", plugin->recent_list, "empty", TRUE);

    return action;
}


gboolean
cproject_plugin_init (CProjectPlugin *plugin)
{
    MooWindowClass *klass = g_type_class_ref (MOO_TYPE_EDIT_WINDOW);
    MooEditor *editor = moo_editor_instance ();
    MooUIXML *xml = moo_editor_get_ui_xml (editor);
    const char *markup;

    g_return_val_if_fail (klass != NULL, FALSE);
    g_return_val_if_fail (editor != NULL, FALSE);

    moo_window_class_new_action (klass, "NewProject",
                                 "name", "New Project",
                                 "label", "New Project",
                                 "tooltip", "New Project",
                                 "icon-stock-id", MOO_STOCK_NEW_PROJECT,
                                 "closure::callback", new_project_cb,
                                 NULL);

    moo_window_class_new_action (klass, "OpenProject",
                                 "name", "Open Project",
                                 "label", "Open Project",
                                 "tooltip", "Open Project",
                                 "icon-stock-id", MOO_STOCK_OPEN_PROJECT,
                                 "closure::callback", open_project_cb,
                                 NULL);

    moo_window_class_new_action_custom (klass, "OpenRecentProject",
                                        (MooWindowActionFunc) create_recent_action,
                                        NULL);

    moo_window_class_new_action (klass, "CloseProject",
                                 "name", "Close Project",
                                 "label", "Close Project",
                                 "tooltip", "Close Project",
                                 "icon-stock-id", MOO_STOCK_CLOSE_PROJECT,
                                 "closure::callback", close_project_cb,
                                 NULL);

    moo_window_class_new_action (klass, "ProjectOptions",
                                 "name", "Project Options",
                                 "label", "Project Options",
                                 "tooltip", "Project Options",
                                 "icon-stock-id", MOO_STOCK_PROJECT_OPTIONS,
                                 "closure::callback", project_options_cb,
                                 NULL);

    moo_window_class_new_action_custom (klass, "BuildConfiguration",
                                        (MooWindowActionFunc) create_build_configuration_action,
                                        NULL);

    moo_window_class_new_action (klass, "BuildProject",
                                 "name", "Build Project",
                                 "label", "Build Project",
                                 "tooltip", "Build Project",
                                 "icon-stock-id", MOO_STOCK_BUILD,
                                 "closure::callback", build_project_cb,
                                 NULL);

    moo_window_class_new_action (klass, "CompileFile",
                                 "name", "Compile File",
                                 "label", "Compile File",
                                 "tooltip", "Compile File",
                                 "icon-stock-id", MOO_STOCK_COMPILE,
                                 "closure::callback", compile_file_cb,
                                 NULL);

    moo_window_class_new_action (klass, "Execute",
                                 "name", "Execute",
                                 "label", "Execute",
                                 "tooltip", "Execute",
                                 "icon-stock-id", MOO_STOCK_EXECUTE,
                                 "closure::callback", execute_cb,
                                 NULL);

    plugin->recent_list = moo_history_list_new (CPROJECT_PLUGIN_ID);
    g_signal_connect_swapped (plugin->recent_list, "activate_item",
                              G_CALLBACK (open_recent_project), plugin);

    plugin->ui_merge_id = moo_ui_xml_new_merge_id (xml);

    markup =
    "<item name=\"Project\" label=\"Project\">"
    "    <separator/>"
    "    <item name=\"NewProject\" action=\"NewProject\"/>"
    "    <item name=\"OpenProject\" action=\"OpenProject\"/>"
    "    <item name=\"OpenRecentProject\" action=\"OpenRecentProject\"/>"
    "    <separator/>"
    "    <item name=\"ProjectOptions\" action=\"ProjectOptions\"/>"
    "    <item name=\"BuildConfiguration\" action=\"BuildConfiguration\"/>"
    "    <separator/>"
    "    <item name=\"CloseProject\" action=\"CloseProject\"/>"
    "    <separator/>"
    "</item>"
    ""
    "<item name=\"Build\" label=\"Build\">"
    "    <separator/>"
    "    <item name=\"BuildProject\" action=\"BuildProject\"/>"
    "    <item name=\"CompileFile\" action=\"CompileFile\"/>"
    "    <separator/>"
    "    <item name=\"Execute\" action=\"Execute\"/>"
    "    <separator/>"
    "</item>"
    ;

    moo_ui_xml_insert_markup_after (xml, plugin->ui_merge_id,
                                    "Editor/Menubar",
                                    "View", markup);

    g_type_class_unref (klass);
    return TRUE;
}


void
cproject_plugin_deinit (CProjectPlugin *plugin)
{
    MooEditor *editor = moo_editor_instance ();
    MooUIXML *xml = moo_editor_get_ui_xml (editor);
    MooWindowClass *klass = g_type_class_ref (MOO_TYPE_EDIT_WINDOW);

    if (plugin->ui_merge_id)
        moo_ui_xml_remove_ui (xml, plugin->ui_merge_id);
    plugin->ui_merge_id = 0;

    moo_window_class_remove_action (klass, "NewProject");
    moo_window_class_remove_action (klass, "OpenProject");
    moo_window_class_remove_action (klass, "OpenRecentProject");
    moo_window_class_remove_action (klass, "CloseProject");
    moo_window_class_remove_action (klass, "ProjectOptions");
    moo_window_class_remove_action (klass, "BuildConfiguration");
    moo_window_class_remove_action (klass, "BuildProject");
    moo_window_class_remove_action (klass, "CompileFile");
    moo_window_class_remove_action (klass, "Execute");

    g_type_class_unref (klass);
}


void
cproject_plugin_attach (CProjectPlugin             *plugin,
                        MooEditWindow              *window)
{
    MooPaneLabel *label;
    GtkWidget *widget, *swin;
    MooEditor *editor;

    g_return_if_fail (plugin->window == NULL);

    plugin->window = window;

    editor = moo_edit_window_get_editor (window);
    g_object_set (editor,
                  "allow-empty-window", TRUE,
                  "single-window", TRUE,
                  NULL);

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

    widget = g_object_new (MOO_TYPE_CMD_VIEW,
                           "wrap-mode", GTK_WRAP_WORD,
                           "highlight-current-line", FALSE,
                           NULL);

    gtk_container_add (GTK_CONTAINER (swin), widget);
    gtk_widget_show_all (swin);

    label = moo_pane_label_new (GTK_STOCK_EXECUTE, NULL, NULL, "Build");

    moo_edit_window_add_pane (window, CPROJECT_PLUGIN_ID,
                              swin, label, MOO_PANE_POS_BOTTOM);

    plugin->output = MOO_CMD_VIEW (widget);
}


void
cproject_plugin_detach (CProjectPlugin             *plugin,
                        MooEditWindow              *window)
{
    g_return_if_fail (plugin->window == window);

    cproject_close_project (plugin);

    g_signal_handlers_disconnect_by_func (plugin->window,
                                          (gpointer) new_doc,
                                          plugin);
    g_signal_handlers_disconnect_by_func (plugin->window,
                                          (gpointer) cproject_update_filelist,
                                          plugin);

    moo_edit_window_remove_pane (window, CPROJECT_PLUGIN_ID);

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

    moo_history_list_add_filename (plugin->recent_list, path);
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

    actions = moo_window_get_actions (MOO_WINDOW (plugin->window));

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

    actions = moo_window_get_actions (MOO_WINDOW (plugin->window));
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
run_command (CProjectPlugin *plugin,
             char           *command)
{
    GtkWidget *pane;

    g_return_if_fail (plugin->window != NULL);
    g_return_if_fail (command != NULL);

    pane = moo_edit_window_get_pane (plugin->window, CPROJECT_PLUGIN_ID);
    g_return_if_fail (pane != NULL);

    moo_line_view_clear (MOO_LINE_VIEW (plugin->output));
    moo_big_paned_present_pane (plugin->window->paned, pane);

    moo_cmd_view_run_command (plugin->output, command);
}


void
cproject_build_project (CProjectPlugin *plugin)
{
    char *command;

    g_return_if_fail (plugin->project != NULL);
    g_return_if_fail (plugin->window != NULL);

    command = project_get_command (plugin->project,
                                   COMMAND_BUILD_PROJECT);
    run_command (plugin, command);
    g_free (command);
}


GtkWidget*
cproject_plugin_create_prefs_page (G_GNUC_UNUSED CProjectPlugin *plugin)
{
    return moo_prefs_dialog_page_new ("CProject", GTK_STOCK_PREFERENCES);
}
