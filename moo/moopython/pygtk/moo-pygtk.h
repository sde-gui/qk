/*
 *   moopython/moo-pygtk.h
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

#ifndef __MOO_PYGTK_H__
#define __MOO_PYGTK_H__

#include <Python.h>
#include <glib.h>

G_BEGIN_DECLS


gboolean    _moo_pygtk_init             (void);

gboolean    _moo_utils_mod_init         (void);
gboolean    _moo_edit_mod_init          (void);
gboolean    _moo_term_mod_init          (void);
gboolean    _moo_app_mod_init           (void);


G_END_DECLS

#endif /* __MOO_PYGTK_H__ */
