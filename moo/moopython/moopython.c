/*
 *   mooapp/moopython.c
 *
 *   Copyright (C) 2004-2005 by Yevgen Muntyan <muntyan@math.tamu.edu>
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
#ifdef MOO_USE_PYGTK
#define NO_IMPORT_PYGOBJECT
#include <pygobject.h>
#endif
#include <glib.h>
#include <stdio.h>
#include "moopython/moopython.h"
#include "mooutils/moocompat.h"


static gboolean running;
MooPyAPI *_moo_py_api = NULL;
static MooPyAPI moo_py_api;
static PyObject *main_mod;
static MooPythonLogFunc log_in_func;
static MooPythonLogFunc log_out_func;
static MooPythonLogFunc log_err_func;
static gpointer log_func_data;


gboolean
moo_python_running (void)
{
    return running;
}


void
moo_python_shutdown (void)
{
    g_return_if_fail (running);

    Py_XDECREF (main_mod);
    _moo_py_api = NULL;
    running = FALSE;
}


static MooPyObject*
run_string (const char *str,
            gboolean    silent)
{
    PyObject *dict;

    g_return_val_if_fail (str != NULL, NULL);

    if (!silent && log_in_func)
        log_in_func (str, -1, log_func_data);

    dict = PyModule_GetDict (main_mod);
    return (MooPyObject*) PyRun_String (str, Py_file_input, dict, dict);
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


static void
set_log_func (MooPythonLogFunc  in,
              MooPythonLogFunc  out,
              MooPythonLogFunc  err,
              gpointer           data)
{
    log_in_func = in;
    log_out_func = out;
    log_err_func = err;
    log_func_data = data;
}


static void
write_log (int         kind,
           const char *text,
           int         len)
{
    char *freeme = NULL;
    const char *write_text;

    g_return_if_fail (kind == 2 || kind == 3);
    g_return_if_fail (text != NULL);

    if (!len)
        return;

    if (len > 0)
    {
        freeme = g_strndup (text, len);
        write_text = freeme;
    }
    else
    {
        write_text = text;
    }

    if (kind == 2)
    {
        if (log_out_func)
            log_out_func (write_text, -1, log_func_data);
        else
            fprintf (stdout, "%s", write_text);
    }

    if (kind == 3)
    {
        if (log_err_func)
            log_err_func (write_text, -1, log_func_data);
        else
            fprintf (stderr, "%s", write_text);
    }

    g_free (freeme);
}


#define WRITE_LOG "_write_log"

static PyObject*
write_log_meth (G_GNUC_UNUSED PyObject *self,
                PyObject* args)
{
    int kind;
    const char *string;

    if (PyArg_ParseTuple(args, (char*)"is:" WRITE_LOG, &kind, &string))
    {
        write_log (kind, string, -1);
    }
    else
    {
        g_critical (WRITE_LOG ": incorrect parameters passed");
        PyErr_Clear ();
    }

    Py_INCREF(Py_None);
    return Py_None;
}


static void
init_logger (void)
{
    PyObject *fun, *res;
    const char *script;
    static PyMethodDef meth = {(char*) WRITE_LOG, write_log_meth, METH_VARARGS, (char*) WRITE_LOG};

    fun = PyCFunction_New (&meth, main_mod);
    g_return_if_fail (fun != NULL);

    if (PyModule_AddObject (main_mod, (char*) WRITE_LOG, fun))
    {
        Py_DECREF (fun);
        return;
    }

    script =
        "import sys\n"
        "class __Logger:\n"
        "   def __init__ (self, t):\n"
        "       self.kind = t\n"
        "   def write (self, data):\n"
        "       " WRITE_LOG " (self.kind, data)\n"
        "sys.stdout = __Logger (2)\n"
        "sys.stderr = __Logger (3)\n";

    res = (PyObject*) run_string (script, TRUE);

    if (res)
    {
        Py_DECREF (res);
    }
    else
    {
        g_critical ("%s: error in Logger initialization", G_STRLOC);
        g_critical ("%s", script);
        PyErr_Print ();
    }
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

static void
err_print (void)
{
    PyErr_Print ();
}


gboolean
moo_python_start (int        argc,
                  char     **argv)
{
    g_return_val_if_fail (!running, FALSE);

    if (argc)
        Py_SetProgramName (argv[0]);

    Py_Initialize ();

    if (argc)
        PySys_SetArgv (argc, argv);

    main_mod = PyImport_AddModule ((char*)"__main__");
    Py_XINCREF ((PyObject*) main_mod);

    _moo_py_api = &moo_py_api;

    moo_py_api.incref = incref;
    moo_py_api.decref = decref;
    moo_py_api.err_print = err_print;
    moo_py_api.set_log_func = set_log_func;
    moo_py_api.run_string = run_string;
    moo_py_api.run_file = run_file;

    init_logger ();

    running = TRUE;
    return TRUE;
}
