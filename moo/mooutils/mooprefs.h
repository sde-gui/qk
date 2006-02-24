/*
 *   mooprefs.h
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

#ifndef __MOO_PREFS_H__
#define __MOO_PREFS_H__

#include <glib-object.h>
#include <gdk/gdkcolor.h>
#include <mooutils/moomarkup.h>

G_BEGIN_DECLS


#define MOO_TYPE_PREFS              (moo_prefs_get_type ())
#define MOO_TYPE_PREFS_MATCH_TYPE   (moo_prefs_match_type_get_type ())

#define MOO_PREFS(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_PREFS, MooPrefs))
#define MOO_PREFS_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_PREFS, MooPrefsClass))
#define MOO_IS_PREFS(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_PREFS))
#define MOO_IS_PREFS_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_PREFS))
#define MOO_PREFS_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_PREFS, MooPrefsClass))

typedef struct _MooPrefs        MooPrefs;
typedef struct _MooPrefsPrivate MooPrefsPrivate;
typedef struct _MooPrefsClass   MooPrefsClass;

struct _MooPrefs
{
    GObject          gobject;

    MooPrefsPrivate *priv;
};

struct _MooPrefsClass
{
    GObjectClass   parent_class;
};


GType           moo_prefs_get_type          (void) G_GNUC_CONST;
GType           moo_prefs_match_type_get_type (void) G_GNUC_CONST;

gboolean        moo_prefs_load          (const char     *file,
                                         GError        **error);
gboolean        moo_prefs_save          (const char     *file,
                                         GError        **error);

MooMarkupDoc   *moo_prefs_get_markup    (void);

void            moo_prefs_new_key       (const char     *key,
                                         GType           value_type,
                                         const GValue   *default_value);
void            moo_prefs_delete_key    (const char     *key);

char          **moo_prefs_list_keys     (guint          *n_keys);
GType           moo_prefs_get_key_type  (const char     *key);
gboolean        moo_prefs_key_registered(const char     *key);

const GValue   *moo_prefs_get           (const char     *key);
const GValue   *moo_prefs_get_default   (const char     *key);
void            moo_prefs_set           (const char     *key,
                                         const GValue   *value);
void            moo_prefs_set_default   (const char     *key,
                                         const GValue   *value);

void            moo_prefs_new_key_bool  (const char     *key,
                                         gboolean        default_val);
void            moo_prefs_new_key_int   (const char     *key,
                                         int             default_val);
void            moo_prefs_new_key_string(const char     *key,
                                         const char     *default_val);
void            moo_prefs_new_key_color (const char     *key,
                                         const GdkColor *default_val);
void            moo_prefs_new_key_enum  (const char     *key,
                                         GType           enum_type,
                                         int             default_val);

char           *moo_prefs_make_key      (const char     *first_comp,
                                         ...);
char           *moo_prefs_make_keyv     (const char     *first_comp,
                                         va_list         var_args);

const char     *moo_prefs_get_string    (const char     *key);
const char     *moo_prefs_get_filename  (const char     *key);
gboolean        moo_prefs_get_bool      (const char     *key);
gdouble         moo_prefs_get_number    (const char     *key);
const GdkColor *moo_prefs_get_color     (const char     *key);
int             moo_prefs_get_int       (const char     *key);
int             moo_prefs_get_enum      (const char     *key);

void            moo_prefs_set_string    (const char     *key,
                                         const char     *val);
void            moo_prefs_set_filename  (const char     *key,
                                         const char     *val);
void            moo_prefs_set_number    (const char     *key,
                                         double          val);
void            moo_prefs_set_int       (const char     *key,
                                         int             val);
void            moo_prefs_set_bool      (const char     *key,
                                         gboolean        val);
void            moo_prefs_set_color     (const char     *key,
                                         const GdkColor *val);
void            moo_prefs_set_enum      (const char     *key,
                                         int             val);

typedef void  (*MooPrefsNotify)         (const char     *key,
                                         const GValue   *newval,
                                         gpointer        data);

typedef enum
{
    MOO_PREFS_MATCH_KEY       = 1 << 0,
    MOO_PREFS_MATCH_PREFIX    = 1 << 1,
    MOO_PREFS_MATCH_REGEX     = 1 << 2
} MooPrefsMatchType;

guint           moo_prefs_notify_connect    (const char     *pattern,
                                             MooPrefsMatchType match_type,
                                             MooPrefsNotify  callback,
                                             gpointer        data);
gboolean        moo_prefs_notify_block      (guint           id);
gboolean        moo_prefs_notify_unblock    (guint           id);
gboolean        moo_prefs_notify_disconnect (guint           id);


G_END_DECLS

#endif /* __MOO_PREFS_H__ */
