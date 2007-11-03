/*
 *   project-plugin.m
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#include "config.h"
#include "mooedit/mooplugin-macro.h"
#include "mooedit/plugins/mooeditplugins.h"
#include "mooedit/mooeditwindow.h"
#include "mooutils/moostock.h"
#include "mooutils/moomenuaction.h"
#include "mooutils/moomarshals.h"
#include "manager.h"
#include "project-plugin.h"
#include <gtk/gtk.h>

typedef struct {
    MooPlugin parent;
    guint ui_merge_id;
    MooHistoryList *recent_list;
    MPManager *pm;
    char *project_to_open;
} ProjectPlugin;

#define PROJECT_PLUGIN(mpl) ((ProjectPlugin*)mpl)


static void
project_plugin_attach_window (MooPlugin     *mplugin,
                              MooEditWindow *window)
{
    ProjectPlugin *plugin = PROJECT_PLUGIN (mplugin);

    g_return_if_fail (plugin->pm != nil);

    moo_objc_push_autorelease_pool ();
    [plugin->pm attachWindow: window];
    moo_objc_pop_autorelease_pool ();
}

static void
project_plugin_detach_window (MooPlugin     *mplugin,
                              MooEditWindow *window)
{
    ProjectPlugin *plugin = PROJECT_PLUGIN (mplugin);

    g_return_if_fail (plugin->pm != nil);

    moo_objc_push_autorelease_pool ();
    [plugin->pm detachWindow: window];
    moo_objc_pop_autorelease_pool ();
}

static void
open_project_cb (void)
{
    ProjectPlugin *plugin = moo_plugin_lookup (PROJECT_PLUGIN_ID);
    g_return_if_fail (plugin && plugin->pm);

    moo_objc_push_autorelease_pool ();
    [plugin->pm openProject:NULL];
    moo_objc_pop_autorelease_pool ();
}

static void
close_project_cb (void)
{
    ProjectPlugin *plugin = moo_plugin_lookup (PROJECT_PLUGIN_ID);
    g_return_if_fail (plugin && plugin->pm);

    moo_objc_push_autorelease_pool ();
    [plugin->pm closeProject];
    moo_objc_pop_autorelease_pool ();
}

static void
project_options_cb (void)
{
    ProjectPlugin *plugin = moo_plugin_lookup (PROJECT_PLUGIN_ID);
    g_return_if_fail (plugin && plugin->pm);

    moo_objc_push_autorelease_pool ();
    [plugin->pm projectOptions];
    moo_objc_pop_autorelease_pool ();
}

static GtkAction *
create_recent_list_action (G_GNUC_UNUSED MooWindow *window,
                           ProjectPlugin *plugin)
{
    GtkAction *action;

    action = moo_menu_action_new ("OpenRecentProject", "Open Recent Project");
    g_object_set (action, "display-name", "Open Recent Project", NULL);
    moo_menu_action_set_mgr (MOO_MENU_ACTION (action),
                             moo_history_list_get_menu_mgr (plugin->recent_list));
    moo_bind_bool_property (action, "sensitive", plugin->recent_list, "empty", TRUE);

    return action;
}

static void
recent_item_activated (G_GNUC_UNUSED MooHistoryList *list,
                       MooHistoryItem *item,
                       G_GNUC_UNUSED gpointer menu_data,
                       ProjectPlugin  *plugin)
{
    [plugin->pm openProject:item->data];
}

static void
meth_open_project (ProjectPlugin *plugin,
                   const char    *filename)
{
    g_return_if_fail (filename != NULL);

    if (plugin->pm)
    {
        [plugin->pm openProject:filename];
    }
    else
    {
        g_free (plugin->project_to_open);
        plugin->project_to_open = g_strdup (filename);
    }
}

static gboolean
project_plugin_init (ProjectPlugin *plugin)
{
    MooEditor *editor;
    MooWindowClass *klass;
    MooUIXML *xml;

    moo_objc_push_autorelease_pool ();

    if (!(plugin->pm = [[MPManager alloc] init]))
    {
        moo_objc_pop_autorelease_pool ();
        return FALSE;
    }

    editor = moo_editor_instance ();
    g_object_set (editor, "allow-empty-window", TRUE, "single-window", TRUE, NULL);

    klass = g_type_class_peek (MOO_TYPE_EDIT_WINDOW);
    moo_window_class_new_action (klass, "OpenProject", NULL,
                                 "display-name", "Open Project",
                                 "label", "Open Project",
                                 "stock-id", MOO_STOCK_OPEN_PROJECT,
                                 "closure-callback", open_project_cb,
                                 NULL);
    moo_window_class_new_action (klass, "ProjectOptions", NULL,
                                 "display-name", "Project _Options",
                                 "label", "Project Options",
                                 "stock-id", MOO_STOCK_PROJECT_OPTIONS,
                                 "closure-callback", project_options_cb,
                                 NULL);
    moo_window_class_new_action (klass, "CloseProject", NULL,
                                 "display-name", "Close Project",
                                 "label", "Close Project",
                                 "stock-id", MOO_STOCK_CLOSE_PROJECT,
                                 "closure-callback", close_project_cb,
                                 NULL);

    plugin->recent_list = moo_history_list_new (MP_RECENT_LIST_ID);
    g_signal_connect (plugin->recent_list, "activate-item",
                      G_CALLBACK (recent_item_activated), plugin);
    moo_window_class_new_action_custom (klass, "OpenRecentProject", NULL,
                                        (MooWindowActionFunc) create_recent_list_action,
                                        plugin, NULL);

    xml = moo_editor_get_ui_xml (editor);
    plugin->ui_merge_id = moo_ui_xml_new_merge_id (xml);
    moo_ui_xml_insert_markup_after (xml, plugin->ui_merge_id,
                                    "Editor/Menubar", "View",
                                    "<item name=\"Project\" _label=\"_Project\">"
                                    "  <item action=\"OpenProject\"/>"
                                    "  <item action=\"OpenRecentProject\"/>"
                                    "  <separator/>"
                                    "  <item action=\"ProjectOptions\"/>"
                                    "  <separator/>"
                                    "  <item action=\"CloseProject\"/>"
                                    "  <separator/>"
                                    "</item>");

    if (plugin->project_to_open)
    {
        [plugin->pm openProject:plugin->project_to_open];
        g_free (plugin->project_to_open);
        plugin->project_to_open = NULL;
    }

    moo_objc_pop_autorelease_pool ();
    return TRUE;
}

static void
project_plugin_deinit (ProjectPlugin *plugin)
{
    MooWindowClass *klass = g_type_class_peek (MOO_TYPE_EDIT_WINDOW);

    moo_window_class_remove_action (klass, "OpenProject");
    moo_window_class_remove_action (klass, "CloseProject");
    moo_window_class_remove_action (klass, "ProjectOptions");
    moo_window_class_remove_action (klass, "OpenRecentProject");

    if (plugin->ui_merge_id)
    {
        MooEditor *editor = moo_editor_instance ();
        MooUIXML *xml = moo_editor_get_ui_xml (editor);
        moo_ui_xml_remove_ui (xml, plugin->ui_merge_id);
        plugin->ui_merge_id = 0;
    }

    if (plugin->recent_list)
        g_object_unref (plugin->recent_list);
    plugin->recent_list = NULL;

    if (plugin->pm)
    {
        moo_objc_push_autorelease_pool ();

        [plugin->pm deinit];
        [plugin->pm release];
        plugin->pm = nil;

        moo_objc_pop_autorelease_pool ();
    }

    g_free (plugin->project_to_open);
    plugin->project_to_open = NULL;
}


MOO_PLUGIN_DEFINE_INFO (project, "Project", "Project manager",
                        "Yevgen Muntyan <muntyan@tamu.edu>",
                        MOO_VERSION, NULL)
MOO_PLUGIN_DEFINE_FULL (Project, project,
                        project_plugin_attach_window, project_plugin_detach_window,
                        NULL, NULL, NULL, 0, 0)

gboolean
_moo_project_plugin_init (void)
{
    MooPluginParams params = {TRUE, TRUE};

    if (!moo_plugin_register (PROJECT_PLUGIN_ID,
                              project_plugin_get_type (),
                              &project_plugin_info,
                              &params))
        return FALSE;

    moo_plugin_method_new ("open-project", project_plugin_get_type (),
                           G_CALLBACK (meth_open_project),
                           _moo_marshal_VOID__STRING,
                           G_TYPE_NONE, 1, G_TYPE_STRING);

    return TRUE;
}
