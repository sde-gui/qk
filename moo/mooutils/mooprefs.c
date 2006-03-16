/*
 *   mooutils/mooprefs.c
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

#include "mooutils/mooprefs.h"
#include "mooutils/moocompat.h"
#include "mooutils/eggregex.h"
#include "mooutils/moomarshals.h"
#include "mooutils/moomarkup.h"
#include "mooutils/mooutils-fs.h"
#include "mooutils/mooutils-gobject.h"
#include <string.h>
#include <errno.h>

#define PREFS_ROOT "Prefs"
/* #define DEBUG_READWRITE 1 */

#if !GLIB_CHECK_VERSION(2,4,0)
#define g_value_take_string g_value_set_string_take_ownership
#endif


struct _MooPrefsPrivate {
    GHashTable      *data; /* char* -> Item* */
    MooMarkupDoc    *xml;
    GList           *closures;
    GHashTable      *closures_map; /* guint -> closures list link */
    guint            last_notify_id;
};


typedef struct {
    GType   type;
    GValue  value;
    GValue  default_value;
} PrefsItem;


static void          prefs_new_key      (MooPrefs       *prefs,
                                         const char     *key,
                                         GType           value_type,
                                         const GValue   *default_value);
static void          prefs_new_key_from_string (MooPrefs *prefs,
                                         const char     *key,
                                         const char     *value);
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
                                         const GValue   *default_value);
static void          item_free          (PrefsItem      *item);
static void          item_set_type      (PrefsItem      *item,
                                         GType           type);
static const GValue *item_value         (PrefsItem      *item);
static const GValue *item_default_value (PrefsItem      *item);
static void          item_set           (PrefsItem      *item,
                                         const GValue   *value);
static void          item_set_default   (PrefsItem      *item,
                                         const GValue   *value);

static void          moo_prefs_finalize (GObject        *object);
static void          moo_prefs_new_key_from_string (const char     *key,
                                         const char     *value);


/* MOO_TYPE_PREFS */
G_DEFINE_TYPE (MooPrefs, moo_prefs, G_TYPE_OBJECT)


static MooPrefs*
instance (void)
{
    static MooPrefs *p = NULL;

    if (!p)
        p = g_object_new (MOO_TYPE_PREFS, NULL);

    return p;
}


/***************************************************************************/
/* MooPrefs
 */

static void
moo_prefs_class_init (MooPrefsClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    gobject_class->finalize = moo_prefs_finalize;
}


static void
moo_prefs_init (MooPrefs *prefs)
{
    prefs->priv = g_new0 (MooPrefsPrivate, 1);

    prefs->priv->data =
            g_hash_table_new_full (g_str_hash, g_str_equal,
                                   (GDestroyNotify) g_free,
                                   (GDestroyNotify) item_free);

    prefs->priv->closures_map = g_hash_table_new (g_direct_hash, NULL);
}


static void
moo_prefs_finalize (GObject *obj)
{
    MooPrefs *prefs = MOO_PREFS (obj);

    g_hash_table_destroy (prefs->priv->data);
    g_free (prefs->priv);
    prefs->priv = NULL;

    G_OBJECT_CLASS(moo_prefs_parent_class)->finalize (obj);
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
moo_prefs_get_markup (void)
{
    MooPrefs *prefs = instance ();

    if (!prefs->priv->xml)
        prefs->priv->xml = moo_markup_doc_new ("Prefs");

    return prefs->priv->xml;
}


void
moo_prefs_new_key (const char     *key,
                   GType           value_type,
                   const GValue   *default_value)
{
    g_return_if_fail (key != NULL);
    g_return_if_fail (moo_value_type_supported (value_type));
    g_return_if_fail (default_value != NULL);
    g_return_if_fail (G_VALUE_TYPE (default_value) == value_type);

    prefs_new_key (instance(), key, value_type, default_value);
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
    g_return_val_if_fail (key != NULL, NULL);
    return prefs_get (instance(), key);
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
                               const char     *value)
{
    g_return_if_fail (key != NULL);
    g_return_if_fail (value != NULL);
    prefs_new_key_from_string (instance(), key, value);
}


/***************************************************************************/
/* Prefs
 */


static void
prefs_new_key (MooPrefs       *prefs,
               const char     *key,
               GType           type,
               const GValue   *default_value)
{
    PrefsItem *item;

    g_return_if_fail (key && key[0]);
    g_return_if_fail (g_utf8_validate (key, -1, NULL));
    g_return_if_fail (moo_value_type_supported (type));
    g_return_if_fail (G_IS_VALUE (default_value));
    g_return_if_fail (G_VALUE_TYPE (default_value) == type);

    item = prefs_get_item (prefs, key);

    if (!item)
    {
        item = item_new (type, default_value, default_value);
        g_hash_table_insert (prefs->priv->data, g_strdup (key), item);
    }
    else
    {
        item_set_type (item, type);
        item_set_default (item, default_value);
    }

    prefs_emit_notify (prefs, key, item_value (item));
}


static void
prefs_new_key_from_string (MooPrefs     *prefs,
                           const char   *key,
                           const char   *value)
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
        prefs_new_key (prefs, key, G_TYPE_STRING, &default_val);
        item = prefs_get_item (prefs, key);
        item_set (item, &val);
    }
    else
    {
        moo_value_convert (&val, &item->value);
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

    item_set (item, value);
    prefs_emit_notify (prefs, key, item_value (item));
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

    g_hash_table_remove (prefs->priv->data, key);
    prefs_emit_notify (prefs, key, NULL);
}



static PrefsItem*
prefs_get_item (MooPrefs       *prefs,
                const char     *key)
{
    g_return_val_if_fail (key != NULL, NULL);
    return g_hash_table_lookup (prefs->priv->data, key);
}


/***************************************************************************/
/* PrefsItem
 */

static PrefsItem*
item_new (GType           type,
          const GValue   *value,
          const GValue   *default_value)
{
    PrefsItem *item;

    g_return_val_if_fail (moo_value_type_supported (type), NULL);
    g_return_val_if_fail (value && default_value, NULL);
    g_return_val_if_fail (G_VALUE_TYPE (value) == type, NULL);
    g_return_val_if_fail (G_VALUE_TYPE (default_value) == type, NULL);

    item = g_new0 (PrefsItem, 1);

    item->type = type;

    g_value_init (&item->value, type);
    g_value_copy (value, &item->value);
    g_value_init (&item->default_value, type);
    g_value_copy (default_value, &item->default_value);

    return item;
}


static void
item_set_type (PrefsItem      *item,
               GType           type)
{
    g_return_if_fail (item != NULL);
    g_return_if_fail (moo_value_type_supported (type));

    if (type != item->type)
    {
        item->type = type;
        moo_value_change_type (&item->value, type);
        moo_value_change_type (&item->default_value, type);
    }
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


static void
item_set (PrefsItem      *item,
          const GValue   *value)
{
    g_return_if_fail (item != NULL);
    g_return_if_fail (G_IS_VALUE (value));
    g_return_if_fail (item->type == G_VALUE_TYPE (value));
    g_value_copy (value, &item->value);
}


static void
item_set_default (PrefsItem      *item,
                  const GValue   *value)
{
    g_return_if_fail (item != NULL);
    g_return_if_fail (G_IS_VALUE (value));
    g_return_if_fail (item->type == G_VALUE_TYPE (value));
    g_value_copy (value, &item->default_value);
}


/***************************************************************************/
/* PrefsNotify
 */

typedef union {
    char        *key;
    EggRegex    *regex;
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

    for (l = prefs->priv->closures; l != NULL; l = l->next)
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

    prefs->priv->closures = g_list_prepend (prefs->priv->closures, closure);
    g_hash_table_insert (prefs->priv->closures_map,
                         GUINT_TO_POINTER (closure->id),
                         prefs->priv->closures);

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
    EggRegex *regex;
    Closure *closure;
    GError *err = NULL;

    closure = g_new (Closure, 1);
    closure->type = match_type;
    closure->callback = callback;
    closure->data = data;
    closure->notify = notify;
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
            egg_regex_clear (closure->pattern.regex);
            return egg_regex_match (closure->pattern.regex,
                                    key, -1, 0) > 0;

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
    if (!p.key) return;
    if (type == MOO_PREFS_MATCH_REGEX)
        egg_regex_free (p.regex);
    else
        g_free (p.key);
}


static Closure*
find_closure (MooPrefs           *prefs,
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


/***************************************************************************/
/* Loading abd saving
 */

static void
process_element (MooMarkupElement *elm)
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
            moo_prefs_new_key_from_string (key, elm->content);

#ifdef DEBUG_READWRITE
            g_print ("key: '%s', val: '%s'\n", key, elm->content);
#endif

            g_free (path);
            return;
        }
    }

    for (child = elm->children; child != NULL; child = child->next)
        if (child->type == MOO_MARKUP_ELEMENT_NODE)
            process_element (MOO_MARKUP_ELEMENT (child));
}


gboolean
moo_prefs_load (const char     *file,
                GError        **error)
{
    MooMarkupDoc *xml;
    MooMarkupNode *root;
    MooPrefs *prefs;

    prefs = instance ();

    g_return_val_if_fail (file != NULL, FALSE);

    if (!g_file_test (file, G_FILE_TEST_EXISTS))
        return TRUE;

    xml = moo_markup_parse_file (file, error);

    if (!xml)
        return FALSE;

    if (prefs->priv->xml)
    {
        g_warning ("%s: implement me", G_STRLOC);
        moo_markup_doc_unref (prefs->priv->xml);
    }

    prefs->priv->xml = xml;

    root = moo_markup_get_root_element (xml, PREFS_ROOT);

    if (!root)
        return TRUE;

    process_element (MOO_MARKUP_ELEMENT (root));

    return TRUE;
}


typedef struct {
    MooMarkupDoc  *xml;
    MooMarkupNode *root;
} Stuff;


static void
write_item (const char  *key,
            PrefsItem   *item,
            Stuff       *stuff)
{
    const char *string = NULL;

    g_return_if_fail (key != NULL && key[0] != 0);
    g_return_if_fail (item != NULL && stuff != NULL);
    g_return_if_fail (moo_value_type_supported (item->type));

    if (moo_value_equal (item_value (item), item_default_value (item)))
    {
#ifdef DEBUG_READWRITE
        g_print ("skipping '%s'\n", key);
#endif
        return;
    }

    string = moo_value_convert_to_string (item_value (item));
    g_return_if_fail (string != NULL);

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
sync_xml (void)
{
    MooPrefs *prefs = instance ();
    MooMarkupDoc *xml;
    MooMarkupNode *root;
    Stuff stuff;

    if (!prefs->priv->xml)
        prefs->priv->xml = moo_markup_doc_new ("Prefs");

    xml = prefs->priv->xml;
    root = moo_markup_get_root_element (xml, PREFS_ROOT);

    if (root)
        moo_markup_delete_node (root);

    stuff.xml = xml;
    stuff.root = NULL;

    g_hash_table_foreach (prefs->priv->data,
                          (GHFunc) write_item,
                          &stuff);
}


gboolean
moo_prefs_save (const char     *file,
                GError        **error)
{
    MooPrefs *prefs = instance ();
    MooMarkupDoc *xml;
    MooMarkupNode *node;
    gboolean empty;

    g_return_val_if_fail (file != NULL, FALSE);

    sync_xml ();

    xml = prefs->priv->xml;

    g_return_val_if_fail (xml != NULL, FALSE);

    empty = TRUE;
    for (node = xml->children; empty && node != NULL; node = node->next)
        if (MOO_MARKUP_IS_ELEMENT (node))
            empty = FALSE;

    if (empty)
    {
        if (g_file_test (file, G_FILE_TEST_EXISTS))
            if (m_unlink (file))
                g_critical ("%s: %s", G_STRLOC,
                            g_strerror (errno));
        return TRUE;
    }

    return moo_markup_save_pretty (xml, file, 2, error);
}


/***************************************************************************/
/* Helpers
 */

void
moo_prefs_new_key_bool (const char     *key,
                        gboolean        default_val)
{
    GValue val;

    g_return_if_fail (key != NULL);

    val.g_type = 0;
    g_value_init (&val, G_TYPE_BOOLEAN);
    g_value_set_boolean (&val, default_val);

    moo_prefs_new_key (key, G_TYPE_BOOLEAN, &val);
}


void
moo_prefs_new_key_int (const char     *key,
                       int             default_val)
{
    GValue val;

    g_return_if_fail (key != NULL);

    val.g_type = 0;
    g_value_init (&val, G_TYPE_INT);
    g_value_set_int (&val, default_val);

    moo_prefs_new_key (key, G_TYPE_INT, &val);
}


void
moo_prefs_new_key_enum (const char     *key,
                        GType           enum_type,
                        int             default_val)
{
    GValue val;

    g_return_if_fail (key != NULL);

    val.g_type = 0;
    g_value_init (&val, enum_type);
    g_value_set_enum (&val, default_val);

    moo_prefs_new_key (key, enum_type, &val);
}


void
moo_prefs_new_key_string (const char     *key,
                          const char     *default_val)
{
    static GValue val;

    g_return_if_fail (key != NULL);

    val.g_type = 0;
    g_value_init (&val, G_TYPE_STRING);

    g_value_set_static_string (&val, default_val);
    moo_prefs_new_key (key, G_TYPE_STRING, &val);

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
    moo_prefs_new_key (key, GDK_TYPE_COLOR, &val);
}


const char*
moo_prefs_get_string (const char *key)
{
    const GValue *val;

    val = moo_prefs_get (key);
    g_return_val_if_fail (val != NULL, NULL);

    return g_value_get_string (val);
}


const char*
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

    return moo_value_convert_to_double (value);
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
        return moo_prefs_set_string (key, NULL);

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
    moo_value_convert (&double_val, &gval);
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


GType
moo_prefs_match_type_get_type (void)
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

    g_hash_table_foreach (prefs->priv->data,
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
