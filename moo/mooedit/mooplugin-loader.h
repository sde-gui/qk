/*
 *   mooplugin-loader.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_PLUGIN_LOADER_H
#define MOO_PLUGIN_LOADER_H

#include <mooedit/mooplugin.h>

G_BEGIN_DECLS


typedef struct _MooPluginLoader         MooPluginLoader;

typedef void (*MooLoadModuleFunc) (const char      *module_file,
                                   const char      *ini_file,
                                   gpointer         data);
typedef void (*MooLoadPluginFunc) (const char      *plugin_file,
                                   const char      *plugin_id,
                                   MooPluginInfo   *info,
                                   MooPluginParams *params,
                                   const char      *ini_file,
                                   gpointer         data);


struct _MooPluginLoader
{
    MooLoadModuleFunc load_module;
    MooLoadPluginFunc load_plugin;
    gpointer data;
};

void             moo_plugin_loader_register (const MooPluginLoader  *loader,
                                             const char             *type);
MooPluginLoader *moo_plugin_loader_lookup   (const char             *type);

void             _moo_plugin_load           (const char             *dir,
                                             const char             *ini_file);
void             _moo_plugin_finish_load    (void);


G_END_DECLS

#endif /* MOO_PLUGIN_LOADER_H */
