/*
 *   mooutils/mooiconview.h
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

#ifndef MOOUTILS_MOOICONVIEW_H
#define MOOUTILS_MOOICONVIEW_H

#include <gtk/gtk.h>

G_BEGIN_DECLS


#define MOO_TYPE_ICON_VIEW              (moo_icon_view_get_type ())
#define MOO_ICON_VIEW(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_ICON_VIEW, MooIconView))
#define MOO_ICON_VIEW_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_ICON_VIEW, MooIconViewClass))
#define MOO_IS_ICON_VIEW(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_ICON_VIEW))
#define MOO_IS_ICON_VIEW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_ICON_VIEW))
#define MOO_ICON_VIEW_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_ICON_VIEW, MooIconViewClass))


typedef struct _MooIconView         MooIconView;
typedef struct _MooIconViewPrivate  MooIconViewPrivate;
typedef struct _MooIconViewClass    MooIconViewClass;

struct _MooIconView
{
    GtkVBox             vbox;
    MooIconViewPrivate *priv;
};

struct _MooIconViewClass
{
    GtkVBoxClass        vbox_class;

    void    (*set_scroll_adjustments)   (GtkWidget      *widget,
                                         GtkAdjustment  *hadjustment,
                                         GtkAdjustment  *vadjustment);

    void    (*item_activated)           (MooIconView    *iconview,
                                         GtkTreePath    *path);
};

typedef enum {
    MOO_ICON_VIEW_CELL_PIXBUF,
    MOO_ICON_VIEW_CELL_TEXT
} MooIconViewCell;


typedef void (* MooIconCellDataFunc)        (MooIconView        *view,
                                             GtkCellRenderer    *cell,
                                             GtkTreeModel       *model,
                                             GtkTreeIter        *iter,
                                             gpointer            data);


GType         moo_icon_view_get_type          (void) G_GNUC_CONST;

GtkWidget    *moo_icon_view_new               (void);
GtkWidget    *moo_icon_view_new_with_model    (GtkTreeModel *model);

GtkTreeModel *moo_icon_view_get_model         (MooIconView    *view);
void          moo_icon_view_set_model         (MooIconView    *view,
                                               GtkTreeModel   *model);

void          moo_icon_view_set_cell          (MooIconView    *view,
                                               MooIconViewCell cell_type,
                                               GtkCellRenderer*cell);
GtkCellRenderer *moo_icon_view_get_cell       (MooIconView    *view,
                                               MooIconViewCell cell_type);

void          moo_icon_view_set_attributes    (MooIconView    *view,
                                               MooIconViewCell cell_type,
                                               const char     *first_attr,
                                               ...);
void          moo_icon_view_clear_attributes  (MooIconView    *view,
                                               MooIconViewCell cell_type);

void          moo_icon_view_set_cell_data_func(MooIconView    *view,
                                               MooIconViewCell cell,
                                               MooIconCellDataFunc func,
                                               gpointer        func_data,
                                               GDestroyNotify  destroy);

void          moo_icon_view_set_adjustment    (MooIconView    *view,
                                               GtkAdjustment  *adjustment);

void          moo_icon_view_select_path       (MooIconView    *view,
                                               GtkTreePath    *path);
void          moo_icon_view_select_iter       (MooIconView    *view,
                                               GtkTreeIter    *iter);
gboolean      moo_icon_view_path_is_selected  (MooIconView    *view,
                                               GtkTreePath    *path);
GtkTreePath  *moo_icon_view_get_path          (MooIconView    *view,
                                               int             window_x,
                                               int             window_y);
GtkTreePath  *moo_icon_view_get_selected      (MooIconView    *view);
void          moo_icon_view_activate_selected (MooIconView    *view);
void          moo_icon_view_move_cursor       (MooIconView    *view,
                                               GtkTreePath    *path);


G_END_DECLS

#endif /* MOOUTILS_MOOICONVIEW_H */
