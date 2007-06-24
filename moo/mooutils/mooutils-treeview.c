/*
 *   mooutils-treeview.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#include "mooutils/mooutils-treeview.h"
#include "mooutils/mooutils-gobject.h"
#include "mooutils/moomarshals.h"
#include "mooutils/mooutils-debug.h"
#include <string.h>
#include <gobject/gvaluecollector.h>


typedef enum {
    TREE_VIEW,
    COMBO_BOX
} WidgetType;

static void moo_tree_helper_new_row             (MooTreeHelper      *helper);
static void moo_tree_helper_delete_row          (MooTreeHelper      *helper);
static void moo_tree_helper_row_up              (MooTreeHelper      *helper);
static void moo_tree_helper_row_down            (MooTreeHelper      *helper);
static void moo_tree_helper_real_update_widgets (MooTreeHelper      *helper,
                                                 GtkTreeModel       *model,
                                                 GtkTreePath        *path);
static GtkTreeModel *moo_tree_helper_get_model  (MooTreeHelper      *helper);
static gboolean moo_tree_helper_get_selected    (MooTreeHelper      *helper,
                                                 GtkTreeModel      **model,
                                                 GtkTreeIter        *iter);

static gboolean tree_helper_new_row_default     (MooTreeHelper      *helper,
                                                 GtkTreeModel       *model,
                                                 GtkTreePath        *path);
static gboolean tree_helper_delete_row_default  (MooTreeHelper      *helper,
                                                 GtkTreeModel       *model,
                                                 GtkTreePath        *path);
static gboolean tree_helper_move_row_default    (MooTreeHelper      *helper,
                                                 GtkTreeModel       *model,
                                                 GtkTreePath        *old_path,
                                                 GtkTreePath        *new_path);


G_DEFINE_TYPE (MooTreeHelper, _moo_tree_helper, GTK_TYPE_OBJECT)


enum {
    NEW_ROW,
    DELETE_ROW,
    MOVE_ROW,
    UPDATE_WIDGETS,
    UPDATE_MODEL,
    TREE_NUM_SIGNALS
};

static guint tree_signals[TREE_NUM_SIGNALS];


static void
tree_selection_changed (GtkTreeSelection *selection,
                        MooTreeHelper    *helper)
{
    GtkTreeRowReference *old_row;
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreePath *path, *old_path;

    old_row = g_object_get_data (G_OBJECT (selection), "moo-tree-helper-current-row");
    old_path = old_row ? gtk_tree_row_reference_get_path (old_row) : NULL;

    if (old_row && !old_path)
    {
        g_object_set_data (G_OBJECT (selection), "moo-tree-helper-current-row", NULL);
        old_row = NULL;
    }

    if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
        path = gtk_tree_model_get_path (model, &iter);

        if (old_path && !gtk_tree_path_compare (old_path, path))
        {
            gtk_tree_path_free (old_path);
            gtk_tree_path_free (path);
            return;
        }
    }
    else
    {
        if (!old_path)
            return;

        path = NULL;
    }

    if (old_path)
        _moo_tree_helper_update_model (helper, model, old_path);

    moo_tree_helper_real_update_widgets (helper, model, path);

    if (path)
    {
        GtkTreeRowReference *row;
        row = gtk_tree_row_reference_new (model, path);
        g_object_set_data_full (G_OBJECT (selection), "moo-tree-helper-current-row", row,
                                (GDestroyNotify) gtk_tree_row_reference_free);
    }
    else
    {
        g_object_set_data (G_OBJECT (selection), "moo-tree-helper-current-row", NULL);
    }

    gtk_tree_path_free (path);
    gtk_tree_path_free (old_path);
}


static void
combo_changed (GtkComboBox   *combo,
               MooTreeHelper *helper)
{
    GtkTreeRowReference *old_row;
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreePath *path, *old_path;

    g_return_if_fail (MOO_IS_TREE_HELPER (helper));
    g_return_if_fail (combo == helper->widget);

    old_row = g_object_get_data (G_OBJECT (combo), "moo-tree-helper-current-row");
    old_path = old_row ? gtk_tree_row_reference_get_path (old_row) : NULL;

    if (old_row && !old_path)
    {
        g_object_set_data (G_OBJECT (combo), "moo-tree-helper-current-row", NULL);
        old_row = NULL;
    }

    if (moo_tree_helper_get_selected (helper, &model, &iter))
    {
        path = gtk_tree_model_get_path (model, &iter);

        if (old_path && !gtk_tree_path_compare (old_path, path))
        {
            gtk_tree_path_free (old_path);
            gtk_tree_path_free (path);
            return;
        }
    }
    else
    {
        if (!old_path)
            return;

        path = NULL;
    }

    if (old_path)
        _moo_tree_helper_update_model (helper, model, old_path);

    moo_tree_helper_real_update_widgets (helper, model, path);

    if (path)
    {
        GtkTreeRowReference *row;
        row = gtk_tree_row_reference_new (model, path);
        g_object_set_data_full (G_OBJECT (combo), "moo-tree-helper-current-row", row,
                                (GDestroyNotify) gtk_tree_row_reference_free);
    }
    else
    {
        g_object_set_data (G_OBJECT (combo), "moo-tree-helper-current-row", NULL);
    }

    gtk_tree_path_free (path);
    gtk_tree_path_free (old_path);
}


static void
moo_tree_helper_destroy (GtkObject *object)
{
    MooTreeHelper *helper = MOO_TREE_HELPER (object);

    if (helper->widget)
    {
        GtkTreeSelection *selection;

        g_signal_handlers_disconnect_by_func (helper->widget,
                                              (gpointer) gtk_object_destroy,
                                              helper);

        switch (helper->type)
        {
            case TREE_VIEW:
                selection = gtk_tree_view_get_selection (helper->widget);
                g_signal_handlers_disconnect_by_func (selection,
                                                      (gpointer) tree_selection_changed,
                                                      helper);
                break;

            case COMBO_BOX:
                g_signal_handlers_disconnect_by_func (helper->widget,
                                                      (gpointer) combo_changed,
                                                      helper);
                break;
        }

        if (helper->new_btn)
            g_signal_handlers_disconnect_by_func (helper->new_btn,
                                                  (gpointer) moo_tree_helper_new_row,
                                                  helper);
        if (helper->delete_btn)
            g_signal_handlers_disconnect_by_func (helper->delete_btn,
                                                  (gpointer) moo_tree_helper_delete_row,
                                                  helper);
        if (helper->up_btn)
            g_signal_handlers_disconnect_by_func (helper->up_btn,
                                                  (gpointer) moo_tree_helper_row_up,
                                                  helper);
        if (helper->down_btn)
            g_signal_handlers_disconnect_by_func (helper->down_btn,
                                                  (gpointer) moo_tree_helper_row_down,
                                                  helper);

        helper->widget = NULL;
    }

    GTK_OBJECT_CLASS (_moo_tree_helper_parent_class)->destroy (object);
}


static gboolean
tree_helper_new_row_default (G_GNUC_UNUSED MooTreeHelper *helper,
                             GtkTreeModel  *model,
                             GtkTreePath   *path)
{
    GtkTreeIter iter;

    if (!GTK_IS_LIST_STORE (model))
        return FALSE;

    g_return_val_if_fail (path != NULL, FALSE);
    g_return_val_if_fail (gtk_tree_path_get_depth (path) == 1, FALSE);

    gtk_list_store_insert (GTK_LIST_STORE (model), &iter,
                           gtk_tree_path_get_indices(path)[0]);

    return TRUE;
}


static gboolean
tree_helper_delete_row_default (G_GNUC_UNUSED MooTreeHelper *helper,
                                GtkTreeModel  *model,
                                GtkTreePath   *path)
{
    GtkTreeIter iter;

    if (!GTK_IS_LIST_STORE (model))
        return FALSE;

    g_return_val_if_fail (path != NULL, FALSE);
    g_return_val_if_fail (gtk_tree_path_get_depth (path) == 1, FALSE);

    gtk_tree_model_get_iter (model, &iter, path);
    gtk_list_store_remove (GTK_LIST_STORE (model), &iter);

    return TRUE;
}


static gboolean
tree_helper_move_row_default (G_GNUC_UNUSED MooTreeHelper *helper,
                              GtkTreeModel *model,
                              GtkTreePath  *old_path,
                              GtkTreePath  *new_path)
{
    GtkTreeIter old_iter, new_iter;
    int new, old;

    if (!GTK_IS_LIST_STORE (model))
        return FALSE;

    g_return_val_if_fail (old_path && new_path, FALSE);
    g_return_val_if_fail (gtk_tree_path_get_depth (old_path) == 1, FALSE);
    g_return_val_if_fail (gtk_tree_path_get_depth (new_path) == 1, FALSE);

    new = gtk_tree_path_get_indices(new_path)[0];
    old = gtk_tree_path_get_indices(old_path)[0];
    g_return_val_if_fail (ABS (new - old) == 1, FALSE);

    gtk_tree_model_get_iter (model, &old_iter, old_path);
    gtk_tree_model_get_iter (model, &new_iter, new_path);
    gtk_list_store_swap (GTK_LIST_STORE (model), &old_iter, &new_iter);

    return TRUE;
}


static void
_moo_tree_helper_class_init (MooTreeHelperClass *klass)
{
    GTK_OBJECT_CLASS(klass)->destroy = moo_tree_helper_destroy;

    klass->move_row = tree_helper_move_row_default;
    klass->new_row = tree_helper_new_row_default;
    klass->delete_row = tree_helper_delete_row_default;

    tree_signals[NEW_ROW] =
            g_signal_new ("new-row",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTreeHelperClass, new_row),
                          g_signal_accumulator_true_handled, NULL,
                          _moo_marshal_BOOL__OBJECT_BOXED,
                          G_TYPE_BOOLEAN, 2,
                          GTK_TYPE_TREE_MODEL,
                          GTK_TYPE_TREE_PATH | G_SIGNAL_TYPE_STATIC_SCOPE);

    tree_signals[DELETE_ROW] =
            g_signal_new ("delete-row",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTreeHelperClass, delete_row),
                          g_signal_accumulator_true_handled, NULL,
                          _moo_marshal_BOOL__OBJECT_BOXED,
                          G_TYPE_BOOLEAN, 2,
                          GTK_TYPE_TREE_MODEL,
                          GTK_TYPE_TREE_PATH | G_SIGNAL_TYPE_STATIC_SCOPE);

    tree_signals[MOVE_ROW] =
            g_signal_new ("move-row",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTreeHelperClass, move_row),
                          g_signal_accumulator_true_handled, NULL,
                          _moo_marshal_BOOL__OBJECT_BOXED_BOXED,
                          G_TYPE_BOOLEAN, 3,
                          GTK_TYPE_TREE_MODEL,
                          GTK_TYPE_TREE_PATH | G_SIGNAL_TYPE_STATIC_SCOPE,
                          GTK_TYPE_TREE_PATH | G_SIGNAL_TYPE_STATIC_SCOPE);

    tree_signals[UPDATE_WIDGETS] =
            g_signal_new ("update-widgets",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTreeHelperClass, update_widgets),
                          NULL, NULL,
                          _moo_marshal_VOID__OBJECT_BOXED_BOXED,
                          G_TYPE_NONE, 3,
                          GTK_TYPE_TREE_MODEL,
                          GTK_TYPE_TREE_PATH | G_SIGNAL_TYPE_STATIC_SCOPE,
                          GTK_TYPE_TREE_ITER | G_SIGNAL_TYPE_STATIC_SCOPE);

    tree_signals[UPDATE_MODEL] =
            g_signal_new ("update-model",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTreeHelperClass, update_model),
                          NULL, NULL,
                          _moo_marshal_VOID__OBJECT_BOXED_BOXED,
                          G_TYPE_NONE, 3,
                          GTK_TYPE_TREE_MODEL,
                          GTK_TYPE_TREE_PATH | G_SIGNAL_TYPE_STATIC_SCOPE,
                          GTK_TYPE_TREE_ITER | G_SIGNAL_TYPE_STATIC_SCOPE);
}


void
_moo_tree_helper_set_modified (MooTreeHelper *helper,
                               gboolean       modified)
{
    g_return_if_fail (MOO_IS_TREE_HELPER (helper));

#ifdef MOO_DEBUG
    if (!helper->modified && modified)
    {
        g_message ("%s: helper modified", G_STRLOC);
    }
#endif

    helper->modified = modified != 0;
}


gboolean
_moo_tree_helper_get_modified (MooTreeHelper *helper)
{
    g_return_val_if_fail (MOO_IS_TREE_HELPER (helper), FALSE);
    return helper->modified;
}


void
_moo_tree_view_select_first (GtkTreeView *tree_view)
{
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;

    g_return_if_fail (GTK_IS_TREE_VIEW (tree_view));

    selection = gtk_tree_view_get_selection (tree_view);
    model = gtk_tree_view_get_model (tree_view);

    if (gtk_tree_model_get_iter_first (model, &iter))
        gtk_tree_selection_select_iter (selection, &iter);
}


void
_moo_combo_box_select_first (GtkComboBox *combo)
{
    GtkTreeModel *model;
    GtkTreeIter iter;

    g_return_if_fail (GTK_IS_COMBO_BOX (combo));

    model = gtk_combo_box_get_model (combo);

    if (gtk_tree_model_get_iter_first (model, &iter))
        gtk_combo_box_set_active_iter (combo, &iter);
}


static void
moo_tree_helper_real_update_widgets (MooTreeHelper      *helper,
                                     GtkTreeModel       *model,
                                     GtkTreePath        *path)
{
    GtkTreeIter iter;

    if (!path)
    {
        if (helper->delete_btn)
            gtk_widget_set_sensitive (helper->delete_btn, FALSE);
        if (helper->up_btn)
            gtk_widget_set_sensitive (helper->up_btn, FALSE);
        if (helper->down_btn)
            gtk_widget_set_sensitive (helper->down_btn, FALSE);
    }
    else
    {
        int *indices;
        int n_rows;

        if (helper->delete_btn)
            gtk_widget_set_sensitive (helper->delete_btn, TRUE);

        indices = gtk_tree_path_get_indices (path);

        if (helper->up_btn)
            gtk_widget_set_sensitive (helper->up_btn, indices[0] != 0);

        n_rows = gtk_tree_model_iter_n_children (model, NULL);

        if (helper->down_btn)
            gtk_widget_set_sensitive (helper->down_btn, indices[0] != n_rows - 1);
    }

    if (path)
        gtk_tree_model_get_iter (model, &iter, path);

    g_signal_emit (helper, tree_signals[UPDATE_WIDGETS], 0,
                   model, path, path ? &iter : NULL);
}


void
_moo_tree_helper_update_model (MooTreeHelper *helper,
                               GtkTreeModel  *model,
                               GtkTreePath   *path)
{
    GtkTreePath *freeme = NULL;
    GtkTreeIter iter;

    g_return_if_fail (MOO_IS_TREE_HELPER (helper));
    g_return_if_fail (!model || GTK_IS_TREE_MODEL (model));

    if (!model)
        model = moo_tree_helper_get_model (helper);

    if (!path && moo_tree_helper_get_selected (helper, NULL, &iter))
        path = freeme = gtk_tree_model_get_path (model, &iter);

    if (path)
    {
        gtk_tree_model_get_iter (model, &iter, path);
        g_signal_emit (helper, tree_signals[UPDATE_MODEL], 0,
                       model, path, &iter);
    }

    gtk_tree_path_free (freeme);
}


static GtkTreeModel *
moo_tree_helper_get_model (MooTreeHelper *helper)
{
    switch (helper->type)
    {
        case TREE_VIEW:
            return gtk_tree_view_get_model (helper->widget);
        case COMBO_BOX:
            return gtk_combo_box_get_model (helper->widget);
    }

    g_return_val_if_reached (NULL);
}


static gboolean
moo_tree_helper_get_selected (MooTreeHelper *helper,
                              GtkTreeModel **model,
                              GtkTreeIter   *iter)
{
    GtkTreeSelection *selection;

    switch (helper->type)
    {
        case TREE_VIEW:
            selection = gtk_tree_view_get_selection (helper->widget);
            return gtk_tree_selection_get_selected (selection, model, iter);

        case COMBO_BOX:
            if (model)
                *model = gtk_combo_box_get_model (helper->widget);
            return gtk_combo_box_get_active_iter (helper->widget, iter);
    }

    g_return_val_if_reached (FALSE);
}


static int
iter_get_index (GtkTreeModel *model,
                GtkTreeIter  *iter)
{
    int index;
    GtkTreePath *path;

    path = gtk_tree_model_get_path (model, iter);

    if (!path)
        return -1;

    index = gtk_tree_path_get_indices(path)[0];

    gtk_tree_path_free (path);
    return index;
}


static void
moo_tree_helper_new_row (MooTreeHelper *helper)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreeSelection *selection;
    GtkTreePath *path;
    int index;
    gboolean result;

    g_return_if_fail (helper->type == TREE_VIEW);

    selection = gtk_tree_view_get_selection (helper->widget);

    if (!gtk_tree_selection_get_selected (selection, &model, &iter))
        index = gtk_tree_model_iter_n_children (model, NULL);
    else
        index = iter_get_index (model, &iter) + 1;

    path = gtk_tree_path_new_from_indices (index, -1);

    g_signal_emit (helper, tree_signals[NEW_ROW], 0, model, path, &result);

    if (result && gtk_tree_model_get_iter (model, &iter, path))
    {
        gtk_tree_selection_select_iter (selection, &iter);
        helper->modified = TRUE;
    }

    gtk_tree_path_free (path);
}


static void
moo_tree_helper_delete_row (MooTreeHelper *helper)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreeSelection *selection;
    GtkTreePath *path;
    gboolean result;

    g_return_if_fail (helper->type == TREE_VIEW);

    selection = gtk_tree_view_get_selection (helper->widget);

    if (!gtk_tree_selection_get_selected (selection, &model, &iter))
    {
        g_critical ("%s: oops", G_STRLOC);
        return;
    }

    path = gtk_tree_model_get_path (model, &iter);

    g_signal_emit (helper, tree_signals[DELETE_ROW], 0, model, path, &result);

    if (result)
        helper->modified = TRUE;

    if (result && (gtk_tree_model_get_iter (model, &iter, path) ||
                   (gtk_tree_path_prev (path) && gtk_tree_model_get_iter (model, &iter, path))))
        gtk_tree_selection_select_iter (selection, &iter);
    else
        _moo_tree_helper_update_widgets (helper);

    gtk_tree_path_free (path);
}


static void
moo_tree_helper_row_move (MooTreeHelper *helper,
                          gboolean       up)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreePath *path, *new_path;
    GtkTreeSelection *selection;
    gboolean result;

    g_return_if_fail (helper->type == TREE_VIEW);

    selection = gtk_tree_view_get_selection (helper->widget);

    if (!gtk_tree_selection_get_selected (selection, &model, &iter))
    {
        g_critical ("%s: oops", G_STRLOC);
        return;
    }

    path = gtk_tree_model_get_path (model, &iter);
    new_path = gtk_tree_path_copy (path);

    if (up && !gtk_tree_path_prev (new_path))
        g_return_if_reached ();
    else if (!up)
        gtk_tree_path_next (new_path);

    g_signal_emit (helper, tree_signals[MOVE_ROW], 0, model, path, new_path, &result);

    if (result)
    {
        helper->modified = TRUE;
        moo_tree_helper_real_update_widgets (helper, model, new_path);
    }

    gtk_tree_path_free (new_path);
    gtk_tree_path_free (path);
}


static void
moo_tree_helper_row_up (MooTreeHelper *helper)
{
    moo_tree_helper_row_move (helper, TRUE);
}


static void
moo_tree_helper_row_down (MooTreeHelper *helper)
{
    moo_tree_helper_row_move (helper, FALSE);
}


static void
_moo_tree_helper_init (G_GNUC_UNUSED MooTreeHelper *helper)
{
}


void
_moo_tree_helper_connect (MooTreeHelper *helper,
                          GtkWidget     *widget,
                          GtkWidget     *new_btn,
                          GtkWidget     *delete_btn,
                          GtkWidget     *up_btn,
                          GtkWidget     *down_btn)
{
    GtkTreeSelection *selection;

    g_return_if_fail (MOO_IS_TREE_HELPER (helper));
    g_return_if_fail (GTK_IS_TREE_VIEW (widget) || GTK_IS_COMBO_BOX (widget));
    g_return_if_fail (helper->widget == NULL);

    helper->widget = widget;

    if (GTK_IS_TREE_VIEW (widget))
        helper->type = TREE_VIEW;
    else
        helper->type = COMBO_BOX;

    helper->new_btn = new_btn;
    helper->delete_btn = delete_btn;
    helper->up_btn = up_btn;
    helper->down_btn = down_btn;

    g_signal_connect_swapped (widget, "destroy",
                              G_CALLBACK (gtk_object_destroy),
                              helper);

    switch (helper->type)
    {
        case TREE_VIEW:
            selection = gtk_tree_view_get_selection (helper->widget);
            g_signal_connect (selection, "changed",
                              G_CALLBACK (tree_selection_changed),
                              helper);
            break;

        case COMBO_BOX:
            g_signal_connect (widget, "changed",
                              G_CALLBACK (combo_changed),
                              helper);
            break;
    }

    if (new_btn)
        g_signal_connect_swapped (new_btn, "clicked",
                                  G_CALLBACK (moo_tree_helper_new_row),
                                  helper);
    if (delete_btn)
        g_signal_connect_swapped (delete_btn, "clicked",
                                  G_CALLBACK (moo_tree_helper_delete_row),
                                  helper);
    if (up_btn)
        g_signal_connect_swapped (up_btn, "clicked",
                                  G_CALLBACK (moo_tree_helper_row_up),
                                  helper);
    if (down_btn)
        g_signal_connect_swapped (down_btn, "clicked",
                                  G_CALLBACK (moo_tree_helper_row_down),
                                  helper);
}


MooTreeHelper *
_moo_tree_helper_new (GtkWidget *widget,
                      GtkWidget *new_btn,
                      GtkWidget *delete_btn,
                      GtkWidget *up_btn,
                      GtkWidget *down_btn)
{
    MooTreeHelper *helper;

    g_return_val_if_fail (GTK_IS_TREE_VIEW (widget) || GTK_IS_COMBO_BOX (widget), NULL);

    helper = g_object_new (MOO_TYPE_TREE_HELPER, NULL);
    MOO_OBJECT_REF_SINK (helper);

    _moo_tree_helper_connect (helper, widget, new_btn, delete_btn, up_btn, down_btn);

    return helper;
}


void
_moo_tree_helper_update_widgets (MooTreeHelper *helper)
{
    GtkTreeModel *model;
    GtkTreeIter iter;

    g_return_if_fail (MOO_IS_TREE_HELPER (helper));

    if (moo_tree_helper_get_selected (helper, &model, &iter))
    {
        GtkTreePath *path = gtk_tree_model_get_path (model, &iter);
        moo_tree_helper_real_update_widgets (MOO_TREE_HELPER (helper), model, path);
        gtk_tree_path_free (path);
    }
    else
    {
        moo_tree_helper_real_update_widgets (MOO_TREE_HELPER (helper), NULL, NULL);
    }
}


static gboolean
set_value (GtkTreeModel  *model,
           GtkTreeIter   *iter,
           int            column,
           GValue        *value)
{
    GValue old_value;
    gboolean modified;

    old_value.g_type = 0;
    gtk_tree_model_get_value (model, iter, column, &old_value);

    modified = !_moo_value_equal (value, &old_value);

    if (GTK_IS_TREE_STORE (model))
        gtk_tree_store_set_value (GTK_TREE_STORE (model), iter, column, value);
    else if (GTK_IS_LIST_STORE (model))
        gtk_list_store_set_value (GTK_LIST_STORE (model), iter, column, value);

    g_value_unset (&old_value);
    return modified;
}

static gboolean
moo_tree_helper_set_valist (MooTreeHelper *helper,
                            GtkTreeModel  *model,
                            GtkTreeIter   *iter,
                            va_list	   var_args)
{
    int column;
    int n_columns;
    gboolean modified = FALSE;

    n_columns = gtk_tree_model_get_n_columns (model);
    column = va_arg (var_args, gint);

    while (column >= 0)
    {
        GType type;
        GValue value;
        char *error = NULL;

        value.g_type = 0;

        if (column >= n_columns)
        {
            g_warning ("%s: invalid column number %d", G_STRLOC, column);
            break;
        }

        type = gtk_tree_model_get_column_type (model, column);
        g_value_init (&value, type);

        G_VALUE_COLLECT (&value, var_args, 0, &error);

        if (error)
        {
            g_warning ("%s: %s", G_STRLOC, error);
            g_free (error);
            break;
        }

        modified = set_value (model, iter, column, &value) || modified;

        g_value_unset (&value);
        column = va_arg (var_args, gint);
    }

    helper->modified = helper->modified || modified;
    return modified;
}

gboolean
_moo_tree_helper_set (MooTreeHelper *helper,
                      GtkTreeIter   *iter,
                      ...)
{
      va_list args;
      GtkTreeModel *model;
      gboolean ret;

      g_return_val_if_fail (MOO_IS_TREE_HELPER (helper), FALSE);

      model = moo_tree_helper_get_model (helper);
      g_return_val_if_fail (GTK_IS_TREE_STORE (model) || GTK_IS_LIST_STORE (model), FALSE);

      va_start (args, iter);
      ret = moo_tree_helper_set_valist (helper, model, iter, args);
      va_end (args);

      return ret;
}
