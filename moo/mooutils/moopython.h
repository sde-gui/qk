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

#ifndef MOO_PYTHON_H
#define MOO_PYTHON_H

#include <glib.h>

G_BEGIN_DECLS

#define MOO_PY_API_VERSION 94

typedef struct _MooPyAPI MooPyAPI;
typedef struct _MooPyObject MooPyObject;
typedef struct _MooPyMethodDef MooPyMethodDef;

typedef MooPyObject* (*MooPyCFunction) (MooPyObject*, MooPyObject*);

enum {
    MOO_PY_METH_VARARGS  = 1 << 0,
    MOO_PY_METH_KEYWORDS = 1 << 1,
    MOO_PY_METH_NOARGS   = 1 << 2,
    MOO_PY_METH_O        = 1 << 3,
    MOO_PY_METH_CLASS    = 1 << 4,
    MOO_PY_METH_STATIC   = 1 << 5
};

enum {
    MOO_PY_RUNTIME_ERROR,
    MOO_PY_TYPE_ERROR,
    MOO_PY_VALUE_ERROR,
    MOO_PY_NOT_IMPLEMENTED_ERROR
};

struct _MooPyMethodDef {
    const char     *ml_name;
    MooPyCFunction  ml_meth;
    int             ml_flags;
    const char     *ml_doc;
};

struct _MooPyAPI {
    MooPyObject *py_none;
    MooPyObject *py_true;
    MooPyObject *py_false;

    MooPyObject* (*incref)                  (MooPyObject    *obj);
    void         (*decref)                  (MooPyObject    *obj);

    char*        (*get_info)                (void);

    MooPyObject* (*run_simple_string)       (const char     *str);
    MooPyObject* (*run_string)              (const char     *str,
                                             MooPyObject    *globals,
                                             MooPyObject    *locals);
    MooPyObject* (*run_file)                (void           *fp,
                                             const char     *filename,
                                             MooPyObject    *globals,
                                             MooPyObject    *locals);
    MooPyObject* (*run_code)                (const char     *str,
                                             MooPyObject    *globals,
                                             MooPyObject    *locals);
    MooPyObject* (*create_script_dict)      (const char     *name);

    MooPyObject* (*py_object_from_gobject)  (gpointer        gobj);
    gpointer     (*gobject_from_py_object)  (MooPyObject    *pyobj);

    MooPyObject* (*dict_get_item)           (MooPyObject    *dict,
                                             const char     *key);
    gboolean     (*dict_set_item)           (MooPyObject    *dict,
                                             const char     *key,
                                             MooPyObject    *val);
    gboolean     (*dict_del_item)           (MooPyObject    *dict,
                                             const char     *key);

    MooPyObject* (*import_exec)             (const char     *name,
                                             const char     *string);

    void         (*set_error)               (int             type,
                                             const char     *format,
                                             ...);
    void         (*py_err_print)            (void);
    MooPyObject* (*py_object_call_method)   (MooPyObject    *object,
                                             const char     *method,
                                             const char     *format,
                                             ...);
    MooPyObject* (*py_object_call_function) (MooPyObject    *callable,
                                             const char     *format,
                                             ...);
    MooPyObject* (*py_c_function_new)       (MooPyMethodDef *meth,
                                             MooPyObject    *self);
    int          (*py_module_add_object)    (MooPyObject    *mod,
                                             const char     *name,
                                             MooPyObject    *obj);
    gboolean     (*py_arg_parse_tuple)      (MooPyObject    *args,
                                             const char     *format,
                                            ...);

    GSList *_free_list;
};


extern MooPyAPI *moo_py_api;
gboolean     moo_python_init        (guint           version,
                                     MooPyAPI       *api);

void         moo_python_add_data    (gpointer        data,
                                     GDestroyNotify  destroy);

MooPyObject *moo_Py_INCREF          (MooPyObject    *obj);
void         moo_Py_DECREF          (MooPyObject    *obj);

#define moo_python_running() (moo_py_api != NULL)

#define moo_python_get_info             moo_py_api->get_info

#define moo_python_run_simple_string    moo_py_api->run_simple_string
#define moo_python_run_string           moo_py_api->run_string
#define moo_python_run_file             moo_py_api->run_file
#define moo_python_run_code             moo_py_api->run_code
#define moo_python_create_script_dict   moo_py_api->create_script_dict

#define moo_py_dict_get_item            moo_py_api->dict_get_item
#define moo_py_dict_set_item            moo_py_api->dict_set_item
#define moo_py_dict_del_item            moo_py_api->dict_del_item

#define moo_py_import_exec              moo_py_api->import_exec
#define moo_py_object_from_gobject      moo_py_api->py_object_from_gobject
#define moo_gobject_from_py_object      moo_py_api->gobject_from_py_object

#define moo_py_set_error                moo_py_api->set_error

#define moo_PyErr_Print                 moo_py_api->py_err_print
#define moo_PyObject_CallMethod         moo_py_api->py_object_call_method
#define moo_PyObject_CallFunction       moo_py_api->py_object_call_function
#define moo_PyCFunction_New             moo_py_api->py_c_function_new
#define moo_PyModule_AddObject          moo_py_api->py_module_add_object
#define moo_PyArg_ParseTuple            moo_py_api->py_arg_parse_tuple
#define moo_Py_None                     moo_py_api->py_none
#define moo_Py_True                     moo_py_api->py_true
#define moo_Py_False                    moo_py_api->py_false


G_END_DECLS

#endif /* MOO_PYTHON_H */
