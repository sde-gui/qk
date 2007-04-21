/*
 *   completion.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#include "completion.h"
#include "mooedit/moocompletionsimple.h"
#include "mooedit/mookeyfile.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/mooutils-fs.h"
#include "mooutils/moopython.h"
#include <gdk/gdkkeysyms.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <time.h>

typedef enum {
    DATA_FILE_SIMPLE,
    DATA_FILE_CONFIG,
    DATA_FILE_PYTHON
} DataFileType;

#define DATA_FILE_INVALID ((DataFileType) -1)

typedef struct {
    char *path;
    DataFileType type;
    time_t mtime;
    MooTextCompletion *cmpl;
} CmplData;

static CmplData *cmpl_data_new              (void);
static void      cmpl_data_free             (CmplData   *data);
static void      cmpl_data_clear            (CmplData   *data);

static CmplData *cmpl_plugin_load_data      (CmplPlugin *plugin,
                                             const char *id);


static GList *
parse_words (const char *string,
             const char *prefix,
             const char *path)
{
    GList *list = NULL;
    char **words, **p;

    g_return_val_if_fail (string != NULL, NULL);

    words = g_strsplit_set (string, " \t\r\n", 0);

    for (p = words; p && *p; ++p)
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
    data->cmpl = moo_completion_simple_new_text (list);
    _moo_message ("read %d words from %s", g_list_length (list), data->path);

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
    MooKeyFile *key_file;
    guint i, n_items;
    MooCompletionGroup *group = NULL;
    GError *error = NULL;

    g_return_if_fail (data->cmpl == NULL);
    g_return_if_fail (data->path != NULL);

    data->cmpl = moo_completion_simple_new_text (NULL);

    key_file = moo_key_file_new_from_file (data->path, &error);

    if (!key_file)
    {
        g_warning ("%s", error->message);
        g_error_free (error);
        return;
    }

    n_items = moo_key_file_n_items (key_file);
    g_return_if_fail (n_items != 0);

    for (i = 0; i < n_items; ++i)
    {
        MooKeyFileItem *item;
        const char *pattern, *prefix, *suffix, *script;
        const char *groups;
        guint *parens;
        guint n_parens;
        GList *words;

        item = moo_key_file_nth_item (key_file, i);

        pattern = moo_key_file_item_get (item, "pattern");
        prefix = moo_key_file_item_get (item, "prefix");
        suffix = moo_key_file_item_get (item, "insert-suffix");
        script = moo_key_file_item_get (item, "insert-script");

        groups = moo_key_file_item_get (item, "group");
        groups = groups ? groups : moo_key_file_item_get (item, "groups");
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

        words = parse_words (moo_key_file_item_get_content (item),
                             prefix, data->path);

        if (!words)
        {
            g_warning ("%s: empty group", G_STRLOC);
            g_free (parens);
            continue;
        }

        _moo_message ("read %d words for patttern '%s' from %s",
                      g_list_length (words), pattern, data->path);

        group = moo_completion_simple_new_group (MOO_COMPLETION_SIMPLE (data->cmpl), NULL);
        moo_completion_group_add_data (group, words);
        moo_completion_group_set_pattern (group, pattern, parens, n_parens);

        if (script && script[0])
            moo_completion_group_set_script (group, script);
        else if (suffix && suffix[0])
            moo_completion_group_set_suffix (group, suffix);

        g_free (parens);
    }

    moo_key_file_unref (key_file);

    if (!group)
    {
        g_warning ("%s: no completions", G_STRLOC);
        return;
    }
}


static void
cmpl_data_read_python_file (CmplData *data)
{
    FILE *file;
    MooPyObject *dict = NULL;
    MooPyObject *res = NULL;
    MooPyObject *py_cmpl = NULL;
    MooCompletionSimple *cmpl;

    g_return_if_fail (data->cmpl == NULL);
    g_return_if_fail (data->path != NULL);

    if (!moo_python_running ())
        return;

    file = _moo_fopen (data->path, "r");
    g_return_if_fail (file != NULL);

    dict = moo_python_create_script_dict ("__completion_module__");
    res = moo_python_run_file (file, data->path, dict, dict);

    fclose (file);

    if (!res)
    {
        g_message ("error executing file '%s'", data->path);
        moo_PyErr_Print ();
        goto out;
    }

    py_cmpl = moo_py_dict_get_item (dict, "__completion__");

    if (!py_cmpl)
    {
        g_message ("no '__completion__' variable in file '%s'", data->path);
        goto out;
    }

    cmpl = moo_gobject_from_py_object (py_cmpl);

    if (!MOO_IS_TEXT_COMPLETION (cmpl))
    {
        g_message ("'__completion__' variable in file '%s' is not of type MooTextCompletion", data->path);
        goto out;
    }

    data->cmpl = g_object_ref (cmpl);

out:
    if (!data->cmpl)
        data->cmpl = moo_completion_simple_new_text (NULL);

    moo_Py_DECREF (dict);
    moo_Py_DECREF (res);
    moo_Py_DECREF (py_cmpl);
}


static void
cmpl_data_read_file (CmplData *data)
{
    g_return_if_fail (data->path != NULL);
    g_return_if_fail (data->cmpl == NULL);

    switch (data->type)
    {
        case DATA_FILE_SIMPLE:
            cmpl_data_read_simple_file (data);
            break;
        case DATA_FILE_CONFIG:
            cmpl_data_read_config_file (data);
            break;
        case DATA_FILE_PYTHON:
            cmpl_data_read_python_file (data);
            break;
    }
}


static CmplData *
cmpl_plugin_load_data (CmplPlugin *plugin,
                       const char *id)
{
    CmplData *data;

    g_return_val_if_fail (id != NULL, NULL);

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


void
_cmpl_plugin_set_lang_completion (CmplPlugin        *plugin,
                                  const char        *lang,
                                  MooTextCompletion *cmpl)
{
    CmplData *data;

    g_return_if_fail (!cmpl || MOO_IS_TEXT_COMPLETION (cmpl));

    if (!lang || !lang[0])
        lang = _moo_lang_id (NULL);

    data = cmpl_data_new ();
    data->cmpl = g_object_ref (cmpl);
    g_hash_table_insert (plugin->data, g_strdup (lang), data);
}


void
_cmpl_plugin_set_doc_completion (CmplPlugin        *plugin,
                                 MooEdit           *doc,
                                 MooTextCompletion *cmpl)
{
    g_return_if_fail (MOO_IS_EDIT (doc));
    g_return_if_fail (!cmpl || MOO_IS_TEXT_COMPLETION (cmpl));

    g_object_set_qdata_full (G_OBJECT (doc),
                             plugin->cmpl_quark,
                             g_object_ref (cmpl),
                             g_object_unref);
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
    _moo_message ("found file '%s' for lang '%s'", path, id);
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
        guint i, len;
        char *file, *name, *id;
        DataFileType type = DATA_FILE_INVALID;

        const char *suffixes[] = {
            CMPL_FILE_SUFFIX_LIST,
            CMPL_FILE_SUFFIX_CONFIG,
            CMPL_FILE_SUFFIX_PYTHON
        };
        DataFileType types[] = {
            DATA_FILE_SIMPLE,
            DATA_FILE_CONFIG,
            DATA_FILE_PYTHON
        };

        name = g_ascii_strdown (base, -1);
        len = strlen (name);

        for (i = 0; i < G_N_ELEMENTS (suffixes); ++i)
        {
            if (g_str_has_suffix (base, suffixes[i]))
            {
                name[len - strlen (suffixes[i])] = 0;

                if (name[0])
                    type = types[i];

                break;
            }
        }

        if (type == DATA_FILE_INVALID)
            continue;

        id = _moo_lang_id_from_name (name);
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

    dirs = _moo_strv_reverse (moo_get_data_subdirs (CMPL_DIR, MOO_DATA_SHARE, &n_dirs));

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


static MooTextCompletion *
get_doc_completion (CmplPlugin *plugin,
                    MooEdit    *doc)
{
    MooTextCompletion *cmpl = NULL;

    cmpl = g_object_get_qdata (G_OBJECT (doc), plugin->cmpl_quark);

    if (!cmpl)
    {
        MooLang *lang;
        CmplData *data;

        lang = moo_text_view_get_lang (MOO_TEXT_VIEW (doc));

        data = cmpl_plugin_load_data (plugin, _moo_lang_id (lang));
        g_return_val_if_fail (data != NULL, NULL);

        cmpl = data->cmpl;
    }

    return cmpl;
}


static void
completion_finished (MooTextCompletion *cmpl,
                     CmplPlugin        *plugin)
{
    plugin->working = FALSE;
    g_signal_handlers_disconnect_by_func (cmpl, (gpointer) completion_finished, plugin);
}

void
_completion_complete (CmplPlugin *plugin,
                      MooEdit    *doc,
                      gboolean    automatic)
{
    MooTextCompletion *cmpl;

    g_return_if_fail (!plugin->working);
    g_return_if_fail (MOO_IS_EDIT (doc));

    cmpl = get_doc_completion (plugin, doc);

    if (cmpl)
    {
        plugin->working = TRUE;
        moo_text_completion_set_doc (cmpl, GTK_TEXT_VIEW (doc));
        g_signal_connect (cmpl, "finish", G_CALLBACK (completion_finished), plugin);
        moo_text_completion_try_complete (cmpl, automatic);
    }
}


static gboolean
popup_timeout (CmplPlugin *plugin)
{
    plugin->popup_timeout = 0;

    if (!plugin->working)
        _completion_complete (plugin, plugin->focused_doc, TRUE);

    return FALSE;
}


static void
remove_popup_timeout (CmplPlugin *plugin)
{
    if (plugin->popup_timeout)
        g_source_remove (plugin->popup_timeout);
    plugin->popup_timeout = 0;
}

static void
reinstall_popup_timeout (CmplPlugin *plugin)
{
    remove_popup_timeout (plugin);

    if (!plugin->working)
    {
        if (!plugin->popup_timeout)
            plugin->popup_timeout =
                _moo_timeout_add (plugin->popup_interval, (GSourceFunc) popup_timeout, plugin);
    }
}

static gboolean
doc_key_press (CmplPlugin *plugin)
{
    remove_popup_timeout (plugin);
    return FALSE;
}

static gboolean
doc_char_inserted (CmplPlugin *plugin)
{
    reinstall_popup_timeout (plugin);
    return FALSE;
}

static void
connect_doc (CmplPlugin *plugin,
             MooEdit    *doc)
{
    g_signal_connect_swapped (doc, "key-press-event",
                              G_CALLBACK (doc_key_press),
                              plugin);
    g_signal_connect_swapped (doc, "char-inserted",
                              G_CALLBACK (doc_char_inserted),
                              plugin);
}

static void
disconnect_doc (CmplPlugin *plugin,
                MooEdit    *doc)
{
    remove_popup_timeout (plugin);
    g_signal_handlers_disconnect_by_func (doc, (gpointer) doc_key_press, plugin);
    g_signal_handlers_disconnect_by_func (doc, (gpointer) doc_char_inserted, plugin);
}

void
_cmpl_plugin_set_focused_doc (CmplPlugin *plugin,
                              MooEdit    *doc)
{
    if (plugin->focused_doc == doc)
        return;

    if (plugin->focused_doc)
    {
        disconnect_doc (plugin, plugin->focused_doc);
        plugin->focused_doc = NULL;
    }

    if (doc)
    {
        plugin->focused_doc = doc;
        connect_doc (plugin, doc);
    }
}
