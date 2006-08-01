/*
 *   mooplugin-python.c
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

#include <Python.h>

#ifndef MOO_INSTALL_LIB
#define NO_IMPORT_PYGOBJECT
#endif

#include "pygobject.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "moopython/mooplugin-python.h"
#include "moopython/moopython-utils.h"
#include "mooutils/moopython.h"
#include "mooutils/mooutils-misc.h"
#include "moopython/pygtk/moo-pygtk.h"
#include "mooedit/mooplugin-macro.h"

#define PLUGIN_SUFFIX ".py"
#define LIBDIR "lib"


static PyObject *main_mod;
static gboolean in_moo_module;


static MooPyObject*
run_simple_string (const char *str)
{
    PyObject *dict;
    g_return_val_if_fail (str != NULL, NULL);
    dict = PyModule_GetDict (main_mod);
    return (MooPyObject*) PyRun_String (str, Py_file_input, dict, dict);
}


static MooPyObject*
run_string (const char  *str,
            MooPyObject *locals,
            MooPyObject *globals)
{
    g_return_val_if_fail (str != NULL, NULL);
    g_return_val_if_fail (locals != NULL, NULL);
    g_return_val_if_fail (globals != NULL, NULL);
    return (MooPyObject*) PyRun_String (str, Py_file_input,
                                        (PyObject*) locals,
                                        (PyObject*) globals);
}


static MooPyObject*
run_file (gpointer    fp,
          const char *filename)
{
    PyObject *dict;
    g_return_val_if_fail (fp != NULL && filename != NULL, NULL);
    dict = PyModule_GetDict (main_mod);
    return (MooPyObject*) PyRun_File (fp, filename, Py_file_input, dict, dict);
}


static MooPyObject*
incref (MooPyObject *obj)
{
    if (obj)
    {
        Py_INCREF ((PyObject*) obj);
    }

    return obj;
}

static void
decref (MooPyObject *obj)
{
    if (obj)
    {
        Py_DECREF ((PyObject*) obj);
    }
}


static MooPyObject *
py_object_from_gobject (gpointer gobj)
{
    g_return_val_if_fail (!gobj || G_IS_OBJECT (gobj), NULL);
    return (MooPyObject*) pygobject_new (gobj);
}


static MooPyObject *
dict_get_item (MooPyObject *dict,
               const char  *key)
{
    g_return_val_if_fail (dict != NULL, NULL);
    g_return_val_if_fail (key != NULL, NULL);
    return (MooPyObject*) PyDict_GetItemString ((PyObject*) dict, (char*) key);
}

static gboolean
dict_set_item (MooPyObject *dict,
               const char  *key,
               MooPyObject *val)
{
    g_return_val_if_fail (dict != NULL, FALSE);
    g_return_val_if_fail (key != NULL, FALSE);
    g_return_val_if_fail (val != NULL, FALSE);

    if (PyDict_SetItemString ((PyObject*) dict, (char*) key, (PyObject*) val))
    {
        PyErr_Print ();
        return FALSE;
    }

    return TRUE;
}

static gboolean
dict_del_item (MooPyObject *dict,
               const char  *key)
{
    g_return_val_if_fail (dict != NULL, FALSE);
    g_return_val_if_fail (key != NULL, FALSE);

    if (PyDict_DelItemString ((PyObject*) dict, (char*) key))
    {
        PyErr_Print ();
        return FALSE;
    }

    return TRUE;
}


static MooPyObject *
get_script_dict (const char *name)
{
    PyObject *dict, *builtins;

    builtins = PyImport_ImportModule ((char*) "__builtin__");
    g_return_val_if_fail (builtins != NULL, NULL);

    dict = PyDict_New ();
    PyDict_SetItemString (dict, (char*) "__builtins__", builtins);

    if (name)
    {
        PyObject *py_name = PyString_FromString (name);
        PyDict_SetItemString (dict, (char*) "__name__", py_name);
        Py_XDECREF (py_name);
    }

    Py_XDECREF (builtins);
    return (MooPyObject*) dict;
}


static void
err_print (void)
{
    PyErr_Print ();
}


static char *
get_info (void)
{
    return g_strdup_printf ("%s %s", Py_GetVersion (), Py_GetPlatform ());
}


static gboolean
moo_python_api_init (void)
{
    static int argc;
    static char **argv;

    static MooPyAPI api = {
        incref, decref, err_print,
        get_info,
        run_simple_string, run_string, run_file,
        py_object_from_gobject,
        get_script_dict,
        dict_get_item, dict_set_item, dict_del_item
    };

    g_return_val_if_fail (!moo_python_running(), FALSE);

    if (!moo_python_init (MOO_PY_API_VERSION, &api))
    {
        g_warning ("%s: oops", G_STRLOC);
        return FALSE;
    }

    g_assert (moo_python_running ());

    if (!argv)
    {
        argc = 1;
        argv = g_new0 (char*, 2);
        argv[0] = g_strdup ("");
    }

#if PY_MINOR_VERSION >= 4
    /* do not let python install signal handlers */
    Py_InitializeEx (FALSE);
#else
    Py_Initialize ();
#endif

    /* pygtk wants sys.argv */
    PySys_SetArgv (argc, argv);

    moo_py_init_print_funcs ();

    main_mod = PyImport_AddModule ((char*)"__main__");

    if (!main_mod)
    {
        g_warning ("%s: could not import __main__", G_STRLOC);
        PyErr_Print ();
        moo_python_init (MOO_PY_API_VERSION, NULL);
        return FALSE;
    }

    Py_XINCREF ((PyObject*) main_mod);
    return TRUE;
}


static void
moo_python_plugin_read_file (const char *path)
{
    PyObject *mod, *code;
    char *modname = NULL, *content = NULL;
    GError *error = NULL;

    g_return_if_fail (path != NULL);

    if (!g_file_get_contents (path, &content, NULL, &error))
    {
        g_warning ("%s: could not read plugin file", G_STRLOC);
        g_warning ("%s: %s", G_STRLOC, error->message);
        g_error_free (error);
        return;
    }

    modname = g_strdup_printf ("moo_plugin_%08x%08x", g_random_int (), g_random_int ());
    code = Py_CompileString (content, path, Py_file_input);

    if (!code)
    {
        PyErr_Print ();
        goto out;
    }

    mod = PyImport_ExecCodeModule (modname, code);
    Py_DECREF (code);

    if (!mod)
    {
        PyErr_Print ();
        goto out;
    }

    Py_DECREF (mod);

out:
    g_free (content);
    g_free (modname);
}


static void
moo_python_plugin_read_dir (const char *path)
{
    GDir *dir;
    const char *name;

    g_return_if_fail (path != NULL);

    dir = g_dir_open (path, 0, NULL);

    if (!dir)
        return;

    while ((name = g_dir_read_name (dir)))
    {
        char *file_path, *prefix, *suffix;

        if (!g_str_has_suffix (name, PLUGIN_SUFFIX))
            continue;

        suffix = g_strrstr (name, PLUGIN_SUFFIX);
        prefix = g_strndup (name, suffix - name);

        file_path = g_build_filename (path, name, NULL);

        /* don't try broken links */
        if (g_file_test (file_path, G_FILE_TEST_EXISTS))
            moo_python_plugin_read_file (file_path);

        g_free (prefix);
        g_free (file_path);
    }

    g_dir_close (dir);
}


static void
moo_python_plugin_read_dirs (void)
{
    char **d;
    char **dirs = moo_plugin_get_dirs ();
    PyObject *sys = NULL, *path = NULL;

    sys = PyImport_ImportModule ((char*) "sys");

    if (sys)
        path = PyObject_GetAttrString (sys, (char*) "path");

    if (!path || !PyList_Check (path))
    {
        g_critical ("%s: oops", G_STRLOC);
    }
    else
    {
        for (d = dirs; d && *d; ++d)
        {
            char *libdir = g_build_filename (*d, LIBDIR, NULL);
            PyObject *s = PyString_FromString (libdir);
            PyList_Append (path, s);
            Py_XDECREF (s);
            g_free (libdir);
        }
    }

    for (d = dirs; d && *d; ++d)
        moo_python_plugin_read_dir (*d);

    g_strfreev (dirs);
}


static gboolean
init_moo_pygtk (void)
{
#ifndef MOO_INSTALL_LIB
    return _moo_pygtk_init ();
#else
    PyObject *module = PyImport_ImportModule ((char*) "moo");

    if (!module)
        PyErr_SetString(PyExc_ImportError,
                        "could not import moo");

    return module != NULL;
#endif
}


gboolean
_moo_python_plugin_init (void)
{
    if (!in_moo_module)
    {
        if (!moo_python_api_init ())
            return FALSE;

        if (!init_moo_pygtk ())
        {
            PyErr_Print ();
            moo_python_init (MOO_PY_API_VERSION, NULL);
            return FALSE;
        }

#ifdef pyg_disable_warning_redirections
        pyg_disable_warning_redirections ();
#else
        moo_reset_log_func ();
#endif
    }

    moo_python_plugin_read_dirs ();
    return TRUE;
}


void
_moo_python_plugin_deinit (void)
{
    Py_XDECREF (main_mod);
    main_mod = NULL;
    moo_python_init (MOO_PY_API_VERSION, NULL);
}


#ifdef MOO_PYTHON_PLUGIN
MOO_PLUGIN_INIT_FUNC_DECL;
MOO_PLUGIN_INIT_FUNC_DECL
{
    MOO_MODULE_CHECK_VERSION ();

    return !moo_python_running () &&
            _moo_python_plugin_init ();
}
#endif


#ifdef MOO_PYTHON_MODULE
void initmoo (void)
{
    in_moo_module = TRUE;

    if (!moo_python_running())
        moo_python_api_init ();

    _moo_pygtk_init ();
}
#endif
