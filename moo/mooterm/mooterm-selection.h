/*
 *   mooterm-selection.h
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

#ifndef MOOTERM_COMPILATION
#error "This file may not be included"
#endif

#ifndef __MOO_TERM_SELECTION_H__
#define __MOO_TERM_SELECTION_H__

#include <mooterm/mooterm.h>

G_BEGIN_DECLS


enum {
    NOT_SELECTED    = 0,
    FULL_SELECTED   = 1,
    PART_SELECTED   = 2
};


gboolean    _moo_term_selection_empty       (MooTerm    *term);

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

#endif /* __MOO_TERM_SELECTION_H__ */
