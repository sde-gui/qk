/*
 *   mooutils-treeview.h
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

#ifndef __MOO_UTILS_TREE_VIEW_H__
#define __MOO_UTILS_TREE_VIEW_H__

#include <mooutils/mooconfig.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS


typedef struct _MooTreeHelper MooTreeHelper;
typedef struct _MooConfigHelper MooConfigHelper;
typedef struct _MooTreeHelperClass MooTreeHelperClass;
typedef struct _MooConfigHelperClass MooConfigHelperClass;

struct _MooTreeHelperClass {
    GtkObjectClass parent_class;

    gboolean    (*new_row)          (MooTreeHelper  *helper,
                                     GtkTreeModel   *model,
                                     GtkTreePath    *path);
    gboolean    (*delete_row)       (MooTreeHelper  *helper,
                                     GtkTreeModel   *model,
                                     GtkTreePath    *path);
    gboolean    (*move_row)         (MooTreeHelper  *helper,
                                     GtkTreeModel   *model,
                                     GtkTreePath    *old_path,
                                     GtkTreePath    *new_path);

    void        (*update_widgets)   (MooTreeHelper  *helper,
                                     GtkTreeModel   *model,
                                     GtkTreePath    *path,
                                     GtkTreeIter    *iter);
    void        (*update_model)     (MooTreeHelper  *helper,
                                     GtkTreeModel   *model,
                                     GtkTreePath    *path,
                                     GtkTreeIter    *iter);
};

struct _MooConfigHelperClass {
    MooTreeHelperClass parent_class;

    void        (*new_item)         (MooConfigHelper    *helper,
                                     MooConfig          *config,
                                     MooConfigItem      *item);
    void        (*set_from_item)    (MooConfigHelper    *helper,
                                     MooConfig          *config,
                                     MooConfigItem      *item);
    void        (*set_from_widgets) (MooConfigHelper    *helper,
                                     MooConfig          *config,
                                     MooConfigItem      *item);
};


MooTreeHelper   *moo_tree_helper_new                (GtkWidget          *treeview_or_combo,
                                                     GtkWidget          *new_btn,
                                                     GtkWidget          *delete_btn,
                                                     GtkWidget          *up_btn,
                                                     GtkWidget          *down_btn);
void             moo_tree_helper_update_model       (MooTreeHelper      *helper,
                                                     GtkTreeModel       *model,
                                                     GtkTreePath        *path);
void             moo_tree_helper_update_widgets     (MooTreeHelper      *helper);

MooConfigHelper *moo_config_helper_new              (GtkWidget          *tree_view,
                                                     GtkWidget          *new_btn,
                                                     GtkWidget          *delete_btn,
                                                     GtkWidget          *up_btn,
                                                     GtkWidget          *down_btn);

void             moo_config_helper_add_widget       (MooConfigHelper    *helper,
                                                     GtkWidget          *widget,
                                                     const char         *key,
                                                     gboolean            update_live);
void             moo_config_helper_update_model     (MooConfigHelper    *helper,
                                                     GtkTreeModel       *model,
                                                     GtkTreePath        *path);
void             moo_config_helper_update_widgets   (MooConfigHelper    *helper);


void             moo_tree_view_select_first         (GtkTreeView        *tree_view);
void             moo_combo_box_select_first         (GtkComboBox        *combo);


G_END_DECLS

#endif /* __MOO_UTILS_TREE_VIEW_H__ */
