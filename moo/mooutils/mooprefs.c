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
#include "mooutils/moomarkup.h"
#include "mooutils/moofileutils.h"
#include <string.h>
#include <errno.h>

#define PREFS_ROOT "Prefs"
/* #define DEBUG_READWRITE 1 */


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
    GType   type;
    GValue  value;
    GValue  default_value;
} PrefsItem;


static gboolean type_is_supported   (GType           type);
static gboolean convert_value       (const GValue   *src,
                                     GValue         *dest);
static gboolean value_equal         (const GValue   *a,
                                     const GValue   *b);

static gboolean convert_to_bool         (const GValue   *val);
static int      convert_to_int          (const GValue   *val);
static int      convert_to_enum         (const GValue   *val,
                                         GType           enum_type);
static double   convert_to_double       (const GValue   *val);
static const GdkColor *convert_to_color (const GValue   *val);
static const char *convert_to_string    (const GValue   *val);

static PrefsItem   *item_new            (GType           type,
                                         const GValue   *value,
                                         const GValue   *default_value)
{
    PrefsItem *item;

    g_return_val_if_fail (type_is_supported (type), NULL);
    g_return_val_if_fail (!G_IS_VALUE (value) ||
            G_VALUE_TYPE (value) == type, NULL);
    g_return_val_if_fail (!G_IS_VALUE (default_value) ||
            G_VALUE_TYPE (default_value) == type, NULL);

    item = g_new0 (PrefsItem, 1);
    item->type = type;

    if (value)
    {
        g_value_init (&item->value, type);
        g_value_copy (value, &item->value);
    }
    if (default_value)
    {
        g_value_init (&item->default_value, type);
        g_value_copy (default_value, &item->default_value);
    }

    return item;
}


static const GValue *item_value         (PrefsItem      *item)
{
    g_return_val_if_fail (item != NULL, NULL);
    if (G_IS_VALUE (&item->value))
        return &item->value;
    else
        return NULL;
}

static const GValue *item_default_value (PrefsItem      *item)
{
    g_return_val_if_fail (item != NULL, NULL);
    if (G_IS_VALUE (&item->default_value))
        return &item->default_value;
    else
        return NULL;
}


#define CONVERT_VALUE_TYPE(val,type)                \
G_STMT_START {                                      \
    if (G_IS_VALUE (val))                           \
    {                                               \
        GValue tmp__;                               \
        tmp__.g_type = 0;                           \
        g_value_init (&tmp__, (type));              \
        if (!convert_value (val, &tmp__))           \
        {                                           \
            g_warning ("%s: could not convert "     \
                       "old value", G_STRLOC);      \
            g_value_unset (val);                    \
        }                                           \
        else                                        \
        {                                           \
            g_value_unset (val);                    \
            g_value_init (val, type);               \
            g_value_copy (&tmp__, val);             \
            g_value_unset (&tmp__);                 \
        }                                           \
    }                                               \
} G_STMT_END


static void         item_set_type       (PrefsItem      *item,
                                         GType           type)
{
    g_return_if_fail (item != NULL);
    g_return_if_fail (type_is_supported (type));

    if (type != item->type)
    {
        item->type = type;
        CONVERT_VALUE_TYPE (&item->value, type);
        CONVERT_VALUE_TYPE (&item->default_value, type);
    }
}


static void set_val (PrefsItem      *item,
                     GValue         *dest,
                     const GValue   *val)
{
    g_return_if_fail (item != NULL);
    g_return_if_fail (!val || G_IS_VALUE (val));

    if (val)
    {
        if (G_VALUE_TYPE (val) == item->type)
        {
            if (!G_IS_VALUE (dest))
                g_value_init (dest, item->type);
            g_value_copy (val, dest);
        }
        else
        {
            g_return_if_fail (type_is_supported (G_VALUE_TYPE (val)));
            if (!G_IS_VALUE (dest))
                g_value_init (dest, item->type);
            g_return_if_fail (convert_value (val, dest));
        }
    }
    else
    {
        if (G_IS_VALUE (dest))
            g_value_unset (dest);
    }
}


static void         item_set            (PrefsItem      *item,
                                         const GValue   *new_value)
{
    set_val (item, &item->value, new_value);
}

static void         item_set_default    (PrefsItem      *item,
                                         const GValue   *new_value)
{
    set_val (item, &item->default_value, new_value);
}


static void         item_unset_value    (PrefsItem      *item)
{
    if (G_IS_VALUE (&item->value))
        g_value_unset (&item->value);
}


static void         item_free           (PrefsItem      *item)
{
    if (item)
    {
        item->type = 0;
        if (G_IS_VALUE (&item->value))
            g_value_unset (&item->value);
        if (G_IS_VALUE (&item->default_value))
            g_value_unset (&item->default_value);
        g_free (item);
    }
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
                                 const GValue       *value);


struct _MooPrefsPrivate {
    GHashTable      *data;              /* char* -> Item* */
    guint            last_notify_id;
    GList           *closures;
    GHashTable      *closures_map;      /* guint -> closures list link */
    MooMarkupDoc    *xml;
};


static void         prefs_set       (MooPrefs       *prefs,
                                     const char     *key,
                                     const GValue   *value);

static const GValue *prefs_get      (MooPrefs   *prefs,
                                     const char *key);
static PrefsItem   *prefs_get_item  (MooPrefs   *prefs,
                                     const char *key);

static void         prefs_change    (MooPrefs       *prefs,
                                     const char     *key,
                                     PrefsItem      *item,
                                     const GValue   *value);
static void         prefs_create    (MooPrefs       *prefs,
                                     const char     *key,
                                     const GValue   *value,
                                     const GValue   *default_value);
static void       prefs_create_type (MooPrefs      *prefs,
                                     const char    *key,
                                     GType          type);
static void         prefs_remove    (MooPrefs       *prefs,
                                     const char     *key);

static void         emit_notify     (MooPrefs       *prefs,
                                     const char     *key,
                                     const GValue   *value);


static void      moo_prefs_finalize (GObject *object);


static void moo_prefs_class_init (MooPrefsClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = moo_prefs_finalize;
}


static void moo_prefs_init (MooPrefs *prefs)
{
    prefs->priv = g_new0 (MooPrefsPrivate, 1);

    prefs->priv->data =
            g_hash_table_new_full (g_str_hash, g_str_equal,
                                   (GDestroyNotify) g_free,
                                   (GDestroyNotify) item_free);
    prefs->priv->last_notify_id = 0;
    prefs->priv->closures = NULL;
    prefs->priv->closures_map = g_hash_table_new (g_direct_hash, NULL);
}


static void moo_prefs_finalize (GObject *obj)
{
    MooPrefs *prefs = MOO_PREFS (obj);

    g_hash_table_destroy (prefs->priv->data);
    prefs->priv->data = NULL;

    g_hash_table_destroy (prefs->priv->closures_map);
    prefs->priv->closures_map = NULL;

    g_list_foreach (prefs->priv->closures,
                    (GFunc) closure_free,
                    NULL);
    g_list_free (prefs->priv->closures);
    prefs->priv->closures = NULL;

    if (prefs->priv->xml)
        moo_markup_doc_unref (prefs->priv->xml);
    prefs->priv->xml = NULL;

    g_free (prefs->priv);
    prefs->priv = NULL;

    G_OBJECT_CLASS(moo_prefs_parent_class)->finalize (obj);
}


static void      prefs_set      (MooPrefs       *prefs,
                                 const char     *key,
                                 const GValue   *val)
{
    g_return_if_fail (MOO_IS_PREFS (prefs));
    g_return_if_fail (key && key[0]);

    if (val)
    {
        PrefsItem *item = prefs_get_item (prefs, key);

        if (item)
            prefs_change (prefs, key, item, val);
        else
            prefs_create (prefs, key, val, NULL);
    }
    else
    {
        prefs_remove (prefs, key);
    }
}


static void      prefs_new_key  (MooPrefs       *prefs,
                                 const char     *key,
                                 GType           type,
                                 const GValue   *default_value)
{
    PrefsItem *item;

    g_return_if_fail (MOO_IS_PREFS (prefs));
    g_return_if_fail (key != NULL);
    g_return_if_fail (type_is_supported (type));
    g_return_if_fail (!G_IS_VALUE (default_value) ||
            G_VALUE_TYPE (default_value) == type);

    item = prefs_get_item (prefs, key);

    if (item)
    {
        item_set_type (item, type);
        if (default_value)
            item_set_default (item, default_value);
        if (!item_value && item_default_value (item))
            item_set (item, item_default_value (item));
        emit_notify (prefs, key, item_value (item));
    }
    else
    {
        prefs_create_type (prefs, key, type);
        if (default_value)
        {
            item = prefs_get_item (prefs, key);
            g_assert (item != NULL);
            if (default_value)
            {
                item_set_default (item, default_value);
                item_set (item, default_value);
            }
        }
    }
}


static PrefsItem    *prefs_get_item (MooPrefs   *prefs,
                                     const char *key)
{
    return g_hash_table_lookup (prefs->priv->data, key);
}


static const GValue *prefs_get  (MooPrefs   *prefs,
                                 const char *key)
{
    PrefsItem *item = prefs_get_item (prefs, key);

    if (item)
        return item_value (item);
    else
        return NULL;
}


static void      prefs_change   (MooPrefs      *prefs,
                                 const char    *key,
                                 PrefsItem     *item,
                                 const GValue  *val)
{
    item_set (item, val);
    emit_notify (prefs, key, val);
}


static void      prefs_create   (MooPrefs      *prefs,
                                 const char    *key,
                                 const GValue  *value,
                                 const GValue  *default_value)
{
    PrefsItem *item;
    GType type;

    if (G_IS_VALUE (value))
        type = G_VALUE_TYPE (value);
    else if (G_IS_VALUE (default_value))
        type = G_VALUE_TYPE (default_value);
    else
        g_return_if_reached ();

    item = item_new (type, value, default_value);
    g_hash_table_insert (prefs->priv->data,
                         g_strdup (key), item);
    emit_notify (prefs, key, value);
}


static void   prefs_create_type (MooPrefs      *prefs,
                                 const char    *key,
                                 GType          type)
{
    PrefsItem *item;

    item = item_new (type, NULL, NULL);
    g_hash_table_insert (prefs->priv->data,
                         g_strdup (key), item);
    emit_notify (prefs, key, NULL);
}


static void      prefs_remove   (MooPrefs   *prefs,
                                 const char *key)
{
    PrefsItem *item;

    item = g_hash_table_lookup (prefs->priv->data, key);

    if (item)
    {
        if (item_default_value (item))
            item_unset_value (item);
        else
            g_hash_table_remove (prefs->priv->data, key);

        emit_notify (prefs, key, NULL);
    }
}


static void      emit_notify    (MooPrefs       *prefs,
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
    if (closure)
    {
        pattern_free (closure->pattern, closure->type);
        g_free (closure);
    }
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
            g_return_val_if_reached (FALSE);
    }
}


static void      closure_invoke (Closure            *closure,
                                 const char         *key,
                                 const GValue       *value)
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


void            moo_prefs_new_key   (const char     *key,
                                     GType           value_type,
                                     const GValue   *default_value)
{
    g_return_if_fail (key != NULL);
    prefs_new_key (instance(), key, value_type, default_value);
}


const GValue   *moo_prefs_get       (const char     *key)
{
    const GValue *val;
    g_return_val_if_fail (key != NULL, NULL);
    val = prefs_get (instance(), key);
    return val;
}


void            moo_prefs_set       (const char     *key,
                                     const GValue   *value)
{
    g_return_if_fail (key != NULL);
    prefs_set (instance(), key, value);
}


/***************************************************************************/
/* Loading abd saving
 */

static void process_element (MooMarkupElement *elm)
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
            moo_prefs_set_string (key, elm->content);

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


static gboolean moo_prefs_load_old          (const char     *file);
gboolean        moo_prefs_load              (const char     *file)
{
    MooMarkupDoc *xml;
    MooMarkupElement *root;
    GError *err = NULL;
    MooPrefs *prefs;

    prefs = instance ();

    g_return_val_if_fail (file != NULL, FALSE);

    xml = moo_markup_parse_file (file, &err);

    if (!xml)
    {
        if (err)
        {
            if (err->domain == G_MARKUP_ERROR)
            {
                g_message ("%s: parse error, trying old format", G_STRLOC);
                g_error_free (err);
                return moo_prefs_load_old (file);
            }
            else
            {
                g_warning ("%s: %s", G_STRLOC, err->message);
                g_error_free (err);
            }
        }

        return FALSE;
    }

    if (prefs->priv->xml)
    {
        g_warning ("%s: implement me", G_STRLOC);
        moo_markup_doc_unref (prefs->priv->xml);
    }

    prefs->priv->xml = xml;

    root = moo_markup_get_root_element (xml, PREFS_ROOT);

    if (!root)
    {
        g_warning ("%s: no " PREFS_ROOT " element", G_STRLOC);
        return TRUE;
    }

    process_element (root);

    return TRUE;
}


typedef struct {
    MooMarkupDoc     *xml;
    MooMarkupElement *root;
} Stuff;

static void write_item (const char  *key,
                        PrefsItem   *item,
                        Stuff       *stuff)
{
    gboolean save = FALSE;
    const char *string = NULL;

    g_return_if_fail (key != NULL && key[0] != 0);
    g_return_if_fail (item != NULL && stuff != NULL);
    g_return_if_fail (type_is_supported (item->type));

    if (item_value (item))
    {
        if (!item_default_value (item))
            save = TRUE;
        else
            save = !value_equal (item_value (item),
                                 item_default_value (item));
    }

    if (!save)
    {
#ifdef DEBUG_READWRITE
        g_print ("skipping '%s'\n", key);
#endif
        return;
    }

    string = convert_to_string (item_value (item));
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

static void     sync_xml    (void)
{
    MooPrefs *prefs = instance ();
    MooMarkupDoc *xml;
    MooMarkupElement *root;
    Stuff stuff;

    if (!prefs->priv->xml)
        prefs->priv->xml = moo_markup_doc_new ("Prefs");

    xml = prefs->priv->xml;
    root = moo_markup_get_root_element (xml, PREFS_ROOT);

    if (root)
        moo_markup_delete_node (MOO_MARKUP_NODE (root));

    stuff.xml = xml;
    stuff.root = NULL;

    g_hash_table_foreach (prefs->priv->data,
                          (GHFunc) write_item,
                          &stuff);
}


#define INDENT_SIZE 2
#define INDENT_CHAR ' '

#ifdef __WIN32__
#define LINE_SEPARATOR "\r\n"
#elif defined(OS_DARWIN)
#define LINE_SEPARATOR "\r"
#else
#define LINE_SEPARATOR "\n"
#endif

static void format_element (MooMarkupElement *elm,
                            GString          *str,
                            guint             indent)
{
    gboolean isdir = FALSE;
    gboolean empty = TRUE;
    MooMarkupNode *child;
    char *fill;
    guint i;

    g_return_if_fail (MOO_MARKUP_IS_ELEMENT (elm));
    g_return_if_fail (str != NULL);

    for (child = elm->children; child != NULL; child = child->next)
    {
        if (elm->content)
            empty = FALSE;

        if (MOO_MARKUP_IS_ELEMENT (child))
        {
            isdir = TRUE;
            empty = FALSE;
            break;
        }
    }

    fill = g_strnfill (indent, INDENT_CHAR);

    g_string_append_len (str, fill, indent);
    g_string_append_printf (str, "<%s", elm->name);
    for (i = 0; i < elm->n_attrs; ++i)
        g_string_append_printf (str, " %s=\"%s\"",
                                elm->attr_names[i],
                                elm->attr_vals[i]);

    if (empty)
    {
        g_string_append (str, "/>" LINE_SEPARATOR);
    }
    else if (isdir)
    {
        g_string_append (str, ">" LINE_SEPARATOR);

        for (child = elm->children; child != NULL; child = child->next)
            if (MOO_MARKUP_IS_ELEMENT (child))
                format_element (MOO_MARKUP_ELEMENT (child), str,
                                indent + INDENT_SIZE);

        g_string_append_printf (str, "%s</%s>" LINE_SEPARATOR,
                                fill, elm->name);
    }
    else
    {
        char *escaped = g_markup_escape_text (elm->content, -1);
        g_string_append_printf (str, ">%s</%s>" LINE_SEPARATOR,
                                escaped, elm->name);
        g_free (escaped);
    }

    g_free (fill);
}

static char *format_xml (MooMarkupDoc *doc)
{
    GString *str = NULL;
    MooMarkupNode *child;

    for (child = doc->children; child != NULL; child = child->next)
    {
        if (MOO_MARKUP_IS_ELEMENT (child))
        {
            if (!str) str = g_string_new ("");
            format_element (MOO_MARKUP_ELEMENT (child), str, 0);
        }
    }

    if (str)
        return g_string_free (str, FALSE);
    else
        return NULL;
}


gboolean        moo_prefs_save              (const char     *file)
{
    MooPrefs *prefs = instance ();
    MooMarkupDoc *xml;
    MooMarkupNode *node;
    gboolean empty;
    GError *err = NULL;
    char *text;
    gboolean result;

    g_return_val_if_fail (file != NULL, FALSE);

    sync_xml ();

    xml = prefs->priv->xml;

    g_return_val_if_fail (xml != NULL, FALSE);

    empty = TRUE;
    for (node = xml->children; empty && node != NULL; node = node->next)
        if (MOO_MARKUP_IS_ELEMENT (node))
            empty = FALSE;

    if (empty)
        return TRUE;

    text = format_xml (xml);

    if (text)
    {
        result = moo_save_file_utf8 (file, text, -1, &err);

        if (err)
        {
            g_critical ("%s: %s", G_STRLOC, err->message);
            g_error_free (err);
        }
    }
    else if (moo_unlink (file))
    {
        g_critical ("%s: %s", G_STRLOC,
                    g_strerror (errno));
    }

    g_free (text);
    return result;
}


static gboolean moo_prefs_load_old          (const char     *file)
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
            {
                char **pieces = g_strsplit (keyval[0], "::", 0);
                guint j;

                if (!pieces || !pieces[0])
                {
                    g_critical ("%s: error in file '%s' "
                                "on line %d", G_STRLOC, file, i);
                }
                else
                {
                    char *key;

                    for (j = 0; pieces[j] != NULL; ++j)
                    {
                        const char *replacement = NULL;

                        if (j == 0 && !strcmp (pieces[j], "terminal"))
                            replacement = "Terminal";
                        else if (!strcmp (pieces[j], "window_save_position"))
                            replacement = "window/save_position";
                        else if (!strcmp (pieces[j], "window_save_size"))
                            replacement = "window/save_size";
                        else if (!strcmp (pieces[j], "window_width"))
                            replacement = "window/width";
                        else if (!strcmp (pieces[j], "window_height"))
                            replacement = "window/height";
                        else if (!strcmp (pieces[j], "window_show_toolbar"))
                            replacement = "window/show_toolbar";
                        else if (!strcmp (pieces[j], "window_show_menubar"))
                            replacement = "window/show_menubar";
                        else if (!strcmp (pieces[j], "window_toolbar_style"))
                            replacement = "window/toolbar_style";
                        else if (!strcmp (pieces[j], "window_x"))
                            replacement = "window/x";
                        else if (!strcmp (pieces[j], "window_y"))
                            replacement = "window/y";

                        if (replacement)
                        {
                            g_free (pieces[j]);
                            pieces[j] = g_strdup (replacement);
                        }
                    }

                    key = g_strjoinv ("/", pieces);
                    moo_prefs_set_string (key, keyval[1]);
#ifdef DEBUG_READWRITE
                    g_print ("key: '%s', val: '%s'\n",
                             key, keyval[1]);
#endif
                    g_free (key);
                }

                g_strfreev (pieces);
            }
            else
            {
                g_critical ("%s: error in file '%s' "
                        "on line %d", G_STRLOC, file, i);
            }
        }

        g_strfreev (keyval);
    }

    g_free (content);
    g_strfreev (lines);
    return TRUE;
}


/***************************************************************************/
/* Helpers
 */


void    moo_prefs_new_key_bool      (const char     *key,
                                     gboolean        default_val)
{
    GValue val;

    g_return_if_fail (key != NULL);

    val.g_type = 0;
    g_value_init (&val, G_TYPE_BOOLEAN);
    g_value_set_boolean (&val, default_val);

    moo_prefs_new_key (key, G_TYPE_BOOLEAN, &val);
}


void    moo_prefs_new_key_int       (const char     *key,
                                     int             default_val)
{
    GValue val;

    g_return_if_fail (key != NULL);

    val.g_type = 0;
    g_value_init (&val, G_TYPE_INT);
    g_value_set_int (&val, default_val);

    moo_prefs_new_key (key, G_TYPE_INT, &val);
}


void    moo_prefs_new_key_enum      (const char     *key,
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


void    moo_prefs_new_key_string    (const char     *key,
                                     const char     *default_val)
{
    g_return_if_fail (key != NULL);

    if (default_val)
    {
        GValue val;

        val.g_type = 0;
        g_value_init (&val, G_TYPE_STRING);
        g_value_set_static_string (&val, default_val);

        moo_prefs_new_key (key, G_TYPE_STRING, &val);
    }
    else
    {
        moo_prefs_new_key (key, G_TYPE_STRING, NULL);
    }
}


void    moo_prefs_new_key_color     (const char     *key,
                                     const GdkColor *default_val)
{
    g_return_if_fail (key != NULL);

    if (default_val)
    {
        GValue val;

        val.g_type = 0;
        g_value_init (&val, GDK_TYPE_COLOR);
        g_value_set_boxed (&val, default_val);

        moo_prefs_new_key (key, GDK_TYPE_COLOR, &val);
    }
    else
    {
        moo_prefs_new_key (key, GDK_TYPE_COLOR, NULL);
    }
}


const char     *moo_prefs_get_string        (const char     *key)
{
    const GValue *val;
    g_return_val_if_fail (key != NULL, NULL);
    val = moo_prefs_get (key);
    if (val)
    {
        g_return_val_if_fail (G_VALUE_HOLDS (val, G_TYPE_STRING), NULL);
        return g_value_get_string (val);
    }
    else
    {
        return NULL;
    }
}


gboolean        moo_prefs_get_bool          (const char     *key)
{
    const GValue *val;
    g_return_val_if_fail (key != NULL, FALSE);
    val = moo_prefs_get (key);
    if (val)
        return convert_to_bool (val);
    else
        return FALSE;
}


gdouble         moo_prefs_get_double        (const char     *key)
{
    const GValue *val;
    g_return_val_if_fail (key != NULL, FALSE);
    val = moo_prefs_get (key);
    if (val)
        return convert_to_double (val);
    else
        return 0;
}


const GdkColor *moo_prefs_get_color         (const char     *key)
{
    const GValue *val;
    g_return_val_if_fail (key != NULL, FALSE);
    val = moo_prefs_get (key);
    if (val)
        return convert_to_color (val);
    else
        return NULL;
}


int             moo_prefs_get_int           (const char     *key)
{
    const GValue *val;
    g_return_val_if_fail (key != NULL, FALSE);
    val = moo_prefs_get (key);
    if (val)
        return convert_to_int (val);
    else
        return 0;
}


int             moo_prefs_get_enum          (const char     *key,
                                             GType           enum_type)
{
    const GValue *val;
    g_return_val_if_fail (key != NULL, FALSE);
    val = moo_prefs_get (key);
    if (val)
        return convert_to_enum (val, enum_type);
    else
        g_return_val_if_reached (0);
}


void            moo_prefs_set_string        (const char     *key,
                                             const char     *val)
{
    g_return_if_fail (key != NULL);

    if (val)
    {
        GValue gval;
        gval.g_type = 0;
        g_value_init (&gval, G_TYPE_STRING);
        g_value_set_string (&gval, val);
        moo_prefs_set (key, &gval);
        g_value_unset (&gval);
    }
    else
    {
        moo_prefs_set (key, NULL);
    }
}


void            moo_prefs_set_double        (const char     *key,
                                             double          val)
{
    static GValue gval;
    g_return_if_fail (key != NULL);
    if (!G_IS_VALUE (&gval))
        g_value_init (&gval, G_TYPE_DOUBLE);
    g_value_set_double (&gval, val);
    moo_prefs_set (key, &gval);
}


void            moo_prefs_set_int           (const char     *key,
                                             int             val)
{
    static GValue gval;
    g_return_if_fail (key != NULL);
    if (!G_IS_VALUE (&gval))
        g_value_init (&gval, G_TYPE_INT);
    g_value_set_int (&gval, val);
    moo_prefs_set (key, &gval);
}


void            moo_prefs_set_bool          (const char     *key,
                                             gboolean        val)
{
    static GValue gval;
    g_return_if_fail (key != NULL);
    if (!G_IS_VALUE (&gval))
        g_value_init (&gval, G_TYPE_BOOLEAN);
    g_value_set_boolean (&gval, val);
    moo_prefs_set (key, &gval);
}


void            moo_prefs_set_color         (const char     *key,
                                             const GdkColor *val)
{
    static GValue gval;
    g_return_if_fail (key != NULL);
    gval.g_type = 0;
    if (!G_IS_VALUE (&gval))
        g_value_init (&gval, GDK_TYPE_COLOR);
    g_value_set_boxed (&gval, val);
    moo_prefs_set (key, &gval);
}


void            moo_prefs_set_enum          (const char     *key,
                                             GType           type,
                                             int             value)
{
    GValue gval;
    g_return_if_fail (key != NULL);
    g_return_if_fail (G_TYPE_IS_ENUM (type));
    gval.g_type = 0;
    g_value_init (&gval, type);
    g_value_set_enum (&gval, value);
    moo_prefs_set (key, &gval);
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


static gboolean convert_to_bool         (const GValue   *val)
{
    GValue result;
    result.g_type = 0;
    g_value_init (&result, G_TYPE_BOOLEAN);
    g_return_val_if_fail (convert_value (val, &result), FALSE);
    return g_value_get_boolean (&result);
}


static int      convert_to_int          (const GValue   *val)
{
    GValue result;
    result.g_type = 0;
    g_value_init (&result, G_TYPE_INT);
    g_return_val_if_fail (convert_value (val, &result), 0);
    return g_value_get_int (&result);
}


static int      convert_to_enum         (const GValue   *val,
                                         GType           enum_type)
{
    GValue result;
    result.g_type = 0;
    g_value_init (&result, enum_type);
    g_return_val_if_fail (convert_value (val, &result), 0);
    return g_value_get_enum (&result);
}


static double   convert_to_double       (const GValue   *val)
{
    GValue result;
    result.g_type = 0;
    g_value_init (&result, G_TYPE_DOUBLE);
    g_return_val_if_fail (convert_value (val, &result), 0);
    return g_value_get_double (&result);
}


static const GdkColor *convert_to_color (const GValue   *val)
{
    static GdkColor color;
    GdkColor *c_result;
    GValue result;
    result.g_type = 0;
    g_value_init (&result, GDK_TYPE_COLOR);
    g_return_val_if_fail (convert_value (val, &result), NULL);
    c_result = g_value_get_boxed (&result);
    g_return_val_if_fail (c_result != NULL, NULL);
    color = *c_result;
    g_value_unset (&result);
    return &color;
}


static const char *convert_to_string    (const GValue   *val)
{
    static GValue result;
    if (G_IS_VALUE (&result))
        g_value_unset (&result);
    g_value_init (&result, G_TYPE_STRING);
    g_return_val_if_fail (convert_value (val, &result), NULL);
    return g_value_get_string (&result);
}


static gboolean type_is_supported   (GType           type)
{
    if (!type)
        return FALSE;

    return type == G_TYPE_BOOLEAN ||
            type == G_TYPE_INT ||
            type == G_TYPE_DOUBLE ||
            type == G_TYPE_STRING ||
            type == GDK_TYPE_COLOR ||
            G_TYPE_IS_ENUM (type);
}


static int      value_equal         (const GValue   *a,
                                     const GValue   *b)
{
    GType type;

    g_return_val_if_fail (G_IS_VALUE (a) && G_IS_VALUE (b), a == b);
    g_return_val_if_fail (G_VALUE_TYPE (a) == G_VALUE_TYPE (b), a == b);
    g_return_val_if_fail (G_VALUE_TYPE (a) == G_VALUE_TYPE (b), a == b);

    type = G_VALUE_TYPE (a);

    if (type == G_TYPE_BOOLEAN)
    {
        gboolean ba = g_value_get_boolean (a);
        gboolean bb = g_value_get_boolean (b);
        return (ba && bb) || (!ba && !bb);
    }

    if (type == G_TYPE_INT)
        return g_value_get_int (a) == g_value_get_int (b);

    if (type == G_TYPE_DOUBLE)
        return g_value_get_double (a) == g_value_get_double (b);

    if (type == G_TYPE_STRING)
    {
        const char *sa, *sb;
        sa = g_value_get_string (a);
        sb = g_value_get_string (b);
        if (!sa || !sb)
            return sa == sb;
        else
            return !strcmp (sa, sb);
    }

    if (type == GDK_TYPE_COLOR)
    {
        const GdkColor *ca, *cb;
        ca = g_value_get_boxed (a);
        cb = g_value_get_boxed (b);
        if (!ca || !cb)
            return ca == cb;
        else
            return ca->red == cb->red &&
                    ca->green == cb->green &&
                    ca->blue == cb->blue;
    }

    if (G_TYPE_IS_ENUM (type))
        return g_value_get_enum (a) == g_value_get_enum (b);

    g_return_val_if_reached (a == b);
}


static gboolean convert_value       (const GValue   *src,
                                     GValue         *dest)
{
    GType src_type, dest_type;

    g_return_val_if_fail (G_IS_VALUE (src) && G_IS_VALUE (dest), FALSE);

    src_type = G_VALUE_TYPE (src);
    dest_type = G_VALUE_TYPE (dest);

    g_return_val_if_fail (type_is_supported (src_type), FALSE);
    g_return_val_if_fail (type_is_supported (dest_type), FALSE);

    if (src_type == dest_type)
    {
        g_value_copy (src, dest);
        return TRUE;
    }

    if (dest_type == G_TYPE_STRING)
    {
        if (src_type == G_TYPE_BOOLEAN)
        {
            const char *string =
                    g_value_get_boolean (src) ? "TRUE" : "FALSE";
            g_value_set_static_string (dest, string);
            return TRUE;
        }

        if (src_type == G_TYPE_DOUBLE)
        {
            char *string =
                    g_strdup_printf ("%f", g_value_get_double (src));
            g_value_take_string (dest, string);
            return TRUE;
        }

        if (src_type == G_TYPE_INT)
        {
            char *string =
                    g_strdup_printf ("%d", g_value_get_int (src));
            g_value_take_string (dest, string);
            return TRUE;
        }

        if (src_type == GDK_TYPE_COLOR)
        {
            char string[14];
            const GdkColor *color = g_value_get_boxed (src);

            if (!color)
            {
                g_value_set_string (dest, NULL);
                return TRUE;
            }
            else
            {
                g_snprintf (string, 8, "#%02x%02x%02x",
                            color->red >> 8,
                            color->green >> 8,
                            color->blue >> 8);
                g_value_set_string (dest, string);
                return TRUE;
            }
        }

        if (G_TYPE_IS_ENUM (src_type))
        {
            gpointer klass;
            GEnumClass *enum_class;
            GEnumValue *enum_value;

            klass = g_type_class_ref (src_type);
            g_return_val_if_fail (G_IS_ENUM_CLASS (klass), FALSE);
            enum_class = G_ENUM_CLASS (klass);

            enum_value = g_enum_get_value (enum_class,
                                           g_value_get_enum (src));

            if (!enum_value)
            {
                char *string = g_strdup_printf ("%d", g_value_get_enum (src));
                g_value_take_string (dest, string);
                g_type_class_unref (klass);
                g_return_val_if_reached (TRUE);
            }

            g_value_set_static_string (dest, enum_value->value_nick);
            g_type_class_unref (klass);
            return TRUE;
        }

        g_return_val_if_reached (FALSE);
    }

    if (src_type == G_TYPE_STRING)
    {
        const char *string = g_value_get_string (src);

        if (dest_type == G_TYPE_BOOLEAN)
        {
            if (!string)
                g_value_set_boolean (dest, FALSE);
            else
                g_value_set_boolean (dest,
                                     ! g_ascii_strcasecmp (string, "1") ||
                                             ! g_ascii_strcasecmp (string, "yes") ||
                                             ! g_ascii_strcasecmp (string, "true"));
            return TRUE;
        }

        if (dest_type == G_TYPE_DOUBLE)
        {
            if (!string)
                g_value_set_double (dest, 0);
            else
                g_value_set_double (dest, g_ascii_strtod (string, NULL));
            return TRUE;
        }

        if (dest_type == G_TYPE_INT)
        {
            if (!string)
                g_value_set_int (dest, 0);
            else
                g_value_set_int (dest, g_ascii_strtod (string, NULL));
            return TRUE;
        }

        if (dest_type == GDK_TYPE_COLOR)
        {
            GdkColor color;

            if (!string)
            {
                g_value_set_boxed (dest, NULL);
                return TRUE;
            }

            g_return_val_if_fail (gdk_color_parse (string, &color),
                                  FALSE);

            g_value_set_boxed (dest, &color);
            return TRUE;
        }

        if (G_TYPE_IS_ENUM (dest_type))
        {
            gpointer klass;
            GEnumClass *enum_class;
            GEnumValue *enum_value;
            int ival;

            if (!string || !string[0])
            {
                g_value_set_enum (dest, 0);
                g_return_val_if_reached (TRUE);
            }

            klass = g_type_class_ref (dest_type);
            g_return_val_if_fail (G_IS_ENUM_CLASS (klass), FALSE);
            enum_class = G_ENUM_CLASS (klass);

            enum_value = g_enum_get_value_by_name (enum_class, string);
            if (!enum_value)
                enum_value = g_enum_get_value_by_nick (enum_class, string);

            if (enum_value)
            {
                ival = enum_value->value;
            }
            else
            {
                ival = g_ascii_strtod (string, NULL);

                if (ival < enum_class->minimum || ival > enum_class->maximum)
                {
                    g_value_set_enum (dest, ival);
                    g_type_class_unref (klass);
                    g_return_val_if_reached (TRUE);
                }
            }

            g_value_set_enum (dest, ival);
            g_type_class_unref (klass);
            return TRUE;
        }

        g_return_val_if_reached (FALSE);
    }

    if (G_TYPE_IS_ENUM (src_type) && dest_type == G_TYPE_INT)
    {
        g_value_set_int (dest, g_value_get_enum (src));
        return TRUE;
    }

    if (G_TYPE_IS_ENUM (dest_type) && src_type == G_TYPE_INT)
    {
        g_value_set_enum (dest, g_value_get_int (src));
        return TRUE;
    }

    if (src_type == G_TYPE_DOUBLE && dest_type == G_TYPE_INT)
    {
        g_value_set_int (dest, g_value_get_double (src));
        return TRUE;
    }

    if (dest_type == G_TYPE_DOUBLE && src_type == G_TYPE_INT)
    {
        g_value_set_double (dest, g_value_get_int (src));
        return TRUE;
    }

    g_return_val_if_reached (FALSE);
}
