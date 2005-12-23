/*
 *   mooutils/moopython.h
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

#define MOO_PY_API_VERSION 87

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


extern MooPyAPI *moo_py_api;
gboolean moo_python_init (guint     version,
                          MooPyAPI *api);


#define moo_python_running() (moo_py_api != NULL)

#define moo_Py_INCREF               moo_py_api->incref
#define moo_Py_DECREF               moo_py_api->decref
#define moo_PyErr_Print             moo_py_api->err_print

#define moo_python_run_string       moo_py_api->run_string
#define moo_python_run_file         moo_py_api->run_file


G_END_DECLS

#endif /* __MOO_PYTHON_H__ */
