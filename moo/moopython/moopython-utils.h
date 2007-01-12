/*
 *   moopython/moopython-utils.h
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

#ifndef __MOO_PYTHON_UTILS_H__
#define __MOO_PYTHON_UTILS_H__

#include <Python.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define MOO_TYPE_PY_OBJECT (_moo_py_object_get_type())

PyObject    *_moo_py_object_ref             (PyObject       *obj);
void         _moo_py_object_unref           (PyObject       *obj);
GType        _moo_py_object_get_type        (void) G_GNUC_CONST;

PyObject    *_moo_strv_to_pyobject          (char          **strv);

/* result may not be freed */
int          _moo_pyobject_to_strv          (PyObject       *obj,
                                             char         ***dest);
int          _moo_pyobject_to_strv_no_null  (PyObject       *obj,
                                             char         ***dest);

PyObject    *_moo_object_slist_to_pyobject  (GSList         *list);
PyObject    *_moo_string_slist_to_pyobject  (GSList         *list);
PyObject    *_moo_boxed_slist_to_pyobject   (GSList         *list,
                                             GType           type);

PyObject    *_moo_gvalue_to_pyobject        (const GValue   *val);
void         _moo_pyobject_to_gvalue        (PyObject       *object,
                                             GValue         *value);

char        *_moo_py_err_string             (void);
void         _moo_py_init_print_funcs       (void);


#define return_None     return Py_INCREF(Py_None), (Py_None)
#define return_Self     return Py_INCREF(self), (self)

#define return_True     return Py_INCREF(Py_True), (Py_True)
#define return_False    return Py_INCREF(Py_False), (Py_False)

#define return_Int(v)   return Py_BuildValue ((char*)"i", (v))

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

#define return_AttrError(msg)       return PyErr_SetString (PyExc_AttributeError, msg), NULL
#define return_AttrErrorInt(msg)    return PyErr_SetString (PyExc_AttributeError, msg), -1
#define return_TypeError(msg)       return PyErr_SetString (PyExc_TypeError, msg), NULL
#define return_TypeErrorInt(msg)    return PyErr_SetString (PyExc_TypeError, msg), -1
#define return_RuntimeError(msg)    return PyErr_SetString (PyExc_RuntimeError, msg), NULL
#define return_RuntimeErrorInt(msg) return PyErr_SetString (PyExc_RuntimeError, msg), -1
#define return_ValueError(msg)      return PyErr_SetString (PyExc_ValueError, msg), NULL
#define return_ValueErrorInt(msg)   return PyErr_SetString (PyExc_ValueError, msg), -1


G_END_DECLS

#endif /* __MOO_PYTHON_UTILS_H__ */
