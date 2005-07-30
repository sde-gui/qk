/*
 *   mooui/mooui-mod.c
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


void        moo_ui_mod_init         (PyObject   *moo_mod);
void        moo_ui_register_classes (PyObject   *dict);
void        moo_ui_add_constants    (PyObject   *module,
                                     const gchar *strip_prefix);


extern PyMethodDef moo_ui_functions[];

static char *moo_ui_module_doc = (char*)"moo.ui module.";


void        moo_ui_mod_init         (PyObject   *moo_mod)
{
    PyObject *mod;

    mod = Py_InitModule3 ("moo.ui", moo_ui_functions, moo_ui_module_doc);
    g_return_if_fail (mod != NULL);
    Py_INCREF (mod);
    PyModule_AddObject (moo_mod, "ui", mod);
    /* moo_ui_add_constants (mod, "MOO_"); */

    moo_ui_register_classes (PyModule_GetDict (moo_mod));
}
