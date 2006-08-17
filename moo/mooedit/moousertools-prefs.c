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
    COLUMN_COMMAND,
    N_COLUMNS
};

enum {
    COMBO_COLUMN_NAME,
    COMBO_COLUMN_TYPE
};


static void     combo_changed   (MooPrefsDialogPage *page,
                                 GtkComboBox        *combo);
static void     block_combo     (MooPrefsDialogPage *page);
static void     unblock_combo   (MooPrefsDialogPage *page);

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


static void
init_type_combo (MooPrefsDialogPage *page)
{
    GtkListStore *store;
    GSList *types;
    GtkCellRenderer *cell;
    GtkComboBox *combo;

    combo = GET_WID ("type_combo");

    cell = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), cell, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo), cell,
                                    "text", COMBO_COLUMN_NAME, NULL);

    store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_POINTER);
    gtk_combo_box_set_model (combo, GTK_TREE_MODEL (store));

    types = moo_command_list_types ();

    while (types)
    {
        GtkTreeIter iter;
        char *type;
        MooCommandTypeInfo *info;
        GtkWidget *widget;

        type = types->data;
        info = moo_command_type_lookup (type);
        g_return_if_fail (info != NULL);

        widget = info->create_widget (info->data);
        g_return_if_fail (widget != NULL);

        gtk_widget_show (widget);
        gtk_notebook_append_page (GET_WID ("notebook"), widget, NULL);

        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter,
                            COMBO_COLUMN_NAME, info->name,
                            COMBO_COLUMN_TYPE, info,
                            -1);

        g_free (type);
        types = g_slist_delete_link (types, types);
    }

    g_signal_connect_swapped (combo, "changed", G_CALLBACK (combo_changed), page);

    g_object_unref (store);
}


static void
combo_set_type (MooPrefsDialogPage *page,
                const char         *type)
{
    GtkComboBox *combo = GET_WID ("type_combo");
    GtkTreeModel *model = gtk_combo_box_get_model (combo);

    if (type)
    {
        gboolean found = FALSE;
        int active;
        int index = 0;
        MooCommandTypeInfo *info;
        GtkTreeIter iter;

        if (gtk_tree_model_get_iter_first (model, &iter))
        {
            do
            {
                gtk_tree_model_get (model, &iter, COMBO_COLUMN_TYPE, &info, -1);

                if (!strcmp (info->id, type))
                {
                    found = TRUE;
                    break;
                }

                ++index;
            }
            while (gtk_tree_model_iter_next (model, &iter));
        }

        g_return_if_fail (found);

        active = gtk_combo_box_get_active (combo);

        if (active < 0 || active != index)
        {
            block_combo (page);
            gtk_combo_box_set_active (combo, index);
            gtk_notebook_set_current_page (GET_WID ("notebook"), index);
            gtk_widget_show (GET_WID ("notebook"));
            unblock_combo (page);
        }
    }
    else
    {
        block_combo (page);
        gtk_combo_box_set_active (combo, -1);
        gtk_widget_hide (GET_WID ("notebook"));
        unblock_combo (page);
    }
}


static void
block_combo (MooPrefsDialogPage *page)
{
    g_signal_handlers_block_by_func (GET_WID ("type_combo"),
                                     (gpointer) combo_changed,
                                     page);
}


static void
unblock_combo (MooPrefsDialogPage *page)
{
    g_signal_handlers_unblock_by_func (GET_WID ("type_combo"),
                                       (gpointer) combo_changed,
                                       page);
}


static void
combo_get_type (MooPrefsDialogPage  *page,
                const char         **type_p,
                MooCommandTypeInfo **info_p,
                GtkWidget          **widget_p)
{
    GtkTreeIter iter;
    GtkComboBox *combo = GET_WID ("type_combo");
    GtkTreeModel *model = gtk_combo_box_get_model (combo);
    int index = 0;
    MooCommandTypeInfo *info;

    index = gtk_combo_box_get_active (combo);

    if (index >= 0)
    {
        gtk_tree_model_iter_nth_child (model, &iter, NULL, index);
        gtk_tree_model_get (model, &iter, COMBO_COLUMN_TYPE, &info, -1);

        if (type_p)
            *type_p = info->id;
        if (info_p)
            *info_p = info;
        if (widget_p)
            *widget_p = gtk_notebook_get_nth_page (GET_WID ("notebook"), index);
    }
    else
    {
        if (type_p)
            *type_p = NULL;
        if (info_p)
            *info_p = NULL;
        if (widget_p)
            *widget_p = NULL;
    }
}


static void
combo_changed (MooPrefsDialogPage *page,
               GtkComboBox        *combo)
{
    GtkTreeView *treeview;
    GtkTreeSelection *selection;
    GtkTreeModel *model, *combo_model;
    GtkTreeIter iter;
    MooCommandTypeInfo *info = NULL;
    MooCommandData *data;
    GtkWidget *widget;

    treeview = GET_WID ("treeview");
    selection = gtk_tree_view_get_selection (treeview);
    combo_model = gtk_combo_box_get_model (combo);

    if (!gtk_tree_selection_get_selected (selection, &model, &iter))
        g_return_if_reached ();

    combo_get_type (page, NULL, &info, &widget);
    g_return_if_fail (info != NULL);

    set_changed (page, TRUE);
    gtk_tree_model_get (model, &iter, COLUMN_COMMAND, &data, -1);
    moo_command_data_clear (data);
    gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                        COLUMN_COMMAND_TYPE, info->id, -1);

    info->load_data (widget, data, info->data);

    moo_command_data_unref (data);
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
                        COLUMN_COMMAND, info->cmd_data,
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
                        COLUMN_COMMAND_TYPE, "MooScript",
                        COLUMN_COMMAND, cmd_data,
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
    gboolean enabled;
    char *name, *label, *type, *langs, *options;
    gboolean sensitive;
    MooCommandData *cmd_data;
    MooCommandTypeInfo *info;
    GtkWidget *widget;

    if (path)
    {
        gtk_tree_model_get (model, iter,
                            COLUMN_ENABLED, &enabled,
                            COLUMN_NAME, &name,
                            COLUMN_LABEL, &label,
                            COLUMN_LANGS, &langs,
                            COLUMN_OPTIONS, &options,
                            COLUMN_COMMAND_TYPE, &type,
                            COLUMN_COMMAND, &cmd_data,
                            -1);
        sensitive = TRUE;
    }
    else
    {
        enabled = FALSE;
        name = label = type = langs = options = NULL;
        sensitive = FALSE;
        cmd_data = NULL;
    }

    combo_set_type (page, type);

    if (type)
    {
        combo_get_type (page, NULL, &info, &widget);
        g_return_if_fail (info != NULL);
        info->load_data (widget, cmd_data, info->data);
    }

    gtk_toggle_button_set_active (GET_WID ("enabled"), enabled);
    gtk_entry_set_text (GET_WID ("name"), name ? name : "");
    gtk_entry_set_text (GET_WID ("label"), label ? label : "");
    gtk_entry_set_text (GET_WID ("langs"), langs ? langs : "");
    gtk_entry_set_text (GET_WID ("options"), options ? options : "");
    gtk_widget_set_sensitive (GET_WID ("tool_vbox"), sensitive);

    g_free (name);
    g_free (label);
    g_free (type);
    g_free (langs);
    g_free (options);

    if (cmd_data)
        moo_command_data_unref (cmd_data);
}

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
    gboolean old_enabled;
    char *old_name, *old_label, *old_type, *old_langs, *old_options;
    gboolean enabled;
    const char *name, *label, *type, *langs, *options;
    MooCommandData *data;
    MooCommandTypeInfo *info = NULL;
    GtkWidget *widget;

    enabled = gtk_toggle_button_get_active (GET_WID ("enabled"));
    name = gtk_entry_get_text (GET_WID ("name"));
    label = gtk_entry_get_text (GET_WID ("label"));
    langs = gtk_entry_get_text (GET_WID ("langs"));
    options = gtk_entry_get_text (GET_WID ("options"));

    combo_get_type (page, &type, &info, &widget);
    g_return_if_fail (info != NULL);

    gtk_tree_model_get (model, iter, COLUMN_COMMAND, &data, -1);

    if (info->save_data (widget, data, info->data))
        set_changed (page, TRUE);

    gtk_tree_model_get (model, iter,
                        COLUMN_ENABLED, &old_enabled,
                        COLUMN_NAME, &old_name,
                        COLUMN_LABEL, &old_label,
                        COLUMN_LANGS, &old_langs,
                        COLUMN_OPTIONS, &old_options,
                        COLUMN_COMMAND_TYPE, &old_type,
                        -1);

    if (enabled != old_enabled ||
        !string_equal (old_name, name) ||
        !string_equal (old_label, label) ||
        !string_equal (old_langs, langs) ||
        !string_equal (old_options, options) ||
        !string_equal (old_type, type))
    {
        gtk_list_store_set (GTK_LIST_STORE (model), iter,
                            COLUMN_ENABLED, enabled,
                            COLUMN_NAME, name,
                            COLUMN_LABEL, label,
                            COLUMN_LANGS, langs,
                            COLUMN_OPTIONS, options,
                            COLUMN_COMMAND_TYPE, type,
                            -1);
        set_changed (page, TRUE);
    }

    g_free (old_name);
    g_free (old_label);
    g_free (old_langs);
    g_free (old_options);
    g_free (old_type);
    moo_command_data_unref (data);
}


static void
page_init (MooPrefsDialogPage *page)
{
    GtkTreeView *treeview;
    GtkListStore *store;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    MooTreeHelper *helper;

    init_type_combo (page);

    treeview = moo_glade_xml_get_widget (page->xml, "treeview");
    store = gtk_list_store_new (N_COLUMNS, G_TYPE_BOOLEAN,
                                G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                                G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                                MOO_TYPE_COMMAND_DATA);
    gtk_tree_view_set_model (treeview, GTK_TREE_MODEL (store));

    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ("id", cell, "text", COLUMN_ID, NULL);
    gtk_tree_view_append_column (treeview, column);

    populate_store (store);
    _moo_tree_view_select_first (treeview);

    helper = _moo_tree_helper_new (GTK_WIDGET (treeview),
                                   moo_glade_xml_get_widget (page->xml, "new"),
                                   moo_glade_xml_get_widget (page->xml, "delete"),
                                   moo_glade_xml_get_widget (page->xml, "up"),
                                   moo_glade_xml_get_widget (page->xml, "down"));
    g_object_set_data_full (G_OBJECT (page), "moo-tree-helper", helper, g_object_unref);
    g_signal_connect_swapped (page, "destroy", G_CALLBACK (gtk_object_destroy), helper);

    g_signal_connect_swapped (helper, "new-row", G_CALLBACK (new_row), page);
    g_signal_connect_swapped (helper, "update-widgets", G_CALLBACK (update_widgets), page);
    g_signal_connect_swapped (helper, "update-model", G_CALLBACK (update_model), page);

    _moo_tree_helper_update_widgets (helper);

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
moo_user_tools_prefs_page_new (void)
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
