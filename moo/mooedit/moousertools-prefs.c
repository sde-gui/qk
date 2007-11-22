/*
 *   moousertools-prefs.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
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
#include "mooutils/moohelp.h"
#include "help-sections.h"
#include <string.h>

#define GET_WID(name) (moo_glade_xml_get_widget (page->xml, (name)))

enum {
    COLUMN_INFO,
    N_COLUMNS
};


static MooUserToolType
page_get_type (MooPrefsDialogPage *page)
{
    return GPOINTER_TO_INT (g_object_get_data (G_OBJECT (page), "moo-user-tool-type"));
}


static void
page_set_type (MooPrefsDialogPage *page,
               MooUserToolType     type)
{
    g_object_set_data (G_OBJECT (page), "moo-user-tool-type", GINT_TO_POINTER (type));
}


static gboolean
get_changed (MooPrefsDialogPage *page)
{
    return g_object_get_data (G_OBJECT (page), "moo-changed") != NULL;
}


static void
set_changed (MooPrefsDialogPage *page,
             gboolean            changed)
{
    g_object_set_data (G_OBJECT (page), "moo-changed", GINT_TO_POINTER (changed));
}


static MooCommandDisplay *
get_helper (MooPrefsDialogPage *page)
{
    return g_object_get_data (G_OBJECT (page), "moo-tree-helper");
}


static void
populate_store (GtkListStore   *store,
                MooUserToolType type)
{
    GtkTreeIter iter;
    GSList *list;

    list = _moo_edit_parse_user_tools (type);

    while (list)
    {
        MooUserToolInfo *info = list->data;

        if (info->os_type == MOO_USER_TOOL_THIS_OS)
        {
            gtk_list_store_append (store, &iter);
            gtk_list_store_set (store, &iter, COLUMN_INFO, info, -1);
        }

        _moo_user_tool_info_unref (info);
        list = g_slist_delete_link (list, list);
    }
}


static gboolean
new_row (MooPrefsDialogPage *page,
         GtkTreeModel       *model,
         GtkTreePath        *path)
{
    GtkTreeIter iter;
    MooUserToolInfo *info;
    GtkTreeViewColumn *column;

    info = _moo_user_tool_info_new ();
    info->cmd_factory = moo_command_factory_lookup ("lua");
    info->cmd_data = info->cmd_factory ? moo_command_data_new (info->cmd_factory->n_keys) : NULL;
    info->name = g_strdup (_("New Command"));
    info->options = g_strdup ("need-doc");
    info->enabled = TRUE;

    set_changed (page, TRUE);
    gtk_list_store_insert (GTK_LIST_STORE (model), &iter,
                           gtk_tree_path_get_indices(path)[0]);
    gtk_list_store_set (GTK_LIST_STORE (model), &iter, COLUMN_INFO, info, -1);

    column = gtk_tree_view_get_column (GET_WID ("treeview"), 0);
    gtk_tree_view_set_cursor (GET_WID ("treeview"), path, column, TRUE);

    _moo_user_tool_info_unref (info);
    return TRUE;
}

static gboolean
delete_row (MooPrefsDialogPage *page)
{
    set_changed (page, TRUE);
    return FALSE;
}

static gboolean
move_row (MooPrefsDialogPage *page)
{
    set_changed (page, TRUE);
    return FALSE;
}

static void
set_text (MooPrefsDialogPage *page,
          const char         *name,
          const char         *text)
{
    gtk_entry_set_text (GET_WID (name), text ? text : "");
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
        MooUserToolInfo *info;

        gtk_tree_model_get (model, iter, COLUMN_INFO, &info, -1);
        g_return_if_fail (info != NULL);

        _moo_command_display_set (helper, info->cmd_factory, info->cmd_data);

        gtk_toggle_button_set_active (GET_WID ("enabled"), info->enabled);
        set_text (page, "filter", info->filter);
        set_text (page, "options", info->options);

        _moo_user_tool_info_unref (info);
    }
    else
    {
        gtk_toggle_button_set_active (GET_WID ("enabled"), FALSE);
        set_text (page, "filter", NULL);
        set_text (page, "options", NULL);
        _moo_command_display_set (helper, NULL, NULL);
    }

    gtk_widget_set_sensitive (GET_WID ("tool_vbox"), path != NULL);
}


static gboolean
get_text (MooPrefsDialogPage *page,
          const char         *name,
          char              **dest)
{
    const char *text = gtk_entry_get_text (GET_WID (name));

    if (strcmp (*dest ? *dest : "", text) != 0)
    {
        g_free (*dest);
        *dest = text[0] ? g_strdup (text) : NULL;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static void
update_model (MooPrefsDialogPage *page,
              GtkTreeModel       *model,
              G_GNUC_UNUSED GtkTreePath *path,
              GtkTreeIter        *iter)
{
    MooCommandDisplay *helper;
    MooCommandFactory *factory;
    MooCommandData *data;
    MooUserToolInfo *info;
    gboolean changed = FALSE;

    helper = get_helper (page);
    gtk_tree_model_get (model, iter, COLUMN_INFO, &info, -1);
    g_return_if_fail (info != NULL);

    if (_moo_command_display_get (helper, &factory, &data))
    {
        moo_command_data_unref (info->cmd_data);
        info->cmd_data = moo_command_data_ref (data);
        info->cmd_factory = factory;
        gtk_list_store_set (GTK_LIST_STORE (model), iter, COLUMN_INFO, info, -1);
        changed = TRUE;
    }

    if (gtk_toggle_button_get_active (GET_WID ("enabled")) != info->enabled)
    {
        info->enabled = gtk_toggle_button_get_active (GET_WID ("enabled"));
        changed = TRUE;
    }

    changed = get_text (page, "filter", &info->filter) || changed;
    changed = get_text (page, "options", &info->options) || changed;

    if (changed)
    {
        g_print ("page changed\n");
        set_changed (page, TRUE);
    }

    _moo_user_tool_info_unref (info);
}


static void
name_data_func (G_GNUC_UNUSED GtkTreeViewColumn *column,
                GtkCellRenderer *cell,
                GtkTreeModel    *model,
                GtkTreeIter     *iter)
{
    MooUserToolInfo *info = NULL;

    gtk_tree_model_get (model, iter, COLUMN_INFO, &info, -1);
    g_return_if_fail (info != NULL);

    g_object_set (cell, "text", info->name, NULL);

    _moo_user_tool_info_unref (info);
}

static void
name_cell_edited (MooPrefsDialogPage *page,
                  char               *path_string,
                  char               *text)
{
    GtkTreeModel *model;
    GtkTreePath *path;
    GtkTreeIter iter;
    MooUserToolInfo *info = NULL;

    path = gtk_tree_path_new_from_string (path_string);
    model = gtk_tree_view_get_model (GET_WID ("treeview"));

    if (gtk_tree_model_get_iter (model, &iter, path))
    {
        gtk_tree_model_get (model, &iter, COLUMN_INFO, &info, -1);
        g_return_if_fail (info != NULL);

        if (strcmp (info->name ? info->name : "", text) != 0)
        {
            set_changed (page, TRUE);
            g_free (info->name);
            info->name = g_strdup (text);
        }
    }

    gtk_tree_path_free (path);
}

static void
command_page_init (MooPrefsDialogPage *page,
                   MooUserToolType     type)
{
    GtkTreeView *treeview;
    GtkListStore *store;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    MooCommandDisplay *helper;

    page_set_type (page, type);

    treeview = moo_glade_xml_get_widget (page->xml, "treeview");
    store = gtk_list_store_new (N_COLUMNS, MOO_TYPE_USER_TOOL_INFO);

    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_pack_start (column, cell, TRUE);
    gtk_tree_view_column_set_cell_data_func (column, cell,
                                             (GtkTreeCellDataFunc) name_data_func,
                                             NULL, NULL);
    gtk_tree_view_append_column (treeview, column);
    g_object_set (cell, "editable", TRUE, NULL);
    g_signal_connect_swapped (cell, "edited", G_CALLBACK (name_cell_edited), page);

    populate_store (store, type);

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
    g_signal_connect_swapped (helper, "delete-row", G_CALLBACK (delete_row), page);
    g_signal_connect_swapped (helper, "move-row", G_CALLBACK (move_row), page);
    g_signal_connect_swapped (helper, "update-widgets", G_CALLBACK (update_widgets), page);
    g_signal_connect_swapped (helper, "update-model", G_CALLBACK (update_model), page);

    gtk_tree_view_set_model (treeview, GTK_TREE_MODEL (store));

    _moo_tree_view_select_first (treeview);
    _moo_tree_helper_update_widgets (MOO_TREE_HELPER (helper));

    g_object_unref (store);
}


static void
command_page_apply (MooPrefsDialogPage *page)
{
    MooTreeHelper *helper;
    GtkTreeModel *model;
    GtkTreeIter iter;
    GSList *list = NULL;

    helper = g_object_get_data (G_OBJECT (page), "moo-tree-helper");
    _moo_tree_helper_update_model (helper, NULL, NULL);

    if (!get_changed (page))
        return;

    g_print ("apply\n");

    model = gtk_tree_view_get_model (GET_WID ("treeview"));

    if (gtk_tree_model_get_iter_first (model, &iter))
    {
        do
        {
            MooUserToolInfo *info;
            gtk_tree_model_get (model, &iter, COLUMN_INFO, &info, -1);
            list = g_slist_prepend (list, info);
        }
        while (gtk_tree_model_iter_next (model, &iter));
    }

    list = g_slist_reverse (list);

    _moo_edit_save_user_tools (page_get_type (page), list);
    _moo_edit_load_user_tools (page_get_type (page));

    g_slist_foreach (list, (GFunc) _moo_user_tool_info_unref, NULL);
    g_slist_free (list);

    set_changed (page, FALSE);
}


static void
main_page_init (MooPrefsDialogPage *main_page)
{
    MooPrefsDialogPage *page_context, *page_menu;

    page_menu = moo_glade_xml_get_widget (main_page->xml, "page_menu");
    page_context = moo_glade_xml_get_widget (main_page->xml, "page_context");

    command_page_init (page_menu, MOO_USER_TOOL_MENU);
    command_page_init (page_context, MOO_USER_TOOL_CONTEXT);
}


static void
main_page_apply (MooPrefsDialogPage *main_page)
{
    MooPrefsDialogPage *page_context, *page_menu;

    page_menu = moo_glade_xml_get_widget (main_page->xml, "page_menu");
    page_context = moo_glade_xml_get_widget (main_page->xml, "page_context");

    command_page_apply (page_menu);
    command_page_apply (page_context);
}


GtkWidget *
moo_user_tools_prefs_page_new (void)
{
    MooPrefsDialogPage *page;
    MooPrefsDialogPage *page_menu, *page_context;
    MooGladeXML *xml;

    xml = moo_glade_xml_new_empty (GETTEXT_PACKAGE);
    moo_glade_xml_map_id (xml, "page_menu", MOO_TYPE_PREFS_DIALOG_PAGE);
    moo_glade_xml_map_id (xml, "page_context", MOO_TYPE_PREFS_DIALOG_PAGE);
    page = moo_prefs_dialog_page_new_from_xml (_("Tools"), GTK_STOCK_EXECUTE,
                                               xml, mooedittools_glade_xml,
                                               "page", NULL);
    g_signal_connect (page, "init", G_CALLBACK (main_page_init), NULL);
    g_signal_connect (page, "apply", G_CALLBACK (main_page_apply), NULL);
    moo_help_set_id (GTK_WIDGET (page), HELP_SECTION_PREFS_USER_TOOLS);

    page_menu = moo_glade_xml_get_widget (xml, "page_menu");
    moo_prefs_dialog_page_fill_from_xml (page_menu, NULL, mooedittools_glade_xml,
                                         "page_command", NULL);

    page_context = moo_glade_xml_get_widget (xml, "page_context");
    moo_prefs_dialog_page_fill_from_xml (page_context, NULL, mooedittools_glade_xml,
                                         "page_command", NULL);

    g_object_unref (xml);
    return GTK_WIDGET (page);
}
