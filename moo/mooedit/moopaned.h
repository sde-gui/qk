/*
 *   mooutils/moopaned.h
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

#ifndef MOOUTILS_MOOPANED_H
#define MOOUTILS_MOOPANED_H

#include <gtk/gtk.h>

G_BEGIN_DECLS


#define MOO_TYPE_PANED              (moo_paned_get_type ())
#define MOO_PANED(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_PANED, MooPaned))
#define MOO_PANED_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_PANED, MooPanedClass))
#define MOO_IS_PANED(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_PANED))
#define MOO_IS_PANED_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_PANED))
#define MOO_PANED_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_PANED, MooPanedClass))

#define MOO_TYPE_PANE_POSITION      (moo_pane_position_get_type ())

typedef struct _MooPaned         MooPaned;
typedef struct _MooPanedPrivate  MooPanedPrivate;
typedef struct _MooPanedClass    MooPanedClass;

typedef enum {
    MOO_PANE_POS_LEFT = 0,
    MOO_PANE_POS_RIGHT,
    MOO_PANE_POS_TOP,
    MOO_PANE_POS_BOTTOM
} MooPanePosition;

struct _MooPaned
{
    GtkBin           bin;
    GtkWidget       *button_box;
    MooPanedPrivate *priv;
};

struct _MooPanedClass
{
    GtkBinClass bin_class;

    void (*open_pane) (MooPaned *paned,
                       guint     index);
    void (*hide_pane) (MooPaned *paned);
};


GType       moo_paned_get_type          (void) G_GNUC_CONST;
GType       moo_pane_position_get_type  (void) G_GNUC_CONST;

GtkWidget  *moo_paned_new               (MooPanePosition pane_position);

void        moo_paned_add_pane          (MooPaned   *paned,
                                         GtkWidget  *pane_widget,
                                         const char *button_label,
                                         const char *button_stock_id);
void        moo_paned_insert_pane       (MooPaned   *paned,
                                         GtkWidget  *pane_widget,
                                         GtkWidget  *button_widget,
                                         int         position);

guint       moo_paned_n_panes           (MooPaned   *paned);
GSList     *moo_paned_get_panes         (MooPaned   *paned);
GtkWidget  *moo_paned_get_nth_pane      (MooPaned   *paned,
                                         guint       n);

void        moo_paned_set_sticky_pane   (MooPaned   *paned,
                                         gboolean    sticky);

void        moo_paned_set_pane_size     (MooPaned   *paned,
                                         int         size);

int         moo_paned_get_open_pane     (MooPaned   *paned);
gboolean    moo_paned_is_open           (MooPaned   *paned);

void        moo_paned_open_pane         (MooPaned   *paned,
                                         guint       index);
void        moo_paned_hide_pane         (MooPaned   *paned);


G_END_DECLS

#endif /* MOOUTILS_MOOPANED_H */
