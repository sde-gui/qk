/*
 *   mooutils/mooparam.h
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

#ifndef MOOUTILS_MOOPARAM_H
#define MOOUTILS_MOOPARAM_H

#include <glib-object.h>

G_BEGIN_DECLS


GParameter  *moo_param_array_collect    (GType       type,
                                         guint      *len,
                                         const char *first_prop_name,
                                         ...);
GParameter  *moo_param_array_collect_valist
                                        (GType       type,
                                         guint      *len,
                                         const char *first_prop_name,
                                         va_list     var_args);

GParameter  *moo_param_array_add        (GType       type,
                                         GParameter *src,
                                         guint       len,
                                         guint      *new_len,
                                         const char *first_prop_name,
                                         ...);
GParameter  *moo_param_array_add_valist (GType       type,
                                         GParameter *src,
                                         guint       len,
                                         guint      *new_len,
                                         const char *first_prop_name,
                                         va_list     var_args);

GParameter  *moo_param_array_add_type   (GParameter *src,
                                         guint       len,
                                         guint      *new_len,
                                         const char *first_prop_name,
                                         ...);
GParameter  *moo_param_array_add_type_valist
                                        (GParameter *src,
                                         guint       len,
                                         guint      *new_len,
                                         const char *first_prop_name,
                                         va_list     var_args);

void         moo_param_array_free       (GParameter *array,
                                         guint       len);


G_END_DECLS

#endif /* MOOUTILS_MOOPARAM_H */
