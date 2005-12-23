/*
 *   mooutils/moopython.c
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

#include "mooutils/moopython.h"


MooPyAPI *_moo_py_api = NULL;

gboolean
moo_python_init (guint     version,
                 MooPyAPI *api)
{
    if (version != MOO_PY_API_VERSION)
        return FALSE;

    g_return_val_if_fail (!_moo_py_api || !api, FALSE);

    _moo_py_api = api;
    return TRUE;
}
