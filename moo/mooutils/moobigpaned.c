/*
 *   moobigpaned.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "moobigpaned.h"
#include "moomarshals.h"

#ifdef MOO_COMPILATION
#include "mooutils-misc.h"
#else
#if GLIB_CHECK_VERSION(2,10,0)
#define MOO_OBJECT_REF_SINK(obj) g_object_ref_sink (obj)
#else
#define MOO_OBJECT_REF_SINK(obj) gtk_object_sink (g_object_ref (obj))
#endif
#endif

struct MooBigPanedPrivate {
    MooPanePosition order[4]; /* inner is paned[order[3]]*/
    GtkWidget   *inner;
    GtkWidget   *outer;

    int          drop_pos;
    int          drop_button_index;
    GdkRectangle drop_rect;
    GdkRectangle drop_button_rect;
    GdkWindow   *drop_outline;
    struct {
        int        bbox_size;
        GdkRegion *bbox_region;
        GdkRegion *def_region;
    } *dz;
    GdkRegion    *drop_region;
    guint         drop_region_is_buttons : 1;
};

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

static void     child_set_pane_size         (GtkWidget      *child,
                                             int             size,
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
                                             gboolean        drop,
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
    SET_PANE_SIZE,
    NUM_SIGNALS
};

static guint signals[NUM_SIGNALS];

static void
moo_big_paned_class_init (MooBigPanedClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (klass, sizeof (MooBigPanedPrivate));

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

    signals[SET_PANE_SIZE] =
            g_signal_new ("set-pane-size",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooBigPanedClass, set_pane_size),
                          NULL, NULL,
                          _moo_marshal_VOID__INT,
                          G_TYPE_NONE, 1,
                          G_TYPE_INT);
}


#define NTH_CHILD(paned,n) paned->paned[paned->priv->order[n]]

static void
moo_big_paned_init (MooBigPaned *paned)
{
    int i;

    paned->priv = G_TYPE_INSTANCE_GET_PRIVATE (paned,
                                               MOO_TYPE_BIG_PANED,
                                               MooBigPanedPrivate);

    paned->priv->drop_pos = -1;

    /* XXX destroy */
    for (i = 0; i < 4; ++i)
    {
        GtkWidget *child;

        paned->paned[i] = child =
                g_object_new (MOO_TYPE_PANED,
                              "pane-position", (MooPanePosition) i,
                              NULL);

        MOO_OBJECT_REF_SINK (child);
        gtk_widget_show (child);

        g_signal_connect_after (child, "set-pane-size",
                                G_CALLBACK (child_set_pane_size),
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

    paned->priv->order[0] = MOO_PANE_POS_LEFT;
    paned->priv->order[1] = MOO_PANE_POS_RIGHT;
    paned->priv->order[2] = MOO_PANE_POS_TOP;
    paned->priv->order[3] = MOO_PANE_POS_BOTTOM;

    paned->priv->inner = NTH_CHILD (paned, 3);
    paned->priv->outer = NTH_CHILD (paned, 0);

    gtk_container_add (GTK_CONTAINER (paned), NTH_CHILD (paned, 0));

    for (i = 0; i < 3; ++i)
        gtk_container_add (GTK_CONTAINER (NTH_CHILD (paned, i)), NTH_CHILD (paned, i+1));

    g_assert (check_children_order (paned));
}


static gboolean
check_children_order (MooBigPaned *paned)
{
    int i;

    if (GTK_BIN (paned)->child != NTH_CHILD (paned, 0))
        return FALSE;

    for (i = 0; i < 3; ++i)
        if (GTK_BIN (NTH_CHILD (paned, i))->child != NTH_CHILD (paned, i+1))
                return FALSE;

    return TRUE;
}


void
moo_big_paned_set_pane_order (MooBigPaned *paned,
                              int         *order)
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
        if (new_order[i] != paned->priv->order[i])
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
        paned->priv->order[i] = new_order[i];

    gtk_container_add (GTK_CONTAINER (paned), NTH_CHILD (paned, 0));

    for (i = 0; i < 3; ++i)
        gtk_container_add (GTK_CONTAINER (NTH_CHILD (paned, i)), NTH_CHILD (paned, i+1));

    paned->priv->inner = NTH_CHILD (paned, 3);
    paned->priv->outer = NTH_CHILD (paned, 0);

    if (child)
    {
        gtk_container_add (GTK_CONTAINER (paned->priv->inner), child);
        g_object_unref (child);
    }

    g_assert (check_children_order (paned));
    g_object_notify (G_OBJECT (paned), "pane-order");
}


static void
moo_big_paned_finalize (GObject *object)
{
    MooBigPaned *paned = MOO_BIG_PANED (object);
    int i;

    for (i = 0; i < 4; ++i)
        g_object_unref (paned->paned[i]);

    if (paned->priv->drop_outline)
    {
        g_critical ("%s: oops", G_STRLOC);
        gdk_window_set_user_data (paned->priv->drop_outline, NULL);
        gdk_window_destroy (paned->priv->drop_outline);
    }

    G_OBJECT_CLASS (moo_big_paned_parent_class)->finalize (object);
}


GtkWidget*
moo_big_paned_new (void)
{
    return g_object_new (MOO_TYPE_BIG_PANED, NULL);
}


static void
child_set_pane_size (GtkWidget      *child,
                     int             size,
                     MooBigPaned    *paned)
{
    MooPanePosition pos;

    g_object_get (child, "pane-position", &pos, NULL);
    g_return_if_fail (paned->paned[pos] == child);

    g_signal_emit (paned, signals[SET_PANE_SIZE], 0, pos, size);
}


MooPane *
moo_big_paned_insert_pane (MooBigPaned        *paned,
                           GtkWidget          *pane_widget,
                           MooPaneLabel       *pane_label,
                           MooPanePosition     position,
                           int                 index_)
{
    g_return_val_if_fail (MOO_IS_BIG_PANED (paned), NULL);
    g_return_val_if_fail (GTK_IS_WIDGET (pane_widget), NULL);
    g_return_val_if_fail (position < 4, NULL);

    return moo_paned_insert_pane (MOO_PANED (paned->paned[position]),
                                  pane_widget, pane_label, index_);
}


void
moo_big_paned_add_child (MooBigPaned *paned,
                         GtkWidget   *child)
{
    g_return_if_fail (MOO_IS_BIG_PANED (paned));
    gtk_container_add (GTK_CONTAINER (paned->priv->inner), child);
}


void
moo_big_paned_remove_child (MooBigPaned *paned)
{
    g_return_if_fail (MOO_IS_BIG_PANED (paned));
    gtk_container_remove (GTK_CONTAINER (paned->priv->inner),
                          GTK_BIN (paned->priv->inner)->child);
}


GtkWidget *
moo_big_paned_get_child (MooBigPaned *paned)
{
    g_return_val_if_fail (MOO_IS_BIG_PANED (paned), NULL);
    return GTK_BIN(paned->priv->inner)->child;
}


gboolean
moo_big_paned_remove_pane (MooBigPaned *paned,
                           GtkWidget   *widget)
{
    MooPaned *child;

    g_return_val_if_fail (MOO_IS_BIG_PANED (paned), FALSE);
    g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);

    if (!moo_big_paned_find_pane (paned, widget, &child))
        return FALSE;

    return moo_paned_remove_pane (child, widget);
}


#define PROXY_FUNC(name)                                    \
void                                                        \
moo_big_paned_##name (MooBigPaned *paned,                   \
                      GtkWidget   *widget)                  \
{                                                           \
    MooPane *pane;                                          \
    MooPaned *child = NULL;                                 \
                                                            \
    g_return_if_fail (MOO_IS_BIG_PANED (paned));            \
    g_return_if_fail (GTK_IS_WIDGET (widget));              \
                                                            \
    pane = moo_big_paned_find_pane (paned, widget, &child); \
    g_return_if_fail (pane != NULL);                        \
                                                            \
    moo_paned_##name (child, pane);                         \
}

PROXY_FUNC (open_pane)
PROXY_FUNC (present_pane)
PROXY_FUNC (attach_pane)
PROXY_FUNC (detach_pane)

#undef PROXY_FUNC

void
moo_big_paned_hide_pane (MooBigPaned *paned,
                         GtkWidget   *widget)
{
    MooPaned *child = NULL;

    g_return_if_fail (MOO_IS_BIG_PANED (paned));
    g_return_if_fail (GTK_IS_WIDGET (widget));

    moo_big_paned_find_pane (paned, widget, &child);
    g_return_if_fail (child != NULL);

    moo_paned_hide_pane (child);
}


MooPaned *
moo_big_paned_get_paned (MooBigPaned    *paned,
                         MooPanePosition position)
{
    g_return_val_if_fail (MOO_IS_BIG_PANED (paned), NULL);
    g_return_val_if_fail (position < 4, NULL);
    return MOO_PANED (paned->paned[position]);
}


MooPane *
moo_big_paned_find_pane (MooBigPaned    *paned,
                         GtkWidget      *widget,
                         MooPaned      **child_paned)
{
    int i;
    MooPane *pane;

    g_return_val_if_fail (MOO_IS_BIG_PANED (paned), NULL);
    g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);

    if (child_paned)
        *child_paned = NULL;

    for (i = 0; i < 4; ++i)
    {
        pane = moo_paned_get_pane (MOO_PANED (paned->paned[i]), widget);

        if (pane)
        {
            if (child_paned)
                *child_paned = MOO_PANED (paned->paned[i]);
            return pane;
        }
    }

    return NULL;
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
            g_value_set_pointer (value, paned->priv->order);
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


GtkWidget *
moo_big_paned_get_pane (MooBigPaned    *paned,
                        MooPanePosition position,
                        int             index_)
{
    g_return_val_if_fail (MOO_IS_BIG_PANED (paned), NULL);
    g_return_val_if_fail (position < 4, NULL);
    return moo_pane_get_child (moo_paned_get_nth_pane (MOO_PANED (paned->paned[position]), index_));
}


/*****************************************************************************/
/* rearranging panes
 */

static void         create_drop_outline     (MooBigPaned    *paned);
static void         get_drop_area           (MooBigPaned    *paned,
                                             MooPaned       *active_child,
                                             MooPanePosition position,
                                             int             index,
                                             GdkRectangle   *rect,
                                             GdkRectangle   *button_rect);
// static void         invalidate_drop_outline (MooBigPaned    *paned);


static GdkRegion *
region_6 (int x0, int y0,
          int x1, int y1,
          int x2, int y2,
          int x3, int y3,
          int x4, int y4,
          int x5, int y5)
{
    GdkPoint points[6];
    points[0].x = x0; points[0].y = y0;
    points[1].x = x1; points[1].y = y1;
    points[2].x = x2; points[2].y = y2;
    points[3].x = x3; points[3].y = y3;
    points[4].x = x4; points[4].y = y4;
    points[5].x = x5; points[5].y = y5;
    return gdk_region_polygon (points, 6, GDK_WINDING_RULE);
}

static GdkRegion *
region_4 (int x0, int y0,
          int x1, int y1,
          int x2, int y2,
          int x3, int y3)
{
    GdkPoint points[4];
    points[0].x = x0; points[0].y = y0;
    points[1].x = x1; points[1].y = y1;
    points[2].x = x2; points[2].y = y2;
    points[3].x = x3; points[3].y = y3;
    return gdk_region_polygon (points, 4, GDK_WINDING_RULE);
}


#define LEFT(rect) ((rect).x)
#define RIGHT(rect) ((rect).x + (rect).width - 1)
#define TOP(rect) ((rect).y)
#define BOTTOM(rect) ((rect).y + (rect).height - 1)

static void
get_drop_zones (MooBigPaned *paned)
{
    int pos;
    GdkRectangle parent;
    GdkRectangle child_rect;
    GdkRectangle button_rect;
    GdkRectangle drop_rect;
    int bbox_size[4];

    g_return_if_fail (paned->priv->dz == NULL);

    paned->priv->dz = g_malloc0 (4 * sizeof *paned->priv->dz);

    for (pos = 0; pos < 4; ++pos)
    {
        bbox_size[pos] = moo_paned_get_button_box_size (MOO_PANED (paned->paned[pos]));
        if (bbox_size[pos] <= 0)
            bbox_size[pos] = 30;
        paned->priv->dz[pos].bbox_size = bbox_size[pos];
    }

    parent = paned->priv->outer->allocation;
//     g_print ("parent: %d, %d, %d, %d\n", parent.x, parent.y, parent.width, parent.height);

    child_rect = parent;
    child_rect.x += bbox_size[MOO_PANE_POS_LEFT];
    child_rect.width -= bbox_size[MOO_PANE_POS_LEFT] +
                        bbox_size[MOO_PANE_POS_RIGHT];
    child_rect.y += bbox_size[MOO_PANE_POS_TOP];
    child_rect.height -= bbox_size[MOO_PANE_POS_TOP] +
                         bbox_size[MOO_PANE_POS_BOTTOM];
//     g_print ("child_rect: %d, %d, %d, %d\n", child_rect.x, child_rect.y, child_rect.width, child_rect.height);

    button_rect = parent;
    button_rect.x += 2*bbox_size[MOO_PANE_POS_LEFT];
    button_rect.width -= 2*bbox_size[MOO_PANE_POS_LEFT] +
                         2*bbox_size[MOO_PANE_POS_RIGHT];
    button_rect.y += 2*bbox_size[MOO_PANE_POS_TOP];
    button_rect.height -= 2*bbox_size[MOO_PANE_POS_TOP] +
                          2*bbox_size[MOO_PANE_POS_BOTTOM];
//     g_print ("button_rect: %d, %d, %d, %d\n", button_rect.x, button_rect.y, button_rect.width, button_rect.height);

    drop_rect = button_rect;
    drop_rect.x += button_rect.width / 3;
    drop_rect.y += button_rect.height / 3;
    drop_rect.width -= 2 * button_rect.width / 3;
    drop_rect.height -= 2 * button_rect.height / 3;
//     g_print ("drop_rect: %d, %d, %d, %d\n", drop_rect.x, drop_rect.y, drop_rect.width, drop_rect.height);

    paned->priv->dz[MOO_PANE_POS_TOP].bbox_region =
        region_6 (LEFT (child_rect), TOP (parent),
                  RIGHT (child_rect), TOP (parent),
                  RIGHT (child_rect), TOP (child_rect),
                  RIGHT (button_rect), TOP (button_rect),
                  LEFT (button_rect), TOP (button_rect),
                  LEFT (child_rect), TOP (child_rect));
    paned->priv->dz[MOO_PANE_POS_TOP].def_region =
        region_4 (LEFT (button_rect), TOP (button_rect),
                  RIGHT (button_rect), TOP (button_rect),
                  RIGHT (drop_rect), TOP (drop_rect),
                  LEFT (drop_rect), TOP (drop_rect));

    paned->priv->dz[MOO_PANE_POS_LEFT].bbox_region =
        region_6 (LEFT (parent), TOP (child_rect),
                  LEFT (child_rect), TOP (child_rect),
                  LEFT (button_rect), TOP (button_rect),
                  LEFT (button_rect), BOTTOM (button_rect),
                  LEFT (child_rect), BOTTOM (child_rect),
                  LEFT (parent), BOTTOM (child_rect));
    paned->priv->dz[MOO_PANE_POS_LEFT].def_region =
        region_4 (LEFT (button_rect), TOP (button_rect),
                  LEFT (drop_rect), TOP (drop_rect),
                  LEFT (drop_rect), BOTTOM (drop_rect),
                  LEFT (button_rect), BOTTOM (button_rect));

    paned->priv->dz[MOO_PANE_POS_BOTTOM].bbox_region =
        region_6 (LEFT (child_rect), BOTTOM (parent),
                  LEFT (child_rect), BOTTOM (child_rect),
                  LEFT (button_rect), BOTTOM (button_rect),
                  RIGHT (button_rect), BOTTOM (button_rect),
                  RIGHT (child_rect), BOTTOM (child_rect),
                  RIGHT (child_rect), BOTTOM (parent));
    paned->priv->dz[MOO_PANE_POS_BOTTOM].def_region =
        region_4 (LEFT (button_rect), BOTTOM (button_rect),
                  LEFT (drop_rect), BOTTOM (drop_rect),
                  RIGHT (drop_rect), BOTTOM (drop_rect),
                  RIGHT (button_rect), BOTTOM (button_rect));

    paned->priv->dz[MOO_PANE_POS_RIGHT].bbox_region =
        region_6 (RIGHT (parent), TOP (child_rect),
                  RIGHT (child_rect), TOP (child_rect),
                  RIGHT (button_rect), TOP (button_rect),
                  RIGHT (button_rect), BOTTOM (button_rect),
                  RIGHT (child_rect), BOTTOM (child_rect),
                  RIGHT (parent), BOTTOM (child_rect));
    paned->priv->dz[MOO_PANE_POS_RIGHT].def_region =
        region_4 (RIGHT (button_rect), TOP (button_rect),
                  RIGHT (drop_rect), TOP (drop_rect),
                  RIGHT (drop_rect), BOTTOM (drop_rect),
                  RIGHT (button_rect), BOTTOM (button_rect));

    paned->priv->drop_region = NULL;
    paned->priv->drop_region_is_buttons = FALSE;
}

#undef LEFT
#undef RIGHT
#undef TOP
#undef BOTTOM


static void
handle_drag_start (G_GNUC_UNUSED MooPaned *child,
                   G_GNUC_UNUSED GtkWidget *pane_widget,
                   MooBigPaned *paned)
{
    g_return_if_fail (GTK_WIDGET_REALIZED (paned->priv->outer));

    g_signal_connect (paned->priv->outer, "expose-event",
                      G_CALLBACK (moo_big_paned_expose), paned);

    paned->priv->drop_pos = -1;
    get_drop_zones (paned);
}


static gboolean
get_new_button_index (MooBigPaned *paned,
                      MooPaned    *active_child,
                      int          x,
                      int          y)
{
    int new_button;
    MooPaned *child;

    g_assert (paned->priv->drop_pos >= 0);
    child = MOO_PANED (paned->paned[paned->priv->drop_pos]);

    new_button = _moo_paned_get_button (child, x, y,
                                        paned->priv->outer->window);

    if (child == active_child)
    {
        int n_buttons = moo_paned_n_panes (child);
        if (new_button >= n_buttons || new_button < 0)
            new_button = n_buttons - 1;
    }

    if (new_button != paned->priv->drop_button_index)
    {
        paned->priv->drop_button_index = new_button;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static void
get_default_button_index (MooBigPaned *paned,
                          MooPaned    *active_child)
{
    MooPaned *child;

    g_assert (paned->priv->drop_pos >= 0);
    child = MOO_PANED (paned->paned[paned->priv->drop_pos]);

    if (child == active_child)
        paned->priv->drop_button_index =
            _moo_paned_get_open_pane_index (child);
    else
        paned->priv->drop_button_index = -1;
}

static gboolean
get_new_drop_position (MooBigPaned *paned,
                       MooPaned    *active_child,
                       int          x,
                       int          y)
{
    int pos;
    gboolean changed = FALSE;

    if (paned->priv->drop_region)
    {
        g_assert (paned->priv->drop_pos >= 0);

        if (gdk_region_point_in (paned->priv->drop_region, x, y))
        {
            if (!paned->priv->drop_region_is_buttons)
                return FALSE;
            else
                return get_new_button_index (paned, active_child, x, y);
        }

        paned->priv->drop_pos = -1;
        paned->priv->drop_region = NULL;
        changed = TRUE;
    }

    for (pos = 0; pos < 4; ++pos)
    {
        if (gdk_region_point_in (paned->priv->dz[pos].def_region, x, y))
        {
            paned->priv->drop_pos = pos;
            paned->priv->drop_region = paned->priv->dz[pos].def_region;
            paned->priv->drop_region_is_buttons = FALSE;
            get_default_button_index (paned, active_child);
            return TRUE;
        }

        if (gdk_region_point_in (paned->priv->dz[pos].bbox_region, x, y))
        {
            paned->priv->drop_pos = pos;
            paned->priv->drop_region = paned->priv->dz[pos].bbox_region;
            paned->priv->drop_region_is_buttons = TRUE;
            get_new_button_index (paned, active_child, x, y);
            return TRUE;
        }
    }

    return changed;
}

static void
handle_drag_motion (MooPaned       *child,
                    G_GNUC_UNUSED GtkWidget *pane_widget,
                    MooBigPaned    *paned)
{
    int x, y;

    g_return_if_fail (GTK_WIDGET_REALIZED (paned->priv->outer));

    gdk_window_get_pointer (paned->priv->outer->window, &x, &y, NULL);

    if (!get_new_drop_position (paned, child, x, y))
        return;

    if (paned->priv->drop_outline)
    {
        gdk_window_set_user_data (paned->priv->drop_outline, NULL);
        gdk_window_destroy (paned->priv->drop_outline);
        paned->priv->drop_outline = NULL;
    }

    if (paned->priv->drop_pos >= 0)
    {
        get_drop_area (paned, child, paned->priv->drop_pos,
                       paned->priv->drop_button_index,
                       &paned->priv->drop_rect,
                       &paned->priv->drop_button_rect);
        create_drop_outline (paned);
    }
}


static void
cleanup_drag (MooBigPaned *paned)
{
    int pos;

    if (paned->priv->drop_outline)
    {
        gdk_window_set_user_data (paned->priv->drop_outline, NULL);
        gdk_window_destroy (paned->priv->drop_outline);
        paned->priv->drop_outline = NULL;
    }

    paned->priv->drop_pos = -1;
    paned->priv->drop_region = NULL;

    g_signal_handlers_disconnect_by_func (paned->priv->outer,
                                          (gpointer) moo_big_paned_expose,
                                          paned);

    for (pos = 0; pos < 4; ++pos)
    {
        gdk_region_destroy (paned->priv->dz[pos].bbox_region);
        gdk_region_destroy (paned->priv->dz[pos].def_region);
    }

    g_free (paned->priv->dz);
    paned->priv->dz = NULL;
}

static void
handle_drag_end (MooPaned    *child,
                 GtkWidget   *pane_widget,
                 gboolean     drop,
                 MooBigPaned *paned)
{
    int x, y;
    MooPanePosition new_pos, old_pos;
    int new_index, old_index;

    g_return_if_fail (GTK_WIDGET_REALIZED (paned->priv->outer));

    if (!drop)
    {
        cleanup_drag (paned);
        return;
    }

    gdk_window_get_pointer (paned->priv->outer->window, &x, &y, NULL);
    get_new_drop_position (paned, child, x, y);

    if (paned->priv->drop_pos < 0)
    {
        cleanup_drag (paned);
        return;
    }

    new_pos = paned->priv->drop_pos;
    new_index = paned->priv->drop_button_index;
    cleanup_drag (paned);

//     {
//         const char *pos_names[] = {"left", "right", "top", "bottom"};
//         g_print ("moving to %s, %d\n", pos_names[new_pos], new_index);
//     }

    g_object_get (child, "pane-position", &old_pos, NULL);
    old_index = _moo_paned_get_open_pane_index (child);

    if (old_pos == new_pos && new_index == old_index)
        return;

    if (old_pos == new_pos)
    {
        MooPane *pane;
        pane = moo_paned_get_pane (child, pane_widget);
        _moo_paned_reorder_child (child, pane, new_index);
    }
    else
    {
        MooPane *pane;

        pane = moo_paned_get_pane (child, pane_widget);
        g_object_ref (pane);

        moo_paned_remove_pane (child, pane_widget);
        _moo_paned_insert_pane (MOO_PANED (paned->paned[new_pos]), pane, new_index);
        moo_pane_open (pane);
        _moo_pane_params_changed (pane);

        g_object_unref (pane);
    }
}


static void
get_drop_area (MooBigPaned    *paned,
               MooPaned       *active_child,
               MooPanePosition position,
               int             index,
               GdkRectangle   *rect,
               GdkRectangle   *button_rect)
{
    int width, height, size = 0;
    MooPanePosition active_position;

    width = paned->priv->outer->allocation.width;
    height = paned->priv->outer->allocation.height;

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
            rect->y = paned->priv->outer->allocation.y;
            rect->width = size;
            rect->height = height;
            break;
        case MOO_PANE_POS_TOP:
        case MOO_PANE_POS_BOTTOM:
            rect->x = paned->priv->outer->allocation.x;
            rect->width = width;
            rect->height = size;
            break;
    }

    switch (position)
    {
        case MOO_PANE_POS_LEFT:
            rect->x = paned->priv->outer->allocation.x;
            break;
        case MOO_PANE_POS_RIGHT:
            rect->x = paned->priv->outer->allocation.x + width - size;
            break;
        case MOO_PANE_POS_TOP:
            rect->y = paned->priv->outer->allocation.y;
            break;
        case MOO_PANE_POS_BOTTOM:
            rect->y = paned->priv->outer->allocation.y + height - size;
            break;
    }

    _moo_paned_get_button_position (MOO_PANED (paned->paned[position]),
                                    index, button_rect,
                                    paned->priv->outer->window);
}


#define RECT_POINT_IN(rect,x,y) (x < (rect)->x + (rect)->width &&   \
                                 y < (rect)->height + (rect)->y &&  \
                                 x >= (rect)->x && y >= (rect)->y)

// static int
// get_drop_position (MooBigPaned *paned,
//                    MooPaned    *child,
//                    int          x,
//                    int          y,
//                    int         *button_index)
// {
//     int width, height, i;
//     MooPanePosition position;
//     GdkRectangle rect, button_rect;
//
//     *button_index = -1;
//
//     width = paned->priv->outer->allocation.width;
//     height = paned->priv->outer->allocation.height;
//
//     if (x < paned->priv->outer->allocation.x ||
//         x >= paned->priv->outer->allocation.x + width ||
//         y < paned->priv->outer->allocation.y ||
//         y >= paned->priv->outer->allocation.y + height)
//             return -1;
//
//     g_object_get (child, "pane-position", &position, NULL);
//     g_return_val_if_fail (position < 4, -1);
//
//     get_drop_area (paned, child, position, -1, &rect, &button_rect);
//
//     if (RECT_POINT_IN (&rect, x, y))
//         return position;
//
//     for (i = 0; i < 4; ++i)
//     {
//         if (paned->priv->order[i] == position)
//             continue;
//
//         get_drop_area (paned, child, paned->priv->order[i], -1,
//                        &rect, &button_rect);
//
//         if (RECT_POINT_IN (&rect, x, y))
//             return paned->priv->order[i];
//     }
//
//     return -1;
// }


// static void
// invalidate_drop_outline (MooBigPaned *paned)
// {
//     GdkRectangle line;
//     GdkRegion *outline;
//
//     outline = gdk_region_new ();
//
//     line.x = paned->priv->drop_rect.x;
//     line.y = paned->priv->drop_rect.y;
//     line.width = 2;
//     line.height = paned->priv->drop_rect.height;
//     gdk_region_union_with_rect (outline, &line);
//
//     line.x = paned->priv->drop_rect.x;
//     line.y = paned->priv->drop_rect.y + paned->priv->drop_rect.height;
//     line.width = paned->priv->drop_rect.width;
//     line.height = 2;
//     gdk_region_union_with_rect (outline, &line);
//
//     line.x = paned->priv->drop_rect.x + paned->priv->drop_rect.width;
//     line.y = paned->priv->drop_rect.y;
//     line.width = 2;
//     line.height = paned->priv->drop_rect.height;
//     gdk_region_union_with_rect (outline, &line);
//
//     line.x = paned->priv->drop_rect.x;
//     line.y = paned->priv->drop_rect.y;
//     line.width = paned->priv->drop_rect.width;
//     line.height = 2;
//     gdk_region_union_with_rect (outline, &line);
//
//     gdk_window_invalidate_region (paned->priv->outer->window, outline, TRUE);
//
//     gdk_region_destroy (outline);
// }

static gboolean
moo_big_paned_expose (GtkWidget      *widget,
                      GdkEventExpose *event,
                      MooBigPaned    *paned)
{
    GTK_WIDGET_CLASS(G_OBJECT_GET_CLASS (widget))->expose_event (widget, event);

    if (paned->priv->drop_pos >= 0)
    {
        g_return_val_if_fail (paned->priv->drop_outline != NULL, FALSE);
        gdk_draw_rectangle (paned->priv->drop_outline,
                            widget->style->fg_gc[GTK_STATE_NORMAL],
                            FALSE, 0, 0,
                            paned->priv->drop_rect.width - 1,
                            paned->priv->drop_rect.height - 1);
        gdk_draw_rectangle (paned->priv->drop_outline,
                            widget->style->fg_gc[GTK_STATE_NORMAL],
                            FALSE, 1, 1,
                            paned->priv->drop_rect.width - 3,
                            paned->priv->drop_rect.height - 3);
    }

    return FALSE;
}

static GdkBitmap *
create_rect_mask (int           width,
                  int           height,
                  GdkRectangle *rect)
{
    GdkBitmap *bitmap;
    GdkGC *gc;
    GdkColor white = {0, 0, 0, 0};
    GdkColor black = {1, 1, 1, 1};

    bitmap = gdk_pixmap_new (NULL, width, height, 1);
    gc = gdk_gc_new (bitmap);

    gdk_gc_set_foreground (gc, &white);
    gdk_draw_rectangle (bitmap, gc, TRUE, 0, 0,
                        width, height);

    gdk_gc_set_foreground (gc, &black);
    gdk_draw_rectangle (bitmap, gc, FALSE, 0, 0,
                        width - 1, height - 1);
    gdk_draw_rectangle (bitmap, gc, FALSE, 1, 1,
                        width - 3, height - 3);

    gdk_draw_rectangle (bitmap, gc, FALSE,
                        rect->x, rect->y,
                        rect->width, rect->height);
    gdk_draw_rectangle (bitmap, gc, FALSE,
                        rect->x + 1, rect->y + 1,
                        rect->width - 2, rect->height - 2);

//     g_print ("%d, %d: %d, %d, %d, %d\n", width, height, rect->x, rect->y, rect->width, rect->height);

    g_object_unref (gc);
    return bitmap;
}

static void
create_drop_outline (MooBigPaned *paned)
{
    static GdkWindowAttr attributes;
    int attributes_mask;
    GdkBitmap *mask;
    GdkRectangle button_rect;

    g_return_if_fail (paned->priv->drop_outline == NULL);

    attributes.x = paned->priv->drop_rect.x;
    attributes.y = paned->priv->drop_rect.y;
    attributes.width = paned->priv->drop_rect.width;
    attributes.height = paned->priv->drop_rect.height;
    attributes.window_type = GDK_WINDOW_CHILD;

    attributes.visual = gtk_widget_get_visual (paned->priv->outer);
    attributes.colormap = gtk_widget_get_colormap (paned->priv->outer);
    attributes.wclass = GDK_INPUT_OUTPUT;

    attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

    paned->priv->drop_outline = gdk_window_new (paned->priv->outer->window,
                                                &attributes, attributes_mask);
    gdk_window_set_user_data (paned->priv->drop_outline, paned);

    button_rect = paned->priv->drop_button_rect;
    button_rect.x -= paned->priv->drop_rect.x;
    button_rect.y -= paned->priv->drop_rect.y;
    mask = create_rect_mask (paned->priv->drop_rect.width,
                             paned->priv->drop_rect.height,
                             &button_rect);
    gdk_window_shape_combine_mask (paned->priv->drop_outline, mask, 0, 0);
    g_object_unref (mask);

    gdk_window_show (paned->priv->drop_outline);
}
