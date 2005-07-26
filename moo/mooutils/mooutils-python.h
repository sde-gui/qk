/*
 *   mooutils/mooutils-python.h
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

#ifndef MOOUTILS_MOOUTILS_PYTHON_H
#define MOOUTILS_MOOUTILS_PYTHON_H

#include <Python.h>
#include <glib.h>

G_BEGIN_DECLS


PyObject *moo_strv_to_pyobject (char **strv);
int moo_pyobject_to_strv (PyObject *obj, char ***dest);


G_END_DECLS

#endif /* MOOUTILS_MOOUTILS_PYTHON_H */
