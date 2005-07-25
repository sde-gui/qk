/*
 *   mooutils/mooprefs.c
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mooutils/mooprefs.h"
#include "mooutils/moocompat.h"
#include "mooutils/eggregex.h"
#include "mooutils/moomarshals.h"
#include <string.h>


static MooPrefs *instance (void)
{
    static MooPrefs *p = NULL;
    if (!p)
        p = MOO_PREFS (g_object_new (MOO_TYPE_PREFS, NULL));
    return p;
}


/* MOO_TYPE_PREFS */
G_DEFINE_TYPE (MooPrefs, moo_prefs, G_TYPE_OBJECT)


typedef struct {
    char *value;
    guint changed : 1;
} Item;

static Item     *item_new       (const char         *val,
                                 gboolean            changed)
{
    Item *item = g_new (Item, 1);
    item->value = g_strdup (val);
    item->changed = changed;
    return item;
}

static void      item_free      (Item               *item)
{
    if (!item) return;
    g_free (item->value);
    g_free (item);
}


typedef union {
    char        *key;
    EggRegex    *regex;
} Pattern;

static void      pattern_free   (Pattern             p,
                                 MooPrefsMatchType   type);


typedef struct {
    guint               id;
    MooPrefsMatchType   type;
    Pattern             pattern;
    guint               prefix_len;
    MooPrefsNotify      callback;
    gpointer            data;
    guint               blocked : 1;
} Closure;

static Closure  *closure_new    (MooPrefs           *prefs,
                                 const char         *pattern,
                                 MooPrefsMatchType   match_type,
                                 MooPrefsNotify      callback,
                                 gpointer            data);
static void      closure_free   (Closure            *closure);
static gboolean  closure_match  (Closure            *closure,
                                 const char         *key);
static void      closure_invoke (Closure            *closure,
                                 const char         *key,
                                 const char         *value);


struct _MooPrefsPrivate {
    GHashTable  *data;              /* char* -> Item* */
    guint        last_notify_id;
    GList       *closures;
    GHashTable  *closures_map;      /* guint -> closures list link */
};


static void      prefs_set      (MooPrefs   *prefs,
                                 const char *key,
                                 const char *val,
                                 gboolean    changed);

static Item     *prefs_get      (MooPrefs   *prefs,
                                 const char *key);

static void      prefs_change   (MooPrefs   *prefs,
                                 const char *key,
                                 Item       *item,
                                 const char *val,
                                 gboolean    changed);
static void      prefs_create   (MooPrefs   *prefs,
                                 const char *key,
                                 const char *val,
                                 gboolean    changed);
static void      prefs_remove   (MooPrefs   *prefs,
                                 const char *key);

static void      emit_notify    (MooPrefs   *prefs,
                                 const char *key,
                                 const char *val);


static void      moo_prefs_finalize (GObject *object);

enum {
    CHANGED,
    SET,
    UNSET,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];


static void moo_prefs_class_init (MooPrefsClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = moo_prefs_finalize;

    signals[CHANGED] =
            g_signal_new ("changed",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooPrefsClass, changed),
                          NULL, NULL,
                          _moo_marshal_VOID__STRING_STRING,
                          G_TYPE_NONE, 2,
                          G_TYPE_STRING, G_TYPE_STRING);

    signals[SET] =
            g_signal_new ("set",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooPrefsClass, changed),
                          NULL, NULL,
                          _moo_marshal_VOID__STRING_STRING,
                          G_TYPE_NONE, 2,
                          G_TYPE_STRING, G_TYPE_STRING);

    signals[UNSET] =
            g_signal_new ("unset",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooPrefsClass, changed),
                          NULL, NULL,
                          _moo_marshal_VOID__STRING,
                          G_TYPE_NONE, 1,
                          G_TYPE_STRING);
}


static void moo_prefs_init (MooPrefs *prefs)
{
    prefs->priv = g_new0 (MooPrefsPrivate, 1);

    prefs->priv->data = g_hash_table_new (g_str_hash, g_str_equal);
    prefs->priv->last_notify_id = 0;
    prefs->priv->closures = NULL;
    prefs->priv->closures_map = g_hash_table_new (g_direct_hash, NULL);
}


static void free_key_and_item (char *key,
                               Item *item)
{
    g_free (key);
    item_free (item);
}


static void moo_prefs_finalize (GObject *obj)
{
    MooPrefs *prefs = MOO_PREFS (obj);

    g_hash_table_foreach (prefs->priv->data,
                          (GHFunc) free_key_and_item,
                          NULL);
    g_hash_table_destroy (prefs->priv->data);

    g_hash_table_destroy (prefs->priv->closures_map);

    g_list_foreach (prefs->priv->closures,
                    (GFunc) closure_free,
                    NULL);
    g_list_free (prefs->priv->closures);
}


static void      prefs_set      (MooPrefs   *prefs,
                                 const char *key,
                                 const char *val,
                                 gboolean    changed)
{
    g_return_if_fail (MOO_IS_PREFS (prefs));
    g_return_if_fail (key != NULL);

    if (!val)
    {
        prefs_remove (prefs, key);
    }
    else
    {
        Item *item = prefs_get (prefs, key);
        if (item)
            prefs_change (prefs, key, item, val, changed);
        else
            prefs_create (prefs, key, val, changed);
    }
}


static Item     *prefs_get      (MooPrefs   *prefs,
                                 const char *key)
{
    return g_hash_table_lookup (prefs->priv->data, key);
}


static void      prefs_change   (MooPrefs   *prefs,
                                 const char *key,
                                 Item       *item,
                                 const char *val,
                                 gboolean    changed)
{
    g_free (item->value);
    item->value = g_strdup (val);
    if (changed)
        item->changed = TRUE;
    emit_notify (prefs, key, val);
}


static void      prefs_create   (MooPrefs   *prefs,
                                 const char *key,
                                 const char *val,
                                 gboolean    changed)
{
    Item *item = item_new (val, changed);
    g_hash_table_insert (prefs->priv->data,
                         g_strdup (key),
                         item);
    emit_notify (prefs, key, val);
}


static void      prefs_remove   (MooPrefs   *prefs,
                                 const char *key)
{
    Item *item = NULL;
    char *orig_key = NULL;
    gboolean found;

    found = g_hash_table_lookup_extended (prefs->priv->data,
                                          key,
                                          (gpointer*) &orig_key,
                                          (gpointer*) &item);
    if (!found) return;

    g_free (orig_key);
    item_free (item);
    g_hash_table_remove (prefs->priv->data, key);

    emit_notify (prefs, key, NULL);
}


static void      emit_notify    (MooPrefs   *prefs,
                                 const char *key,
                                 const char *val)
{
    GList *l;

    g_object_ref (prefs);

    for (l = prefs->priv->closures; l != NULL; l = l->next)
    {
        Closure *closure = l->data;
        if (!closure->blocked && closure_match (closure, key))
            closure_invoke (closure, key, val);
    }

    g_object_unref (prefs);
}


/***************************************************************************/
/* Closure
 */

static Closure  *closure_new    (MooPrefs           *prefs,
                                 const char         *pattern,
                                 MooPrefsMatchType   match_type,
                                 MooPrefsNotify      callback,
                                 gpointer            data)
{
    EggRegex *regex;
    Closure *closure;
    GError *err = NULL;

    closure = g_new (Closure, 1);
    closure->type = match_type;
    closure->callback = callback;
    closure->data = data;
    closure->blocked = FALSE;

    closure->id = ++prefs->priv->last_notify_id;

    switch (match_type) {
        case MOO_PREFS_MATCH_REGEX:
            regex = egg_regex_new (pattern, EGG_REGEX_EXTENDED, 0, &err);
            if (err)
            {
                g_warning ("%s: %s", G_STRLOC, err->message);
                g_error_free (err);
                egg_regex_free (regex);
                g_free (closure);
                return NULL;
            }
            egg_regex_optimize (regex, &err);
            if (err)
            {
                g_warning ("%s: %s", G_STRLOC, err->message);
                g_error_free (err);
            }

            closure->pattern.regex = regex;
            break;

        case MOO_PREFS_MATCH_PREFIX:
            closure->pattern.key = g_strdup (pattern);
            closure->prefix_len = strlen (pattern);
            break;

        case MOO_PREFS_MATCH_KEY:
            closure->pattern.key = g_strdup (pattern);
            break;

        default:
            g_assert_not_reached ();
    }

    return closure;
}


static void      closure_free   (Closure            *closure)
{
    if (!closure) return;
    pattern_free (closure->pattern, closure->type);
    g_free (closure);
}


static gboolean  closure_match  (Closure            *closure,
                                 const char         *key)
{
    switch (closure->type) {
        case MOO_PREFS_MATCH_KEY:
            return !strcmp (key, closure->pattern.key);

        case MOO_PREFS_MATCH_PREFIX:
            if (closure->prefix_len)
                return !strncmp (key, closure->pattern.key,
                                 closure->prefix_len);
            else
                return TRUE;

        case MOO_PREFS_MATCH_REGEX:
            egg_regex_clear (closure->pattern.regex);
            return egg_regex_match (closure->pattern.regex,
                                    key, -1, 0) > 0;

        default:
#ifndef G_DISABLE_ASSERT
            g_assert_not_reached ();
#else
            g_critical ("%s: should not be reached", G_STRLOC);
            return FALSE;
#endif
    }
}


static void      closure_invoke (Closure            *closure,
                                 const char         *key,
                                 const char         *value)
{
    closure->callback (key, value, closure->data);
}


static void      pattern_free   (Pattern             p,
                                 MooPrefsMatchType   type)
{
    if (!p.key) return;
    if (type == MOO_PREFS_MATCH_REGEX)
        egg_regex_free (p.regex);
    else
        g_free (p.key);
}


/***************************************************************************/
/* MooPrefs
 */

guint           moo_prefs_notify_connect        (const char         *pattern,
                                                 MooPrefsMatchType   match_type,
                                                 MooPrefsNotify      callback,
                                                 gpointer            data)
{
    Closure *closure;
    MooPrefs *prefs = instance ();

    g_return_val_if_fail (pattern != NULL, 0);
    g_return_val_if_fail (match_type == MOO_PREFS_MATCH_KEY ||
                          match_type == MOO_PREFS_MATCH_PREFIX ||
                          match_type == MOO_PREFS_MATCH_REGEX, 0);
    g_return_val_if_fail (callback != NULL, 0);

    closure = closure_new (prefs, pattern, match_type, callback, data);
    g_return_val_if_fail (closure != NULL, 0);

    prefs->priv->closures = g_list_prepend (prefs->priv->closures, closure);
    g_hash_table_insert (prefs->priv->closures_map,
                         GUINT_TO_POINTER (closure->id),
                         prefs->priv->closures);

    return closure->id;
}


static Closure *find_closure                    (MooPrefs           *prefs,
                                                 guint               id)
{
    GList *l;

    l = g_hash_table_lookup (prefs->priv->closures_map,
                             GUINT_TO_POINTER (id));
    if (l)
        return l->data;
    else
        return NULL;
}


gboolean        moo_prefs_notify_block          (guint               id)
{
    Closure *c;

    g_return_val_if_fail (id != 0, FALSE);

    c = find_closure (instance(), id);
    g_return_val_if_fail (c != NULL, FALSE);

    c->blocked = TRUE;
    return TRUE;
}


gboolean        moo_prefs_notify_unblock        (guint               id)
{
    Closure *c;

    g_return_val_if_fail (id != 0, FALSE);

    c = find_closure (instance(), id);
    g_return_val_if_fail (c != NULL, FALSE);

    c->blocked = FALSE;
    return TRUE;
}


gboolean        moo_prefs_notify_disconnect     (guint               id)
{
    GList *l;
    MooPrefs *prefs = instance ();

    g_return_val_if_fail (id != 0, FALSE);

    l = g_hash_table_lookup (prefs->priv->closures_map,
                             GUINT_TO_POINTER (id));
    g_return_val_if_fail (l != NULL, FALSE);

    g_hash_table_remove (prefs->priv->closures_map,
                         GUINT_TO_POINTER (id));

    closure_free (l->data);
    prefs->priv->closures =
            g_list_delete_link (prefs->priv->closures, l);

    return TRUE;
}


void            moo_prefs_set               (const char     *key,
                                             const char     *val)
{
    g_return_if_fail (key != NULL);
    prefs_set (instance(), key, val, TRUE);
}

void            moo_prefs_set_ignore_change (const char     *key,
                                             const char     *val)
{
    g_return_if_fail (key != NULL);
    prefs_set (instance(), key, val, FALSE);
}


const gchar    *moo_prefs_get               (const char     *key)
{
    Item *item;

    g_return_val_if_fail (key != NULL, NULL);

    item = prefs_get (instance(), key);
    if (item)
        return item->value;
    else
        return NULL;
}


/***************************************************************************/
/* Loading abd saving
 */

gboolean        moo_prefs_load          (const char     *file)
{
    GError *err = NULL;
    char *content = NULL;
    gsize len = 0;
    char** lines;
    guint i;

    g_return_val_if_fail (file != NULL, FALSE);

    if (!g_file_get_contents (file, &content, &len, &err))
    {
        g_critical ("%s: could not load file '%s'", G_STRLOC, file);
        if (err) {
            g_critical ("%s: %s", G_STRLOC, err->message);
            g_error_free (err);
        }
        return FALSE;
    }

    if (!len) {
        g_free (content);
        return TRUE;
    }

    g_strdelimit (content, "\r\f", '\n');
    lines = g_strsplit (content, "\n", 0);

    for (i = 0; lines[i]; ++i)
    {
        char **keyval = g_strsplit (lines[i], "=", 2);

        if (keyval[0])
        {
            if (keyval[1])
                moo_prefs_set (keyval[0], keyval[1]);
            else
                g_critical ("%s: error in file '%s' "
                            "on line %d", G_STRLOC, file, i);
        }

        g_strfreev (keyval);
    }

    g_free (content);
    g_strfreev (lines);
    return TRUE;
}


#ifdef __WIN32__
#define LINE_SEPARATOR "\r\n"
#elif defined(OS_DARWIN)
#define LINE_SEPARATOR "\r"
#else
#define LINE_SEPARATOR "\n"
#endif

typedef struct {
    const char *filename;
    GIOChannel *file;
    gboolean    error;
} Stuff;

static void write_item (const char  *key,
                        Item        *item,
                        Stuff       *stuff)
{
    gsize written;
    GIOStatus status;
    GError *err = NULL;
    char *string;

    if (!(item->value && item->changed) || stuff->error)
        return;

    if (!stuff->file)
    {
        stuff->file = g_io_channel_new_file (stuff->filename, "w", &err);
        if (!stuff->file)
        {
            g_critical ("%s: could not open file '%s' "
                        "for writing", G_STRLOC, stuff->filename);
            if (err)
            {
                g_critical ("%s: %s", G_STRLOC, err->message);
                g_error_free (err);
            }

            stuff->error = TRUE;
            return;
        }
    }

    string = g_strdup_printf ("%s=%s" LINE_SEPARATOR, key, item->value);
    status = g_io_channel_write_chars (stuff->file,
                                       string, -1,
                                       &written, &err);
    g_free (string);

    if (status != G_IO_STATUS_NORMAL)
    {
        g_critical ("%s: could not write to file '%s'",
                    G_STRLOC, stuff->filename);

        if (err)
        {
            g_critical ("%s: %s", G_STRLOC, err->message);
            g_error_free (err);
            err = NULL;
        }

        g_io_channel_shutdown (stuff->file, TRUE, &err);

        if (err)
        {
            g_critical ("%s: %s", G_STRLOC, err->message);
            g_error_free (err);
            err = NULL;
        }

        g_io_channel_unref (stuff->file);
        stuff->file = NULL;
        stuff->error = TRUE;
    }
}


gboolean        moo_prefs_save          (const char     *file)
{
    GError *err = NULL;
    Stuff stuff;
    MooPrefs *prefs = instance ();

    g_return_val_if_fail (file != NULL, FALSE);

    stuff.file = NULL;
    stuff.error = FALSE;
    stuff.filename = file;

    g_hash_table_foreach (prefs->priv->data,
                          (GHFunc) write_item,
                          &stuff);

    if (stuff.file)
    {
        g_io_channel_shutdown (stuff.file, TRUE, &err);
        if (err) {
            g_critical ("%s: %s", G_STRLOC, err->message);
            g_error_free (err);
        }
        g_io_channel_unref (stuff.file);
    }

    return !err && !stuff.error;
}


/***************************************************************************/
/* Helpers
 */

gboolean        moo_prefs_get_bool          (const char     *key)
{
    const char *val = moo_prefs_get (key);

    if (!val) return FALSE;

    return ! g_ascii_strcasecmp (val, "1") ||
            ! g_ascii_strcasecmp (val, "yes") ||
            ! g_ascii_strcasecmp (val, "true");
}


gdouble         moo_prefs_get_double        (const char     *key)
{
    const char *strval = moo_prefs_get (key);
    if (!strval)
        return 0;
    else return g_ascii_strtod (strval, NULL);
}


const GdkColor *moo_prefs_get_color         (const char     *key)
{
    static GdkColormap *sys_colormap = NULL;
    static GdkColor color;

    const char *strval = moo_prefs_get (key);

    if (!strval)
        return NULL;

    if (!sys_colormap)
        sys_colormap = gdk_colormap_get_system ();

    if (gdk_color_parse (strval, &color) &&
        gdk_colormap_alloc_color (sys_colormap, &color, TRUE, TRUE))
            return &color;

    g_warning ("%s: invalid color string '%s' in key '%s'",
               G_STRLOC, strval, key);
    return NULL;
}


int             moo_prefs_get_enum          (const char     *key,
                                             GType           type)
{
    gpointer klass;
    GEnumClass *enum_class;
    const char *sval;
    guint i;
    int val;

    sval = moo_prefs_get (key);
    if (!sval || !sval[0])
        return 0;

    klass = g_type_class_peek (type);
    g_return_val_if_fail (G_IS_ENUM_CLASS (klass), 0);
    enum_class = G_ENUM_CLASS (klass);

    for (i = 0; i < enum_class->n_values; ++i)
    {
        if (!strcmp (sval, enum_class->values[i].value_name))
            return enum_class->values[i].value;
    }

    val = moo_prefs_get_int (key);
    if (val < enum_class->minimum || val > enum_class->maximum)
        g_warning ("%s: value %d is illegal for type %s", G_STRLOC,
                   val, g_type_name (type));
    return val;
}


void            moo_prefs_set_if_not_set    (const char     *key,
                                             const char     *val)
{
    if (!moo_prefs_get (key))
        prefs_set (instance(), key, val, TRUE);
}

void            moo_prefs_set_if_not_set_ignore_change
                                            (const char     *key,
                                             const char     *val)
{
    if (!moo_prefs_get (key))
        prefs_set (instance(), key, val, FALSE);
}


static void     set_double                  (const char     *key,
                                             double          val,
                                             gboolean        ignore_change)
{
    char value[G_ASCII_DTOSTR_BUF_SIZE];
    g_ascii_dtostr (value, G_ASCII_DTOSTR_BUF_SIZE, val);
    prefs_set (instance(), key, value, !ignore_change);
}


void            moo_prefs_set_double        (const char     *key,
                                             double          val)
{
    set_double (key, val, FALSE);
}

void            moo_prefs_set_double_ignore_change
                                            (const char     *key,
                                             double          val)
{
    set_double (key, val, TRUE);
}

void            moo_prefs_set_double_if_not_set
                                            (const char     *key,
                                             double          val)
{
    if (!moo_prefs_get (key))
        set_double (key, val, FALSE);
}

void            moo_prefs_set_double_if_not_set_ignore_change
                                            (const char     *key,
                                             double          val)
{
    if (!moo_prefs_get (key))
        set_double (key, val, TRUE);
}


static void     set_bool                    (const char     *key,
                                             gboolean        val,
                                             gboolean        ignore_change)
{
    prefs_set (instance(), key,
               val ? "TRUE" : "FALSE",
               !ignore_change);
}

void            moo_prefs_set_bool          (const char     *key,
                                             gboolean        val)
{
    set_bool (key, val, FALSE);
}

void            moo_prefs_set_bool_ignore_change
                                            (const char     *key,
                                             gboolean        val)
{
    set_bool (key, val, TRUE);
}

void            moo_prefs_set_bool_if_not_set
                                            (const char     *key,
                                             gboolean        val)
{
    if (!moo_prefs_get (key))
        set_bool (key, val, FALSE);
}

void            moo_prefs_set_bool_if_not_set_ignore_change
                                            (const char     *key,
                                             gboolean        val)
{
    if (!moo_prefs_get (key))
        set_bool (key, val, TRUE);
}


static void     set_color                   (const char     *key,
                                             const GdkColor *val,
                                             gboolean        ignore_change)
{
    char sval[14];

    if (!val)
    {
        moo_prefs_set (key, NULL);
        return;
    }
    else
    {
        g_snprintf (sval, 8, "#%02x%02x%02x",
                    val->red >> 8,
                    val->green >> 8,
                    val->blue >> 8);
        prefs_set (instance(), key, sval, !ignore_change);
    }
}

void            moo_prefs_set_color         (const char     *key,
                                             const GdkColor *val)
{
    set_color (key, val, FALSE);
}

void            moo_prefs_set_color_ignore_change
                                            (const char     *key,
                                             const GdkColor *val)
{
    set_color (key, val, TRUE);
}

void            moo_prefs_set_color_if_not_set
                                            (const char     *key,
                                             const GdkColor *val)
{
    if (!moo_prefs_get (key))
        set_color (key, val, FALSE);
}

void            moo_prefs_set_color_if_not_set_ignore_change
                                            (const char     *key,
                                             const GdkColor *val)
{
    if (!moo_prefs_get (key))
        set_color (key, val, TRUE);
}


GType           moo_prefs_match_type_get_type   (void)
{
    static GType type = 0;

    if (!type)
    {
        static const GFlagsValue values[] = {
            { MOO_PREFS_MATCH_KEY, (char*)"MOO_PREFS_MATCH_KEY", (char*)"match-key" },
            { MOO_PREFS_MATCH_PREFIX, (char*)"MOO_PREFS_MATCH_PREFIX", (char*)"match-prefix" },
            { MOO_PREFS_MATCH_REGEX, (char*)"MOO_PREFS_MATCH_REGEX", (char*)"match-regex" },
            { 0, NULL, NULL }
        };
        type = g_flags_register_static ("MooPrefsMatchType", values);
    }

    return type;
}
