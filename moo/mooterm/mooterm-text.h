/*
 *   mooterm-text.h
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

#ifndef __MOO_TERM_TEXT_H__
#define __MOO_TERM_TEXT_H__

#include <mooterm/mooterm.h>
#include <mooedit/mootextview.h>


void        _moo_term_window_to_buffer_coords(MooTerm            *term,
                                             int                 window_x,
                                             int                 window_y,
                                             int                *buffer_x,
                                             int                *buffer_y);
void        _moo_term_get_iter_at_location  (MooTerm            *term,
                                             GtkTextIter        *iter,
                                             int                 x,
                                             int                 y);
gboolean    _moo_term_get_selection_bounds  (MooTerm            *term,
                                             GtkTextIter        *sel_start,
                                             GtkTextIter        *sel_end);
void        _moo_term_iter_order             (GtkTextIter        *first,
                                             GtkTextIter        *second);
gboolean    _moo_term_iter_in_range         (const GtkTextIter  *iter,
                                             const GtkTextIter  *start,
                                             const GtkTextIter  *end);
void        _moo_term_place_selection_end   (MooTerm            *term,
                                             const GtkTextIter  *where);
void        _moo_term_select_range          (MooTerm            *term,
                                             const GtkTextIter  *start,
                                             const GtkTextIter  *end);
gboolean    _moo_term_extend_selection      (MooTerm            *term,
                                             MooTextSelectionType type,
                                             GtkTextIter        *end,
                                             GtkTextIter        *start);
void        _moo_term_get_visible_rect      (MooTerm            *term,
                                             GdkRectangle       *rect);


#endif /* __MOO_TERM_TEXT_H__ */
