/*
 *   moopython-mod.c
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <Python.h>
#include <pygobject.h>
#include <pygtk/pygtk.h>

#include "moopython/moopython-api.h"
#include "moopython/moopython-loader.h"
#include "moopython/moopython-pygtkmod.h"
#include "mooedit/mooplugin-macro.h"
#include "mooutils/moopython.h"


static PyObject *sys_module = NULL;

static gboolean
sys_path_add_dir (const char *dir)
{
    PyObject *path;
    PyObject *s;

    if (!sys_module)
        sys_module = PyImport_ImportModule ((char*) "sys");

    if (!sys_module)
    {
        PyErr_Print ();
        return FALSE;
    }

    path = PyObject_GetAttrString (sys_module, (char*) "path");

    if (!path)
    {
        PyErr_Print ();
        return FALSE;
    }

    if (!PyList_Check (path))
    {
        g_critical ("sys.path is not a list");
        Py_DECREF (path);
        return FALSE;
    }

    s = PyString_FromString (dir);
    PyList_Append (path, s);

    Py_DECREF (s);
    Py_DECREF (path);
    return TRUE;
}

static void
sys_path_remove_dir (const char *dir)
{
    PyObject *path;
    int i;

    if (!sys_module)
        return;

    path = PyObject_GetAttrString (sys_module, (char*) "path");

    if (!path || !PyList_Check (path))
        return;

    for (i = PyList_GET_SIZE (path) - 1; i >= 0; --i)
    {
        PyObject *item = PyList_GET_ITEM (path, i);

        if (PyString_CheckExact (item) &&
            !strcmp (PyString_AsString (item), dir))
        {
            if (PySequence_DelItem (path, i) != 0)
                PyErr_Print ();
            break;
        }
    }

    Py_DECREF (path);
}


MOO_MODULE_INIT_FUNC_DECL;
MOO_MODULE_INIT_FUNC_DECL
{
    PyObject *moo_mod;
    char *dlldir = NULL;

    if (moo_python_running ())
        return FALSE;

    if (!moo_python_api_init (TRUE))
    {
        g_warning ("%s: oops", G_STRLOC);
        return FALSE;
    }

#ifdef __WIN32__
    dlldir = moo_win32_get_dll_dir (MOO_PYTHON_MODULE_DLL_NAME);
#endif

    if (dlldir && !sys_path_add_dir (dlldir))
    {
        g_free (dlldir);
        dlldir = NULL;
    }

    moo_mod = PyImport_ImportModule ((char*) "moo");

    if (dlldir)
    {
        sys_path_remove_dir (dlldir);
        g_free (dlldir);
    }

    if (!moo_mod)
    {
        PyErr_Print ();
        g_warning ("%s: could not import moo", G_STRLOC);
        moo_python_api_deinit ();
        return FALSE;
    }

    init_pygtk_mod ();

    if (PyErr_Occurred ())
    {
        PyErr_Print ();
        g_warning ("%s: could not import gobject", G_STRLOC);
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

    return TRUE;
}
