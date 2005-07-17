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
#include "mooterm/mooterm-selection.h"


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


#define HOWMANY(x, y)               (((x) + (y) - 1) / y())
#define CALC_ROW(y, char_height)    (HOWMANY (y + 1, char_height) - 1)
#define CALC_COL(x, char_width)     (HOWMANY (x + 1, char_width) - 1)
#define DIFF(x, y)                  (ABS ((x) - (y)))


gboolean    moo_term_button_press           (GtkWidget      *widget,
                                             GdkEventButton *event)
{
    MooTerm *term;

    term = MOO_TERM (widget);

    moo_term_set_pointer_visible (term, TRUE);

    if (event->button != 1)
    {
        if (event->type != GDK_BUTTON_PRESS)
            return TRUE;

        switch (event->button)
        {
            case 2:
                moo_term_paste_clipboard (term, GDK_SELECTION_PRIMARY);
                break;

            case 3:
                moo_term_do_popup_menu (term, event);
                break;
        }

        return TRUE;
    }

    return TRUE;
}


gboolean    moo_term_button_release         (GtkWidget      *widget,
                                             GdkEventButton *event)
{
    MooTerm *term;

    term = MOO_TERM (widget);

    moo_term_set_pointer_visible (term, TRUE);

    return TRUE;
}


gboolean    moo_term_motion_notify          (GtkWidget      *widget,
                                             GdkEventMotion *event)
{
    MooTerm *term;

    term = MOO_TERM (widget);

    moo_term_set_pointer_visible (term, TRUE);

    return TRUE;
}
