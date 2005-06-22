/*
 *   mooterm/mootermcursor.c
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

#define MOOTERM_COMPILATION
#include "mooterm/mooterm-private.h"


void        moo_term_im_commit          (G_GNUC_UNUSED GtkIMContext   *imcontext,
                                         G_GNUC_UNUSED gchar          *arg,
                                         G_GNUC_UNUSED MooTerm        *term)
{
    g_warning ("%s: implement me", G_STRLOC);
}
