/*
 *   mootextsearch.h
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

#ifndef MOO_TEXT_SEARCH_H
#define MOO_TEXT_SEARCH_H

#include <mooedit/mootextiter.h>

G_BEGIN_DECLS


typedef enum /*< flags >*/
{
    MOO_TEXT_SEARCH_CASELESS        = 1 << 0,
    MOO_TEXT_SEARCH_REGEX           = 1 << 1,
    MOO_TEXT_SEARCH_WHOLE_WORDS     = 1 << 2,
    MOO_TEXT_SEARCH_REPL_LITERAL    = 1 << 3
} MooTextSearchFlags;

typedef enum /*< enum >*/
{
    MOO_TEXT_REPLACE_STOP = 0,
    MOO_TEXT_REPLACE_SKIP = 1,
    MOO_TEXT_REPLACE_DO_REPLACE = 2,
    MOO_TEXT_REPLACE_ALL = 3
} MooTextReplaceResponse;

gboolean moo_text_search_forward            (const GtkTextIter      *start,
                                             const char             *str,
                                             MooTextSearchFlags      flags,
                                             GtkTextIter            *match_start,
                                             GtkTextIter            *match_end,
                                             const GtkTextIter      *end);
gboolean moo_text_search_backward           (const GtkTextIter      *start,
                                             const char             *str,
                                             MooTextSearchFlags      flags,
                                             GtkTextIter            *match_start,
                                             GtkTextIter            *match_end,
                                             const GtkTextIter      *end);

int      moo_text_replace_all               (GtkTextIter            *start,
                                             GtkTextIter            *end,
                                             const char             *text,
                                             const char             *replacement,
                                             MooTextSearchFlags      flags);


G_END_DECLS

#endif /* MOO_TEXT_SEARCH_H */
