/*
 *   ctags-plugin.c
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
#include "mooutils/mooi18n.h"
#include "mooutils/mooutils-gobject.h"
#include "ctags-view.h"
#include "ctags-doc.h"
#include <gtk/gtk.h>

#define CTAGS_PLUGIN_ID "Ctags"

typedef struct {
    MooPlugin parent;
    guint ui_merge_id;
} CtagsPlugin;

typedef struct {
    MooWinPlugin parent;
    MooCtagsView *view;
    guint update_idle;
} CtagsWindowPlugin;


static gboolean
window_plugin_update (CtagsWindowPlugin *plugin)
{
    MooEditWindow *window;
    MooEdit *doc;
    MooCtagsDocPlugin *dp;
    GtkTreeModel *model;

    plugin->update_idle = 0;

    window = MOO_WIN_PLUGIN (plugin)->window;
    doc = moo_edit_window_get_active_doc (window);

    if (!doc)
        return FALSE;

    dp = moo_doc_plugin_lookup (CTAGS_PLUGIN_ID, doc);
    g_return_val_if_fail (MOO_IS_CTAGS_DOC_PLUGIN (dp), FALSE);

    model = _moo_ctags_doc_plugin_get_store (dp);
    gtk_tree_view_set_model (GTK_TREE_VIEW (plugin->view), model);
    gtk_tree_view_expand_all (GTK_TREE_VIEW (plugin->view));

    return FALSE;
}

static void
active_doc_changed (CtagsWindowPlugin *plugin)
{
    if (!plugin->update_idle)
        plugin->update_idle =
            g_idle_add_full (G_PRIORITY_LOW,
                             (GSourceFunc) window_plugin_update,
                             plugin, NULL);
}

static void
entry_activated (CtagsWindowPlugin *plugin,
                 MooCtagsEntry     *entry)
{
    MooEditWindow *window;
    MooEdit *doc;

    window = MOO_WIN_PLUGIN (plugin)->window;
    doc = moo_edit_window_get_active_doc (window);

    if (doc && entry->line >= 0)
        moo_text_view_move_cursor (doc, entry->line, -1, FALSE, FALSE);
}

static gboolean
ctags_window_plugin_create (CtagsWindowPlugin *plugin)
{
    GtkWidget *swin, *view;
    MooPaneLabel *label;
    MooEditWindow *window = MOO_WIN_PLUGIN (plugin)->window;

    view = _moo_ctags_view_new ();
    g_return_val_if_fail (view != NULL, FALSE);
    plugin->view = MOO_CTAGS_VIEW (view);
    g_signal_connect_swapped (view, "activate-entry",
                              G_CALLBACK (entry_activated),
                              plugin);

    swin = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swin),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    gtk_container_add (GTK_CONTAINER (swin), GTK_WIDGET (plugin->view));
    gtk_widget_show_all (swin);

    label = moo_pane_label_new (NULL, NULL, "Functions", "Functions");
    moo_edit_window_add_pane (window, CTAGS_PLUGIN_ID,
                              swin, label, MOO_PANE_POS_RIGHT);
    moo_pane_label_free (label);

    g_signal_connect_swapped (window, "notify::active-doc",
                              G_CALLBACK (active_doc_changed),
                              plugin);
    active_doc_changed (plugin);

    return TRUE;
}

static void
ctags_window_plugin_destroy (CtagsWindowPlugin *plugin)
{
    MooEditWindow *window = MOO_WIN_PLUGIN(plugin)->window;

    moo_edit_window_remove_pane (window, CTAGS_PLUGIN_ID);

    g_signal_handlers_disconnect_by_func (window,
                                          (gpointer) active_doc_changed,
                                          plugin);
    if (plugin->update_idle)
        g_source_remove (plugin->update_idle);
    plugin->update_idle = 0;
}


static void
show_functions_pane (MooEditWindow *window)
{
    GtkWidget *pane;
    pane = moo_edit_window_get_pane (window, CTAGS_PLUGIN_ID);
    moo_big_paned_present_pane (window->paned, pane);
}

static gboolean
ctags_plugin_init (CtagsPlugin *plugin)
{
    MooWindowClass *klass = g_type_class_ref (MOO_TYPE_EDIT_WINDOW);
    MooEditor *editor = moo_editor_instance ();
    MooUIXML *xml = moo_editor_get_ui_xml (editor);

    g_return_val_if_fail (klass != NULL, FALSE);
    g_return_val_if_fail (editor != NULL, FALSE);

    moo_window_class_new_action (klass, "ShowFunctions", NULL,
                                 "display-name", "Show Functions",
                                 "label", "Show Functions",
                                 "tooltip", "Show functions list",
                                 "closure-callback", show_functions_pane,
                                 NULL);

    if (xml)
    {
        plugin->ui_merge_id = moo_ui_xml_new_merge_id (xml);
        moo_ui_xml_add_item (xml, plugin->ui_merge_id,
                             "Editor/Menubar/View",
                             "ShowFunctions", "ShowFunctions", -1);
    }

    g_type_class_unref (klass);
    return TRUE;
}

static void
ctags_plugin_deinit (CtagsPlugin *plugin)
{
    MooWindowClass *klass = g_type_class_ref (MOO_TYPE_EDIT_WINDOW);
    MooEditor *editor = moo_editor_instance ();
    MooUIXML *xml = moo_editor_get_ui_xml (editor);

    moo_window_class_remove_action (klass, "CtagsCtags");

    if (plugin->ui_merge_id)
        moo_ui_xml_remove_ui (xml, plugin->ui_merge_id);
    plugin->ui_merge_id = 0;

    g_type_class_unref (klass);
}


MOO_PLUGIN_DEFINE_INFO (ctags, "Ctags", "Shows functions in the open document",
                        "Yevgen Muntyan <muntyan@tamu.edu>\n"
                        "Christian Dywan <christian@twotoasts.de>",
                        MOO_VERSION, NULL)
MOO_WIN_PLUGIN_DEFINE (Ctags, ctags)
MOO_PLUGIN_DEFINE_FULL (Ctags, ctags,
                        NULL, NULL, NULL, NULL, NULL,
                        ctags_window_plugin_get_type (),
                        MOO_TYPE_CTAGS_DOC_PLUGIN)

gboolean
_moo_ctags_plugin_init (void)
{
    MooPluginParams params = {FALSE, TRUE};
    return moo_plugin_register (CTAGS_PLUGIN_ID,
                                ctags_plugin_get_type (),
                                &ctags_plugin_info,
                                &params);
}
