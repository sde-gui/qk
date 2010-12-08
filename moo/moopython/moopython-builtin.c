/*
 *   moopython.c
 *
 *   Copyright (C) 2004-2008 by Yevgen Muntyan <muntyan@tamu.edu>
 *
 *   This file is part of medit.  medit is free software; you can
 *   redistribute it and/or modify it under the terms of the
 *   GNU Lesser General Public License as published by the
 *   Free Software Foundation; either version 2.1 of the License,
 *   or (at your option) any later version.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with medit.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
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
#include "mooutils/mooutils-misc.h"

gboolean
_moo_python_builtin_init (void)
{
    if (!Py_IsInitialized ())
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

#ifndef MOO_BUILD_MOO_MODULE
        reset_log_func ();
#endif
    }

    if (!moo_plugin_loader_lookup (MOO_PYTHON_PLUGIN_LOADER_ID))
    {
        MooPluginLoader *loader = _moo_python_get_plugin_loader ();
        moo_plugin_loader_register (loader, MOO_PYTHON_PLUGIN_LOADER_ID);
        _moo_python_plugin_loader_free (loader);
    }

    return TRUE;
}
