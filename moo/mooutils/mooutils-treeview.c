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


typedef struct {
    char *key;
    gboolean update_live;
    gulong handler;
    MooConfigWidgetToItem widget_to_item;
    MooConfigItemToWidget item_to_widget;
    gpointer data;
} WidgetInfo;

typedef struct {
    GtkTreeView *treeview;
    GSList *widgets;
    GtkWidget *new_btn;
    GtkWidget *delete_btn;
    GtkWidget *up_btn;
    GtkWidget *down_btn;
    MooConfigSetupItemFunc setup_func;
    gpointer data;
} Widgets;

static WidgetInfo  *widget_info_new     (const char         *key,
                                         gboolean            update_live);
static void         widget_info_free    (WidgetInfo         *info);
static Widgets     *widgets_new         (GtkWidget          *tree_view,
                                         GtkWidget          *new_btn,
                                         GtkWidget          *delete_btn,
                                         GtkWidget          *up_btn,
                                         GtkWidget          *down_btn);
static void         widgets_free        (Widgets            *widgets);

static void         selection_changed   (GtkTreeSelection   *selection,
                                         Widgets            *widgets);
static void         set_from_model      (Widgets            *widgets,
                                         GtkTreeModel       *model,
                                         GtkTreePath        *path);
static void         button_new          (Widgets            *widgets);
static void         button_delete       (Widgets            *widgets);
static void         button_up           (Widgets            *widgets);
static void         button_down         (Widgets            *widgets);
static void         entry_changed       (Widgets            *widgets,
                                         GtkEntry           *entry);
static void         button_toggled      (Widgets            *widgets,
                                         GtkToggleButton    *button);


static Widgets *
get_widgets (GtkWidget *treeview)
{
    return g_object_get_data (G_OBJECT (treeview), "moo-config-widgets");
}


void
moo_config_connect_widget (GtkWidget      *treeview,
                           GtkWidget      *new_btn,
                           GtkWidget      *delete_btn,
                           GtkWidget      *up_btn,
                           GtkWidget      *down_btn,
                           MooConfigSetupItemFunc func,
                           gpointer        data)
{
    Widgets *widgets;
    GtkTreeSelection *selection;

    g_return_if_fail (GTK_IS_TREE_VIEW (treeview));
    g_return_if_fail (!get_widgets (treeview));

    widgets = widgets_new (treeview, new_btn, delete_btn, up_btn, down_btn);
    widgets->setup_func = func;
    widgets->data = data;
    g_object_set_data_full (G_OBJECT (treeview), "moo-config-widgets", widgets,
                            (GDestroyNotify) widgets_free);
    g_signal_connect (treeview, "destroy",
                      G_CALLBACK (moo_config_disconnect_widget),
                      NULL);

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
    g_signal_connect (selection, "changed",
                      G_CALLBACK (selection_changed), widgets);

    if (new_btn)
        g_signal_connect_swapped (new_btn, "clicked", G_CALLBACK (button_new), widgets);
    if (delete_btn)
        g_signal_connect_swapped (delete_btn, "clicked", G_CALLBACK (button_delete), widgets);
    if (up_btn)
        g_signal_connect_swapped (up_btn, "clicked", G_CALLBACK (button_up), widgets);
    if (down_btn)
        g_signal_connect_swapped (down_btn, "clicked", G_CALLBACK (button_down), widgets);
}


static WidgetInfo *
get_info (gpointer widget)
{
    return g_object_get_data (G_OBJECT (widget), "moo-config-widget-info");
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


void
moo_config_add_widget_full (GtkWidget      *tree_view,
                            GtkWidget      *widget,
                            const char     *key,
                            MooConfigWidgetToItem widget_to_item_func,
                            MooConfigItemToWidget item_to_widget_func,
                            gpointer        data,
                            gboolean        update_live)
{
    Widgets *widgets;
    WidgetInfo *info;

    widgets = get_widgets (tree_view);
    g_return_if_fail (widgets != NULL);
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

    widgets->widgets = g_slist_prepend (widgets->widgets, widget);
    g_object_set_data_full (G_OBJECT (widget), "moo-config-widget-info", info,
                            (GDestroyNotify) widget_info_free);

    if (update_live)
    {
        if (GTK_IS_TOGGLE_BUTTON (widget))
            info->handler =
                    g_signal_connect_swapped (widget, "toggled",
                                              G_CALLBACK (button_toggled),
                                              widgets);
        else if (GTK_IS_ENTRY (widget))
            info->handler =
                    g_signal_connect_swapped (widget, "changed",
                                              G_CALLBACK (entry_changed),
                                              widgets);
    }
}


void
moo_config_add_widget (GtkWidget      *tree_view,
                       GtkWidget      *widget,
                       const char     *key,
                       gboolean        update_live)
{
    return moo_config_add_widget_full (tree_view, widget, key,
                                       NULL, NULL, NULL, update_live);
}


void
moo_config_disconnect_widget (GtkWidget *treeview)
{
    GtkTreeSelection *selection;
    Widgets *widgets;
    GSList *l;

    widgets = get_widgets (treeview);

    if (!widgets)
        return;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
    g_signal_handlers_disconnect_by_func (selection, (gpointer) selection_changed, widgets);

    if (widgets->new_btn)
        g_signal_handlers_disconnect_by_func (widgets->new_btn,
                                              (gpointer) button_new, widgets);
    if (widgets->delete_btn)
        g_signal_handlers_disconnect_by_func (widgets->delete_btn,
                                              (gpointer) button_delete, widgets);
    if (widgets->up_btn)
        g_signal_handlers_disconnect_by_func (widgets->up_btn,
                                              (gpointer) button_up, widgets);
    if (widgets->down_btn)
        g_signal_handlers_disconnect_by_func (widgets->down_btn,
                                              (gpointer) button_down, widgets);

    for (l = widgets->widgets; l != NULL; l = l->next)
    {
        GtkWidget *widget = l->data;
        WidgetInfo *info = get_info (widget);

        if (info->handler)
            g_signal_handler_disconnect (widget, info->handler);

        g_object_set_data (G_OBJECT (widget), "moo-config-widget-info", NULL);
    }

    g_object_set_data (G_OBJECT (treeview), "moo-config-widgets", NULL);
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


static Widgets *
widgets_new (GtkWidget *tree_view,
             GtkWidget *new_btn,
             GtkWidget *delete_btn,
             GtkWidget *up_btn,
             GtkWidget *down_btn)
{
    Widgets *w = g_new0 (Widgets, 1);
    w->treeview = GTK_TREE_VIEW (tree_view);
    w->new_btn = new_btn;
    w->delete_btn = delete_btn;
    w->up_btn = up_btn;
    w->down_btn = down_btn;
    return w;
}


static void
widgets_free (Widgets *widgets)
{
    if (widgets)
    {
        g_slist_free (widgets->widgets);
        g_free (widgets);
    }
}


static void
selection_changed (GtkTreeSelection *selection,
                   Widgets          *widgets)
{
    GtkTreeRowReference *old_row;
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreePath *path, *old_path;

    old_row = g_object_get_data (G_OBJECT (selection), "moo-config-current-row");
    old_path = old_row ? gtk_tree_row_reference_get_path (old_row) : NULL;

    if (old_row && !old_path)
    {
        g_object_set_data (G_OBJECT (selection), "moo-config-current-row", NULL);
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
        moo_config_update_tree_view (GTK_WIDGET (widgets->treeview),
                                     model, old_path);

    set_from_model (widgets, model, path);

    if (path)
    {
        GtkTreeRowReference *row;
        row = gtk_tree_row_reference_new (model, path);
        g_object_set_data_full (G_OBJECT (selection), "moo-config-current-row", row,
                                (GDestroyNotify) gtk_tree_row_reference_free);
    }
    else
    {
        g_object_set_data (G_OBJECT (selection), "moo-config-current-row", NULL);
    }

    gtk_tree_path_free (path);
    gtk_tree_path_free (old_path);
}


static void
set_from_model (Widgets      *widgets,
                GtkTreeModel *model,
                GtkTreePath  *path)
{
    GtkTreeIter iter;
    MooConfigItem *item = NULL;
    GSList *l;

    g_return_if_fail (MOO_IS_CONFIG (model));

    if (path)
    {
        gtk_tree_model_get_iter (model, &iter, path);
        gtk_tree_model_get (model, &iter, 0, &item, -1);
        g_return_if_fail (item != NULL);
    }

    for (l = widgets->widgets; l != NULL; l = l->next)
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

    if (!item)
    {
        if (widgets->delete_btn)
            gtk_widget_set_sensitive (widgets->delete_btn, FALSE);
        if (widgets->up_btn)
            gtk_widget_set_sensitive (widgets->up_btn, FALSE);
        if (widgets->down_btn)
            gtk_widget_set_sensitive (widgets->down_btn, FALSE);
    }
    else
    {
        int *indices;

        if (widgets->delete_btn)
            gtk_widget_set_sensitive (widgets->delete_btn, TRUE);

        indices = gtk_tree_path_get_indices (path);

        if (widgets->up_btn)
            gtk_widget_set_sensitive (widgets->up_btn, indices[0] != 0);
        if (widgets->down_btn)
            gtk_widget_set_sensitive (widgets->down_btn, (guint) indices[0] !=
                                      moo_config_n_items (MOO_CONFIG (model)) - 1);
    }
}


void
moo_config_update_widgets (GtkWidget *treeview)
{
    Widgets *widgets;
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;

    widgets = get_widgets (treeview);
    g_return_if_fail (widgets != NULL);

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));

    if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
        GtkTreePath *path = gtk_tree_model_get_path (model, &iter);
        set_from_model (widgets, model, path);
        gtk_tree_path_free (path);
    }
    else
    {
        set_from_model (widgets, model, NULL);
    }
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
button_new (Widgets *widgets)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreeSelection *selection;
    MooConfigItem *item;
    int index;

    selection = gtk_tree_view_get_selection (widgets->treeview);

    if (!gtk_tree_selection_get_selected (selection, &model, &iter))
        index = -1;
    else
        index = iter_get_index (model, &iter) + 1;

    item = moo_config_new_item (MOO_CONFIG (model), index, TRUE);

    if (widgets->setup_func)
        widgets->setup_func (MOO_CONFIG (model), item, widgets->data);

    moo_config_get_item_iter (MOO_CONFIG (model), item, &iter);
    gtk_tree_selection_select_iter (selection, &iter);
}


static void
button_delete (Widgets *widgets)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreeSelection *selection;
    int index;

    selection = gtk_tree_view_get_selection (widgets->treeview);

    if (!gtk_tree_selection_get_selected (selection, &model, &iter))
        g_return_if_reached ();

    index = iter_get_index (model, &iter);
    moo_config_delete_item (MOO_CONFIG (model), index, TRUE);

    if (gtk_tree_model_iter_nth_child (model, &iter, NULL, index))
        gtk_tree_selection_select_iter (selection, &iter);
}


static void
button_up (Widgets *widgets)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreePath *path, *new_path;
    GtkTreeSelection *selection;
    int *indices;

    selection = gtk_tree_view_get_selection (widgets->treeview);

    if (!gtk_tree_selection_get_selected (selection, &model, &iter))
        g_return_if_reached ();

    path = gtk_tree_model_get_path (model, &iter);
    indices = gtk_tree_path_get_indices (path);

    if (!indices[0])
        g_return_if_reached ();

    moo_config_move_item (MOO_CONFIG (model), indices[0], indices[0] - 1, TRUE);
    new_path = gtk_tree_path_new_from_indices (indices[0] - 1, -1);
    set_from_model (widgets, model, new_path);

    gtk_tree_path_free (new_path);
    gtk_tree_path_free (path);
}


static void
button_down (Widgets *widgets)
{
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreePath *path, *new_path;
    int *indices;
    int n_children;

    selection = gtk_tree_view_get_selection (widgets->treeview);

    if (!gtk_tree_selection_get_selected (selection, &model, &iter))
        g_return_if_reached ();

    path = gtk_tree_model_get_path (model, &iter);
    indices = gtk_tree_path_get_indices (path);
    n_children = gtk_tree_model_iter_n_children (model, NULL);

    if (indices[0] == n_children - 1)
        g_return_if_reached ();

    moo_config_move_item (MOO_CONFIG (model), indices[0], indices[0] + 1, TRUE);
    new_path = gtk_tree_path_new_from_indices (indices[0] + 1, -1);
    set_from_model (widgets, model, new_path);

    gtk_tree_path_free (new_path);
    gtk_tree_path_free (path);
}


static void
entry_changed (Widgets  *widgets,
               GtkEntry *entry)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    const char *text;
    MooConfigItem *item = NULL;
    WidgetInfo *info;
    GtkTreeSelection *selection;

    selection = gtk_tree_view_get_selection (widgets->treeview);

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
button_toggled (Widgets         *widgets,
                GtkToggleButton *button)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    MooConfigItem *item = NULL;
    WidgetInfo *info;
    GtkTreeSelection *selection;

    selection = gtk_tree_view_get_selection (widgets->treeview);

    if (!gtk_tree_selection_get_selected (selection, &model, &iter))
        return;

    info = get_info (button);
    g_return_if_fail (info != NULL);

    gtk_tree_model_get (model, &iter, 0, &item, -1);
    g_return_if_fail (item != NULL);

    moo_config_set_bool (MOO_CONFIG (model), item, info->key,
                         gtk_toggle_button_get_active (button), TRUE);
}


void
moo_config_update_tree_view (GtkWidget      *treeview,
                             GtkTreeModel   *model,
                             GtkTreePath    *path)
{
    GtkTreeIter iter;
    Widgets *widgets;
    GSList *l;
    MooConfigItem *item = NULL;

    widgets = get_widgets (treeview);
    g_return_if_fail (widgets != NULL);

    if (!model)
        model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));

    if (!path)
    {
        GtkTreeSelection *selection =
                gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
        if (gtk_tree_selection_get_selected (selection, NULL, &iter))
        {
            path = gtk_tree_model_get_path (model, &iter);
            gtk_tree_model_get_iter (model, &iter, path);
            gtk_tree_model_get (model, &iter, 0, &item, -1);
            gtk_tree_path_free (path);
        }
    }
    else
    {
        gtk_tree_model_get_iter (model, &iter, path);
        gtk_tree_model_get (model, &iter, 0, &item, -1);
    }

    if (!item)
        return;

    for (l = widgets->widgets; l != NULL; l = l->next)
    {
        GtkWidget *widget;
        WidgetInfo *info;

        widget = l->data;
        info = get_info (widget);

        info->widget_to_item (widget, MOO_CONFIG (model), item, info->data);
    }
}
