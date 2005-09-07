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
#include "mooedit/mooeditplugin.h"
#include "mooedit/moofileview/moofileview.h"
#include "mooedit/moofileview/moobookmarkmgr.h"
#include "mooui/moouiobject.h"
#include "mooutils/moostock.h"

#ifndef MOO_VERSION
#define MOO_VERSION NULL
#endif

#define FILE_SELECTOR_PLUGIN_ID "FileSelector"
#define FILE_SELECTOR_DIR_PREFS MOO_EDIT_PLUGIN_PREFS_ROOT "/" FILE_SELECTOR_PLUGIN_ID "/last_dir"

gboolean fileselector_init (void);


typedef struct {
    MooEditWindow *window;
    MooFileView *fileview;
} FileSelectorPluginStuff;

typedef struct {
    MooBookmarkMgr *bookmark_mgr;
} FileSelectorPluginGlobalStuff;


static void
show_file_selector (MooEditWindow *window)
{
    MooEditPluginWindowData *data;
    FileSelectorPluginStuff *stuff;

    data = moo_edit_plugin_get_window_data (window, FILE_SELECTOR_PLUGIN_ID);
    g_return_if_fail (data != NULL && data->data != NULL);
    stuff = data->data;

    moo_big_paned_present_pane (window->paned,
                                GTK_WIDGET (stuff->fileview));
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

    moo_prefs_new_key_string (FILE_SELECTOR_DIR_PREFS, NULL);

    g_type_class_unref (klass);
    return TRUE;
}


static void
file_selector_plugin_deinit (FileSelectorPluginGlobalStuff *global_stuff)
{
    /* XXX remove action */
    if (global_stuff->bookmark_mgr)
        g_object_unref (global_stuff->bookmark_mgr);
    global_stuff->bookmark_mgr = NULL;
}


static void
file_selector_plugin_attach (MooEditPluginWindowData *plugin_window_data)
{
    FileSelectorPluginStuff *stuff;

    stuff = g_new0 (FileSelectorPluginStuff, 1);
    stuff->window = plugin_window_data->window;
    stuff->fileview = NULL;

    plugin_window_data->data = stuff;
}


static void
file_selector_plugin_detach (MooEditPluginWindowData *plugin_window_data)
{
    g_free (plugin_window_data->data);
    plugin_window_data->data = NULL;
}


/* XXX */
static gboolean
file_selector_go_home (MooFileView *fileview)
{
    const char *dir;
    char *real_dir = NULL;

    if (!MOO_IS_FILE_VIEW (fileview))
        return FALSE;

    dir = moo_prefs_get_string (FILE_SELECTOR_DIR_PREFS);

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
        moo_prefs_set (FILE_SELECTOR_DIR_PREFS, NULL);
        return;
    }

    utf8_dir = g_filename_to_utf8 (dir, -1, NULL, NULL, NULL);
    moo_prefs_set_string (FILE_SELECTOR_DIR_PREFS, utf8_dir);

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


static gboolean
file_selector_plugin_pane_create (MooEditPluginWindowData        *plugin_window_data,
                                  MooPaneLabel                  **label,
                                  GtkWidget                     **widget,
                                  FileSelectorPluginGlobalStuff  *global_stuff)
{
    GtkWidget *fileview;
    FileSelectorPluginStuff *stuff;
    MooEditor *editor;

    g_return_val_if_fail (global_stuff != NULL, FALSE);
    g_return_val_if_fail (plugin_window_data->data != NULL, FALSE);
    stuff = plugin_window_data->data;

    editor = moo_edit_window_get_editor (plugin_window_data->window);

    if (!global_stuff->bookmark_mgr)
        global_stuff->bookmark_mgr = moo_bookmark_mgr_new ();

    fileview = g_object_new (MOO_TYPE_FILE_VIEW,
                             "filter-mgr", moo_editor_get_filter_mgr (editor),
                             "bookmark-mgr", global_stuff->bookmark_mgr,
                             NULL);

    gtk_object_sink (GTK_OBJECT (g_object_ref (fileview)));

    g_idle_add ((GSourceFunc) file_selector_go_home, fileview);
    g_signal_connect (fileview, "notify::current-directory",
                      G_CALLBACK (fileview_chdir), NULL);
    g_signal_connect_swapped (fileview, "activate",
                              G_CALLBACK (fileview_activate),
                              plugin_window_data->window);

    *widget = fileview;
    *label = moo_pane_label_new (MOO_STOCK_FILE_SELECTOR,
                                 NULL, NULL, "File Selector");

    stuff->fileview = MOO_FILE_VIEW (fileview);
    return TRUE;
}


static void
file_selector_plugin_pane_destroy (MooEditPluginWindowData *plugin_window_data)
{
    FileSelectorPluginStuff *stuff;

    g_return_if_fail (plugin_window_data->data != NULL);
    stuff = plugin_window_data->data;

    g_return_if_fail (stuff->fileview != NULL);
    g_signal_handlers_disconnect_by_func (stuff->fileview,
                                          (gpointer) fileview_chdir,
                                          NULL);
    g_signal_handlers_disconnect_by_func (stuff->fileview,
                                          (gpointer) fileview_activate,
                                          stuff->window);
}


G_MODULE_EXPORT gboolean
fileselector_init (void)
{
    MooEditPluginParams params = { TRUE, TRUE, MOO_PANE_POS_LEFT };
    MooEditPluginPrefsParams prefs_params = { NULL };

    MooEditPluginInfo info = {
        MOO_EDIT_PLUGIN_CURRENT_VERSION,

        FILE_SELECTOR_PLUGIN_ID,
        "File Selector",
        "File Selector",
        "Yevgen Muntyan <muntyan@tamu.edu>",
        MOO_VERSION,

        (MooEditPluginInitFunc) file_selector_plugin_init,
        (MooEditPluginDeinitFunc) file_selector_plugin_deinit,
        (MooEditPluginWindowAttachFunc) file_selector_plugin_attach,
        (MooEditPluginWindowDetachFunc) file_selector_plugin_detach,
        (MooEditPluginPaneCreateFunc) file_selector_plugin_pane_create,
        (MooEditPluginPaneDestroyFunc) file_selector_plugin_pane_destroy,

        &params,
        &prefs_params
    };

    static FileSelectorPluginGlobalStuff stuff = { NULL };

    return moo_edit_plugin_register (&info, &stuff);
}
