/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *   fileselector.c
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

#include <gmodule.h>
#include "mooedit/mooplugin.h"
#include "mooedit/moofileview/moofileview.h"
#include "mooedit/moofileview/moobookmarkmgr.h"
#include "mooedit/plugins/mooeditplugins.h"
#include "mooui/moouiobject.h"
#include "mooutils/moostock.h"

#ifndef MOO_VERSION
#define MOO_VERSION NULL
#endif

#define PLUGIN_ID "FileSelector"
#define DIR_PREFS MOO_PLUGIN_PREFS_ROOT "/" PLUGIN_ID "/last_dir"


typedef struct {
    MooBookmarkMgr *bookmark_mgr;
} Plugin;


static void
show_file_selector (MooEditWindow *window)
{
    GtkWidget *fileview;
    fileview = moo_edit_window_get_pane (window, PLUGIN_ID);
    moo_big_paned_present_pane (window->paned, fileview);
}


static gboolean
file_selector_plugin_init (void)
{
    GObjectClass *klass = g_type_class_ref (MOO_TYPE_EDIT_WINDOW);
    g_return_val_if_fail (klass != NULL, FALSE);

    moo_ui_object_class_new_action (klass,
                                    "id", "ShowFileSelector",
                                    "name", "Show File Selector",
                                    "label", "Show File Selector",
                                    "tooltip", "Show file selector",
                                    "icon-stock-id", MOO_STOCK_FILE_SELECTOR,
                                    "closure::callback", show_file_selector,
                                    NULL);

    moo_prefs_new_key_string (DIR_PREFS, NULL);

    g_type_class_unref (klass);
    return TRUE;
}


static void
file_selector_plugin_deinit (Plugin *plugin)
{
    /* XXX remove action */
    if (plugin->bookmark_mgr)
        g_object_unref (plugin->bookmark_mgr);
    plugin->bookmark_mgr = NULL;
}


/* XXX */
static gboolean
file_selector_go_home (MooFileView *fileview)
{
    const char *dir;
    char *real_dir = NULL;

    if (!MOO_IS_FILE_VIEW (fileview))
        return FALSE;

    dir = moo_prefs_get_string (DIR_PREFS);

    if (dir)
        real_dir = g_filename_from_utf8 (dir, -1, NULL, NULL, NULL);

    if (!real_dir || !moo_file_view_chdir (fileview, real_dir, NULL))
        g_signal_emit_by_name (fileview, "go-home");

    g_free (real_dir);
    return FALSE;
}


static void
fileview_chdir (MooFileView *fileview)
{
    char *dir = NULL;
    char *utf8_dir = NULL;

    g_object_get (fileview, "current-directory", &dir, NULL);

    if (!dir)
    {
        moo_prefs_set (DIR_PREFS, NULL);
        return;
    }

    utf8_dir = g_filename_to_utf8 (dir, -1, NULL, NULL, NULL);
    moo_prefs_set_string (DIR_PREFS, utf8_dir);

    g_free (utf8_dir);
    g_free (dir);
}


static void
fileview_activate (MooEditWindow    *window,
                   const char       *path)
{
    moo_editor_open_file (moo_edit_window_get_editor (window),
                          window, NULL, path, NULL);
}


static void
file_selector_plugin_attach (MooEditWindow *window,
                             Plugin        *plugin)
{
    GtkWidget *fileview;
    MooEditor *editor;
    MooPaneLabel *label;

    editor = moo_edit_window_get_editor (window);

    if (!plugin->bookmark_mgr)
        plugin->bookmark_mgr = moo_bookmark_mgr_new ();

    fileview = g_object_new (MOO_TYPE_FILE_VIEW,
                             "filter-mgr", moo_editor_get_filter_mgr (editor),
                             "bookmark-mgr", plugin->bookmark_mgr,
                             NULL);

    g_idle_add ((GSourceFunc) file_selector_go_home, fileview);
    g_signal_connect (fileview, "notify::current-directory",
                      G_CALLBACK (fileview_chdir), NULL);
    g_signal_connect_swapped (fileview, "activate",
                              G_CALLBACK (fileview_activate),
                              window);

    label = moo_pane_label_new (MOO_STOCK_FILE_SELECTOR,
                                NULL, NULL, "File Selector");
    moo_edit_window_add_pane (window, PLUGIN_ID, fileview,
                              label, MOO_PANE_POS_LEFT);
}


static void
file_selector_plugin_detach (MooEditWindow *window)
{
    GtkWidget *fileview = moo_edit_window_get_pane (window, PLUGIN_ID);

    g_return_if_fail (fileview != NULL);

    g_signal_handlers_disconnect_by_func (fileview,
                                          (gpointer) fileview_chdir,
                                          NULL);
    g_signal_handlers_disconnect_by_func (fileview,
                                          (gpointer) fileview_activate,
                                          window);

    moo_edit_window_remove_pane (window, PLUGIN_ID);
}


gboolean
moo_file_selector_init (void)
{
    MooPluginParams params = { TRUE };
    MooPluginPrefsParams prefs_params;

    MooPluginInfo info = {
        MOO_PLUGIN_CURRENT_VERSION,

        PLUGIN_ID,
        "File Selector",
        "File Selector",
        "Yevgen Muntyan <muntyan@tamu.edu>",
        MOO_VERSION,

        (MooPluginInitFunc) file_selector_plugin_init,
        (MooPluginDeinitFunc) file_selector_plugin_deinit,
        (MooPluginWindowAttachFunc) file_selector_plugin_attach,
        (MooPluginWindowDetachFunc) file_selector_plugin_detach,

        &params,
        &prefs_params
    };

    static Plugin plugin;

    return moo_plugin_register (&info, &plugin);
}
