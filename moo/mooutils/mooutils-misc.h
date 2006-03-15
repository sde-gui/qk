/*
 *   mooutils-misc.h
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

#ifndef __MOO_UTILS_MISC_H__
#define __MOO_UTILS_MISC_H__

#include <gtk/gtkwindow.h>

G_BEGIN_DECLS


gboolean    moo_save_file_utf8              (const char *name,
                                             const char *text,
                                             gssize      len,
                                             GError    **error);
int         moo_unlink                      (const char *filename);
int         moo_mkdir                       (const char *path);
gboolean    moo_rmdir                       (const char *path,
                                             gboolean    recursive);

gboolean    moo_open_url                    (const char *url);
gboolean    moo_open_email                  (const char *address,
                                             const char *subject,
                                             const char *body);


gboolean    moo_window_is_hidden            (GtkWindow  *window);
void        moo_window_present              (GtkWindow  *window);
GtkWindow  *moo_get_top_window              (GSList     *windows);
GtkWindow  *moo_get_toplevel_window         (void);

gboolean    moo_window_set_icon_from_stock  (GtkWindow  *window,
                                             const char *stock_id);


void        moo_log_window_show             (void);
void        moo_log_window_hide             (void);

void        moo_print                       (const char     *string);
void        moo_print_err                   (const char     *string);

void        moo_set_log_func_window         (gboolean        show_now);
void        moo_set_log_func_file           (const char     *log_file);
void        moo_set_log_func_silent         (void);
void        moo_reset_log_func              (void);

void        moo_segfault                    (void);


void        moo_selection_data_set_pointer  (GtkSelectionData *data,
                                             GdkAtom         type,
                                             gpointer        ptr);
gpointer    moo_selection_data_get_pointer  (GtkSelectionData *data,
                                             GdkAtom         type);

GdkModifierType moo_get_modifiers           (GtkWidget      *widget);

void        moo_menu_item_set_accel_label   (GtkWidget      *menu_item,
                                             const char     *label);
void        moo_menu_item_set_label         (GtkWidget      *menu_item,
                                             const char     *label,
                                             gboolean        mnemonic);


G_END_DECLS

#endif /* __MOO_UTILS_MISC_H__ */
