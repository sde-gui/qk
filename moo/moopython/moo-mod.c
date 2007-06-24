/*
 *   moo-mod.c
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <Python.h>
#define NO_IMPORT_PYGOBJECT
#include "pygobject.h"

#include "moopython/pygtk/moo-pygtk.h"
#include "moopython/moopython-api.h"
#include "moopython/moopython-loader.h"
#include "mooutils/moopython.h"

void initmoo (void);

void initmoo (void)
{
    if (!_moo_pygtk_init ())
        return;

    if (!moo_python_running() && !moo_python_api_init ())
        return;

    if (!moo_plugin_loader_lookup (MOO_PYTHON_PLUGIN_LOADER_ID))
    {
        MooPluginLoader *loader = _moo_python_get_plugin_loader ();
        moo_plugin_loader_register (loader, MOO_PYTHON_PLUGIN_LOADER_ID);
        _moo_python_plugin_loader_free (loader);
    }
}
