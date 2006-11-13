/*
 *   completion.c
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

#include "completion.h"
#include "mooedit/moocompletion.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/mooutils-fs.h"
#include "mooutils/mooconfig.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <time.h>

typedef enum {
    DATA_FILE_SIMPLE,
    DATA_FILE_CONFIG
} DataFileType;

typedef struct {
    char *path;
    DataFileType type;
    time_t mtime;
    MooCompletion *cmpl;
} CmplData;

static CmplData *cmpl_data_new              (void);
static void      cmpl_data_free             (CmplData   *data);
static void      cmpl_data_clear            (CmplData   *data);

static CmplData *cmpl_plugin_load_data      (CmplPlugin *plugin,
                                             const char *id);


void
_completion_callback (MooEditWindow *window)
{
    CmplPlugin *plugin;
    MooEdit *doc;

    plugin = moo_plugin_lookup (CMPL_PLUGIN_ID);
    g_return_if_fail (plugin != NULL);

    doc = moo_edit_window_get_active_doc (window);
    g_return_if_fail (doc != NULL);

    _completion_complete (plugin, doc);
}


static GList *
parse_words (const char *string,
             const char *prefix,
             const char *path)
{
    GList *list = NULL;
    char **words, **p;

    g_return_val_if_fail (string != NULL, NULL);

    words = g_strsplit_set (string, " \t\r\n", 0);

    for (p = words; p && **p; ++p)
    {
        if (p[0][0] && p[0][1])
        {
            if (!g_utf8_validate (*p, -1, NULL))
            {
                g_critical ("%s: invalid utf8 in '%s'",
                            G_STRLOC, path);
            }
            else if (prefix)
            {
                list = g_list_prepend (list, g_strdup_printf ("%s%s", prefix, *p));
            }
            else
            {
                list = g_list_prepend (list, *p);
                *p = NULL;
            }
        }

        g_free (*p);
    }

    g_free (words);
    return g_list_reverse (list);
}


static void
cmpl_data_read_simple_file (CmplData *data)
{
    GError *error = NULL;
    GList *list;
    char *contents;

    g_return_if_fail (data->cmpl == NULL);
    g_return_if_fail (data->path != NULL);

    g_file_get_contents (data->path, &contents, NULL, &error);

    if (error)
    {
        g_warning ("%s: could not read file '%s': %s",
                   G_STRLOC, data->path, error->message);
        g_error_free (error);
        return;
    }

    list = parse_words (contents, NULL, data->path);
    data->cmpl = moo_completion_new_text (list);
//     g_message ("read %d words from %s", g_list_length (list), data->path);

    g_free (contents);
}


static guint *
parse_numbers (const char *string,
               guint      *n_numbers_p)
{
    guint *numbers;
    guint n_numbers, i;
    char **pieces, **p;
    GSList *list = NULL;

    g_return_val_if_fail (string != NULL, NULL);

    pieces = g_strsplit_set (string, " \t,;:", 0);
    g_return_val_if_fail (pieces != NULL, NULL);

    for (p = pieces; p && *p; ++p)
    {
        if (**p)
        {
            guint64 n64;
            guint n;

            errno = 0;
            n64 = g_ascii_strtoull (*p, NULL, 10);

            if (errno || n64 > 10000)
            {
                g_warning ("%s: could not parse number '%s'", G_STRLOC, *p);
                goto error;
            }

            n = n64;
            list = g_slist_prepend (list, GUINT_TO_POINTER (n));
        }
    }

    list = g_slist_reverse (list);
    n_numbers = g_slist_length (list);
    numbers = g_new (guint, n_numbers);

    for (i = 0; i < n_numbers; ++i)
    {
        numbers[i] = GPOINTER_TO_UINT (list->data);
        list = g_slist_delete_link (list, list);
    }

    if (n_numbers_p)
        *n_numbers_p = n_numbers;

    g_strfreev (pieces);
    return numbers;

error:
    g_strfreev (pieces);
    g_slist_free (list);
    return NULL;
}


static void
cmpl_data_read_config_file (CmplData *data)
{
    MooConfig *config;
    guint i, n_items;
    MooCompletionGroup *group = NULL;

    g_return_if_fail (data->cmpl == NULL);
    g_return_if_fail (data->path != NULL);

    config = moo_config_new_from_file (data->path, FALSE, NULL);
    g_return_if_fail (config != NULL);

    n_items = moo_config_n_items (config);
    g_return_if_fail (n_items != 0);

    data->cmpl = moo_completion_new_text (NULL);

    for (i = 0; i < n_items; ++i)
    {
        MooConfigItem *item;
        const char *pattern, *prefix, *suffix, *script;
        const char *groups;
        guint *parens;
        guint n_parens;
        GList *words;

        item = moo_config_nth_item (config, i);

        pattern = moo_config_item_get (item, "pattern");
        prefix = moo_config_item_get (item, "prefix");
        suffix = moo_config_item_get (item, "insert-suffix");
        script = moo_config_item_get (item, "insert-script");

        groups = moo_config_item_get (item, "group");
        groups = groups ? groups : moo_config_item_get (item, "groups");
        groups = groups ? groups : "0";

        if (!pattern)
        {
            g_warning ("%s: pattern missing", G_STRLOC);
            continue;
        }

        parens = parse_numbers (groups, &n_parens);

        if (!parens)
        {
            g_warning ("%s: invalid group string '%s'", G_STRLOC, groups);
            continue;
        }

        words = parse_words (moo_config_item_get_content (item),
                             prefix, data->path);

        if (!words)
        {
            g_warning ("%s: empty group", G_STRLOC);
            g_free (parens);
            continue;
        }

//         g_message ("read %d words for patttern '%s' from %s",
//                    g_list_length (words), pattern, data->path);

        group = moo_completion_new_group (data->cmpl, NULL);
        moo_completion_group_add_data (group, words);
        moo_completion_group_set_pattern (group, pattern, parens, n_parens);

        if (script && script[0])
            moo_completion_group_set_script (group, script);
        else if (suffix && suffix[0])
            moo_completion_group_set_suffix (group, suffix);

        g_free (parens);
    }

    g_object_unref (config);

    if (!group)
    {
        g_warning ("%s: no completions", G_STRLOC);
        return;
    }
}


static void
cmpl_data_read_file (CmplData *data)
{
    g_return_if_fail (data->path != NULL);
    g_return_if_fail (data->cmpl == NULL);

    switch (data->type)
    {
        case DATA_FILE_SIMPLE:
            return cmpl_data_read_simple_file (data);
        case DATA_FILE_CONFIG:
            return cmpl_data_read_config_file (data);
    }

    g_return_if_reached ();
}


static CmplData *
cmpl_plugin_load_data (CmplPlugin *plugin,
                       const char *id)
{
    CmplData *data;

    id = id ? id : CMPL_FILE_NONE;
    data = g_hash_table_lookup (plugin->data, id);

    if (!data)
    {
        data = cmpl_data_new ();
        g_hash_table_insert (plugin->data, g_strdup (id), data);
    }
    else if (!data->cmpl && data->path)
    {
        cmpl_data_read_file (data);
    }

    return data;
}


static CmplData *
cmpl_data_new (void)
{
    return g_new0 (CmplData, 1);
}


static void
cmpl_data_clear (CmplData *data)
{
    g_free (data->path);
    if (data->cmpl)
        g_object_unref (data->cmpl);
    data->path = NULL;
    data->cmpl = NULL;
}


static void
cmpl_data_free (CmplData *data)
{
    if (data)
    {
        cmpl_data_clear (data);
        g_free (data);
    }
}


static void
cmpl_plugin_check_file (CmplPlugin  *plugin,
                        const char  *path,
                        DataFileType type,
                        const char  *id)
{
    struct stat buf;
    CmplData *data;

    /* TODO: filename encoding */

    if (stat (path, &buf) == -1)
    {
        int err_no = errno;
        g_warning ("%s: could not stat file '%s': %s",
                   G_STRLOC, path, g_strerror (err_no));
        return;
    }

    if (!S_ISREG (buf.st_mode))
    {
        g_warning ("%s: '%s' is not a regular file",
                   G_STRLOC, path);
        return;
    }

    data = g_hash_table_lookup (plugin->data, id);

    if (data)
    {
        if (data->path && !strcmp (data->path, path) && data->mtime == buf.st_mtime)
            return;

        cmpl_data_clear (data);
    }
    else
    {
        data = cmpl_data_new ();
        g_hash_table_insert (plugin->data, g_strdup (id), data);
    }

    data->path = g_strdup (path);
    data->type = type;
    data->mtime = buf.st_mtime;
//     g_message ("found file '%s' for lang '%s'", path, id);
}


static void
cmpl_plugin_load_dir (CmplPlugin *plugin,
                      const char *path)
{
    GDir *dir;
    const char *base;

    if (!(dir = g_dir_open (path, 0, NULL)))
        return;

    while ((base = g_dir_read_name (dir)))
    {
        char *file, *name, *id;
        guint base_len;
        DataFileType type;

        base_len = strlen (base);

        if (base_len <= strlen (CMPL_FILE_SUFFIX))
            continue;

        if (g_str_has_suffix (base, CMPL_FILE_SUFFIX))
            type = DATA_FILE_SIMPLE;
        else if (g_str_has_suffix (base, CMPL_CONFIG_SUFFIX))
            type = DATA_FILE_CONFIG;
        else
            continue;

        name = g_strndup (base, base_len - strlen (CMPL_FILE_SUFFIX));
        id = moo_lang_id_from_name (name);
        file = g_build_filename (path, base, NULL);

        cmpl_plugin_check_file (plugin, file, type, id);

        g_free (name);
        g_free (id);
        g_free (file);
    }

    g_dir_close (dir);
}


void
_cmpl_plugin_load (CmplPlugin *plugin)
{
    char **dirs;
    guint n_dirs, i;

    plugin->data = g_hash_table_new_full (g_str_hash, g_str_equal, g_free,
                                          (GDestroyNotify) cmpl_data_free);

    dirs = moo_get_data_subdirs (CMPL_DIR, MOO_DATA_SHARE, &n_dirs);

    for (i = 0; i < n_dirs; ++i)
        cmpl_plugin_load_dir (plugin, dirs[i]);

    g_strfreev (dirs);
}


void
_cmpl_plugin_clear (CmplPlugin *plugin)
{
    g_hash_table_destroy (plugin->data);
    plugin->data = NULL;
}


void
_completion_complete (CmplPlugin *plugin,
                      MooEdit    *doc)
{
    MooLang *lang;
    CmplData *data;

    lang = moo_text_view_get_lang (MOO_TEXT_VIEW (doc));

    data = cmpl_plugin_load_data (plugin, lang ? moo_lang_id (lang) : NULL);
    g_return_if_fail (data != NULL);

    if (data->cmpl)
    {
        moo_completion_set_doc (data->cmpl, GTK_TEXT_VIEW (doc));
        moo_completion_try_complete (data->cmpl, TRUE);
    }
}
