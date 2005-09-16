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
#include "mooedit/moopluginprefs-glade.h"
#include "mooutils/mooprefsdialog.h"
#include "mooutils/moostock.h"
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
                               moo_plugin_id (plugin),
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
        if (!strcmp (plugin_id, moo_plugin_id (plugin)))
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
sync_pages (MooPrefsDialog      *dialog,
            MooPrefsDialogPage  *main_page)
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
        if (moo_plugin_enabled (l->data) &&
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
                    g_object_set_data_full (G_OBJECT (plugin_page), "moo-plugin-id",
                                            g_strdup (moo_plugin_id (plugin)),
                                            g_free);
                    plugin_pages = g_slist_append (plugin_pages, plugin_page);
                    moo_prefs_dialog_insert_page (dialog, plugin_page, GTK_WIDGET (main_page), -1);
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
        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter,
                            COLUMN_ENABLED, moo_plugin_enabled (plugin),
                            COLUMN_PLUGIN_ID, moo_plugin_id (plugin),
                            COLUMN_PLUGIN_NAME, moo_plugin_name (plugin),
                            -1);
    }

    selection_changed (gtk_tree_view_get_selection (treeview), page);

    g_slist_free (plugins);
    sync_pages (dialog, page);
}


static void
prefs_apply (MooPrefsDialog      *dialog,
             MooPrefsDialogPage  *page)
{
    GtkTreeView *treeview;
    GtkTreeModel *model;
    GtkTreeIter iter;

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

    sync_pages (dialog, page);
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
                                               MOO_PLUGIN_PREFS_GLADE_UI,
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
