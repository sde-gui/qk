/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mooeditsearch.h
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

#ifndef __MOO_EDIT_SEARCH_H__
#define __MOO_EDIT_SEARCH_H__

#include <mooedit/mooedit.h>
#include <mooedit/mootextview.h>
#include <mooutils/eggregex.h>

G_BEGIN_DECLS


#define MOO_TYPE_TEXT_SEARCH_OPTIONS        (moo_text_search_options_get_type ())
#define MOO_TYPE_TEXT_REPLACE_RESPONSE_TYPE (moo_text_replace_response_type_get_type ())
GType moo_text_search_options_get_type          (void) G_GNUC_UNUSED;
GType moo_text_replace_response_type_get_type   (void) G_GNUC_UNUSED;


typedef enum {
    MOO_TEXT_SEARCH_BACKWARDS           = 1 << 0,
    MOO_TEXT_SEARCH_CASE_INSENSITIVE    = 1 << 1,
    MOO_TEXT_SEARCH_REGEX               = 1 << 2
} MooTextSearchOptions;


gboolean    moo_text_search                 (const GtkTextIter  *start,
                                             const GtkTextIter  *limit,
                                             const char         *text,
                                             GtkTextIter        *match_start,
                                             GtkTextIter        *match_end,
                                             MooTextSearchOptions options,
                                             GError            **error);

gboolean    moo_text_search_regex           (const GtkTextIter  *start,
                                             const GtkTextIter  *limit,
                                             EggRegex           *regex,
                                             GtkTextIter        *match_start,
                                             GtkTextIter        *match_end,
                                             gboolean            backwards);


#define MOO_TEXT_REPLACE_INVALID_ARGS   ((int)-1)
#define MOO_TEXT_REPLACE_REGEX_ERROR    ((int)-2)

/* Do not change numerical values!!! */
typedef enum {
    MOO_TEXT_REPLACE_STOP       = 0,
    MOO_TEXT_REPLACE_AND_STOP   = 1,
    MOO_TEXT_REPLACE_SKIP       = 2,
    MOO_TEXT_REPLACE            = 3,
    MOO_TEXT_REPLACE_ALL        = 4
} MooTextReplaceResponseType;

/* replacement is real replacement text when searching for regex */
typedef MooTextReplaceResponseType (* MooTextReplaceFunc) (const char         *text,
                                                           EggRegex           *regex,
                                                           const char         *replacement,
                                                           GtkTextIter        *to_replace_start,
                                                           GtkTextIter        *to_replace_end,
                                                           gpointer            data);

MooTextReplaceResponseType moo_text_replace_func_replace_all
                                            (const char         *text,
                                             EggRegex           *regex,
                                             const char         *replacement,
                                             GtkTextIter        *to_replace_start,
                                             GtkTextIter        *to_replace_end,
                                             gpointer            data);

int         moo_text_replace_all_interactive(GtkTextIter        *start,
                                             GtkTextIter        *end,
                                             const char         *text,
                                             const char         *replacement,
                                             MooTextSearchOptions options,
                                             GError            **error,
                                             MooTextReplaceFunc  func,
                                             gpointer            data);

int         moo_text_replace_regex_all_interactive
                                            (GtkTextIter        *start,
                                             GtkTextIter        *end,
                                             EggRegex           *regex,
                                             const char         *replacement,
                                             gboolean            backwards,
                                             GError            **error,
                                             MooTextReplaceFunc  func,
                                             gpointer            data);


void        _moo_text_view_find             (MooTextView        *edit);
void        _moo_text_view_replace          (MooTextView        *edit);
void        _moo_text_view_find_next        (MooTextView        *edit);
void        _moo_text_view_find_previous    (MooTextView        *edit);


G_END_DECLS

#endif /* __MOO_EDIT_SEARCH_H__ */
