/*
 *   mooutils-misc.h
 *
 *   Copyright (C) 2004-2010 by Yevgen Muntyan <emuntyan@users.sourceforge.net>
 *
 *   This file is part of medit.  medit is free software; you can
 *   redistribute it and/or modify it under the terms of the
 *   GNU Lesser General Public License as published by the
 *   Free Software Foundation; either version 2.1 of the License,
 *   or (at your option) any later version.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with medit.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MOO_UTILS_MISC_H
#define MOO_UTILS_MISC_H

#include <gtk/gtk.h>
#include <mooutils/mooutils-messages.h>

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

void MOO_NORETURN moo_segfault              (void);
void MOO_NORETURN moo_abort                 (void);

static inline void
seriously_ignore_return_value (G_GNUC_UNUSED int v)
{
    /* gcc guys are funny, casting to void is not enough */
}

static inline void
seriously_ignore_return_value_p (G_GNUC_UNUSED void *p)
{
}

void        moo_disable_win32_error_message (void);
void        moo_enable_win32_error_message  (void);

void       _moo_set_app_instance_name       (const char     *name);
void        moo_set_user_data_dir           (const char     *path);
void        moo_set_display_app_name        (const char     *name);
const char *moo_get_display_app_name        (void);

gboolean    moo_make_user_data_dir          (const char     *path);
char       *moo_get_user_data_dir           (void);
char       *moo_get_user_data_file          (const char     *basename);
char       *moo_get_named_user_data_file    (const char     *basename);
gboolean    moo_save_user_data_file         (const char     *basename,
                                             const char     *content,
                                             gssize          len,
                                             GError        **error);
char       *moo_get_user_cache_dir          (void);
char       *moo_get_user_cache_file         (const char     *basename);
gboolean    moo_save_user_cache_file        (const char     *basename,
                                             const char     *content,
                                             gssize          len,
                                             GError        **error);
gboolean    moo_save_config_file            (const char     *filename,
                                             const char     *content,
                                             gssize          len,
                                             GError        **error);

/* user data comes first */
char      **moo_get_data_dirs               (void);
char      **moo_get_lib_dirs                (void);
char      **moo_get_data_subdirs            (const char     *subdir);
char      **moo_get_sys_data_subdirs        (const char     *subdir);
char      **moo_get_lib_subdirs             (const char     *subdir);
char      **moo_get_data_and_lib_subdirs    (const char     *subdir);

#define moo_get_data_files moo_get_data_subdirs
#define moo_get_sys_data_files moo_get_sys_data_subdirs

gboolean    moo_getenv_bool                 (const char     *var);

char       *moo_tempnam                     (void);
void        moo_cleanup                     (void);

const char *moo_get_locale_dir              (void);

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

typedef struct {
    const char *text;
    gsize len;
} MooLineReader;
void        moo_line_reader_init            (MooLineReader  *lr,
                                             const char     *text,
                                             gssize          len);
const char *moo_line_reader_get_line        (MooLineReader  *lr,
                                             gsize          *line_len,
                                             gsize          *lt_len);
gboolean    moo_find_line_end               (const char     *string,
                                             gssize          len,
                                             gsize          *le_start,
                                             gsize          *le_len);
char      **moo_strnsplit_lines             (const char     *string,
                                             gssize          len,
                                             guint          *n_tokens);
char      **moo_splitlines                  (const char     *string);

char     **_moo_strv_reverse                (char          **str_array);

gboolean   _moo_str_equal                   (const char     *s1,
                                             const char     *s2);

static inline void
moo_assign_string (char       **where,
                   const char  *value)
{
    char *tmp = *where;
    *where = g_strdup (value);
    g_free (tmp);
}

#define MOO_ASSIGN_STRING(where, value) moo_assign_string (&(where), (value))

static inline void
moo_assign_strv (char ***where,
                 char  **value)
{
    char **tmp = *where;
    *where = g_strdupv (value);
    g_strfreev (tmp);
}

#define MOO_ASSIGN_STRV(where, value) moo_assign_strv (&(where), (value))

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
# define moo_alloc_block(sz)   g_slice_alloc (sz)
# define moo_alloc0_block(sz)  g_slice_alloc0 (sz)
# define moo_free_block(sz,p)  g_slice_free1 (sz, p)
#else
# define moo_alloc_block(sz)   g_malloc (sz)
# define moo_alloc0_block(sz)  g_malloc0 (sz)
# define moo_free_block(sz,p)  g_free (p)
#endif

#ifdef g_slice_dup
# define moo_dup_block(sz,p)   g_slice_copy (sz, p)
#else
# define moo_dup_block(sz,p)   g_memdup (p, sz)
#endif

#define moo_new_n(type, n)      ((type*) moo_alloc_block (sizeof (type) * (n)))
#define moo_new0_n(type, n)     ((type*) moo_alloc0_block (sizeof (type) * (n)))
#define moo_free_n(type, n, p)  do { type *p__ = p; moo_free_block (sizeof (type) * n, p__); } while (0)

#define moo_new(type)           moo_new_n (type, 1)
#define moo_new0(type)          moo_new0_n (type, 1)
#define moo_free(type,p)        moo_free_n (type, 1, p)

#if defined(MOO_CL_GCC)
#define moo_obj_dup(type,p)     ({ type const *cp__ = p; type *p__ = (type*) moo_dup_block (sizeof (type), cp__); p__; })
#else
#define moo_obj_dup(type, p)    ((type*) moo_dup_block (sizeof (type), p))
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

int         _moo_win32_message_box          (GtkWidget      *parent,
                                             guint           type,
                                             const char     *title,
                                             const char     *format,
                                             ...) G_GNUC_PRINTF (4, 5);


G_END_DECLS

#endif /* G_OS_WIN32 */

#endif /* MOO_UTILS_MISC_H */
