/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mooeditsearch.c
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

#define MOOEDIT_COMPILATION
#include "mooedit/mooedit-private.h"
#include "mooedit/mootextiter.h"
#include "mooedit/gtksourceiter.h"
#include <string.h>


GType
moo_text_search_options_get_type (void)
{
    static GType type = 0;

    if (!type)
    {
        static const GFlagsValue values[] = {
            { MOO_TEXT_SEARCH_BACKWARDS, (char*)"MOO_TEXT_SEARCH_BACKWARDS", (char*)"backwards" },
            { MOO_TEXT_SEARCH_CASE_INSENSITIVE, (char*)"MOO_TEXT_SEARCH_CASE_INSENSITIVE", (char*)"case-insensitive" },
            { MOO_TEXT_SEARCH_REGEX, (char*)"MOO_TEXT_SEARCH_REGEX", (char*)"regex" },
            { 0, NULL, NULL }
        };

        type = g_flags_register_static ("MooTextSearchOptions", values);
    }

    return type;
}

GType
moo_text_replace_response_type_get_type (void)
{
    static GType type = 0;

    if (!type)
    {
        static const GEnumValue values[] = {
            { MOO_TEXT_REPLACE_STOP, (char*)"MOO_TEXT_REPLACE_STOP", (char*)"stop" },
            { MOO_TEXT_REPLACE_AND_STOP, (char*)"MOO_TEXT_REPLACE_AND_STOP", (char*)"replace-and-stop" },
            { MOO_TEXT_REPLACE_SKIP, (char*)"MOO_TEXT_REPLACE_SKIP", (char*)"skip" },
            { MOO_TEXT_REPLACE, (char*)"MOO_TEXT_REPLACE", (char*)"replace" },
            { MOO_TEXT_REPLACE_ALL, (char*)"MOO_TEXT_REPLACE_ALL", (char*)"all" },
            { 0, NULL, NULL }
        };
        type = g_enum_register_static ("MooTextReplaceResponseType", values);
    }

    return type;
}


#define NUM_LINES_FOR_REGEX_SEARCH 1

static EggRegex *last_regex = NULL;
static MooTextSearchOptions last_options = 0;


static gboolean moo_text_search_regex_forward   (const GtkTextIter  *start,
                                                 const GtkTextIter  *end,
                                                 EggRegex           *regex,
                                                 GtkTextIter        *match_start,
                                                 GtkTextIter        *match_end);
/* assumes start < end */
static gboolean moo_text_search_regex_backward  (const GtkTextIter  *start,
                                                 const GtkTextIter  *end,
                                                 EggRegex           *regex,
                                                 GtkTextIter        *match_start,
                                                 GtkTextIter        *match_end);
static gboolean moo_text_match_regex_forward    (const GtkTextIter  *start,
                                                 const GtkTextIter  *end,
                                                 EggRegex           *regex,
                                                 GtkTextIter        *match_start,
                                                 GtkTextIter        *match_end);
/* assumes start < end */
static gboolean moo_text_match_regex_backward   (const GtkTextIter  *start,
                                                 const GtkTextIter  *end,
                                                 EggRegex           *regex,
                                                 GtkTextIter        *match_start,
                                                 GtkTextIter        *match_end);


gboolean    moo_text_search                 (const GtkTextIter  *start,
                                             const GtkTextIter  *limit,
                                             const char         *text,
                                             GtkTextIter        *match_start,
                                             GtkTextIter        *match_end,
                                             MooTextSearchOptions options,
                                             GError            **error)
{
    GtkSourceSearchFlags flags;
    gboolean backwards = MOO_TEXT_SEARCH_BACKWARDS & options;

    g_return_val_if_fail (text != NULL, FALSE);
    g_return_val_if_fail (start != NULL, FALSE);

    if (error) *error = NULL;

    if (MOO_TEXT_SEARCH_REGEX & options)
    {
        EggRegex *regex = last_regex;

        if (!last_regex || last_options != options ||
            strcmp (egg_regex_get_pattern (last_regex), text))
        {
            GError *err = NULL;
            EggRegexCompileFlags flags;

            if (last_regex) egg_regex_unref (last_regex);
            last_regex = NULL;
            last_options = 0;

            flags = EGG_REGEX_MULTILINE;
            if (options & MOO_TEXT_SEARCH_CASE_INSENSITIVE)
                flags |= EGG_REGEX_CASELESS;

            last_regex = egg_regex_new (text, flags, 0, &err);
            if (err)
            {
                if (error) *error = err;
                else g_error_free (err);
                egg_regex_free (last_regex);
                last_regex = NULL;
                return FALSE;
            }

            egg_regex_optimize (last_regex, &err);
            if (err)
            {
                g_warning ("%s: %s", G_STRLOC, err->message);
                g_error_free (err);
            }

            last_options = options;

            regex = last_regex;
        }

        return moo_text_search_regex (start, limit, regex,
                                      match_start, match_end,
                                      options & MOO_TEXT_SEARCH_BACKWARDS);
    }

    flags = GTK_SOURCE_SEARCH_VISIBLE_ONLY | GTK_SOURCE_SEARCH_TEXT_ONLY;
    if (MOO_TEXT_SEARCH_CASE_INSENSITIVE & options)
        flags |= GTK_SOURCE_SEARCH_CASE_INSENSITIVE;

    if (backwards)
        return gtk_source_iter_backward_search (start, text, flags,
                                                match_start, match_end, limit);
    else
        return gtk_source_iter_forward_search (start, text, flags,
                                               match_start, match_end, limit);
}


gboolean    moo_text_search_regex           (const GtkTextIter  *start,
                                             const GtkTextIter  *limit,
                                             EggRegex           *regex,
                                             GtkTextIter        *match_start,
                                             GtkTextIter        *match_end,
                                             gboolean            backwards)
{
    g_return_val_if_fail (start != NULL && limit != NULL &&
                          regex != NULL, FALSE);

    egg_regex_clear (regex);

    if (backwards)
    {
        GtkTextIter real_start = *limit;
        GtkTextIter real_end = *start;
        return moo_text_search_regex_backward (&real_start, &real_end, regex,
                                               match_start, match_end);
    }
    else
    {
        return moo_text_search_regex_forward (start, limit, regex,
                                              match_start, match_end);
    }
}


static gboolean moo_text_search_regex_forward   (const GtkTextIter  *start,
                                                 const GtkTextIter  *end,
                                                 EggRegex           *regex,
                                                 GtkTextIter        *match_start,
                                                 GtkTextIter        *match_end)
{
    GtkTextIter current_start, current_end;
    int start_line, end_line;
    GtkTextBuffer *buffer;
    int i;

    g_return_val_if_fail (gtk_text_iter_compare (start, end) <= 0, FALSE);

    buffer = gtk_text_iter_get_buffer (start);

    start_line = gtk_text_iter_get_line (start);
    end_line = gtk_text_iter_get_line (end);

    if (end_line - start_line < NUM_LINES_FOR_REGEX_SEARCH)
        return moo_text_match_regex_forward (start, end, regex,
                                             match_start, match_end);

    gtk_text_buffer_get_iter_at_line (buffer, &current_end,
                                      start_line + NUM_LINES_FOR_REGEX_SEARCH - 1);
    if (!gtk_text_iter_ends_line (&current_end))
        gtk_text_iter_forward_to_line_end (&current_end);
    if (moo_text_match_regex_forward (start, &current_end, regex,
                                      match_start, match_end))
            return TRUE;

    for (i = start_line + 1; end_line - i > NUM_LINES_FOR_REGEX_SEARCH; ++i)
    {
        egg_regex_clear (regex);
        gtk_text_buffer_get_iter_at_line (buffer, &current_start, i);
        gtk_text_buffer_get_iter_at_line (buffer, &current_end,
                                          i + NUM_LINES_FOR_REGEX_SEARCH - 1);
        if (!gtk_text_iter_ends_line (&current_end))
            gtk_text_iter_forward_to_line_end (&current_end);
        if (moo_text_match_regex_forward (&current_start, &current_end, regex,
                                          match_start, match_end))
                return TRUE;
    }

    egg_regex_clear (regex);
    gtk_text_buffer_get_iter_at_line (buffer, &current_start,
                                      end_line - NUM_LINES_FOR_REGEX_SEARCH + 1);
    return moo_text_match_regex_forward (&current_start, end, regex,
                                         match_start, match_end);
}


static gboolean moo_text_match_regex_forward    (const GtkTextIter  *start,
                                                 const GtkTextIter  *end,
                                                 EggRegex           *regex,
                                                 GtkTextIter        *match_start,
                                                 GtkTextIter        *match_end)
{
    GtkTextBuffer *buffer;
    char *text;
    int result;
    int start_pos, end_pos;
    long start_offset, end_offset;
    EggRegexMatchFlags options = 0;
    gunichar c;

    buffer = gtk_text_iter_get_buffer (start);
    text = gtk_text_buffer_get_slice (buffer, start, end, TRUE);

    if (!gtk_text_iter_ends_line (end))
    {
        options = EGG_REGEX_MATCH_NOTEOL;
    }
    if (!gtk_text_iter_starts_line (start))
    {
        options |= EGG_REGEX_MATCH_NOTBOL;
    }

    result = egg_regex_match (regex, text, -1, options);

    if (result <= 0)
    {
        g_free (text);
        return FALSE;
    }

    egg_regex_fetch_pos (regex, text, 0, &start_pos, &end_pos);

    c = g_utf8_get_char_validated (text + start_pos, -1);
    if (c == ((gunichar)-2) || c == ((gunichar)-1))
    {
        g_critical ("%s: egg_regex_fetch_pos() returned invalid "
                    "match start position", G_STRLOC);
        g_free (text);
        return FALSE;
    }
    c = g_utf8_get_char_validated (text + end_pos, -1);
    if (c == ((gunichar)-2) || c == ((gunichar)-1))
    {
        g_critical ("%s: egg_regex_fetch_pos() returned invalid "
                    "match end position", G_STRLOC);
        g_free (text);
        return FALSE;
    }

    start_offset = g_utf8_pointer_to_offset (text, text + start_pos);
    end_offset = g_utf8_pointer_to_offset (text + start_pos, text + end_pos);
    end_offset += start_offset;

    /* TODO: why end and start are swapped? */

    if (match_start)
    {
        *match_start = *start;
        gtk_text_iter_forward_chars (match_start, start_offset);
        if (match_end)
        {
            *match_end = *match_start;
            gtk_text_iter_forward_chars (match_start, end_offset - start_offset);
        }

        gtk_text_iter_order (match_start, match_end);
    }
    else
    {
        *match_end = *start;
        gtk_text_iter_forward_chars (match_end, start_offset); /* TODO: <- swapping */
    }

    g_free (text);
    g_assert (gtk_text_iter_compare (match_start, match_end) <= 0);
    return TRUE;
}


static gboolean moo_text_search_regex_backward  (const GtkTextIter  *start,
                                                 const GtkTextIter  *end,
                                                 EggRegex           *regex,
                                                 GtkTextIter        *match_start,
                                                 GtkTextIter        *match_end)
{
    GtkTextIter current_start, current_end;
    int start_line, end_line;
    GtkTextBuffer *buffer;
    int i;

    g_return_val_if_fail (gtk_text_iter_compare (start, end) <= 0, FALSE);

    buffer = gtk_text_iter_get_buffer (start);

    start_line = gtk_text_iter_get_line (start);
    end_line = gtk_text_iter_get_line (end);

    if (end_line - start_line < NUM_LINES_FOR_REGEX_SEARCH)
        return moo_text_match_regex_backward (start, end, regex,
                                              match_start, match_end);

    gtk_text_buffer_get_iter_at_line (buffer, &current_start,
                                      end_line - NUM_LINES_FOR_REGEX_SEARCH + 1);
    if (moo_text_match_regex_backward (&current_start, end, regex,
                                       match_start, match_end))
            return TRUE;

    for (i = end_line - 1; i > start_line + NUM_LINES_FOR_REGEX_SEARCH - 1; --i)
    {
        egg_regex_clear (regex);
        gtk_text_buffer_get_iter_at_line (buffer, &current_start,
                                          i - NUM_LINES_FOR_REGEX_SEARCH + 1);
        gtk_text_buffer_get_iter_at_line (buffer, &current_end, i);
        if (!gtk_text_iter_ends_line (&current_end))
            gtk_text_iter_forward_to_line_end (&current_end);
        if (moo_text_match_regex_forward (&current_start, &current_end, regex,
                                          match_start, match_end))
                return TRUE;
    }

    egg_regex_clear (regex);
    gtk_text_buffer_get_iter_at_line (buffer, &current_end,
                                      start_line + NUM_LINES_FOR_REGEX_SEARCH - 1);
    if (!gtk_text_iter_ends_line (&current_end))
        gtk_text_iter_forward_to_line_end (&current_end);
    return moo_text_match_regex_forward (start, &current_end, regex,
                                         match_start, match_end);
}


/* assumes start < end */
static gboolean moo_text_match_regex_backward   (const GtkTextIter  *start,
                                                 const GtkTextIter  *end,
                                                 EggRegex           *regex,
                                                 GtkTextIter        *match_start,
                                                 GtkTextIter        *match_end)
{
    GtkTextBuffer *buffer;
    char *text;
    int start_pos = -1, end_pos = -1;
    long start_offset, end_offset;
    EggRegexMatchFlags options = 0;

    buffer = gtk_text_iter_get_buffer (start);
    text = gtk_text_buffer_get_slice (buffer, start, end, TRUE);

    if (!gtk_text_iter_ends_line (end))
        options = EGG_REGEX_MATCH_NOTEOL;
    if (!gtk_text_iter_starts_line (start))
        options |= EGG_REGEX_MATCH_NOTBOL;

    while (TRUE)
    {
        if (egg_regex_match_next (regex, text, -1, 0) <= 0)
        {
            if (start_pos >= 0)
            {
                break;
            }
            else
            {
                g_free (text);
                return FALSE;
            }
        }
        else
            egg_regex_fetch_pos (regex, text, 0, &start_pos, &end_pos);
    }

    start_offset = g_utf8_pointer_to_offset (text, text + start_pos);
    end_offset = g_utf8_pointer_to_offset (text + start_pos, text + end_pos);
    end_offset += start_offset;

    /* TODO: why end and start are swapped? */

    if (match_start)
    {
        *match_start = *start;
        gtk_text_iter_forward_chars (match_start, start_offset);
        if (match_end)
        {
            *match_end = *match_start;
            gtk_text_iter_forward_chars (match_start, end_offset - start_offset);
        }
    }
    else
    {
        *match_end = *start;
        gtk_text_iter_forward_chars (match_end, end_offset); /* TODO: <- swapping */
    }

    g_free (text);
    g_assert (gtk_text_iter_compare (match_start, match_end) >= 0);
    return TRUE;
}


/****************************************************************************/
/* Replace
 */

MooTextReplaceResponseType moo_text_replace_func_replace_all
                                            (G_GNUC_UNUSED const char         *text,
                                             G_GNUC_UNUSED EggRegex           *regex,
                                             G_GNUC_UNUSED const char         *replacement,
                                             G_GNUC_UNUSED GtkTextIter        *to_replace_start,
                                             G_GNUC_UNUSED GtkTextIter        *to_replace_end,
                                             G_GNUC_UNUSED gpointer            data)
{
    return MOO_TEXT_REPLACE_ALL;
}

static int  moo_text_replace_all_interactive_forward
                                            (GtkTextIter        *start,
                                             GtkTextIter        *end,
                                             const char         *text,
                                             const char         *replacement,
                                             MooTextSearchOptions options,
                                             MooTextReplaceFunc  func,
                                             gpointer            data);
static int  moo_text_replace_all_interactive_backward
                                            (GtkTextIter        *start,
                                             GtkTextIter        *end,
                                             const char         *text,
                                             const char         *replacement,
                                             MooTextSearchOptions options,
                                             MooTextReplaceFunc  func,
                                             gpointer            data);
static int  moo_text_replace_regex_all_interactive_forward
                                            (GtkTextIter        *start,
                                             GtkTextIter        *end,
                                             EggRegex           *regex,
                                             const char         *replacement,
                                             GError            **error,
                                             MooTextReplaceFunc  func,
                                             gpointer            data);
static int  moo_text_replace_regex_all_interactive_backward
                                            (GtkTextIter        *start,
                                             GtkTextIter        *end,
                                             EggRegex           *regex,
                                             const char         *replacement,
                                             GError            **error,
                                             MooTextReplaceFunc  func,
                                             gpointer            data);

static gboolean moo_text_search_regex_replace_forward
                                                (const GtkTextIter  *start,
                                                 const GtkTextIter  *end,
                                                 EggRegex           *regex,
                                                 const char         *replacement,
                                                 char              **replacement_text,
                                                 GtkTextIter        *match_start,
                                                 GtkTextIter        *match_end,
                                                 GError            **error);
static gboolean moo_text_search_regex_replace_backward
                                                (const GtkTextIter  *start,
                                                 const GtkTextIter  *end,
                                                 EggRegex           *regex,
                                                 const char         *replacement,
                                                 char              **replacement_text,
                                                 GtkTextIter        *match_start,
                                                 GtkTextIter        *match_end,
                                                 GError            **error);
static gboolean moo_text_match_regex_replace_forward
                                                (const GtkTextIter  *start,
                                                 const GtkTextIter  *end,
                                                 EggRegex           *regex,
                                                 const char         *replacement,
                                                 char              **replacement_text,
                                                 GtkTextIter        *match_start,
                                                 GtkTextIter        *match_end,
                                                 GError            **error);
#if 0
static gboolean moo_text_match_regex_replace_backward
                                                (const GtkTextIter  *start,
                                                 const GtkTextIter  *end,
                                                 EggRegex           *regex,
                                                 const char         *replacement,
                                                 char              **replacement_text,
                                                 GtkTextIter        *match_start,
                                                 GtkTextIter        *match_end,
                                                 GError            **error);
#endif


int         moo_text_replace_all_interactive(GtkTextIter        *start,
                                             GtkTextIter        *end,
                                             const char         *text,
                                             const char         *replacement,
                                             MooTextSearchOptions options,
                                             GError            **error,
                                             MooTextReplaceFunc  func,
                                             gpointer            data)
{
    gboolean backwards = MOO_TEXT_SEARCH_BACKWARDS & options;

    g_return_val_if_fail (start != NULL && end != NULL, MOO_TEXT_REPLACE_INVALID_ARGS);
    g_return_val_if_fail (text != NULL && replacement != NULL, MOO_TEXT_REPLACE_INVALID_ARGS);
    g_return_val_if_fail (func != NULL, MOO_TEXT_REPLACE_INVALID_ARGS);

    if (backwards)
        g_return_val_if_fail (gtk_text_iter_compare (start, end) == 1,
                              MOO_TEXT_REPLACE_INVALID_ARGS);
    else
        g_return_val_if_fail (gtk_text_iter_compare (start, end) == -1,
                              MOO_TEXT_REPLACE_INVALID_ARGS);

    if (error) *error = NULL;

    if (MOO_TEXT_SEARCH_REGEX & options)
    {
        EggRegex *regex = last_regex;

        if (!last_regex || last_options != options ||
            strcmp (egg_regex_get_pattern (last_regex), text))
        {
            GError *err = NULL;
            EggRegexCompileFlags flags;

            if (last_regex) egg_regex_unref (last_regex);
            last_regex = NULL;
            last_options = 0;

            flags = EGG_REGEX_MULTILINE;
            if (options & MOO_TEXT_SEARCH_CASE_INSENSITIVE)
                flags |= EGG_REGEX_CASELESS;

            last_regex = egg_regex_new (text, flags, 0, &err);
            if (err)
            {
                if (error) *error = err;
                else g_error_free (err);
                egg_regex_free (last_regex);
                last_regex = NULL;
                return MOO_TEXT_REPLACE_REGEX_ERROR;
            }

            egg_regex_optimize (last_regex, &err);
            if (err)
            {
                g_warning ("%s: %s", G_STRLOC, err->message);
                g_error_free (err);
            }

            last_options = options;

            regex = last_regex;
        }

        if (backwards)
            return moo_text_replace_regex_all_interactive_backward (
                                start, end, regex, replacement,
                                error, func, data);
        else
            return moo_text_replace_regex_all_interactive_forward (
                                start, end, regex, replacement,
                                error, func, data);
    }

    if (backwards)
        return moo_text_replace_all_interactive_backward (start, end,
                                                          text, replacement,
                                                          options, func, data);
    else
        return moo_text_replace_all_interactive_forward (start, end,
                                                         text, replacement,
                                                         options, func, data);
}


int         moo_text_replace_regex_all_interactive
                                            (GtkTextIter        *start,
                                             GtkTextIter        *end,
                                             EggRegex           *regex,
                                             const char         *replacement,
                                             gboolean            backwards,
                                             GError            **error,
                                             MooTextReplaceFunc  func,
                                             gpointer            data)
{
    g_return_val_if_fail (start != NULL && end != NULL, MOO_TEXT_REPLACE_INVALID_ARGS);
    g_return_val_if_fail (regex != NULL && replacement != NULL, MOO_TEXT_REPLACE_INVALID_ARGS);
    g_return_val_if_fail (func != NULL, MOO_TEXT_REPLACE_INVALID_ARGS);

    if (backwards)
        g_return_val_if_fail (gtk_text_iter_compare (start, end) == 1,
                              MOO_TEXT_REPLACE_INVALID_ARGS);
    else
        g_return_val_if_fail (gtk_text_iter_compare (start, end) == -1,
                              MOO_TEXT_REPLACE_INVALID_ARGS);

    if (error) *error = NULL;

    if (backwards)
        return moo_text_replace_regex_all_interactive_backward (
                            start, end, regex, replacement,
                            error, func, data);
    else
        return moo_text_replace_regex_all_interactive_forward (
                            start, end, regex, replacement,
                            error, func, data);
}


static int  moo_text_replace_all_interactive_forward
                                            (GtkTextIter        *start,
                                             GtkTextIter        *end,
                                             const char         *text,
                                             const char         *replacement,
                                             MooTextSearchOptions options,
                                             MooTextReplaceFunc  func,
                                             gpointer            data)
{
    GtkTextMark *start_mark, *end_mark;
    GtkTextBuffer *buffer;
    int count = 0;
    GtkSourceSearchFlags flags;
    gsize replacement_len = strlen (replacement);
    MooTextReplaceResponseType response = MOO_TEXT_REPLACE;

    buffer = gtk_text_iter_get_buffer (start);
    start_mark = gtk_text_buffer_create_mark (buffer, NULL, start, FALSE);
    end_mark = gtk_text_buffer_create_mark (buffer, NULL, end, FALSE);

    flags = GTK_SOURCE_SEARCH_TEXT_ONLY;
    if (options & MOO_TEXT_SEARCH_CASE_INSENSITIVE)
        flags |= GTK_SOURCE_SEARCH_CASE_INSENSITIVE;

    gtk_text_buffer_begin_user_action (buffer);

    while (TRUE)
    {
        GtkTextIter match_start, match_end;

        if (!gtk_source_iter_forward_search (start, text, flags,
                                             &match_start, &match_end,
                                             end))
            break;

        if (response != MOO_TEXT_REPLACE_ALL)
        {
            response = func (text, NULL, replacement, &match_start, &match_end, data);

            if (response == MOO_TEXT_REPLACE_STOP)
                break;
        }

        if (response != MOO_TEXT_REPLACE_SKIP)
        {
            gtk_text_buffer_delete (buffer, &match_start, &match_end);
            if (replacement_len)
                gtk_text_buffer_insert (buffer, &match_end,
                                        replacement, replacement_len);
            ++count;
        }

        *start = match_end;
        gtk_text_buffer_get_iter_at_mark (buffer, end, end_mark);

        if (response == MOO_TEXT_REPLACE_AND_STOP)
            break;

        if (gtk_text_iter_compare (start, end) >= 0)
            break;
    }

    gtk_text_buffer_end_user_action (buffer);

    gtk_text_buffer_get_iter_at_mark (buffer, start, start_mark);
    gtk_text_buffer_delete_mark (buffer, start_mark);
    gtk_text_buffer_delete_mark (buffer, end_mark);

    return count;
}


static int  moo_text_replace_all_interactive_backward
                                            (GtkTextIter        *start,
                                             GtkTextIter        *end,
                                             const char         *text,
                                             const char         *replacement,
                                             MooTextSearchOptions options,
                                             MooTextReplaceFunc  func,
                                             gpointer            data)
{
    GtkTextMark *start_mark, *end_mark, *insert;
    GtkTextBuffer *buffer;
    int count = 0;
    GtkSourceSearchFlags flags;
    gsize replacement_len;
    MooTextReplaceResponseType response = MOO_TEXT_REPLACE;

    buffer = gtk_text_iter_get_buffer (start);
    start_mark = gtk_text_buffer_create_mark (buffer, NULL, start, FALSE);
    end_mark = gtk_text_buffer_create_mark (buffer, NULL, end, TRUE);
    insert = gtk_text_buffer_create_mark (buffer, NULL, start, TRUE);

    flags = GTK_SOURCE_SEARCH_TEXT_ONLY;
    if (options & MOO_TEXT_SEARCH_CASE_INSENSITIVE)
        flags |= GTK_SOURCE_SEARCH_CASE_INSENSITIVE;

    gtk_text_buffer_begin_user_action (buffer);

    replacement_len = strlen (replacement);

    while (TRUE)
    {
        GtkTextIter match_start, match_end;

        if (!gtk_source_iter_backward_search (start, text, flags,
                                              &match_start, &match_end,
                                              end))
            break;

        if (response != MOO_TEXT_REPLACE_ALL)
        {
            response = func (text, NULL, replacement, &match_start, &match_end, data);

            if (response == MOO_TEXT_REPLACE_STOP)
                break;
        }

        gtk_text_buffer_move_mark (buffer, insert, &match_start);
        if (response != MOO_TEXT_REPLACE_SKIP)
        {
            gtk_text_buffer_delete (buffer, &match_start, &match_end);
            if (replacement_len)
                gtk_text_buffer_insert (buffer, &match_start,
                                        replacement, replacement_len);
            ++count;
        }

        gtk_text_buffer_get_iter_at_mark (buffer, end, end_mark);
        gtk_text_buffer_get_iter_at_mark (buffer, start, insert);

        if (response == MOO_TEXT_REPLACE_AND_STOP)
            break;

        if (gtk_text_iter_compare (start, end) <= 0)
            break;
    }

    gtk_text_buffer_end_user_action (buffer);

    gtk_text_buffer_get_iter_at_mark (buffer, start, start_mark);
    gtk_text_buffer_delete_mark (buffer, start_mark);
    gtk_text_buffer_delete_mark (buffer, end_mark);

    return count;
}


static int  moo_text_replace_regex_all_interactive_forward
                                            (GtkTextIter        *start,
                                             GtkTextIter        *end,
                                             EggRegex           *regex,
                                             const char         *replacement,
                                             GError            **error,
                                             MooTextReplaceFunc  func,
                                             gpointer            data)
{
    GtkTextMark *start_mark, *end_mark;
    GtkTextBuffer *buffer;
    int count = 0;
    MooTextReplaceResponseType response = MOO_TEXT_REPLACE;

    buffer = gtk_text_iter_get_buffer (start);
    start_mark = gtk_text_buffer_create_mark (buffer, NULL, start, FALSE);
    end_mark = gtk_text_buffer_create_mark (buffer, NULL, end, FALSE);

    gtk_text_buffer_begin_user_action (buffer);

    while (TRUE)
    {
        GtkTextIter match_start, match_end;
        char *replacement_text = NULL;

        if (!moo_text_search_regex_replace_forward (start, end,
                                                    regex, replacement,
                                                    &replacement_text,
                                                    &match_start, &match_end,
                                                    error))
            break;

        if (response != MOO_TEXT_REPLACE_ALL)
        {
            response = func (egg_regex_get_pattern (regex), regex,
                             replacement_text, &match_start, &match_end, data);

            if (response == MOO_TEXT_REPLACE_STOP)
            {
                g_free (replacement_text);
                break;
            }
        }

        if (response != MOO_TEXT_REPLACE_SKIP)
        {
            gtk_text_buffer_delete (buffer, &match_start, &match_end);
            gtk_text_buffer_insert (buffer, &match_end, replacement_text, -1);
            g_free (replacement_text);
            ++count;
        }

        *start = match_end;
        gtk_text_buffer_get_iter_at_mark (buffer, end, end_mark);

        if (response == MOO_TEXT_REPLACE_AND_STOP)
            break;

        if (gtk_text_iter_compare (start, end) >= 0)
            break;
    }

    gtk_text_buffer_end_user_action (buffer);

    gtk_text_buffer_get_iter_at_mark (buffer, start, start_mark);
    gtk_text_buffer_delete_mark (buffer, start_mark);
    gtk_text_buffer_delete_mark (buffer, end_mark);

    return count;
}


static int  moo_text_replace_regex_all_interactive_backward
                                            (GtkTextIter        *start,
                                             GtkTextIter        *end,
                                             EggRegex           *regex,
                                             const char         *replacement,
                                             GError            **error,
                                             MooTextReplaceFunc  func,
                                             gpointer            data)
{
    GtkTextMark *start_mark, *end_mark, *insert;
    GtkTextBuffer *buffer;
    int count = 0;
    MooTextReplaceResponseType response = MOO_TEXT_REPLACE;

    buffer = gtk_text_iter_get_buffer (start);
    start_mark = gtk_text_buffer_create_mark (buffer, NULL, start, FALSE);
    end_mark = gtk_text_buffer_create_mark (buffer, NULL, end, TRUE);
    insert = gtk_text_buffer_create_mark (buffer, NULL, start, TRUE);

    gtk_text_buffer_begin_user_action (buffer);

    while (TRUE)
    {
        GtkTextIter match_start, match_end;
        char *replacement_text = NULL;

        if (!moo_text_search_regex_replace_backward (start, end, regex,
                                                     replacement, &replacement_text,
                                                     &match_start, &match_end,
                                                     error))
            break;

        if (response != MOO_TEXT_REPLACE_ALL)
        {
            response = func (egg_regex_get_pattern (regex),
                             regex, replacement_text, &match_start, &match_end,
                             data);

            if (response == MOO_TEXT_REPLACE_STOP)
            {
                g_free (replacement_text);
                break;
            }
        }

        gtk_text_buffer_move_mark (buffer, insert, &match_start);
        if (response != MOO_TEXT_REPLACE_SKIP)
        {
            gtk_text_buffer_delete (buffer, &match_start, &match_end);
            gtk_text_buffer_insert (buffer, &match_start, replacement_text, -1);
            g_free (replacement_text);
            ++count;
        }

        gtk_text_buffer_get_iter_at_mark (buffer, end, end_mark);
        gtk_text_buffer_get_iter_at_mark (buffer, start, insert);

        if (response == MOO_TEXT_REPLACE_AND_STOP)
            break;

        if (gtk_text_iter_compare (start, end) <= 0)
            break;
    }

    gtk_text_buffer_end_user_action (buffer);

    gtk_text_buffer_get_iter_at_mark (buffer, start, start_mark);
    gtk_text_buffer_delete_mark (buffer, start_mark);
    gtk_text_buffer_delete_mark (buffer, end_mark);

    return count;
}


static gboolean moo_text_search_regex_replace_forward
                                                (const GtkTextIter  *start,
                                                 const GtkTextIter  *end,
                                                 EggRegex           *regex,
                                                 const char         *replacement,
                                                 char              **replacement_text,
                                                 GtkTextIter        *match_start,
                                                 GtkTextIter        *match_end,
                                                 GError            **error)
{
    GtkTextIter current_start, current_end;
    int start_line, end_line;
    GtkTextBuffer *buffer;
    int i;

    g_return_val_if_fail (gtk_text_iter_compare (start, end) <= 0, FALSE);

    start_line = gtk_text_iter_get_line (start);
    end_line = gtk_text_iter_get_line (end);

    if (end_line - start_line < NUM_LINES_FOR_REGEX_SEARCH)
        return moo_text_match_regex_replace_forward (start, end, regex,
                                                     replacement, replacement_text,
                                                     match_start, match_end,
                                                     error);

    buffer = gtk_text_iter_get_buffer (start);
    gtk_text_buffer_get_iter_at_line (buffer, &current_end,
                                      start_line + NUM_LINES_FOR_REGEX_SEARCH - 1);
    if (!gtk_text_iter_ends_line (&current_end))
        gtk_text_iter_forward_to_line_end (&current_end);
    if (moo_text_match_regex_replace_forward (start, &current_end, regex,
                                              replacement, replacement_text,
                                              match_start, match_end,
                                              error))
            return TRUE;

    for (i = start_line + 1; end_line - i > NUM_LINES_FOR_REGEX_SEARCH; ++i)
    {
        egg_regex_clear (regex);
        gtk_text_buffer_get_iter_at_line (buffer, &current_start, i);
        gtk_text_buffer_get_iter_at_line (buffer, &current_end,
                                          i + NUM_LINES_FOR_REGEX_SEARCH - 1);
        if (!gtk_text_iter_ends_line (&current_end))
            gtk_text_iter_forward_to_line_end (&current_end);
        if (moo_text_match_regex_replace_forward (&current_start, &current_end, regex,
                                                  replacement, replacement_text,
                                                  match_start, match_end,
                                                  error))
                return TRUE;
    }

    egg_regex_clear (regex);
    gtk_text_buffer_get_iter_at_line (buffer, &current_start,
                                      end_line - NUM_LINES_FOR_REGEX_SEARCH + 1);
    return moo_text_match_regex_replace_forward (&current_start, end, regex,
                                                 replacement, replacement_text,
                                                 match_start, match_end,
                                                 error);
}


static gboolean moo_text_match_regex_replace_forward
                                                (const GtkTextIter  *start,
                                                 const GtkTextIter  *end,
                                                 EggRegex           *regex,
                                                 const char         *replacement,
                                                 char              **replacement_text,
                                                 GtkTextIter        *match_start,
                                                 GtkTextIter        *match_end,
                                                 GError            **error)
{
    GtkTextBuffer *buffer;
    char *text;
    int result;
    int start_pos, end_pos;
    long start_offset, end_offset;
    gunichar c;

    buffer = gtk_text_iter_get_buffer (start);
    text = gtk_text_buffer_get_slice (buffer, start, end, TRUE);
    result = egg_regex_match (regex, text, -1, 0);

    if (result <= 0) {
        g_free (text);
        return FALSE;
    }

    egg_regex_fetch_pos (regex, text, 0, &start_pos, &end_pos);

    c = g_utf8_get_char_validated (text + start_pos, -1);
    if (c == ((gunichar)-2) || c == ((gunichar)-1)) {
        g_critical ("%s: egg_regex_fetch_pos() returned invalid "
                    "match start position", G_STRLOC);
        g_free (text);
        return FALSE;
    }
    c = g_utf8_get_char_validated (text + end_pos, -1);
    if (c == ((gunichar)-2) || c == ((gunichar)-1)) {
        g_critical ("%s: egg_regex_fetch_pos() returned invalid "
                    "match end position", G_STRLOC);
        g_free (text);
        return FALSE;
    }

    start_offset = g_utf8_pointer_to_offset (text, text + start_pos);
    end_offset = g_utf8_pointer_to_offset (text + start_pos, text + end_pos);
    end_offset += start_offset;

    /* TODO: why end and start are swapped? */

    if (match_start)
    {
        *match_start = *start;
        gtk_text_iter_forward_chars (match_start, start_offset);
        if (match_end) {
            *match_end = *match_start;
            gtk_text_iter_forward_chars (match_start, end_offset - start_offset);
        }

        gtk_text_iter_order (match_start, match_end);
    }
    else
    {
        *match_end = *start;
        gtk_text_iter_forward_chars (match_end, start_offset); /* TODO: <- swapping */
    }

    {
        gchar *rtext = egg_regex_eval_replacement (regex, text, replacement, error);
        if (rtext)
        {
            if (replacement_text) *replacement_text = rtext;
            else g_free (rtext);
            g_free (text);
            return TRUE;
        }
        else
        {
            if (replacement_text) *replacement_text = NULL;
            g_free (text);
            return FALSE;
        }
    }
}


static gboolean moo_text_search_regex_replace_backward
                                                (G_GNUC_UNUSED const GtkTextIter  *start,
                                                 G_GNUC_UNUSED const GtkTextIter  *end,
                                                 G_GNUC_UNUSED EggRegex           *regex,
                                                 G_GNUC_UNUSED const char         *replacement,
                                                 G_GNUC_UNUSED char              **replacement_text,
                                                 G_GNUC_UNUSED GtkTextIter        *match_start,
                                                 G_GNUC_UNUSED GtkTextIter        *match_end,
                                                 G_GNUC_UNUSED GError            **error)
{
    g_warning ("%s: implement me", G_STRLOC);
    return FALSE;
}


#if 0
static gboolean moo_text_match_regex_replace_backward
                                                (G_GNUC_UNUSED const GtkTextIter  *start,
                                                 G_GNUC_UNUSED const GtkTextIter  *end,
                                                 G_GNUC_UNUSED EggRegex           *regex,
                                                 G_GNUC_UNUSED const char         *replacement,
                                                 G_GNUC_UNUSED char              **replacement_text,
                                                 G_GNUC_UNUSED GtkTextIter        *match_start,
                                                 G_GNUC_UNUSED GtkTextIter        *match_end,
                                                 G_GNUC_UNUSED GError            **error)
{
    g_warning ("%s: implement me", G_STRLOC);
    return FALSE;
}
#endif
