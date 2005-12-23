/*
 *   moopython/mooutils-mod.c
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
#include "moopython/pygtk/moo-pygtk.h"
#include "moopython/pygtk/mooutils-mod.h"
#include "moopython/moopython-utils.h"
#include "mooutils/moostock.h"


static char *moo_utils_module_doc = (char*)"_moo_utils module.";

#define add_constant(mod_,name_,val_) PyModule_AddStringConstant (mod, (char*) name_, (char*) val_)


static PyObject *pyobj_from_gval (const GValue *value)
{
    if (!G_VALUE_HOLDS (value, MOO_TYPE_PY_OBJECT))
        return_RuntimeErr ("invalid value passed");
    return moo_py_object_ref (g_value_get_boxed (value));
}


static int gval_from_pyobj (GValue *value, PyObject *obj)
{
    if (!G_VALUE_HOLDS (value, MOO_TYPE_PY_OBJECT))
        return_RuntimeErrInt ("invalid value passed");
    g_value_set_boxed (value, obj);
    return 0;
}


gboolean
_moo_utils_mod_init (void)
{
    PyObject *mod;

    pyg_register_boxed_custom (MOO_TYPE_PY_OBJECT, pyobj_from_gval, gval_from_pyobj);

    mod = Py_InitModule3 ((char*) "_moo_utils", _moo_utils_functions, moo_utils_module_doc);

    if (!mod)
        return FALSE;

    _moo_utils_add_constants (mod, "MOO_");
    _moo_utils_register_classes (PyModule_GetDict (mod));

    if (PyErr_Occurred ())
        return FALSE;

    add_constant (mod, "STOCK_APP", MOO_STOCK_APP);
    add_constant (mod, "STOCK_GAP", MOO_STOCK_GAP);
    add_constant (mod, "STOCK_TERMINAL", MOO_STOCK_TERMINAL);
    add_constant (mod, "STOCK_KEYBOARD", MOO_STOCK_KEYBOARD);
    add_constant (mod, "STOCK_RESTART", MOO_STOCK_RESTART);
    add_constant (mod, "STOCK_CLOSE", MOO_STOCK_CLOSE);
    add_constant (mod, "STOCK_STICKY", MOO_STOCK_STICKY);
    add_constant (mod, "STOCK_DETACH", MOO_STOCK_DETACH);
    add_constant (mod, "STOCK_ATTACH", MOO_STOCK_ATTACH);
    add_constant (mod, "STOCK_KEEP_ON_TOP", MOO_STOCK_KEEP_ON_TOP);
    add_constant (mod, "STOCK_DOC_DELETED", MOO_STOCK_DOC_DELETED);
    add_constant (mod, "STOCK_DOC_MODIFIED_ON_DISK", MOO_STOCK_DOC_MODIFIED_ON_DISK);
    add_constant (mod, "STOCK_DOC_DELETED", MOO_STOCK_DOC_DELETED);
    add_constant (mod, "STOCK_DOC_MODIFIED", MOO_STOCK_DOC_MODIFIED);
    add_constant (mod, "STOCK_FILE_SELECTOR", MOO_STOCK_FILE_SELECTOR);
    add_constant (mod, "STOCK_SAVE_NONE", MOO_STOCK_SAVE_NONE);
    add_constant (mod, "STOCK_SAVE_SELECTED", MOO_STOCK_SAVE_SELECTED);
    add_constant (mod, "STOCK_NEW_PROJECT", MOO_STOCK_NEW_PROJECT);
    add_constant (mod, "STOCK_OPEN_PROJECT", MOO_STOCK_OPEN_PROJECT);
    add_constant (mod, "STOCK_CLOSE_PROJECT", MOO_STOCK_CLOSE_PROJECT);
    add_constant (mod, "STOCK_PROJECT_OPTIONS", MOO_STOCK_PROJECT_OPTIONS);
    add_constant (mod, "STOCK_BUILD", MOO_STOCK_BUILD);
    add_constant (mod, "STOCK_COMPILE", MOO_STOCK_COMPILE);
    add_constant (mod, "STOCK_EXECUTE", MOO_STOCK_EXECUTE);
    add_constant (mod, "STOCK_FIND_IN_FILES", MOO_STOCK_FIND_IN_FILES);
    add_constant (mod, "STOCK_FIND_FILE", MOO_STOCK_FIND_FILE);
    add_constant (mod, "STOCK_PLUGINS", MOO_STOCK_PLUGINS);

    if (!PyErr_Occurred ())
    {
        PyObject *fake_mod, *code;

        code = Py_CompileString (MOO_UTILS_PY, "moo/utils.py", Py_file_input);

        if (!code)
            return FALSE;

        fake_mod = PyImport_ExecCodeModule ((char*) "moo_utils", code);
        Py_DECREF (code);

        if (!fake_mod)
            PyErr_Print ();
    }

    return PyErr_Occurred () == NULL;
}
