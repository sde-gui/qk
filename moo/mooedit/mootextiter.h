/*
 *   mooedit/mootextiter.h
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

#ifndef MOOEDIT_MOOTEXTITER_H
#define MOOEDIT_MOOTEXTITER_H

#include "mooedit/gtksourceview/gtksourcebuffer.h"

G_BEGIN_DECLS


typedef enum _MooEditBracketMatchType {
    MOO_EDIT_BRACKET_MATCH_NOT_AT_BRACKET   = -1,
    MOO_EDIT_BRACKET_MATCH_NONE             = 0,
    MOO_EDIT_BRACKET_MATCH_CORRECT          = 1,
    MOO_EDIT_BRACKET_MATCH_INCORRECT        = 2
} MooEditBracketMatchType;


/* it assumes that iter points to a bracket */
MooEditBracketMatchType moo_edit_find_matching_bracket
                                        (GtkSourceBuffer    *buffer,
                                         GtkTextIter        *iter);

/* tries to find bracket near the iter, i.e. like |( or (|,
 *   and chooses right one in the case )|(                  */
gboolean    moo_edit_at_bracket         (GtkSourceBuffer    *buffer,
                                         GtkTextIter        *iter);

/* does the same as gtk_text_buffer_get_iter_at_line_offset, with the
 * difference that it accepts invalid position */
gboolean    moo_get_iter_at_line_offset (GtkTextBuffer      *buffer,
                                         GtkTextIter        *iter,
                                         gint                line_number,
                                         gint                char_offset,
                                         gboolean            exact);


G_END_DECLS

#endif /* MOOEDIT_MOOTEXTITER_H */
