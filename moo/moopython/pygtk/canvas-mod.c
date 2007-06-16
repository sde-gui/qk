/*
 *   canvas-mod.c
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
#include "moopython/pygtk/canvas-mod.h"

void _moo_initgoocanvas (void);

gboolean
_moo_canvas_mod_init (void)
{
    PyObject *mod;

    _moo_initgoocanvas ();

    if (PyErr_Occurred ())
        return FALSE;

    PyImport_AddModule ("moo.canvas");

    if (!PyErr_Occurred ())
    {
        PyObject *fake_mod, *code;

        code = Py_CompileString (MOO_CANVAS_PY, "moo/canvas.py", Py_file_input);

        if (!code)
            return FALSE;

        fake_mod = PyImport_ExecCodeModule ("moo.canvas", code);
        Py_DECREF (code);

        if (!fake_mod)
            PyErr_Print ();
    }

    return PyErr_Occurred () == NULL;
}
