/*
 *   moopython-api.h
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

#ifndef __MOO_PYTHON_API_H__
#define __MOO_PYTHON_API_H__


#include "moopython/moopython-utils.h"
#include "mooutils/moopython.h"
#include <Python.h>
#define NO_IMPORT_PYGOBJECT
#include "pygobject.h"


static MooPyObject *
moo_python_api_run_simple_string (const char *str)
{
    PyObject *dict, *main_mod;
    g_return_val_if_fail (str != NULL, NULL);
    main_mod = PyImport_AddModule ((char*)"__main__");
    dict = PyModule_GetDict (main_mod);
    return (MooPyObject*) PyRun_String (str, Py_file_input, dict, dict);
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


static MooPyObject*
moo_python_api_run_string (const char  *str,
                           MooPyObject *locals,
                           MooPyObject *globals)
{
    PyObject *ret;

    g_return_val_if_fail (str != NULL, NULL);

    if (!locals)
        locals = get_script_dict ("__script__");
    else
        moo_Py_INCREF (locals);

    g_return_val_if_fail (locals != NULL, NULL);

    if (!globals)
        globals = locals;

    ret = PyRun_String (str, Py_file_input, (PyObject*) locals, (PyObject*) globals);

    moo_Py_DECREF (locals);
    return (MooPyObject*) ret;
}


static MooPyObject*
moo_python_api_run_file (gpointer    fp,
                         const char *filename)
{
    PyObject *dict, *main_mod;
    g_return_val_if_fail (fp != NULL && filename != NULL, NULL);
    main_mod = PyImport_AddModule ((char*)"__main__");
    dict = PyModule_GetDict (main_mod);
    return (MooPyObject*) PyRun_File (fp, filename, Py_file_input, dict, dict);
}


static MooPyObject *
moo_python_api_run_code (const char  *str,
                         MooPyObject *locals,
                         MooPyObject *globals)
{
    PyObject *ret;

    g_return_val_if_fail (str != NULL, NULL);

    if (!locals)
        locals = get_script_dict ("__script__");
    else
        moo_Py_INCREF (locals);

    g_return_val_if_fail (locals != NULL, NULL);

    if (!globals)
        globals = locals;

    ret = PyRun_String (str, Py_file_input, (PyObject*) locals, (PyObject*) globals);

    if (ret)
    {
        Py_DECREF (ret);

        if (PyMapping_HasKeyString ((PyObject*) locals, (char*) "__retval__"))
            ret = PyMapping_GetItemString ((PyObject*) locals, (char*) "__retval__");
        else
            ret = NULL;
    }

    moo_Py_DECREF (locals);
    return (MooPyObject*) ret;
}


static MooPyObject*
moo_python_api_incref (MooPyObject *obj)
{
    if (obj)
    {
        Py_INCREF ((PyObject*) obj);
    }

    return obj;
}

static void
moo_python_api_decref (MooPyObject *obj)
{
    if (obj)
    {
        Py_DECREF ((PyObject*) obj);
    }
}


static MooPyObject *
moo_python_api_py_object_from_gobject (gpointer gobj)
{
    g_return_val_if_fail (!gobj || G_IS_OBJECT (gobj), NULL);
    return (MooPyObject*) pygobject_new (gobj);
}


static MooPyObject *
moo_python_api_dict_get_item (MooPyObject *dict,
                              const char  *key)
{
    g_return_val_if_fail (dict != NULL, NULL);
    g_return_val_if_fail (key != NULL, NULL);
    return (MooPyObject*) PyDict_GetItemString ((PyObject*) dict, (char*) key);
}

static gboolean
moo_python_api_dict_set_item (MooPyObject *dict,
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
moo_python_api_dict_del_item (MooPyObject *dict,
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


static void
moo_python_api_err_print (void)
{
    PyErr_Print ();
}


static char *
moo_python_api_get_info (void)
{
    return g_strdup_printf ("%s %s", Py_GetVersion (), Py_GetPlatform ());
}


static MooPyObject *
moo_python_api_call_meth (MooPyObject *obj,
                          const char  *meth,
                          const char  *arg)
{
    PyObject *ret;

    g_return_val_if_fail (obj != NULL, NULL);
    g_return_val_if_fail (meth != NULL, NULL);

    if (arg)
        ret = PyObject_CallMethod ((PyObject*) obj, (char*) meth, (char*) "(s)", arg);
    else
        ret = PyObject_CallMethod ((PyObject*) obj, (char*) meth, (char*) "()");

    return (MooPyObject*) ret;

}


static MooPyObject *
moo_python_api_import_exec (const char  *name,
                            const char  *string)
{
    PyObject *code;
    PyObject *mod;
    char *filename;

    g_return_val_if_fail (name != NULL, NULL);
    g_return_val_if_fail (string != NULL, NULL);

    filename = g_strdup_printf ("%s.py", name);
    code = Py_CompileString (string, filename, Py_file_input);
    g_free (filename);

    if (!code)
        return FALSE;

    mod = PyImport_ExecCodeModule ((char*) name, code);

    Py_DECREF (code);
    return (MooPyObject*) mod;
}


static gboolean
moo_python_api_init (void)
{
    static int argc;
    static char **argv;

    static MooPyAPI api = {
        moo_python_api_incref,
        moo_python_api_decref,
        moo_python_api_err_print,
        moo_python_api_get_info,
        moo_python_api_run_simple_string,
        moo_python_api_run_string,
        moo_python_api_run_file,
        moo_python_api_run_code,
        moo_python_api_py_object_from_gobject,
        moo_python_api_dict_get_item,
        moo_python_api_dict_set_item,
        moo_python_api_dict_del_item,
        moo_python_api_import_exec,
        moo_python_api_call_meth
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

    return TRUE;
}


inline static void
moo_python_api_deinit (void)
{
    moo_python_init (MOO_PY_API_VERSION, NULL);
}


#endif /* __MOO_PYTHON_API_H__ */
