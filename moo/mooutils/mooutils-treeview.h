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


typedef void (*MooConfigSetupItemFunc)      (MooConfig     *config,
                                             MooConfigItem *item,
                                             gpointer       data);
typedef void (*MooConfigWidgetToItem)       (GtkWidget     *widget,
                                             MooConfig     *config,
                                             MooConfigItem *item,
                                             gpointer       data);
typedef void (*MooConfigItemToWidget)       (MooConfigItem *item,
                                             GtkWidget     *widget,
                                             gpointer       data);

void            moo_config_connect_widget   (GtkWidget      *tree_view,
                                             GtkWidget      *new_btn,
                                             GtkWidget      *delete_btn,
                                             GtkWidget      *up_btn,
                                             GtkWidget      *down_btn,
                                             MooConfigSetupItemFunc func,
                                             gpointer        data);
void            moo_config_add_widget       (GtkWidget      *tree_view,
                                             GtkWidget      *widget,
                                             const char     *key,
                                             gboolean        update_live,
                                             gboolean        default_bool);
void            moo_config_add_widget_full  (GtkWidget      *tree_view,
                                             GtkWidget      *widget,
                                             const char     *key,
                                             MooConfigWidgetToItem widget_to_item_func,
                                             MooConfigItemToWidget item_to_widget_func,
                                             gpointer        data,
                                             gboolean        update_live,
                                             gboolean        default_bool);
void            moo_config_disconnect_widget(GtkWidget      *tree_view);
void            moo_config_update_tree_view (GtkWidget      *tree_view,
                                             GtkTreeModel   *model,
                                             GtkTreePath    *path);
void            moo_config_update_widgets   (GtkWidget      *tree_view);


void            moo_tree_view_select_first  (GtkTreeView    *tree_view);


G_END_DECLS

#endif /* __MOO_UTILS_TREE_VIEW_H__ */
