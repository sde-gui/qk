/*
 *   mooterm/mootermselection.c
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


TermSelection   *term_selection_new         (void)
{
    TermSelection *sel = g_new0 (TermSelection, 1);

    sel->screen_width = 0;
    sel->button_pressed = FALSE;
    sel->click = 1;
    sel->drag = FALSE;
    sel->scroll = SELECT_SCROLL_NONE;
    sel->st_id = 0;
    sel->empty = TRUE;

    return sel;
}


inline static void reset_aux_stuff (TermSelection *sel)
{
    sel->button_pressed = FALSE;
    sel->click = 1;
    sel->drag = FALSE;
}


void             term_selection_clear       (MooTerm       *term)
{
    if (!term->priv->selection->empty)
    {
        term->priv->selection->empty = TRUE;
        moo_term_invalidate_all (term);
    }
}


char            *term_selection_get_text    (MooTerm       *term)
{
    return NULL;
}
