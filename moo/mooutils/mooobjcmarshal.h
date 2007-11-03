/*
 *   mooobjcmarshal.h
 *
 *   Copyright (C) 2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_OBJC_MARSHAL_H
#define MOO_OBJC_MARSHAL_H

#include <glib-object.h>
#include "moocobject.h"

G_BEGIN_DECLS


void _moo_objc_marshal (GCallback          callback,
                        id                 add_obj,
                        SEL                add_sel,
                        GValue            *return_value,
                        guint              n_params,
                        const GValue      *params,
                        id                 data,
                        gboolean           use_data,
                        gboolean           swap);


G_END_DECLS

#endif /* MOO_OBJC_MARSHAL_H */
/* -*- objc -*- */
