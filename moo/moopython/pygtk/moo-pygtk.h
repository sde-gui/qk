/*
 *   moo-pygtk.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_PYGTK_H
#define MOO_PYGTK_H

#include <Python.h>
#include <glib.h>

G_BEGIN_DECLS


gboolean    _moo_pygtk_init             (void);

gboolean    _moo_utils_mod_init         (void);
gboolean    _moo_edit_mod_init          (void);
gboolean    _moo_app_mod_init           (void);

extern const PyMethodDef _moo_utils_functions[];
extern const PyMethodDef _moo_edit_functions[];
extern const PyMethodDef _moo_app_functions[];

void        _moo_utils_register_classes (PyObject       *dict);
void        _moo_utils_add_constants    (PyObject       *module,
                                         const char     *strip_prefix);
void        _moo_edit_register_classes  (PyObject       *dict);
void        _moo_edit_add_constants     (PyObject       *module,
                                         const char     *strip_prefix);
void        _moo_app_register_classes   (PyObject       *dict);
void        _moo_app_add_constants      (PyObject       *module,
                                         const char     *strip_prefix);


G_END_DECLS

#endif /* MOO_PYGTK_H */
