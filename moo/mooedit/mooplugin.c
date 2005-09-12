/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mooplugin.c
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

#include "mooedit/mooplugin.h"
#include <string.h>
#include <gmodule.h>

#define PLUGIN_PREFS_ENABLED "enabled"


typedef struct {
    MooPlugin *plugin;
    MooWindowPlugin *wplugin;
} PluginWindowData;

typedef struct {
    GSList *plugins; /* PluginWindowData* */
    MooEditWindow *window;
} WindowInfo;

typedef struct {
    GSList *plugins;   /* MooPlugin* */
    GSList *windows;         /* WindowInfo* */
} PluginStore;

static PluginStore *plugin_store = NULL;
static void         plugin_store_init   (void);

static void         moo_plugin_class_init (MooPluginClass *klass);
static void         moo_window_plugin_class_init (MooWindowPluginClass *klass);

static gboolean     moo_window_plugin_create_default (MooWindowPlugin *wplugin);

static WindowInfo  *window_info_new     (MooEditWindow  *window);
static void         window_info_free    (WindowInfo     *window_info);
static WindowInfo  *window_info_lookup  (MooEditWindow  *window);

static gboolean     init_plugin         (MooPlugin      *plugin);
static void         deinit_plugin       (MooPlugin      *plugin);
static void         attach_plugin       (WindowInfo     *window_info,
                                         MooPlugin      *plugin);
static void         detach_plugin       (WindowInfo     *window_info,
                                         MooPlugin      *plugin);

static gboolean     check_info          (MooPluginInfo  *info);

static char        *make_prefs_key      (MooPlugin      *plugin,
                                         const char     *key);


static gpointer parent_class = NULL;

enum {
    WPROP_0,
    WPROP_PLUGIN
};

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
moo_window_plugin_get_type (void)
{
    static GType type = 0;

    if (G_UNLIKELY (type == 0))
    {
        static const GTypeInfo info = {
            sizeof (MooWindowPluginClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) moo_window_plugin_class_init,
            (GClassFinalizeFunc) NULL,
            NULL,   /* class_data */
            sizeof (MooWindowPlugin),
            0,      /* n_preallocs */
            NULL,
            NULL    /* value_table */
        };

        type = g_type_register_static (G_TYPE_OBJECT, "MooWindowPlugin", &info, 0);
    }

    return type;
}


static void
moo_plugin_class_init (MooPluginClass *klass)
{
    parent_class = g_type_class_peek_parent (klass);
}


static void
moo_window_plugin_class_init (MooWindowPluginClass *klass)
{
    parent_class = g_type_class_peek_parent (klass);
    klass->create = moo_window_plugin_create_default;
}


static gboolean
moo_window_plugin_create_default (G_GNUC_UNUSED MooWindowPlugin *wplugin)
{
    g_return_val_if_reached (FALSE);
}


gboolean
moo_plugin_register (GType type)
{
    MooPluginClass *klass;
    MooPlugin *plugin;
    char *prefs_key;
    GSList *l;

    g_return_val_if_fail (g_type_is_a (type, MOO_TYPE_PLUGIN), FALSE);

    klass = g_type_class_ref (type);
    g_return_val_if_fail (klass != NULL, FALSE);

    if (klass->plugin_system_version != MOO_PLUGIN_CURRENT_VERSION)
    {
        g_warning ("%s: plugin %s of version %d is incompatible with "
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
    g_return_val_if_fail (plugin != NULL, FALSE);

    if (!check_info (plugin->info))
    {
        g_warning ("%s: invalid info in plugin %s",
                   G_STRLOC, g_type_name (type));
        return FALSE;
    }

    prefs_key = make_prefs_key (plugin, PLUGIN_PREFS_ENABLED);
    moo_prefs_new_key_bool (prefs_key, moo_plugin_enabled (plugin));
    plugin->info->params->enabled = moo_prefs_get_bool (prefs_key);
    g_free (prefs_key);

    if (!init_plugin (plugin))
    {
        g_object_unref (plugin);
        return FALSE;
    }

    plugin_store->plugins = g_slist_append (plugin_store->plugins, plugin);

    for (l = plugin_store->windows; l != NULL; l = l->next)
    {
        WindowInfo *window_info = l->data;
        attach_plugin (window_info, plugin);
    }

    return TRUE;
}


static gboolean
init_plugin (MooPlugin *plugin)
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
deinit_plugin (MooPlugin *plugin)
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


static PluginWindowData*
plugin_window_data_new (WindowInfo *window_info,
                        MooPlugin  *plugin)
{
    PluginWindowData *window_data;

    g_return_val_if_fail (window_info != NULL, NULL);
    g_return_val_if_fail (MOO_IS_PLUGIN (plugin), NULL);

    window_data = g_new0 (PluginWindowData, 1);
    window_data->plugin = plugin;

    return window_data;
}


static void
plugin_window_data_free (PluginWindowData *window_data)
{
    g_free (window_data);
}


static void
attach_plugin (WindowInfo     *window_info,
               MooPlugin      *plugin)
{
    PluginWindowData *window_data;
    MooPluginClass *klass;
    MooWindowPluginClass *wklass;
    MooWindowPlugin *wplugin;
    GType wtype;

    g_return_if_fail (window_info != NULL);
    g_return_if_fail (MOO_IS_PLUGIN (plugin));

    if (!moo_plugin_enabled (plugin))
        return;

    window_data = plugin_window_data_new (window_info, plugin);
    window_info->plugins = g_slist_append (window_info->plugins, window_data);

    klass = MOO_PLUGIN_GET_CLASS (plugin);

    if (klass->attach)
        klass->attach (plugin, window_info->window);

    wtype = plugin->window_plugin_type;

    if (wtype == G_TYPE_NONE)
        return;

    g_return_if_fail (g_type_is_a (wtype, MOO_TYPE_WINDOW_PLUGIN));

    wplugin = g_object_new (wtype, NULL);
    g_return_if_fail (wplugin != NULL);

    wplugin->plugin = plugin;
    wplugin->window = window_info->window;
    wklass = MOO_WINDOW_PLUGIN_GET_CLASS (wplugin);

    if (wklass->create (wplugin))
        window_data->wplugin = wplugin;
    else
        g_object_unref (wplugin);
}


static PluginWindowData*
window_info_lookup_plugin (WindowInfo *window_info,
                           MooPlugin  *plugin)
{
    GSList *l;

    g_return_val_if_fail (window_info != NULL, NULL);
    g_return_val_if_fail (MOO_IS_PLUGIN (plugin), NULL);

    for (l = window_info->plugins; l != NULL; l = l->next)
    {
        PluginWindowData *window_data = l->data;
        if (window_data->plugin == plugin)
            return window_data;
    }

    return NULL;
}


static void
detach_plugin (WindowInfo   *window_info,
               MooPlugin    *plugin)
{
    PluginWindowData *window_data;
    MooPluginClass *klass;
    MooWindowPluginClass *wklass;

    g_return_if_fail (window_info != NULL);
    g_return_if_fail (MOO_IS_PLUGIN (plugin));

    window_data = window_info_lookup_plugin (window_info, plugin);

    if (!window_data)
        return;

    if (window_data->wplugin)
    {
        wklass = MOO_WINDOW_PLUGIN_GET_CLASS (window_data->wplugin);
        if (wklass->destroy)
            wklass->destroy (window_data->wplugin);
        g_object_unref (window_data->wplugin);
        window_data->wplugin = NULL;
    }

    klass = MOO_PLUGIN_GET_CLASS (plugin);

    if (klass->detach)
        klass->detach (plugin, window_info->window);

    window_info->plugins = g_slist_remove (window_info->plugins, window_data);
    plugin_window_data_free (window_data);
}


static void
plugin_store_init (void)
{
    if (!plugin_store)
    {
        static PluginStore store;

        store.plugins = NULL;
        store.windows = NULL;

        plugin_store = &store;
    }
}


MooPlugin*
moo_plugin_get (GType type)
{
    GSList *l;

    g_return_val_if_fail (g_type_is_a (type, MOO_TYPE_PLUGIN), NULL);

    plugin_store_init ();

    for (l = plugin_store->plugins; l != NULL; l = l->next)
    {
        MooPlugin *plugin = l->data;
        if (G_OBJECT_TYPE (plugin) == type)
            return plugin;
    }

    return NULL;
}


gboolean
moo_plugin_registered (GType type)
{
    return moo_plugin_get (type) != NULL;
}


static WindowInfo*
window_info_new (MooEditWindow *window)
{
    WindowInfo *window_info;

    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), NULL);

    window_info = g_new0 (WindowInfo, 1);

    window_info->window = window;
    window_info->plugins = NULL;

    return window_info;
}


static void
window_info_free (WindowInfo *window_info)
{
    if (window_info)
    {
        g_assert (window_info->plugins == NULL);
        g_free (window_info);
    }
}


static WindowInfo*
window_info_lookup (MooEditWindow *window)
{
    GSList *l;

    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), NULL);

    plugin_store_init ();

    for (l = plugin_store->windows; l != NULL; l = l->next)
    {
        WindowInfo *window_info = l->data;
        if (window_info->window == window)
            return window_info;
    }

    return NULL;
}


gpointer
moo_window_plugin_lookup (const char     *plugin_id,
                          MooEditWindow  *window)
{
    PluginWindowData *window_data;
    WindowInfo *window_info = window_info_lookup (window);
    MooPlugin *plugin = moo_plugin_lookup (plugin_id);
    MooWindowPlugin *wplugin = NULL;

    g_return_val_if_fail (window_info != NULL && plugin != NULL, NULL);

    window_data = window_info_lookup_plugin (window_info, plugin);

    wplugin = window_data ? window_data->wplugin : NULL;
    return wplugin;
}


static gboolean
check_info (MooPluginInfo *info)
{
    return info->id && info->id[0] &&
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
                               moo_plugin_get_id (plugin),
                               PLUGIN_PREFS_ENABLED,
                               NULL);
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


gboolean
moo_plugin_set_enabled (MooPlugin  *plugin,
                        gboolean    enabled)
{
    GSList *l;

    g_return_val_if_fail (MOO_IS_PLUGIN (plugin), FALSE);

    if (moo_plugin_enabled (plugin))
    {
        if (enabled)
            return TRUE;

        g_assert (plugin->initialized);

        for (l = plugin_store->windows; l != NULL; l = l->next)
        {
            WindowInfo *window_info = l->data;
            detach_plugin (window_info, plugin);
        }

        deinit_plugin (plugin);
        plugin->info->params->enabled = FALSE;

        return TRUE;
    }
    else
    {
        if (!enabled)
            return TRUE;

        g_assert (!plugin->initialized);

        plugin->info->params->enabled = TRUE;

        if (!init_plugin (plugin))
        {
            plugin->info->params->enabled = FALSE;
            return FALSE;
        }

        for (l = plugin_store->windows; l != NULL; l = l->next)
        {
            WindowInfo *window_info = l->data;
            attach_plugin (window_info, plugin);
        }

        return TRUE;
    }
}


gpointer
moo_plugin_lookup (const char *plugin_id)
{
    GSList *l;

    g_return_val_if_fail (plugin_id != NULL, NULL);

    plugin_store_init ();

    for (l = plugin_store->plugins; l != NULL; l = l->next)
    {
        MooPlugin *plugin = l->data;
        if (!strcmp (plugin_id, moo_plugin_get_id (plugin)))
            return plugin;
    }

    return NULL;
}


GSList*
moo_list_plugins (void)
{
    plugin_store_init ();
    return g_slist_copy (plugin_store->plugins);
}


const char*
moo_plugin_get_id (MooPlugin *plugin)
{
    g_return_val_if_fail (MOO_IS_PLUGIN (plugin), NULL);
    return plugin->info->id;
}


static gboolean
moo_plugin_read_module (GModule     *module,
                        const char  *name)
{
    MooPluginModuleInitFunc init_func;
    char *init_func_name;
    gboolean result = FALSE;

    g_return_val_if_fail (module != NULL, FALSE);
    g_return_val_if_fail (name != NULL, FALSE);

    init_func_name = g_strdup_printf ("%s_init", name);

    if (!g_module_symbol (module, init_func_name, (gpointer*) &init_func))
        goto out;

    if (!init_func ())
        goto out;

    result = TRUE;

out:
    g_free (init_func_name);
    return result;
}


void
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
        module = g_module_open (module_path, 0);

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


void
_moo_window_attach_plugins (MooEditWindow *window)
{
    GSList *l;
    WindowInfo *window_info;

    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));

    plugin_store_init ();

    window_info = window_info_new (window);
    plugin_store->windows = g_slist_append (plugin_store->windows, window_info);

    for (l = plugin_store->plugins; l != NULL; l = l->next)
    {
        MooPlugin *plugin = l->data;
        attach_plugin (window_info, plugin);
    }
}


void
_moo_plugin_detach_plugins (MooEditWindow *window)
{
    WindowInfo *window_info;
    GSList *plugins, *l;

    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));

    window_info = window_info_lookup (window);
    g_return_if_fail (window_info != NULL);

    plugins = g_slist_copy (window_info->plugins);
    plugins = g_slist_reverse (plugins);

    for (l = plugins; l != NULL; l = l->next)
    {
        PluginWindowData *window_data = l->data;
        detach_plugin (window_info, window_data->plugin);
    }

    plugin_store->windows = g_slist_remove (plugin_store->windows, window_info);
    window_info_free (window_info);

    g_slist_free (plugins);
}
