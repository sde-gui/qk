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
#endif /* MOO_USE_PYGTK */
#include <glib.h>
#include "mooapp/moopython.h"
#include "mooapp/moopythonconsole.h"
#include "mooutils/moocompat.h"


static MooPython *instance = NULL;

MooPython  *moo_python_get_instance     (void)
{
    return instance;
}


static void     moo_python_class_init   (MooPythonClass *klass);
static void     moo_python_init         (MooPython      *python);
static GObject *moo_python_constructor  (GType           type,
                                         guint           n_construct_properties,
                                         GObjectConstructParam *construct_properties);
static void     moo_python_finalize     (GObject        *object);

static void     init_logger             (MooPython      *python);


/* MOO_TYPE_PYTHON */
G_DEFINE_TYPE (MooPython, moo_python, G_TYPE_OBJECT);


static void     moo_python_class_init   (MooPythonClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    gobject_class->constructor = moo_python_constructor;
    gobject_class->finalize = moo_python_finalize;
}


static void     moo_python_init         (MooPython      *python)
{
    g_assert (instance == NULL);
    instance = python;

    python->running = FALSE;
    python->log_in_func = NULL;
    python->log_out_func = NULL;
    python->log_err_func = NULL;
}


static GObject *moo_python_constructor  (GType                  type,
                                         guint                  n_props,
                                         GObjectConstructParam *props)
{
    GObject *object;
    MooPython *python;
    object = G_OBJECT_CLASS (moo_python_parent_class)->constructor (type, n_props, props);
    python = MOO_PYTHON (object);
    python->console = moo_python_console_new (python);
    return object;
}


static void moo_python_finalize         (GObject    *object)
{
    instance = NULL;
    G_OBJECT_CLASS (moo_python_parent_class)->finalize (object);
}


void        moo_python_start            (MooPython *python,
                                         int        argc,
                                         char     **argv)
{
    g_return_if_fail (MOO_IS_PYTHON (python));
    g_return_if_fail (!python->running);

    if (argc)
        Py_SetProgramName (argv[0]);

    Py_Initialize ();

    if (argc)
        PySys_SetArgv (argc, argv);

    python->main_mod = PyImport_AddModule ((char*)"__main__");
    Py_XINCREF ((PyObject*) python->main_mod);

    init_logger (python);

    python->running = TRUE;
}


void        moo_python_shutdown            (MooPython *python)
{
    g_return_if_fail (MOO_IS_PYTHON (python));
    g_return_if_fail (python->running);

    Py_XDECREF ((PyObject*) python->main_mod);
    /* TODO: Py_Finalize is crash if shutdown() was initiated from python,
               like with app.quit()
        It also aborts nobody knows why
    */
#if 0
    Py_Finalize ();
#endif

    moo_python_set_log_func (python, NULL, NULL, NULL, NULL);

    python->running = FALSE;
}


int         moo_python_run_simple_string    (MooPython *python,
                                             const char *cmd)
{
    g_return_val_if_fail (MOO_IS_PYTHON (python), -1);
    g_return_val_if_fail (cmd != NULL, -1);
    return PyRun_SimpleString (cmd);
}


int         moo_python_run_simple_file      (MooPython *python,
                                             gpointer    fp,
                                             const char *filename)
{
    g_return_val_if_fail (MOO_IS_PYTHON (python), -1);
    g_return_val_if_fail (fp != NULL && filename != NULL, -1);
    return PyRun_SimpleFile ((FILE*) fp, filename);
}


gpointer    moo_python_run_string           (MooPython *python,
                                             const char *str,
                                             gboolean    silent)
{
    PyObject *dict;
    g_return_val_if_fail (MOO_IS_PYTHON (python), NULL);
    g_return_val_if_fail (str != NULL, NULL);
    if (!silent && python->log_in_func)
        python->log_in_func (str, -1, python->log_data);
    dict = PyModule_GetDict ((PyObject*) python->main_mod);
    return PyRun_String (str, Py_file_input, dict, dict);
}


gpointer    moo_python_run_file             (MooPython *python,
                                             gpointer    fp,
                                             const char *filename)
{
    PyObject *dict;
    g_return_val_if_fail (MOO_IS_PYTHON (python), NULL);
    g_return_val_if_fail (fp != NULL && filename != NULL, NULL);
    dict = PyModule_GetDict ((PyObject*) python->main_mod);
    return PyRun_File ((FILE*) fp, filename, Py_file_input, dict, dict);
}


void        moo_python_set_log_func        (MooPython        *python,
                                             MooPythonLogFunc  in,
                                             MooPythonLogFunc  out,
                                             MooPythonLogFunc  err,
                                             gpointer           data)
{
    g_return_if_fail (MOO_IS_PYTHON (python));
    python->log_in_func = in;
    python->log_out_func = out;
    python->log_err_func = err;
    python->log_data = data;
}


void        moo_python_write_log           (MooPython *python,
                                             int         kind,
                                             const char *text,
                                             int         len)
{
    g_return_if_fail (MOO_IS_PYTHON (python));
    g_return_if_fail (kind == 2 || kind == 3);
    g_return_if_fail (text != NULL);

    if (kind == 2)
        if (python->log_out_func)
            python->log_out_func (text, len, python->log_data);

    if (kind == 3)
        if (python->log_err_func)
                python->log_err_func (text, len, python->log_data);
}


MooPython *moo_python_new                 (void)
{
    return MOO_PYTHON (g_object_new (MOO_TYPE_PYTHON, NULL));
}


#define WRITE_LOG "_write_log"

static PyObject *write_log (G_GNUC_UNUSED PyObject *self,
                            PyObject* args)
{
    int kind;
    const char *string;

    if (PyArg_ParseTuple(args, (char*)"is:" WRITE_LOG, &kind, &string))
    {
        moo_python_write_log (instance, kind, string, -1);
    }
    else
    {
        g_critical (WRITE_LOG ": incorrect parameters passed");
        PyErr_Clear ();
    }

    Py_INCREF(Py_None);
    return Py_None;
}


static void     init_logger             (MooPython      *python)
{
    PyObject *fun, *res;
    const char *script;
    static PyMethodDef meth =
    {(char*)WRITE_LOG, write_log, METH_VARARGS, (char*)WRITE_LOG};

    fun = PyCFunction_New (&meth, python->main_mod);
    g_return_if_fail (fun != NULL);

    if (PyModule_AddObject (python->main_mod, (char*)WRITE_LOG, fun))
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

    res = (PyObject*) moo_python_run_string (python, script, TRUE);

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
