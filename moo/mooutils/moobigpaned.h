/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   moobigpaned.h
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

#ifndef __MOO_BIG_PANED_H__
#define __MOO_BIG_PANED_H__

#include <mooutils/moopaned.h>
#include <gtk/gtkframe.h>

G_BEGIN_DECLS


#define MOO_TYPE_BIG_PANED              (moo_big_paned_get_type ())
#define MOO_BIG_PANED(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_BIG_PANED, MooBigPaned))
#define MOO_BIG_PANED_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_BIG_PANED, MooBigPanedClass))
#define MOO_IS_BIG_PANED(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_BIG_PANED))
#define MOO_IS_BIG_PANED_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_BIG_PANED))
#define MOO_BIG_PANED_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_BIG_PANED, MooBigPanedClass))


typedef struct _MooBigPaned         MooBigPaned;
typedef struct _MooBigPanedPrivate  MooBigPanedPrivate;
typedef struct _MooBigPanedClass    MooBigPanedClass;

struct _MooBigPaned
{
    GtkFrame     parent;

    GtkWidget   *paned[4];  /* indexed by PanePos */
    MooPanePosition order[4]; /* inner is paned[order[3]]*/
    GtkWidget   *inner;
    GtkWidget   *outer;

    int          drop_pos;
    GdkRectangle drop_rect;
    GdkWindow   *drop_outline;
};

struct _MooBigPanedClass
{
    GtkFrameClass parent_class;

    /* these just proxy corresponding Paned signals */
    void (*open_pane)           (MooBigPaned    *paned,
                                 MooPanePosition position,
                                 guint           index);
    void (*hide_pane)           (MooBigPaned    *paned,
                                 MooPanePosition position);
    void (*attach_pane)         (MooBigPaned    *paned,
                                 MooPanePosition position,
                                 guint           index);
    void (*detach_pane)         (MooBigPaned    *paned,
                                 MooPanePosition position,
                                 guint           index);
    void (*pane_params_changed) (MooBigPaned    *paned,
                                 MooPanePosition position,
                                 guint           index);
    void (*set_pane_size)       (MooBigPaned    *paned,
                                 MooPanePosition position,
                                 int             size);
};


GType           moo_big_paned_get_type          (void) G_GNUC_CONST;

GtkWidget      *moo_big_paned_new               (void);

void            moo_big_paned_set_pane_order    (MooBigPaned    *paned,
                                                 int            *order);

gboolean        moo_big_paned_find_pane         (MooBigPaned    *paned,
                                                 GtkWidget      *pane_widget,
                                                 MooPaned      **child_paned,
                                                 int            *pane_index);

void            moo_big_paned_add_child         (MooBigPaned    *paned,
                                                 GtkWidget      *widget);
void            moo_big_paned_remove_child      (MooBigPaned    *paned);
GtkWidget      *moo_big_paned_get_child         (MooBigPaned    *paned);

int             moo_big_paned_insert_pane       (MooBigPaned    *paned,
                                                 GtkWidget      *pane_widget,
                                                 MooPaneLabel   *pane_label,
                                                 MooPanePosition position,
                                                 int             index_);
gboolean        moo_big_paned_remove_pane       (MooBigPaned    *paned,
                                                 GtkWidget      *pane_widget);

GtkWidget      *moo_big_paned_get_pane          (MooBigPaned    *paned,
                                                 MooPanePosition position,
                                                 int             index_);
GtkWidget      *moo_big_paned_get_button        (MooBigPaned    *paned,
                                                 GtkWidget      *pane_widget);

void            moo_big_paned_open_pane         (MooBigPaned    *paned,
                                                 GtkWidget      *pane_widget);
void            moo_big_paned_hide_pane         (MooBigPaned    *paned,
                                                 GtkWidget      *pane_widget);
void            moo_big_paned_present_pane      (MooBigPaned    *paned,
                                                 GtkWidget      *pane_widget);

MooPaneParams  *moo_big_paned_get_pane_params   (MooBigPaned    *paned,
                                                 MooPanePosition position,
                                                 guint           index_);
void            moo_big_paned_set_pane_params   (MooBigPaned    *paned,
                                                 MooPanePosition position,
                                                 guint           index_,
                                                 MooPaneParams  *params);


G_END_DECLS

#endif /* __MOO_BIG_PANED_H__ */
