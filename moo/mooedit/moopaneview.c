/*
 *   moopaneview.c
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

#include "mooedit/moopaneview.h"


static void     moo_pane_view_finalize      (GObject        *object);

static void     moo_pane_view_set_property  (GObject        *object,
                                             guint           prop_id,
                                             const GValue   *value,
                                             GParamSpec     *pspec);
static void     moo_pane_view_get_property  (GObject        *object,
                                             guint           prop_id,
                                             GValue         *value,
                                             GParamSpec     *pspec);

static gboolean moo_pane_view_button_press  (GtkWidget      *widget,
                                             GdkEventButton *event);


enum {
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];


enum {
    PROP_0,
};


/* MOO_TYPE_PANE_VIEW */
G_DEFINE_TYPE (MooPaneView, moo_pane_view, GTK_TYPE_SOURCE_VIEW)


static void moo_pane_view_class_init (MooPaneViewClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    gobject_class->set_property = moo_pane_view_set_property;
    gobject_class->get_property = moo_pane_view_get_property;
    gobject_class->finalize = moo_pane_view_finalize;

    widget_class->button_press_event = moo_pane_view_button_press;
}


// static void moo_pane_view_init (MooPaneView *view)
// {
//     view->priv = moo_pane_view_private_new ();
// }


// static void moo_pane_view_finalize       (GObject      *object)
// {
//     MooPaneView *view = MOO_PANE_VIEW (object);
//
//     moo_prefs_notify_disconnect (view->priv->prefs_notify);
//     if (view->priv->file_watch_id)
//         g_source_remove (view->priv->file_watch_id);
//
//     g_free (view->priv->filename);
//     g_free (view->priv->basename);
//     g_free (view->priv->display_filename);
//     g_free (view->priv->display_basename);
//     g_free (view->priv->encoding);
//
//     if (view->priv->lang)
//         g_object_unref (view->priv->lang);
//
//     g_free (view->priv);
//     view->priv = NULL;
//
//     moo_pane_view_instances = g_slist_remove (moo_pane_view_instances, view);
//
//     G_OBJECT_CLASS (moo_pane_view_parent_class)->finalize (object);
// }


GtkWidget*
moo_pane_view_new (void)
{
    return g_object_new (MOO_TYPE_PANE_VIEW, NULL);
}
