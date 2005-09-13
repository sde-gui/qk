/*
 *   moovalue.h
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

#ifndef __MOO_VALUE_H__
#define __MOO_VALUE_H__

#include <gdk/gdkcolor.h>

G_BEGIN_DECLS


#define MOO_TYPE_GTYPE                  (moo_gtype_get_type())

GType           moo_gtype_get_type          (void);

void            moo_value_set_gtype         (GValue     *value,
                                             GType       v_gtype);
GType           moo_value_get_gtype         (const GValue *value);

gboolean        moo_value_type_supported    (GType           type);
gboolean        moo_value_convert           (const GValue   *src,
                                             GValue         *dest);
gboolean        moo_value_equal             (const GValue   *a,
                                             const GValue   *b);

gboolean        moo_value_change_type       (GValue         *val,
                                             GType           new_type);

gboolean        moo_value_convert_to_bool   (const GValue   *val);
int             moo_value_convert_to_int    (const GValue   *val);
int             moo_value_convert_to_enum   (const GValue   *val,
                                             GType           enum_type);
double          moo_value_convert_to_double (const GValue   *val);
const GdkColor *moo_value_convert_to_color  (const GValue   *val);
const char     *moo_value_convert_to_string (const GValue   *val);


G_END_DECLS

#endif /* __MOO_VALUE_H__ */
