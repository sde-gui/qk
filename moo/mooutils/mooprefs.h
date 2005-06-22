/*
 *   mooutils/mooprefs.h
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

#ifndef MOOPREFS_MOOPREFS_H
#define MOOPREFS_MOOPREFS_H

#include <glib-object.h>
#include <gdk/gdkcolor.h>

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

    void (* changed)        (MooPrefs     *prefs,
                             const char   *key,
                             const char   *newval);
    void (* set)            (MooPrefs     *prefs,
                             const char   *key,
                             const char   *val);
    void (* unset)          (MooPrefs     *prefs,
                             const char   *key);
};


GType           moo_prefs_get_type              (void) G_GNUC_CONST;
GType           moo_prefs_match_type_get_type   (void) G_GNUC_CONST;

gboolean        moo_prefs_load              (const char     *file);
gboolean        moo_prefs_save              (const char     *file);

const gchar    *moo_prefs_get               (const char     *key);
void            moo_prefs_set               (const char     *key,
                                             const char     *val);
void            moo_prefs_set_ignore_change (const char     *key,
                                             const char     *val);


typedef void  (*MooPrefsNotify)             (const char     *key,
                                             const char     *newval,
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


gboolean        moo_prefs_get_bool          (const char     *key);
gdouble         moo_prefs_get_double        (const char     *key);
const GdkColor *moo_prefs_get_color         (const char     *key);
#define         moo_prefs_get_int(key)      ((int)moo_prefs_get_double (key))
int             moo_prefs_get_enum          (const char     *key,
                                             GType           type);

void            moo_prefs_set_if_not_set    (const char     *key,
                                             const char     *val);
void            moo_prefs_set_if_not_set_ignore_change
                                            (const char     *key,
                                             const char     *val);

void            moo_prefs_set_double        (const char     *key,
                                             double          val);
void            moo_prefs_set_double_ignore_change
                                            (const char     *key,
                                             double          val);
void            moo_prefs_set_double_if_not_set
                                            (const char     *key,
                                             double          val);
void            moo_prefs_set_double_if_not_set_ignore_change
                                            (const char     *key,
                                             double          val);

#define moo_prefs_set_int(key,val)                          \
    moo_prefs_set_double ((key), (double)(val))
#define moo_prefs_set_int_ignore_change(key,val)            \
    moo_prefs_set_double_ignore_change ((key), (double)(val))
#define moo_prefs_set_int_if_not_set(key,val)               \
    moo_prefs_set_double_if_not_set ((key), (double)(val))
#define moo_prefs_set_int_if_not_set_ignore_change(key,val) \
    moo_prefs_set_double_if_not_set_ignore_change ((key), (double)(val))

void            moo_prefs_set_bool          (const char     *key,
                                             gboolean        val);
void            moo_prefs_set_bool_ignore_change
                                            (const char     *key,
                                             gboolean        val);
void            moo_prefs_set_bool_if_not_set
                                            (const char     *key,
                                             gboolean        val);
void            moo_prefs_set_bool_if_not_set_ignore_change
                                            (const char     *key,
                                             gboolean        val);

void            moo_prefs_set_color         (const char     *key,
                                             const GdkColor *val);
void            moo_prefs_set_color_ignore_change
                                            (const char     *key,
                                             const GdkColor *val);
void            moo_prefs_set_color_if_not_set
                                            (const char     *key,
                                             const GdkColor *val);
void            moo_prefs_set_color_if_not_set_ignore_change
                                            (const char     *key,
                                             const GdkColor *val);


G_END_DECLS

#endif /* MOOPREFS_MOOPREFS_H */
