/*
 *   mooui-mod.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#include <Python.h>
#define NO_IMPORT_PYGOBJECT
#include <pygobject.h>
#include <glib.h>
#include "moopython/pygtk/moo-pygtk.h"
#include "moopython/pygtk/mooui-mod.h"


static char *moo_ui_module_doc = "_moo_ui module.";


gboolean
_moo_ui_mod_init (void)
{
    PyObject *mod;

    mod = Py_InitModule3 ("_moo_ui", (PyMethodDef*) _moo_ui_functions, moo_ui_module_doc);
    PyImport_AddModule ("moo.ui");

    if (!mod)
        return FALSE;

    _moo_ui_register_classes (PyModule_GetDict (mod));

    if (!PyErr_Occurred ())
    {
        PyObject *fake_mod, *code;

        code = Py_CompileString (MOO_UI_PY, "moo/ui.py", Py_file_input);

        if (!code)
            return FALSE;

        fake_mod = PyImport_ExecCodeModule ("moo.ui", code);
        Py_DECREF (code);

        if (!fake_mod)
            PyErr_Print ();
    }

    return PyErr_Occurred () == NULL;
}
