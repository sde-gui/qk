/*
 *   moofileselector-prefs.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#include "moofileselector.h"
#include "plugins/moofileselector-prefs-glade.h"
#include "mooutils/mooprefsdialogpage.h"
#include "mooutils/mooutils-treeview.h"
#include "mooutils/mooi18n.h"
#include "mooutils/moostock.h"
#include "mooutils/moohelp.h"
#include "help-sections.h"
#include <gtk/gtk.h>
#include <string.h>


enum {
    COLUMN_LABEL,
    COLUMN_COMMAND,
    COLUMN_EXTENSIONS,
    COLUMN_MIMETYPES,
    N_COLUMNS
};


static void     prefs_page_apply        (MooGladeXML    *xml);
static void     prefs_page_init         (MooGladeXML    *xml);

static gboolean helper_new_row          (MooTreeHelper  *helper,
                                         GtkTreeModel   *model,
                                         GtkTreePath    *path);
static gboolean helper_delete_row       (MooTreeHelper  *helper,
                                         GtkTreeModel   *model,
                                         GtkTreePath    *path);
static gboolean helper_move_row         (MooTreeHelper  *helper,
                                         GtkTreeModel   *model,
                                         GtkTreePath    *old_path,
                                         GtkTreePath    *new_path);
static void     helper_update_widgets   (MooGladeXML    *xml,
                                         GtkTreeModel   *model,
                                         GtkTreePath    *path,
                                         GtkTreeIter    *iter);
static void     helper_update_model     (MooTreeHelper  *helper,
                                         GtkTreeModel   *model,
                                         GtkTreePath    *path,
                                         GtkTreeIter    *iter,
                                         MooGladeXML    *xml);


GtkWidget *
_moo_file_selector_prefs_page (MooPlugin *plugin)
{
    MooPrefsDialogPage *page;
    MooGladeXML *xml;
    GtkWidget *treeview;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    GtkListStore *store;
    MooTreeHelper *helper;

    xml = moo_glade_xml_new_empty (GETTEXT_PACKAGE);
    page = moo_prefs_dialog_page_new_from_xml (_("File Selector"), MOO_STOCK_FILE_SELECTOR,
                                               xml, moofileselector_prefs_glade_xml,
                                               "page", NULL);

    g_signal_connect_swapped (page, "apply", G_CALLBACK (prefs_page_apply), xml);
    g_signal_connect_swapped (page, "init", G_CALLBACK (prefs_page_init), xml);
    moo_help_set_id (GTK_WIDGET (page), HELP_SECTION_PREFS_FILE_SELECTOR);

    treeview = moo_glade_xml_get_widget (xml, "treeview");

    column = gtk_tree_view_column_new ();
    gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

    cell = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (column, cell, TRUE);
    gtk_tree_view_column_set_attributes (column, cell, "text", COLUMN_LABEL, NULL);

    store = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING,
                                G_TYPE_STRING, G_TYPE_STRING);
    gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), GTK_TREE_MODEL (store));
    g_object_unref (store);

    helper = _moo_tree_helper_new (treeview,
                                   moo_glade_xml_get_widget (xml, "new"),
                                   moo_glade_xml_get_widget (xml, "delete"),
                                   moo_glade_xml_get_widget (xml, "up"),
                                   moo_glade_xml_get_widget (xml, "down"));

    g_signal_connect (helper, "new-row", G_CALLBACK (helper_new_row), NULL);
    g_signal_connect (helper, "delete-row", G_CALLBACK (helper_delete_row), NULL);
    g_signal_connect (helper, "move-row", G_CALLBACK (helper_move_row), NULL);
    g_signal_connect_swapped (helper, "update-widgets",
                              G_CALLBACK (helper_update_widgets), xml);
    g_signal_connect (helper, "update-model",
                      G_CALLBACK (helper_update_model), xml);
    g_object_set_data_full (G_OBJECT (xml), "moo-tree-helper", helper, g_object_unref);

    g_object_set_data (G_OBJECT (xml), "moo-file-selector-plugin", plugin);
    g_object_unref (xml);
    return GTK_WIDGET (page);
}


static void
save_store (MooTreeHelper *helper,
            GtkTreeModel  *model)
{
    MooMarkupDoc *doc;
    MooMarkupNode *root;
    GtkTreeIter iter;

    if (!_moo_tree_helper_get_modified (helper))
        return;

    _moo_tree_helper_set_modified (helper, FALSE);

    doc = moo_prefs_get_markup (MOO_PREFS_RC);
    g_return_if_fail (doc != NULL);

    root = moo_markup_get_element (MOO_MARKUP_NODE (doc), "MooFileView/Tools");

    if (root)
        moo_markup_delete_node (root);

    if (!gtk_tree_model_get_iter_first (model, &iter))
        return;

    root = moo_markup_create_element (MOO_MARKUP_NODE (doc), "MooFileView/Tools");

    do
    {
        MooMarkupNode *node;
        char *label, *extensions, *mimetypes, *command;

        gtk_tree_model_get (model, &iter, COLUMN_LABEL, &label,
                            COLUMN_COMMAND, &command,
                            COLUMN_EXTENSIONS, &extensions,
                            COLUMN_MIMETYPES, &mimetypes, -1);

        if (command)
            g_strstrip (command);
        if (extensions)
            g_strstrip (extensions);
        if (mimetypes)
            g_strstrip (mimetypes);

        if (command && !command[0])
        {
            g_free (command);
            command = NULL;
        }

        if (extensions && !extensions[0])
        {
            g_free (extensions);
            extensions = NULL;
        }

        if (mimetypes && !mimetypes[0])
        {
            g_free (mimetypes);
            mimetypes = NULL;
        }

        if (!label && !command)
        {
            g_warning ("%s: neither label nor command given", G_STRLOC);
            g_free (label);
            g_free (command);
            g_free (extensions);
            g_free (mimetypes);
            continue;
        }

        if (command)
            node = moo_markup_create_text_element (root, "tool", command);
        else
            node = moo_markup_create_element (root, "tool");

        if (label)
            moo_markup_set_prop (node, "label", label);
        if (extensions)
            moo_markup_set_prop (node, "extensions", extensions);
        if (mimetypes)
            moo_markup_set_prop (node, "mimetypes", mimetypes);
    }
    while (gtk_tree_model_iter_next (model, &iter));
}


static void
prefs_page_apply (MooGladeXML *xml)
{
    GtkTreeView *treeview = moo_glade_xml_get_widget (xml, "treeview");
    gpointer store = gtk_tree_view_get_model (treeview);
    MooTreeHelper *helper = g_object_get_data (G_OBJECT (xml), "moo-tree-helper");

    _moo_tree_helper_update_model (helper, NULL, NULL);
    save_store (helper, store);
    _moo_file_selector_update_tools (g_object_get_data (G_OBJECT (xml), "moo-file-selector-plugin"));
}


static void
populate_store (GtkListStore *store)
{
    MooMarkupDoc *doc;
    MooMarkupNode *root, *child;

    gtk_list_store_clear (store);

    doc = moo_prefs_get_markup (MOO_PREFS_RC);
    g_return_if_fail (doc != NULL);

    root = moo_markup_get_element (MOO_MARKUP_NODE (doc), "MooFileView/Tools");

    if (!root)
        return;

    for (child = root->children; child != NULL; child = child->next)
    {
        const char *label, *extensions, *mimetypes;
        const char *command;
        GtkTreeIter iter;

        if (!MOO_MARKUP_IS_ELEMENT (child))
            continue;

        if (strcmp (child->name, "tool"))
        {
            g_warning ("%s: unknown node '%s'", G_STRLOC, child->name);
            continue;
        }

        label = moo_markup_get_prop (child, "label");
        extensions = moo_markup_get_prop (child, "extensions");
        mimetypes = moo_markup_get_prop (child, "mimetypes");
        command = moo_markup_get_content (child);

        if (!label)
        {
            g_warning ("%s: label missing", G_STRLOC);
            continue;
        }

        if (!command)
        {
            g_warning ("%s: command missing", G_STRLOC);
            continue;
        }

        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter, COLUMN_LABEL, label,
                            COLUMN_COMMAND, command, COLUMN_EXTENSIONS, extensions,
                            COLUMN_MIMETYPES, mimetypes, -1);
    }
}


static void
prefs_page_init (MooGladeXML *xml)
{
    GtkWidget *treeview = moo_glade_xml_get_widget (xml, "treeview");
    gpointer store = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
    populate_store (store);
    _moo_tree_view_select_first (GTK_TREE_VIEW (treeview));
    _moo_tree_helper_update_widgets (g_object_get_data (G_OBJECT (xml), "moo-tree-helper"));
    _moo_tree_helper_set_modified (g_object_get_data (G_OBJECT (xml), "moo-tree-helper"), FALSE);
}


static gboolean
helper_new_row (MooTreeHelper  *helper,
                GtkTreeModel   *model,
                GtkTreePath    *path)
{
    GtkTreeIter iter;
    int pos;

    g_return_val_if_fail (helper != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_LIST_STORE (model), FALSE);
    g_return_val_if_fail (gtk_tree_path_get_depth (path) == 1, FALSE);

    pos = gtk_tree_path_get_indices(path)[0];
    gtk_list_store_insert_with_values (GTK_LIST_STORE (model), &iter, pos,
                                       COLUMN_LABEL, "Program",
                                       COLUMN_COMMAND, "program %f",
                                       -1);

    return TRUE;
}


static gboolean
helper_delete_row (MooTreeHelper  *helper,
                   GtkTreeModel   *model,
                   GtkTreePath    *path)
{
    GtkTreeIter iter;

    g_return_val_if_fail (helper != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_LIST_STORE (model), FALSE);

    if (gtk_tree_model_get_iter (model, &iter, path))
    {
        gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
        return TRUE;
    }

    g_return_val_if_reached (FALSE);
}


static gboolean
helper_move_row (MooTreeHelper  *helper,
                 GtkTreeModel   *model,
                 GtkTreePath    *old_path,
                 GtkTreePath    *new_path)
{
    GtkTreeIter iter, position;

    g_return_val_if_fail (helper != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_LIST_STORE (model), FALSE);

    gtk_tree_model_get_iter (model, &iter, old_path);
    gtk_tree_model_get_iter (model, &position, new_path);

    if (gtk_tree_path_compare (old_path, new_path) > 0)
        gtk_list_store_move_before (GTK_LIST_STORE (model), &iter, &position);
    else
        gtk_list_store_move_after (GTK_LIST_STORE (model), &iter, &position);

    return TRUE;
}


static void
helper_update_widgets (MooGladeXML    *xml,
                       GtkTreeModel   *model,
                       G_GNUC_UNUSED GtkTreePath *path,
                       GtkTreeIter    *iter)
{
    char *label = NULL, *command = NULL, *extensions = NULL, *mimetypes = NULL;
    gboolean sensitive = FALSE;

    if (model && iter)
    {
        sensitive = TRUE;
        gtk_tree_model_get (model, iter, COLUMN_LABEL, &label,
                            COLUMN_COMMAND, &command,
                            COLUMN_EXTENSIONS, &extensions,
                            COLUMN_MIMETYPES, &mimetypes, -1);
    }

    gtk_entry_set_text (moo_glade_xml_get_widget (xml, "label"),
                        label ? label : "");
    gtk_entry_set_text (moo_glade_xml_get_widget (xml, "command"),
                        command ? command : "");
    gtk_entry_set_text (moo_glade_xml_get_widget (xml, "extensions"),
                        extensions ? extensions : "");
    gtk_entry_set_text (moo_glade_xml_get_widget (xml, "mimetypes"),
                        mimetypes ? mimetypes : "");
    gtk_widget_set_sensitive (moo_glade_xml_get_widget (xml, "table"), sensitive);

    g_free (label);
    g_free (command);
    g_free (extensions);
    g_free (mimetypes);
}


static void
helper_update_model (MooTreeHelper  *helper,
                     G_GNUC_UNUSED GtkTreeModel *model,
                     G_GNUC_UNUSED GtkTreePath *path,
                     GtkTreeIter    *iter,
                     MooGladeXML    *xml)
{
    const char *label, *command, *extensions, *mimetypes;

    label = gtk_entry_get_text (moo_glade_xml_get_widget (xml, "label"));
    command = gtk_entry_get_text (moo_glade_xml_get_widget (xml, "command"));
    extensions = gtk_entry_get_text (moo_glade_xml_get_widget (xml, "extensions"));
    mimetypes = gtk_entry_get_text (moo_glade_xml_get_widget (xml, "mimetypes"));

    _moo_tree_helper_set (helper, iter,
                          COLUMN_LABEL, label[0] ? label : NULL,
                          COLUMN_COMMAND, command[0] ? command : NULL,
                          COLUMN_EXTENSIONS, extensions[0] ? extensions : NULL,
                          COLUMN_MIMETYPES, mimetypes[0] ? mimetypes : NULL,
                          -1);
}
