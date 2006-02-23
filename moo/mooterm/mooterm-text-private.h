/*
 *   mooterm-text-pri.h
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

#ifndef MOOTERM_COMPILATION
#error "This file may not be included"
#endif

#ifndef __MOO_TERM_TEXT_PRIVATE_H__
#define __MOO_TERM_TEXT_PRIVATE_H__

#include "mooterm/mooterm-text.h"
#include "mooedit/mootextview.h"


gboolean    _moo_term_get_selection_bounds  (MooTerm            *term,
                                             MooTermIter        *sel_start,
                                             MooTermIter        *sel_end);
void        _moo_term_iter_order            (MooTermIter        *first,
                                             MooTermIter        *second);
gboolean    _moo_term_iter_in_range         (const MooTermIter  *iter,
                                             const MooTermIter  *start,
                                             const MooTermIter  *end);
void        _moo_term_place_selection_end   (MooTerm            *term,
                                             const MooTermIter  *where);
void        _moo_term_select_range          (MooTerm            *term,
                                             const MooTermIter  *start,
                                             const MooTermIter  *end);
gboolean    _moo_term_extend_selection      (MooTerm            *term,
                                             MooTextSelectionType type,
                                             MooTermIter        *end,
                                             MooTermIter        *start);
void        _moo_term_get_visible_rect      (MooTerm            *term,
                                             GdkRectangle       *rect);


#endif /* __MOO_TERM_TEXT_PRIVATE_H__ */
