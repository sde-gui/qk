/*
 *   moopython.c
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

#include "config.h"
#include <Python.h>
#define NO_IMPORT_PYGOBJECT
#define NO_IMPORT_PYGTK
#include <pygobject.h>
#include <pygtk/pygtk.h>
#include "mooedit/mooplugin-loader.h"
#include "moopython/moopython-builtin.h"
#include "moopython/moopython-api.h"
#include "moopython/moopython-loader.h"
#include "moopython/pygtk/moo-pygtk.h"
#include "moopython/moopython-pygtkmod.h"
#include "mooutils/moopython.h"
#include "mooutils/mooutils-misc.h"

gboolean
_moo_python_init (void)
{
    if (!moo_python_running ())
    {
        if (!moo_python_api_init ())
        {
            g_warning ("%s: oops", G_STRLOC);
            return FALSE;
        }

        if (!_moo_pygtk_init ())
        {
            g_warning ("%s: could not initialize moo module", G_STRLOC);
            PyErr_Print ();
            moo_python_api_deinit ();
            return FALSE;
        }

        reset_log_func ();

        if (!moo_plugin_loader_lookup (MOO_PYTHON_PLUGIN_LOADER_ID))
        {
            MooPluginLoader *loader = _moo_python_get_plugin_loader ();
            moo_plugin_loader_register (loader, MOO_PYTHON_PLUGIN_LOADER_ID);
            g_free (loader);
        }
    }

    return TRUE;
}
