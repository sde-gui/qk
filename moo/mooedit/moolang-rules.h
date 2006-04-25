/*
 *   moolang-rules.h
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

#ifndef __MOO_LANG_RULES_H__
#define __MOO_LANG_RULES_H__

#ifndef MOOEDIT_COMPILATION
#error "This file may not be included"
#endif

#include "mooedit/moolang.h"
#include "mooutils/eggregex.h"
#include <gtk/gtk.h>


#define SIZE_UNKNOWN -1
#define SIZE_NOT_SET -2

struct _MooRuleMatchData {
    int line_number;
    GtkTextIter line_start;
    GtkTextIter line_end;
    char *line_string;
    int line_string_len; /* byte len */

    GtkTextIter start_iter;
    char *start; /* points to some byte in line_string */
    int start_offset; /* chars from line_start */

    char *limit; /* rule matches only if match is <= limit */
    int limit_offset; /* chars from start */
};


struct _MooRuleMatchResult {
    char *match_start;
    char *match_end;
    int match_len;    /* chars */
    int match_offset; /* chars from start */
};

typedef MooRuleMatchData MatchData;
typedef MooRuleMatchResult MatchResult;

MooRule    *moo_rule_string_new         (const char         *string,
                                         MooRuleFlags        flags,
                                         const char         *style);
MooRule    *moo_rule_regex_new          (const char         *pattern,
                                         gboolean            non_empty,
                                         EggRegexCompileFlags regex_compile_options,
                                         EggRegexMatchFlags  regex_match_options,
                                         MooRuleFlags        flags,
                                         const char         *style);
MooRule    *moo_rule_char_new           (char                ch,
                                         MooRuleFlags        flags,
                                         const char         *style);
MooRule    *moo_rule_2char_new          (char                ch1,
                                         char                ch2,
                                         MooRuleFlags        flags,
                                         const char         *style);
MooRule    *moo_rule_any_char_new       (const char         *string,
                                         MooRuleFlags        flags,
                                         const char         *style);
MooRule    *moo_rule_keywords_new       (GSList             *words,
                                         MooRuleFlags        flags,
                                         const char         *style);
MooRule    *moo_rule_zero_new           (MooRuleFlags        flags);
MooRule    *moo_rule_include_new        (MooContext         *context);

MooRule    *moo_rule_int_new            (MooRuleFlags        flags,
                                         const char         *style);
MooRule    *moo_rule_float_new          (MooRuleFlags        flags,
                                         const char         *style);
MooRule    *moo_rule_octal_new          (MooRuleFlags        flags,
                                         const char         *style);
MooRule    *moo_rule_hex_new            (MooRuleFlags        flags,
                                         const char         *style);
MooRule    *moo_rule_escaped_char_new   (MooRuleFlags        flags,
                                         const char         *style);
MooRule    *moo_rule_c_char_new         (MooRuleFlags        flags,
                                         const char         *style);
MooRule    *moo_rule_whitespace_new     (MooRuleFlags        flags,
                                         const char         *style);
MooRule    *moo_rule_identifier_new     (MooRuleFlags        flags,
                                         const char         *style);
MooRule    *moo_rule_line_continue_new  (MooRuleFlags        flags,
                                         const char         *style);

void        moo_rule_add_child_rule     (MooRule            *rule,
                                         MooRule            *child_rule);
void        moo_rule_set_end_stay       (MooRule            *rule);
void        moo_rule_set_end_pop        (MooRule            *rule,
                                         guint               num);
void        moo_rule_set_end_switch     (MooRule            *rule,
                                         MooContext         *target);
#if 0
void        moo_rule_set_end_jump       (MooRule            *rule,
                                         MooContext         *target);
#endif


void        moo_rule_free               (MooRule            *rule);


MooRule    *moo_rule_array_match        (MooRuleArray       *array,
                                         MatchData          *data,
                                         MatchResult        *result);

void        moo_match_data_init         (MatchData          *data,
                                         int                 line_number,
                                         const GtkTextIter  *line_start,
                                         const GtkTextIter  *line_end); /* line_end may be NULL */
void        moo_match_data_set_start    (MatchData          *data,
                                         const GtkTextIter  *start_iter,
                                         char               *start,
                                         int                 start_offset); /* chars offset */
gboolean    moo_match_data_line_start   (MatchData          *data);
void        moo_match_data_destroy      (MatchData          *data);


#endif /* __MOO_LANG_RULES_H__ */
