/*
 *   mooplugin-loader.c
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

#include <config.h>
#include "mooedit/mooplugin-loader.h"
#include "mooedit/mooplugin.h"
#include "moopython/mooplugin-python.h"
#include "mooutils/mooutils-misc.h"
#include <string.h>
#include <stdlib.h>
#include <gmodule.h>

#ifdef __WIN32__
#include <windows.h>
#endif

#define GROUP_MODULE    "module"
#define GROUP_PLUGIN    "plugin"
#define KEY_LOADER      "type"
#define KEY_FILE        "file"
#define KEY_ID          "id"
#define KEY_NAME        "name"
#define KEY_DESCRIPTION "description"
#define KEY_AUTHOR      "author"
#define KEY_VERSION     "version"
#define KEY_LANGS       "langs"
#define KEY_ENABLED     "enabled"
#define KEY_VISIBLE     "visible"


typedef struct {
    char            *loader;
    char            *file;
    char            *plugin_id;
    MooPluginInfo   *plugin_info;
    MooPluginParams *plugin_params;
} ModuleInfo;


static GHashTable *registered_loaders;
static void init_loaders (void);
GType _moo_c_plugin_loader_get_type (void);


typedef MooPluginLoader MooCPluginLoader;
typedef MooPluginLoaderClass MooCPluginLoaderClass;

G_DEFINE_TYPE (MooPluginLoader, moo_plugin_loader, G_TYPE_OBJECT)
G_DEFINE_TYPE (MooCPluginLoader, _moo_c_plugin_loader, MOO_TYPE_PLUGIN_LOADER)


static void
moo_plugin_loader_init (G_GNUC_UNUSED MooPluginLoader *loader)
{
}


static void
moo_plugin_loader_class_init (G_GNUC_UNUSED MooPluginLoaderClass *klass)
{
}


static MooPluginLoader *
moo_plugin_loader_lookup (const char *id)
{
    g_return_val_if_fail (id != NULL, NULL);
    init_loaders ();
    return g_hash_table_lookup (registered_loaders, id);
}


static void
moo_plugin_loader_add (MooPluginLoader *loader,
                       const char      *type)
{
    g_return_if_fail (MOO_IS_PLUGIN_LOADER (loader));
    g_return_if_fail (type != NULL);
    g_return_if_fail (registered_loaders != NULL);

    g_hash_table_insert (registered_loaders, g_strdup (type), g_object_ref (loader));
}


void
moo_plugin_loader_register (MooPluginLoader *loader,
                            const char      *type)
{
    g_return_if_fail (MOO_IS_PLUGIN_LOADER (loader));
    g_return_if_fail (type != NULL);
    init_loaders ();
    moo_plugin_loader_add (loader, type);
}


static void
moo_plugin_loader_load (MooPluginLoader *loader,
                        ModuleInfo      *module_info,
                        const char      *ini_file)
{
    if (module_info->plugin_id)
    {
        g_return_if_fail (MOO_PLUGIN_LOADER_GET_CLASS(loader)->load_plugin != NULL);
        MOO_PLUGIN_LOADER_GET_CLASS(loader)->load_plugin (loader,
                                                          module_info->file,
                                                          module_info->plugin_id,
                                                          module_info->plugin_info,
                                                          module_info->plugin_params,
                                                          ini_file);
    }
    else
    {
        g_return_if_fail (MOO_PLUGIN_LOADER_GET_CLASS(loader)->load_module != NULL);
        MOO_PLUGIN_LOADER_GET_CLASS(loader)->load_module (loader, module_info->file, ini_file);
    }
}


static gboolean
parse_plugin_info (GKeyFile         *key_file,
                   const char       *plugin_id,
                   MooPluginInfo   **info_p,
                   MooPluginParams **params_p)
{
    MooPluginInfo *info;
    MooPluginParams *params;
    char *name;
    char *description;
    char *author;
    char *version;
    char *langs;
    gboolean enabled = TRUE;
    gboolean visible = TRUE;

    name = g_key_file_get_string (key_file, GROUP_PLUGIN, KEY_NAME, NULL);
    description = g_key_file_get_string (key_file, GROUP_PLUGIN, KEY_DESCRIPTION, NULL);
    author = g_key_file_get_string (key_file, GROUP_PLUGIN, KEY_AUTHOR, NULL);
    version = g_key_file_get_string (key_file, GROUP_PLUGIN, KEY_VERSION, NULL);
    langs = g_key_file_get_string (key_file, GROUP_PLUGIN, KEY_LANGS, NULL);

    if (g_key_file_has_key (key_file, GROUP_PLUGIN, KEY_ENABLED, NULL))
        enabled = g_key_file_get_boolean (key_file, GROUP_PLUGIN, KEY_ENABLED, NULL);
    if (g_key_file_has_key (key_file, GROUP_PLUGIN, KEY_VISIBLE, NULL))
        visible = g_key_file_get_boolean (key_file, GROUP_PLUGIN, KEY_VISIBLE, NULL);

    info = moo_plugin_info_new (name ? name : plugin_id,
                                description ? description : "",
                                author ? author : "",
                                version ? version : "",
                                langs);
    params = moo_plugin_params_new (enabled, visible);

    *info_p = info;
    *params_p = params;

    g_free (name);
    g_free (description);
    g_free (author);
    g_free (version);
    g_free (langs);

    return TRUE;
}


static gboolean
check_version (const char *version,
               const char *ini_file_path)
{
    char *dot;
    char *end = NULL;
    long major, minor;

    dot = strchr (version, '.');

    if (!dot || dot == version)
        goto invalid;

    major = strtol (version, &end, 10);

    if (end != dot)
        goto invalid;

    minor = strtol (dot + 1, &end, 10);

    if (*end != 0)
        goto invalid;

    if (major != MOO_VERSION_MAJOR || minor > MOO_VERSION_MINOR)
    {
        g_warning ("module version %s is not compatible with current version %d.%d",
                   version, MOO_VERSION_MAJOR, MOO_VERSION_MINOR);
        return FALSE;
    }

    return TRUE;

invalid:
    g_warning ("invalid module version '%s' in file '%s'",
               version, ini_file_path);
    return FALSE;
}


static gboolean
parse_ini_file (const char       *dir,
                const char       *ini_file,
                ModuleInfo       *module_info)
{
    GKeyFile *key_file;
    GError *error = NULL;
    char *ini_file_path;
    gboolean success = FALSE;
    char *file = NULL, *loader = NULL, *id = NULL, *version = NULL;
    MooPluginInfo *info = NULL;
    MooPluginParams *params = NULL;

    ini_file_path = g_build_filename (dir, ini_file, NULL);
    key_file = g_key_file_new ();

    if (!g_key_file_load_from_file (key_file, ini_file_path, 0, &error))
    {
        g_warning ("error parsing plugin ini file '%s': %s", ini_file_path, error->message);
        goto out;
    }

    if (!g_key_file_has_group (key_file, GROUP_MODULE))
    {
        g_warning ("plugin ini file '%s' does not have '" GROUP_MODULE "' group", ini_file_path);
        goto out;
    }

    if (!(version = g_key_file_get_string (key_file, GROUP_MODULE, KEY_VERSION, &error)))
    {
        g_warning ("plugin ini file '%s' does not specify version of module system", ini_file_path);
        goto out;
    }

    if (!check_version (version, ini_file_path))
        goto out;

    if (!(loader = g_key_file_get_string (key_file, GROUP_MODULE, KEY_LOADER, &error)))
    {
        g_warning ("plugin ini file '%s' does not specify module type", ini_file_path);
        goto out;
    }

    if (!(file = g_key_file_get_string (key_file, GROUP_MODULE, KEY_FILE, &error)))
    {
        g_warning ("plugin ini file '%s' does not specify module file", ini_file_path);
        goto out;
    }

    if (!g_path_is_absolute (file))
    {
        char *tmp = file;
        file = g_build_filename (dir, file, NULL);
        g_free (tmp);
    }

    if (g_key_file_has_group (key_file, GROUP_PLUGIN))
    {
        if (!(id = g_key_file_get_string (key_file, GROUP_PLUGIN, KEY_ID, NULL)))
        {
            g_warning ("plugin ini file '%s' does not specify plugin id", ini_file_path);
            goto out;
        }

        if (!parse_plugin_info (key_file, id, &info, &params))
            goto out;
    }

    module_info->loader = loader;
    module_info->file = g_build_path (dir, file, NULL);
    module_info->plugin_id = id;
    module_info->plugin_info = info;
    module_info->plugin_params = params;
    success = TRUE;

out:
    if (error)
        g_error_free (error);
    g_free (ini_file_path);
    g_key_file_free (key_file);
    g_free (file);
    g_free (version);

    if (!success)
    {
        g_free (loader);
        g_free (id);
        moo_plugin_info_free (info);
        moo_plugin_params_free (params);
    }

    return success;
}


static void
module_info_destroy (ModuleInfo *module_info)
{
    g_free (module_info->loader);
    g_free (module_info->file);
    g_free (module_info->plugin_id);
    moo_plugin_info_free (module_info->plugin_info);
    moo_plugin_params_free (module_info->plugin_params);
}


void
_moo_plugin_load (const char *dir,
                  const char *ini_file)
{
    ModuleInfo module_info;
    MooPluginLoader *loader;
    char *ini_file_path;

    g_return_if_fail (dir != NULL && ini_file != NULL);

    if (!parse_ini_file (dir, ini_file, &module_info))
        return;

    loader = moo_plugin_loader_lookup (module_info.loader);

    if (!loader)
    {
        /* XXX */
        g_warning ("unknown module type '%s' in file '%s'", module_info.loader, ini_file);
        module_info_destroy (&module_info);
        return;
    }

    ini_file_path = g_build_filename (dir, ini_file, NULL);
    moo_plugin_loader_load (loader, &module_info, ini_file_path);

    g_free (ini_file_path);
    module_info_destroy (&module_info);
}


static GModule *
module_open (const char *path)
{
    GModule *module;

    _moo_disable_win32_error_message ();
    module = g_module_open (path, G_MODULE_BIND_LAZY | G_MODULE_BIND_LOCAL);
    _moo_enable_win32_error_message ();

    if (!module)
        g_warning ("could not open module '%s': %s", path, g_module_error ());

    return module;
}


static void
load_c_module (G_GNUC_UNUSED MooPluginLoader *loader,
               const char *module_file,
               G_GNUC_UNUSED const char *ini_file)
{
    MooModuleInitFunc init_func;
    GModule *module;

    module = module_open (module_file);

    if (!module)
        return;

    if (g_module_symbol (module, MOO_MODULE_INIT_FUNC_NAME,
                         (gpointer*) &init_func) &&
        init_func ())
    {
        g_module_make_resident (module);
    }

    g_module_close (module);
}


static void
load_c_plugin (G_GNUC_UNUSED MooPluginLoader *loader,
               const char      *plugin_file,
               const char      *plugin_id,
               MooPluginInfo   *info,
               MooPluginParams *params,
               G_GNUC_UNUSED const char      *ini_file)
{
    MooPluginModuleInitFunc init_func;
    GModule *module;
    GType type = 0;

    module = module_open (plugin_file);

    if (!module)
        return;

    if (!g_module_symbol (module, MOO_PLUGIN_INIT_FUNC_NAME, (gpointer*) &init_func))
    {
        g_module_close (module);
        return;
    }

    if (!init_func (&type))
    {
        g_module_close (module);
        return;
    }

    g_return_if_fail (g_type_is_a (type, MOO_TYPE_PLUGIN));

    if (!moo_plugin_register (plugin_id, type, info, params))
    {
        g_module_close (module);
        return;
    }

    g_module_make_resident (module);
    g_module_close (module);
}


static void
_moo_c_plugin_loader_class_init (MooPluginLoaderClass *klass)
{
    klass->load_module = load_c_module;
    klass->load_plugin = load_c_plugin;
}


static void
_moo_c_plugin_loader_init (G_GNUC_UNUSED MooPluginLoader *loader)
{
}


static void
init_loaders (void)
{
    MooPluginLoader *loader;

    if (registered_loaders)
        return;

    registered_loaders = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                g_free, g_object_unref);

    loader = g_object_new (_moo_c_plugin_loader_get_type (), NULL);
    moo_plugin_loader_add (loader, "C");
    g_object_unref (loader);

#ifdef MOO_USE_PYGTK
    loader = _moo_python_get_plugin_loader ();
    moo_plugin_loader_add (loader, MOO_PYTHON_PLUGIN_LOADER_ID);
    g_object_unref (loader);
#endif
}
