/*
 *   moopython-loader.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef __MOO_PYTHON_LOADER_H__
#define __MOO_PYTHON_LOADER_H__

#include <mooedit/mooplugin-loader.h>

G_BEGIN_DECLS


#define MOO_PYTHON_PLUGIN_LOADER_ID "Python"
MooPluginLoader *_moo_python_get_plugin_loader (void);

#define _moo_python_plugin_loader_free g_free


G_END_DECLS

#endif /* __MOO_PYTHON_LOADER_H__ */
