/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *   mooeditplugin.c
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

#define MOOEDIT_COMPILATION
#include "mooeditplugin.h"
#include <string.h>


#define PLUGIN_PREFS_ENABLED "enabled"


static GSList *registered_plugins = NULL;   /* Plugin* */
static GSList *edit_windows = NULL;         /* WindowInfo* */


typedef struct {
    MooEditPluginInfo info;
    gpointer data;
    gboolean initialized;
    GModule *module;
} Plugin;

typedef struct {
    Plugin *plugin;
    MooEditPluginWindowData window_data;
    GtkWidget *pane_widget;
} PluginFullWindowData;

typedef struct {
    GSList *plugins; /* PluginFullWindowData* */
    MooEditWindow *window;
} WindowInfo;


static WindowInfo  *window_info_new         (MooEditWindow      *window);
static void         window_info_free        (WindowInfo         *window_info);
static WindowInfo  *window_info_lookup      (MooEditWindow      *window);

static Plugin      *plugin_new              (MooEditPluginInfo  *info,
                                             gpointer            plugin_data);
static void         plugin_free             (Plugin             *plugin);

static Plugin      *plugin_lookup           (const char         *id);
static const char  *plugin_id               (Plugin             *plugin);

static gboolean     plugin_enabled          (Plugin             *plugin);
static gboolean     plugin_init             (Plugin             *plugin);
static void         plugin_deinit           (Plugin             *plugin);

static void         moo_edit_plugin_attach  (WindowInfo         *window_info,
                                             Plugin             *plugin);
static void         moo_edit_plugin_detach  (WindowInfo         *window_info,
                                             Plugin             *plugin);

static const char  *make_prefs_key          (Plugin             *plugin,
                                             const char         *key);


gboolean
moo_edit_plugin_register (MooEditPluginInfo  *info,
                          gpointer            plugin_data)
{
    Plugin *plugin;
    GSList *l;
    const char *prefs_key;

    g_return_val_if_fail (info != NULL, FALSE);

    if (info->plugin_system_version != MOO_EDIT_PLUGIN_CURRENT_VERSION)
    {
        g_warning ("%s: plugin of version %d is incompatible with "
                   "current version %d", G_STRLOC, info->plugin_system_version,
                   MOO_EDIT_PLUGIN_CURRENT_VERSION);
        return FALSE;
    }

    g_return_val_if_fail (info->id && info->id[0], FALSE);
    g_return_val_if_fail (g_utf8_validate (info->id, -1, NULL), FALSE);
    g_return_val_if_fail (!info->name || g_utf8_validate (info->name, -1, NULL), FALSE);
    g_return_val_if_fail (!info->description || g_utf8_validate (info->description, -1, NULL), FALSE);
    g_return_val_if_fail (info->init || info->attach || info->pane_create, FALSE);
    g_return_val_if_fail (info->params != NULL, FALSE);
    g_return_val_if_fail (info->prefs_params != NULL, FALSE);
    g_return_val_if_fail (!info->params->want_pane || info->pane_create, FALSE);

    plugin = plugin_new (info, plugin_data);
    g_return_val_if_fail (plugin != NULL, FALSE);

    if (moo_edit_plugin_lookup (plugin_id (plugin)))
    {
        g_warning ("plugin with id '%s' already registered", plugin_id (plugin));
        moo_edit_plugin_unregister (plugin_id (plugin));
    }

    prefs_key = make_prefs_key (plugin, PLUGIN_PREFS_ENABLED);
    moo_prefs_new_key_bool (prefs_key, plugin_enabled (plugin));
    plugin->info.params->enabled = moo_prefs_get_bool (prefs_key);

    if (!plugin_init (plugin))
    {
        plugin_free (plugin);
        return FALSE;
    }

    registered_plugins = g_slist_append (registered_plugins, plugin);

    for (l = edit_windows; l != NULL; l = l->next)
    {
        WindowInfo *window_info = l->data;
        moo_edit_plugin_attach (window_info, plugin);
    }

    return TRUE;
}


void
moo_edit_plugin_unregister (const char *id)
{
    Plugin *plugin;
    GSList *l;

    g_return_if_fail (id != NULL);

    plugin = plugin_lookup (id);
    g_return_if_fail (plugin != NULL);

    registered_plugins = g_slist_remove (registered_plugins, plugin);

    for (l = edit_windows; l != NULL; l = l->next)
    {
        WindowInfo *window_info = l->data;
        moo_edit_plugin_detach (window_info, plugin);
    }

    plugin_deinit (plugin);
    plugin_free (plugin);
}


MooEditWindow*
_moo_edit_window_new (MooEditor *editor)
{
    GSList *l;
    MooEditWindow *window;
    WindowInfo *window_info;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);

    window = g_object_new (MOO_TYPE_EDIT_WINDOW, "editor", editor, NULL);

    window_info = window_info_new (window);
    edit_windows = g_slist_append (edit_windows, window_info);

    for (l = registered_plugins; l != NULL; l = l->next)
    {
        Plugin *plugin = l->data;
        moo_edit_plugin_attach (window_info, plugin);
    }

    gtk_widget_show (GTK_WIDGET (window));
    return window;
}


void
_moo_edit_window_close (MooEditWindow *window)
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
        PluginFullWindowData *window_data = l->data;
        moo_edit_plugin_detach (window_info, window_data->plugin);
    }

    edit_windows = g_slist_remove (edit_windows, window_info);
    window_info_free (window_info);

    gtk_widget_destroy (GTK_WIDGET (window));
    g_slist_free (plugins);
}


static PluginFullWindowData*
plugin_full_window_data_new (WindowInfo         *window_info,
                             Plugin             *plugin)
{
    PluginFullWindowData *full_window_data;

    g_return_val_if_fail (window_info != NULL && plugin != NULL, NULL);

    full_window_data = g_new0 (PluginFullWindowData, 1);

    full_window_data->plugin = plugin;
    full_window_data->pane_widget = NULL;

    full_window_data->window_data.window = window_info->window;
    full_window_data->window_data.data = NULL;

    return full_window_data;
}


static void
plugin_full_window_data_free (PluginFullWindowData *full_window_data)
{
    if (full_window_data)
    {
        g_assert (full_window_data->pane_widget == NULL);
        g_free (full_window_data);
    }
}


static PluginFullWindowData*
window_info_lookup_plugin (WindowInfo *window_info,
                           Plugin     *plugin)
{
    GSList *l;

    g_return_val_if_fail (window_info != NULL && plugin != NULL, NULL);

    for (l = window_info->plugins; l != NULL; l = l->next)
    {
        PluginFullWindowData *full_window_data = l->data;
        if (full_window_data->plugin == plugin)
            return full_window_data;
    }

    return NULL;
}


static void
moo_edit_plugin_attach (WindowInfo         *window_info,
                        Plugin             *plugin)
{
    PluginFullWindowData *full_window_data;
    GtkWidget *widget = NULL;
    MooPaneLabel *label = NULL;

    g_return_if_fail (window_info != NULL);
    g_return_if_fail (plugin != NULL);

    if (!plugin_enabled (plugin))
        return;

    full_window_data = plugin_full_window_data_new (window_info, plugin);
    window_info->plugins = g_slist_append (window_info->plugins, full_window_data);

    if (plugin->info.attach)
        plugin->info.attach (&plugin->info, &full_window_data->window_data, plugin->data);

    if (plugin->info.params->want_pane &&
        plugin->info.pane_create (&plugin->info, &full_window_data->window_data,
                                  &label, &widget, plugin->data))
    {
        if (moo_big_paned_insert_pane (window_info->window->paned, widget, label,
                                       plugin->info.params->pane_position, -1) >= 0)
            full_window_data->pane_widget = widget;

        gtk_widget_unref (widget);
        moo_pane_label_free (label);
    }
}


static void
moo_edit_plugin_detach (WindowInfo         *window_info,
                        Plugin             *plugin)
{
    PluginFullWindowData *full_window_data;

    full_window_data = window_info_lookup_plugin (window_info, plugin);

    if (!full_window_data)
        return;

    if (full_window_data->pane_widget)
    {
        if (plugin->info.pane_destroy)
            plugin->info.pane_destroy (&plugin->info,
                                       &full_window_data->window_data,
                                       plugin->data);
        moo_big_paned_remove_pane (window_info->window->paned,
                                   full_window_data->pane_widget);
        full_window_data->pane_widget = NULL;
    }

    if (plugin->info.detach)
        plugin->info.detach (&plugin->info,
                             &full_window_data->window_data,
                             plugin->data);

    window_info->plugins = g_slist_remove (window_info->plugins, full_window_data);
    plugin_full_window_data_free (full_window_data);
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

    for (l = edit_windows; l != NULL; l = l->next)
    {
        WindowInfo *window_info = l->data;
        if (window_info->window == window)
            return window_info;
    }

    return NULL;
}


static MooEditPluginParams*
plugin_params_copy (MooEditPluginParams *params)
{
    MooEditPluginParams *copy;

    g_return_val_if_fail (params != NULL, NULL);

    copy = g_new0 (MooEditPluginParams, 1);
    copy->enabled = params->enabled;
    copy->want_pane = params->want_pane;
    copy->pane_position = params->pane_position;

    return copy;
}


static void
plugin_params_free (MooEditPluginParams *params)
{
    if (params)
        g_free (params);
}


static MooEditPluginPrefsParams*
plugin_prefs_params_copy (MooEditPluginPrefsParams *params)
{
    MooEditPluginPrefsParams *copy;

    g_return_val_if_fail (params != NULL, NULL);

    copy = g_new0 (MooEditPluginPrefsParams, 1);
    copy->prefs_page_create = params->prefs_page_create;

    return copy;
}


static void
plugin_prefs_params_free (MooEditPluginPrefsParams *params)
{
    if (params)
        g_free (params);
}


static Plugin*
plugin_new (MooEditPluginInfo  *info,
            gpointer            plugin_data)
{
    Plugin *plugin = g_new0 (Plugin, 1);

    plugin->data = plugin_data;
    plugin->initialized = FALSE;

    plugin->info.id = g_strdup (info->id);
    plugin->info.name = g_strdup (info->name ? info->name : info->id);
    plugin->info.description = g_strdup (info->description ? info->description : plugin->info.name);

    plugin->info.init = info->init;
    plugin->info.deinit = info->deinit;
    plugin->info.attach = info->attach;
    plugin->info.detach = info->detach;
    plugin->info.pane_create = info->pane_create;
    plugin->info.pane_destroy = info->pane_destroy;

    plugin->info.params = plugin_params_copy (info->params);
    plugin->info.prefs_params = plugin_prefs_params_copy (info->prefs_params);

    return plugin;
}


static void
plugin_free (Plugin *plugin)
{
    if (plugin)
    {
        g_free ((char*) plugin->info.id);
        g_free ((char*) plugin->info.name);
        g_free ((char*) plugin->info.description);
        plugin_params_free (plugin->info.params);
        plugin_prefs_params_free (plugin->info.prefs_params);
        g_free (plugin);
    }
}


static Plugin*
plugin_lookup (const char *id)
{
    GSList *l;

    g_return_val_if_fail (id != NULL, NULL);

    for (l = registered_plugins; l != NULL; l = l->next)
    {
        Plugin *plugin = l->data;
        if (!strcmp (plugin->info.id, id))
            return plugin;
    }

    return NULL;
}


static const char*
plugin_id (Plugin *plugin)
{
    g_return_val_if_fail (plugin != NULL, NULL);
    return plugin->info.id;
}


static gboolean
plugin_enabled (Plugin *plugin)
{
    g_return_val_if_fail (plugin != NULL, FALSE);
    return plugin->info.params->enabled;
}


/* XXX should it attach? */
static gboolean
plugin_init (Plugin *plugin)
{
    g_return_val_if_fail (plugin != NULL, FALSE);

    if (!plugin_enabled (plugin) || plugin->initialized)
        return TRUE;

    if (!plugin->info.init ||
         plugin->info.init (&plugin->info, plugin->data))
    {
        plugin->initialized = TRUE;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


/* XXX should it detach? */
static void
plugin_deinit (Plugin *plugin)
{
    g_return_if_fail (plugin != NULL);

    if (!plugin->initialized)
        return;

    if (plugin->info.deinit)
        plugin->info.deinit (&plugin->info, plugin->data);

    plugin->initialized = FALSE;
}


MooEditPluginInfo*
moo_edit_plugin_lookup (const char *id)
{
    Plugin *plugin = plugin_lookup (id);
    return plugin ? &plugin->info : NULL;
}


GSList*
moo_edit_list_plugins (void)
{
    GSList *list = NULL, *l;

    for (l = registered_plugins; l != NULL; l = l->next)
    {
        Plugin *plugin = l->data;
        list = g_slist_prepend (list, g_strdup (plugin_id (plugin)));
    }

    return g_slist_reverse (list);
}


MooEditPluginWindowData*
moo_edit_plugin_get_window_data (MooEditWindow      *window,
                                 const char         *id)
{
    WindowInfo *window_info;
    Plugin *plugin;
    PluginFullWindowData *full_window_data;

    plugin = plugin_lookup (id);
    window_info = window_info_lookup (window);

    if (!window_info || !plugin)
        return NULL;

    full_window_data = window_info_lookup_plugin (window_info, plugin);

    return full_window_data ? &full_window_data->window_data : NULL;
}


static const char*
make_prefs_key (Plugin             *plugin,
                const char         *key)
{
    static char *prefs_key = NULL;

    g_return_val_if_fail (plugin != NULL, NULL);
    g_return_val_if_fail (key != NULL, NULL);

    g_free (prefs_key);
    prefs_key = g_strdup_printf (MOO_EDIT_PLUGIN_PREFS_ROOT "/%s/%s",
                                 plugin_id (plugin), key);
    return prefs_key;
}


void
moo_edit_plugin_read_dir (const char *path)
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
            if (!moo_edit_plugin_read_module (module, prefix))
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


gboolean
moo_edit_plugin_read_module (GModule            *module,
                             const char         *name)
{
    MooEditPluginModuleInitFunc init_func;
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
moo_edit_plugin_attach_prefs (MooPrefsDialog *dialog)
{
    GSList *l;

    g_return_if_fail (MOO_IS_PREFS_DIALOG (dialog));

    for (l = registered_plugins; l != NULL; l = l->next)
    {
        Plugin *plugin = l->data;

        if (plugin->initialized && plugin->info.prefs_params->prefs_page_create)
        {
            GtkWidget *page = plugin->info.prefs_params->prefs_page_create (&plugin->info, plugin->data);

            if (page)
            {
                gtk_object_sink (GTK_OBJECT (g_object_ref (page)));
                moo_prefs_dialog_append_page (dialog, page);
                g_object_unref (page);
            }
        }
    }
}
