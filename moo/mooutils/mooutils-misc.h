/*
 *   mooutils-misc.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
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

#include <gtk/gtk.h>
#include <string.h>

G_BEGIN_DECLS


gboolean    moo_open_url                    (const char *url);
gboolean    moo_open_email                  (const char *address,
                                             const char *subject,
                                             const char *body);
gboolean    _moo_open_file                  (const char *path);

void        moo_window_present              (GtkWindow  *window,
                                             guint32     stamp);
GtkWindow  *_moo_get_top_window             (GSList     *windows);

gboolean    _moo_window_set_icon_from_stock (GtkWindow  *window,
                                             const char *stock_id);

void        moo_log_window_show             (void);
void        moo_log_window_hide             (void);

void        moo_set_log_func_window         (gboolean        show_now);
void        moo_set_log_func_file           (const char     *log_file);
void        moo_set_log_func_silent         (void);
void        moo_reset_log_func              (void);

void        moo_segfault                    (void);
void        _moo_message                    (const char     *format,
                                             ...);

void        moo_disable_win32_error_message (void);
void        moo_enable_win32_error_message  (void);

#define MOO_TYPE_DATA_DIR_TYPE              (moo_data_dir_type_get_type ())

typedef enum {
    MOO_DATA_SHARE,
    MOO_DATA_LIB
} MooDataDirType;

GType       moo_data_dir_type_get_type      (void) G_GNUC_CONST;

/* ~/.appname */
gboolean    moo_make_user_data_dir          (const char     *path);
char       *moo_get_user_data_dir           (void);
char       *moo_get_user_data_file          (const char     *basename);
gboolean    moo_save_user_data_file         (const char     *basename,
                                             const char     *content,
                                             gssize          len,
                                             GError        **error);

/* user data comes last; MOO_DATA_DIR comes first */
/* $MOO_APP_DIR:$MOO_DATA_DIRS:$prefix/share/appname or
   $MOO_APP_DIR:$MOO_LIB_DIRS:$prefix/lib/appname */
char      **moo_get_data_dirs               (MooDataDirType  type,
                                             guint          *n_dirs);
char      **moo_get_data_subdirs            (const char     *subdir,
                                             MooDataDirType  type,
                                             guint          *n_dirs);
#define moo_get_data_files moo_get_data_subdirs


void        _moo_selection_data_set_pointer (GtkSelectionData *data,
                                             GdkAtom         type,
                                             gpointer        ptr);
gpointer    _moo_selection_data_get_pointer (GtkSelectionData *data,
                                             GdkAtom         type);

GdkModifierType _moo_get_modifiers          (GtkWidget      *widget);

void        _moo_menu_item_set_accel_label  (GtkWidget      *menu_item,
                                             const char     *label);
void        _moo_menu_item_set_label        (GtkWidget      *menu_item,
                                             const char     *label,
                                             gboolean        mnemonic);

void        _moo_widget_set_tooltip         (GtkWidget      *widget,
                                             const char     *tip);

char      **_moo_splitlines                 (const char     *string);

static inline gboolean
_moo_str_equal_inline (const char *s1,
                       const char *s2)
{
    return !strcmp (s1 ? s1 : "", s2 ? s2 : "");
}

#define _moo_str_equal(s1, s2) (_moo_str_equal_inline ((s1), (s2)))


const char *_moo_get_pid_string             (void);

guint       _moo_idle_add_full              (gint            priority,
                                             GSourceFunc     function,
                                             gpointer        data,
                                             GDestroyNotify  notify);
guint       _moo_idle_add                   (GSourceFunc     function,
                                             gpointer        data);
guint       _moo_timeout_add_full           (gint            priority,
                                             guint           interval,
                                             GSourceFunc     function,
                                             gpointer        data,
                                             GDestroyNotify  notify);
guint       _moo_timeout_add                (guint           interval,
                                             GSourceFunc     function,
                                             gpointer        data);
guint       _moo_io_add_watch               (GIOChannel     *channel,
                                             GIOCondition    condition,
                                             GIOFunc         func,
                                             gpointer        data);
guint       _moo_io_add_watch_full          (GIOChannel     *channel,
                                             int             priority,
                                             GIOCondition    condition,
                                             GIOFunc         func,
                                             gpointer        data,
                                             GDestroyNotify  notify);


G_END_DECLS

#endif /* __MOO_UTILS_MISC_H__ */
