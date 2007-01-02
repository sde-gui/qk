/*
 *   moopython-pygtkmod.h
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


#include "mooutils/mooutils-misc.h"


G_GNUC_UNUSED static void
init_pygtk_mod (void)
{
    PyObject *gobject, *pygtk;
    PyObject *mdict;
    PyObject *cobject;

    if (!(gobject = PyImport_ImportModule ((char*) "gobject")))
        return;

    mdict = PyModule_GetDict (gobject);
    cobject = PyDict_GetItemString (mdict, "_PyGObject_API");

    if (!cobject || !PyCObject_Check (cobject))
    {
        PyErr_SetString (PyExc_RuntimeError,
                         "could not find _PyGObject_API object");
        return;
    }

    _PyGObject_API = (struct _PyGObject_Functions *) PyCObject_AsVoidPtr (cobject);

    if (!(pygtk = PyImport_ImportModule((char*) "gtk._gtk")))
        return;

    mdict = PyModule_GetDict (pygtk);
    cobject = PyDict_GetItemString (mdict, "_PyGtk_API");

    if (!cobject || !PyCObject_Check (cobject))
    {
        PyErr_SetString (PyExc_RuntimeError,
                         "could not find _PyGtk_API object");
        return;
    }

    _PyGtk_API = (struct _PyGtk_FunctionStruct*) PyCObject_AsVoidPtr (cobject);
}


inline static gboolean
check_pygtk_version (const char *module,
                     int         req_major,
                     int         req_minor,
                     int         req_micro)
{
    PyObject *mod, *mdict, *version;
    int found_major, found_minor, found_micro;

    mod = PyImport_ImportModule ((char*) module);
    g_return_val_if_fail (mod != NULL, FALSE);

    mdict = PyModule_GetDict (mod);

    version = PyDict_GetItemString (mdict, "pygobject_version");

    if (!version)
        version = PyDict_GetItemString (mdict, "pygtk_version");

    if (!version)
        return FALSE;

    if (!PyArg_ParseTuple (version, (char*) "iii",
                           &found_major, &found_minor, &found_micro))
    {
        PyErr_Print ();
        return FALSE;
    }

    if (req_major != found_major ||
        req_minor > found_minor ||
        (req_minor == found_minor && req_micro > found_micro))
            return FALSE;

    return TRUE;
}


G_GNUC_UNUSED static void
reset_log_func (void)
{
#ifdef pyg_disable_warning_redirections
    if (check_pygtk_version ("gobject", 2, 12, 0))
        pyg_disable_warning_redirections ();
    else
        moo_reset_log_func ();
#else
    moo_reset_log_func ();
#endif
}
