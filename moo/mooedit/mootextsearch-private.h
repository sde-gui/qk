/*
 *   mootextsearch-private.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_TEXT_SEARCH_PRIVATE_H
#define MOO_TEXT_SEARCH_PRIVATE_H

#include <mooedit/mootextsearch.h>
#include <glib/gregex.h>

G_BEGIN_DECLS


/* replacement is evaluated in case of regex */
typedef MooTextReplaceResponse (*MooTextReplaceFunc) (const char         *text,
                                                      GRegex             *regex,
                                                      const char         *replacement,
                                                      const GtkTextIter  *to_replace_start,
                                                      const GtkTextIter  *to_replace_end,
                                                      gpointer            user_data);


gboolean _moo_text_search_regex_forward     (const GtkTextIter      *start,
                                             const GtkTextIter      *end,
                                             GRegex                 *regex,
                                             GtkTextIter            *match_start,
                                             GtkTextIter            *match_end,
                                             char                  **string,
                                             int                    *match_offset,
                                             int                    *match_len,
                                             GMatchInfo            **match_info);
gboolean _moo_text_search_regex_backward    (const GtkTextIter      *start,
                                             const GtkTextIter      *end,
                                             GRegex                 *regex,
                                             GtkTextIter            *match_start,
                                             GtkTextIter            *match_end,
                                             char                  **string,
                                             int                    *match_offset,
                                             int                    *match_len,
                                             GMatchInfo            **match_info);

int      _moo_text_replace_regex_all        (GtkTextIter            *start,
                                             GtkTextIter            *end,
                                             GRegex                 *regex,
                                             const char             *replacement,
                                             gboolean                replacement_literal);

int      _moo_text_replace_regex_all_interactive
                                            (GtkTextIter            *start,
                                             GtkTextIter            *end,
                                             GRegex                 *regex,
                                             const char             *replacement,
                                             gboolean                replacement_literal,
                                             MooTextReplaceFunc      func,
                                             gpointer                func_data);
int      _moo_text_replace_all_interactive  (GtkTextIter            *start,
                                             GtkTextIter            *end,
                                             const char             *text,
                                             const char             *replacement,
                                             MooTextSearchFlags      flags,
                                             MooTextReplaceFunc      func,
                                             gpointer                func_data);


G_END_DECLS

#endif /* MOO_TEXT_SEARCH_PRIVATE_H */
