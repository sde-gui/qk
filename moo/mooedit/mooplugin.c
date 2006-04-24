/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mooplugin.c
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "mooedit/mooplugin.h"
#include "mooedit/moopluginprefs-glade.h"
#include "mooedit/plugins/mooeditplugins.h"
#include "moopython/mooplugin-python.h"
#include "mooutils/mooprefsdialog.h"
#include "mooutils/moostock.h"
#include "mooutils/mooutils-misc.h"
#include <string.h>
#include <gmodule.h>

#ifdef __WIN32__
#include <windows.h>
#endif

#define PLUGIN_PREFS_ENABLED "enabled"
#define LANG_ID(lang) ((lang) ? (lang)->id : "none")


typedef struct {
    MooEditor *editor;
    GSList *list; /* MooPlugin* */
    GHashTable *names;
    char **dirs;
    gboolean dirs_read;
} PluginStore;

static PluginStore *plugin_store = NULL;

static void     plugin_store_init       (void);
static void     plugin_store_add        (MooPlugin      *plugin);

static void     moo_plugin_class_init   (MooPluginClass *klass);
static void     some_plugin_class_init  (gpointer        klass);

static gboolean plugin_init             (MooPlugin      *plugin);
static void     plugin_deinit           (MooPlugin      *plugin);
static void     plugin_attach_win       (MooPlugin      *plugin,
                                         MooEditWindow  *window);
static void     plugin_detach_win       (MooPlugin      *plugin,
                                         MooEditWindow  *window);
static void     plugin_attach_doc       (MooPlugin      *plugin,
                                         MooEditWindow  *window,
                                         MooEdit        *doc);
static void     plugin_detach_doc       (MooPlugin      *plugin,
                                         MooEditWindow  *window,
                                         MooEdit        *doc);

static gboolean plugin_info_check       (MooPluginInfo  *info);

static char    *make_prefs_key          (MooPlugin      *plugin,
                                         const char     *key);
static GQuark   make_id_quark           (MooPlugin      *plugin);

static MooWinPlugin *window_get_plugin  (MooEditWindow  *window,
                                         MooPlugin      *plugin);
static MooDocPlugin *doc_get_plugin     (MooEdit        *doc,
                                         MooPlugin      *plugin);
static void     window_set_plugin       (MooEditWindow  *window,
                                         MooPlugin      *plugin,
                                         MooWinPlugin   *win_plugin);
static void     doc_set_plugin          (MooEdit        *doc,
                                         MooPlugin      *plugin,
                                         MooDocPlugin   *doc_plugin);

static gboolean moo_plugin_registered   (GType           type);


static gpointer parent_class = NULL;


GType
moo_plugin_get_type (void)
{
    static GType type = 0;

    if (G_UNLIKELY (type == 0))
    {
        static const GTypeInfo info = {
            sizeof (MooPluginClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) moo_plugin_class_init,
            (GClassFinalizeFunc) NULL,
            NULL,   /* class_data */
            sizeof (MooPlugin),
            0,      /* n_preallocs */
            NULL,
            NULL    /* value_table */
        };

        type = g_type_register_static (G_TYPE_OBJECT, "MooPlugin", &info, 0);
    }

    return type;
}


GType
moo_win_plugin_get_type (void)
{
    static GType type = 0;

    if (G_UNLIKELY (type == 0))
    {
        static const GTypeInfo info = {
            sizeof (MooWinPluginClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) some_plugin_class_init,
            (GClassFinalizeFunc) NULL,
            NULL,   /* class_data */
            sizeof (MooWinPlugin),
            0,      /* n_preallocs */
            NULL,
            NULL    /* value_table */
        };

        type = g_type_register_static (G_TYPE_OBJECT, "MooWinPlugin", &info, 0);
    }

    return type;
}


GType
moo_doc_plugin_get_type (void)
{
    static GType type = 0;

    if (G_UNLIKELY (type == 0))
    {
        static const GTypeInfo info = {
            sizeof (MooDocPluginClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) some_plugin_class_init,
            (GClassFinalizeFunc) NULL,
            NULL,   /* class_data */
            sizeof (MooDocPlugin),
            0,      /* n_preallocs */
            NULL,
            NULL    /* value_table */
        };

        type = g_type_register_static (G_TYPE_OBJECT, "MooDocPlugin", &info, 0);
    }

    return type;
}


static void
some_plugin_class_init (gpointer klass)
{
    parent_class = g_type_class_peek_parent (klass);
}


static void
moo_plugin_finalize (GObject *object)
{
    MooPlugin *plugin = MOO_PLUGIN (object);

    if (plugin->langs)
        g_hash_table_destroy (plugin->langs);

    G_OBJECT_CLASS(parent_class)->finalize (object);
}


static void
moo_plugin_class_init (MooPluginClass *klass)
{
    some_plugin_class_init (klass);
    G_OBJECT_CLASS(klass)->finalize = moo_plugin_finalize;
}


gboolean
moo_plugin_register (GType type)
{
    MooPluginClass *klass;
    MooPlugin *plugin;
    char *prefs_key;
    GSList *l, *windows;

    g_return_val_if_fail (g_type_is_a (type, MOO_TYPE_PLUGIN), FALSE);

    klass = g_type_class_ref (type);
    g_return_val_if_fail (klass != NULL, FALSE);

    if (klass->plugin_system_version != MOO_PLUGIN_CURRENT_VERSION)
    {
        g_message ("%s: plugin %s of version %d is incompatible with "
                   "current version %d", G_STRLOC, g_type_name (type),
                   klass->plugin_system_version, MOO_PLUGIN_CURRENT_VERSION);
        return FALSE;
    }

    if (moo_plugin_registered (type))
    {
        g_warning ("%s: plugin %s already registered",
                   G_STRLOC, g_type_name (type));
        return FALSE;
    }

    plugin = g_object_new (type, NULL);

    if (!plugin_info_check (plugin->info))
    {
        g_warning ("%s: invalid info in plugin %s",
                   G_STRLOC, g_type_name (type));
        g_object_unref (plugin);
        return FALSE;
    }

    if (plugin->info->langs)
    {
        char **langs, **p;
        GHashTable *table;

        langs = g_strsplit_set (plugin->info->langs, " \t\r\n", 0);
        table = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

        for (p = langs; p && *p; ++p)
        {
            char *id;

            if (!**p)
                continue;

            id = moo_lang_id_from_name (*p);

            if (!g_hash_table_lookup (table, id))
                g_hash_table_insert (table, id, id);
            else
                g_free (id);
        }

        if (!g_hash_table_size (table))
        {
            g_warning ("%s: invalid langs string '%s'", G_STRLOC, plugin->info->langs);
            g_hash_table_destroy (table);
        }
        else
        {
            plugin->langs = table;
        }

        g_strfreev (langs);
    }

    if (moo_plugin_lookup (moo_plugin_id (plugin)))
    {
        g_warning ("%s: plugin with id %s already registered",
                   G_STRLOC, moo_plugin_id (plugin));
        g_object_unref (plugin);
        return FALSE;
    }

    plugin->id_quark = make_id_quark (plugin);

    prefs_key = make_prefs_key (plugin, PLUGIN_PREFS_ENABLED);
    moo_prefs_new_key_bool (prefs_key, moo_plugin_enabled (plugin));
    plugin->info->params->enabled = moo_prefs_get_bool (prefs_key);
    g_free (prefs_key);

    if (!plugin_init (plugin))
    {
        g_object_unref (plugin);
        return FALSE;
    }

    plugin_store_add (plugin);

    windows = moo_editor_list_windows (plugin_store->editor);
    for (l = windows; l != NULL; l = l->next)
        plugin_attach_win (plugin, l->data);
    g_slist_free (windows);

    return TRUE;
}


static gboolean
plugin_init (MooPlugin *plugin)
{
    MooPluginClass *klass;

    g_return_val_if_fail (MOO_IS_PLUGIN (plugin), FALSE);

    if (!moo_plugin_enabled (plugin) || plugin->initialized)
        return TRUE;

    klass = MOO_PLUGIN_GET_CLASS (plugin);

    if (!klass->init || klass->init (plugin))
    {
        plugin->initialized = TRUE;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


static void
plugin_deinit (MooPlugin *plugin)
{
    MooPluginClass *klass;

    g_return_if_fail (MOO_IS_PLUGIN (plugin));

    if (!plugin->initialized)
        return;

    klass = MOO_PLUGIN_GET_CLASS (plugin);

    if (klass->deinit)
        klass->deinit (plugin);

    plugin->initialized = FALSE;
}


static void
plugin_attach_win (MooPlugin      *plugin,
                   MooEditWindow  *window)
{
    MooPluginClass *klass;
    MooWinPluginClass *wklass;
    MooWinPlugin *win_plugin;
    GType wtype;
    GSList *l, *docs;

    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));
    g_return_if_fail (MOO_IS_PLUGIN (plugin));

    if (!moo_plugin_enabled (plugin))
        return;

    klass = MOO_PLUGIN_GET_CLASS (plugin);

    if (klass->attach_win)
        klass->attach_win (plugin, window);

    wtype = plugin->win_plugin_type;

    if (wtype && g_type_is_a (wtype, MOO_TYPE_WIN_PLUGIN))
    {
        win_plugin = g_object_new (wtype, NULL);
        g_return_if_fail (win_plugin != NULL);

        win_plugin->plugin = plugin;
        win_plugin->window = window;
        wklass = MOO_WIN_PLUGIN_GET_CLASS (win_plugin);

        if (!wklass->create || wklass->create (win_plugin))
            window_set_plugin (window, plugin, win_plugin);
        else
            g_object_unref (win_plugin);
    }

    docs = moo_edit_window_list_docs (window);
    for (l = docs; l != NULL; l = l->next)
        plugin_attach_doc (plugin, window, l->data);
    g_slist_free (docs);
}


static void
plugin_detach_win (MooPlugin      *plugin,
                   MooEditWindow  *window)
{
    MooPluginClass *klass;
    MooWinPluginClass *wklass;
    MooWinPlugin *win_plugin;
    GSList *l, *docs;

    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));
    g_return_if_fail (MOO_IS_PLUGIN (plugin));

    if (!moo_plugin_enabled (plugin))
        return;

    docs = moo_edit_window_list_docs (window);
    for (l = docs; l != NULL; l = l->next)
        plugin_detach_doc (plugin, window, l->data);
    g_slist_free (docs);

    win_plugin = window_get_plugin (window, plugin);

    if (win_plugin)
    {
        wklass = MOO_WIN_PLUGIN_GET_CLASS (win_plugin);

        if (wklass->destroy)
            wklass->destroy (win_plugin);

        g_object_unref (win_plugin);
        window_set_plugin (window, plugin, NULL);
    }

    klass = MOO_PLUGIN_GET_CLASS (plugin);

    if (klass->detach_win)
        klass->detach_win (plugin, window);
}


static void
plugin_attach_doc (MooPlugin      *plugin,
                   MooEditWindow  *window,
                   MooEdit        *doc)
{
    MooPluginClass *klass;
    MooDocPluginClass *dklass;
    MooDocPlugin *doc_plugin;
    GType dtype;

    g_return_if_fail (!window || MOO_IS_EDIT_WINDOW (window));
    g_return_if_fail (MOO_IS_EDIT (doc));
    g_return_if_fail (MOO_IS_PLUGIN (plugin));

    if (!moo_plugin_enabled (plugin))
        return;

    if (plugin->langs)
    {
        MooLang *lang;
        const char *id;

        lang = moo_text_view_get_lang (MOO_TEXT_VIEW (doc));
        id = LANG_ID (lang);

        if (!g_hash_table_lookup (plugin->langs, id))
            return;
    }

    plugin->docs = g_slist_prepend (plugin->docs, doc);

    klass = MOO_PLUGIN_GET_CLASS (plugin);

    if (klass->attach_doc)
        klass->attach_doc (plugin, doc, window);

    dtype = plugin->doc_plugin_type;

    if (dtype && g_type_is_a (dtype, MOO_TYPE_DOC_PLUGIN))
    {
        doc_plugin = g_object_new (dtype, NULL);
        g_return_if_fail (doc_plugin != NULL);

        doc_plugin->plugin = plugin;
        doc_plugin->window = window;
        doc_plugin->doc = doc;
        dklass = MOO_DOC_PLUGIN_GET_CLASS (doc_plugin);

        if (!dklass->create || dklass->create (doc_plugin))
            doc_set_plugin (doc, plugin, doc_plugin);
        else
            g_object_unref (doc_plugin);
    }
}


static void
plugin_detach_doc (MooPlugin      *plugin,
                   MooEditWindow  *window,
                   MooEdit        *doc)
{
    MooPluginClass *klass;
    MooDocPluginClass *dklass;
    MooDocPlugin *doc_plugin;

    g_return_if_fail (MOO_IS_PLUGIN (plugin));
    g_return_if_fail (!window || MOO_IS_EDIT_WINDOW (window));
    g_return_if_fail (MOO_IS_EDIT (doc));

    if (!moo_plugin_enabled (plugin))
        return;

    if (!g_slist_find (plugin->docs, doc))
        return;

    plugin->docs = g_slist_remove (plugin->docs, doc);

    doc_plugin = doc_get_plugin (doc, plugin);

    if (doc_plugin)
    {
        dklass = MOO_DOC_PLUGIN_GET_CLASS (doc_plugin);

        if (dklass->destroy)
            dklass->destroy (doc_plugin);

        g_object_unref (doc_plugin);
        doc_set_plugin (doc, plugin, NULL);
    }

    klass = MOO_PLUGIN_GET_CLASS (plugin);

    if (klass->detach_doc)
        klass->detach_doc (plugin, doc, window);
}


static MooWinPlugin*
window_get_plugin (MooEditWindow  *window,
                   MooPlugin      *plugin)
{
    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), NULL);
    g_return_val_if_fail (MOO_IS_PLUGIN (plugin), NULL);
    return g_object_get_qdata (G_OBJECT (window), plugin->id_quark);
}


static MooDocPlugin*
doc_get_plugin (MooEdit        *doc,
                MooPlugin      *plugin)
{
    g_return_val_if_fail (MOO_IS_EDIT (doc), NULL);
    g_return_val_if_fail (MOO_IS_PLUGIN (plugin), NULL);
    return g_object_get_qdata (G_OBJECT (doc), plugin->id_quark);
}


static void
window_set_plugin (MooEditWindow  *window,
                   MooPlugin      *plugin,
                   MooWinPlugin   *win_plugin)
{
    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));
    g_return_if_fail (MOO_IS_PLUGIN (plugin));
    g_return_if_fail (!win_plugin || MOO_IS_WIN_PLUGIN (win_plugin));
    g_object_set_qdata (G_OBJECT (window), plugin->id_quark, win_plugin);
}


static void
doc_set_plugin (MooEdit        *doc,
                MooPlugin      *plugin,
                MooDocPlugin   *doc_plugin)
{
    g_return_if_fail (MOO_IS_EDIT (doc));
    g_return_if_fail (MOO_IS_PLUGIN (plugin));
    g_return_if_fail (!doc_plugin || MOO_IS_DOC_PLUGIN (doc_plugin));
    g_object_set_qdata (G_OBJECT (doc), plugin->id_quark, doc_plugin);
}


static void
plugin_store_init (void)
{
    if (!plugin_store)
    {
        static PluginStore store;

        store.editor = moo_editor_instance ();
        store.list = NULL;
        store.names = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

        plugin_store = &store;
    }
}


static void
plugin_store_add (MooPlugin      *plugin)
{
    g_return_if_fail (MOO_IS_PLUGIN (plugin));

    plugin_store_init ();

    g_return_if_fail (!g_hash_table_lookup (plugin_store->names, moo_plugin_id (plugin)));

    plugin_store->list = g_slist_append (plugin_store->list, plugin);
    g_hash_table_insert (plugin_store->names,
                         g_strdup (moo_plugin_id (plugin)),
                         plugin);
}


static void
plugin_store_remove (MooPlugin *plugin)
{
    g_return_if_fail (plugin_store != NULL);
    g_return_if_fail (MOO_IS_PLUGIN (plugin));
    plugin_store->list = g_slist_remove (plugin_store->list, plugin);
    g_hash_table_remove (plugin_store->names, moo_plugin_id (plugin));
}


MooPlugin*
moo_plugin_get (GType type)
{
    GSList *l;

    g_return_val_if_fail (g_type_is_a (type, MOO_TYPE_PLUGIN), NULL);

    plugin_store_init ();

    for (l = plugin_store->list; l != NULL; l = l->next)
        if (G_OBJECT_TYPE (l->data) == type)
            return l->data;

    return NULL;
}


static gboolean
moo_plugin_registered (GType type)
{
    return moo_plugin_get (type) != NULL;
}


gpointer
moo_win_plugin_lookup (const char     *plugin_id,
                       MooEditWindow  *window)
{
    MooPlugin *plugin;

    g_return_val_if_fail (plugin_id != NULL, NULL);
    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), NULL);

    plugin = moo_plugin_lookup (plugin_id);
    return plugin ? window_get_plugin (window, plugin) : NULL;
}


gpointer
moo_doc_plugin_lookup (const char     *plugin_id,
                       MooEdit        *doc)
{
    MooPlugin *plugin;

    g_return_val_if_fail (plugin_id != NULL, NULL);
    g_return_val_if_fail (MOO_IS_EDIT (doc), NULL);

    plugin = moo_plugin_lookup (plugin_id);
    return plugin ? doc_get_plugin (doc, plugin) : NULL;
}


static gboolean
plugin_info_check (MooPluginInfo *info)
{
    return info && info->id && info->id[0] &&
            g_utf8_validate (info->id, -1, NULL) &&
            info->name && g_utf8_validate (info->name, -1, NULL) &&
            info->description && g_utf8_validate (info->description, -1, NULL) &&
            info->params && info->prefs_params;
}


static char*
make_prefs_key (MooPlugin      *plugin,
                const char     *key)
{
    g_return_val_if_fail (MOO_IS_PLUGIN (plugin), NULL);
    g_return_val_if_fail (key != NULL, NULL);

    return moo_prefs_make_key (MOO_PLUGIN_PREFS_ROOT,
                               moo_plugin_id (plugin),
                               PLUGIN_PREFS_ENABLED,
                               NULL);
}


static GQuark
make_id_quark (MooPlugin      *plugin)
{
    char *string;
    GQuark quark;

    g_return_val_if_fail (MOO_IS_PLUGIN (plugin), 0);

    string = g_strdup_printf ("MooPlugin::%s", moo_plugin_id (plugin));
    quark = g_quark_from_string (string);

    g_free (string);
    return quark;
}


gboolean
moo_plugin_initialized (MooPlugin *plugin)
{
    g_return_val_if_fail (MOO_IS_PLUGIN (plugin), FALSE);
    return plugin->initialized;
}


gboolean
moo_plugin_enabled (MooPlugin *plugin)
{
    g_return_val_if_fail (MOO_IS_PLUGIN (plugin), FALSE);
    return plugin->info->params->enabled;
}


static gboolean
plugin_enable (MooPlugin  *plugin)
{
    GSList *l, *windows;

    g_return_val_if_fail (MOO_IS_PLUGIN (plugin), FALSE);

    if (moo_plugin_enabled (plugin))
        return TRUE;

    g_assert (!plugin->initialized);

    plugin->info->params->enabled = TRUE;

    if (!plugin_init (plugin))
    {
        plugin->info->params->enabled = FALSE;
        return FALSE;
    }

    windows = moo_editor_list_windows (plugin_store->editor);
    for (l = windows; l != NULL; l = l->next)
        plugin_attach_win (plugin, l->data);
    g_slist_free (windows);

    return TRUE;
}


static void
plugin_disable (MooPlugin  *plugin)
{
    GSList *l, *windows;

    g_return_if_fail (MOO_IS_PLUGIN (plugin));

    if (!moo_plugin_enabled (plugin))
        return;

    g_assert (plugin->initialized);

    windows = moo_editor_list_windows (plugin_store->editor);
    for (l = windows; l != NULL; l = l->next)
        plugin_detach_win (plugin, l->data);
    g_slist_free (windows);

    plugin_deinit (plugin);
    plugin->info->params->enabled = FALSE;
}


gboolean
moo_plugin_set_enabled (MooPlugin  *plugin,
                        gboolean    enabled)
{
    g_return_val_if_fail (MOO_IS_PLUGIN (plugin), FALSE);

    if (enabled)
    {
        return plugin_enable (plugin);
    }
    else
    {
        plugin_disable (plugin);
        return TRUE;
    }
}


void
moo_plugin_unregister (GType type)
{
    MooPlugin *plugin = moo_plugin_get (type);
    g_return_if_fail (plugin != NULL);
    moo_plugin_set_enabled (plugin, FALSE);
    plugin_store_remove (plugin);
    g_object_unref (plugin);
}


gpointer
moo_plugin_lookup (const char *plugin_id)
{
    g_return_val_if_fail (plugin_id != NULL, NULL);
    plugin_store_init ();
    return g_hash_table_lookup (plugin_store->names, plugin_id);
}


GSList*
moo_list_plugins (void)
{
    plugin_store_init ();
    return g_slist_copy (plugin_store->list);
}


#define DEFINE_GETTER(what)                                 \
const char*                                                 \
moo_plugin_##what (MooPlugin *plugin)                       \
{                                                           \
    g_return_val_if_fail (MOO_IS_PLUGIN (plugin), NULL);    \
    return plugin->info->what;                              \
}

DEFINE_GETTER(id)
DEFINE_GETTER(name)
DEFINE_GETTER(description)
DEFINE_GETTER(version)
DEFINE_GETTER(author)

#undef DEFINE_GETTER


static gboolean
moo_plugin_read_module (GModule     *module,
                        const char  *name)
{
    MooPluginModuleInitFunc init_func;
    char *init_func_name;
    gboolean result = FALSE;

    g_return_val_if_fail (module != NULL, FALSE);
    g_return_val_if_fail (name != NULL, FALSE);

    if (!g_module_symbol (module, MOO_PLUGIN_INIT_FUNC_NAME,
                          (gpointer*) &init_func))
        goto out;

    if (!init_func ())
        goto out;

    result = TRUE;

out:
    g_free (init_func_name);
    return result;
}


static GModule *
module_open (const char *path)
{
    GModule *module;
    G_GNUC_UNUSED guint saved;

#ifdef __WIN32__
    saved = SetErrorMode (SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
#endif

    module = g_module_open (path, 0);

#ifdef __WIN32__
    SetErrorMode (saved);
#endif

    return module;
}

static void
moo_plugin_read_dir (const char *path)
{
    GDir *dir;
    const char *name;

    g_return_if_fail (path != NULL);

    dir = g_dir_open (path, 0, NULL);

    if (!dir)
        return;

    while ((name = g_dir_read_name (dir)))
    {
        char *module_path, *prefix, *suffix;
        GModule *module;

        if (!g_str_has_suffix (name, "." G_MODULE_SUFFIX))
            continue;

        suffix = g_strrstr (name, "." G_MODULE_SUFFIX);
        prefix = g_strndup (name, suffix - name);

        module_path = g_build_filename (path, name, NULL);
        module = module_open (module_path);

        if (module)
        {
            if (!moo_plugin_read_module (module, prefix))
                g_module_close (module);
        }
        else
        {
            g_message ("%s: %s", G_STRLOC, g_module_error ());
        }

        g_free (prefix);
        g_free (module_path);
    }

    g_dir_close (dir);
}


char **
moo_plugin_get_dirs (void)
{
    plugin_store_init ();
    return g_strdupv (plugin_store->dirs);
}


static void
moo_plugin_init_builtin (void)
{
#ifndef __WIN32__
    _moo_find_plugin_init ();
#endif
#if GTK_CHECK_VERSION(2,6,0)
    _moo_file_selector_plugin_init ();
#endif
    _moo_active_strings_plugin_init ();
    _moo_completion_plugin_init ();
#ifdef MOO_USE_PYGTK
    _moo_python_plugin_init ();
#endif
}


void
moo_plugin_read_dirs (void)
{
    char **d, **dirs;
    guint n_dirs;

    plugin_store_init ();

    if (plugin_store->dirs_read)
        return;

    plugin_store->dirs_read = TRUE;

    dirs = moo_get_data_subdirs (MOO_PLUGIN_DIR_BASENAME,
                                 MOO_DATA_LIB, &n_dirs);
    g_strfreev (plugin_store->dirs);
    plugin_store->dirs = dirs;

    moo_plugin_init_builtin ();

    for (d = plugin_store->dirs; d && *d; ++d)
        moo_plugin_read_dir (*d);
}


void
_moo_window_attach_plugins (MooEditWindow *window)
{
    GSList *l;

    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));

    plugin_store_init ();

    for (l = plugin_store->list; l != NULL; l = l->next)
        plugin_attach_win (l->data, window);
}


void
_moo_window_detach_plugins (MooEditWindow *window)
{
    GSList *l;

    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));

    plugin_store_init ();

    for (l = plugin_store->list; l != NULL; l = l->next)
        plugin_detach_win (l->data, window);
}


static void
doc_lang_changed (MooEdit *doc)
{
    GSList *l;
    MooLang *lang;
    const char *id;
    MooEditWindow *window = NULL;

    g_return_if_fail (MOO_IS_EDIT (doc));

    window = moo_edit_get_window (doc);
    lang = moo_text_view_get_lang (MOO_TEXT_VIEW (doc));
    id = LANG_ID (lang);

    for (l = plugin_store->list; l != NULL; l = l->next)
    {
        MooPlugin *plugin = l->data;

        if (moo_plugin_enabled (plugin) && plugin->langs)
        {
            if (g_hash_table_lookup (plugin->langs, id))
                plugin_attach_doc (plugin, window, doc);
            else
                plugin_detach_doc (plugin, window, doc);
        }
    }
}


void
_moo_doc_attach_plugins (MooEditWindow *window,
                         MooEdit       *doc)
{
    GSList *l;

    g_return_if_fail (!window || MOO_IS_EDIT_WINDOW (window));
    g_return_if_fail (MOO_IS_EDIT (doc));

    plugin_store_init ();

    g_signal_connect (doc, "config-notify::lang",
                      G_CALLBACK (doc_lang_changed),
                      plugin_store);

    for (l = plugin_store->list; l != NULL; l = l->next)
        plugin_attach_doc (l->data, window, doc);
}


void
_moo_doc_detach_plugins (MooEditWindow *window,
                         MooEdit       *doc)
{
    GSList *l;

    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));
    g_return_if_fail (MOO_IS_EDIT (doc));

    plugin_store_init ();

    g_signal_handlers_disconnect_by_func (doc,
                                          (gpointer) doc_lang_changed,
                                          plugin_store);

    for (l = plugin_store->list; l != NULL; l = l->next)
        plugin_detach_doc (l->data, window, doc);
}


static gboolean
moo_plugin_visible (MooPlugin *plugin)
{
    g_return_val_if_fail (MOO_IS_PLUGIN (plugin), FALSE);
    return plugin->info->params->visible ? TRUE : FALSE;
}


/***************************************************************************/
/* Preferences dialog
 */

enum {
    COLUMN_ENABLED,
    COLUMN_PLUGIN_ID,
    COLUMN_PLUGIN_NAME,
    N_COLUMNS
};


static void
selection_changed (GtkTreeSelection   *selection,
                   MooPrefsDialogPage *page)
{
    MooPlugin *plugin = NULL;
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkWidget *info;
    GtkLabel *version, *author, *description;

    if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
        char *id = NULL;
        gtk_tree_model_get (model, &iter, COLUMN_PLUGIN_ID, &id, -1);
        plugin = moo_plugin_lookup (id);
        g_free (id);
    }

    info = moo_glade_xml_get_widget (page->xml, "info");
    author = moo_glade_xml_get_widget (page->xml, "author");
    version = moo_glade_xml_get_widget (page->xml, "version");
    description = moo_glade_xml_get_widget (page->xml, "description");

    gtk_widget_set_sensitive (info, plugin != NULL);

    if (plugin)
    {
        gtk_label_set_text (author, moo_plugin_author (plugin));
        gtk_label_set_text (version, moo_plugin_version (plugin));
        gtk_label_set_text (description, moo_plugin_description (plugin));
    }
    else
    {
        gtk_label_set_text (author, "");
        gtk_label_set_text (version, "");
        gtk_label_set_text (description, "");
    }
}


static int
cmp_page_and_id (GObject    *page,
                 const char *id)
{
    const char *page_id = g_object_get_data (page, "moo-plugin-id");
    return page_id ? strcmp (id, page_id) : 1;
}

static void
sync_pages (MooPrefsDialog *dialog)
{
    GSList *old_plugin_pages, *plugin_pages, *plugin_ids, *l, *plugins;

    plugins = moo_list_plugins ();
    plugin_ids = NULL;

    for (l = plugins; l != NULL; l = l->next)
    {
        MooPlugin *plugin = l->data;
        plugin_ids = g_slist_append (plugin_ids, g_strdup (moo_plugin_id (plugin)));
    }

    old_plugin_pages = g_object_get_data (G_OBJECT (dialog), "moo-plugin-prefs-pages");
    plugin_pages = NULL;

    for (l = plugins; l != NULL; l = l->next)
    {
        MooPlugin *plugin = l->data;

        if (moo_plugin_enabled (plugin) &&
            MOO_PLUGIN_GET_CLASS(plugin)->create_prefs_page)
        {
            GSList *link = g_slist_find_custom (old_plugin_pages,
                                                moo_plugin_id (l->data),
                                                (GCompareFunc) cmp_page_and_id);

            if (link)
            {
                plugin_pages = g_slist_append (plugin_pages, link->data);
            }
            else
            {
                GtkWidget *plugin_page = MOO_PLUGIN_GET_CLASS(plugin)->create_prefs_page (plugin);

                if (plugin_page)
                {
                    if (!MOO_IS_PREFS_DIALOG_PAGE (plugin_page))
                    {
                        g_critical ("%s: oops", G_STRLOC);
                    }
                    else
                    {
                        MOO_PREFS_DIALOG_PAGE (plugin_page)->auto_apply = FALSE;

                        g_object_set_data_full (G_OBJECT (plugin_page), "moo-plugin-id",
                                                g_strdup (moo_plugin_id (plugin)),
                                                g_free);

                        plugin_pages = g_slist_append (plugin_pages, plugin_page);
                        moo_prefs_dialog_insert_page (dialog, plugin_page, -1);
                    }
                }
            }
        }
    }

    for (l = old_plugin_pages; l != NULL; l = l->next)
        if (!g_slist_find (plugin_pages, l->data))
            moo_prefs_dialog_remove_page (dialog, l->data);

    g_object_set_data_full (G_OBJECT (dialog), "moo-plugin-prefs-pages",
                            plugin_pages, (GDestroyNotify) g_slist_free);

    g_slist_foreach (plugin_ids, (GFunc) g_free, NULL);
    g_slist_free (plugin_ids);
    g_slist_free (plugins);
}


static void
prefs_init (MooPrefsDialog      *dialog,
            MooPrefsDialogPage  *page)
{
    GtkTreeView *treeview;
    GtkListStore *store;
    GtkTreeModel *model;
    GSList *l, *plugins;

    treeview = moo_glade_xml_get_widget (page->xml, "treeview");
    model = gtk_tree_view_get_model (treeview);
    store = GTK_LIST_STORE (model);

    gtk_list_store_clear (store);
    plugins = moo_list_plugins ();

    for (l = plugins; l != NULL; l = l->next)
    {
        GtkTreeIter iter;
        MooPlugin *plugin = l->data;

        if (moo_plugin_visible (plugin))
        {
            gtk_list_store_append (store, &iter);
            gtk_list_store_set (store, &iter,
                                COLUMN_ENABLED, moo_plugin_enabled (plugin),
                                COLUMN_PLUGIN_ID, moo_plugin_id (plugin),
                                COLUMN_PLUGIN_NAME, moo_plugin_name (plugin),
                                -1);
        }
    }

    selection_changed (gtk_tree_view_get_selection (treeview), page);

    g_slist_free (plugins);
    sync_pages (dialog);
}


static void
prefs_apply (MooPrefsDialog      *dialog,
             MooPrefsDialogPage  *page)
{
    GtkTreeView *treeview;
    GtkTreeModel *model;
    GtkTreeIter iter;
    GSList *plugin_pages;

    treeview = moo_glade_xml_get_widget (page->xml, "treeview");
    model = gtk_tree_view_get_model (treeview);

    if (gtk_tree_model_get_iter_first (model, &iter)) do
    {
        MooPlugin *plugin;
        gboolean enabled;
        char *id = NULL;
        char *prefs_key;

        gtk_tree_model_get (model, &iter,
                            COLUMN_ENABLED, &enabled,
                            COLUMN_PLUGIN_ID, &id,
                            -1);

        g_return_if_fail (id != NULL);
        plugin = moo_plugin_lookup (id);
        g_return_if_fail (plugin != NULL);

        moo_plugin_set_enabled (plugin, enabled);

        prefs_key = make_prefs_key (plugin, PLUGIN_PREFS_ENABLED);
        moo_prefs_set_bool (prefs_key, enabled);

        g_free (prefs_key);
        g_free (id);
    }
    while (gtk_tree_model_iter_next (model, &iter));

    sync_pages (dialog);

    plugin_pages = g_object_get_data (G_OBJECT (dialog),
                                      "moo-plugin-prefs-pages");

    while (plugin_pages)
    {
        g_signal_emit_by_name (plugin_pages->data, "apply");
        plugin_pages = plugin_pages->next;
    }
}


static void
enable_toggled (GtkCellRendererToggle *cell,
                gchar                 *tree_path,
                GtkListStore          *store)
{
    GtkTreePath *path = gtk_tree_path_new_from_string (tree_path);
    GtkTreeIter iter;
    if (gtk_tree_model_get_iter (GTK_TREE_MODEL (store), &iter, path))
        gtk_list_store_set (store, &iter,
                            COLUMN_ENABLED,
                            !gtk_cell_renderer_toggle_get_active (cell),
                            -1);
    gtk_tree_path_free (path);
}


void
_moo_plugin_attach_prefs (GtkWidget *dialog)
{
    GtkWidget *page;
    MooGladeXML *xml;
    GtkTreeView *treeview;
    GtkCellRenderer *cell;
    GtkListStore *store;
    GtkTreeSelection *selection;

    g_return_if_fail (MOO_IS_PREFS_DIALOG (dialog));

    page = moo_prefs_dialog_page_new_from_xml ("Plugins", MOO_STOCK_PLUGINS,
                                               NULL, MOO_PLUGIN_PREFS_GLADE_UI,
                                               -1, "page", MOO_PLUGIN_PREFS_ROOT);
    g_return_if_fail (page != NULL);

    xml = MOO_PREFS_DIALOG_PAGE(page)->xml;

    treeview = moo_glade_xml_get_widget (xml, "treeview");

    selection = gtk_tree_view_get_selection (treeview);
    gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
    g_signal_connect (selection, "changed", G_CALLBACK (selection_changed), page);

    store = gtk_list_store_new (N_COLUMNS,
                                G_TYPE_BOOLEAN,
                                G_TYPE_STRING,
                                G_TYPE_STRING);
    gtk_tree_view_set_model (treeview, GTK_TREE_MODEL (store));
    g_object_unref (store);

    cell = gtk_cell_renderer_toggle_new ();
    g_object_set (cell, "activatable", TRUE, NULL);
    g_signal_connect (cell, "toggled", G_CALLBACK (enable_toggled), store);
    gtk_tree_view_insert_column_with_attributes (treeview, 0, "Enabled", cell,
                                                 "active", COLUMN_ENABLED, NULL);

    cell = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes (treeview, 1, "Plugin", cell,
                                                 "text", COLUMN_PLUGIN_NAME, NULL);

    g_signal_connect (dialog, "init", G_CALLBACK (prefs_init), page);
    g_signal_connect (dialog, "apply", G_CALLBACK (prefs_apply), page);

    moo_prefs_dialog_append_page (MOO_PREFS_DIALOG (dialog), page);
}
