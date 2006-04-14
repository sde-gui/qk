/*
 *   moocompletion.c
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

#include "moocompletion.h"
#include "mooedit/mootextcompletion.h"
#include "mooutils/mooutils-misc.h"
#include <sys/stat.h>
#include <errno.h>
#include <string.h>


typedef struct {
    char *path;
    time_t mtime;
    GCompletion *cmpl;
    char **words;
    GtkListStore *store;
} CmplData;

static CmplData *cmpl_data_new              (void);
static void      cmpl_data_free             (CmplData   *data);
static void      cmpl_data_clear            (CmplData   *data);

static CmplData *cmpl_plugin_load_data      (CmplPlugin *plugin,
                                             const char *id);


void
_moo_completion_callback (MooEditWindow *window)
{
    CmplPlugin *plugin;
    MooEdit *doc;

    plugin = moo_plugin_lookup (CMPL_PLUGIN_ID);
    g_return_if_fail (plugin != NULL);

    doc = moo_edit_window_get_active_doc (window);
    g_return_if_fail (doc != NULL);

    _moo_completion_complete (plugin, doc);
}


static void
cmpl_data_read_file (CmplData *data)
{
    GError *error = NULL;
    char *contents;
    char **words, **p;
    GList *list = NULL;

    g_return_if_fail (data->cmpl == NULL);
    g_return_if_fail (data->path != NULL);

    data->cmpl = g_completion_new (NULL);
    data->store = gtk_list_store_new (1, G_TYPE_STRING);

    g_file_get_contents (data->path, &contents, NULL, &error);

    if (error)
    {
        g_warning ("%s: could not read file '%s': %s",
                   G_STRLOC, data->path, error->message);
        g_error_free (error);
        return;
    }

    words = g_strsplit_set (contents, " \t\r\n", 0);

    for (p = words; p && **p; ++p)
    {
        if (p[0][0] && p[0][1])
        {
            if (!g_utf8_validate (*p, -1, NULL))
            {
                g_critical ("%s: invalid utf8 in '%s'",
                            G_STRLOC, data->path);
            }
            else
            {
                list = g_list_prepend (list, *p);
                *p = NULL;
            }
        }

        g_free (*p);
    }

    if (list)
    {
        guint n_words, i;
        GList *l;

        g_completion_add_items (data->cmpl, list);

        n_words = g_list_length (list);
        data->words = g_new (char*, n_words + 1);
        data->words[n_words] = 0;

        g_message ("read %d words from %s", n_words, data->path);

        for (i = 0, l = list; i < n_words; ++i, l = l->next)
            data->words[i] = l->data;

        g_list_free (list);
    }
    else
    {
        g_message ("read no words from %s", data->path);
    }

    g_free (contents);
    g_free (words);
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
    g_strfreev (data->words);
    g_free (data->path);
    if (data->cmpl)
        g_completion_free (data->cmpl);
    if (data->store)
        g_object_unref (data->store);
    data->words = NULL;
    data->path = NULL;
    data->cmpl = NULL;
    data->store = NULL;
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
cmpl_plugin_check_file (CmplPlugin *plugin,
                        const char *path,
                        const char *id)
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
    data->mtime = buf.st_mtime;
    g_message ("found file '%s' for lang '%s'", path, id);
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

        base_len = strlen (base);

        if (base_len <= strlen (CMPL_FILE_SUFFIX) ||
            !g_str_has_suffix (base, CMPL_FILE_SUFFIX))
                continue;

        name = g_strndup (base, base_len - strlen (CMPL_FILE_SUFFIX));
        id = moo_lang_id_from_name (name);
        file = g_build_filename (path, base, NULL);

        cmpl_plugin_check_file (plugin, file, id);

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


inline static gboolean
is_word (const char *p)
{
    gunichar c = g_utf8_get_char (p);
    return c == '_' || g_unichar_isalnum (c);
}


static gboolean
find_word_end (const char *text,
               char      **word_start)
{
    char *p;
    guint len = g_utf8_strlen (text, -1);

    if (!len)
        return FALSE;

    p = g_utf8_offset_to_pointer (text, len - 1);
    *word_start = NULL;

    if (!is_word (p))
        return FALSE;

    while (is_word (p))
    {
        *word_start = p;

        if (p > text)
            p = g_utf8_prev_char (p);
        else
            break;
    }

    return *word_start != NULL;
}


void
_moo_completion_complete (CmplPlugin *plugin,
                          MooEdit    *doc)
{
    MooLang *lang;
    CmplData *data;
    MooTextCompletion *cmpl;
    GtkTextIter start, end;
    gboolean get_all = TRUE;

    lang = moo_text_view_get_lang (MOO_TEXT_VIEW (doc));

    data = cmpl_plugin_load_data (plugin, lang ? lang->id : NULL);
    g_return_if_fail (data != NULL);

    if (!data->words)
        return;

    moo_text_view_get_cursor (MOO_TEXT_VIEW (doc), &end);
    start = end;

    if (!gtk_text_iter_starts_line (&end))
    {
        char *text, *word_start;
        GtkTextBuffer *buffer;
        GtkTextIter line_start;

        line_start = end;
        gtk_text_iter_set_line_offset (&line_start, 0);

        buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (doc));
        text = gtk_text_buffer_get_slice (buffer, &line_start, &end, TRUE);

        if (find_word_end (text, &word_start))
        {
            char *new_word_start;
            GList *words;
            int offset;

            offset = g_utf8_pointer_to_offset (text, word_start);
            gtk_text_iter_set_line_offset (&line_start, offset);

            words = g_completion_complete_utf8 (data->cmpl, word_start, &new_word_start);

            if (!words)
            {
                g_free (text);
                return;
            }

            get_all = FALSE;

            if (strcmp (new_word_start, word_start))
            {
                gtk_text_buffer_delete (buffer, &line_start, &end);
                gtk_text_buffer_insert (buffer, &end, new_word_start, -1);
            }

            if (!words->next)
                return;

            start = end;
            gtk_text_iter_set_line_offset (&start, offset);

            gtk_list_store_clear (data->store);

            while (words)
            {
                GtkTreeIter iter;
                gtk_list_store_append (data->store, &iter);
                gtk_list_store_set (data->store, &iter, 0, words->data, -1);
                words = words->next;
            }

            g_free (new_word_start);
        }

        g_free (text);
    }

    if (get_all)
    {
        char **p;

        gtk_list_store_clear (data->store);

        for (p = data->words; p && *p; ++p)
        {
            GtkTreeIter iter;
            gtk_list_store_append (data->store, &iter);
            gtk_list_store_set (data->store, &iter, 0, *p, -1);
        }
    }

    cmpl = g_object_get_data (G_OBJECT (doc), "moo-completion");

    if (!cmpl)
    {
        cmpl = moo_text_completion_new ();
        moo_text_completion_set_doc (cmpl, GTK_TEXT_VIEW (doc));
        g_object_set_data_full (G_OBJECT (doc), "moo-completion",
                                cmpl, g_object_unref);
        moo_text_completion_set_model (cmpl, GTK_TREE_MODEL (data->store), 0);
    }

    moo_text_completion_show (cmpl, &start, &end);
}
