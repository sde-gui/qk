/*
 *   moopython-mod.c
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
#include "mooutils/mooutils-win32.h"


static gboolean
sys_path_add_dir (const char *dir)
{
    PyObject *new_path, *path;
    PyObject *add;

    g_return_val_if_fail (dir != NULL, FALSE);

    path = PySys_GetObject ((char*)"path");

    if (!path)
    {
        PyErr_Print ();
        return FALSE;
    }

    if (!PySequence_Check (path))
    {
        g_critical ("sys.path is not a sequence");
        return FALSE;
    }

    add = PyList_New (1);
    PyList_SET_ITEM (add, 0, PyString_FromString (dir));
    new_path = PySequence_Concat (add, path);
    Py_DECREF (add);

    if (!new_path)
    {
        PyErr_Print ();
        return FALSE;
    }

    if (PySys_SetObject ((char*)"path", new_path) != 0)
    {
        PyErr_Print ();
        Py_DECREF (new_path);
        return FALSE;
    }

    Py_DECREF (new_path);
    return TRUE;
}

static void
sys_path_remove_dir (const char *dir)
{
    PyObject *path;
    int i, len;
    gboolean found = FALSE;

    g_return_if_fail (dir != NULL);

    path = PySys_GetObject ((char*) "path");

    if (!path)
    {
        PyErr_Print ();
        return;
    }

    if (!PySequence_Check (path))
        return;

    len = PySequence_Size (path);

    if (len < 0)
    {
        PyErr_Print ();
        return;
    }

    for (i = len - 1; !found && i >= 0; --i)
    {
        PyObject *item = PySequence_ITEM (path, i);

        if (item && PyString_CheckExact (item) &&
            !strcmp (PyString_AsString (item), dir))
        {
            found = TRUE;
            PySequence_DelItem (path, i);
        }

        if (PyErr_Occurred ())
            PyErr_Print ();

        Py_XDECREF (item);
    }
}


MOO_MODULE_INIT_FUNC_DECL;
MOO_MODULE_INIT_FUNC_DECL
{
    PyObject *moo_mod;
    char *dlldir = NULL;

    if (g_getenv ("MOO_DEBUG_NO_PYTHON"))
        return FALSE;

    if (moo_python_running ())
        return FALSE;

    if (!moo_python_api_init ())
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
        _moo_python_plugin_loader_free (loader);
    }

    return TRUE;
}
