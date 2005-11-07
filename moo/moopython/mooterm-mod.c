/*
 *   moopython/mooterm-mod.c
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

#include <Python.h>
#define NO_IMPORT_PYGOBJECT
#include <pygobject.h>
#include <glib.h>
#include "moopython/moo-pygtk.h"
#include "moopython/mooterm-mod.h"


static char *moo_term_module_doc = (char*)"_moo_term module.";


gboolean
_moo_term_mod_init (void)
{
    PyObject *mod;

    mod = Py_InitModule3 ((char*) "_moo_term", _moo_term_functions, moo_term_module_doc);

    if (!mod)
        return FALSE;

//     _moo_term_add_constants (mod, "MOO_");
    _moo_term_register_classes (PyModule_GetDict (mod));

    if (!PyErr_Occurred ())
    {
        PyObject *fake_mod, *code;

        code = Py_CompileString (MOO_TERM_PY, "moo/term.py", Py_file_input);

        if (!code)
            return FALSE;

        fake_mod = PyImport_ExecCodeModule ((char*) "moo_term", code);
        Py_DECREF (code);

        if (!fake_mod)
            PyErr_Print ();
    }

    return PyErr_Occurred () == NULL;
}
