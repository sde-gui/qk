/*
 *   mootreeview.h
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

#ifndef MOO_FILE_VIEW_COMPILATION
#error "This file may not be included"
#endif

#ifndef __MOO_TREE_VIEW_H__
#define __MOO_TREE_VIEW_H__

#include <gtk/gtktreeview.h>
#include "moofileview/mooiconview.h"

G_BEGIN_DECLS


#define MOO_TYPE_TREE_VIEW              (_moo_tree_view_get_type ())
#define MOO_TREE_VIEW(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_TREE_VIEW, MooTreeView))
#define MOO_TREE_VIEW_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_TREE_VIEW, MooTreeViewClass))
#define MOO_IS_TREE_VIEW(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_TREE_VIEW))
#define MOO_IS_TREE_VIEW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_TREE_VIEW))
#define MOO_TREE_VIEW_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_TREE_VIEW, MooTreeViewClass))


typedef struct _MooTreeView         MooTreeView;
typedef struct _MooTreeViewClass    MooTreeViewClass;
typedef struct _MooTreeViewChild    MooTreeViewChild;
typedef struct _MooTreeViewTree     MooTreeViewTree;
typedef struct _MooTreeViewIcon     MooTreeViewIcon;

typedef enum {
    MOO_TREE_VIEW_TREE,
    MOO_TREE_VIEW_ICON
} MooTreeViewChildType;

struct _MooTreeViewTree
{
    GtkTreeView *view;
    GtkTreeSelection *selection;
};

struct _MooTreeViewIcon
{
    MooIconView *view;
};

struct _MooTreeViewChild
{
    MooTreeViewChildType type;
    MooTreeView *parent;
    GtkWidget *widget;
    union {
        MooTreeViewTree tree;
        MooTreeViewIcon icon;
    } u;
};

struct _MooTreeView
{
    GObject object;
    GSList *children;
    MooTreeViewChild *active;
    GtkTreeModel *model;
};

struct _MooTreeViewClass
{
    GObjectClass object_class;

    gboolean (* button_press_event) (MooTreeView        *view,
                                     GtkWidget          *widget,
                                     GdkEventButton     *event);
    gboolean (* key_press_event)    (MooTreeView        *view,
                                     GtkWidget          *widget,
                                     GdkEventKey        *event);

    void     (* row_activated)      (MooTreeView        *view,
                                     const GtkTreePath  *path);
    void     (* selection_changed)  (MooTreeView        *view);
};


typedef void (*MooTreeViewForeachFunc)  (GtkTreeModel       *model,
                                         GtkTreePath        *path,
                                         GtkTreeIter        *iter,
                                         gpointer            data);


GType           _moo_tree_view_get_type             (void) G_GNUC_CONST;

MooTreeView    *_moo_tree_view_new                  (GtkTreeModel   *model);

void            _moo_tree_view_add                  (MooTreeView    *view,
                                                     GtkWidget      *real_view);
void            _moo_tree_view_set_active           (MooTreeView    *view,
                                                     GtkWidget      *real_view);

GtkTreeModel   *_moo_tree_view_get_model            (gpointer        view);
void            _moo_tree_view_set_model            (MooTreeView    *view,
                                                     GtkTreeModel   *model);

gboolean        _moo_tree_view_selection_is_empty   (MooTreeView    *view);
gboolean        _moo_tree_view_path_is_selected     (MooTreeView    *view,
                                                     GtkTreePath    *path);
void            _moo_tree_view_unselect_all         (MooTreeView    *view);
GList          *_moo_tree_view_get_selected_rows    (MooTreeView    *view);
GtkTreePath    *_moo_tree_view_get_selected_path    (MooTreeView    *view);
void            _moo_tree_view_selected_foreach     (MooTreeView    *view,
                                                     MooTreeViewForeachFunc func,
                                                     gpointer        data);

/* window coordinates */
gboolean        _moo_tree_view_get_path_at_pos      (gpointer        view,
                                                     int             x,
                                                     int             y,
                                                     GtkTreePath   **path);

void            _moo_tree_view_widget_to_abs_coords (gpointer        view,
                                                     int             wx,
                                                     int             wy,
                                                     int            *absx,
                                                     int            *absy);
void            _moo_tree_view_abs_to_widget_coords (gpointer        view,
                                                     int             absx,
                                                     int             absy,
                                                     int            *wx,
                                                     int            *wy);

void            _moo_tree_view_set_cursor           (MooTreeView    *view,
                                                     GtkTreePath    *path,
                                                     gboolean        start_editing);
void            _moo_tree_view_scroll_to_cell       (MooTreeView    *view,
                                                     GtkTreePath    *path);

void            _moo_tree_view_set_drag_dest_row    (gpointer        view,
                                                     GtkTreePath    *path);


// void            _moo_tree_view_set_selection_mode   (MooTreeView    *view,
//                                                      GtkSelectionMode mode);
// gboolean        _moo_tree_view_get_selected         (MooTreeView    *view,
//                                                      GtkTreeIter    *iter);
// GtkTreePath    *_moo_tree_view_get_selected_path    (MooTreeView    *view);
// gint            _moo_tree_view_count_selected_rows  (MooTreeView    *view);
// void            _moo_tree_view_select_path          (MooTreeView    *view,
//                                                      GtkTreePath    *path);
// void            _moo_tree_view_unselect_path        (MooTreeView    *view,
//                                                      GtkTreePath    *path);
// void            _moo_tree_view_select_range         (MooTreeView    *view,
//                                                      GtkTreePath    *start,
//                                                      GtkTreePath    *end);
// gboolean        _moo_tree_view_path_is_selected     (MooTreeView    *view,
//                                                      GtkTreePath    *path);
// void            _moo_tree_view_select_all           (MooTreeView    *view);
// void            _moo_tree_view_unselect_all         (MooTreeView    *view);
//
// void            _moo_tree_view_scroll_to_cell       (MooTreeView    *view,
//                                                      GtkTreePath    *path);
// void            _moo_tree_view_set_cursor           (MooTreeView    *view,
//                                                      GtkTreePath    *path,
//                                                      gboolean        start_editing);
// GtkTreePath    *_moo_tree_view_get_cursor           (MooTreeView    *view);
// void            _moo_tree_view_row_activated        (MooTreeView    *view,
//                                                      GtkTreePath    *path);
//
// // gboolean        _moo_tree_view_get_path_at_pos      (MooTreeView    *view,
// //                                                      int             x,
// //                                                      int             y,
// //                                                      GtkTreePath   **path,
// //                                                      MooTreeViewCell *cell,
// //                                                      int            *cell_x,
// //                                                      int            *cell_y);
//
// void            _moo_tree_view_enable_drag_source   (MooTreeView    *view,
//                                                      GdkModifierType start_button_mask,
//                                                      GtkTargetEntry *targets,
//                                                      gint            n_targets,
//                                                      GdkDragAction   actions);
// GtkTargetList  *_moo_tree_view_get_source_targets   (MooTreeView    *view);
// void            _moo_tree_view_disable_drag_source  (MooTreeView    *view);
//
// void            _moo_tree_view_enable_drag_dest     (MooTreeView    *view,
//                                                      GtkTargetEntry *targets,
//                                                      gint            n_targets,
//                                                      GdkDragAction   actions);
// void            _moo_tree_view_set_dest_targets     (MooTreeView    *view,
//                                                      GtkTargetList  *targets);
// void            _moo_tree_view_disable_drag_dest    (MooTreeView    *view);
//
// void            _moo_tree_view_set_drag_dest_row    (MooTreeView    *view,
//                                                      GtkTreePath    *path);
// GtkTreePath    *_moo_tree_view_get_drag_dest_row    (MooTreeView    *view);


G_END_DECLS

#endif /* __MOO_TREE_VIEW_H__ */
