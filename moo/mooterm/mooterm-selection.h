/*
 *   mooterm-selection.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOOTERM_COMPILATION
#error "This file may not be included"
#endif

#ifndef MOO_TERM_SELECTION_H
#define MOO_TERM_SELECTION_H

#include <mooterm/mooterm.h>

G_BEGIN_DECLS


enum {
    NOT_SELECTED    = 0,
    FULL_SELECTED   = 1,
    PART_SELECTED   = 2
};

gpointer    _moo_term_selection_new         (MooTerm    *term);


void        _moo_term_selection_invalidate  (MooTerm    *term);
void        _moo_term_selection_clear       (MooTerm    *term);
int         _moo_term_row_selected          (MooTerm    *term,
                                             guint       row);
gboolean    _moo_term_cell_selected         (MooTerm    *term,
                                             guint       row,
                                             guint       col);
gboolean    _moo_term_get_selected_cells    (MooTerm    *term,
                                             guint      *left_row,
                                             guint      *left_col,
                                             guint      *right_row,
                                             guint      *right_col);


G_END_DECLS

#endif /* MOO_TERM_SELECTION_H */
