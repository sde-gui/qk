/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mootextsearch.h
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

#ifndef __MOO_TEXT_SEARCH_H__
#define __MOO_TEXT_SEARCH_H__

#include <mooedit/mootextiter.h>
#include <mooutils/eggregex.h>

G_BEGIN_DECLS


#define MOO_TYPE_TEXT_SEARCH_FLAGS          (moo_text_search_flags_get_type ())
#define MOO_TYPE_TEXT_REPLACE_RESPONSE      (moo_text_replace_response_get_type ())
GType moo_text_search_flags_get_type        (void) G_GNUC_CONST;
GType moo_text_replace_response_get_type    (void) G_GNUC_CONST;


typedef enum {
    MOO_TEXT_SEARCH_CASELESS        = 1 << 0,
    MOO_TEXT_SEARCH_REGEX           = 1 << 1,
    MOO_TEXT_SEARCH_WHOLE_WORDS     = 1 << 2,
    MOO_TEXT_SEARCH_REPL_LITERAL    = 1 << 3
} MooTextSearchFlags;

typedef enum {
    MOO_TEXT_REPLACE_STOP = 0,
    MOO_TEXT_REPLACE_SKIP = 1,
    MOO_TEXT_REPLACE_DO_REPLACE = 2,
    MOO_TEXT_REPLACE_ALL = 3
} MooTextReplaceResponse;

/* replacement is evaluated in case of regex */
typedef MooTextReplaceResponse (*MooTextReplaceFunc) (const char         *text,
                                                      EggRegex           *regex,
                                                      const char         *replacement,
                                                      const GtkTextIter  *to_replace_start,
                                                      const GtkTextIter  *to_replace_end,
                                                      gpointer            user_data);


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

gboolean moo_text_search_regex_forward      (const GtkTextIter      *start,
                                             const GtkTextIter      *end,
                                             EggRegex               *regex,
                                             GtkTextIter            *match_start,
                                             GtkTextIter            *match_end,
                                             char                  **string,
                                             int                    *match_offset,
                                             int                    *match_len);
gboolean moo_text_search_regex_backward     (const GtkTextIter      *start,
                                             const GtkTextIter      *end,
                                             EggRegex               *regex,
                                             GtkTextIter            *match_start,
                                             GtkTextIter            *match_end,
                                             char                  **string,
                                             int                    *match_offset,
                                             int                    *match_len);

int      moo_text_replace_regex_all         (GtkTextIter            *start,
                                             GtkTextIter            *end,
                                             EggRegex               *regex,
                                             const char             *replacement,
                                             gboolean                replacement_literal);
int      moo_text_replace_all               (GtkTextIter            *start,
                                             GtkTextIter            *end,
                                             const char             *text,
                                             const char             *replacement,
                                             MooTextSearchFlags      flags);

int      moo_text_replace_regex_all_interactive
                                            (GtkTextIter            *start,
                                             GtkTextIter            *end,
                                             EggRegex               *regex,
                                             const char             *replacement,
                                             gboolean                replacement_literal,
                                             MooTextReplaceFunc      func,
                                             gpointer                func_data);
int      moo_text_replace_all_interactive   (GtkTextIter            *start,
                                             GtkTextIter            *end,
                                             const char             *text,
                                             const char             *replacement,
                                             MooTextSearchFlags      flags,
                                             MooTextReplaceFunc      func,
                                             gpointer                func_data);


G_END_DECLS

#endif /* __MOO_TEXT_SEARCH_H__ */
