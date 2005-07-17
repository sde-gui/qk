/*
 *   mooterm/mooterm-selection.h
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

#ifndef MOOTERM_MOOTERM_SELECTION_H
#define MOOTERM_MOOTERM_SELECTION_H

#include <glib.h>

G_BEGIN_DECLS


enum {
    NOT_SELECTED    = 0,
    FULL_SELECTED   = 1,
    PART_SELECTED   = 2
};

#define SELECT_SCROLL_NONE  (0)
#define SELECT_SCROLL_UP    (-1)
#define SELECT_SCROLL_DOWN  (1)

struct _TermSelection {
    guint       screen_width;

    /* absolute coordinates in the buffer
    selected range is [(l_row, l_col), (r_row, r_col))
    l_row, l_col and r_row are valid
    r_col may be equal to _width */
    guint       l_row;
    guint       l_col;
    guint       r_row;
    guint       r_col;
    gboolean    empty;

    gboolean    button_pressed;
    int         click;
    gboolean    drag;
    // buffer coordinates
    guint       drag_begin_row;
    guint       drag_begin_col;
    guint       drag_end_row;
    guint       drag_end_col;

    int scroll;
    guint st_id;
};


TermSelection   *term_selection_new         (void);

#define term_selection_free(sel) g_free (sel)

void             term_set_selection         (struct _MooTerm    *term,
                                             guint               row1,
                                             guint               col1,
                                             guint               row2,
                                             guint               col2);
void             term_selection_clear       (struct _MooTerm    *term);
char            *term_selection_get_text    (struct _MooTerm    *term);

inline static void term_selection_set_width (struct _MooTerm    *term,
                                             guint               width)
{
    term->priv->selection->screen_width = width;
    term_selection_clear (term);
}

inline static int term_selection_row_selected (TermSelection *sel,
                                               guint          row)
{
    if (sel->empty || sel->r_row < row || row < sel->l_row)
        return NOT_SELECTED;
    else if (sel->l_row < row && row < sel->r_row)
        return FULL_SELECTED;
    else
        return PART_SELECTED;
}

inline static gboolean term_selected    (TermSelection   *sel,
                                         guint            row,
                                         guint            col)
{
    if (sel->empty || sel->r_row < row || row < sel->l_row)
        return FALSE;
    else if (sel->l_row < row && row < sel->r_row)
        return TRUE;
    else if (sel->l_row == sel->r_row)
        return sel->l_col <= col && col < sel->r_col;
    else if (sel->l_row == row)
        return sel->l_col <= col;
    else
        return col < sel->r_row;
}


G_END_DECLS

#endif /* MOOTERM_MOOTERM_SELECTION_H */
