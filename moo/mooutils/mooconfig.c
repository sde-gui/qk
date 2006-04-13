/*
 *   mooconfig.c
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

#include "mooutils/mooconfig.h"
#include "mooutils/mooutils-gobject.h"
#include "mooutils/moocompat.h"
#include <string.h>


#ifdef __WIN32__
#define LINE_TERM "\r\n"
#else
#define LINE_TERM "\n"
#endif

struct _MooConfig {
    GPtrArray *items;
};

struct _MooConfigItem {
    GHashTable *dict;
    char *content;
    guint start;
    guint end;
};

#define NTH_ITEM(cfg,n) ((MooConfigItem*) (cfg)->items->pdata[n])

static MooConfigItem   *moo_config_item_new     (void);
static void             moo_config_item_free    (MooConfigItem  *item);


MooConfig *
moo_config_new (void)
{
    MooConfig *config = g_new0 (MooConfig, 1);
    config->items = g_ptr_array_new ();
    return config;
}


void
moo_config_free (MooConfig *config)
{
    if (config)
    {
        g_ptr_array_foreach (config->items,
                             (GFunc) moo_config_item_free,
                             NULL);
        g_ptr_array_free (config->items, TRUE);
        g_free (config);
    }
}


guint
moo_config_n_items (MooConfig *config)
{
    g_return_val_if_fail (config != NULL, 0);
    return config->items->len;
}


MooConfigItem *
moo_config_nth_item (MooConfig  *config,
                     guint       n)
{
    g_return_val_if_fail (config != NULL, NULL);
    g_return_val_if_fail (n < config->items->len, NULL);
    return NTH_ITEM (config, n);
}


MooConfigItem *
moo_config_new_item (MooConfig *config)
{
    MooConfigItem *item;

    g_return_val_if_fail (config != NULL, NULL);

    item = moo_config_item_new ();
    g_ptr_array_add (config->items, item);

    return item;
}


static MooConfigItem *
moo_config_item_new (void)
{
    MooConfigItem *item = g_new0 (MooConfigItem, 1);
    item->dict = g_hash_table_new_full (g_str_hash, g_str_equal,
                                        g_free, g_free);
    return item;
}


static void
moo_config_item_free (MooConfigItem *item)
{
    if (item)
    {
        g_hash_table_destroy (item->dict);
        g_free (item->content);
        g_free (item);
    }
}


static char *
normalize_key (const char *string)
{
    char *norm = g_ascii_strdown (string, -1);
    return g_strdelimit (norm, "_", '-');
}


const char *
moo_config_item_get_value (MooConfigItem  *item,
                           const char     *key)
{
    char *norm;
    const char *value;

    g_return_val_if_fail (item != NULL, NULL);
    g_return_val_if_fail (key != NULL, NULL);

    norm = normalize_key (key);
    value = g_hash_table_lookup (item->dict, norm);

    g_free (norm);
    return value;
}


void
moo_config_item_set_value (MooConfigItem  *item,
                           const char     *key,
                           const char     *value)
{
    char *norm;

    g_return_if_fail (item != NULL);
    g_return_if_fail (key != NULL);

    norm = normalize_key (key);

    if (value)
    {
        g_hash_table_insert (item->dict, norm, g_strdup (value));
    }
    else
    {
        g_hash_table_remove (item->dict, norm);
        g_free (norm);
    }
}


const char *
moo_config_item_get_content (MooConfigItem *item)
{
    g_return_val_if_fail (item != NULL, NULL);
    return item->content;
}


void
moo_config_item_set_content (MooConfigItem  *item,
                             const char     *content)
{
    char *tmp;

    g_return_if_fail (item != NULL);

    tmp = item->content;
    item->content = g_strdup (content);
    g_free (tmp);
}


/*********************************************************************/
/* Parsing
 */

inline static const char *
find_line_term (const char *string,
                guint       len,
                guint      *term_len)
{
    while (len)
    {
        switch (string[0])
        {
            case '\r':
                if (len > 1 && string[1] == '\n')
                    *term_len = 2;
                else
                    *term_len = 1;
                return string;

            case '\n':
                *term_len = 1;
                return string;

            default:
                len--;
                string++;
        }
    }

    return NULL;
}

inline static char *
normalize_indent (const char *line,
                  guint       len)
{
    char *norm;
    guint n_tabs;

    if (!len || line[0] != '\t')
        return g_strndup (line, len);

    for (n_tabs = 0; n_tabs < len && line[n_tabs] == '\t'; ++n_tabs) ;

    norm = g_new (char, len - n_tabs + n_tabs * 8 + 1);
    norm[len - n_tabs + n_tabs * 8] = 0;

    memset (norm, n_tabs * 8, ' ');

    if (n_tabs < len)
        memcpy (norm + n_tabs * 8, line + n_tabs, len - n_tabs);

    return norm;
}

static char **
splitlines (const char *string,
            guint       len,
            guint      *n_lines_p)
{
    GSList *list = NULL;
    char **lines;
    guint n_lines = 0, i;

    if (!len)
    {
        *n_lines_p = 0;
        return g_new0 (char*, 1);
    }

    while (len)
    {
        guint term_len = 0;
        const char *term = find_line_term (string, len, &term_len);

        n_lines++;

        if (term)
        {
            list = g_slist_prepend (list, normalize_indent (string, term - string));
            len -= (term - string + term_len);
            string = term + term_len;
        }
        else
        {
            list = g_slist_prepend (list, normalize_indent (string, len));
            break;
        }
    }

    list = g_slist_reverse (list);
    lines = g_new (char*, n_lines + 1);
    lines[n_lines] = NULL;
    i = 0;

    while (list)
    {
        lines[i++] = list->data;
        list = g_slist_delete_link (list, list);
    }

    *n_lines_p = n_lines;
    return lines;
}


static gboolean
line_is_blank_or_comment (const char *line)
{
    if (!line[0] || line[0] == '#')
        return TRUE;

    while (*line)
    {
        if (*line != ' ' && *line != '\t')
            return FALSE;
        line++;
    }

    return TRUE;
}


static gboolean
line_is_blank (const char *line)
{
    while (line[0])
    {
        if (line[0] != ' ' && line[0] != '\t')
            return FALSE;

        line++;
    }

    return TRUE;
}


static guint
line_indent (const char *line)
{
    guint indent = 0;

    while (TRUE)
    {
        switch (*line++)
        {
            case ' ':
                indent += 1;
                break;

            case '\t':
                indent += 8;
                break;

            default:
                return indent;
        }
    }
}


inline static gboolean
line_is_comment (const char *line)
{
    return line[0] == '#';
}


inline static gboolean
line_is_indented (const char *line)
{
    return line[0] == ' ';
}


static gboolean
parse_header_line (char          **lines,
                   guint           n,
                   MooConfigItem **item)
{
    char *line = lines[n];
    char *value = NULL, *key = NULL;

    value = strchr (line, ':');

    if (!value)
    {
        g_warning ("%s: value missing on line %d", G_STRLOC, n+1);
        goto error;
    }

    key = g_strstrip (g_strndup (line, value - line));
    value = g_strstrip (g_strdup (value + 1));

    if (!key[0])
    {
        g_warning ("%s: empty key on line %d", G_STRLOC, n+1);
        goto error;
    }

    if (!value[0])
    {
        g_warning ("%s: value missing on line %d", G_STRLOC, n+1);
        goto error;
    }

    if (!*item)
        *item = moo_config_item_new ();

    if (moo_config_item_get_value (*item, key))
    {
        g_warning ("%s: duplicated key '%s' on line %d", G_STRLOC, key, n+1);
        goto error;
    }

    moo_config_item_set_value (*item, key, value);
    g_free (value);
    g_free (key);
    return TRUE;

error:
    g_free (value);
    g_free (key);
    return FALSE;
}


static MooConfig *
parse_lines (char   **lines,
             guint    n_lines)
{
    MooConfig *config;
    MooConfigItem *item;
    GPtrArray *content;
    guint start;
    GSList *items;

    content = g_ptr_array_new ();
    start = 0;
    items = NULL;
    item = NULL;

    while (start < n_lines)
    {
        guint content_start;

        while (start < n_lines && line_is_blank_or_comment (lines[start]))
            start++;

        if (start == n_lines)
            break;

        if (line_is_indented (lines[start]))
        {
            g_warning ("line %d", start+1);
            goto error;
        }

        content_start = start;

        while (content_start < n_lines)
        {
            const char *line = lines[content_start];

            if (line_is_blank (line) || line_is_indented (line))
                break;

            if (line_is_comment (lines[content_start]))
            {
                content_start++;
                continue;
            }

            if (!parse_header_line (lines, content_start, &item))
                goto error;

            content_start++;
        }

        if (!item)
            break;

        start = content_start;

        if (start < n_lines &&
            line_is_indented (lines[start]) &&
            !line_is_blank (lines[start]))
        {
            while (start < n_lines)
            {
                if (line_is_blank_or_comment (lines[start]))
                {
                    start++;
                    continue;
                }

                if (!line_is_indented (lines[start]))
                    break;

                g_ptr_array_add (content, lines[start]);
                start++;
            }
        }

        if (content->len)
        {
            guint indent, i;
            GString *str;

            indent = line_indent (content->pdata[0]);

            for (i = 1; i < content->len; ++i)
            {
                guint indent_here = line_indent (content->pdata[i]);
                indent = MIN (indent_here, indent);
            }

            if (!indent)
            {
                g_critical ("%s: oops", G_STRLOC);
                goto error;
            }

            str = g_string_new (NULL);

            for (i = 0; i < content->len; ++i)
            {
                char *line = content->pdata[i];
                g_string_append (str, line + indent);
                g_string_append_c (str, '\n');
            }

            item->content = str->str;
            g_string_free (str, FALSE);
            g_ptr_array_set_size (content, 0);
        }

        items = g_slist_prepend (items, item);
        item = NULL;
    }

    config = moo_config_new ();
    items = g_slist_reverse (items);

    while (items)
    {
        g_ptr_array_add (config->items, items->data);
        items = items->next;
    }

    g_slist_free (items);
    g_ptr_array_free (content, TRUE);

    return config;

error:
    moo_config_item_free (item);
    g_ptr_array_free (content, TRUE);
    g_slist_foreach (items, (GFunc) moo_config_item_free, NULL);
    g_slist_free (items);
    return NULL;
}


MooConfig *
moo_config_parse (const char *string,
                  int         len)
{
    char **lines;
    guint n_lines;
    MooConfig *config;

    g_return_val_if_fail (string != NULL, NULL);

    if (len < 0)
        len = strlen (string);

    lines = splitlines (string, len, &n_lines);
    config = parse_lines (lines, n_lines);

    g_strfreev (lines);
    return config;
}


MooConfig *
moo_config_parse_file (const char *path)
{
    MooConfig *config;
    GMappedFile *file;
    GError *error = NULL;

    g_return_val_if_fail (path != NULL, NULL);

    file = g_mapped_file_new (path, FALSE, &error);

    if (!file)
    {
        g_warning ("%s: %s", G_STRLOC, error->message);
        g_error_free (error);
        return NULL;
    }

    config = moo_config_parse (g_mapped_file_get_contents (file),
                               g_mapped_file_get_length (file));

    g_mapped_file_free (file);
    return config;
}


gboolean
moo_config_item_get_bool (MooConfigItem  *item,
                          const char     *key,
                          gboolean        default_val)
{
    return moo_convert_string_to_bool (moo_config_item_get_value (item, key),
                                       default_val);
}


void
moo_config_item_set_bool (MooConfigItem  *item,
                          const char     *key,
                          gboolean        value)
{
    moo_config_item_set_value (item, key, moo_convert_bool_to_string (value));
}


static void
format_key (const char *key,
            const char *value,
            GString    *string)
{
    g_string_append_printf (string, "%s: %s" LINE_TERM, key, value);
}

static void
moo_config_item_format (MooConfigItem *item,
                        GString       *string)
{
    char **lines;
    guint n_lines, i;
    char *indent;

    g_return_if_fail (g_hash_table_size (item->dict) != 0);
    g_hash_table_foreach (item->dict, (GHFunc) format_key, string);

    if (!item->content)
        return;

    lines = splitlines (item->content, strlen (item->content), &n_lines);
    indent = g_strnfill (2, ' ');

    for (i = 0; i < n_lines; ++i)
    {
        g_string_append (string, indent);
        g_string_append (string, lines[i]);
        g_string_append (string, LINE_TERM);
    }

    g_free (indent);
    g_strfreev (lines);
}


char *
moo_config_format (MooConfig *config)
{
    GString *string;
    guint i;

    g_return_val_if_fail (config != NULL, NULL);

    string = g_string_new (NULL);

    for (i = 0; i < config->items->len; ++i)
    {
        if (i > 0)
            g_string_append (string, LINE_TERM);

        moo_config_item_format (config->items->pdata[i], string);
    }

    return g_string_free (string, FALSE);
}
