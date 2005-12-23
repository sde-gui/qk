/*
 *   mooapp/moopython.c
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

#include "moopython/moopython.h"


MooPyAPI *_moo_py_api = NULL;

void moo_python_init (MooPyAPI *api)
{
    _moo_py_api = api;
}
