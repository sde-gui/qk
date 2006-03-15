/*
 *   mooutils/moopython.h
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

#ifndef __MOO_PYTHON_H__
#define __MOO_PYTHON_H__

#include <glib.h>

G_BEGIN_DECLS

#define MOO_PY_API_VERSION 90

typedef struct _MooPyAPI MooPyAPI;
typedef struct _MooPyObject MooPyObject;


struct _MooPyAPI {
    MooPyObject* (*incref)              (MooPyObject *obj);
    void         (*decref)              (MooPyObject *obj);
    void         (*err_print)           (void);

    char*        (*get_info)            (void);

    MooPyObject* (*run_simple_string)   (const char  *str);
    MooPyObject* (*run_string)          (const char  *str,
                                         MooPyObject *locals,
                                         MooPyObject *globals);
    MooPyObject* (*run_file)            (void        *fp,
                                         const char  *filename);

    MooPyObject* (*py_object_from_gobject) (gpointer gobj);

    MooPyObject* (*get_script_dict)     (const char  *name);

    MooPyObject* (*dict_get_item)       (MooPyObject *dict,
                                         const char  *key);
    gboolean     (*dict_set_item)       (MooPyObject *dict,
                                         const char  *key,
                                         MooPyObject *val);
    gboolean     (*dict_del_item)       (MooPyObject *dict,
                                         const char  *key);
};


extern MooPyAPI *moo_py_api;
gboolean moo_python_init (guint     version,
                          MooPyAPI *api);


#define moo_python_running() (moo_py_api != NULL)

#define moo_Py_INCREF                   moo_py_api->incref
#define moo_Py_DECREF                   moo_py_api->decref
#define moo_PyErr_Print                 moo_py_api->err_print

#define moo_python_get_info             moo_py_api->get_info

#define moo_python_run_simple_string    moo_py_api->run_simple_string
#define moo_python_run_string           moo_py_api->run_string
#define moo_python_run_file             moo_py_api->run_file

#define moo_py_get_script_dict          moo_py_api->get_script_dict

#define moo_py_dict_get_item            moo_py_api->dict_get_item
#define moo_py_dict_set_item            moo_py_api->dict_set_item
#define moo_py_dict_del_item            moo_py_api->dict_del_item

#define moo_py_object_from_gobject      moo_py_api->py_object_from_gobject


G_END_DECLS

#endif /* __MOO_PYTHON_H__ */
