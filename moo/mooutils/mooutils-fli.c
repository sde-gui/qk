/*
 *   mooutils-fli.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#include "mooutils/moopython.h"


/**
 * Python
 */

typedef struct {
    gpointer data;
    GDestroyNotify destroy;
} Data;

static MooPyAPI *moo_py_api_var = NULL;

MooPyAPI *
moo_py_api_ (void)
{
    return moo_py_api_var;
}


void
moo_python_add_data (gpointer       data,
                     GDestroyNotify destroy)
{
    Data *d;

    g_return_if_fail (moo_py_api_var != NULL);
    g_return_if_fail (destroy != NULL);

    d = g_new (Data, 1);
    d->data = data;
    d->destroy = destroy;
    moo_py_api_var->_free_list = g_slist_prepend (moo_py_api_var->_free_list, d);
}

static void
data_free (Data *d)
{
    if (d)
    {
        d->destroy (d->data);
        g_free (d);
    }
}


gboolean
moo_python_init (guint     version,
                 MooPyAPI *api)
{
    if (version != MOO_PY_API_VERSION)
        return FALSE;

    g_return_val_if_fail (!moo_py_api_var || !api, FALSE);

    if (moo_py_api_var)
    {
        g_slist_foreach (moo_py_api_var->_free_list, (GFunc) data_free, NULL);
        g_slist_free (moo_py_api_var->_free_list);
    }

    moo_py_api_var = api;

    if (moo_py_api_var)
        moo_py_api_var->_free_list = NULL;

    return TRUE;
}


MooPyObject *
moo_Py_INCREF (MooPyObject *obj)
{
    g_return_val_if_fail (moo_python_running (), obj);

    if (obj)
        moo_py_api_var->incref (obj);

    return obj;
}


void
moo_Py_DECREF (MooPyObject *obj)
{
    g_return_if_fail (moo_python_running ());

    if (obj)
        moo_py_api_var->decref (obj);
}
