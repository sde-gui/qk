/*
 *   mooedit/mooeditsearch.h
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

#ifndef MOOEDIT_MOOEDITSEARCH_H
#define MOOEDIT_MOOEDITSEARCH_H

#include "mooedit/mooedit.h"
#include "mooutils/eggregex.h"

G_BEGIN_DECLS


#define MOO_TYPE_EDIT_SEARCH_OPTIONS        (moo_edit_search_options_get_type ())
#define MOO_TYPE_EDIT_REPLACE_RESPONSE_TYPE (moo_edit_replace_response_type_get_type ())
GType moo_edit_search_options_get_type          (void) G_GNUC_UNUSED;
GType moo_edit_replace_response_type_get_type   (void) G_GNUC_UNUSED;


typedef enum {
    MOO_EDIT_SEARCH_BACKWARDS           = 1 << 0,
    MOO_EDIT_SEARCH_CASE_INSENSITIVE    = 1 << 1,
    MOO_EDIT_SEARCH_REGEX               = 1 << 2
} MooEditSearchOptions;


gboolean    moo_edit_search                 (const GtkTextIter  *start,
                                             const GtkTextIter  *limit,
                                             const char         *text,
                                             GtkTextIter        *match_start,
                                             GtkTextIter        *match_end,
                                             MooEditSearchOptions options,
                                             GError            **error);

gboolean    moo_edit_search_regex           (const GtkTextIter  *start,
                                             const GtkTextIter  *limit,
                                             EggRegex           *regex,
                                             GtkTextIter        *match_start,
                                             GtkTextIter        *match_end,
                                             gboolean            backwards);


#define MOO_EDIT_REPLACE_INVALID_ARGS   ((int)-1)
#define MOO_EDIT_REPLACE_REGEX_ERROR    ((int)-2)

/* Do not change numerical values!!! */
typedef enum {
    MOO_EDIT_REPLACE_STOP       = 0,
    MOO_EDIT_REPLACE_AND_STOP   = 1,
    MOO_EDIT_REPLACE_SKIP       = 2,
    MOO_EDIT_REPLACE            = 3,
    MOO_EDIT_REPLACE_ALL        = 4
} MooEditReplaceResponseType;

/* replacement is real replacement text when searching for regex */
typedef MooEditReplaceResponseType (* MooEditReplaceFunc)
                                            (const char         *text,
                                             EggRegex           *regex,
                                             const char         *replacement,
                                             GtkTextIter        *to_replace_start,
                                             GtkTextIter        *to_replace_end,
                                             gpointer            data);

MooEditReplaceResponseType moo_edit_replace_func_replace_all
                                            (const char         *text,
                                             EggRegex           *regex,
                                             const char         *replacement,
                                             GtkTextIter        *to_replace_start,
                                             GtkTextIter        *to_replace_end,
                                             gpointer            data);

int         moo_edit_replace_all_interactive(GtkTextIter        *start,
                                             GtkTextIter        *end,
                                             const char         *text,
                                             const char         *replacement,
                                             MooEditSearchOptions options,
                                             GError            **error,
                                             MooEditReplaceFunc  func,
                                             gpointer            data);

int         moo_edit_replace_regex_all_interactive
                                            (GtkTextIter        *start,
                                             GtkTextIter        *end,
                                             EggRegex           *regex,
                                             const char         *replacement,
                                             gboolean            backwards,
                                             GError            **error,
                                             MooEditReplaceFunc  func,
                                             gpointer            data);


void        _moo_edit_find                  (MooEdit            *edit);
void        _moo_edit_replace               (MooEdit            *edit);
void        _moo_edit_find_next             (MooEdit            *edit);
void        _moo_edit_find_previous         (MooEdit            *edit);


G_END_DECLS

#endif /* MOOEDIT_MOOEDITSEARCH_H */
