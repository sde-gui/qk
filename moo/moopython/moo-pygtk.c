/*
 *   moo/moo-pygtk.c
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
#include <pygobject.h>  /* _PyGObjectAPI lives here */
#include <pygtk/pygtk.h>
#include <glib.h>
#include "moopython/moo-mod.h"


gboolean    initmoo             (void);
void        moo_utils_mod_init  (PyObject   *moo_mod);
void        moo_edit_mod_init   (PyObject   *moo_mod);
void        moo_term_mod_init   (PyObject   *moo_mod);
void        moo_app_mod_init    (PyObject   *moo_mod);


static PyObject *moo_version (void)
{
    return PyString_FromString (MOO_VERSION);
}

static PyObject *moo_detailed_version (void)
{
    PyObject *res = PyDict_New ();
    g_return_val_if_fail (res != NULL, NULL);

    PyDict_SetItemString (res, "full", PyString_FromString (MOO_VERSION));
    PyDict_SetItemString (res, "api", PyString_FromString (MOO_API_VERSION));
    PyDict_SetItemString (res, "major", PyInt_FromLong (MOO_VERSION_MAJOR));
    PyDict_SetItemString (res, "minor", PyInt_FromLong (MOO_VERSION_MINOR));
    PyDict_SetItemString (res, "micro", PyInt_FromLong (MOO_VERSION_MICRO));

    return res;
}


static PyMethodDef _moo_functions[] = {
    {NULL, NULL, 0, NULL}
};

static char *_moo_module_doc = (char*)"_moo module.";


#define mod_init(class)                         \
{                                               \
    class##_mod_init (_moo_module);             \
    if (PyErr_Occurred ())                      \
        return FALSE;                           \
}

static void
func_init_pygobject (void)
{
    init_pygobject ();
    init_pygtk ();
}

gboolean
initmoo (void)
{
    PyObject *_moo_module, *code, *moo_mod;

    func_init_pygobject ();

    if (PyErr_Occurred ())
        return FALSE;

    _moo_module = Py_InitModule3 ((char*) "_moo", _moo_functions, _moo_module_doc);

    if (PyErr_Occurred ())
        return FALSE;

    PyModule_AddObject (_moo_module, (char*)"version", moo_version());
    PyModule_AddObject (_moo_module, (char*)"detailed_version", moo_detailed_version());

#ifdef MOO_BUILD_UTILS
    mod_init (moo_utils);
#endif
#ifdef MOO_BUILD_EDIT
    mod_init (moo_edit);
#endif
#ifdef MOO_BUILD_TERM
    mod_init (moo_term);
#endif
#ifdef MOO_BUILD_APP
    mod_init (moo_app);
#endif

    code = Py_CompileString (MOO_PY, "moo.py", Py_file_input);

    if (!code)
        return FALSE;

    moo_mod = PyImport_ExecCodeModule ((char*) "moo", code);
    Py_DECREF (code);

    if (!moo_mod)
        return FALSE;

    return TRUE;
}
