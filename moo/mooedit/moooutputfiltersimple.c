/*
 *   moooutputfiltersimple.c
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

#include "mooedit/moooutputfiltersimple.h"
#include <string.h>


G_DEFINE_TYPE (MooOutputFilterSimple, moo_output_filter_simple, G_TYPE_OBJECT)


static void
moo_output_filter_simple_class_init (G_GNUC_UNUSED MooOutputFilterSimpleClass *klass)
{
}


static void
moo_output_filter_simple_init (G_GNUC_UNUSED MooOutputFilterSimple *filter)
{
}


void
_moo_command_filter_simple_init (void)
{
    static gboolean been_here = FALSE;

    if (been_here)
        return;

    been_here = TRUE;
}
