/*
 *   mooplugin-loader.h
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

#ifndef __MOO_PLUGIN_LOADER_H__
#define __MOO_PLUGIN_LOADER_H__

#include <mooedit/mooplugin.h>

G_BEGIN_DECLS


#define MOO_TYPE_PLUGIN_LOADER                 (moo_plugin_loader_get_type ())
#define MOO_PLUGIN_LOADER(object)              (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_PLUGIN_LOADER, MooPluginLoader))
#define MOO_PLUGIN_LOADER_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_PLUGIN_LOADER, MooPluginLoaderClass))
#define MOO_IS_PLUGIN_LOADER(object)           (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_PLUGIN_LOADER))
#define MOO_IS_PLUGIN_LOADER_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_PLUGIN_LOADER))
#define MOO_PLUGIN_LOADER_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_PLUGIN_LOADER, MooPluginLoaderClass))

typedef struct _MooPluginLoadInfo       MooPluginLoadInfo;
typedef struct _MooPluginLoader         MooPluginLoader;
typedef struct _MooPluginLoaderClass    MooPluginLoaderClass;

struct _MooPluginLoader
{
    GObject base;
};

struct _MooPluginLoaderClass
{
    GObjectClass base_class;

    void (*load_module) (MooPluginLoader *loader,
                         const char      *module_file,
                         const char      *ini_file);

    void (*load_plugin) (MooPluginLoader *loader,
                         const char      *plugin_file,
                         const char      *plugin_id,
                         MooPluginInfo   *info,
                         MooPluginParams *params,
                         const char      *ini_file);
};


GType   moo_plugin_loader_get_type  (void) G_GNUC_CONST;
void    moo_plugin_loader_register  (MooPluginLoader    *loader,
                                     const char         *type);

void    _moo_plugin_load            (const char         *dir,
                                     const char         *ini_file);


G_END_DECLS

#endif /* __MOO_PLUGIN_LOADER_H__ */
