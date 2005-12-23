/*
 *   moopython/moo-pygtk.h
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

#ifndef __MOO_PYGTK_H__
#define __MOO_PYGTK_H__

#include <Python.h>
#include <glib.h>

G_BEGIN_DECLS

extern PyMethodDef _moo_utils_functions[];
extern PyMethodDef _moo_term_functions[];
extern PyMethodDef _moo_edit_functions[];
extern PyMethodDef _moo_app_functions[];

gboolean    initmoo                     (void);

gboolean    _moo_utils_mod_init         (void);
gboolean    _moo_edit_mod_init          (void);
gboolean    _moo_term_mod_init          (void);
gboolean    _moo_app_mod_init           (void);

void        _moo_utils_register_classes (PyObject       *dict);
void        _moo_utils_add_constants    (PyObject       *module,
                                         const char     *strip_prefix);
void        _moo_edit_register_classes  (PyObject       *dict);
void        _moo_edit_add_constants     (PyObject       *module,
                                         const char     *strip_prefix);
void        _moo_term_register_classes  (PyObject       *dict);
void        _moo_term_add_constants     (PyObject       *module,
                                         const char     *strip_prefix);
void        _moo_app_register_classes   (PyObject       *dict);
void        _moo_app_add_constants      (PyObject       *module,
                                         const char     *strip_prefix);


G_END_DECLS

#endif /* __MOO_PYGTK_H__ */
