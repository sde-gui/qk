/*
 *   moopython/moopython.h
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

#ifndef __MOO_PYTHON_H__
#define __MOO_PYTHON_H__

#include <glib.h>

G_BEGIN_DECLS

typedef struct _MooPyAPI MooPyAPI;
typedef struct _MooPyObject MooPyObject;


struct _MooPyAPI {
    MooPyObject* (*incref)        (MooPyObject *obj);
    void         (*decref)        (MooPyObject *obj);
    void         (*err_print)     (void);

    MooPyObject* (*run_string)    (const char  *str);
    MooPyObject* (*run_file)      (void        *fp,
                                   const char  *filename);
};


extern MooPyAPI *_moo_py_api;
void moo_python_init (MooPyAPI *api);


#define moo_python_running() (_moo_py_api != NULL)

#define moo_Py_INCREF               _moo_py_api->incref
#define moo_Py_DECREF               _moo_py_api->decref
#define moo_PyErr_Print             _moo_py_api->err_print

#define moo_python_run_string       _moo_py_api->run_string
#define moo_python_run_file         _moo_py_api->run_file


G_END_DECLS

#endif /* __MOO_PYTHON_H__ */
