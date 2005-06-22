/*
 *   mooutils/mootypes.c
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

#include "mooutils/mootypes.h"


static gpointer copy_gtype (gpointer boxed)
{
    return boxed;
}

static void     free_gtype (G_GNUC_UNUSED gpointer boxed)
{
}


GType       moo_gtype_get_type              (void)
{
    static GType type = 0;
    if (!type)
    {
        type = g_boxed_type_register_static ("GType",
                                             copy_gtype, free_gtype);
    }

    return type;
}


void        moo_value_set_gtype             (GValue     *value,
                                             GType       v_gtype)
{
    g_value_set_static_boxed (value, (gconstpointer) v_gtype);
}


GType       moo_value_get_gtype             (const GValue *value)
{
    return (GType) g_value_get_boxed (value);
}
