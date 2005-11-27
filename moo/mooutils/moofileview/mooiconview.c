/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *   mooiconview.c
 *
 *   Copyright (C) 2004-2005 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#include "mooiconview.h"
#include MOO_MARSHALS_H
#include "mooutils/mooutils-gobject.h"
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <string.h>
#include <math.h>


typedef struct _Column      Column;
typedef struct _Layout      Layout;
typedef struct _CellInfo    CellInfo;
typedef struct _Selection   Selection;
typedef struct _DndInfo     DndInfo;

struct _Column {
    GtkTreePath *first;
    int          index;
    int          width;
    int          offset;
    GPtrArray   *entries;
};


struct _Layout {
    int     num_rows;

    int     width;
    int     height;
    int     row_height;

    int     pixbuf_width;
    int     pixbuf_height;
    int     text_height;

    GSList *columns;
};


struct _CellInfo {
    GtkCellRenderer *cell;
    GSList *attributes;
    MooIconCellDataFunc func;
    gpointer func_data;
    GDestroyNotify destroy;
    gboolean show;
};


struct _DndInfo {
    GtkTargetList *dest_targets;
    GdkDragAction dest_actions;

    GdkModifierType start_button_mask;
    GtkTargetList *source_targets;
    GdkDragAction source_actions;

    guint scroll_timeout;
    GdkDragContext *drag_motion_context;

    guint drag_dest_inside : 1;
    guint source_enabled : 1;
    guint dest_enabled : 1;
};


struct _MooIconViewPrivate {
    GtkTreeModel    *model;

    CellInfo         pixbuf;
    CellInfo         text;
    int              pixel_icon_size;
    int              icon_size; /* GtkIconSize */

    int              xoffset;
    GtkAdjustment   *adjustment;
    GtkTreeRowReference *scroll_to;

    guint            update_idle;

    Layout          *layout;
    Selection       *selection;
    GtkTreeRowReference *cursor;
    GtkTreeRowReference *drop_dest;

    gboolean         mapped;

    DndInfo         *dnd_info;
    int              button_pressed;
    int              button_press_x;
    int              button_press_y;
};


static void     moo_icon_view_finalize      (GObject        *object);
static void     moo_icon_view_set_property  (GObject        *object,
                                             guint           prop_id,
                                             const GValue   *value,
                                             GParamSpec     *pspec);
static void     moo_icon_view_get_property  (GObject        *object,
                                             guint           prop_id,
                                             GValue         *value,
                                             GParamSpec     *pspec);

static void     moo_icon_view_map           (GtkWidget      *widget);
static void     moo_icon_view_realize       (GtkWidget      *widget);
static void     moo_icon_view_unrealize     (GtkWidget      *widget);
static void     moo_icon_view_size_request  (GtkWidget      *widget,
                                             GtkRequisition *requisition);
static void     moo_icon_view_size_allocate (GtkWidget      *widget,
                                             GtkAllocation  *allocation);
static gboolean moo_icon_view_expose        (GtkWidget      *widget,
                                             GdkEventExpose *event);

static gboolean moo_icon_view_button_press  (GtkWidget      *widget,
                                             GdkEventButton *event);
static gboolean moo_icon_view_button_release(GtkWidget      *widget,
                                             GdkEventButton *event);
static gboolean moo_icon_view_motion_notify (GtkWidget      *widget,
                                             GdkEventMotion *event);
static gboolean moo_icon_view_scroll_event  (GtkWidget      *widget,
                                             GdkEventScroll *event);

static gboolean moo_icon_view_maybe_drag    (MooIconView    *view,
                                             GdkEventMotion *event);
static void     moo_icon_view_drag_begin    (GtkWidget      *widget,
                                             GdkDragContext *context);
static void     moo_icon_view_drag_end      (GtkWidget      *widget,
                                             GdkDragContext *context);
static void     moo_icon_drag_data_received (GtkWidget      *widget,
                                             GdkDragContext *context,
                                             int             x,
                                             int             y,
                                             GtkSelectionData *data,
                                             guint           info,
                                             guint           time);
static gboolean moo_icon_view_drag_drop     (GtkWidget      *widget,
                                             GdkDragContext *context,
                                             int             x,
                                             int             y,
                                             guint           time);
static void     moo_icon_view_drag_leave    (GtkWidget      *widget,
                                             GdkDragContext *context,
                                             guint           time);
static gboolean moo_icon_view_drag_motion   (GtkWidget      *widget,
                                             GdkDragContext *context,
                                             int             x,
                                             int             y,
                                             guint           time);
static void     drag_scroll_stop            (MooIconView    *view);

static void     row_changed                 (GtkTreeModel   *model,
                                             GtkTreePath    *path,
                                             GtkTreeIter    *iter,
                                             MooIconView    *view);
static void     row_deleted                 (GtkTreeModel   *model,
                                             GtkTreePath    *path,
                                             MooIconView    *view);
static void     row_inserted                (GtkTreeModel   *model,
                                             GtkTreePath    *path,
                                             GtkTreeIter    *iter,
                                             MooIconView    *view);
static void     rows_reordered              (GtkTreeModel   *model,
                                             GtkTreePath    *path,
                                             GtkTreeIter    *iter,
                                             gpointer        whatsthis,
                                             MooIconView    *view);

static void     cell_data_func              (MooIconView        *view,
                                             GtkCellRenderer    *cell,
                                             GtkTreeModel       *model,
                                             GtkTreeIter        *iter,
                                             gpointer            cell_info);

static void     moo_icon_view_set_scroll_adjustments
                                                (GtkWidget      *widget,
                                                 GtkAdjustment  *hadj,
                                                 GtkAdjustment  *vadj);
static void     moo_icon_view_update_adjustment (MooIconView    *view);
static void     moo_icon_view_scroll_to         (MooIconView    *view,
                                                 int             offset);
static int      clamp_offset                    (MooIconView    *view,
                                                 int             offset);

static void     moo_icon_view_invalidate_layout (MooIconView    *view);
static gboolean moo_icon_view_update_layout     (MooIconView    *view);

static void     init_selection              (MooIconView    *view);
static void     free_selection              (MooIconView    *view);
static void     selection_clear             (MooIconView    *view);
static void     selection_row_deleted       (MooIconView    *view);
static void     cursor_row_deleted          (MooIconView    *view);

static void     free_attributes             (CellInfo       *info);
static gboolean check_empty                 (MooIconView    *view);
static void     init_layout                 (MooIconView    *view);
static void     destroy_layout              (MooIconView    *view);

static void     activate_item_at_cursor     (MooIconView    *view);
static void     add_move_binding            (GtkBindingSet  *binding_set,
                                             guint           keyval,
                                             guint           modmask,
                                             GtkMovementStep step,
                                             gint            count);
static gboolean move_cursor                 (MooIconView    *view,
                                             GtkMovementStep step,
                                             gint            count,
                                             gboolean        extend_selection);
static GtkTreePath *ensure_cursor           (MooIconView    *view);


/* MOO_TYPE_ICON_VIEW */
G_DEFINE_TYPE (MooIconView, moo_icon_view, GTK_TYPE_WIDGET)

enum {
    PROP_0,
    PROP_PIXBUF_CELL,
    PROP_TEXT_CELL,
    PROP_MODEL
};

enum {
    ROW_ACTIVATED,
    SET_SCROLL_ADJUSTMENTS,
    SELECTION_CHANGED,
    CURSOR_MOVED,

    ACTIVATE_ITEM_AT_CURSOR,
    MOVE_CURSOR,

    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

static void moo_icon_view_class_init (MooIconViewClass *klass)
{
    GtkBindingSet *binding_set;
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    gobject_class->finalize = moo_icon_view_finalize;
    gobject_class->set_property = moo_icon_view_set_property;
    gobject_class->get_property = moo_icon_view_get_property;

    widget_class->map = moo_icon_view_map;
    widget_class->realize = moo_icon_view_realize;
    widget_class->unrealize = moo_icon_view_unrealize;
    widget_class->size_request = moo_icon_view_size_request;
    widget_class->size_allocate = moo_icon_view_size_allocate;
    widget_class->expose_event = moo_icon_view_expose;
    widget_class->scroll_event = moo_icon_view_scroll_event;
    widget_class->button_press_event = moo_icon_view_button_press;
    widget_class->button_release_event = moo_icon_view_button_release;
    widget_class->motion_notify_event = moo_icon_view_motion_notify;
    widget_class->drag_begin = moo_icon_view_drag_begin;
    widget_class->drag_end = moo_icon_view_drag_end;
    widget_class->drag_data_received = moo_icon_drag_data_received;
    widget_class->drag_drop = moo_icon_view_drag_drop;
    widget_class->drag_leave = moo_icon_view_drag_leave;
    widget_class->drag_motion = moo_icon_view_drag_motion;

    klass->set_scroll_adjustments = moo_icon_view_set_scroll_adjustments;

    g_object_class_install_property (gobject_class,
                                     PROP_PIXBUF_CELL,
                                     g_param_spec_object ("pixbuf-cell",
                                             "pixbuf-cell",
                                             "pixbuf-cell",
                                             GTK_TYPE_CELL_RENDERER,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_TEXT_CELL,
                                     g_param_spec_object ("text-cell",
                                             "text-cell",
                                             "text-cell",
                                             GTK_TYPE_CELL_RENDERER,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_MODEL,
                                     g_param_spec_object ("model",
                                             "model",
                                             "model",
                                             GTK_TYPE_TREE_MODEL,
                                             G_PARAM_READWRITE));

    signals[ROW_ACTIVATED] =
            g_signal_new ("row-activated",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooIconViewClass, row_activated),
                          NULL, NULL,
                          _moo_marshal_VOID__BOXED,
                          G_TYPE_NONE, 1,
                          GTK_TYPE_TREE_PATH | G_SIGNAL_TYPE_STATIC_SCOPE);

    signals[SELECTION_CHANGED] =
            g_signal_new ("selection-changed",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooIconViewClass, selection_changed),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[CURSOR_MOVED] =
            g_signal_new ("cursor-moved",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooIconViewClass, cursor_moved),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 1,
                          GTK_TYPE_TREE_PATH | G_SIGNAL_TYPE_STATIC_SCOPE);

    signals[SET_SCROLL_ADJUSTMENTS] =
            g_signal_new ("set-scroll-adjustments",
                          G_OBJECT_CLASS_TYPE (gobject_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooIconViewClass, set_scroll_adjustments),
                          NULL, NULL,
                          _moo_marshal_VOID__OBJECT_OBJECT,
                          G_TYPE_NONE, 2,
                          GTK_TYPE_ADJUSTMENT,
                          GTK_TYPE_ADJUSTMENT);
    widget_class->set_scroll_adjustments_signal = signals[SET_SCROLL_ADJUSTMENTS];

    signals[ACTIVATE_ITEM_AT_CURSOR] =
            moo_signal_new_cb ("activate-item-at-cursor",
                               G_OBJECT_CLASS_TYPE (klass),
                               G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                               G_CALLBACK (activate_item_at_cursor),
                               NULL, NULL,
                               _moo_marshal_VOID__VOID,
                               G_TYPE_NONE, 0);

    signals[MOVE_CURSOR] =
            moo_signal_new_cb ("move-cursor",
                               G_OBJECT_CLASS_TYPE (klass),
                               G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                               G_CALLBACK (move_cursor),
                               NULL, NULL,
                               _moo_marshal_BOOLEAN__ENUM_INT_BOOLEAN,
                               G_TYPE_BOOLEAN, 3,
                               GTK_TYPE_MOVEMENT_STEP,
                               G_TYPE_INT, G_TYPE_BOOLEAN);

    binding_set = gtk_binding_set_by_class (klass);

    gtk_binding_entry_add_signal (binding_set, GDK_Return, 0, "activate-item-at-cursor", 0);
    gtk_binding_entry_add_signal (binding_set, GDK_ISO_Enter, 0, "activate-item-at-cursor", 0);
    gtk_binding_entry_add_signal (binding_set, GDK_KP_Enter, 0, "activate-item-at-cursor", 0);

    add_move_binding (binding_set, GDK_Up, 0,
                      GTK_MOVEMENT_DISPLAY_LINES, -1);
    add_move_binding (binding_set, GDK_KP_Up, 0,
                      GTK_MOVEMENT_DISPLAY_LINES, -1);

    add_move_binding (binding_set, GDK_Down, 0,
                      GTK_MOVEMENT_DISPLAY_LINES, 1);
    add_move_binding (binding_set, GDK_KP_Down, 0,
                      GTK_MOVEMENT_DISPLAY_LINES, 1);

    add_move_binding (binding_set, GDK_Home, 0,
                      GTK_MOVEMENT_BUFFER_ENDS, -1);
    add_move_binding (binding_set, GDK_KP_Home, 0,
                      GTK_MOVEMENT_BUFFER_ENDS, -1);

    add_move_binding (binding_set, GDK_End, 0,
                      GTK_MOVEMENT_BUFFER_ENDS, 1);
    add_move_binding (binding_set, GDK_KP_End, 0,
                      GTK_MOVEMENT_BUFFER_ENDS, 1);

    add_move_binding (binding_set, GDK_Page_Up, 0,
                      GTK_MOVEMENT_PAGES, -1);
    add_move_binding (binding_set, GDK_KP_Page_Up, 0,
                      GTK_MOVEMENT_PAGES, -1);

    add_move_binding (binding_set, GDK_Page_Down, 0,
                      GTK_MOVEMENT_PAGES, 1);
    add_move_binding (binding_set, GDK_KP_Page_Down, 0,
                      GTK_MOVEMENT_PAGES, 1);

    add_move_binding (binding_set, GDK_Right, 0,
                      GTK_MOVEMENT_VISUAL_POSITIONS, 1);
    add_move_binding (binding_set, GDK_Left, 0,
                      GTK_MOVEMENT_VISUAL_POSITIONS, -1);

    add_move_binding (binding_set, GDK_KP_Right, 0,
                      GTK_MOVEMENT_VISUAL_POSITIONS, 1);
    add_move_binding (binding_set, GDK_KP_Left, 0,
                      GTK_MOVEMENT_VISUAL_POSITIONS, -1);
}


static void     add_move_binding            (GtkBindingSet  *binding_set,
                                             guint           keyval,
                                             guint           modmask,
                                             GtkMovementStep step,
                                             gint            count)
{
    gtk_binding_entry_add_signal (binding_set, keyval, modmask,
                                  "move_cursor", 3,
                                  G_TYPE_ENUM, step,
                                  G_TYPE_INT, count,
                                  G_TYPE_BOOLEAN, FALSE);

    gtk_binding_entry_add_signal (binding_set, keyval, modmask | GDK_SHIFT_MASK,
                                  "move_cursor", 3,
                                  G_TYPE_ENUM, step,
                                  G_TYPE_INT, count,
                                  G_TYPE_BOOLEAN, TRUE);

    if (modmask & GDK_CONTROL_MASK)
        return;

    gtk_binding_entry_add_signal (binding_set, keyval, GDK_CONTROL_MASK,
                                  "move_cursor", 3,
                                  G_TYPE_ENUM, step,
                                  G_TYPE_INT, count,
                                  G_TYPE_BOOLEAN, FALSE);

    gtk_binding_entry_add_signal (binding_set, keyval, GDK_CONTROL_MASK | GDK_SHIFT_MASK,
                                  "move_cursor", 3,
                                  G_TYPE_ENUM, step,
                                  G_TYPE_INT, count,
                                  G_TYPE_BOOLEAN, TRUE);
}


static void
moo_icon_view_init (MooIconView *view)
{
    GtkWidget *widget = GTK_WIDGET (view);

    widget->allocation.width = -1;
    widget->allocation.height = -1;

    GTK_WIDGET_UNSET_FLAGS (view, GTK_NO_WINDOW);
    GTK_WIDGET_SET_FLAGS (view, GTK_CAN_FOCUS);

    view->priv = g_new0 (MooIconViewPrivate, 1);

    view->priv->pixbuf.cell = gtk_cell_renderer_pixbuf_new ();
    gtk_object_sink (g_object_ref (view->priv->pixbuf.cell));
    view->priv->pixbuf.attributes = NULL;
    view->priv->pixbuf.func = cell_data_func;
    view->priv->pixbuf.func_data = &view->priv->pixbuf;
    view->priv->pixbuf.destroy = NULL;
    view->priv->pixbuf.show = TRUE;

    view->priv->text.cell = gtk_cell_renderer_text_new ();
    gtk_object_sink (g_object_ref (view->priv->text.cell));
    view->priv->text.attributes = NULL;
    view->priv->text.func = cell_data_func;
    view->priv->text.func_data = &view->priv->text;
    view->priv->text.destroy = NULL;
    view->priv->text.show = TRUE;

    view->priv->pixel_icon_size = -1;
    view->priv->icon_size = -1;

    view->priv->xoffset = 0;
    moo_icon_view_set_adjustment (view, NULL);

    view->priv->update_idle = 0;

    init_layout (view);
    init_selection (view);

    view->priv->dnd_info = g_new0 (DndInfo, 1);
}


static void moo_icon_view_finalize  (GObject      *object)
{
    MooIconView *view = MOO_ICON_VIEW (object);

    if (view->priv->model)
        g_object_unref (view->priv->model);
    g_object_unref (view->priv->adjustment);
    if (view->priv->update_idle)
        g_source_remove (view->priv->update_idle);

    g_object_unref (view->priv->pixbuf.cell);
    free_attributes (&view->priv->pixbuf);
    if (view->priv->pixbuf.destroy)
        view->priv->pixbuf.destroy (view->priv->pixbuf.func_data);

    g_object_unref (view->priv->text.cell);
    free_attributes (&view->priv->text);
    if (view->priv->text.destroy)
        view->priv->text.destroy (view->priv->text.func_data);

    destroy_layout (view);
    free_selection (view);
    gtk_tree_row_reference_free (view->priv->cursor);
    gtk_tree_row_reference_free (view->priv->scroll_to);
    gtk_tree_row_reference_free (view->priv->drop_dest);

    if (view->priv->dnd_info->source_targets)
        gtk_target_list_unref (view->priv->dnd_info->source_targets);
    if (view->priv->dnd_info->dest_targets)
        gtk_target_list_unref (view->priv->dnd_info->dest_targets);
    g_free (view->priv->dnd_info);

    g_free (view->priv);

    G_OBJECT_CLASS (moo_icon_view_parent_class)->finalize (object);
}


GtkWidget   *moo_icon_view_new              (void)
{
    return GTK_WIDGET (g_object_new (MOO_TYPE_ICON_VIEW, NULL));
}


GtkWidget       *moo_icon_view_new_with_model   (GtkTreeModel   *model)
{
    return GTK_WIDGET (g_object_new (MOO_TYPE_ICON_VIEW,
                                     "model", model,
                                     NULL));
}


static void         moo_icon_view_set_property  (GObject        *object,
                                                 guint           prop_id,
                                                 const GValue   *value,
                                                 GParamSpec     *pspec)
{
    MooIconView *view = MOO_ICON_VIEW (object);

    switch (prop_id)
    {
        case PROP_MODEL:
            moo_icon_view_set_model (view, g_value_get_object (value));
            break;
        case PROP_PIXBUF_CELL:
            moo_icon_view_set_cell (view,
                                    MOO_ICON_VIEW_CELL_PIXBUF,
                                    g_value_get_object (value));
            break;
        case PROP_TEXT_CELL:
            moo_icon_view_set_cell (view,
                                    MOO_ICON_VIEW_CELL_TEXT,
                                    g_value_get_object (value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void         moo_icon_view_get_property  (GObject        *object,
                                                 guint           prop_id,
                                                 GValue         *value,
                                                 GParamSpec     *pspec)
{
    MooIconView *view = MOO_ICON_VIEW (object);

    switch (prop_id)
    {
        case PROP_MODEL:
            g_value_set_object (value, view->priv->model);
            break;
        case PROP_PIXBUF_CELL:
            g_value_set_object (value, view->priv->pixbuf.cell);
            break;
        case PROP_TEXT_CELL:
            g_value_set_object (value, view->priv->text.cell);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


GtkTreeModel   *moo_icon_view_get_model         (MooIconView    *view)
{
    g_return_val_if_fail (MOO_IS_ICON_VIEW (view), NULL);
    return view->priv->model;
}


GtkCellRenderer *moo_icon_view_get_cell         (MooIconView    *view,
                                                 MooIconViewCell cell_type)
{
    g_return_val_if_fail (MOO_IS_ICON_VIEW (view), NULL);
    g_return_val_if_fail (cell_type == MOO_ICON_VIEW_CELL_PIXBUF ||
            cell_type == MOO_ICON_VIEW_CELL_TEXT, NULL);
    if (cell_type == MOO_ICON_VIEW_CELL_PIXBUF)
        return view->priv->pixbuf.cell;
    else
        return view->priv->text.cell;
}


void            moo_icon_view_set_model         (MooIconView    *view,
                                                 GtkTreeModel   *model)
{
    g_return_if_fail (MOO_IS_ICON_VIEW (view));
    g_return_if_fail (!model || GTK_IS_TREE_MODEL (model));

    if (view->priv->model == model)
        return;

    if (view->priv->model)
    {
        g_signal_handlers_disconnect_by_func (view->priv->model,
                                              (gpointer) row_changed,
                                              view);
        g_signal_handlers_disconnect_by_func (view->priv->model,
                                              (gpointer) row_deleted,
                                              view);
        g_signal_handlers_disconnect_by_func (view->priv->model,
                                              (gpointer) row_inserted,
                                              view);
        g_signal_handlers_disconnect_by_func (view->priv->model,
                                              (gpointer) rows_reordered,
                                              view);

        g_object_unref (view->priv->model);
        view->priv->model = NULL;

        selection_clear (view);

        gtk_tree_row_reference_free (view->priv->cursor);
        gtk_tree_row_reference_free (view->priv->drop_dest);
        gtk_tree_row_reference_free (view->priv->scroll_to);
        view->priv->cursor = NULL;
        view->priv->drop_dest = NULL;
        view->priv->scroll_to = NULL;
    }

    if (model)
    {
        view->priv->model = model;
        g_object_ref (model);

        g_signal_connect (model, "row-changed",
                          G_CALLBACK (row_changed), view);
        g_signal_connect (model, "row-deleted",
                          G_CALLBACK (row_deleted), view);
        g_signal_connect (model, "row-inserted",
                          G_CALLBACK (row_inserted), view);
        g_signal_connect (model, "rows-reordered",
                          G_CALLBACK (rows_reordered), view);
    }

    moo_icon_view_invalidate_layout (view);

    g_object_notify (G_OBJECT (view), "model");
}


void             moo_icon_view_set_cell         (MooIconView    *view,
                                                 MooIconViewCell cell_type,
                                                 GtkCellRenderer*cell)
{
    CellInfo *info;

    g_return_if_fail (MOO_IS_ICON_VIEW (view));
    g_return_if_fail (GTK_IS_CELL_RENDERER (cell));
    g_return_if_fail (cell_type == MOO_ICON_VIEW_CELL_PIXBUF ||
            cell_type == MOO_ICON_VIEW_CELL_TEXT);

    if (cell_type == MOO_ICON_VIEW_CELL_PIXBUF)
        info = &view->priv->pixbuf;
    else
        info = &view->priv->text;

    if (info->cell == cell)
        return;

    g_object_unref (info->cell);

    info->cell = cell;
    gtk_object_sink (g_object_ref (cell));

    moo_icon_view_invalidate_layout (view);
}


static void     free_attributes (CellInfo   *info)
{
    GSList *l;
    for (l = info->attributes; l != NULL; l = l->next->next)
        g_free (l->data);
    g_slist_free (info->attributes);
    info->attributes = NULL;
}


void             moo_icon_view_clear_attributes (MooIconView    *view,
                                                 MooIconViewCell cell)
{
    CellInfo *info;

    g_return_if_fail (MOO_IS_ICON_VIEW (view));
    g_return_if_fail (cell == MOO_ICON_VIEW_CELL_PIXBUF ||
            cell == MOO_ICON_VIEW_CELL_TEXT);

    if (cell == MOO_ICON_VIEW_CELL_PIXBUF)
        info = &view->priv->pixbuf;
    else
        info = &view->priv->text;

    free_attributes (info);

    moo_icon_view_invalidate_layout (view);
}


static void         moo_icon_view_set_attributesv  (MooIconView    *view,
                                                    MooIconViewCell cell_type,
                                                    const char     *first_attr,
                                                    va_list         args)
{
    char *attribute;
    int column;
    CellInfo *info;

    moo_icon_view_clear_attributes (view, cell_type);

    attribute = (char*) first_attr;

    if (cell_type == MOO_ICON_VIEW_CELL_PIXBUF)
        info = &view->priv->pixbuf;
    else
        info = &view->priv->text;

    while (attribute != NULL)
    {
        column = va_arg (args, int);

        info->attributes = g_slist_prepend (info->attributes,
                                            GINT_TO_POINTER (column));
        info->attributes = g_slist_prepend (info->attributes,
                                            g_strdup (attribute));

        attribute = va_arg (args, char*);
    }

    moo_icon_view_invalidate_layout (view);
}


void             moo_icon_view_set_attributes   (MooIconView    *view,
                                                 MooIconViewCell cell_type,
                                                 const char     *first_attr,
                                                 ...)
{
    va_list args;

    g_return_if_fail (MOO_IS_ICON_VIEW (view));
    g_return_if_fail (cell_type == MOO_ICON_VIEW_CELL_PIXBUF ||
            cell_type == MOO_ICON_VIEW_CELL_TEXT);

    va_start (args, first_attr);
    moo_icon_view_set_attributesv (view, cell_type, first_attr, args);
    va_end (args);
}


void    moo_icon_view_set_cell_data_func    (MooIconView    *view,
                                             MooIconViewCell cell,
                                             MooIconCellDataFunc func,
                                             gpointer        func_data,
                                             GDestroyNotify  destroy)
{
    CellInfo *info;

    g_return_if_fail (MOO_IS_ICON_VIEW (view));
    g_return_if_fail (cell == MOO_ICON_VIEW_CELL_PIXBUF ||
            cell == MOO_ICON_VIEW_CELL_TEXT);

    if (cell == MOO_ICON_VIEW_CELL_PIXBUF)
        info = &view->priv->pixbuf;
    else
        info = &view->priv->text;

    if (info->destroy)
        info->destroy (info->func_data);

    if (!func)
    {
        info->func = cell_data_func;
        info->func_data = info;
        info->destroy = NULL;
    }
    else
    {
        info->func = func;
        info->func_data = func_data;
        info->destroy = destroy;
    }

    moo_icon_view_invalidate_layout (view);
}


static gboolean     check_empty                 (MooIconView    *view)
{
    return !view->priv->model || !view->priv->layout->columns;
}


static void     moo_icon_view_map           (GtkWidget      *widget)
{
    GTK_WIDGET_CLASS(moo_icon_view_parent_class)->map (widget);
    moo_icon_view_invalidate_layout (MOO_ICON_VIEW (widget));
}


static void     moo_icon_view_realize       (GtkWidget      *widget)
{
    static GdkWindowAttr attributes;
    gint attributes_mask;
    MooIconView *view;

    view = MOO_ICON_VIEW (widget);

    GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

    attributes.x = widget->allocation.x;
    attributes.y = widget->allocation.y;
    attributes.width = widget->allocation.width;
    attributes.height = widget->allocation.height;
    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.event_mask = gtk_widget_get_events (widget)
            | GDK_POINTER_MOTION_MASK
            | GDK_BUTTON_PRESS_MASK
            | GDK_BUTTON_RELEASE_MASK
            | GDK_EXPOSURE_MASK
            | GDK_ENTER_NOTIFY_MASK
            | GDK_LEAVE_NOTIFY_MASK;

    attributes.visual = gtk_widget_get_visual (widget);
    attributes.colormap = gtk_widget_get_colormap (widget);
    attributes.wclass = GDK_INPUT_OUTPUT;

    attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

    widget->window = gdk_window_new (gtk_widget_get_parent_window (widget),
                                     &attributes, attributes_mask);
    gdk_window_set_user_data (widget->window, widget);

    widget->style = gtk_style_attach (widget->style, widget->window);
    gdk_window_set_background (widget->window, &widget->style->base[GTK_STATE_NORMAL]);

    moo_icon_view_invalidate_layout (view);
}


static void     moo_icon_view_unrealize     (GtkWidget      *widget)
{
    gdk_window_set_user_data (widget->window, NULL);
    gdk_window_destroy (widget->window);
    widget->window = NULL;
    GTK_WIDGET_UNSET_FLAGS (widget, GTK_REALIZED);
}


static void     moo_icon_view_size_request  (G_GNUC_UNUSED GtkWidget *widget,
                                             GtkRequisition *requisition)
{
    requisition->width = 0;
    requisition->height = 0;
}


static void     moo_icon_view_size_allocate (GtkWidget      *widget,
                                             GtkAllocation  *allocation)
{
    gboolean height_changed = FALSE;
    MooIconView *view = MOO_ICON_VIEW (widget);

    if (GTK_WIDGET_REALIZED (widget))
    {
        if (widget->allocation.height < 0 ||
            view->priv->layout->row_height == 0)
        {
            height_changed = TRUE;
        }
        else
        {
            height_changed =
                    (allocation->height/view->priv->layout->row_height !=
                                view->priv->layout->num_rows);
        }
    }

    widget->allocation = *allocation;

    if (GTK_WIDGET_REALIZED (widget))
    {
        gdk_window_move_resize (widget->window,
                                allocation->x,
                                allocation->y,
                                allocation->width,
                                allocation->height);
    }

    if (height_changed)
        moo_icon_view_invalidate_layout (view);
    else
        moo_icon_view_update_adjustment (view);
}


static void     moo_icon_view_invalidate_layout (MooIconView    *view)
{
    if (!view->priv->update_idle)
        view->priv->update_idle =
                g_idle_add_full (G_PRIORITY_HIGH,
                                 (GSourceFunc) moo_icon_view_update_layout,
                                 view, NULL);
}


static void     init_layout                 (MooIconView    *view)
{
    view->priv->layout = g_new0 (Layout, 1);
}


static void     destroy_layout              (MooIconView    *view)
{
    GSList *l;

    for (l = view->priv->layout->columns; l != NULL; l = l->next)
    {
        Column *column = l->data;
        gtk_tree_path_free (column->first);
        g_ptr_array_free (column->entries, TRUE);
        g_free (column);
    }

    g_free (view->priv->layout);
    view->priv->layout = NULL;
}


static int      num_entries                 (Column         *column)
{
    g_assert (column != NULL);
    return column->entries->len;
}


static void     draw_column                 (MooIconView    *view,
                                             Column         *column,
                                             GdkRegion      *clip);
static void     draw_entry                  (MooIconView    *view,
                                             GtkTreeIter    *iter,
                                             GtkTreePath    *path,
                                             GdkRectangle   *entry_rect);

static gboolean moo_icon_view_expose        (GtkWidget      *widget,
                                             GdkEventExpose *event)
{
    GSList *l;
    GdkRegion *area;
    MooIconView *view = MOO_ICON_VIEW (widget);
    Layout *layout = view->priv->layout;

    if (check_empty (view))
        return TRUE;

    area = gdk_region_copy (event->region);
    gdk_region_offset (area, view->priv->xoffset, 0);

    for (l = layout->columns; l != NULL; l = l->next)
    {
        GdkRectangle column_rect;
        GdkRegion *column_region;
        Column *column;

        column = l->data;

        column_rect.x = column->offset;
        column_rect.y = 0;
        column_rect.width = column->width;
        column_rect.height = num_entries (column) * layout->row_height;

        column_region = gdk_region_rectangle (&column_rect);
        gdk_region_intersect (column_region, area);

        if (!gdk_region_empty (column_region))
        {
            draw_column (view, column, column_region);
        }

        gdk_region_destroy (column_region);
    }

    gdk_region_destroy (area);

    return TRUE;
}


static void     draw_column                 (MooIconView    *view,
                                             Column         *column,
                                             GdkRegion      *clip)
{
    int i;
    GdkRectangle clip_rect;
    GtkTreeIter iter;
    GtkTreePath *path;
    Layout *layout = view->priv->layout;

    gdk_region_get_clipbox (clip, &clip_rect);

    gtk_tree_model_get_iter (view->priv->model, &iter,
                             column->first);
    path = gtk_tree_path_copy (column->first);

    for (i = 0; i < num_entries (column); ++i)
    {
        GdkRectangle entry_rect, dummy;

        entry_rect.x = column->offset;
        entry_rect.y = i * layout->row_height;
        entry_rect.width = layout->pixbuf_width +
                GPOINTER_TO_INT (column->entries->pdata[i]);
        entry_rect.height = layout->row_height;

        if (gdk_rectangle_intersect (&entry_rect,
                                     &clip_rect,
                                     &dummy))
        {
            entry_rect.x -= view->priv->xoffset;
            draw_entry (view, &iter, path, &entry_rect);
        }

        gtk_tree_model_iter_next (view->priv->model, &iter);
        gtk_tree_path_next (path);
    }

    gtk_tree_path_free (path);
}


static void     draw_entry                  (MooIconView    *view,
                                             GtkTreeIter    *iter,
                                             GtkTreePath    *path,
                                             GdkRectangle   *entry_rect)
{
    GtkWidget *widget = GTK_WIDGET (view);
    GdkRectangle cell_area = *entry_rect;
    GtkCellRendererState state = 0;
    GtkTreePath *cursor_path, *drop_path;
    gboolean selected, cursor, drop;

    selected = moo_icon_view_path_is_selected (view, path);

    cursor_path = moo_icon_view_get_cursor (view);
    cursor = cursor_path != NULL && !gtk_tree_path_compare (cursor_path, path);
    gtk_tree_path_free (cursor_path);

    drop_path = moo_icon_view_get_drop_dest_row (view);
    drop = drop_path != NULL && !gtk_tree_path_compare (drop_path, path);
    gtk_tree_path_free (drop_path);

    if (selected || drop)
    {
        GdkGC *selection_gc;

        if (GTK_WIDGET_HAS_FOCUS (widget) || drop)
        {
            selection_gc = widget->style->base_gc [GTK_STATE_SELECTED];
            state = GTK_CELL_RENDERER_SELECTED | GTK_CELL_RENDERER_FOCUSED;
        }
        else
        {
            selection_gc = widget->style->base_gc [GTK_STATE_ACTIVE];
            state = GTK_CELL_RENDERER_SELECTED;
        }

        gdk_draw_rectangle (widget->window,
                            selection_gc,
                            TRUE,
                            entry_rect->x,
                            entry_rect->y,
                            entry_rect->width,
                            entry_rect->height);
    }

    if (view->priv->pixbuf.show && view->priv->layout->pixbuf_height > 0)
    {
        view->priv->pixbuf.func (view, view->priv->pixbuf.cell,
                                 view->priv->model, iter,
                                 view->priv->pixbuf.func_data);

        cell_area.width = view->priv->layout->pixbuf_width;

        gtk_cell_renderer_render (view->priv->pixbuf.cell,
                                  widget->window, widget,
                                  entry_rect,
                                  &cell_area,
                                  entry_rect,
                                  state);
    }

    if (view->priv->text.show && view->priv->layout->text_height > 0)
    {
        view->priv->text.func (view, view->priv->text.cell,
                               view->priv->model, iter,
                               view->priv->text.func_data);

        cell_area.x += view->priv->layout->pixbuf_width;
        cell_area.width = entry_rect->width - view->priv->layout->pixbuf_width;

        gtk_cell_renderer_render (view->priv->text.cell,
                                  widget->window, widget,
                                  entry_rect,
                                  &cell_area,
                                  entry_rect,
                                  state);
    }

    if (cursor || drop)
    {
        gtk_paint_focus (widget->style,
                         widget->window,
                         GTK_STATE_SELECTED,
                         entry_rect,
                         widget,
                         "icon_view",
                         entry_rect->x,
                         entry_rect->y,
                         entry_rect->width,
                         entry_rect->height);
    }
}


static void     cell_data_func              (G_GNUC_UNUSED MooIconView *view,
                                             GtkCellRenderer    *cell,
                                             GtkTreeModel       *model,
                                             GtkTreeIter        *iter,
                                             gpointer            cell_info)
{
    GSList *l;
    CellInfo *info = cell_info;
    static GValue value;

    for (l = info->attributes; l && l->next; l = l->next->next)
    {
        gtk_tree_model_get_value (model, iter,
                                  GPOINTER_TO_INT (l->next->data),
                                  &value);
        g_object_set_property (G_OBJECT (cell), (char*)l->data, &value);
        g_value_unset (&value);
    }
}


/****************************************************************************/
/* Creating layout
 */

static gboolean model_empty (GtkTreeModel *model)
{
    GtkTreeIter iter;
    return !gtk_tree_model_get_iter_first (model, &iter);
}


static void     calculate_pixbuf_size   (MooIconView    *view);
static void     calculate_row_height    (MooIconView    *view);
static gboolean calculate_column_width  (MooIconView    *view,
                                         GtkTreeIter    *iter,
                                         Column         *column);


static gboolean moo_icon_view_update_layout     (MooIconView    *view)
{
    GtkWidget *widget = GTK_WIDGET (view);
    Layout *layout = view->priv->layout;
    GtkTreeModel *model = view->priv->model;
    GtkTreeIter iter;
    gboolean finish;
    GSList *l;
    int column_index;

    view->priv->update_idle = 0;
    gtk_widget_queue_draw (GTK_WIDGET (view));

    for (l = layout->columns; l != NULL; l = l->next)
    {
        Column *column = l->data;
        gtk_tree_path_free (column->first);
        g_ptr_array_free (column->entries, TRUE);
        g_free (column);
    }

    layout->columns = NULL;
    layout->num_rows = 0;

    layout->width = 0;
    layout->height = 0;
    layout->row_height = 0;

    layout->pixbuf_width = 0;
    layout->pixbuf_height = 0;
    layout->text_height = 0;

    if (!GTK_WIDGET_REALIZED (view) ||
         !GTK_WIDGET_MAPPED (view) ||
         !view->priv->model ||
         model_empty (view->priv->model))
    {
        view->priv->xoffset = 0;
        moo_icon_view_update_adjustment (view);
        return FALSE;
    }

    calculate_pixbuf_size (view);
    calculate_row_height (view);

    layout->num_rows = MAX (widget->allocation.height / layout->row_height, 1);
    layout->height = layout->row_height * layout->num_rows;
    layout->width = 0;

    gtk_tree_model_get_iter_first (model, &iter);
    finish = FALSE;
    column_index = 0;

    while (!finish)
    {
        Column *column = g_new0 (Column, 1);

        column->index = column_index++;
        column->offset = layout->width;
        column->entries = g_ptr_array_new ();

        column->first = gtk_tree_model_get_path (model, &iter);

        finish = !calculate_column_width (view, &iter, column);

        layout->width += column->width;
        layout->columns = g_slist_append (layout->columns, column);
    }

    if (view->priv->scroll_to)
    {
        if (gtk_tree_row_reference_valid (view->priv->scroll_to))
        {
            GtkTreePath *path = gtk_tree_row_reference_get_path (view->priv->scroll_to);
            moo_icon_view_update_adjustment (view);
            moo_icon_view_scroll_to_cell (view, path);
            gtk_tree_path_free (path);
        }

        gtk_tree_row_reference_free (view->priv->scroll_to);
        view->priv->scroll_to = NULL;
    }
    else
    {
        view->priv->xoffset = clamp_offset (view, view->priv->xoffset);
        moo_icon_view_update_adjustment (view);
    }

    return FALSE;
}


static void     set_pixbuf_size         (MooIconView    *view,
                                         int             width,
                                         int             height)
{
    if (width < 0) width = 0;
    if (height < 0) height = 0;

    if (!width || !height)
    {
        view->priv->layout->pixbuf_width = 0;
        view->priv->layout->pixbuf_height = 0;
    }
    else
    {
        view->priv->layout->pixbuf_width = width;
        view->priv->layout->pixbuf_height = height;
    }
}


static void     calculate_pixbuf_size   (MooIconView    *view)
{
    GtkTreeIter iter;
    int width, height;

    if (!view->priv->pixbuf.show)
        return set_pixbuf_size (view, 0, 0);

    if (view->priv->pixel_icon_size >= 0)
        return set_pixbuf_size (view,
                                view->priv->pixel_icon_size,
                                view->priv->pixel_icon_size);

    if (view->priv->icon_size >= 0)
    {
        if (gtk_icon_size_lookup (view->priv->icon_size, &width, &height))
            return set_pixbuf_size (view, width, height);
    }

    g_object_get (view->priv->pixbuf.cell,
                  "height", &height,
                  "width", &width,
                  NULL);

    if (height >= 0 && width >= 0)
        return set_pixbuf_size (view, width, height);

    gtk_tree_model_get_iter_first (view->priv->model, &iter);
    view->priv->pixbuf.func (view, view->priv->pixbuf.cell,
                             view->priv->model, &iter,
                             view->priv->pixbuf.func_data);
    gtk_cell_renderer_get_size (view->priv->pixbuf.cell,
                                GTK_WIDGET (view), NULL,
                                NULL, NULL, &width, &height);

    set_pixbuf_size (view, width, height);
}


static void     set_text_height         (MooIconView    *view,
                                         int             height)
{
    if (height < 0) height = 0;
    view->priv->layout->text_height = height;
    view->priv->layout->row_height =
            MAX (view->priv->layout->pixbuf_height, height);
}


static void     calculate_row_height    (MooIconView    *view)
{
    GtkTreeIter iter;
    int height;

    if (!view->priv->text.show)
        return set_text_height (view, 0);

    g_object_get (view->priv->text.cell,
                  "height", &height,
                  NULL);

    if (height >= 0)
        return set_text_height (view, height);

    gtk_tree_model_get_iter_first (view->priv->model, &iter);
    view->priv->text.func (view, view->priv->text.cell,
                           view->priv->model, &iter,
                           view->priv->text.func_data);
    gtk_cell_renderer_get_size (view->priv->text.cell,
                                GTK_WIDGET (view), NULL,
                                NULL, NULL, NULL, &height);

    set_text_height (view, height);
}


static gboolean calculate_column_width  (MooIconView    *view,
                                         GtkTreeIter    *iter,
                                         Column         *column)
{
    Layout *layout = view->priv->layout;
    int max_num = layout->num_rows;
    GtkWidget *widget = GTK_WIDGET (view);
    gboolean use_text = (layout->text_height > 0);
    GtkTreeModel *model = view->priv->model;

    g_ptr_array_set_size (column->entries, 0);
    column->width = 0;

    while (TRUE)
    {
        g_ptr_array_add (column->entries, NULL);

        if (use_text)
        {
            int text_width;

            view->priv->text.func (view, view->priv->text.cell,
                                   model, iter,
                                   view->priv->text.func_data);
            gtk_cell_renderer_get_size (view->priv->text.cell,
                                        widget, NULL, NULL, NULL,
                                        &text_width, NULL);
            if (column->width < text_width)
                column->width = text_width;

            column->entries->pdata[num_entries (column) - 1] =
                    GINT_TO_POINTER (text_width);
        }

        if (num_entries (column) == max_num)
        {
            column->width += layout->pixbuf_width;
            return gtk_tree_model_iter_next (model, iter);
        }
        else
        {
            if (!gtk_tree_model_iter_next (model, iter))
            {
                column->width += layout->pixbuf_width;
                return FALSE;
            }
        }
    }
}


static int      path_get_index              (GtkTreePath    *path)
{
    g_assert (gtk_tree_path_get_depth (path) == 1);
    return gtk_tree_path_get_indices(path)[0];
}


static Column  *find_column_by_path         (MooIconView    *view,
                                             GtkTreePath    *path,
                                             int            *index_)
{
    GSList *l;
    int path_index;

    path_index = path_get_index (path);

    for (l = view->priv->layout->columns; l != NULL; l = l->next)
    {
        Column *column = l->data;
        int first = path_get_index (column->first);
        if (first <= path_index && path_index < first +
            num_entries (column))
        {
            if (index_)
                *index_ = path_index - first;
            return column;
        }
    }

    g_return_val_if_reached (NULL);
}


static void     row_changed                 (G_GNUC_UNUSED GtkTreeModel *model,
                                             GtkTreePath    *path,
                                             G_GNUC_UNUSED GtkTreeIter *iter,
                                             MooIconView    *view)
{
    if (!GTK_WIDGET_REALIZED (view) ||
         !GTK_WIDGET_MAPPED (view))
            return;

    if (gtk_tree_path_get_depth (path) != 1)
        return;

    drag_scroll_stop (view);
    moo_icon_view_invalidate_layout (view);
}


static void     row_deleted                 (G_GNUC_UNUSED GtkTreeModel *model,
                                             GtkTreePath    *path,
                                             MooIconView    *view)
{
    if (gtk_tree_path_get_depth (path) != 1)
        return;

    selection_row_deleted (view);
    cursor_row_deleted (view);

    if (!GTK_WIDGET_REALIZED (view) ||
         !GTK_WIDGET_MAPPED (view))
            return;

    drag_scroll_stop (view);
    moo_icon_view_invalidate_layout (view);
}


static void     row_inserted                (G_GNUC_UNUSED GtkTreeModel *model,
                                             GtkTreePath    *path,
                                             G_GNUC_UNUSED GtkTreeIter *iter,
                                             MooIconView    *view)
{
    if (!GTK_WIDGET_REALIZED (view) ||
         !GTK_WIDGET_MAPPED (view))
            return;

    if (gtk_tree_path_get_depth (path) != 1)
        return;

    drag_scroll_stop (view);
    moo_icon_view_invalidate_layout (view);
}


static void     rows_reordered              (G_GNUC_UNUSED GtkTreeModel *model,
                                             GtkTreePath    *path,
                                             G_GNUC_UNUSED GtkTreeIter *iter,
                                             G_GNUC_UNUSED gpointer whatever,
                                             MooIconView    *view)
{
    if (!GTK_WIDGET_REALIZED (view) ||
         !GTK_WIDGET_MAPPED (view))
            return;

    if (gtk_tree_path_get_depth (path) != 0)
        return;

    drag_scroll_stop (view);
    moo_icon_view_invalidate_layout (view);
}


static void     invalidate_cell_rect        (MooIconView    *view,
                                             Column         *column,
                                             int             index_)
{
    GdkRectangle rect;

    if (!GTK_WIDGET_REALIZED (view) || view->priv->update_idle)
        return;

    rect.x = column->offset - view->priv->xoffset;
    rect.y = index_ * view->priv->layout->row_height;
    rect.width = column->width;
    rect.height = view->priv->layout->row_height;

    gdk_window_invalidate_rect (GTK_WIDGET(view)->window, &rect, FALSE);
}


static void
moo_icon_view_set_scroll_adjustments    (GtkWidget      *widget,
                                         GtkAdjustment  *hadj,
                                         G_GNUC_UNUSED GtkAdjustment *vadj)
{
    moo_icon_view_set_adjustment (MOO_ICON_VIEW (widget), hadj);
}


static void     value_changed           (MooIconView    *view,
                                         GtkAdjustment  *adj)
{
    if (adj->value != view->priv->xoffset)
        moo_icon_view_scroll_to (view, adj->value);
}


static void     moo_icon_view_update_adjustment (MooIconView    *view)
{
    GSList *link;

    link = g_slist_last (view->priv->layout->columns);
    view->priv->xoffset = clamp_offset (view, view->priv->xoffset);

    if (!link || view->priv->layout->width <= GTK_WIDGET(view)->allocation.width)
    {
        view->priv->adjustment->lower = 0;
        view->priv->adjustment->upper = GTK_WIDGET(view)->allocation.width - 1;
        view->priv->adjustment->value = 0;
        view->priv->adjustment->step_increment = GTK_WIDGET(view)->allocation.width - 1;
        view->priv->adjustment->page_increment = GTK_WIDGET(view)->allocation.width - 1;
        view->priv->adjustment->page_size = GTK_WIDGET(view)->allocation.width - 1;
    }
    else
    {
        Column *column = link->data;

        view->priv->adjustment->lower = 0;
        view->priv->adjustment->upper =
                view->priv->layout->width - 1;

        view->priv->adjustment->value = view->priv->xoffset;

        view->priv->adjustment->page_increment = GTK_WIDGET(view)->allocation.width;
        view->priv->adjustment->step_increment =
                view->priv->layout->width / (column->index + 1);

        view->priv->adjustment->page_size = GTK_WIDGET(view)->allocation.width;
    }

    gtk_adjustment_changed (view->priv->adjustment);
}


void    moo_icon_view_set_adjustment    (MooIconView    *view,
                                         GtkAdjustment  *adjustment)
{
    g_return_if_fail (MOO_IS_ICON_VIEW (view));
    g_return_if_fail (!adjustment || GTK_IS_ADJUSTMENT (adjustment));

    if (!adjustment)
        adjustment = GTK_ADJUSTMENT (gtk_adjustment_new (0,0,0,0,0,0));

    if (view->priv->adjustment)
    {
        g_signal_handlers_disconnect_by_func (view->priv->adjustment,
                                              (gpointer) value_changed,
                                              view);
        g_object_unref (view->priv->adjustment);
    }

    view->priv->adjustment = adjustment;
    gtk_object_sink (g_object_ref (adjustment));

    g_signal_connect_swapped (adjustment, "value-changed",
                              G_CALLBACK (value_changed),
                              view);

    if (GTK_WIDGET_REALIZED (view) && GTK_WIDGET_MAPPED (view))
        moo_icon_view_update_adjustment (view);
}


static gboolean
moo_icon_view_button_press (GtkWidget      *widget,
                            GdkEventButton *event)
{
    MooIconView *view = MOO_ICON_VIEW (widget);
    GtkTreePath *path = NULL;
    GtkTreePath *cursor_path = NULL;
    GdkModifierType mods = event->state & gtk_accelerator_get_default_mod_mask ();

    view->priv->button_pressed = 0;

    if (event->button == 1)
    {
        gtk_widget_grab_focus (widget);
        moo_icon_view_get_path_at_pos (view, event->x, event->y,
                                       &path, NULL, NULL, NULL);

        if (path)
        {
            switch (event->type)
            {
                case GDK_BUTTON_PRESS:
                    if (mods & GDK_SHIFT_MASK)
                    {
                        cursor_path = ensure_cursor (view);
                        moo_icon_view_unselect_all (view);
                        moo_icon_view_select_range (view, path, cursor_path);
                    }
                    else if (mods & GDK_CONTROL_MASK)
                    {
                        if (moo_icon_view_path_is_selected (view, path))
                            moo_icon_view_unselect_path (view, path);
                        else
                            moo_icon_view_select_path (view, path);
                    }
                    else
                    {
                        moo_icon_view_unselect_all (view);
                        moo_icon_view_set_cursor (view, path, FALSE);
                    }

                    gtk_tree_path_free (path);
                    gtk_tree_path_free (cursor_path);

                    /* do i care? */
                    view->priv->button_pressed = event->button;
                    view->priv->button_press_x = event->x;
                    view->priv->button_press_y = event->y;

                    return TRUE;

                case GDK_2BUTTON_PRESS:
                    activate_item_at_cursor (view);
                    gtk_tree_path_free (path);
                    return TRUE;

                default:
                    gtk_tree_path_free (path);
                    return FALSE;
            }
        }
    }

    return FALSE;
}


static gboolean
moo_icon_view_motion_notify (GtkWidget      *widget,
                             GdkEventMotion *event)
{
    MooIconView *view = MOO_ICON_VIEW (widget);
    drag_scroll_stop (view);
    return moo_icon_view_maybe_drag (view, event);
}


static gboolean
moo_icon_view_button_release (GtkWidget      *widget,
                              G_GNUC_UNUSED GdkEventButton *event)
{
    MooIconView *view = MOO_ICON_VIEW (widget);
    view->priv->button_pressed = 0;
    return FALSE;
}


static void invalidate_path_rectangle       (MooIconView    *view,
                                             GtkTreePath    *path)
{
    Column *column;
    int index_;

    if (!GTK_WIDGET_REALIZED (view) || view->priv->update_idle)
        return;

    if (check_empty (view))
        return;

    column = find_column_by_path (view, path, &index_);
    g_return_if_fail (column != NULL);
    invalidate_cell_rect (view, column, index_);
}


static void     move_cursor_right           (MooIconView    *view,
                                             gboolean        extend_selection);
static void     move_cursor_left            (MooIconView    *view,
                                             gboolean        extend_selection);
static void     move_cursor_up              (MooIconView    *view,
                                             gboolean        extend_selection);
static void     move_cursor_down            (MooIconView    *view,
                                             gboolean        extend_selection);
static void     move_cursor_page_up         (MooIconView    *view,
                                             gboolean        extend_selection);
static void     move_cursor_page_down       (MooIconView    *view,
                                             gboolean        extend_selection);
static void     move_cursor_home            (MooIconView    *view,
                                             gboolean        extend_selection);
static void     move_cursor_end             (MooIconView    *view,
                                             gboolean        extend_selection);

static gboolean move_cursor                 (MooIconView    *view,
                                             GtkMovementStep step,
                                             gint            count,
                                             gboolean        extend_selection)
{
    int i;

    if (check_empty (view))
        return TRUE;

    switch (step)
    {
        case GTK_MOVEMENT_LOGICAL_POSITIONS: /* left/right */
        case GTK_MOVEMENT_VISUAL_POSITIONS:
            if (count > 0)
                for (i = 0; i < count; ++i)
                    move_cursor_right (view, extend_selection);
            else
                for (i = 0; i < -count; ++i)
                    move_cursor_left (view, extend_selection);
            return TRUE;

        case GTK_MOVEMENT_DISPLAY_LINES:     /* up/down */
            if (count > 0)
                for (i = 0; i < count; ++i)
                    move_cursor_down (view, extend_selection);
            else
                for (i = 0; i < -count; ++i)
                    move_cursor_up (view, extend_selection);
            return TRUE;

        case GTK_MOVEMENT_PAGES:             /* pgup/pgdown */
            if (count > 0)
                for (i = 0; i < count; ++i)
                    move_cursor_page_down (view, extend_selection);
            else
                for (i = 0; i < -count; ++i)
                    move_cursor_page_up (view, extend_selection);
            return TRUE;

        case GTK_MOVEMENT_BUFFER_ENDS:       /* home/end */
            if (count > 0)
                move_cursor_end (view, extend_selection);
            else if (count < 0)
                move_cursor_home (view, extend_selection);
            return TRUE;

        default:
            g_return_val_if_reached (FALSE);
    }
}


static GtkTreePath *column_get_path         (Column         *column,
                                             int             index_)
{
    int first;

    g_return_val_if_fail (column != NULL, NULL);
    g_return_val_if_fail (index_ >= 0, NULL);
    g_return_val_if_fail (index_ < num_entries (column), NULL);

    first = gtk_tree_path_get_indices(column->first)[0];
    return gtk_tree_path_new_from_indices (first + index_, -1);
}


static int      get_n_columns               (MooIconView    *view)
{
    return g_slist_length (view->priv->layout->columns);
}

static Column  *get_nth_column              (MooIconView    *view,
                                             int             n)
{
    return g_slist_nth_data (view->priv->layout->columns, n);
}

static Column  *column_next                 (MooIconView    *view,
                                             Column         *column)
{
    g_return_val_if_fail (column != NULL, NULL);
    if (column->index == get_n_columns (view) - 1)
        return NULL;
    else
        return get_nth_column (view, column->index + 1);
}

static Column  *column_prev                 (MooIconView    *view,
                                             Column         *column)
{
    g_return_val_if_fail (column != NULL, NULL);
    if (column->index == 0)
        return NULL;
    else
        return get_nth_column (view, column->index - 1);
}


static GtkTreePath  *ensure_cursor      (MooIconView    *view)
{
    if (!view->priv->cursor)
    {
        GtkTreePath *path = gtk_tree_path_new_from_indices (0, -1);
        view->priv->cursor = gtk_tree_row_reference_new (view->priv->model, path);
        return path;
    }
    else
    {
        return gtk_tree_row_reference_get_path (view->priv->cursor);
    }
}


static void     move_cursor_to_entry        (MooIconView    *view,
                                             Column         *column,
                                             int             index_,
                                             gboolean        extend_selection)
{
    GtkTreePath *path = column_get_path (column, index_);

    if (extend_selection)
    {
        GtkTreePath *cursor_path = ensure_cursor (view);
        moo_icon_view_select_range (view, cursor_path, path);
        gtk_tree_path_free (cursor_path);
    }
    else
    {
        moo_icon_view_unselect_all (view);
    }

    moo_icon_view_set_cursor (view, path, FALSE);
    moo_icon_view_scroll_to_cell (view, path);
    gtk_tree_path_free (path);
}


static void     move_cursor_right           (MooIconView    *view,
                                             gboolean        extend_selection)
{
    GtkTreePath *path;
    Column *column, *next;
    int y;

    path = ensure_cursor (view);

    column = find_column_by_path (view, path, &y);
    g_return_if_fail (column != NULL);

    next = column_next (view, column);

    if (!next)
    {
        move_cursor_to_entry (view, column,
                              num_entries (column) - 1,
                              extend_selection);
    }
    else
    {
        if (y >= num_entries (next))
            y = num_entries (next) - 1;
        move_cursor_to_entry (view, next, y,
                              extend_selection);
    }

    gtk_tree_path_free (path);
}


static void     move_cursor_left            (MooIconView    *view,
                                             gboolean        extend_selection)
{
    GtkTreePath *path;
    Column *column, *prev;
    int index_;

    path = ensure_cursor (view);

    column = find_column_by_path (view, path, &index_);
    g_return_if_fail (column != NULL);

    prev = column_prev (view, column);

    if (!prev)
        move_cursor_to_entry (view, column, 0,
                              extend_selection);
    else
        move_cursor_to_entry (view, prev, index_,
                              extend_selection);

    gtk_tree_path_free (path);
}


static void     move_cursor_up              (MooIconView    *view,
                                             gboolean        extend_selection)
{
    GtkTreePath *path;
    Column *column, *prev;
    int y;

    path = ensure_cursor (view);

    column = find_column_by_path (view, path, &y);
    g_return_if_fail (column != NULL);

    if (y)
    {
        move_cursor_to_entry (view, column, y - 1,
                              extend_selection);
    }
    else
    {
        prev = column_prev (view, column);

        if (!prev)
            moo_icon_view_select_path (view, path);
        else
            move_cursor_to_entry (view, prev, prev->entries->len - 1,
                                  extend_selection);
    }

    gtk_tree_path_free (path);
}


static void     move_cursor_down            (MooIconView    *view,
                                             gboolean        extend_selection)
{
    GtkTreePath *path;
    Column *column, *next;
    int y;

    path = ensure_cursor (view);

    column = find_column_by_path (view, path, &y);
    g_return_if_fail (column != NULL);

    if (y < num_entries (column) - 1)
    {
        move_cursor_to_entry (view, column, y + 1,
                              extend_selection);
    }
    else
    {
        next = column_next (view, column);

        if (!next)
            moo_icon_view_select_path (view, path);
        else
            move_cursor_to_entry (view, next, 0,
                                  extend_selection);
    }

    gtk_tree_path_free (path);
}


static void     move_cursor_page_up         (MooIconView    *view,
                                             gboolean        extend_selection)
{
    GtkTreePath *path;
    Column *column, *prev;
    int index_;

    path = ensure_cursor (view);

    column = find_column_by_path (view, path, &index_);
    g_return_if_fail (column != NULL);

    prev = column_prev (view, column);

    if (index_ || !prev)
        move_cursor_to_entry (view, column, 0,
                              extend_selection);
    else
        move_cursor_to_entry (view, prev, 0,
                              extend_selection);

    gtk_tree_path_free (path);
}


static void     move_cursor_page_down       (MooIconView    *view,
                                             gboolean        extend_selection)
{
    GtkTreePath *path;
    Column *column, *next;
    int index_;

    path = ensure_cursor (view);

    column = find_column_by_path (view, path, &index_);
    g_return_if_fail (column != NULL);

    next = column_next (view, column);

    if (index_ < num_entries (column) - 1 || !next)
        move_cursor_to_entry (view, column,
                              num_entries (column) - 1,
                              extend_selection);
    else
        move_cursor_to_entry (view, next,
                              num_entries (next) - 1,
                              extend_selection);

    gtk_tree_path_free (path);
}


static void     move_cursor_home            (MooIconView    *view,
                                             gboolean        extend_selection)
{
    Column *column;
    column = view->priv->layout->columns->data;
    move_cursor_to_entry (view, column, 0,
                          extend_selection);
}


static void     move_cursor_end             (MooIconView    *view,
                                             gboolean        extend_selection)
{
    Column *column;
    column = g_slist_last (view->priv->layout->columns)->data;
    move_cursor_to_entry (view, column,
                          num_entries (column) - 1,
                          extend_selection);
}


static void     moo_icon_view_scroll_to     (MooIconView    *view,
                                             int             offset)
{
    g_return_if_fail (GTK_WIDGET_REALIZED (view));

    offset = clamp_offset (view, offset);

    if (offset == view->priv->xoffset)
        return;

    gdk_window_scroll (GTK_WIDGET(view)->window, view->priv->xoffset - offset, 0);
    view->priv->xoffset = offset;

    moo_icon_view_update_adjustment (view);
}


/* this is what GtkRange does */
static double get_wheel_delta (MooIconView  *view)
{
    GtkAdjustment *adj = view->priv->adjustment;

#if 1
    return pow (adj->page_size, 2.0 / 3.0);
#else
    return adj->step_increment * 2;
#endif
}


static gboolean moo_icon_view_scroll_event  (GtkWidget      *widget,
                                             GdkEventScroll *event)
{
    MooIconView *view = MOO_ICON_VIEW (widget);
    int offset = view->priv->xoffset;

    switch (event->direction)
    {
        case GDK_SCROLL_UP:
        case GDK_SCROLL_LEFT:
            offset -= get_wheel_delta (view);
            break;

        case GDK_SCROLL_DOWN:
        case GDK_SCROLL_RIGHT:
            offset += get_wheel_delta (view);
            break;
    }

    moo_icon_view_scroll_to (view, offset);
    return TRUE;
}


static int  clamp_offset    (MooIconView    *view,
                             int             offset)
{
    int layout_width = view->priv->layout->width;
    int width = GTK_WIDGET(view)->allocation.width;

    if (layout_width <= width)
        return 0;
    else
        return CLAMP (offset, 0, layout_width - width - 1);
}


/********************************************************************/
/* moo_icon_view_get_path_at_pos
 */

static Column   *get_column_at_x    (MooIconView    *view,
                                     int             x)
{
    GSList *link;

    if (x < 0 || x >= view->priv->layout->width)
        return NULL;

    for (link = view->priv->layout->columns; link != NULL; link = link->next)
    {
        Column *column = link->data;
        if (column->offset <= x && x < column->offset + column->width)
            return column;
    }

    return NULL;
}


static gboolean column_get_path_at_xy   (MooIconView        *view,
                                         Column             *column,
                                         int                 x,
                                         int                 y,
                                         GtkTreePath       **pathp,
                                         MooIconViewCell    *cell,
                                         int                *cell_x,
                                         int                *cell_y)
{
    int index_;
    int pixbuf_width;
    GtkTreeIter iter;
    GtkTreePath *path;

    g_return_val_if_fail (column != NULL, FALSE);
    g_return_val_if_fail (x < column->width, FALSE);

    index_ = y / view->priv->layout->row_height;

    if (index_ < 0 || index_ >= num_entries (column))
        return FALSE;

    path = gtk_tree_path_new_from_indices (index_ + path_get_index (column->first), -1);

    if (!gtk_tree_model_get_iter (view->priv->model, &iter, path))
    {
        gtk_tree_path_free (path);
        g_return_val_if_reached (FALSE);
    }

    pixbuf_width = view->priv->layout->pixbuf_width;

    if (x < pixbuf_width)
    {
        if (pathp)
            *pathp = path;
        else
            gtk_tree_path_free (path);
        if (cell)
            *cell = MOO_ICON_VIEW_CELL_PIXBUF;
        if (cell_x)
            *cell_x = x;
        if (cell_y)
            *cell_y = y - index_ * view->priv->layout->row_height;
        return TRUE;
    }
    else if (view->priv->layout->text_height > 0)
    {
        int text_width;

        view->priv->text.func (view, view->priv->text.cell,
                               view->priv->model, &iter,
                               view->priv->text.func_data);
        gtk_cell_renderer_get_size (view->priv->text.cell,
                                    GTK_WIDGET (view), NULL,
                                    NULL, NULL, &text_width, NULL);

        if (x < text_width + pixbuf_width)
        {
            if (pathp)
                *pathp = path;
            else
                gtk_tree_path_free (path);

            if (cell)
                *cell = MOO_ICON_VIEW_CELL_TEXT;

            if (cell_x)
                *cell_x = x - pixbuf_width;

            if (cell_y)
                *cell_y = y - index_ * view->priv->layout->row_height;

            return TRUE;
        }
    }

    gtk_tree_path_free (path);
    return FALSE;
}


gboolean    moo_icon_view_get_path_at_pos       (MooIconView        *view,
                                                 int                 x,
                                                 int                 y,
                                                 GtkTreePath       **path,
                                                 MooIconViewCell    *cell,
                                                 int                *cell_x,
                                                 int                *cell_y)
{
    Column *column;

    g_return_val_if_fail (MOO_IS_ICON_VIEW (view), FALSE);

    column = get_column_at_x (view, x + view->priv->xoffset);

    if (!column)
        return FALSE;
    else
        return column_get_path_at_xy (view, column,
                                      x + view->priv->xoffset - column->offset,
                                      y, path, cell, cell_x, cell_y);
}


/**************************************************************************/
/* Selection and cursor
 */

struct _Selection {
    GtkSelectionMode mode;
    GSList *selected;   /* GtkTreeRowReference* */
};


static void
init_selection (MooIconView *view)
{
    view->priv->selection = g_new (Selection, 1);
    view->priv->selection->mode = GTK_SELECTION_SINGLE;
    view->priv->selection->selected = NULL;
}


static void
free_selection (MooIconView *view)
{
    g_slist_foreach (view->priv->selection->selected,
                     (GFunc) gtk_tree_row_reference_free, NULL);
    g_slist_free (view->priv->selection->selected);
    g_free (view->priv->selection);
    view->priv->selection = NULL;
}


static void
selection_changed (MooIconView *view)
{
    g_signal_emit (view, signals[SELECTION_CHANGED], 0);
}

static void
cursor_moved (MooIconView *view)
{
    g_signal_emit (view, signals[CURSOR_MOVED], 0);
}


static void
selection_clear (MooIconView *view)
{
    g_slist_foreach (view->priv->selection->selected,
                     (GFunc) gtk_tree_row_reference_free, NULL);
    g_slist_free (view->priv->selection->selected);
    g_free (view->priv->selection);
    view->priv->selection = NULL;
    selection_changed (view);
}


static void
selection_row_deleted (MooIconView *view)
{
    GSList *link;
    Selection *sel = view->priv->selection;

    for (link = sel->selected; link != NULL; link = link->next)
    {
        if (!gtk_tree_row_reference_valid (link->data))
        {
            gtk_tree_row_reference_free (link->data);
            sel->selected = g_slist_delete_link (sel->selected, link);
            selection_changed (view);
            return;
        }
    }
}


void
moo_icon_view_set_selection_mode (MooIconView        *view,
                                  GtkSelectionMode    mode)
{
    Selection *selection;
    GtkTreePath *path = NULL;
    gboolean select_cursor = FALSE;

    g_return_if_fail (MOO_IS_ICON_VIEW (view));

    selection = view->priv->selection;

    if (selection->mode == mode)
        return;

    selection->mode = mode;

    if (!view->priv->model || model_empty (view->priv->model))
        return;

    if (mode == GTK_SELECTION_MULTIPLE)
        return;

    switch (mode)
    {
        case GTK_SELECTION_NONE:
            moo_icon_view_unselect_all (view);
            return;

        case GTK_SELECTION_BROWSE:
        case GTK_SELECTION_SINGLE:
            break;

        default:
            g_return_if_reached ();
    }

    if (mode == GTK_SELECTION_BROWSE)
        select_cursor = TRUE;

    if (!select_cursor &&
         (path = moo_icon_view_get_cursor (view)))
    {
        if (moo_icon_view_path_is_selected (view, path))
            select_cursor = TRUE;
    }

    moo_icon_view_unselect_all (view);

    if (select_cursor)
    {
        if (!path)
            path = moo_icon_view_get_cursor (view);

        if (!path)
        {
            path = gtk_tree_path_new_from_indices (0, -1);
            moo_icon_view_set_cursor (view, path, FALSE);
        }
        else
        {
            moo_icon_view_select_path (view, path);
        }
    }

    gtk_tree_path_free (path);
    selection_changed (view);
}


GtkTreePath *
moo_icon_view_get_selected_path (MooIconView *view)
{
    Selection *selection;

    g_return_val_if_fail (MOO_IS_ICON_VIEW (view), NULL);

    selection = view->priv->selection;

    if (!selection->selected)
        return NULL;

    return gtk_tree_row_reference_get_path (view->priv->selection->selected->data);
}


gboolean
moo_icon_view_get_selected (MooIconView  *view,
                            GtkTreeIter  *iter)
{
    GtkTreePath *path = NULL;
    gboolean result;

    g_return_val_if_fail (MOO_IS_ICON_VIEW (view), FALSE);

    path = moo_icon_view_get_selected_path (view);

    if (path)
    {
        result = gtk_tree_model_get_iter (view->priv->model, iter, path);
        gtk_tree_path_free (path);
        return result;
    }
    else
    {
        return FALSE;
    }
}


void
moo_icon_view_selected_foreach (MooIconView *view,
                                GtkTreeSelectionForeachFunc func,
                                gpointer data)
{
    Selection *selection;
    GSList *link;
    GtkTreePath *path;
    GtkTreeIter iter;

    g_return_if_fail (MOO_IS_ICON_VIEW (view));
    g_return_if_fail (func != NULL);

    selection = view->priv->selection;

    for (link = selection->selected; link != NULL; link = link->next)
    {
        g_assert (gtk_tree_row_reference_valid (link->data));
        path = gtk_tree_row_reference_get_path (link->data);
        gtk_tree_model_get_iter (view->priv->model, &iter, path);
        func (view->priv->model, path, &iter, data);
        gtk_tree_path_free (path);
    }
}


static void
prepend_path (G_GNUC_UNUSED GtkTreeModel *model,
              GtkTreePath   *path,
              G_GNUC_UNUSED GtkTreeIter *iter,
              GList        **list)
{
    *list = g_list_prepend (*list, gtk_tree_path_copy (path));
}

GList*
moo_icon_view_get_selected_rows (MooIconView *view)
{
    GList *list = NULL;
    moo_icon_view_selected_foreach (view,
                                    (GtkTreeSelectionForeachFunc) prepend_path,
                                    &list);
    return g_list_reverse (list);
}


int
moo_icon_view_count_selected_rows (MooIconView *view)
{
    g_return_val_if_fail (MOO_IS_ICON_VIEW (view), 0);
    return g_slist_length (view->priv->selection->selected);
}


static int
row_reference_compare (GtkTreeRowReference *ref1,
                       GtkTreeRowReference *ref2)
{
    int result;
    GtkTreePath *path1, *path2;
    path1 = gtk_tree_row_reference_get_path (ref1);
    path2 = gtk_tree_row_reference_get_path (ref2);
    result = gtk_tree_path_compare (path1, path2);
    gtk_tree_path_free (path1);
    gtk_tree_path_free (path2);
    return result;
}


void
moo_icon_view_select_path (MooIconView *view,
                           GtkTreePath *path)
{
    Selection *selection;
    GtkTreeRowReference *ref;

    g_return_if_fail (MOO_IS_ICON_VIEW (view));
    g_return_if_fail (path != NULL);
    g_return_if_fail (gtk_tree_path_get_depth (path) == 1);

    selection = view->priv->selection;
    g_return_if_fail (selection->mode != GTK_SELECTION_NONE);

    ref = gtk_tree_row_reference_new (view->priv->model, path);
    g_return_if_fail (ref != NULL);

    if (selection->mode == GTK_SELECTION_SINGLE ||
        selection->mode == GTK_SELECTION_BROWSE)
    {
        moo_icon_view_unselect_all (view);
        g_assert (selection->selected == NULL);
        selection->selected = g_slist_prepend (selection->selected, ref);
        invalidate_path_rectangle (view, path);
        selection_changed (view);
        return;
    }

    if (!g_slist_find_custom (view->priv->selection->selected, ref,
                              (GCompareFunc) row_reference_compare))
    {
        selection->selected = g_slist_prepend (selection->selected, ref);
        invalidate_path_rectangle (view, path);
        selection_changed (view);
        return;
    }
}


void
moo_icon_view_select_range (MooIconView  *view,
                            GtkTreePath  *start,
                            GtkTreePath  *end)
{
    Selection *selection;
    GtkTreePath *path;
    int start_index, end_index, i;

    g_return_if_fail (MOO_IS_ICON_VIEW (view));
    g_return_if_fail (start != NULL && end != NULL);
    g_return_if_fail (gtk_tree_path_get_depth (start) == 1);
    g_return_if_fail (gtk_tree_path_get_depth (end) == 1);

    selection = view->priv->selection;
    g_return_if_fail (selection->mode != GTK_SELECTION_NONE);

    if (selection->mode != GTK_SELECTION_MULTIPLE)
    {
        g_return_if_fail (!gtk_tree_path_compare (start, end));
        return moo_icon_view_select_path (view, start);
    }

    start_index = gtk_tree_path_get_indices(start)[0];
    end_index = gtk_tree_path_get_indices(end)[0];

    if (start_index > end_index)
    {
        int tmp = start_index;
        start_index = end_index;
        end_index = tmp;
    }

    path = gtk_tree_path_new_from_indices (start_index, -1);

    for (i = start_index; i <= end_index; ++i, gtk_tree_path_next (path))
        moo_icon_view_select_path (view, path);

    gtk_tree_path_free (path);
}


void
moo_icon_view_unselect_path (MooIconView *view,
                             GtkTreePath *path)
{
    Selection *selection;
    GSList *link;
    GtkTreePath *selected;

    g_return_if_fail (MOO_IS_ICON_VIEW (view));
    g_return_if_fail (path != NULL);

    if (gtk_tree_path_get_depth (path) != 1)
        return;

    selection = view->priv->selection;

    for (link = selection->selected; link != NULL; link = link->next)
    {
        g_assert (gtk_tree_row_reference_valid (link->data));
        selected = gtk_tree_row_reference_get_path (link->data);

        if (!gtk_tree_path_compare (selected, path))
        {
            gtk_tree_row_reference_free (link->data);
            gtk_tree_path_free (selected);
            selection->selected =
                    g_slist_delete_link (selection->selected, link);
            invalidate_path_rectangle (view, path);
            selection_changed (view);
            return;
        }

        gtk_tree_path_free (selected);
    }
}


gboolean
moo_icon_view_path_is_selected (MooIconView *view,
                                GtkTreePath *path)
{
    Selection *selection;
    GSList *link;
    GtkTreePath *selected;

    g_return_val_if_fail (MOO_IS_ICON_VIEW (view), FALSE);
    g_return_val_if_fail (path != NULL, FALSE);

    if (gtk_tree_path_get_depth (path) != 1)
        return FALSE;

    selection = view->priv->selection;

    for (link = selection->selected; link != NULL; link = link->next)
    {
        g_assert (gtk_tree_row_reference_valid (link->data));
        selected = gtk_tree_row_reference_get_path (link->data);

        if (!gtk_tree_path_compare (selected, path))
        {
            gtk_tree_path_free (selected);
            return TRUE;
        }

        gtk_tree_path_free (selected);
    }

    return FALSE;
}


void
moo_icon_view_select_all (MooIconView *view)
{
    GtkTreeIter iter;
    Selection *selection;

    g_return_if_fail (MOO_IS_ICON_VIEW (view));

    if (!view->priv->model || model_empty (view->priv->model))
        return;

    selection = view->priv->selection;
    g_slist_foreach (selection->selected,
                     (GFunc) gtk_tree_row_reference_free, NULL);
    g_slist_free (selection->selected);
    selection->selected = NULL;

    gtk_tree_model_get_iter_first (view->priv->model, &iter);

    do {
        GtkTreePath *path;
        GtkTreeRowReference *ref;
        path = gtk_tree_model_get_path (view->priv->model, &iter);
        g_return_if_fail (path != NULL);
        ref = gtk_tree_row_reference_new (view->priv->model, path);
        selection->selected = g_slist_prepend (selection->selected, ref);
        gtk_tree_path_free (path);
    }
    while (gtk_tree_model_iter_next (view->priv->model, &iter));

    selection->selected = g_slist_reverse (selection->selected);
    gtk_widget_queue_draw (GTK_WIDGET (view));
    selection_changed (view);
}


void
moo_icon_view_unselect_all (MooIconView *view)
{
    Selection *selection;

    g_return_if_fail (MOO_IS_ICON_VIEW (view));

    selection = view->priv->selection;

    if (!selection->selected)
        return;

    g_return_if_fail (selection->mode != GTK_SELECTION_BROWSE);

    g_slist_foreach (selection->selected,
                     (GFunc) gtk_tree_row_reference_free, NULL);
    g_slist_free (selection->selected);
    selection->selected = NULL;

    gtk_widget_queue_draw (GTK_WIDGET (view));
    selection_changed (view);
}


void
moo_icon_view_scroll_to_cell (MooIconView *view,
                              GtkTreePath *path)
{
    Column *column;
    int xoffset, new_offset;
    GtkWidget *widget;

    g_return_if_fail (MOO_IS_ICON_VIEW (view));
    g_return_if_fail (path != NULL);
    g_return_if_fail (gtk_tree_path_get_depth (path) == 1);
    g_return_if_fail (view->priv->model != NULL);
    g_return_if_fail (!model_empty (view->priv->model));

    if (!GTK_WIDGET_REALIZED (view) || view->priv->update_idle)
    {
        if (view->priv->scroll_to)
            gtk_tree_row_reference_free (view->priv->scroll_to);
        view->priv->scroll_to = gtk_tree_row_reference_new (view->priv->model, path);
        return;
    }

    column = find_column_by_path (view, path, NULL);
    g_return_if_fail (column != NULL);

    xoffset = view->priv->xoffset;
    new_offset = xoffset;
    widget = GTK_WIDGET(view);

    if (widget->allocation.width <= column->width)
        new_offset = column->offset;
    else if (column->offset < xoffset)
        new_offset = column->offset;
    else if (column->offset + column->width > xoffset + widget->allocation.width)
        new_offset = column->offset + column->width - widget->allocation.width;

    moo_icon_view_scroll_to (view, new_offset);
}


void
moo_icon_view_set_cursor (MooIconView *view,
                          GtkTreePath *path,
                          gboolean     start_editing)
{
    GtkTreeRowReference *ref;
    GtkTreePath *old;

    g_return_if_fail (MOO_IS_ICON_VIEW (view));
    g_return_if_fail (path != NULL);

    ref = gtk_tree_row_reference_new (view->priv->model, path);
    g_return_if_fail (ref != NULL);

    old = moo_icon_view_get_cursor (view);

    if (old)
    {
        invalidate_path_rectangle (view, old);
        gtk_tree_path_free (old);
        gtk_tree_row_reference_free (view->priv->cursor);
    }

    view->priv->cursor = ref;
    moo_icon_view_select_path (view, path);
    cursor_moved (view);

    if (start_editing)
    {
        moo_icon_view_scroll_to_cell (view, path);
        /* TODO */
        g_warning ("%s: implement me", G_STRLOC);
    }
}


GtkTreePath *
moo_icon_view_get_cursor (MooIconView *view)
{
    g_return_val_if_fail (MOO_IS_ICON_VIEW (view), NULL);

    if (view->priv->cursor)
        return gtk_tree_row_reference_get_path (view->priv->cursor);
    else
        return NULL;
}


static void
cursor_row_deleted (MooIconView *view)
{
    if (view->priv->cursor)
    {
        if (!gtk_tree_row_reference_valid (view->priv->cursor))
        {
            gtk_tree_row_reference_free (view->priv->cursor);
            view->priv->cursor = NULL;
            cursor_moved (view);
        }
    }
}


void
moo_icon_view_row_activated (MooIconView *view,
                             GtkTreePath *path)
{
    g_return_if_fail (MOO_IS_ICON_VIEW (view));
    g_return_if_fail (path != NULL);
    g_signal_emit (view, signals[ROW_ACTIVATED], 0, path);
}


static void
activate_item_at_cursor (MooIconView *view)
{
    GtkTreePath *path;

    if (!view->priv->cursor)
        return;

    path = gtk_tree_row_reference_get_path (view->priv->cursor);
    g_return_if_fail (path != NULL);
    moo_icon_view_row_activated (view, path);
    gtk_tree_path_free (path);
}


/********************************************************************/
/* Drag'n'drop
 */

void
moo_icon_view_enable_drag_source (MooIconView        *view,
                                  GdkModifierType     start_button_mask,
                                  GtkTargetEntry     *targets,
                                  gint                n_targets,
                                  GdkDragAction       actions)
{
    DndInfo *info;

    g_return_if_fail (MOO_IS_ICON_VIEW (view));

    gtk_drag_source_set (GTK_WIDGET (view), 0, targets, n_targets, actions);

    info = view->priv->dnd_info;

    if (info->source_targets)
        gtk_target_list_unref (info->source_targets);

    info->start_button_mask = start_button_mask;
    info->source_targets = gtk_target_list_new (targets, n_targets);
    info->source_actions = actions;
    info->source_enabled = TRUE;
}


GtkTargetList*
moo_icon_view_get_source_targets (MooIconView *view)
{
    g_return_val_if_fail (MOO_IS_ICON_VIEW (view), NULL);
    return view->priv->dnd_info->source_targets;
}


void
moo_icon_view_disable_drag_source (MooIconView *view)
{
    DndInfo *info;

    g_return_if_fail (MOO_IS_ICON_VIEW (view));

    info = view->priv->dnd_info;

    if (info->source_enabled)
    {
        gtk_drag_source_unset (GTK_WIDGET (view));
        gtk_target_list_unref (info->source_targets);
        info->source_targets = NULL;
    }
}


static gboolean
moo_icon_view_maybe_drag (MooIconView    *view,
                          GdkEventMotion *event)
{
    DndInfo *info;
    GtkTreePath *path = NULL;
    gboolean result = FALSE;
    int button;
    GdkDragContext *context;
//     GdkPixmap *pixmap;

    info = view->priv->dnd_info;

    if (!view->priv->button_pressed)
        goto out;

    if (!info->source_enabled)
        goto out;

    if (!gtk_drag_check_threshold (GTK_WIDGET (view),
                                   view->priv->button_press_x,
                                   view->priv->button_press_y,
                                   event->x, event->y))
    {
        goto out;
    }

    button = view->priv->button_pressed;
    view->priv->button_pressed = 0;

    moo_icon_view_get_path_at_pos (view,
                                   view->priv->button_press_x,
                                   view->priv->button_press_y,
                                   &path, NULL, NULL, NULL);

    if (!path)
        goto out;

    if (!(GDK_BUTTON1_MASK << (button - 1) & info->start_button_mask))
        goto out;

    /* drag it! */
    result = TRUE;
    context = gtk_drag_begin (GTK_WIDGET (view),
                              info->source_targets,
                              info->source_actions,
                              button,
                              (GdkEvent*) event);

//     pixmap = moo_icon_view_create_row_drag_icon (view, path);
//     gtk_drag_set_icon_pixmap (context,
//                               gdk_drawable_get_colormap (pixmap),
//                               pixmap,
//                               NULL,
//                               /* the + 1 is for the black border in the icon ? */
//                               view->priv->button_press_x + 1,
//                               1);

out:
    if (path)
        gtk_tree_path_free (path);

    return result;
}


static void
moo_icon_view_drag_begin (G_GNUC_UNUSED GtkWidget      *widget,
                          G_GNUC_UNUSED GdkDragContext *context)
{
}


static void
moo_icon_view_drag_end (GtkWidget      *widget,
                        G_GNUC_UNUSED GdkDragContext *context)
{
    MooIconView *view = MOO_ICON_VIEW (widget);
    view->priv->button_pressed = 0;
}


void
moo_icon_view_enable_drag_dest (MooIconView        *view,
                                GtkTargetEntry     *targets,
                                gint                n_targets,
                                GdkDragAction       actions)
{
    DndInfo *info;

    g_return_if_fail (MOO_IS_ICON_VIEW (view));

    gtk_drag_dest_set (GTK_WIDGET (view), 0, targets, n_targets, actions);

    info = view->priv->dnd_info;

    if (info->dest_targets)
        gtk_target_list_unref (info->dest_targets);

    info->dest_targets = gtk_target_list_new (targets, n_targets);
    info->dest_actions = actions;
    info->dest_enabled = TRUE;
}


GtkTargetList *
moo_icon_view_get_dest_targets (MooIconView *view)
{
    g_return_val_if_fail (MOO_IS_ICON_VIEW (view), NULL);
    return view->priv->dnd_info->dest_targets;
}


void
moo_icon_view_disable_drag_dest (MooIconView *view)
{
    DndInfo *info;

    g_return_if_fail (MOO_IS_ICON_VIEW (view));

    info = view->priv->dnd_info;

    if (info->dest_enabled)
    {
        gtk_drag_dest_unset (GTK_WIDGET (view));
        gtk_target_list_unref (info->dest_targets);
        info->dest_targets = NULL;
    }
}


static void
moo_icon_drag_data_received (G_GNUC_UNUSED GtkWidget      *widget,
                             G_GNUC_UNUSED GdkDragContext *context,
                             G_GNUC_UNUSED int             x,
                             G_GNUC_UNUSED int             y,
                             G_GNUC_UNUSED GtkSelectionData *data,
                             G_GNUC_UNUSED guint           info,
                             G_GNUC_UNUSED guint           time)
{
    g_print ("drag-data-received\n");
}


static gboolean
moo_icon_view_drag_drop (G_GNUC_UNUSED GtkWidget      *widget,
                         G_GNUC_UNUSED GdkDragContext *context,
                         G_GNUC_UNUSED int             x,
                         G_GNUC_UNUSED int             y,
                         G_GNUC_UNUSED guint           time)
{
    g_print ("drag-drop\n");
    return FALSE;
}


static void
moo_icon_view_drag_leave (G_GNUC_UNUSED GtkWidget      *widget,
                          G_GNUC_UNUSED GdkDragContext *context,
                          G_GNUC_UNUSED guint           time)
{
    DndInfo *info;
    MooIconView *view = MOO_ICON_VIEW (widget);

    info = view->priv->dnd_info;

    if (info->drag_motion_context)
        g_object_unref (info->drag_motion_context);
    info->drag_motion_context = NULL;

    if (!info->drag_dest_inside)
        g_warning ("drag_leave: oops\n");

    info->drag_dest_inside = FALSE;
    drag_scroll_stop (view);

    g_print ("drag-leave\n");
}


static void
drag_scroll_stop (MooIconView *view)
{
    DndInfo *info = view->priv->dnd_info;

    if (info->scroll_timeout)
    {
        g_source_remove (info->scroll_timeout);
        info->scroll_timeout = 0;
        g_print ("drag_scroll_stop\n");
    }
}


#define DRAG_SCROLL_MARGIN 0.1
#define DRAG_SCROLL_TIMEOUT 100

static gboolean
drag_scroll_timeout (MooIconView *view)
{
    GtkWidget *widget = GTK_WIDGET (view);
    GtkAllocation *alc = &widget->allocation;
    GtkWidget *toplevel;
    int x, y, new_offset;
    int delta, dist;
    double ratio, margin;
    GdkModifierType mask;
    GdkEvent *event;
    DndInfo *info = view->priv->dnd_info;

    gdk_window_get_pointer (widget->window, &x, &y, &mask);

    if (x < 0 || x >= alc->width || y < 0 || y >= alc->height)
        goto out;

    if (x < widget->allocation.width * DRAG_SCROLL_MARGIN)
    {
        dist = x;
        delta = -1;
    }
    else if (x > widget->allocation.width * (1 - DRAG_SCROLL_MARGIN))
    {
        dist = widget->allocation.width - 1 - x;
        delta = 1;
    }
    else
    {
        goto out;
    }

    margin = widget->allocation.width * DRAG_SCROLL_MARGIN;
    margin = MAX (margin, 2);
    dist = CLAMP (dist, 1, margin - 1);
    ratio = (margin - dist) / margin;
    delta *= 15 * (1 + 3 * ratio);

    new_offset = clamp_offset (view, view->priv->xoffset + delta);

    if (new_offset == view->priv->xoffset)
        goto out;

    moo_icon_view_scroll_to (view, new_offset);

    toplevel = gtk_widget_get_toplevel (widget);

    if (!GTK_WIDGET_TOPLEVEL (toplevel))
    {
        g_critical ("%s: oops", G_STRLOC);
        goto out;
    }

    event = gdk_event_new (GDK_DRAG_MOTION);

    event->dnd.window = g_object_ref (toplevel->window);
    event->dnd.send_event = TRUE;
    event->dnd.context = g_object_ref (info->drag_motion_context);

    gdk_window_get_position (toplevel->window, &x, &y);
    event->dnd.x_root = x;
    event->dnd.y_root = y;
    gdk_window_get_pointer (toplevel->window, &x, &y, &mask);
    event->dnd.x_root += x;
    event->dnd.y_root += y;

    gtk_main_do_event (event);
    gdk_event_free (event);

    return TRUE;

out:
    drag_scroll_stop (view);
    return FALSE;
}


static void
drag_scroll_check (MooIconView *view,
                   int          x,
                   int          y)
{
    GtkAllocation *alc = &GTK_WIDGET(view)->allocation;

    if (x >= 0 && x < alc->width && y >= 0 && y < alc->height &&
        (x < alc->width * DRAG_SCROLL_MARGIN ||
         x > alc->width * (1 - DRAG_SCROLL_MARGIN)))
    {
        DndInfo *info = view->priv->dnd_info;

        if (!info->scroll_timeout)
            info->scroll_timeout =
                    g_timeout_add (DRAG_SCROLL_TIMEOUT,
                                   (GSourceFunc) drag_scroll_timeout,
                                   view);
    }
    else
    {
        drag_scroll_stop (view);
    }
}


static gboolean
moo_icon_view_drag_motion (GtkWidget      *widget,
                           GdkDragContext *context,
                           int             x,
                           int             y,
                           G_GNUC_UNUSED guint time)
{
    DndInfo *info;
    MooIconView *view = MOO_ICON_VIEW (widget);

    info = view->priv->dnd_info;

    if (info->drag_motion_context != context)
    {
        if (info->drag_motion_context)
        {
            g_critical ("%s: oops", G_STRLOC);
            g_object_unref (info->drag_motion_context);
        }

        info->drag_motion_context = g_object_ref (context);
    }

    if (!info->drag_dest_inside)
    {
        info->drag_dest_inside = TRUE;
        g_print ("drag-enter\n");
    }
    else
    {
        drag_scroll_check (view, x, y);
    }

    return FALSE;
}


void
moo_icon_view_set_drag_dest_row (MooIconView *view,
                                 GtkTreePath *path)
{
    GtkTreeRowReference *ref = NULL;

    g_return_if_fail (MOO_IS_ICON_VIEW (view));

    if (path)
        ref = gtk_tree_row_reference_new (view->priv->model, path);

    if (view->priv->drop_dest)
    {
        GtkTreePath *old_path;

        old_path = gtk_tree_row_reference_get_path (view->priv->drop_dest);

        if (old_path)
        {
            invalidate_path_rectangle (view, old_path);
            gtk_tree_path_free (old_path);
        }

        gtk_tree_row_reference_free (view->priv->drop_dest);
        view->priv->drop_dest = NULL;
    }

    if (ref)
    {
        view->priv->drop_dest = ref;
        invalidate_path_rectangle (view, path);
    }
}


GtkTreePath *
moo_icon_view_get_drop_dest_row (MooIconView *view)
{
    g_return_val_if_fail (MOO_IS_ICON_VIEW (view), NULL);

    if (view->priv->drop_dest)
        return gtk_tree_row_reference_get_path (view->priv->drop_dest);
    else
        return NULL;
}
