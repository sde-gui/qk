/*-*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-*/
/*
 *   moobigpaned.c
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

#include "moobigpaned.h"
#include MOO_MARSHALS_H


static void     moo_big_paned_finalize      (GObject        *object);
static void     moo_big_paned_set_property  (GObject        *object,
                                             guint           prop_id,
                                             const GValue   *value,
                                             GParamSpec     *pspec);
static void     moo_big_paned_get_property  (GObject        *object,
                                             guint           prop_id,
                                             GValue         *value,
                                             GParamSpec     *pspec);

static gboolean moo_big_paned_expose        (GtkWidget      *widget,
                                             GdkEventExpose *event,
                                             MooBigPaned    *paned);

static void     child_open_pane             (GtkWidget      *child,
                                             guint           index,
                                             MooBigPaned    *paned);
static void     child_hide_pane             (GtkWidget      *child,
                                             MooBigPaned    *paned);
static gboolean check_children_order        (MooBigPaned    *paned);

static void     handle_drag_start           (MooPaned       *child,
                                             GtkWidget      *pane_widget,
                                             MooBigPaned    *paned);
static void     handle_drag_motion          (MooPaned       *child,
                                             GtkWidget      *pane_widget,
                                             MooBigPaned    *paned);
static void     handle_drag_end             (MooPaned       *child,
                                             GtkWidget      *pane_widget,
                                             MooBigPaned    *paned);


/* MOO_TYPE_BIG_PANED */
G_DEFINE_TYPE (MooBigPaned, moo_big_paned, GTK_TYPE_FRAME)

enum {
    PROP_0,
    PROP_PANE_ORDER,
    PROP_ENABLE_HANDLE_DRAG,
    PROP_ENABLE_DETACHING,
    PROP_HANDLE_CURSOR_TYPE
};

enum {
    OPEN_PANE,
    HIDE_PANE,
    NUM_SIGNALS
};

static guint signals[NUM_SIGNALS];

static void moo_big_paned_class_init (MooBigPanedClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = moo_big_paned_finalize;
    gobject_class->set_property = moo_big_paned_set_property;
    gobject_class->get_property = moo_big_paned_get_property;

    g_object_class_install_property (gobject_class,
                                     PROP_PANE_ORDER,
                                     g_param_spec_pointer ("pane-order",
                                             "pane-order",
                                             "pane-order",
                                             G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_ENABLE_HANDLE_DRAG,
                                     g_param_spec_boolean ("enable-handle-drag",
                                             "enable-handle-drag",
                                             "enable-handle-drag",
                                             TRUE,
                                             G_PARAM_CONSTRUCT | G_PARAM_WRITABLE));

    g_object_class_install_property (gobject_class,
                                     PROP_ENABLE_DETACHING,
                                     g_param_spec_boolean ("enable-detaching",
                                             "enable-detaching",
                                             "enable-detaching",
                                             FALSE,
                                             G_PARAM_CONSTRUCT | G_PARAM_WRITABLE));

    g_object_class_install_property (gobject_class,
                                     PROP_HANDLE_CURSOR_TYPE,
                                     g_param_spec_enum ("handle-cursor-type",
                                             "handle-cursor-type",
                                             "handle-cursor-type",
                                             GDK_TYPE_CURSOR_TYPE,
                                             GDK_HAND2,
                                             G_PARAM_CONSTRUCT | G_PARAM_READWRITE));

    signals[OPEN_PANE] =
            g_signal_new ("open-pane",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_FIRST,
                          G_STRUCT_OFFSET (MooBigPanedClass, open_pane),
                          NULL, NULL,
                          _moo_marshal_VOID__ENUM_UINT,
                          G_TYPE_NONE, 2,
                          MOO_TYPE_PANE_POSITION, G_TYPE_UINT);

    signals[HIDE_PANE] =
            g_signal_new ("hide-pane",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_FIRST,
                          G_STRUCT_OFFSET (MooBigPanedClass, hide_pane),
                          NULL, NULL,
                          _moo_marshal_VOID__ENUM,
                          G_TYPE_NONE, 1,
                          MOO_TYPE_PANE_POSITION);

}


#define NTH_CHILD(paned,n) paned->paned[paned->order[n]]

static void moo_big_paned_init      (MooBigPaned *paned)
{
    int i;

    paned->drop_pos = -1;

    /* XXX destroy */
    for (i = 0; i < 4; ++i)
    {
        GtkWidget *child;

        paned->paned[i] = child =
                g_object_new (MOO_TYPE_PANED,
                              "pane-position", (MooPanePosition) i,
                              NULL);

        g_object_ref (child);
        gtk_object_sink (GTK_OBJECT (child));
        gtk_widget_show (child);

        g_signal_connect (child, "hide-pane",
                          G_CALLBACK (child_hide_pane),
                          paned);
        g_signal_connect (child, "open-pane",
                          G_CALLBACK (child_open_pane),
                          paned);
        g_signal_connect (child, "handle-drag-start",
                          G_CALLBACK (handle_drag_start),
                          paned);
        g_signal_connect (child, "handle-drag-motion",
                          G_CALLBACK (handle_drag_motion),
                          paned);
        g_signal_connect (child, "handle-drag-end",
                          G_CALLBACK (handle_drag_end),
                          paned);
    }

    paned->order[0] = MOO_PANE_POS_LEFT;
    paned->order[1] = MOO_PANE_POS_RIGHT;
    paned->order[2] = MOO_PANE_POS_TOP;
    paned->order[3] = MOO_PANE_POS_BOTTOM;

    paned->inner = NTH_CHILD (paned, 3);
    paned->outer = NTH_CHILD (paned, 0);

    gtk_container_add (GTK_CONTAINER (paned), NTH_CHILD (paned, 0));

    for (i = 0; i < 3; ++i)
        gtk_container_add (GTK_CONTAINER (NTH_CHILD (paned, i)), NTH_CHILD (paned, i+1));

    g_assert (check_children_order (paned));
}


static gboolean check_children_order        (MooBigPaned    *paned)
{
    int i;

    if (GTK_BIN(paned)->child != NTH_CHILD (paned, 0))
        return FALSE;

    for (i = 0; i < 3; ++i)
        if (GTK_BIN (NTH_CHILD (paned, i))->child != NTH_CHILD (paned, i+1))
                return FALSE;

    return TRUE;
}


void        moo_big_paned_set_pane_order    (MooBigPaned    *paned,
                                             int            *order)
{
    MooPanePosition new_order[4] = {8, 8, 8, 8};
    int i;
    GtkWidget *child;

    g_return_if_fail (MOO_IS_BIG_PANED (paned));
    g_return_if_fail (order != NULL);

    for (i = 0; i < 4; ++i)
    {
        g_return_if_fail (new_order[i] >= 4);
        g_return_if_fail (0 <= order[i] && order[i] < 4);
        new_order[i] = order[i];
    }

    g_return_if_fail (check_children_order (paned));

    for (i = 0; i < 4; ++i)
    {
        if (new_order[i] != paned->order[i])
            break;
    }

    if (i == 4)
        return;

    child = moo_big_paned_get_child (paned);

    if (child)
        g_object_ref (child);

    gtk_container_remove (GTK_CONTAINER (paned), NTH_CHILD (paned, 0));
    for (i = 0; i < 3; ++i)
        gtk_container_remove (GTK_CONTAINER (NTH_CHILD (paned, i)), NTH_CHILD (paned, i+1));
    if (child)
        gtk_container_remove (GTK_CONTAINER (NTH_CHILD (paned, 3)), child);

    for (i = 0; i < 4; ++i)
        paned->order[i] = new_order[i];

    gtk_container_add (GTK_CONTAINER (paned), NTH_CHILD (paned, 0));

    for (i = 0; i < 3; ++i)
        gtk_container_add (GTK_CONTAINER (NTH_CHILD (paned, i)), NTH_CHILD (paned, i+1));

    paned->inner = NTH_CHILD (paned, 3);
    paned->outer = NTH_CHILD (paned, 0);

    if (child)
    {
        gtk_container_add (GTK_CONTAINER (paned->inner), child);
        g_object_unref (child);
    }

    g_assert (check_children_order (paned));
    g_object_notify (G_OBJECT (paned), "pane-order");
}


static void moo_big_paned_finalize  (GObject      *object)
{
    MooBigPaned *paned = MOO_BIG_PANED (object);
    int i;

    for (i = 0; i < 4; ++i)
        g_object_unref (paned->paned[i]);

    /* XXX */
    if (paned->drop_outline)
    {
        gdk_window_set_user_data (paned->drop_outline, NULL);
        gdk_window_destroy (paned->drop_outline);
    }

    G_OBJECT_CLASS (moo_big_paned_parent_class)->finalize (object);
}


GtkWidget  *moo_big_paned_new               (void)
{
    return g_object_new (MOO_TYPE_BIG_PANED, NULL);
}


static void     child_open_pane             (GtkWidget      *child,
                                             guint           index,
                                             MooBigPaned    *paned)
{
    MooPanePosition pos;

    g_object_get (child, "pane-position", &pos, NULL);
    g_return_if_fail (paned->paned[pos] == child);

    g_signal_emit (paned, signals[OPEN_PANE], 0, pos, index);
}


static void     child_hide_pane             (GtkWidget      *child,
                                             MooBigPaned    *paned)
{
    MooPanePosition pos;

    g_object_get (child, "pane-position", &pos, NULL);
    g_return_if_fail (paned->paned[pos] == child);

    g_signal_emit (paned, signals[HIDE_PANE], 0, pos);
}


int         moo_big_paned_add_pane          (MooBigPaned        *paned,
                                             GtkWidget          *pane_widget,
                                             MooPanePosition     position,
                                             const char         *button_label,
                                             const char         *button_stock_id,
                                             int                 index_)
{
    g_return_val_if_fail (MOO_IS_BIG_PANED (paned), -1);
    g_return_val_if_fail (GTK_IS_WIDGET (pane_widget), -1);
    g_return_val_if_fail (position < 4, -1);

    return moo_paned_add_pane (MOO_PANED (paned->paned[position]),
                               pane_widget, button_label, button_stock_id, index_);
}


int         moo_big_paned_insert_pane       (MooBigPaned        *paned,
                                             GtkWidget          *pane_widget,
                                             MooPaneLabel       *pane_label,
                                             MooPanePosition     position,
                                             int                 index_)
{
    g_return_val_if_fail (MOO_IS_BIG_PANED (paned), -1);
    g_return_val_if_fail (GTK_IS_WIDGET (pane_widget), -1);
    g_return_val_if_fail (position < 4, -1);

    return moo_paned_insert_pane (MOO_PANED (paned->paned[position]),
                                  pane_widget, pane_label, index_);
}


void        moo_big_paned_add_child         (MooBigPaned        *paned,
                                             GtkWidget          *child)
{
    g_return_if_fail (MOO_IS_BIG_PANED (paned));
    gtk_container_add (GTK_CONTAINER (paned->inner), child);
}


void        moo_big_paned_remove_child      (MooBigPaned        *paned)
{
    g_return_if_fail (MOO_IS_BIG_PANED (paned));
    gtk_container_remove (GTK_CONTAINER (paned->inner), GTK_BIN(paned->inner)->child);
}


GtkWidget  *moo_big_paned_get_child         (MooBigPaned        *paned)
{
    g_return_val_if_fail (MOO_IS_BIG_PANED (paned), NULL);
    return GTK_BIN(paned->inner)->child;
}


gboolean    moo_big_paned_remove_pane       (MooBigPaned    *paned,
                                             GtkWidget      *widget)
{
    int i;

    g_return_val_if_fail (MOO_IS_BIG_PANED (paned), FALSE);
    g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);

    for (i = 0; i < 4; ++i)
        if (moo_paned_get_pane_num (MOO_PANED (paned->paned[i]), widget) >= 0)
            return moo_paned_remove_pane (MOO_PANED (paned->paned[i]), widget);

    return FALSE;
}


void        moo_big_paned_open_pane         (MooBigPaned    *paned,
                                             GtkWidget      *widget)
{
    int i, num;

    g_return_if_fail (MOO_IS_BIG_PANED (paned));
    g_return_if_fail (GTK_IS_WIDGET (widget));

    for (i = 0; i < 4; ++i)
    {
        num = moo_paned_get_pane_num (MOO_PANED (paned->paned[i]), widget);
        if (num >= 0)
            return moo_paned_open_pane (MOO_PANED (paned->paned[i]), num);
    }
}


void        moo_big_paned_hide_pane         (MooBigPaned    *paned,
                                             GtkWidget      *widget)
{
    int i, num;

    g_return_if_fail (MOO_IS_BIG_PANED (paned));
    g_return_if_fail (GTK_IS_WIDGET (widget));

    for (i = 0; i < 4; ++i)
    {
        num = moo_paned_get_pane_num (MOO_PANED (paned->paned[i]), widget);
        if (num >= 0)
            return moo_paned_hide_pane (MOO_PANED (paned->paned[i]));
    }
}


void        moo_big_paned_present_pane      (MooBigPaned    *paned,
                                             GtkWidget      *widget)
{
    int i, num;

    g_return_if_fail (MOO_IS_BIG_PANED (paned));
    g_return_if_fail (GTK_IS_WIDGET (widget));

    for (i = 0; i < 4; ++i)
    {
        num = moo_paned_get_pane_num (MOO_PANED (paned->paned[i]), widget);
        if (num >= 0)
            return moo_paned_present_pane (MOO_PANED (paned->paned[i]), num);
    }
}


static void     moo_big_paned_set_property  (GObject        *object,
                                             guint           prop_id,
                                             const GValue   *value,
                                             GParamSpec     *pspec)
{
    MooBigPaned *paned = MOO_BIG_PANED (object);
    int i;

    switch (prop_id)
    {
        case PROP_PANE_ORDER:
            moo_big_paned_set_pane_order (paned, g_value_get_pointer (value));
            break;

        case PROP_ENABLE_HANDLE_DRAG:
            for (i = 0; i < 4; ++i)
                g_object_set (paned->paned[i],
                              "enable-handle-drag",
                              g_value_get_boolean (value),
                              NULL);
            break;

        case PROP_ENABLE_DETACHING:
            for (i = 0; i < 4; ++i)
                g_object_set (paned->paned[i],
                              "enable-detaching",
                              g_value_get_boolean (value),
                              NULL);
            break;

        case PROP_HANDLE_CURSOR_TYPE:
            for (i = 0; i < 4; ++i)
                g_object_set (paned->paned[i], "handle-cursor-type",
                              (GdkCursorType) g_value_get_enum (value),
                              NULL);
            g_object_notify (object, "handle-cursor-type");
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void     moo_big_paned_get_property  (GObject        *object,
                                             guint           prop_id,
                                             GValue         *value,
                                             GParamSpec     *pspec)
{
    MooBigPaned *paned = MOO_BIG_PANED (object);
    GdkCursorType cursor_type;

    switch (prop_id)
    {
        case PROP_PANE_ORDER:
            g_value_set_pointer (value, paned->order);
            break;

        case PROP_HANDLE_CURSOR_TYPE:
            g_object_get (paned->paned[0], "handle-cursor-type",
                          &cursor_type, NULL);
            g_value_set_enum (value, cursor_type);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


/*****************************************************************************/
/* rearranging panes
 */

static void         create_drop_outline     (MooBigPaned    *paned);
static int          get_drop_position       (MooBigPaned    *paned,
                                             MooPaned       *child,
                                             int             x,
                                             int             y);
static void         get_drop_area           (MooBigPaned    *paned,
                                             MooPaned       *active_child,
                                             MooPanePosition position,
                                             GdkRectangle   *rect);
static void         invalidate_drop_outline (MooBigPaned    *paned);


static void         handle_drag_start       (G_GNUC_UNUSED MooPaned *child,
                                             G_GNUC_UNUSED GtkWidget *pane_widget,
                                             MooBigPaned    *paned)
{
    g_return_if_fail (GTK_WIDGET_REALIZED (paned->outer));

    g_signal_connect (paned->outer, "expose-event",
                      G_CALLBACK (moo_big_paned_expose), paned);

    paned->drop_pos = -1;
}


static void     handle_drag_motion          (MooPaned       *child,
                                             G_GNUC_UNUSED GtkWidget *pane_widget,
                                             MooBigPaned    *paned)
{
    int pos, x, y;

    g_return_if_fail (GTK_WIDGET_REALIZED (paned->outer));

    gdk_window_get_pointer (paned->outer->window, &x, &y, NULL);
    pos = get_drop_position (paned, child, x, y);

    if (pos == paned->drop_pos)
        return;

    if (paned->drop_pos >= 0)
    {
        invalidate_drop_outline (paned);
        g_assert (paned->drop_outline != NULL);
        gdk_window_set_user_data (paned->drop_outline, NULL);
        gdk_window_destroy (paned->drop_outline);
        paned->drop_outline = NULL;
    }

    paned->drop_pos = pos;

    if (pos >= 0)
    {
        get_drop_area (paned, child, pos, &paned->drop_rect);
        invalidate_drop_outline (paned);
        g_assert (paned->drop_outline == NULL);
        create_drop_outline (paned);
    }
}


static void     handle_drag_end             (MooPaned       *child,
                                             GtkWidget      *pane_widget,
                                             MooBigPaned    *paned)
{
    int pos, x, y;
    MooPanePosition old_pos;
    MooPaneLabel *label;

    g_return_if_fail (GTK_WIDGET_REALIZED (paned->outer));

    gdk_window_get_pointer (paned->outer->window, &x, &y, NULL);

    pos = get_drop_position (paned, child, x, y);

    if (paned->drop_pos >= 0)
    {
        invalidate_drop_outline (paned);
        g_assert (paned->drop_outline != NULL);
        gdk_window_set_user_data (paned->drop_outline, NULL);
        gdk_window_destroy (paned->drop_outline);
        paned->drop_outline = NULL;
    }

    paned->drop_pos = -1;

    g_signal_handlers_disconnect_by_func (paned->outer,
                                          (gpointer) moo_big_paned_expose,
                                          paned);

    if (pos < 0)
        return;

    g_object_get (child, "pane-position", &old_pos, NULL);

    if ((int) old_pos == pos)
        return;

    label = moo_paned_get_label (child, pane_widget);
    g_object_ref (pane_widget);

    moo_paned_remove_pane (child, pane_widget);
    g_return_if_fail (pane_widget->parent == NULL);

    moo_paned_insert_pane (MOO_PANED (paned->paned[pos]), pane_widget, label, -1);
    moo_paned_open_pane (MOO_PANED (paned->paned[pos]),
                         moo_paned_n_panes (MOO_PANED (paned->paned[pos])) - 1);

    moo_pane_label_free (label);
    g_object_unref (pane_widget);
}


static void     get_drop_area               (MooBigPaned    *paned,
                                             MooPaned       *active_child,
                                             MooPanePosition position,
                                             GdkRectangle   *rect)
{
    int width, height, size;
    MooPanePosition active_position;

    width = paned->outer->allocation.width;
    height = paned->outer->allocation.height;

    g_object_get (active_child, "pane-position", &active_position, NULL);
    g_return_if_fail (active_position < 4);

    if (active_position == position)
    {
        size = moo_paned_get_pane_size (active_child) +
                moo_paned_get_button_box_size (active_child);
    }
    else
    {
        switch (position)
        {
            case MOO_PANE_POS_LEFT:
            case MOO_PANE_POS_RIGHT:
                size = width / 3;
                break;
            case MOO_PANE_POS_TOP:
            case MOO_PANE_POS_BOTTOM:
                size = height / 3;
                break;
        }
    }

    switch (position)
    {
        case MOO_PANE_POS_LEFT:
        case MOO_PANE_POS_RIGHT:
            rect->y = 0;
            rect->width = size;
            rect->height = height;
            break;
        case MOO_PANE_POS_TOP:
        case MOO_PANE_POS_BOTTOM:
            rect->x = 0;
            rect->width = width;
            rect->height = size;
            break;
    }

    switch (position)
    {
        case MOO_PANE_POS_LEFT:
            rect->x = 0;
            break;
        case MOO_PANE_POS_RIGHT:
            rect->x = width - size;
            break;
        case MOO_PANE_POS_TOP:
            rect->y = 0;
            break;
        case MOO_PANE_POS_BOTTOM:
            rect->y = height - size;
            break;
    }
}


#define RECT_POINT_IN(rect,x,y) (x < (rect)->x + (rect)->width &&   \
                                 y < (rect)->height + (rect)->y &&  \
                                 x >= (rect)->x && y >= (rect)->y)

static int      get_drop_position           (MooBigPaned    *paned,
                                             MooPaned       *child,
                                             int             x,
                                             int             y)
{
    int width, height, i;
    MooPanePosition position;
    GdkRectangle rect;

    gdk_drawable_get_size (GDK_WINDOW (paned->outer->window),
                           &width, &height);

    if (x < 0 || x >= width || y < 0 || y >= height)
        return -1;

    g_object_get (child, "pane-position", &position, NULL);
    g_return_val_if_fail (position < 4, -1);

    get_drop_area (paned, child, position, &rect);

    if (RECT_POINT_IN (&rect, x, y))
        return position;

    for (i = 0; i < 4; ++i)
    {
        if (paned->order[i] == position)
            continue;

        get_drop_area (paned, child, paned->order[i], &rect);

        if (RECT_POINT_IN (&rect, x, y))
            return paned->order[i];
    }

    return -1;
}


static void     invalidate_drop_outline     (MooBigPaned    *paned)
{
    GdkRectangle line;
    GdkRegion *outline;

    outline = gdk_region_new ();

    line.x = paned->drop_rect.x;
    line.y = paned->drop_rect.y;
    line.width = 1;
    line.height = paned->drop_rect.height;
    gdk_region_union_with_rect (outline, &line);

    line.x = paned->drop_rect.x;
    line.y = paned->drop_rect.y + paned->drop_rect.height;
    line.width = paned->drop_rect.width;
    line.height = 1;
    gdk_region_union_with_rect (outline, &line);

    line.x = paned->drop_rect.x + paned->drop_rect.width;
    line.y = paned->drop_rect.y;
    line.width = 1;
    line.height = paned->drop_rect.height;
    gdk_region_union_with_rect (outline, &line);

    line.x = paned->drop_rect.x;
    line.y = paned->drop_rect.y;
    line.width = paned->drop_rect.width;
    line.height = 1;
    gdk_region_union_with_rect (outline, &line);

    gdk_window_invalidate_region (paned->outer->window, outline, TRUE);
    gdk_window_invalidate_rect (paned->outer->window, &paned->drop_rect, TRUE);

    gdk_region_destroy (outline);
}


static gboolean moo_big_paned_expose        (GtkWidget      *widget,
                                             GdkEventExpose *event,
                                             MooBigPaned    *paned)
{
    GTK_WIDGET_CLASS(G_OBJECT_GET_CLASS (widget))->expose_event (widget, event);

    if (paned->drop_pos >= 0)
    {
        g_return_val_if_fail (paned->drop_outline != NULL, FALSE);
        gdk_window_show (paned->drop_outline);
        gdk_draw_rectangle (paned->drop_outline,
                            widget->style->fg_gc[GTK_STATE_NORMAL],
                            FALSE, 0, 0,
                            paned->drop_rect.width - 1,
                            paned->drop_rect.height - 1);
    }

    return TRUE;
}


static GdkBitmap   *create_rect_mask        (int             width,
                                             int             height)
{
    GdkBitmap *bitmap;
    GdkGC *gc;
    GdkColor white = {0, 0, 0, 0};
    GdkColor black = {!0, !0, !0, !0};

    bitmap = gdk_pixmap_new (NULL, width, height, 1);
    gc = gdk_gc_new (bitmap);

    gdk_gc_set_foreground (gc, &white);
    gdk_draw_rectangle (bitmap, gc, TRUE, 0, 0,
                        width, height);

    gdk_gc_set_foreground (gc, &black);
    gdk_draw_rectangle (bitmap, gc, FALSE, 0, 0,
                        width - 1, height - 1);

    g_object_unref (gc);
    return bitmap;
}


static void         create_drop_outline     (MooBigPaned    *paned)
{
    static GdkWindowAttr attributes;
    int attributes_mask;
    GdkBitmap *mask;

    g_return_if_fail (paned->drop_outline == NULL);

    attributes.x = paned->drop_rect.x;
    attributes.y = paned->drop_rect.y;
    attributes.width = paned->drop_rect.width;
    attributes.height = paned->drop_rect.height;
    attributes.window_type = GDK_WINDOW_CHILD;

    attributes.visual = gtk_widget_get_visual (paned->outer);
    attributes.colormap = gtk_widget_get_colormap (paned->outer);
    attributes.wclass = GDK_INPUT_OUTPUT;

    attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

    paned->drop_outline = gdk_window_new (paned->outer->window, &attributes, attributes_mask);
    gdk_window_set_user_data (paned->drop_outline, paned);

    mask = create_rect_mask (paned->drop_rect.width, paned->drop_rect.height);
    gdk_window_shape_combine_mask (paned->drop_outline, mask, 0, 0);
    g_object_unref (mask);
}
