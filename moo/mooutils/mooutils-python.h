/*
 *   mooutils/mooutils-python.h
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

#ifndef MOOUTILS_MOOUTILS_PYTHON_H
#define MOOUTILS_MOOUTILS_PYTHON_H

#include <Python.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define MOO_TYPE_PY_OBJECT (moo_py_object_get_type())

PyObject    *moo_py_object_ref              (PyObject       *obj);
void         moo_py_object_unref            (PyObject       *obj);
GType        moo_py_object_get_type         (void) G_GNUC_CONST;


PyObject    *moo_strv_to_pyobject           (char          **strv);
int          moo_pyobject_to_strv           (PyObject       *obj,
                                             char         ***dest);

PyObject    *moo_object_slist_to_pyobject   (GSList         *list);
PyObject    *moo_string_slist_to_pyobject   (GSList         *list);

PyObject    *moo_gvalue_to_pyobject         (const GValue   *val);


#define return_None     G_STMT_START {Py_INCREF(Py_None); return Py_None;} G_STMT_END
#define return_Self     G_STMT_START {Py_INCREF((PyObject*)self); return (PyObject*)self;} G_STMT_END

#define return_True     G_STMT_START {Py_INCREF(Py_True); return Py_True;} G_STMT_END
#define return_False    G_STMT_START {Py_INCREF(Py_False); return Py_False;} G_STMT_END

#define return_Bool(v)          \
G_STMT_START {                  \
    if ((v))                    \
    {                           \
        Py_INCREF(Py_True);     \
        return Py_True;         \
    } else                      \
    {                           \
        Py_INCREF(Py_False);    \
        return Py_False;        \
    }                           \
} G_STMT_END

#define return_Int(v)   return Py_BuildValue ((char*)"i", (v))


#define return_AttrErr(msg)         G_STMT_START {PyErr_SetString(PyExc_AttributeError, msg); return NULL;} G_STMT_END
#define return_AttrErrInt(msg)      G_STMT_START {PyErr_SetString(PyExc_AttributeError, msg); return -1;} G_STMT_END
#define return_TypeErr(msg)         G_STMT_START {PyErr_SetString(PyExc_TypeError, msg); return NULL;} G_STMT_END
#define return_TypeErrInt(msg)      G_STMT_START {PyErr_SetString(PyExc_TypeError, msg); return -1;} G_STMT_END
#define return_RuntimeErr(msg)      G_STMT_START {PyErr_SetString(PyExc_RuntimeError, msg); return NULL;} G_STMT_END
#define return_RuntimeErrInt(msg)   G_STMT_START {PyErr_SetString(PyExc_RuntimeError, msg); return -1;} G_STMT_END


G_END_DECLS

#endif /* MOOUTILS_MOOUTILS_PYTHON_H */
