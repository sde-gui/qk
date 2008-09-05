/*
 *   moopython-loader.c
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
#include "pygobject.h"
#include <string.h>

#include "moopython/moopython-loader.h"
#include "mooutils/mooutils-misc.h"

#define LIBDIR "lib"


static gboolean
sys_path_add_dir (const char *dir)
{
    PyObject *path;
    PyObject *s;

    path = PySys_GetObject ("path");

    if (!path)
    {
        PyErr_Print ();
        return FALSE;
    }

    if (!PyList_Check (path))
    {
        g_critical ("sys.path is not a list");
        return FALSE;
    }

    s = PyString_FromString (dir);
    PyList_Append (path, s);

    Py_DECREF (s);
    return TRUE;
}

static void
sys_path_remove_dir (const char *dir)
{
    PyObject *path;
    int i;

    path = PySys_GetObject ("path");

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
}

static void
sys_path_add_plugin_dirs (void)
{
    char **d;
    char **dirs;
    static gboolean been_here = FALSE;

    if (been_here)
        return;

    been_here = TRUE;
    dirs = moo_plugin_get_dirs ();

    for (d = dirs; d && *d; ++d)
    {
        char *libdir = g_build_filename (*d, LIBDIR, NULL);
        sys_path_add_dir (libdir);
        g_free (libdir);
    }

    g_strfreev (dirs);
}


static PyObject *
do_load_file (const char *path)
{
    PyObject *mod = NULL;
    PyObject *code;
    char *modname = NULL, *content = NULL;
    GError *error = NULL;

    if (!g_file_get_contents (path, &content, NULL, &error))
    {
        g_warning ("could not read file '%s': %s", path, error->message);
        g_error_free (error);
        return NULL;
    }

    modname = g_strdup_printf ("moo_module_%08x", g_random_int ());
    code = Py_CompileString (content, path, Py_file_input);

    if (!code)
    {
        PyErr_Print ();
        goto out;
    }

    moo_disable_win32_error_message ();
    mod = PyImport_ExecCodeModule (modname, code);
    moo_enable_win32_error_message ();

    Py_DECREF (code);

    if (!mod)
    {
        if (PyErr_ExceptionMatches (PyExc_Exception))
        {
            PyObject *type, *value, *traceback;
            PyObject *r;
            char *s;

            PyErr_Fetch (&type, &value, &traceback);
            PyErr_NormalizeException (&type, &value, &traceback);
            r = PyObject_Repr (value);
            s = r ? PyString_AsString (r) : NULL;

            if (s && strcmp (s, "PluginWontLoad") == 0)
            {
                Py_XDECREF (r);
                Py_XDECREF (type);
                Py_XDECREF (value);
                Py_XDECREF (traceback);
                goto out;
            }

            Py_XDECREF (r);

            PyErr_Restore(type, value, traceback);
        }

        g_warning ("error when loading file '%s'", path);
        PyErr_Print ();
        goto out;
    }

out:
    g_free (content);
    g_free (modname);

    return mod;
}


static PyObject *
load_file (const char *path)
{
    char *dirname;
    gboolean dir_added;
    PyObject *retval;

    g_return_val_if_fail (path != NULL, NULL);

    sys_path_add_plugin_dirs ();

    dirname = g_path_get_dirname (path);
    dir_added = sys_path_add_dir (dirname);

    retval = do_load_file (path);

    if (dir_added)
        sys_path_remove_dir (dirname);

    g_free (dirname);

    return retval;
}


static void
load_python_module (const char *module_file,
                    G_GNUC_UNUSED const char *ini_file,
                    G_GNUC_UNUSED gpointer data)
{
    PyObject *mod;
    mod = load_file (module_file);
    Py_XDECREF (mod);
}


static void
load_python_plugin (const char      *plugin_file,
                    const char      *plugin_id,
                    MooPluginInfo   *info,
                    MooPluginParams *params,
                    G_GNUC_UNUSED const char *ini_file,
                    G_GNUC_UNUSED gpointer data)
{
    PyObject *mod;
    PyObject *py_plugin_type;
    GType plugin_type;

    if (!(mod = load_file (plugin_file)))
        return;

    py_plugin_type = PyObject_GetAttrString (mod, "__plugin__");

    if (!py_plugin_type)
    {
        g_warning ("file %s doesn't define __plugin__ attribute",
                   plugin_file);
    }
    else if (py_plugin_type == Py_None)
    {
        /* it's fine, ignore */
    }
    else if (!PyType_Check (py_plugin_type))
    {
        g_warning ("__plugin__ attribute in file %s is not a type",
                   plugin_file);
    }
    else if (!(plugin_type = pyg_type_from_object (py_plugin_type)))
    {
        g_warning ("__plugin__ attribute in file %s is not a valid type",
                   plugin_file);
        PyErr_Clear ();
    }
    else if (!g_type_is_a (plugin_type, MOO_TYPE_PLUGIN))
    {
        g_warning ("__plugin__ attribute in file %s is not a MooPlugin subclass",
                   plugin_file);
    }
    else
    {
        moo_plugin_register (plugin_id, plugin_type, info, params);
    }

    Py_XDECREF (py_plugin_type);
    Py_XDECREF (mod);
}


MooPluginLoader *
_moo_python_get_plugin_loader (void)
{
    MooPluginLoader *loader = g_new0 (MooPluginLoader, 1);
    loader->load_module = load_python_module;
    loader->load_plugin = load_python_plugin;
    return loader;
}
