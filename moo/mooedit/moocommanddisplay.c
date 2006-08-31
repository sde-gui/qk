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

#define MOOEDIT_COMPILATION
#include "mooedit/moocommanddisplay.h"
#include "mooedit/moocommand-private.h"
#include <string.h>


typedef struct {
    MooCommandData *data;
    MooCommandType *type;
    gboolean changed;
} Data;

struct _MooCommandDisplay {
    MooTreeHelper base;

    GtkComboBox *type_combo;
    GtkNotebook *notebook;

    Data *data;
    int n_types;
    int active;
    int original;
};

struct _MooCommandDisplayClass {
    MooTreeHelperClass base_class;
};


G_DEFINE_TYPE (MooCommandDisplay, _moo_command_display, MOO_TYPE_TREE_HELPER)


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
    GtkWidget *widget;
    int index;
    MooCommandType *type;

    index = gtk_combo_box_get_active (display->type_combo);
    g_return_if_fail (index >= 0);

    if (index == display->active)
        return;

    widget = gtk_notebook_get_nth_page (display->notebook, display->active);
    if (_moo_command_type_save_data (display->data[display->active].type, widget,
                                     display->data[display->active].data))
        display->data[display->active].changed = TRUE;

    display->active = index;
    type = display->data[index].type;
    g_return_if_fail (type != NULL);

    if (!display->data[index].data)
        display->data[index].data = moo_command_data_new (type->n_keys);

    gtk_notebook_set_current_page (display->notebook, index);
    widget = gtk_notebook_get_nth_page (display->notebook, index);
    _moo_command_type_load_data (type, widget, display->data[index].data);
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
    int i;

    g_return_val_if_fail (!type || MOO_IS_COMMAND_TYPE (type), -1);

    if (!type)
        return -1;

    for (i = 0; i < display->n_types; ++i)
        if (display->data[i].type == type)
            return i;

    g_return_val_if_reached (-1);
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

    for (index = 0; index < display->n_types; ++index)
    {
        display->data[index].changed = FALSE;
        if (display->data[index].data)
            moo_command_data_unref (display->data[index].data);
        display->data[index].data = NULL;
    }

    index = combo_find_type (display, type);

    display->active = index;
    display->original = index;
    combo_set_active (display, index);

    if (index >= 0)
    {
        display->data[index].data = moo_command_data_ref (data);
        widget = gtk_notebook_get_nth_page (display->notebook, index);
        _moo_command_type_load_data (type, widget, data);
    }
}


gboolean
_moo_command_display_get (MooCommandDisplay  *display,
                          MooCommandType    **type_p,
                          MooCommandData    **data_p)
{
    GtkWidget *widget;
    Data *data;

    g_return_val_if_fail (MOO_IS_COMMAND_DISPLAY (display), FALSE);

    if (display->active < 0)
        return FALSE;

    data = &display->data[display->active];
    widget = gtk_notebook_get_nth_page (display->notebook, display->active);

    if (_moo_command_type_save_data (data->type, widget, data->data))
        data->changed = TRUE;

    if (display->active == display->original && !data->changed)
        return FALSE;

    *type_p = data->type;
    *data_p = data->data;
    data->changed = FALSE;
    display->original = display->active;

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
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (display->type_combo), cell, "text", 0, NULL);

    store = gtk_list_store_new (1, G_TYPE_STRING);
    gtk_combo_box_set_model (display->type_combo, GTK_TREE_MODEL (store));

    types = moo_command_list_types ();
    display->active = -1;
    display->original = -1;
    display->n_types = 0;
    display->data = g_new0 (Data, g_slist_length (types));

    while (types)
    {
        GtkTreeIter iter;
        MooCommandType *cmd_type;
        GtkWidget *widget;

        cmd_type = types->data;
        widget = _moo_command_type_create_widget (cmd_type);

        if (widget)
        {
            gtk_widget_show (widget);
            gtk_notebook_append_page (display->notebook, widget, NULL);

            gtk_list_store_append (store, &iter);
            gtk_list_store_set (store, &iter, 0, cmd_type->display_name, -1);

            display->data[display->n_types].type = cmd_type;
            display->n_types++;
        }

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
moo_command_display_dispose (GObject *object)
{
    MooCommandDisplay *display = MOO_COMMAND_DISPLAY (object);

    if (display->data)
    {
        int i;

        for (i = 0; i < display->n_types; ++i)
            if (display->data[i].data)
                moo_command_data_unref (display->data[i].data);

        g_free (display->data);
        display->data = NULL;
    }

    G_OBJECT_CLASS (_moo_command_display_parent_class)->dispose (object);
}


static void
_moo_command_display_init (G_GNUC_UNUSED MooCommandDisplay *display)
{
}


static void
_moo_command_display_class_init (MooCommandDisplayClass *klass)
{
    G_OBJECT_CLASS (klass)->dispose = moo_command_display_dispose;
}
