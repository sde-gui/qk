/*
 *   moopython-loader.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_PYTHON_LOADER_H
#define MOO_PYTHON_LOADER_H

#include <mooedit/mooplugin-loader.h>

G_BEGIN_DECLS


#define MOO_PYTHON_PLUGIN_LOADER_ID "Python"
MooPluginLoader *_moo_python_get_plugin_loader (void);

#define _moo_python_plugin_loader_free g_free


G_END_DECLS

#endif /* MOO_PYTHON_LOADER_H */
