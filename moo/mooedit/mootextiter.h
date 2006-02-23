/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mootextiter.h
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

#ifndef __MOO_TEXT_ITER_H__
#define __MOO_TEXT_ITER_H__

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
 *   and chooses right one in the case )|(                  */
gboolean    moo_text_iter_at_bracket    (GtkTextIter        *iter);


G_END_DECLS

#endif /* __MOO_TEXT_ITER_H__ */
