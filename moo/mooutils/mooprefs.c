/*
 *   mooprefs.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#include "mooutils/mooprefs.h"
#include "mooutils/moomarshals.h"
#include "mooutils/moomarkup.h"
#include "mooutils/mooutils-fs.h"
#include "mooutils/mooutils-gobject.h"
#include "mooutils/mootype-macros.h"
#include <string.h>
#include <errno.h>
#include <gobject/gvaluecollector.h>
#include <glib/gregex.h>

#define PREFS_TYPE_LAST 2
#define PREFS_ROOT "Prefs"
/* #define DEBUG_READWRITE 1 */

#define MOO_TYPE_PREFS              (_moo_prefs_get_type ())
#define MOO_PREFS(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_PREFS, MooPrefs))
#define MOO_PREFS_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_PREFS, MooPrefsClass))
#define MOO_IS_PREFS(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_PREFS))
#define MOO_IS_PREFS_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_PREFS))
#define MOO_PREFS_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_PREFS, MooPrefsClass))

typedef struct _MooPrefs        MooPrefs;
typedef struct _MooPrefsClass   MooPrefsClass;

struct _MooPrefs
{
    GObject          gobject;

    GHashTable      *data; /* char* -> Item* */
    MooMarkupDoc    *xml_rc;
    gboolean         rc_modified;
    MooMarkupDoc    *xml_state;
    GList           *closures;
    GHashTable      *closures_map; /* guint -> closures list link */
    guint            last_notify_id;
};

struct _MooPrefsClass
{
    GObjectClass   parent_class;
};


typedef struct {
    GType  type;
    GValue value;
    GValue default_value;
    guint  prefs_type : 1;
} PrefsItem;


static void          prefs_new_key      (MooPrefs       *prefs,
                                         const char     *key,
                                         GType           value_type,
                                         const GValue   *default_value,
                                         MooPrefsType    prefs_type);
static void          prefs_new_key_from_string (MooPrefs *prefs,
                                         const char     *key,
                                         const char     *value,
                                         MooPrefsType    prefs_type);
static void          prefs_delete_key   (MooPrefs       *prefs,
                                         const char     *key);
static GType         prefs_get_key_type (MooPrefs       *prefs,
                                         const char     *key);

static const GValue *prefs_get          (MooPrefs       *prefs,
                                         const char     *key);
static const GValue *prefs_get_default  (MooPrefs       *prefs,
                                         const char     *key);
static void          prefs_set          (MooPrefs       *prefs,
                                         const char     *key,
                                         const GValue   *value);
static void          prefs_set_default  (MooPrefs       *prefs,
                                         const char     *key,
                                         const GValue   *value);
static PrefsItem    *prefs_get_item     (MooPrefs       *prefs,
                                         const char     *key);
static void          prefs_emit_notify  (MooPrefs       *prefs,
                                         const char     *key,
                                         const GValue   *value);

static PrefsItem    *item_new           (GType           type,
                                         const GValue   *value,
                                         const GValue   *default_value,
                                         MooPrefsType    prefs_type);
static void          item_free          (PrefsItem      *item);
static gboolean      item_set_type      (PrefsItem      *item,
                                         GType           type);
static const GValue *item_value         (PrefsItem      *item);
static const GValue *item_default_value (PrefsItem      *item);
static gboolean      item_set           (PrefsItem      *item,
                                         const GValue   *value);
static gboolean      item_set_default   (PrefsItem      *item,
                                         const GValue   *value);

static void          moo_prefs_finalize (GObject        *object);
static void          moo_prefs_new_key_from_string (const char *key,
                                         const char     *value,
                                         MooPrefsType    prefs_type);

static void          moo_prefs_set_modified (gboolean    modified);


MOO_DEFINE_TYPE_STATIC (MooPrefs, _moo_prefs, G_TYPE_OBJECT)


static MooPrefs *
instance (void)
{
    static MooPrefs *p = NULL;

    if (G_UNLIKELY (!p))
        p = g_object_new (MOO_TYPE_PREFS, NULL);

    return p;
}


/***************************************************************************/
/* MooPrefs
 */

static void
_moo_prefs_class_init (MooPrefsClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    gobject_class->finalize = moo_prefs_finalize;
}


static void
_moo_prefs_init (MooPrefs *prefs)
{
    prefs->data =
            g_hash_table_new_full (g_str_hash, g_str_equal,
                                   (GDestroyNotify) g_free,
                                   (GDestroyNotify) item_free);

    prefs->closures_map = g_hash_table_new (g_direct_hash, NULL);
}


static void
moo_prefs_finalize (GObject *obj)
{
    MooPrefs *prefs = MOO_PREFS (obj);

    g_hash_table_destroy (prefs->data);

    G_OBJECT_CLASS(_moo_prefs_parent_class)->finalize (obj);
}


char*
moo_prefs_make_key (const char     *first_comp,
                    ...)
{
    char *key;
    va_list var_args;

    g_return_val_if_fail (first_comp != NULL, NULL);

    va_start (var_args, first_comp);
    key = moo_prefs_make_keyv (first_comp, var_args);
    va_end (var_args);

    return key;
}


char*
moo_prefs_make_keyv (const char     *first_comp,
                     va_list         var_args)
{
    const char *comp;
    GString *key = NULL;

    g_return_val_if_fail (first_comp != NULL, NULL);

    comp = first_comp;

    while (comp)
    {
        if (!key)
        {
            key = g_string_new (comp);
        }
        else
        {
            g_string_append_c (key, '/');
            g_string_append (key, comp);
        }

        comp = va_arg (var_args, const char*);
    }

    return g_string_free (key, FALSE);
}


MooMarkupDoc*
moo_prefs_get_markup (MooPrefsType prefs_type)
{
    MooPrefs *prefs = instance ();

    if (!prefs->xml_rc)
        prefs->xml_rc = moo_markup_doc_new ("Prefs");
    if (!prefs->xml_state)
        prefs->xml_state = moo_markup_doc_new ("Prefs");

    switch (prefs_type)
    {
        case MOO_PREFS_RC:
            return prefs->xml_rc;
        case MOO_PREFS_STATE:
            return prefs->xml_state;
    }

    g_return_val_if_reached (NULL);
}


void
moo_prefs_new_key (const char     *key,
                   GType           value_type,
                   const GValue   *default_value,
                   MooPrefsType    prefs_type)
{
    g_return_if_fail (key != NULL);
    g_return_if_fail (_moo_value_type_supported (value_type));
    g_return_if_fail (default_value != NULL);
    g_return_if_fail (G_VALUE_TYPE (default_value) == value_type);

    prefs_new_key (instance(), key, value_type, default_value, prefs_type);
}


void
moo_prefs_create_key (const char   *key,
                      MooPrefsType  prefs_type,
                      GType         value_type,
                      ...)
{
    va_list args;
    GValue default_value;
    char *error = NULL;

    g_return_if_fail (key != NULL);
    g_return_if_fail (prefs_type < 2);
    g_return_if_fail (_moo_value_type_supported (value_type));

    default_value.g_type = 0;
    g_value_init (&default_value, value_type);

    va_start (args, value_type);
    G_VALUE_COLLECT (&default_value, args, 0, &error);
    va_end (args);

    if (error)
    {
        g_warning ("%s: could not read value: %s", G_STRLOC, error);
        g_free (error);
        return;
    }

    moo_prefs_new_key (key, value_type, &default_value, prefs_type);
    g_value_unset (&default_value);
}


GType
moo_prefs_get_key_type (const char *key)
{
    g_return_val_if_fail (key != NULL, G_TYPE_NONE);
    return prefs_get_key_type (instance(), key);
}


gboolean
moo_prefs_key_registered (const char *key)
{
    g_return_val_if_fail (key != NULL, FALSE);
    return prefs_get_item (instance(), key) != NULL;
}


const GValue*
moo_prefs_get (const char *key)
{
    const GValue *val;

    g_return_val_if_fail (key != NULL, NULL);

    val = prefs_get (instance(), key);

    if (!val)
        g_warning ("%s: key %s not registered", G_STRLOC, key);

    return val;
}


const GValue*
moo_prefs_get_default (const char *key)
{
    g_return_val_if_fail (key != NULL, NULL);
    return prefs_get_default (instance(), key);
}


void
moo_prefs_set (const char     *key,
               const GValue   *value)
{
    g_return_if_fail (key != NULL);
    g_return_if_fail (value != NULL);
    prefs_set (instance(), key, value);
}


void
moo_prefs_set_default (const char     *key,
                       const GValue   *value)
{
    g_return_if_fail (key != NULL);
    g_return_if_fail (value != NULL);
    prefs_set_default (instance(), key, value);
}


void
moo_prefs_delete_key (const char     *key)
{
    g_return_if_fail (key != NULL);
    prefs_delete_key (instance(), key);
}


static void
moo_prefs_new_key_from_string (const char     *key,
                               const char     *value,
                               MooPrefsType    prefs_type)
{
    g_return_if_fail (key != NULL);
    g_return_if_fail (value != NULL);
    prefs_new_key_from_string (instance(), key, value, prefs_type);
}


static void
moo_prefs_set_modified (gboolean modified)
{
    MooPrefs *prefs = instance ();

#ifdef MOO_DEBUG
    if (modified && !prefs->rc_modified)
    {
        g_message ("%s: prefs modified", G_STRLOC);
    }
#endif

    prefs->rc_modified = modified;
}


static void
prepend_key (const char *key,
             PrefsItem  *item,
             gpointer    pdata)
{
    struct {
        GSList *list;
        MooPrefsType prefs_type;
    } *data = pdata;

    if (data->prefs_type == item->prefs_type)
        data->list = g_slist_prepend (data->list, g_strdup (key));
}

GSList *
moo_prefs_list_keys (MooPrefsType prefs_type)
{
    MooPrefs *prefs = instance ();

    struct {
        GSList *list;
        MooPrefsType prefs_type;
    } data;

    data.list = NULL;
    data.prefs_type = prefs_type;
    g_hash_table_foreach (prefs->data, (GHFunc) prepend_key, &data);

    return g_slist_sort (data.list, (GCompareFunc) strcmp);
}


/***************************************************************************/
/* Prefs
 */


static void
prefs_new_key (MooPrefs       *prefs,
               const char     *key,
               GType           type,
               const GValue   *default_value,
               MooPrefsType    prefs_type)
{
    PrefsItem *item;
    gboolean changed;

    g_return_if_fail (key && key[0]);
    g_return_if_fail (g_utf8_validate (key, -1, NULL));
    g_return_if_fail (_moo_value_type_supported (type));
    g_return_if_fail (G_IS_VALUE (default_value));
    g_return_if_fail (G_VALUE_TYPE (default_value) == type);

    item = prefs_get_item (prefs, key);

    if (!item)
    {
        item = item_new (type, default_value, default_value, prefs_type);
        g_hash_table_insert (prefs->data, g_strdup (key), item);
        changed = TRUE;
    }
    else
    {
        changed = item_set_type (item, type);

        if (item_set_default (item, default_value))
            changed = TRUE;

        if (item->prefs_type == MOO_PREFS_RC &&
            item->prefs_type != prefs_type)
                moo_prefs_set_modified (TRUE);

        item->prefs_type = prefs_type;
    }

    if (changed)
        prefs_emit_notify (prefs, key, item_value (item));
}


static void
prefs_new_key_from_string (MooPrefs     *prefs,
                           const char   *key,
                           const char   *value,
                           MooPrefsType  prefs_type)
{
    PrefsItem *item;
    GValue val, default_val;

    g_return_if_fail (key && key[0]);
    g_return_if_fail (g_utf8_validate (key, -1, NULL));

    item = prefs_get_item (prefs, key);

    val.g_type = 0;
    g_value_init (&val, G_TYPE_STRING);
    g_value_set_static_string (&val, value);
    default_val.g_type = 0;
    g_value_init (&default_val, G_TYPE_STRING);

    if (!item)
    {
        prefs_new_key (prefs, key, G_TYPE_STRING, &default_val, prefs_type);
        item = prefs_get_item (prefs, key);
        item_set (item, &val);
    }
    else
    {
        _moo_value_convert (&val, &item->value);
        item->prefs_type = prefs_type;
    }

    g_value_unset (&val);
    g_value_unset (&default_val);
    prefs_emit_notify (prefs, key, item_value (item));
}


static GType
prefs_get_key_type (MooPrefs       *prefs,
                    const char     *key)
{
    PrefsItem *item;

    g_return_val_if_fail (key != NULL, G_TYPE_NONE);

    item = prefs_get_item (prefs, key);
    g_return_val_if_fail (item != NULL, G_TYPE_NONE);

    return item->type;
}


static const GValue*
prefs_get (MooPrefs       *prefs,
           const char     *key)
{
    PrefsItem *item;

    g_return_val_if_fail (key != NULL, NULL);

    item = prefs_get_item (prefs, key);
    return item ? item_value (item) : NULL;
}


static const GValue*
prefs_get_default (MooPrefs       *prefs,
                   const char     *key)
{
    PrefsItem *item;

    g_return_val_if_fail (key != NULL, NULL);

    item = prefs_get_item (prefs, key);
    g_return_val_if_fail (item != NULL, NULL);

    return item_default_value (item);
}


static void
prefs_set (MooPrefs       *prefs,
           const char     *key,
           const GValue   *value)
{
    PrefsItem *item;

    g_return_if_fail (key != NULL);
    g_return_if_fail (G_IS_VALUE (value));

    item = prefs_get_item (prefs, key);

    if (!item)
    {
        g_warning ("key '%s' not registered", key);
        return;
    }

    if (item_set (item, value))
    {
        if (item->prefs_type == MOO_PREFS_RC)
            moo_prefs_set_modified (TRUE);
        prefs_emit_notify (prefs, key, item_value (item));
    }
}


static void
prefs_set_default (MooPrefs       *prefs,
                   const char     *key,
                   const GValue   *value)
{
    PrefsItem *item;

    g_return_if_fail (key != NULL);
    g_return_if_fail (G_IS_VALUE (value));

    item = prefs_get_item (prefs, key);
    g_return_if_fail (item != NULL);

    item_set_default (item, value);
}


static void
prefs_delete_key (MooPrefs       *prefs,
                  const char     *key)
{
    PrefsItem *item;

    g_return_if_fail (key != NULL);

    item = prefs_get_item (prefs, key);

    if (!item)
        return;

    if (item->prefs_type == MOO_PREFS_RC)
        moo_prefs_set_modified (TRUE);

    g_hash_table_remove (prefs->data, key);
    prefs_emit_notify (prefs, key, NULL);
}



static PrefsItem*
prefs_get_item (MooPrefs       *prefs,
                const char     *key)
{
    g_return_val_if_fail (key != NULL, NULL);
    return g_hash_table_lookup (prefs->data, key);
}


/***************************************************************************/
/* PrefsItem
 */

static PrefsItem*
item_new (GType           type,
          const GValue   *value,
          const GValue   *default_value,
          MooPrefsType    prefs_type)
{
    PrefsItem *item;

    g_return_val_if_fail (_moo_value_type_supported (type), NULL);
    g_return_val_if_fail (value && default_value, NULL);
    g_return_val_if_fail (G_VALUE_TYPE (value) == type, NULL);
    g_return_val_if_fail (G_VALUE_TYPE (default_value) == type, NULL);

    item = g_new0 (PrefsItem, 1);

    item->type = type;
    item->prefs_type = prefs_type;

    g_value_init (&item->value, type);
    g_value_copy (value, &item->value);
    g_value_init (&item->default_value, type);
    g_value_copy (default_value, &item->default_value);

    return item;
}


static gboolean
item_set_type (PrefsItem      *item,
               GType           type)
{
    g_return_val_if_fail (item != NULL, FALSE);
    g_return_val_if_fail (_moo_value_type_supported (type), FALSE);

    if (type != item->type)
    {
        item->type = type;
        _moo_value_change_type (&item->value, type);
        _moo_value_change_type (&item->default_value, type);
        return TRUE;
    }

    return FALSE;
}


static void
item_free (PrefsItem *item)
{
    if (item)
    {
        item->type = 0;
        g_value_unset (&item->value);
        g_value_unset (&item->default_value);
        g_free (item);
    }
}


static const GValue*
item_value (PrefsItem *item)
{
    g_return_val_if_fail (item != NULL, NULL);
    return &item->value;
}


static const GValue*
item_default_value (PrefsItem *item)
{
    g_return_val_if_fail (item != NULL, NULL);
    return &item->default_value;
}


static gboolean
item_set (PrefsItem      *item,
          const GValue   *value)
{
    g_return_val_if_fail (item != NULL, FALSE);
    g_return_val_if_fail (G_IS_VALUE (value), FALSE);
    g_return_val_if_fail (item->type == G_VALUE_TYPE (value), FALSE);

    if (!_moo_value_equal (value, &item->value))
    {
        g_value_copy (value, &item->value);
        return TRUE;
    }

    return FALSE;
}


static gboolean
item_set_default (PrefsItem      *item,
                  const GValue   *value)
{
    g_return_val_if_fail (item != NULL, FALSE);
    g_return_val_if_fail (G_IS_VALUE (value), FALSE);
    g_return_val_if_fail (item->type == G_VALUE_TYPE (value), FALSE);

    if (!_moo_value_equal (value, &item->default_value))
    {
        g_value_copy (value, &item->default_value);
        return TRUE;
    }

    return FALSE;
}


/***************************************************************************/
/* PrefsNotify
 */

typedef union {
    char        *key;
    GRegex    *regex;
} Pattern;

typedef struct {
    guint               id;
    MooPrefsMatchType   type;
    Pattern             pattern;
    guint               prefix_len;
    MooPrefsNotify      callback;
    gpointer            data;
    GDestroyNotify      notify;
    guint               blocked : 1;
} Closure;

static void      pattern_free   (Pattern             p,
                                 MooPrefsMatchType   type);

static Closure  *closure_new    (MooPrefs           *prefs,
                                 const char         *pattern,
                                 MooPrefsMatchType   match_type,
                                 MooPrefsNotify      callback,
                                 gpointer            data,
                                 GDestroyNotify      notify);
static void      closure_free   (Closure            *closure);
static gboolean  closure_match  (Closure            *closure,
                                 const char         *key);
static void      closure_invoke (Closure            *closure,
                                 const char         *key,
                                 const GValue       *value);


static void
prefs_emit_notify (MooPrefs       *prefs,
                   const char     *key,
                   const GValue   *value)
{
    GList *l;

    g_object_ref (prefs);

    for (l = prefs->closures; l != NULL; l = l->next)
    {
        Closure *closure = l->data;
        if (!closure->blocked && closure_match (closure, key))
            closure_invoke (closure, key, value);
    }

    g_object_unref (prefs);
}


guint
moo_prefs_notify_connect (const char         *pattern,
                          MooPrefsMatchType   match_type,
                          MooPrefsNotify      callback,
                          gpointer            data,
                          GDestroyNotify      notify)
{
    Closure *closure;
    MooPrefs *prefs = instance ();

    g_return_val_if_fail (pattern != NULL, 0);
    g_return_val_if_fail (match_type == MOO_PREFS_MATCH_KEY ||
                          match_type == MOO_PREFS_MATCH_PREFIX ||
                          match_type == MOO_PREFS_MATCH_REGEX, 0);
    g_return_val_if_fail (callback != NULL, 0);

    closure = closure_new (prefs, pattern, match_type, callback, data, notify);
    g_return_val_if_fail (closure != NULL, 0);

    prefs->closures = g_list_prepend (prefs->closures, closure);
    g_hash_table_insert (prefs->closures_map,
                         GUINT_TO_POINTER (closure->id),
                         prefs->closures);

    return closure->id;
}


static Closure*
closure_new (MooPrefs           *prefs,
             const char         *pattern,
             MooPrefsMatchType   match_type,
             MooPrefsNotify      callback,
             gpointer            data,
             GDestroyNotify      notify)
{
    GRegex *regex;
    Closure *closure;
    GError *err = NULL;

    closure = g_new (Closure, 1);
    closure->type = match_type;
    closure->callback = callback;
    closure->data = data;
    closure->notify = notify;
    closure->blocked = FALSE;

    closure->id = ++prefs->last_notify_id;

    switch (match_type) {
        case MOO_PREFS_MATCH_REGEX:
            regex = g_regex_new (pattern, G_REGEX_EXTENDED | G_REGEX_OPTIMIZE, 0, &err);

            if (!regex)
            {
                g_warning ("%s: %s", G_STRLOC, err->message);
                g_error_free (err);
                g_free (closure);
                return NULL;
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


static void
closure_free (Closure *closure)
{
    if (closure)
    {
        if (closure->notify)
            closure->notify (closure->data);
        pattern_free (closure->pattern, closure->type);
        g_free (closure);
    }
}


static gboolean
closure_match (Closure            *closure,
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
            return g_regex_match (closure->pattern.regex, key, 0, NULL);

        default:
            g_return_val_if_reached (FALSE);
    }
}


static void
closure_invoke (Closure            *closure,
                const char         *key,
                const GValue       *value)
{
    closure->callback (key, value, closure->data);
}


static void
pattern_free (Pattern             p,
              MooPrefsMatchType   type)
{
    if (p.key)
    {
        if (type == MOO_PREFS_MATCH_REGEX)
            g_regex_unref (p.regex);
        else
            g_free (p.key);
    }
}


static Closure*
find_closure (MooPrefs           *prefs,
              guint               id)
{
    GList *l;

    l = g_hash_table_lookup (prefs->closures_map,
                             GUINT_TO_POINTER (id));
    if (l)
        return l->data;
    else
        return NULL;
}


gboolean
moo_prefs_notify_block (guint id)
{
    Closure *c;

    g_return_val_if_fail (id != 0, FALSE);

    c = find_closure (instance(), id);
    g_return_val_if_fail (c != NULL, FALSE);

    c->blocked = TRUE;
    return TRUE;
}


gboolean
moo_prefs_notify_unblock (guint id)
{
    Closure *c;

    g_return_val_if_fail (id != 0, FALSE);

    c = find_closure (instance(), id);
    g_return_val_if_fail (c != NULL, FALSE);

    c->blocked = FALSE;
    return TRUE;
}


gboolean
moo_prefs_notify_disconnect (guint id)
{
    GList *l;
    MooPrefs *prefs = instance ();

    g_return_val_if_fail (id != 0, FALSE);

    l = g_hash_table_lookup (prefs->closures_map,
                             GUINT_TO_POINTER (id));
    g_return_val_if_fail (l != NULL, FALSE);

    g_hash_table_remove (prefs->closures_map,
                         GUINT_TO_POINTER (id));

    closure_free (l->data);
    prefs->closures = g_list_delete_link (prefs->closures, l);

    return TRUE;
}


/***************************************************************************/
/* Loading abd saving
 */

static void
process_element (MooMarkupElement *elm,
                 MooPrefsType      prefs_type)
{
    MooMarkupNode *child;
    gboolean dir = FALSE;

    if (elm->parent->type == MOO_MARKUP_DOC_NODE)
    {
        dir = TRUE;
    }
    else
    {
        for (child = elm->children; child != NULL; child = child->next)
            if (child->type == MOO_MARKUP_ELEMENT_NODE)
                dir = TRUE;
    }

    if (!dir)
    {
        char *path;

        path = moo_markup_element_get_path (elm);

        if (strlen (path) < strlen (PREFS_ROOT) + 2)
        {
            g_free (path);
            g_return_if_reached ();
        }
        else
        {
            const char *key = path + strlen (PREFS_ROOT) + 1;
            moo_prefs_new_key_from_string (key, elm->content, prefs_type);

#ifdef DEBUG_READWRITE
            g_print ("key: '%s', val: '%s'\n", key, elm->content);
#endif

            g_free (path);
            return;
        }
    }

    for (child = elm->children; child != NULL; child = child->next)
        if (child->type == MOO_MARKUP_ELEMENT_NODE)
            process_element (MOO_MARKUP_ELEMENT (child), prefs_type);
}


static gboolean
load_file (const char  *file,
           MooPrefsType prefs_type,
           GError     **error)
{
    MooMarkupDoc *xml;
    MooMarkupNode *root;
    MooPrefs *prefs;
    MooMarkupDoc **target = NULL;

    prefs = instance ();

    switch (prefs_type)
    {
        case MOO_PREFS_RC:
            target = &prefs->xml_rc;
            break;
        case MOO_PREFS_STATE:
            target = &prefs->xml_state;
            break;
    }

    if (!g_file_test (file, G_FILE_TEST_EXISTS))
        return TRUE;

    xml = moo_markup_parse_file (file, error);

    if (!xml)
        return FALSE;

    if (*target)
    {
        g_warning ("%s: implement me", G_STRLOC);
        moo_markup_doc_unref (*target);
    }

    *target = xml;

    if (prefs_type != MOO_PREFS_STATE)
    {
        _moo_markup_set_track_modified (xml, TRUE);
        _moo_markup_set_modified (xml, FALSE);
    }

    root = moo_markup_get_root_element (xml, PREFS_ROOT);

    if (!root)
        return TRUE;

    process_element (MOO_MARKUP_ELEMENT (root), prefs_type);

    return TRUE;
}


gboolean
moo_prefs_load (const char     *file_rc,
                const char     *file_state,
                GError        **error)
{
    g_return_val_if_fail (file_rc != NULL, FALSE);
    g_return_val_if_fail (file_state != NULL, FALSE);

    moo_prefs_set_modified (FALSE);

    return load_file (file_rc, MOO_PREFS_RC, error) &&
           load_file (file_state, MOO_PREFS_STATE, error);
}


typedef struct {
    MooMarkupDoc  *xml;
    MooMarkupNode *root;
    MooPrefsType   prefs_type;
} Stuff;


static void
write_item (const char  *key,
            PrefsItem   *item,
            Stuff       *stuff)
{
    const char *string = NULL;

    g_return_if_fail (key != NULL && key[0] != 0);
    g_return_if_fail (item != NULL && stuff != NULL);
    g_return_if_fail (_moo_value_type_supported (item->type));

    if (item->prefs_type != stuff->prefs_type)
        return;

    if (_moo_value_equal (item_value (item), item_default_value (item)))
    {
#ifdef DEBUG_READWRITE
        g_print ("skipping '%s'\n", key);
#endif
        return;
    }

    string = _moo_value_convert_to_string (item_value (item));

    if (!string)
        string = "";

    if (!stuff->root)
        stuff->root =
                moo_markup_create_root_element (stuff->xml, PREFS_ROOT);

    g_return_if_fail (stuff->root != NULL);

    moo_markup_create_text_element (stuff->root, key, string);

#ifdef DEBUG_READWRITE
    g_print ("writing key: '%s', val: '%s'\n", key, string);
#endif
}


static void
sync_xml (MooPrefsType prefs_type)
{
    MooPrefs *prefs = instance ();
    MooMarkupDoc *xml;
    MooMarkupDoc **xml_ptr = NULL;
    MooMarkupNode *root;
    Stuff stuff;

    switch (prefs_type)
    {
        case MOO_PREFS_RC:
            xml_ptr = &prefs->xml_rc;
            break;
        case MOO_PREFS_STATE:
            xml_ptr = &prefs->xml_state;
            break;
    }

    if (!*xml_ptr)
        *xml_ptr = moo_markup_doc_new ("Prefs");

    xml = *xml_ptr;
    root = moo_markup_get_root_element (xml, PREFS_ROOT);

    if (root)
        moo_markup_delete_node (root);

    stuff.prefs_type = prefs_type;
    stuff.xml = xml;
    stuff.root = NULL;

    g_hash_table_foreach (prefs->data,
                          (GHFunc) write_item,
                          &stuff);
}


static gboolean
check_modified (MooPrefsType prefs_type)
{
    MooPrefs *prefs = instance ();
    MooMarkupDoc *xml = prefs->xml_rc;

    if (prefs_type != MOO_PREFS_RC)
        return TRUE;

    if (xml && _moo_markup_get_modified (xml))
        return TRUE;
    else
        return prefs->rc_modified;
}

static gboolean
save_file (const char    *file,
           MooPrefsType   prefs_type,
           GError       **error)
{
    MooMarkupDoc *xml = NULL;
    MooMarkupNode *node;
    gboolean empty;
    MooPrefs *prefs = instance ();

    if (!check_modified (prefs_type))
        return TRUE;

    sync_xml (prefs_type);

    switch (prefs_type)
    {
        case MOO_PREFS_RC:
            xml = prefs->xml_rc;
            break;
        case MOO_PREFS_STATE:
            xml = prefs->xml_state;
            break;
    }

    g_return_val_if_fail (xml != NULL, FALSE);

    empty = TRUE;
    for (node = xml->children; empty && node != NULL; node = node->next)
        if (MOO_MARKUP_IS_ELEMENT (node))
            empty = FALSE;

    if (empty)
    {
        if (g_file_test (file, G_FILE_TEST_EXISTS))
            if (_moo_unlink (file))
                g_critical ("%s: %s", G_STRLOC,
                            g_strerror (errno));
        return TRUE;
    }

    return moo_markup_save_pretty (xml, file, 2, error);
}


gboolean
moo_prefs_save (const char  *file_rc,
                const char  *file_state,
                GError     **error)
{
    MooPrefs *prefs = instance ();

    g_return_val_if_fail (file_rc != NULL, FALSE);
    g_return_val_if_fail (file_state != NULL, FALSE);

    if (save_file (file_rc, MOO_PREFS_RC, error))
    {
        _moo_markup_set_modified (prefs->xml_rc, FALSE);
        moo_prefs_set_modified (FALSE);
    }
    else
    {
        return FALSE;
    }

    return save_file (file_state, MOO_PREFS_STATE, error);
}


/***************************************************************************/
/* Helpers
 */

void
moo_prefs_new_key_bool (const char *key,
                        gboolean    default_val)
{
    GValue val;

    g_return_if_fail (key != NULL);

    val.g_type = 0;
    g_value_init (&val, G_TYPE_BOOLEAN);
    g_value_set_boolean (&val, default_val);

    moo_prefs_new_key (key, G_TYPE_BOOLEAN, &val, MOO_PREFS_RC);
}


void
moo_prefs_new_key_int (const char *key,
                       int         default_val)
{
    GValue val;

    g_return_if_fail (key != NULL);

    val.g_type = 0;
    g_value_init (&val, G_TYPE_INT);
    g_value_set_int (&val, default_val);

    moo_prefs_new_key (key, G_TYPE_INT, &val, MOO_PREFS_RC);
}


void
moo_prefs_new_key_enum (const char *key,
                        GType       enum_type,
                        int         default_val)
{
    GValue val;

    g_return_if_fail (key != NULL);

    val.g_type = 0;
    g_value_init (&val, enum_type);
    g_value_set_enum (&val, default_val);

    moo_prefs_new_key (key, enum_type, &val, MOO_PREFS_RC);
}


void
moo_prefs_new_key_flags (const char *key,
                         GType       flags_type,
                         int         default_val)
{
    GValue val;

    g_return_if_fail (key != NULL);

    val.g_type = 0;
    g_value_init (&val, flags_type);
    g_value_set_flags (&val, default_val);

    moo_prefs_new_key (key, flags_type, &val, MOO_PREFS_RC);
}


void
moo_prefs_new_key_string (const char *key,
                          const char *default_val)
{
    static GValue val;

    g_return_if_fail (key != NULL);

    val.g_type = 0;
    g_value_init (&val, G_TYPE_STRING);

    g_value_set_static_string (&val, default_val);
    moo_prefs_new_key (key, G_TYPE_STRING, &val, MOO_PREFS_RC);

    g_value_unset (&val);
}


void
moo_prefs_new_key_color (const char     *key,
                         const GdkColor *default_val)
{
    static GValue val;

    g_return_if_fail (key != NULL);

    if (!G_IS_VALUE (&val))
        g_value_init (&val, GDK_TYPE_COLOR);

    g_value_set_boxed (&val, default_val);
    moo_prefs_new_key (key, GDK_TYPE_COLOR, &val, MOO_PREFS_RC);
}


const char *
moo_prefs_get_string (const char *key)
{
    const GValue *val;

    val = moo_prefs_get (key);
    g_return_val_if_fail (val != NULL, NULL);

    return g_value_get_string (val);
}


const char *
moo_prefs_get_filename (const char *key)
{
    const char *utf8_val;
    static char *val = NULL;
    GError *error = NULL;

    utf8_val = moo_prefs_get_string (key);

    if (!utf8_val)
        return NULL;

    g_free (val);
    val = g_filename_from_utf8 (utf8_val, -1, NULL, NULL, &error);

    if (!val)
    {
        g_warning ("%s: could not convert '%s' to filename encoding",
                   G_STRLOC, utf8_val);
        g_warning ("%s: %s", G_STRLOC, error->message);
        g_error_free (error);
    }

    return val;
}


gboolean
moo_prefs_get_bool (const char *key)
{
    const GValue *value;

    g_return_val_if_fail (key != NULL, FALSE);

    value = moo_prefs_get (key);
    g_return_val_if_fail (value != NULL, FALSE);

    return g_value_get_boolean (value);
}


gdouble
moo_prefs_get_number (const char *key)
{
    const GValue *value;

    g_return_val_if_fail (key != NULL, 0);

    value = moo_prefs_get (key);
    g_return_val_if_fail (value != NULL, 0);

    return _moo_value_convert_to_double (value);
}


const GdkColor*
moo_prefs_get_color (const char *key)
{
    const GValue *value;

    g_return_val_if_fail (key != NULL, NULL);

    value = moo_prefs_get (key);
    g_return_val_if_fail (value != NULL, NULL);

    g_return_val_if_fail (G_VALUE_TYPE (value) == GDK_TYPE_COLOR, NULL);
    return g_value_get_boxed (value);
}


int
moo_prefs_get_int (const char *key)
{
    const GValue *value;

    g_return_val_if_fail (key != NULL, 0);

    value = moo_prefs_get (key);
    g_return_val_if_fail (value != NULL, 0);

    return g_value_get_int (value);
}


int
moo_prefs_get_enum (const char *key)
{
    const GValue *value;

    g_return_val_if_fail (key != NULL, 0);

    value = moo_prefs_get (key);
    g_return_val_if_fail (value != NULL, 0);

    return g_value_get_enum (value);
}


int
moo_prefs_get_flags (const char *key)
{
    const GValue *value;

    g_return_val_if_fail (key != NULL, 0);

    value = moo_prefs_get (key);
    g_return_val_if_fail (value != NULL, 0);

    return g_value_get_flags (value);
}


void
moo_prefs_set_string (const char     *key,
                      const char     *val)
{
    GValue gval;

    g_return_if_fail (key != NULL);

    gval.g_type = 0;
    g_value_init (&gval, G_TYPE_STRING);
    g_value_set_static_string (&gval, val);
    moo_prefs_set (key, &gval);
    g_value_unset (&gval);
}


void
moo_prefs_set_filename (const char     *key,
                        const char     *val)
{
    char *utf8_val;

    g_return_if_fail (key != NULL);

    if (!val)
    {
        moo_prefs_set_string (key, NULL);
        return;
    }

    utf8_val = g_filename_display_name (val);

    if (!utf8_val)
    {
        g_warning ("%s: could not convert '%s' to utf8", G_STRLOC, val);
        return;
    }

    moo_prefs_set_string (key, utf8_val);
    g_free (utf8_val);
}


void
moo_prefs_set_number (const char     *key,
                      double          val)
{
    GValue gval, double_val;
    GType type;

    g_return_if_fail (key != NULL);
    g_return_if_fail (moo_prefs_key_registered (key));

    type = moo_prefs_get_key_type (key);

    gval.g_type = 0;
    double_val.g_type = 0;
    g_value_init (&gval, type);
    g_value_init (&double_val, G_TYPE_DOUBLE);

    g_value_set_double (&double_val, val);
    _moo_value_convert (&double_val, &gval);
    moo_prefs_set (key, &gval);
}


void
moo_prefs_set_int (const char     *key,
                   int             val)
{
    static GValue gval;

    g_return_if_fail (key != NULL);

    if (!G_IS_VALUE (&gval))
        g_value_init (&gval, G_TYPE_INT);

    g_value_set_int (&gval, val);
    moo_prefs_set (key, &gval);
}


void
moo_prefs_set_bool (const char     *key,
                    gboolean        val)
{
    static GValue gval;

    g_return_if_fail (key != NULL);

    if (!G_IS_VALUE (&gval))
        g_value_init (&gval, G_TYPE_BOOLEAN);

    g_value_set_boolean (&gval, val);
    moo_prefs_set (key, &gval);
}


void
moo_prefs_set_color (const char     *key,
                     const GdkColor *val)
{
    static GValue gval;

    g_return_if_fail (key != NULL);

    if (!G_IS_VALUE (&gval))
        g_value_init (&gval, GDK_TYPE_COLOR);

    g_value_set_boxed (&gval, val);
    moo_prefs_set (key, &gval);
}


void
moo_prefs_set_enum (const char     *key,
                    int             value)
{
    GValue gval;
    GType type;

    g_return_if_fail (key != NULL);

    type = moo_prefs_get_key_type (key);
    g_return_if_fail (G_TYPE_IS_ENUM (type));

    gval.g_type = 0;
    g_value_init (&gval, type);
    g_value_set_enum (&gval, value);

    moo_prefs_set (key, &gval);
}


void
moo_prefs_set_flags (const char     *key,
                     int             value)
{
    GValue gval;
    GType type;

    g_return_if_fail (key != NULL);

    type = moo_prefs_get_key_type (key);
    g_return_if_fail (G_TYPE_IS_FLAGS (type));

    gval.g_type = 0;
    g_value_init (&gval, type);
    g_value_set_flags (&gval, value);

    moo_prefs_set (key, &gval);
}


GType
moo_prefs_match_type_get_type (void)
{
    static GType type = 0;

    if (G_UNLIKELY (!type))
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


GType
moo_prefs_type_get_type (void)
{
    static GType type = 0;

    if (G_UNLIKELY (!type))
    {
        static const GEnumValue values[] = {
            { MOO_PREFS_RC, (char*)"MOO_PREFS_RC", (char*)"rc" },
            { MOO_PREFS_STATE, (char*)"MOO_PREFS_STATE", (char*)"state" },
            { 0, NULL, NULL }
        };
        type = g_enum_register_static ("MooPrefsType", values);
    }

    return type;
}


#if 0
static void
add_key (const char  *key,
         G_GNUC_UNUSED PrefsItem *item,
         GPtrArray   *array)
{
    g_ptr_array_add (array, g_strdup (key));
}

char**
moo_prefs_list_keys (guint *n_keys)
{
    MooPrefs *prefs = instance ();
    GPtrArray *array;

    array = g_ptr_array_new ();

    g_hash_table_foreach (prefs->data,
                          (GHFunc) add_key,
                          array);

    if (!array->len)
    {
        g_ptr_array_free (array, TRUE);
        if (n_keys)
            *n_keys = 0;
        return NULL;
    }
    else
    {
        if (n_keys)
            *n_keys = array->len;
        g_ptr_array_add (array, NULL);
        return (char**) g_ptr_array_free (array, FALSE);
    }
}
#endif
