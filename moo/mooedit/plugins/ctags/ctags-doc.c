/*
 *   ctags-doc.c
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

#include "config.h"
#include "ctags-doc.h"
#include "ctags-view.h"
#include "readtags.h"
#include <mooutils/mooutils-misc.h>
#include <gtk/gtk.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdlib.h>


G_DEFINE_TYPE (MooCtagsDocPlugin, _moo_ctags_doc_plugin, MOO_TYPE_DOC_PLUGIN)

struct _MooCtagsDocPluginPrivate
{
    GtkTreeStore *store;
    guint update_idle;
};

typedef struct {
    const char *name;
    const char *opts;
    void (*process_list) (GSList *entries, GtkTreeStore *store);
} MooCtagsLanguage;

static gboolean moo_ctags_doc_plugin_create         (MooCtagsDocPlugin  *plugin);
static void     moo_ctags_doc_plugin_destroy        (MooCtagsDocPlugin  *plugin);

static void     moo_ctags_doc_plugin_queue_update   (MooCtagsDocPlugin  *plugin);
static gboolean moo_ctags_doc_plugin_update         (MooCtagsDocPlugin  *plugin);

static GSList  *moo_ctags_parse_file                (const char         *filename,
                                                     const char         *opts);


static void
_moo_ctags_doc_plugin_class_init (MooCtagsDocPluginClass *klass)
{
    MooDocPluginClass *plugin_class = MOO_DOC_PLUGIN_CLASS (klass);

    plugin_class->create = (MooDocPluginCreateFunc) moo_ctags_doc_plugin_create;
    plugin_class->destroy = (MooDocPluginDestroyFunc) moo_ctags_doc_plugin_destroy;

    g_type_class_add_private (klass, sizeof (MooCtagsDocPluginPrivate));
}

static void
_moo_ctags_doc_plugin_init (MooCtagsDocPlugin *plugin)
{
    plugin->priv = G_TYPE_INSTANCE_GET_PRIVATE (plugin, MOO_TYPE_CTAGS_DOC_PLUGIN,
                                                MooCtagsDocPluginPrivate);
}


static void
moo_ctags_doc_plugin_queue_update (MooCtagsDocPlugin *plugin)
{
    if (!plugin->priv->update_idle)
        plugin->priv->update_idle =
            g_idle_add_full (G_PRIORITY_LOW,
                             (GSourceFunc) moo_ctags_doc_plugin_update,
                             plugin, NULL);
}

static void
ensure_model (MooCtagsDocPlugin *plugin)
{
    if (!plugin->priv->store)
        plugin->priv->store = gtk_tree_store_new (2, MOO_TYPE_CTAGS_ENTRY, G_TYPE_STRING);
}

static gboolean
moo_ctags_doc_plugin_create (MooCtagsDocPlugin *plugin)
{
    ensure_model (plugin);
    moo_ctags_doc_plugin_queue_update (plugin);

    g_signal_connect_swapped (MOO_DOC_PLUGIN (plugin)->doc, "save-after",
                              G_CALLBACK (moo_ctags_doc_plugin_queue_update),
                              plugin);

    return TRUE;
}

static void
moo_ctags_doc_plugin_destroy (MooCtagsDocPlugin *plugin)
{
    g_signal_handlers_disconnect_by_func (MOO_DOC_PLUGIN (plugin)->doc,
                                          (gpointer) moo_ctags_doc_plugin_queue_update,
                                          plugin);

    if (plugin->priv->update_idle)
        g_source_remove (plugin->priv->update_idle);
    plugin->priv->update_idle = 0;

    g_object_unref (plugin->priv->store);
}


GtkTreeModel *
_moo_ctags_doc_plugin_get_store (MooCtagsDocPlugin *plugin)
{
    g_return_val_if_fail (MOO_IS_CTAGS_DOC_PLUGIN (plugin), NULL);
    ensure_model (plugin);
    return GTK_TREE_MODEL (plugin->priv->store);
}


static void
get_iter_for_class (GtkTreeStore *store,
                    GtkTreeIter  *class_iter,
                    const char   *name,
                    GHashTable   *classes)
{
    GtkTreeIter *stored = g_hash_table_lookup (classes, name);

    if (!stored)
    {
        GtkTreeIter iter;
        char *label;

        label = g_strdup_printf ("class %s", name);

        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, MOO_CTAGS_VIEW_COLUMN_LABEL, label, -1);

        stored = gtk_tree_iter_copy (&iter);
        g_hash_table_insert (classes, g_strdup (name), stored);

        g_free (label);
    }

    *class_iter = *stored;
}

static void
process_list_simple (GSList       *entries,
                     GtkTreeStore *store)
{
    GHashTable *classes;
    GtkTreeIter funcs_iter;
    gboolean funcs_added = FALSE;

    gtk_tree_store_append (store, &funcs_iter, NULL);
    gtk_tree_store_set (store, &funcs_iter,
                        MOO_CTAGS_VIEW_COLUMN_LABEL, "Functions",
                        -1);

    classes = g_hash_table_new_full (g_str_hash, g_str_equal, g_free,
                                     (GDestroyNotify) gtk_tree_iter_free);

    while (entries)
    {
        MooCtagsEntry *entry;

        entry = entries->data;
        entries = entries->next;

        if (entry->klass || !strcmp (entry->kind, "c"))
        {
            GtkTreeIter iter;
            GtkTreeIter parent_iter;

            get_iter_for_class (store, &parent_iter,
                                entry->klass ? entry->klass : entry->name,
                                classes);

            if (entry->klass)
            {
                gtk_tree_store_append (store, &iter, &parent_iter);
                gtk_tree_store_set (store, &iter, MOO_CTAGS_VIEW_COLUMN_ENTRY, entry, -1);
            }
            else
            {
                gtk_tree_store_set (store, &parent_iter, MOO_CTAGS_VIEW_COLUMN_ENTRY, entry, -1);
            }
        }
        else if (!strcmp (entry->kind, "f"))
        {
            GtkTreeIter iter;
            gtk_tree_store_append (store, &iter, &funcs_iter);
            gtk_tree_store_set (store, &iter, MOO_CTAGS_VIEW_COLUMN_ENTRY, entry, -1);
            funcs_added = TRUE;
        }
    }

    if (!funcs_added)
        gtk_tree_store_remove (store, &funcs_iter);

    g_hash_table_destroy (classes);
}

static void
process_list_c (GSList       *entries,
                GtkTreeStore *store)
{
    process_list_simple (entries, store);
}

static void
process_list_python (GSList       *entries,
                     GtkTreeStore *store)
{
    process_list_simple (entries, store);
}

static MooCtagsLanguage *
_moo_ctags_language_find_for_name (const char *lang_name)
{
    static GHashTable *langs_hash;

    if (!langs_hash)
    {
        static MooCtagsLanguage langs[] = {
            { "c", "--fields=afksS --c-kinds=cf --language-force=c -I G_BEGIN_DECLS,G_END_DECLS", process_list_c },
            { "c++", "--fields=afksS --c-kinds=cf --language-force=c++ -I G_BEGIN_DECLS,G_END_DECLS", process_list_c },
            { "python", "--fields=afksS --c-kinds=cf --language-force=python", process_list_python },
        };

        langs_hash = g_hash_table_new (g_str_hash, g_str_equal);
        g_hash_table_insert (langs_hash, (char*) "c", &langs[0]);
        g_hash_table_insert (langs_hash, (char*) "cpp", &langs[1]);
        g_hash_table_insert (langs_hash, (char*) "chdr", &langs[1]);
        g_hash_table_insert (langs_hash, (char*) "python", &langs[2]);
    }

    return g_hash_table_lookup (langs_hash, lang_name);
}


static void
process_entries (MooCtagsDocPlugin *plugin,
                 GSList            *list,
                 MooCtagsLanguage  *lang)
{
    g_return_if_fail (lang != NULL || lang->process_list != NULL);
    lang->process_list (list, plugin->priv->store);
}

static gboolean
moo_ctags_doc_plugin_update (MooCtagsDocPlugin *plugin)
{
    MooEdit *doc;
    char *filename;
    GSList *list = NULL;
    MooCtagsLanguage *ctags_lang;
    MooLang *lang;

    plugin->priv->update_idle = 0;

    gtk_tree_store_clear (plugin->priv->store);

    doc = MOO_DOC_PLUGIN(plugin)->doc;
    filename = moo_edit_get_filename (doc);
    lang = moo_text_view_get_lang (MOO_TEXT_VIEW (doc));
    ctags_lang = _moo_ctags_language_find_for_name (_moo_lang_id (lang));

    if (filename && ctags_lang && (list = moo_ctags_parse_file (filename, ctags_lang->opts)))
        process_entries (plugin, list, ctags_lang);

    g_slist_foreach (list, (GFunc) _moo_ctags_entry_unref, NULL);
    g_slist_free (list);
    g_free (filename);
    return FALSE;
}


GType
_moo_ctags_entry_get_type (void)
{
    static GType type;

    if (G_UNLIKELY (!type))
        type = g_boxed_type_register_static ("MooCtagsEntry",
                                             (GBoxedCopyFunc) _moo_ctags_entry_ref,
                                             (GBoxedFreeFunc) _moo_ctags_entry_unref);

    return type;
}


MooCtagsEntry *
_moo_ctags_entry_ref (MooCtagsEntry *entry)
{
    g_return_val_if_fail (entry != NULL, NULL);
    entry->ref_count++;
    return entry;
}


void
_moo_ctags_entry_unref (MooCtagsEntry *entry)
{
    if (entry && !--entry->ref_count)
    {
        g_free (entry->name);
        g_free (entry->klass);
        _moo_free (MooCtagsEntry, entry);
    }
}


static MooCtagsEntry *
moo_ctags_entry_new (const tagEntry *te)
{
    MooCtagsEntry *entry;
    guint i;

    g_return_val_if_fail (te != NULL, NULL);

    entry = _moo_new (MooCtagsEntry);
    entry->ref_count = 1;

    entry->name = g_strdup (te->name);
    entry->line = (int) te->address.lineNumber - 1;
    entry->kind = _moo_intern_string (te->kind);
    entry->klass = NULL;
    entry->file_scope = te->fileScope != 0;

    for (i = 0; i < te->fields.count; ++i)
    {
        if (!strcmp (te->fields.list[i].key, "class"))
            entry->klass = g_strdup (te->fields.list[i].value);
    }

    return entry;
}


static GSList *
_moo_ctags_read_tags (const char *filename)
{
    tagFile *file;
    tagFileInfo file_info;
    tagEntry tentry;
    GSList *list = NULL;

    g_return_val_if_fail (filename != NULL, NULL);

    file = tagsOpen (filename, &file_info);

    if (!file_info.status.opened)
    {
        char *file_utf8 = g_filename_display_name (filename);
        g_warning ("could not read file '%s': %s", file_utf8,
                   g_strerror (file_info.status.error_number));
        g_free (file_utf8);
        return NULL;
    }

    while (tagsNext (file, &tentry) == TagSuccess)
    {
        MooCtagsEntry *e = moo_ctags_entry_new (&tentry);
        if (e)
            list = g_slist_prepend (list, e);
    }

    tagsClose (file);

    return g_slist_reverse (list);
}


#if 0
static MooCtagsEntry *
parse_line (const char *line,
            const char *end)
{
    MooCtagsEntry *entry;
    char **tokens;

    tokens = _moo_ascii_strnsplit (line, end - line, 4);

    if (!tokens || g_strv_length (tokens) != 4)
    {
        g_strfreev (tokens);
        return NULL;
    }

    entry = _moo_new (MooCtagsEntry);
    entry->ref_count = 1;

    entry->name = g_strdup (tokens[0]);
    entry->line = strtol (tokens[2], NULL, 10);
    entry->kind = _moo_intern_string (tokens[1]);

    g_strfreev (tokens);
    return entry;
}

static GSList *
_moo_ctags_parse_output (char *output)
{
    GSList *list = NULL;
    char *endl;

    while ((endl = strchr (output, '\n')))
    {
        MooCtagsEntry *entry;
        if ((entry = parse_line (output, endl)))
            list = g_slist_prepend (list, entry);
        output = endl + 1;
    }

    return g_slist_reverse (list);
}

GSList *
_moo_ctags_parse_file (const char *filename,
                       G_GNUC_UNUSED const char *type)
{
    GError *error = NULL;
    const char *argv[4];
    int exit_status;
    GSList *list = NULL;
    char *output;

    g_return_val_if_fail (filename != NULL, NULL);

    argv[0] = "ctags";
    argv[1] = "-x";
    argv[2] = filename;
    argv[3] = NULL;

    if (!g_spawn_sync (NULL, (char**) argv, NULL, G_SPAWN_SEARCH_PATH,
                       NULL, NULL, &output, NULL, &exit_status, &error))
    {
        g_warning ("%s: could not run ctags command: %s", G_STRFUNC, error->message);
        g_error_free (error);
        goto out;
    }

    if (exit_status != 0)
    {
        g_warning ("%s: ctags command returned an error", G_STRFUNC);
        goto out;
    }

    list = _moo_ctags_parse_output (output);

out:
    g_free (output);
    return list;
}
#endif


static char *
get_tmp_file (void)
{
    char *file;
    int fd;
    GError *error = NULL;

    fd = g_file_open_tmp ("moo-ctags-plugin-XXXXXX", &file, &error);

    if (fd == -1)
    {
        g_critical ("%s: could not open temporary file: %s",
                    G_STRFUNC, error->message);
        g_error_free (error);
        return NULL;
    }

    close (fd);
    return file;
}

static GSList *
moo_ctags_parse_file (const char *filename,
                      const char *opts)
{
    char *tags_file;
    GError *error = NULL;
    int exit_status;
    GSList *list = NULL;
    GString *cmd_line;
    char *quoted_tags_file, *quoted_filename;

    g_return_val_if_fail (filename != NULL, NULL);

    if (!(tags_file = get_tmp_file ()))
        return NULL;

    cmd_line = g_string_new ("ctags ");

    if (opts && opts[0])
    {
        g_string_append (cmd_line, opts);
        g_string_append_c (cmd_line, ' ');
    }

    g_string_append (cmd_line, "--excmd=number ");

    quoted_tags_file = g_shell_quote (tags_file);
    quoted_filename = g_shell_quote (filename);
    g_string_append_printf (cmd_line, "-f %s %s", quoted_tags_file, quoted_filename);

    if (!g_spawn_command_line_sync (cmd_line->str, NULL, NULL, &exit_status, &error))
    {
        g_warning ("%s: could not run ctags command: %s", G_STRFUNC, error->message);
        g_error_free (error);
        goto out;
    }

    if (exit_status != 0)
    {
        g_warning ("%s: ctags command returned an error", G_STRFUNC);
        goto out;
    }

    list = _moo_ctags_read_tags (tags_file);

out:
    g_free (quoted_filename);
    g_free (quoted_tags_file);
    g_string_free (cmd_line, TRUE);
    unlink (tags_file);
    g_free (tags_file);
    return list;
}
