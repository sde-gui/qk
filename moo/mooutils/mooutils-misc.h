/*
 *   mooutils-misc.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_UTILS_MISC_H
#define MOO_UTILS_MISC_H

#include <gtk/gtk.h>
#include <string.h>

G_BEGIN_DECLS


gboolean    moo_open_url                    (const char *url);
gboolean    moo_open_email                  (const char *address,
                                             const char *subject,
                                             const char *body);
gboolean    moo_open_file                   (const char *path);
char       *_moo_find_script                (const char *name,
                                             gboolean    search_path);

void        moo_window_present              (GtkWindow  *window,
                                             guint32     stamp);
GtkWindow  *_moo_get_top_window             (GSList     *windows);

void        _moo_window_set_icon_from_stock (GtkWindow  *window,
                                             const char *name);

void        moo_log_window_show             (void);
void        moo_log_window_hide             (void);

void        moo_set_log_func_window         (gboolean        show_now);
void        moo_set_log_func_file           (const char     *log_file);
void        moo_set_log_func_silent         (void);
void        moo_reset_log_func              (void);

void        moo_segfault                    (void);

void        moo_disable_win32_error_message (void);
void        moo_enable_win32_error_message  (void);

#define MOO_TYPE_DATA_DIR_TYPE              (moo_data_dir_type_get_type ())

typedef enum {
    MOO_DATA_SHARE,
    MOO_DATA_LIB
} MooDataDirType;

GType       moo_data_dir_type_get_type      (void) G_GNUC_CONST;

void       _moo_set_app_instance_name       (const char     *name);
void       _moo_set_user_data_dir           (const char     *path);

/* ~/.appname */
gboolean    moo_make_user_data_dir          (const char     *path);
char       *moo_get_user_data_dir           (void);
char       *moo_get_user_data_file          (const char     *basename);
char       *moo_get_named_user_data_file    (const char     *basename);
gboolean    moo_save_user_data_file         (const char     *basename,
                                             const char     *content,
                                             gssize          len,
                                             GError        **error);
char       *moo_get_user_cache_file         (const char     *basename);
gboolean    moo_save_user_cache_file        (const char     *basename,
                                             const char     *content,
                                             gssize          len,
                                             GError        **error);

/* user data comes first; MOO_DATA_DIR comes last */
/* $MOO_APP_DIR:$MOO_DATA_DIRS:$prefix/share/appname or
   $MOO_APP_DIR:$MOO_LIB_DIRS:$prefix/lib/appname */
char      **moo_get_data_dirs               (MooDataDirType  type,
                                             guint          *n_dirs);
char      **moo_get_data_subdirs            (const char     *subdir,
                                             MooDataDirType  type,
                                             guint          *n_dirs);
#define moo_get_data_files moo_get_data_subdirs

char       *moo_tempnam                     (void);
void        moo_cleanup                     (void);

const char *moo_get_locale_dir              (void);
const char *const *_moo_get_shared_data_dirs (void);


void        moo_selection_data_set_pointer  (GtkSelectionData *data,
                                             GdkAtom         type,
                                             gpointer        ptr);
gpointer    moo_selection_data_get_pointer  (GtkSelectionData *data,
                                             GdkAtom         type);

GdkModifierType _moo_get_modifiers          (GtkWidget      *widget);

void       _moo_menu_item_set_accel_label   (GtkWidget      *menu_item,
                                             const char     *label);
void       _moo_menu_item_set_label         (GtkWidget      *menu_item,
                                             const char     *label,
                                             gboolean        mnemonic);

void       _moo_widget_set_tooltip          (GtkWidget      *widget,
                                             const char     *tip);

char      **moo_strnsplit_lines             (const char     *string,
                                             gssize          len,
                                             guint          *n_tokens);
char      **moo_splitlines                  (const char     *string);
char     **_moo_ascii_strnsplit             (const char     *string,
                                             gssize          len,
                                             guint           n_tokens);
char     **_moo_strv_reverse                (char          **str_array);

static inline gboolean
_moo_str_equal_inline (const char *s1,
                       const char *s2)
{
    return !strcmp (s1 ? s1 : "", s2 ? s2 : "");
}

#define _moo_str_equal(s1, s2) (_moo_str_equal_inline ((s1), (s2)))


static inline void
moo_assign_string (char       **where,
                   const char  *value)
{
    char *tmp = *where;
    *where = g_strdup (value);
    g_free (tmp);
}

#define MOO_ASSIGN_STRING(where, value) moo_assign_string (&(where), (value))

const char *_moo_get_pid_string             (void);

const char *_moo_intern_string              (const char     *string);

guint        moo_idle_add_full              (gint            priority,
                                             GSourceFunc     function,
                                             gpointer        data,
                                             GDestroyNotify  notify);
guint        moo_idle_add                   (GSourceFunc     function,
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


gboolean    _moo_regex_escape               (const char *string,
                                             int         bytes,
                                             GString    *dest);


#if GLIB_CHECK_VERSION(2,10,0)
#define _moo_new            g_slice_new
#define _moo_new0           g_slice_new0
#define _moo_free           g_slice_free
#else
#define _moo_new(type)      g_new (type, 1)
#define _moo_new0(type)     g_new0 (type, 1)
#define _moo_free(type,mem) g_free (mem)
#endif

#if GLIB_CHECK_VERSION(2,10,0)
#define MOO_OBJECT_REF_SINK(obj) g_object_ref_sink (obj)
#else
#define MOO_OBJECT_REF_SINK(obj) gtk_object_sink (g_object_ref (obj))
#endif


G_END_DECLS


#ifdef G_OS_WIN32
#include <gtk/gtk.h>
#include <string.h>

G_BEGIN_DECLS


char        *moo_win32_get_app_dir          (void);
char        *moo_win32_get_dll_dir          (const char     *dll);

void        _moo_win32_add_data_dirs        (GPtrArray      *list,
                                             const char     *prefix);

const char *_moo_win32_get_locale_dir       (void);

gboolean    _moo_win32_open_uri             (const char     *uri);
void        _moo_win32_show_fatal_error     (const char     *domain,
                                             const char     *logmsg);

char      **_moo_win32_lame_parse_cmd_line  (const char     *cmd_line,
                                             GError        **error);


G_END_DECLS

#endif /* G_OS_WIN32 */

#endif /* MOO_UTILS_MISC_H */
