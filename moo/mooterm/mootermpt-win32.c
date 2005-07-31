/*
 *   mooterm/mootermvt-win32.c
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

#include "mooterm/mootermpt.h"


void            moo_term_pt_send_intr       (MooTermPt      *pt)
{
}


char            moo_term_pt_get_erase_char  (MooTermPt      *pt_gen)
{
    return 127;
}


GType           moo_term_pt_win_get_type    (void)
{
    return 0;
}


void        moo_term_set_helper_directory   (G_GNUC_UNUSED const char *dir)
{
}
