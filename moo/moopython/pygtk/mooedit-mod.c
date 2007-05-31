/*
 *   mooedit-mod.c
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

#include <Python.h>
#define NO_IMPORT_PYGOBJECT
#include <pygobject.h>
#include <glib.h>
#include "moopython/pygtk/moo-pygtk.h"
#include "moopython/pygtk/mooedit-mod.h"
#include "mooedit/mooplugin.h"


static char *moo_edit_module_doc = "_moo_edit module.";


gboolean
_moo_edit_mod_init (void)
{
    PyObject *mod;

    mod = Py_InitModule3 ("_moo_edit", _moo_edit_functions, moo_edit_module_doc);
    PyImport_AddModule ("moo.edit");

    if (!mod)
        return FALSE;

    _moo_edit_add_constants (mod, "MOO_");
    _moo_edit_register_classes (PyModule_GetDict (mod));

    if (!PyErr_Occurred ())
    {
        PyObject *fake_mod, *code;

        code = Py_CompileString (MOO_EDIT_PY, "moo/edit.py", Py_file_input);

        if (!code)
            return FALSE;

        fake_mod = PyImport_ExecCodeModule ("moo.edit", code);
        Py_DECREF (code);

        if (!fake_mod)
            PyErr_Print ();
    }

    return !PyErr_Occurred ();
}
