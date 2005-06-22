/*
 *   mooapp/pyutils.h
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

#ifndef PYTHON_PYUTILS_H
#define PYTHON_PYUTILS_H


#define return_None     {Py_INCREF(Py_None); return Py_None;}
#define return_Self     {Py_INCREF((PyObject*)self); return (PyObject*)self;}

#define return_True     {Py_INCREF(Py_True); return Py_True;}
#define return_False    {Py_INCREF(Py_False); return Py_False;}

#define return_Bool(v)  { \
                            if ((v)) { \
                                Py_INCREF(Py_True); \
                                return Py_True; \
                            } else { \
                                Py_INCREF(Py_False); \
                                return Py_False; \
                            } \
                        }

#define return_Int(v)   {return Py_BuildValue ((char*)"i", (v));}


#define return_AttrErr(msg) {PyErr_SetString(PyExc_AttributeError, msg); return NULL;}
#define return_AttrErrInt(msg) {PyErr_SetString(PyExc_AttributeError, msg); return -1;}
#define return_TypeErr(msg) {PyErr_SetString(PyExc_TypeError, msg); return NULL;}
#define return_TypeErrInt(msg) {PyErr_SetString(PyExc_TypeError, msg); return -1;}
#define return_RuntimeErr(msg) {PyErr_SetString(PyExc_RuntimeError, msg); return NULL;}
#define return_RuntimeErrInt(msg) {PyErr_SetString(PyExc_RuntimeError, msg); return -1;}


#endif /* PYTHON_PYUTILS_H */
