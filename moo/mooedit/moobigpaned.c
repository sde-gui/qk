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

#include "mooedit/moobigpaned.h"
#include "mooutils/moostock.h"
#include "mooutils/moomarshals.h"


static void     moo_big_paned_finalize      (GObject        *object);
static void     moo_big_paned_set_property  (GObject        *object,
                                             guint           prop_id,
                                             const GValue   *value,
                                             GParamSpec     *pspec);
static void     moo_big_paned_get_property  (GObject        *object,
                                             guint           prop_id,
                                             GValue         *value,
                                             GParamSpec     *pspec);

static void     child_open_pane             (MooPaned       *child,
                                             guint           index,
                                             MooBigPaned    *paned);
static void     child_hide_pane             (MooPaned       *child,
                                             MooBigPaned    *paned);
static gboolean check_children_order        (MooBigPaned    *paned);


/* MOO_TYPE_BIG_PANED */
G_DEFINE_TYPE (MooBigPaned, moo_big_paned, GTK_TYPE_FRAME)

enum {
    PROP_0,
    PROP_PANE_ORDER
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

    /* XXX destroy */
    /* XXX order */
    for (i = 0; i < 4; ++i)
    {
        MooPaned *child;

        paned->paned[i] = child = MOO_PANED (moo_paned_new (i));

        g_object_ref (child);
        gtk_object_sink (GTK_OBJECT (child));
        gtk_widget_show (GTK_WIDGET (child));

        g_signal_connect (child, "hide-pane",
                          G_CALLBACK (child_hide_pane),
                          paned);
        g_signal_connect (child, "open-pane",
                          G_CALLBACK (child_open_pane),
                          paned);
    }

    paned->order[0] = MOO_PANE_POS_LEFT;
    paned->order[1] = MOO_PANE_POS_RIGHT;
    paned->order[2] = MOO_PANE_POS_TOP;
    paned->order[3] = MOO_PANE_POS_BOTTOM;

    paned->inner = NTH_CHILD (paned, 3);

    gtk_container_add (GTK_CONTAINER (paned),
                       GTK_WIDGET (NTH_CHILD (paned, 0)));

    for (i = 0; i < 3; ++i)
        gtk_container_add (GTK_CONTAINER (NTH_CHILD (paned, i)),
                           GTK_WIDGET (NTH_CHILD (paned, i+1)));

    g_assert (check_children_order (paned));
}


static gboolean check_children_order        (MooBigPaned    *paned)
{
    int i;

    if (GTK_BIN(paned)->child != GTK_WIDGET (NTH_CHILD (paned, 0)))
        return FALSE;

    for (i = 0; i < 3; ++i)
        if (GTK_BIN (NTH_CHILD (paned, i))->child !=
            GTK_WIDGET (NTH_CHILD (paned, i+1)))
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

    gtk_container_remove (GTK_CONTAINER (paned), GTK_WIDGET (NTH_CHILD (paned, 0)));
    for (i = 0; i < 3; ++i)
        gtk_container_remove (GTK_CONTAINER (NTH_CHILD (paned, i)),
                              GTK_WIDGET (NTH_CHILD (paned, i+1)));
    if (child)
        gtk_container_remove (GTK_CONTAINER (NTH_CHILD (paned, 3)), child);

    for (i = 0; i < 4; ++i)
        paned->order[i] = new_order[i];

    gtk_container_add (GTK_CONTAINER (paned), GTK_WIDGET (NTH_CHILD (paned, 0)));

    for (i = 0; i < 3; ++i)
        gtk_container_add (GTK_CONTAINER (NTH_CHILD (paned, i)), GTK_WIDGET (NTH_CHILD (paned, i+1)));

    paned->inner = NTH_CHILD (paned, 3);

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

    G_OBJECT_CLASS (moo_big_paned_parent_class)->finalize (object);
}


GtkWidget  *moo_big_paned_new               (void)
{
    return GTK_WIDGET (g_object_new (MOO_TYPE_BIG_PANED, NULL));
}


static void     child_open_pane             (MooPaned       *child,
                                             guint           index,
                                             MooBigPaned    *paned)
{
    MooPanePosition pos;

    g_object_get (child, "pane-position", &pos, NULL);
    g_return_if_fail (paned->paned[pos] == child);

    g_signal_emit (paned, signals[OPEN_PANE], 0, pos, index);
}


static void     child_hide_pane             (MooPaned       *child,
                                             MooBigPaned    *paned)
{
    MooPanePosition pos;

    g_object_get (child, "pane-position", &pos, NULL);
    g_return_if_fail (paned->paned[pos] == child);

    g_signal_emit (paned, signals[HIDE_PANE], 0, pos);
}


void        moo_big_paned_add_pane          (MooBigPaned        *paned,
                                             GtkWidget          *pane_widget,
                                             MooPanePosition     position,
                                             const char         *button_label,
                                             const char         *button_stock_id)
{
    g_return_if_fail (MOO_IS_BIG_PANED (paned));
    g_return_if_fail (GTK_IS_WIDGET (pane_widget));
    g_return_if_fail (position < 4);

    moo_paned_add_pane (paned->paned[position], pane_widget,
                        button_label, button_stock_id);
}


void        moo_big_paned_insert_pane       (MooBigPaned        *paned,
                                             GtkWidget          *pane_widget,
                                             GtkWidget          *button_widget,
                                             MooPanePosition     position,
                                             int                 index_)
{
    g_return_if_fail (MOO_IS_BIG_PANED (paned));
    g_return_if_fail (GTK_IS_WIDGET (pane_widget));
    g_return_if_fail (position < 4);

    moo_paned_insert_pane (paned->paned[position], pane_widget,
                           button_widget, index_);
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


static void     moo_big_paned_set_property  (GObject        *object,
                                             guint           prop_id,
                                             const GValue   *value,
                                             GParamSpec     *pspec)
{
    MooBigPaned *paned = MOO_BIG_PANED (object);

    switch (prop_id)
    {
        case PROP_PANE_ORDER:
            moo_big_paned_set_pane_order (paned, g_value_get_pointer (value));
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

    switch (prop_id)
    {
        case PROP_PANE_ORDER:
            g_value_set_pointer (value, paned->order);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}
