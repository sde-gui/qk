/*
 *   mooutils/mooutils-python.c
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
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include "mooutils/mooutils-python.h"
#ifdef USE_PYGTK
# define NO_IMPORT_PYGOBJECT
# include "pygobject.h"
#endif


PyObject *moo_strv_to_pyobject (char **strv)
{
    PyObject *result;
    guint len, i;

    if (!strv)
    {
        return_None;
    }

    len = g_strv_length (strv);
    result = PyTuple_New (len);

    for (i = 0; i < len; ++i)
    {
        PyTuple_SET_ITEM (result, i,
                          PyString_FromString (strv[i]));
    }

    return result;
}


int moo_pyobject_to_strv (PyObject *obj, char ***dest)
{
    int len, i;

    if (obj == Py_None)
    {
        *dest = NULL;
        return TRUE;
    }

    if (!PySequence_Check (obj))
    {
        PyErr_SetString (PyExc_TypeError,
                         "argument must be a sequence");
        return FALSE;
    }

    len = PySequence_Size (obj);

    if (len < 0)
    {
        PyErr_SetString (PyExc_RuntimeError,
                         "got negative length of a sequence");
        return FALSE;
    }

    for (i = 0; i < len; ++i)
    {
        PyObject *item = PySequence_ITEM (obj, i);

        g_return_val_if_fail (item != NULL, FALSE);

        if (!PyString_Check (item))
        {
            PyErr_SetString (PyExc_TypeError,
                             "argument must be a sequence of strings");
            return FALSE;
        }
    }

    *dest = g_new (char*, len + 1);
    (*dest)[len] = NULL;

    for (i = 0; i < len; ++i)
    {
        PyObject *item = PySequence_ITEM (obj, i);
        (*dest)[i] = g_strdup (PyString_AS_STRING (item));
    }

    return TRUE;
}


PyObject *moo_gvalue_to_pyobject (const GValue *val)
{
    return pyg_value_as_pyobject (val, TRUE);
}
