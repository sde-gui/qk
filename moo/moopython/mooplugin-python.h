/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mooplugin-python.h
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

#ifndef __MOO_PLUGIN_PYTHON_H__
#define __MOO_PLUGIN_PYTHON_H__

#include <Python.h>
#include "mooedit/mooplugin.h"

G_BEGIN_DECLS


gboolean    _moo_python_plugin_init     (char      **dirs);

PyObject   *_moo_python_plugin_hook     (const char *event,
                                         PyObject   *callback,
                                         PyObject   *data);
PyObject   *_moo_python_plugin_register (const char *id,
                                         PyObject   *plugin_type,
                                         PyObject   *win_plugin_type,
                                         PyObject   *doc_plugin_type);


G_END_DECLS

#endif /* __MOO_PLUGIN_PYTHON_H__ */
