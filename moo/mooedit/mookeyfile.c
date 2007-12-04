/*
 *   mookeyfile.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#include "mooedit/mookeyfile.h"
#include "mooutils/mooutils-gobject.h"
#include "mooutils/mooutils-misc.h"
#include <string.h>


typedef enum {
    MOO_KEY_FILE_ERROR_PARSE,
    MOO_KEY_FILE_ERROR_FILE
} MooKeyFileError;

struct _MooKeyFile {
    guint ref_count;
    GQueue *items;
};

struct _MooKeyFileItem {
    MooKeyFile *key_file;
    char *name;
    char *content;
    GHashTable *keys;
};

typedef struct {
    const char *ptr;
    gsize len;
    const char *file;
    guint line_no;
    GError *error;
    MooKeyFile *key_file;
} Parser;

#define CHAR_IS_SPACE(c__) ((c__) == ' ' || (c__) == '\t')

static gboolean          moo_key_file_parse_file    (MooKeyFile         *key_file,
                                                     const char         *filename,
                                                     GError            **error);

static MooKeyFileItem   *key_file_item_new          (MooKeyFile         *key_file,
                                                     const char         *name);
static void              key_file_item_free         (MooKeyFileItem     *item);
static void              key_file_item_take_content (MooKeyFileItem     *item,
                                                     char               *content);

static void              get_line                   (const char         *line,
                                                     gsize               max_len,
                                                     gsize              *line_len,
                                                     gsize              *next_line);
static gboolean          line_is_empty_or_comment   (const char         *line,
                                                     gsize               line_len);
static gboolean          line_is_indented           (const char         *line,
                                                     gsize               line_len);
static gboolean          line_is_item               (const char         *line,
                                                     gsize               line_len,
                                                     char              **item_name);
static gboolean          parse_item                 (Parser             *parser,
                                                     MooKeyFileItem     *item);
static void              parser_next_line           (Parser             *parser,
                                                     gsize               offset);
static gboolean          find_and_parse_item        (Parser             *parser);
static gboolean          parse_content              (Parser             *parser,
                                                     MooKeyFileItem     *item);
static gboolean          line_is_key_val            (const char         *line,
                                                     gsize               line_len,
                                                     char              **key,
                                                     char              **val);
static guint             get_indent                 (const char         *line,
                                                     gsize               line_len);
static gboolean          line_is_blank              (const char         *line,
                                                     gsize               line_len,
                                                     guint              *indent);

#define MOO_KEY_FILE_ERROR            (moo_key_file_error_quark ())
static GQuark            moo_key_file_error_quark   (void) G_GNUC_CONST;


static void
get_line (const char *line,
          gsize       max_len,
          gsize      *line_len,
          gsize      *next_line)
{
    gsize i;

    for (i = 0; i < max_len; ++i)
    {
        if (line[i] == '\n')
        {
            *line_len = i;
            *next_line = i + 1;
            break;
        }
        else if (line[i] == '\r')
        {
            *line_len = i;
            if (i + 1 < max_len && line[i+1] == '\n')
                *next_line = i + 2;
            else
                *next_line = i + 1;
            break;
        }
    }

    if (i == max_len)
    {
        *line_len = max_len;
        *next_line = max_len;
    }
    else
    {
        *next_line = MIN (*next_line, max_len);
    }
}

static gboolean
line_is_empty_or_comment (const char *line,
                          gsize       line_len)
{
    return line_len == 0 || line[0] == '#';
}

static guint
get_indent (const char *line,
            gsize       line_len)
{
    guint i;

    for (i = 0; i < line_len; ++i)
        if (!CHAR_IS_SPACE (line[i]))
            break;

    return i;
}

static gboolean
line_is_indented (const char *line,
                  gsize       line_len)
{
    return line_len != 0 && CHAR_IS_SPACE (*line);
}

static gboolean
line_is_blank (const char *line,
               gsize       line_len,
               guint      *indent)
{
    guint i;

    *indent = line_len;

    for (i = 0; i < line_len; ++i)
    {
        if (!CHAR_IS_SPACE (line[i]))
        {
            *indent = i;
            return FALSE;
        }
    }

    return TRUE;
}

static gboolean
line_is_item (const char *line,
              gsize       line_len,
              char      **item_name)
{
    gsize i;
    gsize bracket = line_len;

    if (!line_len || line[0] != '[')
        return FALSE;

    for (i = 1; i < line_len && bracket == line_len; ++i)
        if (line[i] == ']')
            bracket = i;

    if (bracket == line_len || bracket == 1)
        return FALSE;

    for (i = bracket + 1; i < line_len; ++i)
    {
        if (line[i] == '#')
            break;

        if (!CHAR_IS_SPACE (line[i]))
            return FALSE;
    }

    *item_name = g_strndup (line + 1, bracket - 1);
    return TRUE;
}

static gboolean
line_is_key_val (const char *line,
                 gsize       line_len,
                 char      **key_p,
                 char      **val_p)
{
    gsize i;
    gsize equal = line_len;
    char *key, *val;

    for (i = 0; i < line_len && equal == line_len; ++i)
        if (line[i] == '=')
            equal = i;

    if (equal == line_len || equal == 0)
        return FALSE;

    key = g_strstrip (g_strndup (line, equal));
    val = g_strstrip (g_strndup (line + equal + 1, line_len - equal - 1));

    if (!*key)
    {
        g_free (key);
        g_free (val);
        return FALSE;
    }

    *key_p = key;
    *val_p = val;
    return TRUE;
}

static void
parser_next_line (Parser *parser,
                  gsize   offset)
{
    g_assert (offset <= parser->len);
    parser->ptr += offset;
    parser->len -= offset;
    parser->line_no += 1;
}

static gboolean
parse_content (Parser         *parser,
               MooKeyFileItem *item)
{
    GString *content;
    const char *line;
    guint indent;
    gsize line_len, next_line;

    g_assert (parser->len > 0);

    content = g_string_new (NULL);

    line = parser->ptr;
    get_line (line, parser->len, &line_len, &next_line);
    indent = get_indent (line, line_len);
    g_assert (indent > 0);

    g_string_append_len (content, line + indent, line_len - indent);
    parser_next_line (parser, next_line);

    while (parser->len)
    {
        guint indent_here;

        line = parser->ptr;
        get_line (line, parser->len, &line_len, &next_line);

        line_is_blank (line, line_len, &indent_here);

        if (!indent_here)
            break;

        if (indent_here < indent)
        {
            char *text = g_strndup (line, line_len);
            g_set_error (&parser->error, MOO_KEY_FILE_ERROR,
                         MOO_KEY_FILE_ERROR_PARSE,
                         "wrong indentation in file %s at line %u: %s",
                         parser->file, parser->line_no, text);
            g_free (text);
            goto error;
        }

        g_string_append (content, "\n");
        g_string_append_len (content, line + indent, line_len - indent);

        parser_next_line (parser, next_line);
    }

    key_file_item_take_content (item, g_string_free (content, FALSE));
    return find_and_parse_item (parser);

error:
    g_string_free (content, TRUE);
    return FALSE;
}

static gboolean
parse_item (Parser         *parser,
            MooKeyFileItem *item)
{
    while (parser->len)
    {
        gsize line_len, next_line;
        const char *line;
        char *item_name, *key, *val;

        line = parser->ptr;
        get_line (line, parser->len, &line_len, &next_line);

        if (line_is_empty_or_comment (line, line_len))
        {
            parser_next_line (parser, next_line);
            continue;
        }

        if (line_is_indented (line, line_len))
        {
            if (!parse_content (parser, item))
                return FALSE;
            else
                return find_and_parse_item (parser);
        }

        if (line_is_item (line, line_len, &item_name))
        {
            item = moo_key_file_new_item (parser->key_file, item_name);
            parser_next_line (parser, next_line);
            g_free (item_name);
            return parse_item (parser, item);
        }

        if (!line_is_key_val (line, line_len, &key, &val))
        {
            char *text = g_strndup (line, line_len);
            g_set_error (&parser->error, MOO_KEY_FILE_ERROR,
                         MOO_KEY_FILE_ERROR_PARSE,
                         "unexpected text in file %s at line %u: %s",
                         parser->file, parser->line_no, text);
            g_free (text);
            return FALSE;
        }

        moo_key_file_item_set (item, key, val);
        parser_next_line (parser, next_line);

        g_free (key);
        g_free (val);
    }

    return TRUE;
}

static gboolean
find_and_parse_item (Parser *parser)
{
    while (parser->len)
    {
        gsize line_len, next_line;
        const char *line;
        char *item_name;
        MooKeyFileItem *item;

        line = parser->ptr;
        get_line (line, parser->len, &line_len, &next_line);

        if (line_is_empty_or_comment (line, line_len))
        {
            parser_next_line (parser, next_line);
            continue;
        }

        if (line_is_indented (line, line_len))
        {
            g_set_error (&parser->error, MOO_KEY_FILE_ERROR,
                         MOO_KEY_FILE_ERROR_PARSE,
                         "unexpected indented block in file %s at line %u",
                         parser->file, parser->line_no);
            return FALSE;
        }

        if (!line_is_item (line, line_len, &item_name))
        {
            char *text = g_strndup (line, line_len);
            g_set_error (&parser->error, MOO_KEY_FILE_ERROR,
                         MOO_KEY_FILE_ERROR_PARSE,
                         "unexpected text in file %s at line %u: %s",
                         parser->file, parser->line_no, text);
            g_free (text);
            return FALSE;
        }

        item = moo_key_file_new_item (parser->key_file, item_name);
        parser_next_line (parser, next_line);
        g_free (item_name);
        return parse_item (parser, item);
    }

    return TRUE;
}

static gboolean
parse_buffer (MooKeyFile *key_file,
              const char *string,
              gssize      len,
              const char *file,
              GError    **error)
{
    Parser parser;
    gboolean result;

    g_return_val_if_fail (key_file != NULL, FALSE);
    g_return_val_if_fail (string != NULL, FALSE);

    if (len < 0)
        len = strlen (string);

    parser.key_file = key_file;
    parser.error = NULL;
    parser.len = len;
    parser.ptr = string;
    parser.line_no = 1;
    parser.file = file ? file : "<memory>";

    result = find_and_parse_item (&parser);

    if (parser.error)
        g_propagate_error (error, parser.error);

    return result;
}

#if 0
static gboolean
moo_key_file_parse_buffer (MooKeyFile *key_file,
                           const char *string,
                           gssize      len,
                           GError    **error)
{
    g_return_val_if_fail (key_file != NULL, FALSE);
    g_return_val_if_fail (string != NULL, FALSE);
    return parse_buffer (key_file, string, len, NULL, error);
}
#endif


MooKeyFile *
moo_key_file_ref (MooKeyFile *key_file)
{
    g_return_val_if_fail (key_file != NULL, NULL);
    key_file->ref_count++;
    return key_file;
}


void
moo_key_file_unref (MooKeyFile *key_file)
{
    g_return_if_fail (key_file != NULL);

    if (--key_file->ref_count)
        return;

    g_queue_foreach (key_file->items, (GFunc) key_file_item_free, NULL);
    g_queue_free (key_file->items);

    g_free (key_file);
}


static MooKeyFileItem *
key_file_item_new (MooKeyFile *key_file,
                   const char *name)
{
    MooKeyFileItem *item = g_new0 (MooKeyFileItem, 1);

    item->key_file = key_file;
    item->name = g_strdup (name);
    item->content = NULL;
    item->keys = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

    return item;
}


static void
key_file_item_free (MooKeyFileItem *item)
{
    if (item)
    {
        g_free (item->name);
        g_free (item->content);
        g_hash_table_destroy (item->keys);
        g_free (item);
    }
}


MooKeyFile *
moo_key_file_new (void)
{
    MooKeyFile *key_file = g_new0 (MooKeyFile, 1);
    key_file->ref_count = 1;
    key_file->items = g_queue_new ();
    return key_file;
}


MooKeyFile *
moo_key_file_new_from_file (const char  *filename,
                            GError     **error)
{
    MooKeyFile *key_file;

    g_return_val_if_fail (filename != NULL, NULL);

    key_file = moo_key_file_new ();

    if (!moo_key_file_parse_file (key_file, filename, error))
    {
        moo_key_file_unref (key_file);
        key_file = NULL;
    }

    return key_file;
}


#if 0
MooKeyFile *
moo_key_file_new_from_buffer (const char  *string,
                              gssize       len,
                              GError     **error)
{
    MooKeyFile *key_file;

    g_return_val_if_fail (string != NULL, NULL);

    key_file = moo_key_file_new ();

    if (!moo_key_file_parse_buffer (key_file, string, len, error))
    {
        moo_key_file_unref (key_file);
        key_file = NULL;
    }

    return key_file;
}
#endif


static gboolean
moo_key_file_parse_file (MooKeyFile  *key_file,
                         const char  *filename,
                         GError     **error)
{
    char *contents;
    gsize len;
    gboolean result;

    g_return_val_if_fail (key_file != NULL, FALSE);
    g_return_val_if_fail (filename != NULL, FALSE);

    if (!g_file_get_contents (filename, &contents, &len, error))
        return FALSE;

    result = parse_buffer (key_file, contents, len, filename, error);

    g_free (contents);
    return result;
}


guint
moo_key_file_n_items (MooKeyFile *key_file)
{
    g_return_val_if_fail (key_file != NULL, 0);
    return key_file->items->length;
}


MooKeyFileItem *
moo_key_file_nth_item (MooKeyFile *key_file,
                       guint       n)
{
    g_return_val_if_fail (key_file != NULL, NULL);
    g_return_val_if_fail (n < key_file->items->length, NULL);
    return g_queue_peek_nth (key_file->items, n);
}


MooKeyFileItem *
moo_key_file_new_item (MooKeyFile *key_file,
                       const char *name)
{
    MooKeyFileItem *item;

    g_return_val_if_fail (key_file != NULL, NULL);

    item = key_file_item_new (key_file, name);
    g_queue_push_nth (key_file->items, item, -1);

    return item;
}


#if 0
void
moo_key_file_delete_item (MooKeyFile *key_file,
                          guint       index)
{
    MooKeyFileItem *item;

    g_return_if_fail (key_file != NULL);
    g_return_if_fail (index < key_file->items->length);

    item = g_queue_pop_nth (key_file->items, index);
    key_file_item_free (item);
}
#endif


const char *
moo_key_file_item_name (MooKeyFileItem *item)
{
    g_return_val_if_fail (item != NULL, NULL);
    return item->name;
}


const char *
moo_key_file_item_get (MooKeyFileItem *item,
                       const char     *key)
{
    g_return_val_if_fail (item != NULL, NULL);
    g_return_val_if_fail (key != NULL, NULL);
    return g_hash_table_lookup (item->keys, key);
}


char *
moo_key_file_item_steal (MooKeyFileItem *item,
                         const char     *key)
{
    gpointer orig_key, value;

    g_return_val_if_fail (item != NULL, NULL);
    g_return_val_if_fail (key != NULL, NULL);

    if (!g_hash_table_lookup_extended (item->keys, key,
                                       &orig_key, &value))
        return NULL;

    g_hash_table_steal (item->keys, key);
    g_free (orig_key);
    return value;
}


void
moo_key_file_item_set (MooKeyFileItem *item,
                       const char     *key,
                       const char     *value)
{
    g_return_if_fail (item != NULL);
    g_return_if_fail (key != NULL);

    if (value)
        g_hash_table_insert (item->keys, g_strdup (key), g_strdup (value));
    else
        g_hash_table_remove (item->keys, key);
}


#if 0
gboolean
moo_key_file_item_get_bool (MooKeyFileItem *item,
                            const char     *key,
                            gboolean        default_val)
{
    const char *val;

    g_return_val_if_fail (item != NULL, default_val);
    g_return_val_if_fail (key != NULL, default_val);

    val = moo_key_file_item_get (item, key);

    if (!val)
        return default_val;
    else
        return _moo_convert_string_to_bool (val, default_val);
}
#endif


gboolean
moo_key_file_item_steal_bool (MooKeyFileItem *item,
                              const char     *key,
                              gboolean        default_val)
{
    char *val;
    gboolean ret;

    g_return_val_if_fail (item != NULL, default_val);
    g_return_val_if_fail (key != NULL, default_val);

    val = moo_key_file_item_steal (item, key);

    if (!val)
        return default_val;

    ret = _moo_convert_string_to_bool (val, default_val);

    g_free (val);
    return ret;
}


void
moo_key_file_item_set_bool (MooKeyFileItem *item,
                            const char     *key,
                            gboolean        value)
{
    g_return_if_fail (item != NULL);
    g_return_if_fail (key != NULL);
    moo_key_file_item_set (item, key, value ? "true" : "false");
}


void
moo_key_file_item_foreach (MooKeyFileItem *item,
                           GHFunc          func,
                           gpointer        data)
{
    g_return_if_fail (item != NULL);
    g_return_if_fail (func != NULL);
    g_hash_table_foreach (item->keys, func, data);
}


const char *
moo_key_file_item_get_content (MooKeyFileItem *item)
{
    g_return_val_if_fail (item != NULL, NULL);
    return item->content;
}


char *
moo_key_file_item_steal_content (MooKeyFileItem *item)
{
    char *ret;

    g_return_val_if_fail (item != NULL, NULL);

    ret = item->content;
    item->content = NULL;

    return ret;
}


static void
key_file_item_take_content (MooKeyFileItem *item,
                            char           *content)
{
    g_return_if_fail (item != NULL);
    g_free (item->content);
    item->content = content;
}


void
moo_key_file_item_set_content (MooKeyFileItem *item,
                               const char     *content)
{
    g_return_if_fail (item != NULL);
    key_file_item_take_content (item, g_strdup (content));
}


GType
moo_key_file_get_type (void)
{
    static GType type;

    if (G_UNLIKELY (!type))
        type = g_boxed_type_register_static ("MooKeyFile",
                                             (GBoxedCopyFunc) moo_key_file_ref,
                                             (GBoxedFreeFunc) moo_key_file_unref);

    return type;
}


GType
moo_key_file_item_get_type (void)
{
    static GType type;

    if (G_UNLIKELY (!type))
        type = g_pointer_type_register_static ("MooKeyFileItem");

    return type;
}


static GQuark
moo_key_file_error_quark (void)
{
    return g_quark_from_static_string ("moo-key-file-error");
}


static void
format_key (const char *key,
            const char *value,
            GString    *string)
{
    g_string_append_printf (string, "%s=%s\n", key, value);
}

static void
format_content (const char *content,
                GString    *string,
                const char *indent)
{
    char **p;
    char **lines = moo_splitlines (content);

    for (p = lines; p && *p; ++p)
        g_string_append_printf (string, "%s%s\n", indent, *p);

    g_strfreev (lines);
}

char *
moo_key_file_format (MooKeyFile *key_file,
                     const char *comment,
                     guint       indent)
{
    char *fill;
    GList *l;
    GString *string;

    g_return_val_if_fail (key_file != NULL, NULL);

    fill = g_strnfill (indent, ' ');
    string = g_string_new (NULL);

    if (comment)
        g_string_append_printf (string, "# %s\n", comment);

    for (l = key_file->items->head; l != NULL; l = l->next)
    {
        MooKeyFileItem *item = l->data;

        g_string_append_printf (string, "[%s]\n", item->name);
        g_hash_table_foreach (item->keys, (GHFunc) format_key, string);

        if (item->content)
            format_content (item->content, string, fill);

        if (l->next)
            g_string_append (string, "\n");
    }

    g_free (fill);
    return g_string_free (string, FALSE);
}
