/*
 *   moo-pygtk.c
 *
 *   Copyright (C) 2004-2008 by Yevgen Muntyan <muntyan@tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#include <Python.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "moo-mod.h"
#include "moopython/pygtk/moo-pygtk.h"
#include "moopython/moopython-utils.h"
#include <pygobject.h>  /* _PyGObjectAPI lives here */
#include <pygtk/pygtk.h>
#include <glib.h>
#include "moopython/moopython-pygtkmod.h"


static PyObject *
moo_version (void)
{
    return PyString_FromString (MOO_VERSION);
}

static PyObject *
moo_detailed_version (void)
{
    PyObject *res = PyDict_New ();
    g_return_val_if_fail (res != NULL, NULL);

    PyDict_SetItemString (res, "full", PyString_FromString (MOO_VERSION));
    PyDict_SetItemString (res, "major", PyInt_FromLong (MOO_MAJOR_VERSION));
    PyDict_SetItemString (res, "minor", PyInt_FromLong (MOO_MINOR_VERSION));
    PyDict_SetItemString (res, "micro", PyInt_FromLong (MOO_MICRO_VERSION));

    return res;
}


static PyMethodDef _moo_functions[] = {
    {NULL, NULL, 0, NULL}
};

static char *_moo_module_doc = "_moo module.";


static PyObject *
py_object_from_moo_py_object (const GValue *value)
{
    PyObject *obj;

    g_return_val_if_fail (G_VALUE_TYPE (value) == MOO_TYPE_PY_OBJECT, NULL);

    obj = g_value_get_boxed (value);

    if (!obj)
        obj = Py_None;

    return _moo_py_object_ref (obj);
}


static int
py_object_to_moo_py_object (GValue *value, PyObject *obj)
{
    g_value_set_boxed (value, obj == Py_None ? obj : NULL);
    return 0;
}

gboolean
_moo_pygtk_init (void)
{
    PyObject *_moo_module, *code, *moo_mod, *submod;

    init_pygtk_mod ();

    if (PyErr_Occurred ())
        return FALSE;

    pyg_register_boxed_custom (MOO_TYPE_PY_OBJECT,
                               py_object_from_moo_py_object,
                               py_object_to_moo_py_object);

    _moo_module = Py_InitModule3 ("_moo", _moo_functions, _moo_module_doc);

    if (PyErr_Occurred ())
        return FALSE;

    PyImport_AddModule ("moo");

    PyModule_AddObject (_moo_module, "version", moo_version());
    PyModule_AddObject (_moo_module, "detailed_version", moo_detailed_version());

#ifdef MOO_BUILD_UTILS
    if (!_moo_utils_mod_init ())
        return FALSE;
    submod = PyImport_ImportModule ("moo.utils");
    PyModule_AddObject (_moo_module, "utils", submod);
#endif
#ifdef MOO_BUILD_EDIT
    if (!_moo_edit_mod_init ())
        return FALSE;
    submod = PyImport_ImportModule ("moo.edit");
    PyModule_AddObject (_moo_module, "edit", submod);
#endif
#ifdef MOO_BUILD_APP
    if (!_moo_app_mod_init ())
        return FALSE;
    submod = PyImport_ImportModule ("moo.app");
    PyModule_AddObject (_moo_module, "app", submod);
#endif

    code = Py_CompileString (MOO_PY, "moo/__init__.py", Py_file_input);

    if (!code)
        return FALSE;

    moo_mod = PyImport_ExecCodeModule ("moo", code);
    Py_DECREF (code);

    if (!moo_mod)
        return FALSE;

    return TRUE;
}
