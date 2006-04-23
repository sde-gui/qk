/*
 *   mooutils-treeview.c
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

#include "mooutils/mooutils-treeview.h"
#include "mooutils/moomarshals.h"


#define MOO_TYPE_TREE_HELPER              (_moo_tree_helper_get_type ())
#define MOO_TREE_HELPER(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_TREE_HELPER, MooTreeHelper))
#define MOO_TREE_HELPER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_TREE_HELPER, MooTreeHelperClass))
#define MOO_IS_TREE_HELPER(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_TREE_HELPER))
#define MOO_IS_TREE_HELPER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_TREE_HELPER))
#define MOO_TREE_HELPER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_TREE_HELPER, MooTreeHelperClass))


struct _MooTreeHelper {
    GtkObject parent;

    GtkTreeView *treeview;
    GtkWidget *new_btn;
    GtkWidget *delete_btn;
    GtkWidget *up_btn;
    GtkWidget *down_btn;
};


GType   _moo_tree_helper_get_type    (void) G_GNUC_CONST;

static void moo_tree_helper_new_row         (MooTreeHelper      *helper);
static void moo_tree_helper_delete_row      (MooTreeHelper      *helper);
static void moo_tree_helper_row_up          (MooTreeHelper      *helper);
static void moo_tree_helper_row_down        (MooTreeHelper      *helper);
static void moo_tree_helper_update_widgets  (MooTreeHelper      *helper,
                                             GtkTreeModel       *model,
                                             GtkTreePath        *path);
static void moo_tree_helper_update_model    (MooTreeHelper      *helper,
                                             GtkTreeModel       *model,
                                             GtkTreePath        *path);


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
selection_changed (GtkTreeSelection   *selection,
                   MooTreeHelper      *helper)
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
        moo_tree_helper_update_model (helper, model, old_path);

    moo_tree_helper_update_widgets (helper, model, path);

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
moo_tree_helper_destroy (GtkObject *object)
{
    MooTreeHelper *helper = MOO_TREE_HELPER (object);

    if (helper->treeview)
    {
        GtkTreeSelection *selection;

        g_signal_handlers_disconnect_by_func (helper->treeview,
                                              (gpointer) gtk_object_destroy,
                                              helper);

        selection = gtk_tree_view_get_selection (helper->treeview);
        g_signal_handlers_disconnect_by_func (selection,
                                              (gpointer) selection_changed,
                                              helper);

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

        helper->treeview = NULL;
    }

    GTK_OBJECT_CLASS (_moo_tree_helper_parent_class)->destroy (object);
}


static void
_moo_tree_helper_class_init (MooTreeHelperClass *klass)
{
    GTK_OBJECT_CLASS(klass)->destroy = moo_tree_helper_destroy;

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
moo_tree_view_select_first (GtkTreeView *tree_view)
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


static void
moo_tree_helper_update_widgets (MooTreeHelper      *helper,
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


static void
moo_tree_helper_update_model (MooTreeHelper      *helper,
                              GtkTreeModel       *model,
                              GtkTreePath        *path)
{
    GtkTreePath *freeme = NULL;
    GtkTreeIter iter;

    if (!model)
        model = gtk_tree_view_get_model (helper->treeview);

    if (!path)
    {
        GtkTreeSelection *selection = gtk_tree_view_get_selection (helper->treeview);

        if (gtk_tree_selection_get_selected (selection, NULL, &iter))
            path = freeme = gtk_tree_model_get_path (model, &iter);
    }

    if (path)
    {
        gtk_tree_model_get_iter (model, &iter, path);
        g_signal_emit (helper, tree_signals[UPDATE_MODEL], 0,
                       model, path, &iter);
    }

    gtk_tree_path_free (freeme);
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

    selection = gtk_tree_view_get_selection (helper->treeview);

    if (!gtk_tree_selection_get_selected (selection, &model, &iter))
        index = gtk_tree_model_iter_n_children (model, NULL);
    else
        index = iter_get_index (model, &iter) + 1;

    path = gtk_tree_path_new_from_indices (index, -1);

    g_signal_emit (helper, tree_signals[NEW_ROW], 0, model, path, &result);

    if (result && gtk_tree_model_get_iter (model, &iter, path))
        gtk_tree_selection_select_iter (selection, &iter);

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

    selection = gtk_tree_view_get_selection (helper->treeview);

    if (!gtk_tree_selection_get_selected (selection, &model, &iter))
        g_return_if_reached ();

    path = gtk_tree_model_get_path (model, &iter);

    g_signal_emit (helper, tree_signals[DELETE_ROW], 0, model, path, &result);

    if (result && (gtk_tree_model_get_iter (model, &iter, path) ||
                   (gtk_tree_path_prev (path) && gtk_tree_model_get_iter (model, &iter, path))))
        gtk_tree_selection_select_iter (selection, &iter);

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

    selection = gtk_tree_view_get_selection (helper->treeview);

    if (!gtk_tree_selection_get_selected (selection, &model, &iter))
        g_return_if_reached ();

    path = gtk_tree_model_get_path (model, &iter);
    new_path = gtk_tree_path_copy (path);

    if (up && !gtk_tree_path_prev (new_path))
        g_return_if_reached ();
    else if (!up)
        gtk_tree_path_next (new_path);

    g_signal_emit (helper, tree_signals[MOVE_ROW], 0, model, path, new_path, &result);

    if (result)
        moo_tree_helper_update_widgets (helper, model, new_path);

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


static void
moo_tree_helper_connect (MooTreeHelper *helper,
                         GtkWidget     *tree_view,
                         GtkWidget     *new_btn,
                         GtkWidget     *delete_btn,
                         GtkWidget     *up_btn,
                         GtkWidget     *down_btn)
{
    GtkTreeSelection *selection;

    g_return_if_fail (MOO_IS_TREE_HELPER (helper));
    g_return_if_fail (GTK_IS_TREE_VIEW (tree_view));
    g_return_if_fail (helper->treeview == NULL);

    helper->treeview = GTK_TREE_VIEW (tree_view);
    helper->new_btn = new_btn;
    helper->delete_btn = delete_btn;
    helper->up_btn = up_btn;
    helper->down_btn = down_btn;

    g_signal_connect_swapped (tree_view, "destroy",
                              G_CALLBACK (gtk_object_destroy),
                              helper);

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    g_signal_connect (selection, "changed",
                      G_CALLBACK (selection_changed),
                      helper);

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


/****************************************************************************/
/* MooConfigHelper
 */

#define MOO_TYPE_CONFIG_HELPER            (_moo_config_helper_get_type ())
#define MOO_CONFIG_HELPER(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_CONFIG_HELPER, MooConfigHelper))
#define MOO_CONFIG_HELPER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_CONFIG_HELPER, MooConfigHelperClass))
#define MOO_IS_CONFIG_HELPER(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_CONFIG_HELPER))
#define MOO_IS_CONFIG_HELPER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_CONFIG_HELPER))
#define MOO_CONFIG_HELPER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_CONFIG_HELPER, MooConfigHelperClass))


enum {
    NEW_ITEM,
    SET_FROM_ITEM,
    SET_FROM_WIDGETS,
    CONFIG_NUM_SIGNALS
};

static guint config_signals[CONFIG_NUM_SIGNALS];


typedef void (*MooConfigItemToWidget) (MooConfig     *config,
                                       MooConfigItem *item,
                                       GtkWidget     *widget,
                                       gpointer       data);
typedef void (*MooConfigWidgetToItem) (GtkWidget     *widget,
                                       MooConfig     *config,
                                       MooConfigItem *item,
                                       gpointer       data);


typedef struct {
    char *key;
    gboolean update_live;
    gulong handler;
    MooConfigItemToWidget item_to_widget;
    MooConfigWidgetToItem widget_to_item;
    gpointer data;
} WidgetInfo;

struct _MooConfigHelper {
    MooTreeHelper parent;
    GSList *widgets;
};

GType   _moo_config_helper_get_type  (void) G_GNUC_CONST;
G_DEFINE_TYPE (MooConfigHelper, _moo_config_helper, MOO_TYPE_TREE_HELPER)


static WidgetInfo *
get_info (gpointer widget)
{
    return g_object_get_data (G_OBJECT (widget), "moo-config-helper-widget-info");
}


static void
moo_config_helper_destroy (GtkObject *object)
{
    MooConfigHelper *helper = MOO_CONFIG_HELPER (object);

    if (helper->widgets)
    {
        GSList *l;

        for (l = helper->widgets; l != NULL; l = l->next)
        {
            GtkWidget *widget = l->data;
            WidgetInfo *info = get_info (widget);

            if (info->handler)
                g_signal_handler_disconnect (widget, info->handler);

            g_object_set_data (G_OBJECT (widget), "moo-config-helper-widget-info", NULL);
        }

        g_slist_free (helper->widgets);
        helper->widgets = NULL;
    }

    GTK_OBJECT_CLASS(_moo_config_helper_parent_class)->destroy (object);
}


static gboolean
moo_config_helper_new_row (MooTreeHelper  *tree_helper,
                           GtkTreeModel   *model,
                           GtkTreePath    *path)
{
    int index;
    MooConfigItem *item;
    MooConfigHelper *helper = MOO_CONFIG_HELPER (tree_helper);

    g_return_val_if_fail (path != NULL, FALSE);
    g_return_val_if_fail (gtk_tree_path_get_depth (path) == 1, FALSE);

    index = gtk_tree_path_get_indices(path)[0];
    item = moo_config_new_item (MOO_CONFIG (model), index, TRUE);
    g_return_val_if_fail (item != NULL, FALSE);

    g_signal_emit (helper, config_signals[NEW_ITEM], 0, model, item);

    return TRUE;
}


static gboolean
moo_config_helper_delete_row (G_GNUC_UNUSED MooTreeHelper *tree_helper,
                              GtkTreeModel   *model,
                              GtkTreePath    *path)
{
    int index;

    g_return_val_if_fail (path != NULL, FALSE);
    g_return_val_if_fail (gtk_tree_path_get_depth (path) == 1, FALSE);

    index = gtk_tree_path_get_indices(path)[0];
    moo_config_delete_item (MOO_CONFIG (model), index, TRUE);

    return TRUE;
}


static gboolean
moo_config_helper_move_row (G_GNUC_UNUSED MooTreeHelper *helper,
                            GtkTreeModel   *model,
                            GtkTreePath    *old_path,
                            GtkTreePath    *new_path)
{
    g_return_val_if_fail (old_path && new_path, FALSE);
    g_return_val_if_fail (gtk_tree_path_get_depth (old_path) == 1, FALSE);
    g_return_val_if_fail (gtk_tree_path_get_depth (new_path) == 1, FALSE);
    moo_config_move_item (MOO_CONFIG (model),
                          gtk_tree_path_get_indices(old_path)[0],
                          gtk_tree_path_get_indices(new_path)[0],
                          TRUE);
    return TRUE;
}


static void
moo_config_helper_update_widgets_real (MooTreeHelper  *tree_helper,
                                       GtkTreeModel   *model,
                                       GtkTreePath    *path,
                                       GtkTreeIter    *iter)
{
    GSList *l;
    MooConfigItem *item = NULL;
    MooConfigHelper *helper = MOO_CONFIG_HELPER (tree_helper);

    g_return_if_fail (MOO_IS_CONFIG (model));

    if (path)
    {
        gtk_tree_model_get (model, iter, 0, &item, -1);
        g_return_if_fail (item != NULL);
    }

    for (l = helper->widgets; l != NULL; l = l->next)
    {
        GtkWidget *widget;
        WidgetInfo *info;

        widget = l->data;
        info = get_info (widget);

        if (info->handler)
            g_signal_handler_block (widget, info->handler);

        gtk_widget_set_sensitive (widget, item != NULL);

        if (item)
        {
            info->item_to_widget (MOO_CONFIG (model), item, widget, info->data);
        }
        else
        {
            if (GTK_IS_TOGGLE_BUTTON (widget))
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), FALSE);
            else if (GTK_IS_ENTRY (widget))
                gtk_entry_set_text (GTK_ENTRY (widget), "");
            else
                gtk_text_buffer_set_text (gtk_text_view_get_buffer (GTK_TEXT_VIEW (widget)),
                                          "", -1);
        }

        if (info->handler)
            g_signal_handler_unblock (widget, info->handler);
    }
}


static void
moo_config_helper_update_model_real (MooTreeHelper  *tree_helper,
                                     GtkTreeModel   *model,
                                     GtkTreePath    *path,
                                     GtkTreeIter    *iter)
{
    GSList *l;
    MooConfigItem *item = NULL;
    MooConfigHelper *helper = MOO_CONFIG_HELPER (tree_helper);

    g_return_if_fail (MOO_IS_CONFIG (model));
    g_return_if_fail (path != NULL);

    gtk_tree_model_get (model, iter, 0, &item, -1);

    if (!item)
        return;

    for (l = helper->widgets; l != NULL; l = l->next)
    {
        GtkWidget *widget;
        WidgetInfo *info;

        widget = l->data;
        info = get_info (widget);

        info->widget_to_item (widget, MOO_CONFIG (model), item, info->data);
    }
}


static void
_moo_config_helper_class_init (MooConfigHelperClass *klass)
{
    MooTreeHelperClass *tree_class = MOO_TREE_HELPER_CLASS (klass);

    GTK_OBJECT_CLASS(klass)->destroy = moo_config_helper_destroy;

    tree_class->new_row = moo_config_helper_new_row;
    tree_class->delete_row = moo_config_helper_delete_row;
    tree_class->move_row = moo_config_helper_move_row;
    tree_class->update_widgets = moo_config_helper_update_widgets_real;
    tree_class->update_model = moo_config_helper_update_model_real;

    config_signals[NEW_ITEM] =
            g_signal_new ("new-item",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooConfigHelperClass, new_item),
                          NULL, NULL,
                          _moo_marshal_VOID__OBJECT_POINTER,
                          G_TYPE_NONE, 2,
                          MOO_TYPE_CONFIG,
                          MOO_TYPE_CONFIG_ITEM);

    config_signals[SET_FROM_ITEM] =
            g_signal_new ("set-from-item",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooConfigHelperClass, set_from_item),
                          NULL, NULL,
                          _moo_marshal_VOID__OBJECT_POINTER,
                          G_TYPE_NONE, 2,
                          MOO_TYPE_CONFIG,
                          MOO_TYPE_CONFIG_ITEM);

    config_signals[SET_FROM_WIDGETS] =
            g_signal_new ("set-from-widgets",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooConfigHelperClass, set_from_widgets),
                          NULL, NULL,
                          _moo_marshal_VOID__OBJECT_POINTER,
                          G_TYPE_NONE, 2,
                          MOO_TYPE_CONFIG,
                          MOO_TYPE_CONFIG_ITEM);
}


static void
_moo_config_helper_init (MooConfigHelper *helper)
{
    helper->widgets = NULL;
}


MooConfigHelper *
moo_config_helper_new (GtkWidget *tree_view,
                       GtkWidget *new_btn,
                       GtkWidget *delete_btn,
                       GtkWidget *up_btn,
                       GtkWidget *down_btn)
{
    MooConfigHelper *helper;

    g_return_val_if_fail (GTK_IS_TREE_VIEW (tree_view), NULL);

    helper = g_object_new (MOO_TYPE_CONFIG_HELPER, NULL);
    gtk_object_sink (g_object_ref (helper));

    moo_tree_helper_connect (MOO_TREE_HELPER (helper), tree_view,
                             new_btn, delete_btn, up_btn, down_btn);

    return helper;
}


static void
button_toggled (MooTreeHelper   *helper,
                GtkToggleButton *button)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    MooConfigItem *item = NULL;
    WidgetInfo *info;
    GtkTreeSelection *selection;

    selection = gtk_tree_view_get_selection (helper->treeview);

    if (!gtk_tree_selection_get_selected (selection, &model, &iter))
        return;

    info = get_info (button);
    g_return_if_fail (info != NULL);

    gtk_tree_model_get (model, &iter, 0, &item, -1);
    g_return_if_fail (item != NULL);

    moo_config_set_bool (MOO_CONFIG (model), item, info->key,
                         gtk_toggle_button_get_active (button), TRUE);
}


static void
entry_changed (MooTreeHelper  *helper,
               GtkEntry       *entry)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    const char *text;
    MooConfigItem *item = NULL;
    WidgetInfo *info;
    GtkTreeSelection *selection;

    selection = gtk_tree_view_get_selection (helper->treeview);

    if (!gtk_tree_selection_get_selected (selection, &model, &iter))
        return;

    info = get_info (entry);
    g_return_if_fail (info != NULL);

    text = gtk_entry_get_text (entry);
    gtk_tree_model_get (model, &iter, 0, &item, -1);
    g_return_if_fail (item != NULL);

    if (info->key)
        moo_config_set (MOO_CONFIG (model), item,
                        info->key, text, TRUE);
    else
        moo_config_set_item_content (MOO_CONFIG (model), item,
                                     text, TRUE);
}


static void
toggle_button_to_item (GtkToggleButton *button,
                       MooConfig       *config,
                       MooConfigItem   *item)
{
    gboolean value;
    WidgetInfo *info = get_info (button);

    g_return_if_fail (info != NULL);

    value = gtk_toggle_button_get_active (button);
    moo_config_set_bool (config, item, info->key, value, TRUE);
}

static void
entry_to_item (GtkEntry        *entry,
               MooConfig       *config,
               MooConfigItem   *item)
{
    const char *text;
    WidgetInfo *info = get_info (entry);

    g_return_if_fail (info != NULL);

    text = gtk_entry_get_text (entry);

    if (info->key)
        moo_config_set (config, item, info->key, text, TRUE);
    else
        moo_config_set_item_content (config, item, text, TRUE);
}

static void
text_view_to_item (GtkTextView     *view,
                   MooConfig       *config,
                   MooConfigItem   *item)
{
    char *text;
    GtkTextBuffer *buffer;
    GtkTextIter start, end;
    WidgetInfo *info = get_info (view);

    g_return_if_fail (info != NULL);

    buffer = gtk_text_view_get_buffer (view);
    gtk_text_buffer_get_bounds (buffer, &start, &end);
    text = gtk_text_buffer_get_text (buffer, &start, &end, TRUE);

    if (!text[0])
    {
        g_free (text);
        text = NULL;
    }

    if (info->key)
        moo_config_set (config, item, info->key, text, TRUE);
    else
        moo_config_set_item_content (config, item, text, TRUE);

    g_free (text);
}

static MooConfigWidgetToItem
get_widget_to_item_func (GtkWidget *widget)
{
    if (GTK_IS_TOGGLE_BUTTON (widget))
        return (MooConfigWidgetToItem) toggle_button_to_item;
    else if (GTK_IS_ENTRY (widget))
        return (MooConfigWidgetToItem) entry_to_item;
    else if (GTK_IS_TEXT_VIEW (widget))
        return (MooConfigWidgetToItem) text_view_to_item;
    else
        return NULL;
}


static void
item_to_toggle_button (MooConfig       *config,
                       MooConfigItem   *item,
                       GtkToggleButton *button)
{
    gboolean value;
    WidgetInfo *info = get_info (button);

    g_return_if_fail (info != NULL);

    value = moo_config_get_bool (config, item, info->key);
    gtk_toggle_button_set_active (button, value);
}

static void
item_to_entry (G_GNUC_UNUSED MooConfig *config,
               MooConfigItem   *item,
               GtkEntry        *entry)
{
    const char *text;
    WidgetInfo *info = get_info (entry);

    g_return_if_fail (info != NULL);

    if (info->key)
        text = moo_config_item_get (item, info->key);
    else
        text = moo_config_item_get_content (item);

    gtk_entry_set_text (entry, text ? text : "");
}

static void
item_to_text_view (G_GNUC_UNUSED MooConfig *config,
                   MooConfigItem   *item,
                   GtkTextView     *view)
{
    const char *text;
    GtkTextBuffer *buffer;
    WidgetInfo *info = get_info (view);

    g_return_if_fail (info != NULL);

    if (info->key)
        text = moo_config_item_get (item, info->key);
    else
        text = moo_config_item_get_content (item);

    buffer = gtk_text_view_get_buffer (view);
    gtk_text_buffer_set_text (buffer, text ? text : "", -1);
}

static MooConfigItemToWidget
get_item_to_widget_func (GtkWidget *widget)
{
    if (GTK_IS_TOGGLE_BUTTON (widget))
        return (MooConfigItemToWidget) item_to_toggle_button;
    else if (GTK_IS_ENTRY (widget))
        return (MooConfigItemToWidget) item_to_entry;
    else if (GTK_IS_TEXT_VIEW (widget))
        return (MooConfigItemToWidget) item_to_text_view;
    else
        return NULL;
}


static WidgetInfo *
widget_info_new (const char *key,
                 gboolean    update_live)
{
    WidgetInfo *info = g_new0 (WidgetInfo, 1);

    info->key = g_strdup (key);
    info->update_live = update_live;

    return info;
}


static void
widget_info_free (WidgetInfo *info)
{
    if (info)
    {
        g_free (info->key);
        g_free (info);
    }
}


static void
moo_config_helper_add_widget_full (MooConfigHelper *helper,
                                   GtkWidget       *widget,
                                   const char      *key,
                                   MooConfigWidgetToItem widget_to_item_func,
                                   MooConfigItemToWidget item_to_widget_func,
                                   gpointer         data,
                                   gboolean         update_live)
{
    WidgetInfo *info;

    g_return_if_fail (!update_live || key != NULL);

    g_return_if_fail (GTK_IS_TOGGLE_BUTTON (widget) ||
            GTK_IS_ENTRY (widget) || GTK_IS_TEXT_VIEW (widget));

    if (!widget_to_item_func)
    {
        widget_to_item_func = get_widget_to_item_func (widget);
        g_return_if_fail (widget_to_item_func != NULL);
    }

    if (!item_to_widget_func)
    {
        item_to_widget_func = get_item_to_widget_func (widget);
        g_return_if_fail (item_to_widget_func != NULL);
    }

    info = widget_info_new (key, update_live);
    info->widget_to_item = widget_to_item_func;
    info->item_to_widget = item_to_widget_func;
    info->data = data;

    helper->widgets = g_slist_prepend (helper->widgets, widget);
    g_object_set_data_full (G_OBJECT (widget), "moo-config-helper-widget-info", info,
                            (GDestroyNotify) widget_info_free);

    if (update_live)
    {
        if (GTK_IS_TOGGLE_BUTTON (widget))
            info->handler =
                    g_signal_connect_swapped (widget, "toggled",
                                              G_CALLBACK (button_toggled),
                                              helper);
        else if (GTK_IS_ENTRY (widget))
            info->handler =
                    g_signal_connect_swapped (widget, "changed",
                                              G_CALLBACK (entry_changed),
                                              helper);
    }
}


void
moo_config_helper_add_widget (MooConfigHelper *helper,
                              GtkWidget       *widget,
                              const char      *key,
                              gboolean         update_live)
{
    g_return_if_fail (MOO_IS_CONFIG_HELPER (helper));
    g_return_if_fail (GTK_IS_WIDGET (widget));
    moo_config_helper_add_widget_full (helper, widget, key,
                                      NULL, NULL, NULL,
                                      update_live);
}


void
moo_config_helper_update_model (MooConfigHelper *helper,
                                GtkTreeModel    *model,
                                GtkTreePath     *path)
{
    g_return_if_fail (MOO_IS_CONFIG_HELPER (helper));
    moo_tree_helper_update_model (MOO_TREE_HELPER (helper), model, path);
}


void
moo_config_helper_update_widgets (MooConfigHelper *helper)
{
    GtkTreeView *treeview;
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;

    g_return_if_fail (MOO_IS_CONFIG_HELPER (helper));

    treeview = MOO_TREE_HELPER (helper)->treeview;
    selection = gtk_tree_view_get_selection (treeview);

    if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
        GtkTreePath *path = gtk_tree_model_get_path (model, &iter);
        moo_tree_helper_update_widgets (MOO_TREE_HELPER (helper), model, path);
        gtk_tree_path_free (path);
    }
    else
    {
        moo_tree_helper_update_widgets (MOO_TREE_HELPER (helper), model, NULL);
    }
}
