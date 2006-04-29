/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   moolang-rules.c
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

#define MOOEDIT_COMPILATION
#include "mooedit/moolang-rules.h"
#include "mooedit/moolang-aux.h"


#if 0 && defined(MOO_DEBUG)
#define MOO_PROFILE
#endif

#ifdef MOO_PROFILE
static struct {
    GTimer *timer;
    gboolean time_rules_array_match;
    GHashTable *times;
    guint dump_timeout;
    gboolean did_something;
} profile;


typedef struct {
    const char *description;
    double time;
} Info;

static void
prepend_info (const char *description,
              double     *time,
              GSList    **list)
{
    if (*time > .001)
    {
        Info *i = g_new (Info, 1);
        i->description = description;
        i->time = *time;
        *list = g_slist_prepend (*list, i);
    }
}

static int
cmp_times (Info *i1,
           Info *i2)
{
    return i1->time < i2->time ? 1 : (i1->time > i2->time ? -1 : 0);
}

static gboolean
dump_profile (void)
{
    GSList *list;

    if (!profile.did_something)
    {
        g_hash_table_destroy (profile.times);
        profile.times = NULL;
        g_timer_destroy (profile.timer);
        profile.timer = NULL;
        profile.dump_timeout = 0;
        return FALSE;
    }

    profile.did_something = FALSE;

    list = NULL;
    g_hash_table_foreach (profile.times, (GHFunc) prepend_info, &list);
    list = g_slist_sort (list, (GCompareFunc) cmp_times);

    g_print ("Highlighting profile ------------------------------\n");

    while (list)
    {
        Info *i = list->data;
        g_print ("%.3f: %s\n", i->time, i->description);
        g_free (i);
        list = g_slist_delete_link (list, list);
    }

    g_print ("---------------------------------------------------\n");

    return TRUE;
}
#endif


typedef MooRuleMatchFlags MatchFlags;
#define MATCH_START_ONLY MOO_RULE_MATCH_START_ONLY

#define MooRuleString MooRuleAsciiString
#define MooRuleChar MooRuleAsciiChar
#define MooRule2Char MooRuleAscii2Char
#define MooRuleAnyChar MooRuleAsciiAnyChar

typedef MooRule* (*MatchFunc)   (MooRule                *self,
                                 const MooRuleMatchData *data,
                                 MooRuleMatchResult     *result,
                                 MooRuleMatchFlags       flags);
typedef void     (*DestroyFunc) (MooRule                *self);


static MooRule *rule_new            (MooRuleFlags    flags,
                                     const char     *style,
                                     MatchFunc       match_func,
                                     DestroyFunc     destroy_func);


static void     child_rules_match   (MooRuleArray       *array,
                                     MatchData          *data,
                                     MatchResult        *result);
static MooRule *rules_match_real    (MooRuleArray       *array,
                                     MatchData          *data,
                                     MatchResult        *result,
                                     MatchFlags          flags);


void
_moo_match_data_init (MatchData          *data,
                      int                 line_number,
                      const GtkTextIter  *line_start,
                      const GtkTextIter  *line_end)
{
    GtkTextBuffer *buffer;

    g_assert (data != NULL);
    g_assert (line_start && gtk_text_iter_starts_line (line_start));
    g_assert (line_number == gtk_text_iter_get_line (line_start));
    g_assert (!line_end || gtk_text_iter_ends_line (line_end));

    data->line_start = *line_start;
    data->line_number = line_number;

    if (line_end)
    {
        data->line_end = *line_end;
    }
    else
    {
        data->line_end = *line_start;
        if (!gtk_text_iter_ends_line (&data->line_end))
            gtk_text_iter_forward_to_line_end (&data->line_end);
    }

    buffer = gtk_text_iter_get_buffer (line_start);
    data->line_string = gtk_text_buffer_get_slice (buffer, line_start, &data->line_end, TRUE);
    data->line_string_len = strlen (data->line_string);

    data->start_iter = *line_start;
    data->start = data->line_string;
    data->start_offset = 0;
}


void
_moo_match_data_set_start (MatchData          *data,
                           const GtkTextIter  *start_iter,
                           char               *start,
                           int                 start_offset)
{
    g_assert (data != NULL);
    g_assert (start != NULL);
    g_assert (start_offset >= 0);

    data->start = start;
    data->start_offset = start_offset;

    if (start_iter)
    {
        data->start_iter = *start_iter;
    }
    else
    {
        data->start_iter = data->line_start;
        gtk_text_iter_forward_chars (&data->start_iter, start_offset);
    }
}


void
_moo_match_data_destroy (MatchData *data)
{
    if (data->line_string)
        g_free (data->line_string);
}


static MooRule*
rules_match_real (MooRuleArray *array,
                  MatchData    *data,
                  MatchResult  *result,
                  MatchFlags    flags)
{
    guint i;
    MooRule *matched = NULL;
    MatchResult tmp;

    g_assert (array != NULL);

    if (!array->len)
        return NULL;

    g_assert (data->line_string_len >= 0);

    if (flags & MATCH_START_ONLY)
    {
        data->limit = data->start;
        data->limit_offset = 0;
    }
    else
    {
        data->limit = data->line_string + data->line_string_len; /* this points to the zero char, so it's fine */
        data->limit_offset = SIZE_NOT_SET;
    }

#ifdef MOO_PROFILE
    if (!profile.timer)
    {
        profile.timer = g_timer_new ();
        g_timer_stop (profile.timer);
        profile.time_rules_array_match = TRUE;
        profile.times = g_hash_table_new_full (g_str_hash, g_str_equal,
                                               g_free, g_free);
        profile.dump_timeout = g_timeout_add (5000, (GSourceFunc) dump_profile, NULL);
    }
#endif

    for (i = 0; i < array->len; ++i)
    {
        MooRule *rule = array->data[i];
        MooRule *matched_here = NULL;

        if (!(flags & MATCH_START_ONLY))
        {
            /* TODO: first-non-blank */
            if ((rule->flags & MOO_RULE_MATCH_FIRST_CHAR) && data->start != data->line_string)
                continue;
        }

        if ((rule->flags & MOO_RULE_MATCH_FIRST_LINE) && data->line_number != 0)
            continue;

#ifdef MOO_PROFILE
        if (profile.time_rules_array_match)
        {
            g_timer_start (profile.timer);
        }
#endif

        matched_here = rule->match (rule, data, &tmp, flags);

#ifdef MOO_PROFILE
        if (profile.time_rules_array_match)
        {
            double time;
            double *total;

            g_timer_stop (profile.timer);
            time = g_timer_elapsed (profile.timer, NULL);

            total = g_hash_table_lookup (profile.times, rule->description);

            if (!total)
            {
                total = g_new (double, 1);
                *total = .0;
                g_hash_table_insert (profile.times,
                                     g_strdup (rule->description),
                                     total);
            }

            *total += time;
            profile.did_something = TRUE;
        }
#endif

        if (matched_here)
        {
            if (!(flags & MATCH_START_ONLY))
            {
                if (!matched || tmp.match_start < result->match_start)
                {
                    matched = matched_here ? matched_here : rule;
                    *result = tmp;
                    data->limit = tmp.match_start;
                    data->limit_offset = tmp.match_offset;

                    if (data->limit == data->start)
                        break;

                    g_assert (data->limit_offset != 0);

                    data->limit = utf8_offset_to_pointer (data->limit, -1);

                    if (data->limit_offset > 0)
                        data->limit_offset -= 1;
                }

                if (tmp.match_start == data->start)
                    break;
            }
            else
            {
                matched = matched_here ? matched_here : rule;
                *result = tmp;
                break;
            }
        }
    }

    if (matched && matched->child_rules)
    {
#ifdef MOO_PROFILE
        gboolean old = profile.time_rules_array_match;
        profile.time_rules_array_match = FALSE;
#endif
        child_rules_match (matched->child_rules, data, result);
#ifdef MOO_PROFILE
        profile.time_rules_array_match = old;
#endif
    }

    return matched;
}


static void
child_rules_match (MooRuleArray *array,
                   MatchData    *data,
                   MatchResult  *result)
{
    MatchResult tmp;
    MooRule *matched;
    char *saved_start;

    g_assert (array != NULL);
    g_assert (result->match_start != NULL);
    g_assert (result->match_end != NULL);
    g_assert (result->match_start <= result->match_end);

    tmp = *result;
    saved_start = data->start;
    data->start = result->match_end;

    matched = rules_match_real (array, data, &tmp, MATCH_START_ONLY);

    if (matched)
    {
        g_return_if_fail (tmp.match_start == result->match_end);
        result->match_end = tmp.match_end;
        if (tmp.match_len >= 0)
        {
            if (result->match_len >= 0)
                result->match_len += tmp.match_len;
        }
        else
        {
            result->match_len = -1;
        }
    }

    data->start = saved_start;
}


MooRule*
_moo_rule_array_match (MooRuleArray       *array,
                       MatchData          *data,
                       MatchResult        *result)
{
    return rules_match_real (array, data, result, 0);
}


static MooRule*
rule_new (MooRuleFlags    flags,
          const char     *style,
          MatchFunc       match_func,
          DestroyFunc     destroy_func)
{
    MooRule *rule;

    g_return_val_if_fail (match_func != NULL, NULL);

    rule = g_new0 (MooRule, 1);
    rule->match = match_func;
    rule->destroy = destroy_func;
    rule->flags = flags;
    rule->style = g_strdup (style);

    return rule;
}


void
_moo_rule_free (MooRule *rule)
{
    guint i;

    if (!rule)
        return;

    if (rule->destroy)
        rule->destroy (rule);

    if (rule->child_rules)
    {
        for (i = 0; i < rule->child_rules->len; ++i)
            _moo_rule_free (rule->child_rules->data[i]);
        g_ptr_array_free ((GPtrArray*) rule->child_rules, TRUE);
    }

    g_free (rule->description);
    g_free (rule->style);
    g_free (rule);
}


void
_moo_rule_add_child_rule (MooRule   *rule,
                          MooRule   *child_rule)
{
    g_return_if_fail (rule != NULL && child_rule != NULL);

    if (!rule->child_rules)
        rule->child_rules = (MooRuleArray*) g_ptr_array_new ();

    g_ptr_array_add ((GPtrArray*) rule->child_rules, child_rule);
}


void
_moo_rule_set_end_stay (MooRule *rule)
{
    g_return_if_fail (rule != NULL);
    rule->exit.type = MOO_CONTEXT_STAY;
    rule->exit.num = 0;
}


void
_moo_rule_set_end_pop (MooRule *rule,
                       guint    num)
{
    g_return_if_fail (rule != NULL && num != 0);
    rule->exit.type = MOO_CONTEXT_POP;
    rule->exit.num = num;
}


void
_moo_rule_set_end_switch (MooRule    *rule,
                          MooContext *target)
{
    g_return_if_fail (rule != NULL && target != 0);
    rule->exit.type = MOO_CONTEXT_SWITCH;
    rule->exit.ctx = target;
}


/*************************************************************************/
/* String match
 */

static MooRule*
rule_string_match (MooRule         *rule,
                   const MatchData *data,
                   MatchResult     *result,
                   MatchFlags       flags)
{
    /* TODO: limit */

    result->match_start = NULL;

    if (rule->str.caseless)
    {
        if (flags & MATCH_START_ONLY)
        {
            if (!g_ascii_strncasecmp (data->start, rule->str.string, rule->str.length))
                result->match_start = data->start;
        }
        else
        {
            result->match_start = ascii_casestrstr (data->start, rule->str.string, data->limit);
        }
    }
    else
    {
        if (flags & MATCH_START_ONLY)
        {
            if (!strncmp (data->start, rule->str.string, rule->str.length))
                result->match_start = data->start;
        }
        else
        {
            result->match_start = strstr (data->start, rule->str.string);
        }
    }

    if (!result->match_start)
        return NULL;

    result->match_end = result->match_start + rule->str.length;
    result->match_len = rule->str.length;
    result->match_offset = -1;
    return rule;
}


static void
rule_string_destroy (MooRule *rule)
{
    g_free (rule->str.string);
}


MooRule*
_moo_rule_string_new (const char         *string,
                      MooRuleFlags        flags,
                      const char         *style)
{
    MooRule *rule;
    guint length;

    g_return_val_if_fail (string && string[0], NULL);
    g_return_val_if_fail (g_utf8_validate (string, -1, NULL), NULL);
    g_return_val_if_fail (string_is_ascii (string), NULL);

    length = strlen (string);

    g_return_val_if_fail (length != 0, NULL);

    rule = rule_new (flags, style, rule_string_match, rule_string_destroy);
    g_return_val_if_fail (rule != NULL, NULL);

    rule->description = g_strdup_printf ("STRING %s", string);

    rule->str.caseless = (flags & MOO_RULE_MATCH_CASELESS) ? TRUE : FALSE;

    if (rule->str.caseless)
        rule->str.string = g_ascii_strdown (string, -1);
    else
        rule->str.string = g_strdup (string);

    rule->str.length = length;

    return rule;
}


/*************************************************************************/
/* Regex match
 */

static MooRule*
rule_regex_match (MooRule        *rule,
                  const MatchData *data,
                  MatchResult    *result,
                  MatchFlags      flags)
{
    /* TODO: limit */
    /* XXX line start and stuff */
    int n_matches, start_pos, end_pos;
    EggRegexMatchFlags regex_flags = 0;
    char *start = data->start;

    if (flags & MATCH_START_ONLY)
        regex_flags |= EGG_REGEX_MATCH_ANCHORED;

    while (start <= data->limit)
    {
        egg_regex_clear (rule->regex.regex);

        n_matches = egg_regex_match_extended (rule->regex.regex,
                                              data->line_string,
                                              data->line_string_len,
                                              start - data->line_string,
                                              regex_flags);

        if (n_matches < 1)
            return NULL;

        egg_regex_fetch_pos (rule->regex.regex, data->line_string, 0,
                             &start_pos, &end_pos);

        if (data->line_string + start_pos > data->limit)
            return NULL;

        result->match_start = data->line_string + start_pos;
        result->match_end = data->line_string + end_pos;
        result->match_len = -1;
        result->match_offset = -1;

        if (rule->regex.left_word_bndry && result->match_start > data->line_string &&
            CHAR_IS_WORD (result->match_start[0]) && CHAR_IS_WORD (result->match_start[-1]))
        {
            start = result->match_start + 1;
            continue;
        }

        if (rule->regex.right_word_bndry && result->match_end > data->line_string &&
            CHAR_IS_WORD (result->match_end[0]) && CHAR_IS_WORD (result->match_end[-1]))
        {
            start = result->match_start + 1;
            continue;
        }

        return rule;
    }

    return NULL;
}


static void
rule_regex_destroy (MooRule *rule)
{
    egg_regex_free (rule->regex.regex);
}


MooRule*
_moo_rule_regex_new (const char         *pattern,
                     gboolean            non_empty,
                     EggRegexCompileFlags regex_compile_options,
                     EggRegexMatchFlags  regex_match_options,
                     MooRuleFlags        flags,
                     const char         *style)
{
    MooRule *rule;
    EggRegex *regex;
    GError *error = NULL;

    g_return_val_if_fail (pattern && pattern[0], NULL);

    if (flags & MOO_RULE_MATCH_CASELESS)
        regex_compile_options |= EGG_REGEX_CASELESS;

    if (non_empty)
        regex_match_options |= EGG_REGEX_MATCH_NOTEMPTY;

    regex = egg_regex_new (pattern, regex_compile_options,
                           regex_match_options, &error);

    if (!regex)
    {
        g_warning ("could not compile pattern '%s': %s",
                   pattern, error->message);
        g_error_free (error);
        return NULL;
    }

    egg_regex_optimize (regex, &error);

    if (error)
    {
        g_warning ("egg_regex_optimize() failed: %s", error->message);
        g_error_free (error);
    }

    if (pattern[0] == '^')
        flags |= MOO_RULE_MATCH_FIRST_CHAR;

    rule = rule_new (flags, style, rule_regex_match, rule_regex_destroy);

    if (!rule)
    {
        egg_regex_free (regex);
        return NULL;
    }

    rule->description = g_strdup_printf ("REGEX %s", pattern);

    rule->regex.regex = regex;

    return rule;
}


/*************************************************************************/
/* Char match
 */

static MooRule*
rule_char_match (MooRule         *rule,
                 const MatchData *data,
                 MatchResult     *result,
                 MatchFlags       flags)
{
    result->match_start = NULL;

    if (flags & MATCH_START_ONLY)
    {
        if (rule->_char.caseless)
        {
            if (data->start[0] == rule->_char.ch)
                result->match_start = data->start;
        }
        else
        {
            if (g_ascii_tolower (data->start[0]) == rule->_char.ch)
                result->match_start = data->start;
        }
    }
    else
    {
        if (rule->_char.caseless)
            result->match_start = ascii_lower_strchr (data->start, rule->_char.ch, data->limit);
        else
            result->match_start = ascii_strchr (data->start, rule->_char.ch, data->limit);
    }

    if (!result->match_start)
        return NULL;

    result->match_end = result->match_start + 1;
    result->match_len = 1;
    result->match_offset = -1;

    return rule;
}


static MooRule*
rule_2char_match (MooRule         *rule,
                  const MatchData *data,
                  MatchResult     *result,
                  MatchFlags       flags)
{
    result->match_start = NULL;

    if (flags & MATCH_START_ONLY)
    {
        if (data->start[0] == rule->_2char.str[0] && data->start[1] == rule->_2char.str[1])
            result->match_start = data->start;
    }
    else
    {
        result->match_start = strstr (data->start, rule->_2char.str);
    }

    if (!result->match_start)
        return NULL;

    result->match_end = result->match_start + 2;
    result->match_len = 2;
    result->match_offset = -1;
    return rule;
}


MooRule*
_moo_rule_char_new (char                ch,
                    MooRuleFlags        flags,
                    const char         *style)
{
    MooRule *rule;

    g_return_val_if_fail (ch && CHAR_IS_ASCII (ch), NULL);

    rule = rule_new (flags, style, rule_char_match, NULL);
    g_return_val_if_fail (rule != NULL, NULL);

    rule->description = g_strdup_printf ("CHAR %c", ch);

    if (flags & MOO_RULE_MATCH_CASELESS)
    {
        rule->_char.ch = g_ascii_tolower (ch);
        rule->_char.caseless = TRUE;
    }
    else
    {
        rule->_char.ch = ch;
    }

    return rule;
}


MooRule*
_moo_rule_2char_new (char                ch1,
                     char                ch2,
                     MooRuleFlags        flags,
                     const char         *style)
{
    MooRule *rule;

    g_return_val_if_fail (ch1 && CHAR_IS_ASCII (ch1), NULL);
    g_return_val_if_fail (ch2 && CHAR_IS_ASCII (ch2), NULL);

    rule = rule_new (flags, style, rule_2char_match, NULL);
    g_return_val_if_fail (rule != NULL, NULL);

    rule->description = g_strdup_printf ("TWOCHARS %c%c", ch1, ch2);

    if (flags & MOO_RULE_MATCH_CASELESS)
    {
        ch1 = g_ascii_tolower (ch1);
        ch2 = g_ascii_tolower (ch2);
    }

    rule->_2char.str[0] = ch1;
    rule->_2char.str[1] = ch2;
    rule->_2char.str[2] = 0;

    return rule;
}


/*************************************************************************/
/* AnyChar match
 */

static MooRule*
rule_any_char_match (MooRule         *rule,
                     const MatchData *data,
                     MatchResult     *result,
                     MatchFlags       flags)
{
    guint i;

    result->match_start = NULL;

    if (flags & MATCH_START_ONLY)
    {
        for (i = 0; i < rule->anychar.n_chars; ++i)
        {
            if (data->start[0] == rule->anychar.chars[i])
            {
                result->match_start = data->start;
                break;
            }
        }
    }
    else
    {
        for (i = 0; i < rule->anychar.n_chars; ++i)
        {
            if (!result->match_start)
            {
                result->match_start = ascii_strchr (data->start, rule->anychar.chars[i], data->limit);
            }
            else if (result->match_start == data->start + 1)
            {
                if (data->start[0] == rule->anychar.chars[i])
                {
                    result->match_start = data->start;
                    break;
                }
            }
            else
            {
                char *tmp = ascii_strchr (data->start, rule->anychar.chars[i], data->limit);
                if (tmp < result->match_start)
                    result->match_start = tmp;
            }

            if (result->match_start == data->start)
                break;
        }
    }

    if (!result->match_start)
        return NULL;

    result->match_end = result->match_start + 1;
    result->match_len = 1;
    result->match_offset = -1;
    return rule;
}


static void
rule_any_char_destroy (MooRule *rule)
{
    g_free (rule->anychar.chars);
}


MooRule*
_moo_rule_any_char_new (const char         *string,
                        MooRuleFlags        flags,
                        const char         *style)
{
    MooRule *rule;
    guint i, len;

    g_return_val_if_fail (string && string[0], NULL);

    len = strlen (string);

    for (i = 0; i < len; ++i)
        g_return_val_if_fail (CHAR_IS_ASCII (string[i]), NULL);

    rule = rule_new (flags, style, rule_any_char_match, rule_any_char_destroy);
    g_return_val_if_fail (rule != NULL, NULL);

    rule->description = g_strdup_printf ("ANYCHAR %s", string);

    rule->anychar.n_chars = len;
    rule->anychar.chars = g_strdup (string);

    return rule;
}


/*************************************************************************/
/* Keywords
 */

MooRule*
_moo_rule_keywords_new (GSList             *words,
                        MooRuleFlags        flags,
                        const char         *prefix,
                        const char         *suffix,
                        gboolean            word_boundary,
                        const char         *style)
{
    GSList *l;
    GString *pattern;
    MooRule *rule = NULL;

    g_return_val_if_fail (words != NULL, NULL);

    pattern = g_string_new (NULL);
    g_string_printf (pattern, "%s(",
                     prefix ? prefix : "");

    for (l = words; l != NULL; l = l->next)
    {
        char *word = l->data;

        if (!word || !word[0])
        {
            g_warning ("%s: empty keyword", G_STRLOC);
            goto out;
        }

        if (l != words)
            g_string_append_c (pattern, '|');

        g_string_append (pattern, word);
    }

    g_string_append_printf (pattern, ")%s",
                            suffix ? suffix : "");

    rule = _moo_rule_regex_new (pattern->str, TRUE, 0, 0, flags, style);
    g_return_val_if_fail (rule != NULL, NULL);

    if (word_boundary)
    {
        rule->regex.left_word_bndry = TRUE;
        rule->regex.right_word_bndry = TRUE;
    }

out:
    g_string_free (pattern, TRUE);
    return rule;
}


/*************************************************************************/
/* IncludeRules
 */

static MooRule*
rule_include_match (MooRule         *rule,
                    const MatchData *data,
                    MatchResult     *result,
                    MatchFlags       flags)
{
    return rules_match_real (rule->incl.ctx->rules,
                             (MatchData*) data, result, flags);
}


MooRule*
_moo_rule_include_new (MooContext *ctx)
{
    MooRule *rule;

    g_return_val_if_fail (ctx != NULL, NULL);

    rule = rule_new (0, NULL, rule_include_match, NULL);
    g_return_val_if_fail (rule != NULL, NULL);

    rule->description = g_strdup_printf ("INCLUDE %s", ctx->name);

    rule->incl.ctx = ctx;

    return rule;
}


/*************************************************************************/
/* Special sequences
 */

static MooRule*
rule_int_match (MooRule         *rule,
                const MatchData *data,
                MatchResult     *result,
                MatchFlags       flags)
{
    guint i;
    char *limit = data->limit;
    char *start = data->start;

    if (flags & MATCH_START_ONLY)
        limit = start;

    while (start <= limit)
    {
        while (start <= limit && !CHAR_IS_DIGIT (*start))
            start++;

        if (start > limit)
            return NULL;

        for (i = 1; CHAR_IS_DIGIT (start[i]); ++i) ;

        result->match_start = start;
        result->match_end = start + i;
        result->match_len = i;
        result->match_offset = -1;
        return rule;
    }

    return NULL;
}


MooRule*
_moo_rule_int_new (MooRuleFlags   flags,
                   const char    *style)
{
    MooRule *rule = rule_new (flags, style, rule_int_match, NULL);
    g_return_val_if_fail (rule != NULL, NULL);

    rule->description = g_strdup ("INT");

    return rule;
}


static MooRule*
rule_float_match (MooRule         *rule,
                  const MatchData *data,
                  MatchResult     *result,
                  MatchFlags       flags)
{
    guint i;
    char *limit = data->limit;
    char *start = data->start;

    if (flags & MATCH_START_ONLY)
        limit = start;

    while (start <= limit)
    {
        while (start <= limit && !CHAR_IS_DIGIT (*start) && *start != '.')
            start++;

        if (start > limit)
            return NULL;

        if (*start == '.')
        {
            if (start > data->line_string && CHAR_IS_DIGIT (start[-1]))
            {
                do start++;
                while (start <= limit && CHAR_IS_DIGIT (*start));
                continue;
            }

            if (!CHAR_IS_DIGIT (start[1]))
            {
                start++;
                continue;
            }

            for (i = 2; CHAR_IS_DIGIT (start[i]); ++i) ;

            result->match_start = start;
            result->match_end = start + i;
            result->match_len = i;
            result->match_offset = -1;
            return rule;
        }
        else
        {
            for (i = 1; CHAR_IS_DIGIT (start[i]); ++i) ;

            if (start[i] != '.')
            {
                start = start + i;
                continue;
            }

            for (i = i + 1; CHAR_IS_DIGIT (start[i]); ++i) ;

            result->match_start = start;
            result->match_end = start + i;
            result->match_len = i;
            result->match_offset = -1;
            return rule;
        }
    }

    return NULL;
}


#define PATTERN_OCTAL       "0[0-7]+"
#define PATTERN_HEX         "0x[0-9A-Fa-f]+"
#define PATTERN_ESC_CHAR    "\\\\([abefnrtv\"'?\\\\]|0[0-7]*|x[0-9A-Fa-f])"
#define PATTERN_C_CHAR      "'" PATTERN_ESC_CHAR "'"
#define PATTERN_IDENTIFIER  "[a-zA-Z_][a-zA-Z0-9_]*"
#define PATTERN_WHITESPACE  "\\s+"


MooRule*
_moo_rule_float_new (MooRuleFlags        flags,
                     const char         *style)
{
    MooRule *rule = rule_new (flags, style, rule_float_match, NULL);
    g_return_val_if_fail (rule != NULL, NULL);

    rule->description = g_strdup ("FLOAT");

    return rule;
}


static MooRule*
rule_octal_match (MooRule         *rule,
                  const MatchData *data,
                  MatchResult     *result,
                  MatchFlags       flags)
{
    guint i;
    char *limit = data->limit;
    char *start = data->start;

    if (flags & MATCH_START_ONLY)
        limit = start;

    while (start <= limit)
    {
        while (start <= limit && *start != '0')
            start++;

        if (start > limit)
            return NULL;

        for (i = 1; CHAR_IS_OCTAL (start[i]); ++i) ;

        if (i < 2)
        {
            start = start + i;
            continue;
        }

        result->match_start = start;
        result->match_end = start + i;
        result->match_len = i;
        result->match_offset = -1;
        return rule;
    }

    return NULL;
}


MooRule*
_moo_rule_octal_new (MooRuleFlags        flags,
                     const char         *style)
{
    MooRule *rule = rule_new (flags, style, rule_octal_match, NULL);
    g_return_val_if_fail (rule != NULL, NULL);

    rule->description = g_strdup ("OCTAL");

    return rule;
}


static MooRule*
rule_hex_match (MooRule         *rule,
                const MatchData *data,
                MatchResult     *result,
                MatchFlags       flags)
{
    guint i;
    char *limit = data->limit;
    char *start = data->start;

    if (flags & MATCH_START_ONLY)
        limit = start;

    while (start <= limit)
    {
        while (start <= limit && *start != '0')
            start++;

        if (start > limit)
            return NULL;

        if (start[1] != 'x' && start[1] != 'X')
        {
            start += 2;
            continue;
        }

        for (i = 2; CHAR_IS_HEX (start[i]); ++i) ;

        result->match_start = start;
        result->match_end = start + i;
        result->match_len = i;
        result->match_offset = -1;
        return rule;
    }

    return NULL;
}


MooRule*
_moo_rule_hex_new (MooRuleFlags        flags,
                   const char         *style)
{
    MooRule *rule = rule_new (flags, style, rule_hex_match, NULL);
    g_return_val_if_fail (rule != NULL, NULL);

    rule->description = g_strdup ("HEX");

    return rule;
}


static MooRule*
rule_escaped_char_match (MooRule         *rule,
                         const MatchData *data,
                         MatchResult     *result,
                         MatchFlags       flags)
{
    guint i;
    char *limit = data->limit;
    char *start = data->start;

    if (flags & MATCH_START_ONLY)
        limit = start;

    while (start <= limit)
    {
        while (start <= limit && *start != '\\')
            start++;

        if (start > limit)
            return NULL;

        switch (start[1])
        {
            case '\\':
            case 'a':
            case 'b':
            case 'e':
            case 'f':
            case 'n':
            case 'r':
            case 't':
            case 'v':
            case '\"':
            case '\'':
            case '?':
                result->match_start = start;
                result->match_end = start + 2;
                result->match_len = 2;
                result->match_offset = -1;
                return rule;

            case '0':
                for (i = 2; CHAR_IS_OCTAL (start[i]); ++i) ;

                result->match_start = start;
                result->match_end = start + i;
                result->match_len = i;
                result->match_offset = -1;
                return rule;

            case 'x':
            case 'X':
                for (i = 2; CHAR_IS_HEX (start[i]); ++i) ;

                result->match_start = start;
                result->match_end = start + i;
                result->match_len = i;
                result->match_offset = -1;
                return rule;
        }

        start++;
    }

    return NULL;
}


MooRule*
_moo_rule_escaped_char_new (MooRuleFlags        flags,
                            const char         *style)
{
    MooRule *rule = rule_new (flags, style, rule_escaped_char_match, NULL);
    g_return_val_if_fail (rule != NULL, NULL);
    rule->description = g_strdup ("ESCAPED CHAR");
    return rule;
}


static MooRule*
rule_c_char_match (MooRule         *rule,
                   const MatchData *data,
                   MatchResult     *result,
                   MatchFlags       flags)
{
    guint i;
    char *limit = data->limit;
    char *start = data->start;

    if (flags & MATCH_START_ONLY)
        limit = start;

    while (start <= limit)
    {
        while (start <= limit && *start != '\'')
            start++;

        if (start > limit)
            return NULL;

        if (start[1] != '\\')
        {
            start++;
            continue;
        }

        switch (start[2])
        {
            case '\\':
            case 'a':
            case 'b':
            case 'e':
            case 'f':
            case 'n':
            case 'r':
            case 't':
            case 'v':
            case '\"':
            case '\'':
            case '?':
                if (start[3] != '\'')
                {
                    start = start + 3;
                    continue;
                }

                result->match_start = start;
                result->match_end = start + 4;
                result->match_len = 4;
                result->match_offset = -1;
                return rule;

            case '0':
                for (i = 3; CHAR_IS_OCTAL (start[i]); ++i) ;

                if (start[i] != '\'')
                {
                    start = start + i;
                    continue;
                }

                result->match_start = start;
                result->match_end = start + i + 1;
                result->match_len = i + 1;
                result->match_offset = -1;
                return rule;

            case 'x':
            case 'X':
                for (i = 3; CHAR_IS_HEX (start[i]); ++i) ;

                if (start[i] != '\'')
                {
                    start = start + i;
                    continue;
                }

                result->match_start = start;
                result->match_end = start + i + 1;
                result->match_len = i + 1;
                result->match_offset = -1;
                return rule;
        }

        start++;
    }

    return NULL;
}


MooRule*
_moo_rule_c_char_new (MooRuleFlags        flags,
                      const char         *style)
{
    MooRule *rule = rule_new (flags, style, rule_c_char_match, NULL);
    g_return_val_if_fail (rule != NULL, NULL);
    rule->description = g_strdup ("C CHAR");
    return rule;
}


static MooRule*
rule_whitespace_match (MooRule         *rule,
                       const MatchData *data,
                       MatchResult     *result,
                       G_GNUC_UNUSED MatchFlags flags)
{
    guint i;
    char *start = data->start;

    if (!CHAR_IS_SPACE (*start))
        return NULL;

    for (i = 1; CHAR_IS_SPACE (start[i]); ++i) ;

    result->match_start = start;
    result->match_end = start + i;
    result->match_len = i;
    result->match_offset = -1;
    return rule;
}


MooRule*
_moo_rule_whitespace_new (MooRuleFlags        flags,
                          const char         *style)
{
    MooRule *rule = rule_new (flags, style, rule_whitespace_match, NULL);
    g_return_val_if_fail (rule != NULL, NULL);
    rule->description = g_strdup ("WHITESPACE");
    return rule;
}


static MooRule*
rule_identifier_match (MooRule         *rule,
                       const MatchData *data,
                       MatchResult     *result,
                       MatchFlags       flags)
{
    guint i;
    char *limit = data->limit;
    char *start = data->start;

    if (flags & MATCH_START_ONLY)
        limit = start;

    while (start <= limit)
    {
        while (start <= limit && (!CHAR_IS_WORD (*start) || CHAR_IS_DIGIT (*start)))
            start++;

        if (start > limit)
            return NULL;

        for (i = 1; CHAR_IS_WORD (start[i]); ++i) ;

        result->match_start = start;
        result->match_end = start + i;
        result->match_len = i;
        result->match_offset = -1;
        return rule;
    }

    return NULL;
}


MooRule*
_moo_rule_identifier_new (MooRuleFlags        flags,
                          const char         *style)
{
    MooRule *rule = rule_new (flags, style, rule_identifier_match, NULL);
    g_return_val_if_fail (rule != NULL, NULL);
    rule->description = g_strdup ("WHITESPACE");
    return rule;
}


static MooRule*
rule_line_continue_match (MooRule         *rule,
                          const MatchData *data,
                          MatchResult     *result,
                          MatchFlags       flags)
{
    char *limit = data->limit;
    char *start;

    if (flags & MATCH_START_ONLY)
        limit = data->start;

    g_assert (data->line_string_len >= 0);

    if (data->line_string_len && data->line_string[data->line_string_len - 1] == '\\')
    {
        start = data->line_string + data->line_string_len - 1;

        if (start > limit)
            return NULL;

        result->match_start = start;
        result->match_end = start + 1;
        result->match_len = 1;
        result->match_offset = -1;
        return rule;
    }

    return NULL;
}


MooRule*
_moo_rule_line_continue_new (MooRuleFlags        flags,
                             const char         *style)
{
    MooRule *rule = rule_new (flags, style, rule_line_continue_match, NULL);
    g_return_val_if_fail (rule != NULL, NULL);
    rule->description = g_strdup ("LINE_CONTINUE");
    rule->include_eol = TRUE;
    return rule;
}
