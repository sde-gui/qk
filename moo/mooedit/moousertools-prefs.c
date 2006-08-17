/*
 *   moousertools-prefs.c
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

#include "mooedit/moousertools-prefs.h"
#include "mooedit/moousertools.h"
#include "mooedit/mooedittools-glade.h"
#include "mooedit/moocommand.h"
#include "mooedit/moocommanddisplay.h"
#include "mooutils/mooprefsdialogpage.h"
#include "mooutils/mooi18n.h"
#include "mooutils/mooutils-treeview.h"
#include <string.h>

#define GET_WID(name) (moo_glade_xml_get_widget (page->xml, (name)))

enum {
    COLUMN_ENABLED,
    COLUMN_ID,
    COLUMN_NAME,
    COLUMN_LABEL,
    COLUMN_LANGS,
    COLUMN_OPTIONS,
    COLUMN_COMMAND_TYPE,
    COLUMN_COMMAND_DATA,
    N_COLUMNS
};


static gboolean get_changed     (MooPrefsDialogPage *page);
static void     set_changed     (MooPrefsDialogPage *page,
                                 gboolean            changed);


static gboolean
get_changed (MooPrefsDialogPage *page)
{
    return g_object_get_data (G_OBJECT (page), "moo-changed") != NULL;
}


static void
set_changed (MooPrefsDialogPage *page,
             gboolean            changed)
{
    g_object_set_data (G_OBJECT (page), "moo-changed",
                       GINT_TO_POINTER (changed));
}


static MooCommandDisplay *
get_helper (MooPrefsDialogPage *page)
{
    return g_object_get_data (G_OBJECT (page), "moo-tree-helper");
}


static void
tool_parse_func (MooToolLoadInfo *info,
                 gpointer         data)
{
    GtkTreeIter iter;
    GtkListStore *store = data;

#ifdef __WIN32__
    if (info->os_type != MOO_TOOL_WINDOWS)
        return;
#else
    if (info->os_type != MOO_TOOL_UNIX)
        return;
#endif

    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter,
                        COLUMN_ENABLED, info->enabled,
                        COLUMN_ID, info->id,
                        COLUMN_NAME, info->name,
                        COLUMN_LABEL, info->label,
                        COLUMN_LANGS, info->langs,
                        COLUMN_OPTIONS, info->options,
                        COLUMN_COMMAND_TYPE, info->cmd_type,
                        COLUMN_COMMAND_DATA, info->cmd_data,
                        -1);
}

static void
populate_store (GtkListStore *store)
{
    _moo_edit_parse_user_tools (MOO_TOOL_FILE_TOOLS,
                                tool_parse_func,
                                store);
}


static gboolean
new_row (MooPrefsDialogPage *page,
         GtkTreeModel       *model,
         GtkTreePath        *path)
{
    GtkTreeIter iter;
    MooCommandData *cmd_data;

    cmd_data = moo_command_data_new ();

    set_changed (page, TRUE);
    gtk_list_store_insert (GTK_LIST_STORE (model), &iter,
                           gtk_tree_path_get_indices(path)[0]);
    gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                        COLUMN_ENABLED, TRUE,
                        COLUMN_ID, "New Tool",
                        COLUMN_NAME, "New Tool",
                        COLUMN_LABEL, "New Tool",
                        COLUMN_COMMAND_TYPE, moo_command_type_lookup ("MooScript"),
                        COLUMN_COMMAND_DATA, cmd_data,
                        -1);

    moo_command_data_unref (cmd_data);
    return TRUE;
}


static void
update_widgets (MooPrefsDialogPage *page,
                GtkTreeModel       *model,
                GtkTreePath        *path,
                GtkTreeIter        *iter)
{
    MooCommandDisplay *helper;

    helper = get_helper (page);

    if (path)
    {
        MooCommandType *type;
        MooCommandData *data;
        gtk_tree_model_get (model, iter,
                            COLUMN_COMMAND_TYPE, &type,
                            COLUMN_COMMAND_DATA, &data,
                            -1);
        _moo_command_display_set (helper, type, data);
        g_object_unref (type);
        moo_command_data_unref (data);
    }
    else
    {
        _moo_command_display_set (helper, NULL, NULL);
    }
}
// {
//     gboolean enabled;
//     char *name, *label, *langs, *options;
//     gboolean sensitive;
//     MooCommandData *cmd_data;
//     MooCommandType *cmd_type;
//     GtkWidget *widget;
//
//     if (path)
//     {
//         gtk_tree_model_get (model, iter,
//                             COLUMN_ENABLED, &enabled,
//                             COLUMN_NAME, &name,
//                             COLUMN_LABEL, &label,
//                             COLUMN_LANGS, &langs,
//                             COLUMN_OPTIONS, &options,
//                             COLUMN_COMMAND_TYPE, &cmd_type,
//                             COLUMN_COMMAND_DATA, &cmd_data,
//                             -1);
//         sensitive = TRUE;
//     }
//     else
//     {
//         enabled = FALSE;
//         name = label = langs = options = NULL;
//         sensitive = FALSE;
//         cmd_data = NULL;
//         cmd_type = NULL;
//     }
//
//     combo_set_type (page, cmd_type);
//
//     if (cmd_type)
//     {
//         combo_get_type (page, NULL, &widget);
//         g_return_if_fail (widget != NULL);
//         _moo_command_type_load_data (cmd_type, widget, cmd_data);
//     }
//
//     gtk_toggle_button_set_active (GET_WID ("enabled"), enabled);
//     gtk_entry_set_text (GET_WID ("name"), name ? name : "");
//     gtk_entry_set_text (GET_WID ("label"), label ? label : "");
//     gtk_entry_set_text (GET_WID ("langs"), langs ? langs : "");
//     gtk_entry_set_text (GET_WID ("options"), options ? options : "");
//     gtk_widget_set_sensitive (GET_WID ("tool_vbox"), sensitive);
//
//     g_free (name);
//     g_free (label);
//     g_free (langs);
//     g_free (options);
//
//     if (cmd_data)
//         moo_command_data_unref (cmd_data);
//     if (cmd_type)
//         g_object_unref (cmd_type);
// }

static gboolean
string_equal (const char *s1,
              const char *s2)
{
    return !strcmp (s1 ? s1 : "", s2 ? s2 : "");
}

static void
update_model (MooPrefsDialogPage *page,
              GtkTreeModel       *model,
              G_GNUC_UNUSED GtkTreePath *path,
              GtkTreeIter        *iter)
{
    MooCommandDisplay *helper;
    MooCommandType *type;
    MooCommandData *data;

    helper = get_helper (page);

    if (_moo_command_display_get (helper, &type, &data))
    {
        gtk_list_store_set (GTK_LIST_STORE (model), iter,
                            COLUMN_COMMAND_TYPE, type,
                            COLUMN_COMMAND_DATA, data,
                            -1);
        set_changed (page, TRUE);
    }

//     gboolean old_enabled;
//     char *old_name, *old_label, *old_type, *old_langs, *old_options;
//     gboolean enabled;
//     const char *name, *label, *type, *langs, *options;
//     MooCommandData *data;
//     MooCommandTypeInfo *info = NULL;
//     GtkWidget *widget;
//
//     enabled = gtk_toggle_button_get_active (GET_WID ("enabled"));
//     name = gtk_entry_get_text (GET_WID ("name"));
//     label = gtk_entry_get_text (GET_WID ("label"));
//     langs = gtk_entry_get_text (GET_WID ("langs"));
//     options = gtk_entry_get_text (GET_WID ("options"));
//
//     combo_get_type (page, &type, &info, &widget);
//     g_return_if_fail (info != NULL);
//
//     gtk_tree_model_get (model, iter, COLUMN_COMMAND, &data, -1);
//
//     if (info->save_data (widget, data, info->data))
//         set_changed (page, TRUE);
//
//     gtk_tree_model_get (model, iter,
//                         COLUMN_ENABLED, &old_enabled,
//                         COLUMN_NAME, &old_name,
//                         COLUMN_LABEL, &old_label,
//                         COLUMN_LANGS, &old_langs,
//                         COLUMN_OPTIONS, &old_options,
//                         COLUMN_COMMAND_TYPE, &old_type,
//                         -1);
//
//     if (enabled != old_enabled ||
//         !string_equal (old_name, name) ||
//         !string_equal (old_label, label) ||
//         !string_equal (old_langs, langs) ||
//         !string_equal (old_options, options) ||
//         !string_equal (old_type, type))
//     {
//         gtk_list_store_set (GTK_LIST_STORE (model), iter,
//                             COLUMN_ENABLED, enabled,
//                             COLUMN_NAME, name,
//                             COLUMN_LABEL, label,
//                             COLUMN_LANGS, langs,
//                             COLUMN_OPTIONS, options,
//                             COLUMN_COMMAND_TYPE, type,
//                             -1);
//         set_changed (page, TRUE);
//     }
//
//     g_free (old_name);
//     g_free (old_label);
//     g_free (old_langs);
//     g_free (old_options);
//     g_free (old_type);
//     moo_command_data_unref (data);
}


static void
page_init (MooPrefsDialogPage *page)
{
    GtkTreeView *treeview;
    GtkListStore *store;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    MooCommandDisplay *helper;

    treeview = moo_glade_xml_get_widget (page->xml, "treeview");
    store = gtk_list_store_new (N_COLUMNS, G_TYPE_BOOLEAN, G_TYPE_STRING,
                                G_TYPE_STRING, G_TYPE_STRING,
                                G_TYPE_STRING, G_TYPE_STRING,
                                MOO_TYPE_COMMAND_TYPE,
                                MOO_TYPE_COMMAND_DATA);
    gtk_tree_view_set_model (treeview, GTK_TREE_MODEL (store));

    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ("id", cell, "text", COLUMN_ID, NULL);
    gtk_tree_view_append_column (treeview, column);

    populate_store (store);

    helper = _moo_command_display_new (GET_WID ("type_combo"),
                                       GET_WID ("notebook"),
                                       GTK_WIDGET (treeview),
                                       GET_WID ("new"),
                                       GET_WID ("delete"),
                                       GET_WID ("up"),
                                       GET_WID ("down"));
    g_object_set_data_full (G_OBJECT (page), "moo-tree-helper", helper, g_object_unref);
    g_signal_connect_swapped (page, "destroy", G_CALLBACK (gtk_object_destroy), helper);

    g_signal_connect_swapped (helper, "new-row", G_CALLBACK (new_row), page);
    g_signal_connect_swapped (helper, "update-widgets", G_CALLBACK (update_widgets), page);
    g_signal_connect_swapped (helper, "update-model", G_CALLBACK (update_model), page);

    _moo_tree_view_select_first (treeview);
    _moo_tree_helper_update_widgets (MOO_TREE_HELPER (helper));

    g_object_unref (store);
}


static void
page_apply (MooPrefsDialogPage *page)
{
    MooTreeHelper *helper;

    helper = g_object_get_data (G_OBJECT (page), "moo-tree-helper");
    _moo_tree_helper_update_model (helper, NULL, NULL);

    if (!get_changed (page))
        return;

    g_print ("apply\n");

    set_changed (page, FALSE);
}


GtkWidget *
_moo_user_tools_prefs_page_new (void)
{
    MooPrefsDialogPage *page;
    MooGladeXML *xml;

    xml = moo_glade_xml_new_empty (GETTEXT_PACKAGE);
    page = moo_prefs_dialog_page_new_from_xml (_("Tools"), GTK_STOCK_EXECUTE,
                                               xml, MOO_EDIT_TOOLS_GLADE_XML, -1,
                                               "page", NULL);

    g_signal_connect (page, "init", G_CALLBACK (page_init), NULL);
    g_signal_connect (page, "apply", G_CALLBACK (page_apply), NULL);

    g_object_unref (xml);
    return GTK_WIDGET (page);
}
