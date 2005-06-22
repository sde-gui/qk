/*
 *   mooutils/mootypes.h
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

#ifndef MOOUTILS_TYPES_H
#define MOOUTILS_TYPES_H

#include <glib-object.h>

G_BEGIN_DECLS


#define MOO_TYPE_GTYPE                  (moo_gtype_get_type())
GType       moo_gtype_get_type              (void);

void        moo_value_set_gtype             (GValue     *value,
                                             GType       v_gtype);
GType       moo_value_get_gtype             (const GValue *value);


G_END_DECLS

#endif /* MOOUTILS_TYPES_H */
