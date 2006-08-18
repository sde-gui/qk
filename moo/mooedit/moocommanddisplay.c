/*
 *   moocommanddisplay.c
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

#include "mooedit/moocommanddisplay.h"
#include "mooedit/moocommand.h"
#include <string.h>


G_DEFINE_TYPE (MooCommandDisplay, _moo_command_display, MOO_TYPE_TREE_HELPER)


enum {
    COMBO_COLUMN_NAME,
    COMBO_COLUMN_TYPE
};

static void     combo_changed   (MooCommandDisplay *display);


static void
block_combo (MooCommandDisplay *display)
{
    g_signal_handlers_block_by_func (display->type_combo,
                                     (gpointer) combo_changed,
                                     display);
}


static void
unblock_combo (MooCommandDisplay *display)
{
    g_signal_handlers_unblock_by_func (display->type_combo,
                                       (gpointer) combo_changed,
                                       display);
}


static void
combo_changed (MooCommandDisplay *display)
{
    GtkTreeModel *combo_model;
    GtkWidget *widget;
    GtkTreeIter iter;
    MooCommandType *type = NULL;
    int index;

    display->changed = TRUE;
    combo_model = gtk_combo_box_get_model (display->type_combo);
    index = gtk_combo_box_get_active (display->type_combo);
    g_return_if_fail (index >= 0);

    gtk_tree_model_iter_nth_child (combo_model, &iter, NULL, index);
    gtk_tree_model_get (combo_model, &iter, COMBO_COLUMN_TYPE, &type, -1);
    g_return_if_fail (type != NULL);

    display->active_type = type;
    if (display->active_data)
        moo_command_data_unref (display->active_data);
    display->active_data = moo_command_data_new ();

    gtk_notebook_set_current_page (display->notebook, index);
    widget = gtk_notebook_get_nth_page (display->notebook, index);
    _moo_command_type_load_data (type, widget, display->active_data);
}


static void
combo_set_active (MooCommandDisplay  *display,
                  int                 index)
{
    block_combo (display);
    gtk_combo_box_set_active (display->type_combo, index);
    unblock_combo (display);

    if (index < 0)
    {
        gtk_widget_hide (GTK_WIDGET (display->notebook));
    }
    else
    {
        gtk_notebook_set_current_page (display->notebook, index);
        gtk_widget_show (GTK_WIDGET (display->notebook));
    }
}


static int
combo_find_type (MooCommandDisplay  *display,
                 MooCommandType     *type)
{
    GtkTreeModel *model = gtk_combo_box_get_model (display->type_combo);
    gboolean found = FALSE;
    int index = 0;
    GtkTreeIter iter;

    g_return_val_if_fail (MOO_IS_COMMAND_TYPE (type), -1);

    if (gtk_tree_model_get_iter_first (model, &iter))
    {
        do
        {
            MooCommandType *this_type;

            gtk_tree_model_get (model, &iter, COMBO_COLUMN_TYPE, &this_type, -1);

            if (this_type == type)
            {
                found = TRUE;
                g_object_unref (this_type);
                break;
            }

            g_object_unref (this_type);
            ++index;
        }
        while (gtk_tree_model_iter_next (model, &iter));
    }

    g_return_val_if_fail (found, -1);
    return index;
}


void
_moo_command_display_set (MooCommandDisplay  *display,
                          MooCommandType     *type,
                          MooCommandData     *data)
{
    int index;
    GtkWidget *widget;

    g_return_if_fail (MOO_IS_COMMAND_DISPLAY (display));
    g_return_if_fail (!type || MOO_IS_COMMAND_TYPE (type));
    g_return_if_fail (!type == !data);

    display->changed = FALSE;

    if (display->active_data)
        moo_command_data_unref (display->active_data);
    display->active_data = NULL;
    display->active_type = NULL;

    if (!type)
    {
        combo_set_active (display, -1);
        return;
    }

    index = combo_find_type (display, type);
    g_return_if_fail (index >= 0);

    display->active_data = moo_command_data_ref (data);
    display->active_type = type;

    combo_set_active (display, index);
    widget = gtk_notebook_get_nth_page (display->notebook, index);
    _moo_command_type_load_data (type, widget, display->active_data);
}


gboolean
_moo_command_display_get (MooCommandDisplay  *display,
                          MooCommandType    **type,
                          MooCommandData    **data)
{
    g_return_val_if_fail (MOO_IS_COMMAND_DISPLAY (display), FALSE);

    if (display->active_type)
    {
        int index;
        GtkWidget *widget;

        index = gtk_combo_box_get_active (display->type_combo);
        g_return_val_if_fail (index >= 0, FALSE);

        widget = gtk_notebook_get_nth_page (display->notebook, index);

        if (_moo_command_type_save_data (display->active_type, widget, display->active_data))
            display->changed = TRUE;
    }

    if (!display->changed)
        return FALSE;

    g_return_val_if_fail (display->active_type != NULL, FALSE);
    g_return_val_if_fail (display->active_data != NULL, FALSE);

    *type = display->active_type;
    *data = display->active_data;
    display->changed = FALSE;

    return TRUE;
}


static void
init_type_combo (MooCommandDisplay *display,
                 GtkComboBox       *combo,
                 GtkNotebook       *notebook)
{
    GtkListStore *store;
    GSList *types;
    GtkCellRenderer *cell;

    g_return_if_fail (MOO_IS_COMMAND_DISPLAY (display));
    g_return_if_fail (GTK_IS_COMBO_BOX (combo));
    g_return_if_fail (GTK_IS_NOTEBOOK (notebook));

    display->type_combo = combo;
    display->notebook = notebook;

    cell = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (display->type_combo), cell, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (display->type_combo), cell,
                                    "text", COMBO_COLUMN_NAME, NULL);

    store = gtk_list_store_new (2, G_TYPE_STRING, MOO_TYPE_COMMAND_TYPE);
    gtk_combo_box_set_model (display->type_combo, GTK_TREE_MODEL (store));

    types = moo_command_list_types ();

    while (types)
    {
        GtkTreeIter iter;
        MooCommandType *cmd_type;
        GtkWidget *widget;

        cmd_type = types->data;

        widget = _moo_command_type_create_widget (cmd_type);
        g_return_if_fail (widget != NULL);

        gtk_widget_show (widget);
        gtk_notebook_append_page (display->notebook, widget, NULL);

        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter,
                            COMBO_COLUMN_NAME, cmd_type->display_name,
                            COMBO_COLUMN_TYPE, cmd_type,
                            -1);

        types = g_slist_delete_link (types, types);
    }

    g_signal_connect_swapped (display->type_combo, "changed",
                              G_CALLBACK (combo_changed), display);

    g_object_unref (store);
}


MooCommandDisplay *
_moo_command_display_new (GtkComboBox        *type_combo,
                          GtkNotebook        *notebook,
                          GtkWidget          *treeview,
                          GtkWidget          *new_btn,
                          GtkWidget          *delete_btn,
                          GtkWidget          *up_btn,
                          GtkWidget          *down_btn)
{
    MooCommandDisplay *display;

    display = g_object_new (MOO_TYPE_COMMAND_DISPLAY, NULL);
    _moo_tree_helper_connect (MOO_TREE_HELPER (display), treeview,
                              new_btn, delete_btn, up_btn, down_btn);
    init_type_combo (display, type_combo, notebook);

    gtk_object_sink (g_object_ref (display));
    return display;
}


static void
_moo_command_display_init (G_GNUC_UNUSED MooCommandDisplay *display)
{
}


static void
_moo_command_display_class_init (G_GNUC_UNUSED MooCommandDisplayClass *klass)
{
}
