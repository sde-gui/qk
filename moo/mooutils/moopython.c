/*
 *   mooutils/moopython.c
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

#include "mooutils/moopython.h"


MooPyAPI *moo_py_api = NULL;

gboolean
moo_python_init (guint     version,
                 MooPyAPI *api)
{
    if (version != MOO_PY_API_VERSION)
        return FALSE;

    g_return_val_if_fail (!moo_py_api || !api, FALSE);

    moo_py_api = api;
    return TRUE;
}


MooPyObject *
moo_Py_INCREF (MooPyObject *obj)
{
    g_return_val_if_fail (moo_python_running (), obj);

    if (obj)
        moo_py_api->incref (obj);

    return obj;
}


void
moo_Py_DECREF (MooPyObject *obj)
{
    g_return_if_fail (moo_python_running ());

    if (obj)
        moo_py_api->decref (obj);
}
