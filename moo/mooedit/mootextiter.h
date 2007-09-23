/*
 *   mootextiter.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_TEXT_ITER_H
#define MOO_TEXT_ITER_H

#include <gtk/gtktextbuffer.h>

G_BEGIN_DECLS


typedef enum _MooBracketMatchType {
    MOO_BRACKET_MATCH_NOT_AT_BRACKET   = -1,
    MOO_BRACKET_MATCH_NONE             = 0,
    MOO_BRACKET_MATCH_CORRECT          = 1,
    MOO_BRACKET_MATCH_INCORRECT        = 2
} MooBracketMatchType;


/* it assumes that iter points to a bracket */
MooBracketMatchType moo_text_iter_find_matching_bracket (GtkTextIter *iter,
                                                         int          limit);

/* tries to find bracket near the iter, i.e. like |( or (|,
 *   and chooses the better one in the case )|( */
gboolean    moo_text_iter_at_bracket    (GtkTextIter        *iter);

/* like gtk_text_iter_get_chars_in_line, but tabs are translated to 8,
   and line end is not included */
int         moo_text_iter_get_visual_line_length    (const GtkTextIter  *iter,
                                                     int                 tab_width);
int         moo_text_iter_get_visual_line_offset    (const GtkTextIter  *iter,
                                                     int                 tab_width);
void        moo_text_iter_set_visual_line_offset    (GtkTextIter        *iter,
                                                     int                 offset,
                                                     int                 tab_width);


G_END_DECLS

#endif /* MOO_TEXT_ITER_H */
