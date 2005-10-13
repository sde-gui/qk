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


void        initmoo             (void);
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


static PyMethodDef moo_functions[] = {
    {NULL, NULL, 0, NULL}
};

static char *moo_module_doc = (char*)"moo module.";


#define mod_init(class)                         \
{                                               \
    class##_mod_init (moo_module);              \
    if (PyErr_Occurred ())                      \
    {                                           \
        PyErr_Print ();                         \
        PyErr_SetString (PyExc_RuntimeError,    \
                         "error in " #class     \
                         "_mod_init");          \
        return;                                 \
    }                                           \
}

void        initmoo                     (void)
{
    PyObject *moo_module;

    init_pygobject ();
    init_pygtk ();

    moo_module = Py_InitModule3 ((char*)"moo", moo_functions, moo_module_doc);

    if (PyErr_Occurred ()) {
        PyErr_Print ();
        PyErr_SetString (PyExc_RuntimeError, "could not create 'moo' module");
        return;
    }

    PyModule_AddObject (moo_module, (char*)"version", moo_version());
    PyModule_AddObject (moo_module, (char*)"detailed_version", moo_detailed_version());

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
}
