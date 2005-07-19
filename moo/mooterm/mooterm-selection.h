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

#include <mooterm/mooterm.h>

G_BEGIN_DECLS


enum {
    NOT_SELECTED    = 0,
    FULL_SELECTED   = 1,
    PART_SELECTED   = 2
};


gboolean    moo_term_selection_empty        (MooTerm    *term);

gpointer    term_selection_new              (MooTerm    *term);


void        moo_term_selection_clear        (MooTerm    *term);
char       *moo_term_selection_get_text     (MooTerm    *term);
int         moo_term_selection_row_selected (MooTerm    *term,
                                             guint       row);
gboolean    moo_term_selected               (MooTerm    *term,
                                             guint       row,
                                             guint       col);
gboolean    moo_term_get_selection_bounds   (MooTerm    *term,
                                             guint      *left_row,
                                             guint      *left_col,
                                             guint      *right_row,
                                             guint      *right_col);


G_END_DECLS

#endif /* MOOTERM_MOOTERM_SELECTION_H */
