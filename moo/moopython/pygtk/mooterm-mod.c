/*
 *   mooterm-mod.c
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
#include "moopython/pygtk/mooterm-mod.h"
#include "moopython/moopython-utils.h"


static char *moo_term_module_doc = "_moo_term module.";


gboolean
_moo_term_mod_init (void)
{
    PyObject *mod;

    mod = Py_InitModule3 ("_moo_term", (PyMethodDef*) _moo_term_functions, moo_term_module_doc);
    PyImport_AddModule ("moo.term");

    if (!mod)
        return FALSE;

    _moo_term_add_constants (mod, "MOO_TERM_");
    _moo_term_register_classes (PyModule_GetDict (mod));

    if (!PyErr_Occurred ())
    {
        PyObject *fake_mod, *code;

        code = Py_CompileString (MOO_TERM_PY, "moo/term.py", Py_file_input);

        if (!code)
            return FALSE;

        fake_mod = PyImport_ExecCodeModule ("moo.term", code);
        Py_DECREF (code);

        if (!fake_mod)
            PyErr_Print ();
    }

    return PyErr_Occurred () == NULL;
}
