/*
 *   mooplugin-python.h
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

#ifndef __MOO_PLUGIN_PYTHON_H__
#define __MOO_PLUGIN_PYTHON_H__

#include <glib.h>

G_BEGIN_DECLS


gboolean  _moo_python_plugin_init (void);

gpointer  _moo_python_plugin_register   (gpointer   py_plugin_type,
                                         gpointer   py_win_plugin_type,
                                         gpointer   py_doc_plugin_type);


G_END_DECLS

#endif /* __MOO_PLUGIN_PYTHON_H__ */
