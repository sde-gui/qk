/*
 *   moopython/moopython-utils.c
 *
 *   Copyright (C) 2004-2006 by Yevgen Muntyan <muntyan@math.tamu.edu>
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
#include "mooutils/mooutils-misc.h"
#define NO_IMPORT_PYGOBJECT
#include "pygobject.h"


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


char *
moo_py_err_string (void)
{
    PyObject *exc, *value, *tb;
    PyObject *str_exc, *str_value, *str_tb;
    GString *string;

    PyErr_Fetch (&exc, &value, &tb);
    PyErr_NormalizeException (&exc, &value, &tb);

    if (!exc)
        return NULL;

    string = g_string_new (NULL);

    str_exc = PyObject_Str (exc);
    str_value = PyObject_Str (value);
    str_tb = PyObject_Str (tb);

    if (str_exc)
        g_string_append_printf (string, "%s\n", PyString_AS_STRING (str_exc));
    if (str_value)
        g_string_append_printf (string, "%s\n", PyString_AS_STRING (str_value));
    if (str_tb)
        g_string_append_printf (string, "%s\n", PyString_AS_STRING (str_tb));

    Py_XDECREF(exc);
    Py_XDECREF(value);
    Py_XDECREF(tb);
    Py_XDECREF(str_exc);
    Py_XDECREF(str_value);
    Py_XDECREF(str_tb);

    return g_string_free (string, FALSE);
}


/***********************************************************************/
/* File-like object for sys.stdout and sys.stderr
 */

typedef struct {
    PyObject_HEAD
    GPrintFunc write_func;
} MooPyFile;


// static int
// moo_py_file_new (PyObject *self, PyObject *args, PyObject *kwargs)
// {
//     if (!PyArg_ParseTuple (args, (char*) ""))
//         return -1;
//
//     return 0;
// }


static PyObject *
moo_py_file_close (G_GNUC_UNUSED PyObject *self)
{
    return_None;
}


static PyObject *
moo_py_file_flush (G_GNUC_UNUSED PyObject *self)
{
    return_None;
}


static PyObject *
moo_py_file_write (PyObject *self, PyObject *args)
{
    char *string;
    MooPyFile *file = (MooPyFile *) self;

    if (!PyArg_ParseTuple (args, (char*) "s", &string))
        return NULL;

    if (!file->write_func)
        return_RuntimeErr ("no write function installed");

    file->write_func (string);
    return_None;
}


static PyMethodDef MooPyFile_methods[] = {
    { (char*) "close", (PyCFunction) moo_py_file_close, METH_NOARGS, NULL },
    { (char*) "flush", (PyCFunction) moo_py_file_flush, METH_NOARGS, NULL },
    { (char*) "write", (PyCFunction) moo_py_file_write, METH_VARARGS, NULL },
    { NULL, NULL, 0, NULL }
};


static PyTypeObject MooPyFile_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                  /* ob_size */
    (char*) "MooPyFile",                /* tp_name */
    sizeof (MooPyFile),                 /* tp_basicsize */
    0,                                  /* tp_itemsize */
    /* methods */
    (destructor) 0,                     /* tp_dealloc */
    (printfunc) 0,                      /* tp_print */
    (getattrfunc) 0,                    /* tp_getattr */
    (setattrfunc) 0,                    /* tp_setattr */
    (cmpfunc) 0,                        /* tp_compare */
    (reprfunc) 0,                       /* tp_repr */
    (PyNumberMethods*) 0,               /* tp_as_number */
    (PySequenceMethods*) 0,             /* tp_as_sequence */
    (PyMappingMethods*) 0,              /* tp_as_mapping */
    (hashfunc) 0,                       /* tp_hash */
    (ternaryfunc) 0,                    /* tp_call */
    (reprfunc) 0,                       /* tp_str */
    (getattrofunc) 0,                   /* tp_getattro */
    (setattrofunc) 0,                   /* tp_setattro */
    (PyBufferProcs*) 0,                 /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    NULL,                               /* Documentation string */
    (traverseproc) 0,                   /* tp_traverse */
    (inquiry) 0,                        /* tp_clear */
    (richcmpfunc) 0,                    /* tp_richcompare */
    0,                                  /* tp_weaklistoffset */
    (getiterfunc) 0,                    /* tp_iter */
    (iternextfunc) 0,                   /* tp_iternext */
    MooPyFile_methods,                  /* tp_methods */
    0,                                  /* tp_members */
    0,                                  /* tp_getset */
    NULL,                               /* tp_base */
    NULL,                               /* tp_dict */
    (descrgetfunc) 0,                   /* tp_descr_get */
    (descrsetfunc) 0,                   /* tp_descr_set */
    0,                                  /* tp_dictoffset */
//     (initproc) moo_py_file_new,         /* tp_init */
    (initproc) 0,         /* tp_init */
    (allocfunc) 0,                      /* tp_alloc */
    (newfunc) 0,                        /* tp_new */
    (freefunc) 0,                       /* tp_free */
    (inquiry) 0,                        /* tp_is_gc */
    NULL, NULL, NULL, NULL, NULL, NULL
};


static void
set_file (const char *name,
          GPrintFunc  func)
{
    MooPyFile *file;

    file = PyObject_New (MooPyFile, &MooPyFile_Type);

    if (!file)
    {
        PyErr_Print ();
        return;
    }

    file->write_func = func;

    if (PySys_SetObject ((char*) name, (PyObject*) file))
        PyErr_Print ();

    Py_DECREF (file);
}


void
moo_py_init_print_funcs (void)
{
    static gboolean done;

    if (done)
        return;

    done = TRUE;

    if (PyType_Ready (&MooPyFile_Type) < 0)
    {
        g_critical ("could not init MooPyFile type");
        return;
    }

    set_file ("stdout", moo_print);
    set_file ("stderr", moo_print_err);
}
