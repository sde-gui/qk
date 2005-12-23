/*
 *   moopython/moopython-utils.c
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
#include "moopython/moopython-utils.h"
#include "mooutils/moocompat.h"
#ifdef MOO_USE_PYGTK
# define NO_IMPORT_PYGOBJECT
# include "pygobject.h"
#endif


PyObject *
moo_strv_to_pyobject (char **strv)
{
    PyObject *result;
    guint len, i;

    if (!strv)
        return_None;

    len = g_strv_length (strv);
    result = PyTuple_New (len);

    for (i = 0; i < len; ++i)
    {
        PyTuple_SET_ITEM (result, i,
                          PyString_FromString (strv[i]));
    }

    return result;
}


static char **
moo_pyobject_to_strv_no_check (PyObject *seq,
                               int       len)
{
#define CACHE_SIZE 10
    static char **cache[CACHE_SIZE];
    static guint n;
    int i;
    char **ret;

    g_strfreev (cache[n]);

    cache[n] = ret = g_new (char*, len + 1);
    ret[len] = NULL;

    for (i = 0; i < len; ++i)
    {
        PyObject *item = PySequence_ITEM (seq, i);
        ret[i] = g_strdup (PyString_AS_STRING (item));
    }

    if (++n == CACHE_SIZE)
        n = 0;

    return ret;
#undef CACHE_SIZE
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

    *dest = moo_pyobject_to_strv_no_check (obj, len);

    return TRUE;
}


int
moo_pyobject_to_strv_no_null (PyObject  *obj,
                              char    ***dest)
{
    if (obj == Py_None)
    {
        PyErr_SetString (PyExc_TypeError,
                         "argument must be a sequence, not None");
        return FALSE;
    }

    return moo_pyobject_to_strv (obj, dest);
}


PyObject *moo_gvalue_to_pyobject (const GValue *val)
{
    return pyg_value_as_pyobject (val, TRUE);
}


typedef PyObject *(*PtrToPy) (gpointer ptr);

static PyObject*
slist_to_pyobject (GSList *list,
                   PtrToPy func)
{
    int i;
    GSList *l;
    PyObject *result;

    result = PyList_New (g_slist_length (list));

    for (i = 0, l = list; l != NULL; l = l->next, ++i)
    {
        PyObject *item = func (l->data);

        if (!item)
        {
            Py_DECREF (result);
            return NULL;
        }

        PyList_SetItem (result, i, item);
    }

    return result;
}


PyObject*
moo_object_slist_to_pyobject (GSList *list)
{
    return slist_to_pyobject (list, (PtrToPy) pygobject_new);
}


static PyObject*
string_to_pyobject (gpointer str)
{
    if (!str)
        return_RuntimeErr ("got NULL string");
    else
        return PyString_FromString (str);
}

PyObject*
moo_string_slist_to_pyobject (GSList *list)
{
    return slist_to_pyobject (list, string_to_pyobject);
}


PyObject*
moo_py_object_ref (PyObject *obj)
{
    Py_XINCREF (obj);
    return obj;
}


void
moo_py_object_unref (PyObject *obj)
{
    Py_XDECREF (obj);
}


GType
moo_py_object_get_type (void)
{
    static GType type = 0;

    if (!type)
        type = g_boxed_type_register_static ("MooPyObject",
                                             (GBoxedCopyFunc) moo_py_object_ref,
                                             (GBoxedFreeFunc) moo_py_object_unref);

    return type;
}
