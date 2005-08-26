/*
 *   moobigpaned.h
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

#ifndef __MOO_BIG_PANED_H__
#define __MOO_BIG_PANED_H__

#include "moopaned.h"

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
    MooPaned    *paned[4];
    MooPanePosition order[4]; /* inner is paned[order[3]]*/
    MooPaned    *inner;
};

struct _MooBigPanedClass
{
    GtkFrameClass parent_class;

    void (*open_pane) (MooBigPaned    *paned,
                       MooPanePosition position,
                       guint           index);
    void (*hide_pane) (MooBigPaned    *paned,
                       MooPanePosition position);
};


GType       moo_big_paned_get_type          (void) G_GNUC_CONST;

GtkWidget  *moo_big_paned_new               (void);

void        moo_big_paned_set_pane_order    (MooBigPaned    *paned,
                                             int            *order);

void        moo_big_paned_add_child         (MooBigPaned    *paned,
                                             GtkWidget      *widget);
void        moo_big_paned_remove_child      (MooBigPaned    *paned);
GtkWidget  *moo_big_paned_get_child         (MooBigPaned    *paned);

void        moo_big_paned_add_pane          (MooBigPaned    *paned,
                                             GtkWidget      *pane_widget,
                                             MooPanePosition position,
                                             const char     *button_label,
                                             const char     *button_stock_id);
void        moo_big_paned_insert_pane       (MooBigPaned    *paned,
                                             GtkWidget      *pane_widget,
                                             GtkWidget      *button_widget,
                                             MooPanePosition position,
                                             int             index_);


G_END_DECLS

#endif /* __MOO_BIG_PANED_H__ */
