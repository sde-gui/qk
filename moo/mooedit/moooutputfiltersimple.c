/*
 *   moooutputfiltersimple.c
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

#include "mooedit/moooutputfiltersimple.h"
#include "mooedit/moocmdview.h"
#include "mooedit/moocommand.h"
#include "mooedit/mookeyfile.h"
#include "mooutils/eggregex.h"
#include "mooutils/mooutils-gobject.h"
#include "mooutils/mooutils-misc.h"
#include <string.h>

#define FILTERS_FILE    "filters.cfg"
#define ITEM_FILTER     "filter"
#define KEY_ID          "id"
#define KEY_DELETED     "deleted"
#define KEY_BUILTIN     "builtin"
#define KEY_NAME        "name"
#define KEY_ENABLED     "enabled"


typedef struct {
    EggRegex *re;
    MooOutputTextType type;
} RegexInfo;

typedef struct {
    guint ref_count;
    RegexInfo *patterns;
    guint n_patterns;
} FilterInfo;

typedef struct {
    GHashTable *hash;
} FilterStore;

struct _MooOutputFilterSimplePrivate {
    FilterInfo *info;
};


static FilterStore *filter_store;


static void          filter_store_init  (void);

static FilterInfo   *filter_info_new    (guint                   n_patterns);
static FilterInfo   *filter_info_ref    (FilterInfo             *info);
static void          filter_info_unref  (FilterInfo             *info);


G_DEFINE_TYPE (MooOutputFilterSimple, _moo_output_filter_simple, MOO_TYPE_OUTPUT_FILTER)


static void
moo_output_filter_simple_dispose (GObject *object)
{
    MooOutputFilterSimple *filter = MOO_OUTPUT_FILTER_SIMPLE (object);

    if (filter->priv->info)
    {
        filter_info_unref (filter->priv->info);
        filter->priv->info = NULL;
    }

    G_OBJECT_CLASS (_moo_output_filter_simple_parent_class)->dispose (object);
}


static void
moo_output_filter_simple_attach (MooOutputFilter *base)
{
    MooOutputFilterSimple *filter = MOO_OUTPUT_FILTER_SIMPLE (base);

    g_return_if_fail (filter->priv->info != NULL);
}


static void
moo_output_filter_simple_detach (MooOutputFilter *base)
{
    MooOutputFilterSimple *filter = MOO_OUTPUT_FILTER_SIMPLE (base);

    g_return_if_fail (filter->priv->info != NULL);
}


static MooFileLineData *
parse_file_line (const char *file,
                 const char *line,
                 const char *character)
{
    MooFileLineData *data;

    file = file && *file ? file : NULL;
    line = line && *line ? line : NULL;

    if (!file && !line)
        return NULL;

    data = moo_file_line_data_new (file, -1, -1);
    data->line = _moo_convert_string_to_int (line, 0) - 1;
    data->character = _moo_convert_string_to_int (character, 0) - 1;

    return data;
}

static void
process_result (const char       *text,
                EggRegex         *regex,
                MooLineView      *view)
{
    char *file, *line, *character;
    MooFileLineData *data;
    int line_no;
    GtkTextTag *tag;

    file = egg_regex_fetch_named (regex, "file", text);
    line = egg_regex_fetch_named (regex, "line", text);
    character = egg_regex_fetch_named (regex, "character", text);

    tag = moo_line_view_lookup_tag (view, MOO_CMD_VIEW_STDERR);
    line_no = moo_line_view_write_line (view, text, -1, tag);

    data = parse_file_line (file, line, character);

    if (data)
    {
        moo_line_view_set_boxed (view, line_no, MOO_TYPE_FILE_LINE_DATA, data);
        moo_file_line_data_free (data);
    }

    g_free (file);
    g_free (line);
    g_free (character);
}


static gboolean
process_line (MooOutputFilterSimple *filter,
              const char            *text,
              FilterInfo            *info,
              MooOutputTextType      type)
{
    guint i;

    for (i = 0; i < info->n_patterns; ++i)
    {
        RegexInfo *regex = &info->patterns[i];

        if (regex->type != type && regex->type != MOO_OUTPUT_ALL)
            continue;

        if (!egg_regex_match (regex->re, text, 0))
            continue;

        process_result (text, regex->re, moo_output_filter_get_view (MOO_OUTPUT_FILTER(filter)));

        return TRUE;
    }

    return FALSE;
}


static gboolean
moo_output_filter_simple_stdout_line (MooOutputFilter *base,
                                      const char      *line)
{
    MooOutputFilterSimple *filter = MOO_OUTPUT_FILTER_SIMPLE (base);
    g_return_val_if_fail (filter->priv->info != NULL, FALSE);
    return process_line (filter, line, filter->priv->info, MOO_OUTPUT_STDOUT);
}


static gboolean
moo_output_filter_simple_stderr_line (MooOutputFilter *base,
                                      const char      *line)
{
    MooOutputFilterSimple *filter = MOO_OUTPUT_FILTER_SIMPLE (base);
    g_return_val_if_fail (filter->priv->info != NULL, FALSE);
    return process_line (filter, line, filter->priv->info, MOO_OUTPUT_STDERR);
}


static void
_moo_output_filter_simple_class_init (MooOutputFilterSimpleClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    MooOutputFilterClass *filter_class = MOO_OUTPUT_FILTER_CLASS (klass);

    object_class->dispose = moo_output_filter_simple_dispose;

    filter_class->attach = moo_output_filter_simple_attach;
    filter_class->detach = moo_output_filter_simple_detach;
    filter_class->stdout_line = moo_output_filter_simple_stdout_line;
    filter_class->stderr_line = moo_output_filter_simple_stderr_line;

    g_type_class_add_private (klass, sizeof (MooOutputFilterSimplePrivate));
}


static void
_moo_output_filter_simple_init (MooOutputFilterSimple *filter)
{
    filter->priv = G_TYPE_INSTANCE_GET_PRIVATE (filter, MOO_TYPE_OUTPUT_FILTER_SIMPLE, MooOutputFilterSimplePrivate);
}


static void
filter_store_init (void)
{
    if (!filter_store)
    {
        filter_store = g_new0 (FilterStore, 1);
        filter_store->hash = g_hash_table_new_full (g_str_hash, g_str_equal, g_free,
                                                    (GDestroyNotify) filter_info_unref);
    }
}


static FilterInfo *
filter_info_ref (FilterInfo *info)
{
    g_return_val_if_fail (info != NULL, NULL);
    info->ref_count++;
    return info;
}


static void
filter_info_unref (FilterInfo *info)
{
    guint i;

    if (!info || --info->ref_count)
        return;

    for (i = 0; i < info->n_patterns; ++i)
        if (info->patterns[i].re)
            egg_regex_free (info->patterns[i].re);

    g_free (info->patterns);
    g_free (info);
}


static FilterInfo *
filter_info_new (guint n_patterns)
{
    FilterInfo *info = g_new0 (FilterInfo, 1);

    info->ref_count = 1;
    info->n_patterns = n_patterns;
    info->patterns = g_new0 (RegexInfo, n_patterns);

    return info;
}


static void
prepend_id (const char *id,
            G_GNUC_UNUSED gpointer info,
            GSList **list)
{
    *list = g_slist_prepend (*list, g_strdup (id));
}

static void
filter_store_clear (void)
{
    GSList *list = NULL;

    if (!filter_store)
        return;

    g_hash_table_foreach (filter_store->hash, (GHFunc) prepend_id, &list);

    while (list)
    {
        moo_command_filter_unregister (list->data);
        g_free (list->data);
        list = g_slist_delete_link (list, list);
    }
}


static MooOutputFilter *
factory_func (const char *id,
              G_GNUC_UNUSED gpointer data)
{
    MooOutputFilterSimple *filter;
    FilterInfo *info;

    g_return_val_if_fail (id != NULL, NULL);
    g_return_val_if_fail (filter_store != NULL, NULL);

    info = g_hash_table_lookup (filter_store->hash, id);
    g_return_val_if_fail (info != NULL, NULL);

    filter = g_object_new (MOO_TYPE_OUTPUT_FILTER_SIMPLE, NULL);
    filter->priv->info = filter_info_ref (info);

    return MOO_OUTPUT_FILTER (filter);
}


static void
filter_removed (char *id)
{
    g_return_if_fail (id != NULL);
    g_return_if_fail (filter_store != NULL);
    g_hash_table_remove (filter_store->hash, id);
    g_free (id);
}

static void
filter_store_add (const char *id,
                  const char *name,
                  FilterInfo *info)
{
    g_return_if_fail (id != NULL);
    g_return_if_fail (name != NULL);
    g_return_if_fail (info != NULL);

    moo_command_filter_register (id, name, factory_func, g_strdup (id),
                                 (GDestroyNotify) filter_removed);

    filter_store_init ();
    g_hash_table_insert (filter_store->hash, g_strdup (id), info);
}


static void
filter_store_set_filters (GSList *list)
{
    filter_store_clear ();

    while (list)
    {
        guint i;
        MooOutputFilterInfo *info;
        FilterInfo *filter_info;

        info = list->data;
        list = list->next;

        filter_info = filter_info_new (info->n_patterns);

        for (i = 0; i < info->n_patterns; ++i)
        {
            MooOutputPatternInfo *pattern_info;
            EggRegex *regex;
            GError *error = NULL;

            pattern_info = info->patterns[i];
            regex = egg_regex_new (pattern_info->pattern, 0, 0, &error);

            if (!regex)
            {
                g_warning ("could not create regex: %s", error->message);
                g_error_free (error);
                filter_info_unref (filter_info);
                filter_info = NULL;
                break;
            }

            egg_regex_optimize (regex, NULL);

            filter_info->patterns[i].re = regex;
            filter_info->patterns[i].type = pattern_info->type;
        }

        if (filter_info)
            filter_store_add (info->id, info->name, filter_info);
    }
}


static MooOutputFilterInfo *
_moo_output_filter_info_new (void)
{
    MooOutputFilterInfo *info = g_new0 (MooOutputFilterInfo, 1);
    info->ref_count = 1;
    return info;
}


static MooOutputFilterInfo *
_moo_output_filter_info_ref (MooOutputFilterInfo *info)
{
    g_return_val_if_fail (info != NULL, NULL);
    info->ref_count++;
    return info;
}


static void
moo_output_pattern_info_free (MooOutputPatternInfo *p)
{
    if (p)
    {
        g_free (p->pattern);
        g_free (p);
    }
}

static void
_moo_output_filter_info_unref (MooOutputFilterInfo *info)
{
    guint i;

    g_return_if_fail (info != NULL);

    if (--info->ref_count)
        return;

    for (i = 0; i < info->n_patterns; ++i)
        moo_output_pattern_info_free (info->patterns[i]);

    g_free (info->patterns);
    g_free (info->id);
    g_free (info->name);
    g_free (info);
}


/****************************************************************************/
/* Loading and saving
 */

static gboolean
parse_patterns (MooOutputFilterInfo *info,
                const char          *string,
                const char          *file)
{
    GSList *patterns = NULL;
    guint i;

    if (!string || !string[0])
    {
        g_warning ("patterns missing in filter %s in file %s",
                   info->id, file);
        return FALSE;
    }

    while (TRUE)
    {
        gunichar delim;
        char *pattern_start, *pattern_end;
        MooOutputPatternInfo *pattern_info;
        MooOutputTextType type;

        while (*string)
        {
            gunichar c = g_utf8_get_char (string);

            if (c != ',' && c != ';' && !g_unichar_isspace (c))
                break;

            string = g_utf8_next_char (string);
        }

        if (!*string)
            break;

        switch (*string++)
        {
            case 'o':
                type = MOO_OUTPUT_STDOUT;
                break;
            case 'e':
                type = MOO_OUTPUT_STDERR;
                break;
            case 'a':
                type = MOO_OUTPUT_ALL;
                break;
            default:
                g_warning ("output type missing: '%s'", string - 1);
                goto error;
        }

        delim = g_utf8_get_char (string);
        pattern_start = g_utf8_next_char (string);
        pattern_end = g_utf8_strchr (pattern_start, -1, delim);

        if (!pattern_end)
        {
            g_warning ("unterminated pattern: '%s'", string);
            goto error;
        }

        pattern_info = g_new0 (MooOutputPatternInfo, 1);
        pattern_info->pattern = g_strndup (pattern_start, pattern_end - pattern_start);
        pattern_info->type = type;
        patterns = g_slist_prepend (patterns, pattern_info);

        string = g_utf8_next_char (pattern_end);
    }

    info->n_patterns = g_slist_length (patterns);
    info->patterns = g_new0 (MooOutputPatternInfo*, info->n_patterns);

    patterns = g_slist_reverse (patterns);
    i = 0;

    while (patterns)
    {
        info->patterns[i++] = patterns->data;
        patterns = g_slist_delete_link (patterns, patterns);
    }

    return TRUE;

error:
    g_slist_foreach (patterns, (GFunc) g_free, NULL);
    g_slist_free (patterns);
    return FALSE;
}


static MooOutputFilterInfo *
parse_item (MooKeyFileItem *item,
            const char     *file)
{
    char *pattern_string;
    MooOutputFilterInfo *info;

    if (strcmp (moo_key_file_item_name (item), ITEM_FILTER))
    {
        g_warning ("invalid group %s in file %s", moo_key_file_item_name (item), file);
        return NULL;
    }

    info = _moo_output_filter_info_new ();
    info->id = moo_key_file_item_steal (item, KEY_ID);
    info->name = moo_key_file_item_steal (item, KEY_NAME);
    info->enabled = moo_key_file_item_steal_bool (item, KEY_ENABLED, TRUE);
    info->deleted = moo_key_file_item_steal_bool (item, KEY_DELETED, FALSE);
    info->builtin = moo_key_file_item_steal_bool (item, KEY_BUILTIN, FALSE);

    if (!info->id)
    {
        g_warning ("filter id missing in file %s", file);
        _moo_output_filter_info_unref (info);
        return NULL;
    }

    if (info->deleted || info->builtin)
        return info;

    pattern_string = moo_key_file_item_steal_content (item);

    if (!parse_patterns (info, pattern_string, file))
    {
        _moo_output_filter_info_unref (info);
        info = NULL;
    }

    g_free (pattern_string);

    return info;
}

static GSList *
parse_key_file (MooKeyFile *key_file,
                const char *filename)
{
    guint n_items, i;
    GSList *list = NULL;

    n_items = moo_key_file_n_items (key_file);

    for (i = 0; i < n_items; ++i)
    {
        MooKeyFileItem *item = moo_key_file_nth_item (key_file, i);
        MooOutputFilterInfo *info = parse_item (item, filename);
        if (info)
            list = g_slist_prepend (list, info);
    }

    return g_slist_reverse (list);
}

static GSList *
parse_file_simple (const char *filename)
{
    MooKeyFile *key_file;
    GError *error = NULL;
    GSList *list = NULL;

    g_return_val_if_fail (filename != NULL, NULL);

    key_file = moo_key_file_new_from_file (filename, &error);

    if (key_file)
    {
        list = parse_key_file (key_file, filename);
        moo_key_file_unref (key_file);
    }
    else
    {
        g_warning ("could not load file '%s': %s", filename, error->message);
        g_error_free (error);
    }

    return list;
}

static void
parse_file (const char     *filename,
            GSList        **list,
            GHashTable     *ids)
{
    GSList *new_list;

    new_list = parse_file_simple (filename);

    if (!new_list)
        return;

    while (new_list)
    {
        MooOutputFilterInfo *info, *old_info;
        GSList *old_link;

        info = new_list->data;
        g_return_if_fail (info->id != NULL);

        old_link = g_hash_table_lookup (ids, info->id);
        old_info = old_link ? old_link->data : NULL;

        if (old_link)
        {
            *list = g_slist_delete_link (*list, old_link);
            g_hash_table_remove (ids, info->id);
        }

        if (info->deleted)
        {
            _moo_output_filter_info_unref (info);
            info = NULL;
        }
        else if (info->builtin)
        {
            _moo_output_filter_info_unref (info);
            info = old_info ? _moo_output_filter_info_ref (old_info) : NULL;
        }

        if (info)
        {
            *list = g_slist_prepend (*list, info);
            g_hash_table_insert (ids, g_strdup (info->id), *list);
        }

        if (old_info)
            _moo_output_filter_info_unref (old_info);

        new_list = g_slist_delete_link (new_list, new_list);
    }
}


static void
find_filters_files (char ***sys_files_p,
                    char  **user_file_p)
{
    int i;
    char **files;
    guint n_files;
    GPtrArray *sys_files = NULL;

    *sys_files_p = NULL;
    *user_file_p = NULL;

    files = moo_get_data_files (FILTERS_FILE, MOO_DATA_SHARE, &n_files);

    if (!n_files)
        return;

    if (g_file_test (files[n_files - 1], G_FILE_TEST_EXISTS))
        *user_file_p = g_strdup (files[n_files - 1]);

    for (i = 0; i < (int) n_files - 1; ++i)
    {
        if (g_file_test (files[i], G_FILE_TEST_EXISTS))
        {
            if (!sys_files)
                sys_files = g_ptr_array_new ();
            g_ptr_array_add (sys_files, g_strdup (files[i]));
        }
    }

    if (sys_files)
    {
        g_ptr_array_add (sys_files, NULL);
        *sys_files_p = (char**) g_ptr_array_free (sys_files, FALSE);
    }

    g_strfreev (files);
}

static GSList *
parse_filters (void)
{
    char **sys_files, *user_file;
    GSList *list = NULL;
    GHashTable *ids = NULL;
    char **p;

    find_filters_files (&sys_files, &user_file);
    ids = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

    for (p = sys_files; p && *p; ++p)
        parse_file (*p, &list, ids);

    if (user_file)
        parse_file (user_file, &list, ids);

    g_hash_table_destroy (ids);
    g_strfreev (sys_files);
    g_free (user_file);

    return g_slist_reverse (list);
}


void
_moo_command_filter_simple_load (void)
{
    GSList *list = parse_filters ();

    filter_store_set_filters (list);

    g_slist_foreach (list, (GFunc) _moo_output_filter_info_unref, NULL);
    g_slist_free (list);
}
