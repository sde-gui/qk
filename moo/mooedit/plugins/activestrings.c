/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   activestrings.c
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifndef MOO_VERSION
#define MOO_VERSION NULL
#endif

#include "mooedit/mooplugin-macro.h"
#include "mooedit/mootextview.h"
#include "mooedit/plugins/mooeditplugins.h"
#include "mooutils/eggregex.h"
#include <string.h>

#define AS_PLUGIN_ID "ActiveStrings"
#define AS_DATA "moo-active-strings"
#define AS_WILDCARD '?'


typedef struct _ASPlugin ASPlugin;
typedef struct _ASSet ASSet;
typedef struct _ASString ASString;
typedef struct _ASStringInfo ASStringInfo;
typedef struct _ASMatch ASMatch;


struct _ASPlugin {
    MooPlugin parent;
    ASSet *set;
};

struct _ASStringInfo {
    guint n_wildcards;
    char *pattern;
    guint pattern_len;
    gunichar last_char;
};

struct _ASString {
    guint whole;
    guint *parens;
    guint n_parens;
};

struct _ASSet {
    ASString **strings;
    guint n_strings;
    EggRegex *regex;
    guint ref_count;
    gunichar min_last;
    gunichar max_last;
    gunichar *last_chars;
    guint n_last_chars;
};

struct _ASMatch {
    guint string_no;
    /* all offsets are in chars here */
    guint start_offset;
    guint end_offset;
    guint *parens;
    guint n_parens;
};

typedef enum {
    AS_WORD_BOUNDARY        = 1 << 0,
    AS_NOT_WORD_BOUNDARY    = 1 << 1
} ASOptions;


static gboolean as_plugin_init      (ASPlugin       *plugin);
static void     as_plugin_deinit    (ASPlugin       *plugin);
static void     as_plugin_attach    (ASPlugin       *plugin,
                                     MooEdit        *doc);
static void     as_plugin_detach    (ASPlugin       *plugin,
                                     MooEdit        *doc);

static void     as_plugin_do_action (ASPlugin       *plugin,
                                     MooEdit        *doc,
                                     GtkTextIter    *insert,
                                     ASSet          *set,
                                     ASMatch        *match,
                                     char           *full_text,
                                     char          **parens_text);

static gboolean char_inserted_cb    (MooEdit        *doc,
                                     GtkTextIter    *where,
                                     guint           character,
                                     ASSet          *set);
static void     process_match       (MooEdit        *doc,
                                     GtkTextIter    *end,
                                     ASSet          *set,
                                     ASMatch        *match);

static ASSet   *as_set_new          (ASStringInfo  **strings,
                                     guint           n_strings);
static void     as_set_unref        (ASSet          *set);
static ASSet   *as_set_ref          (ASSet          *set);
static gboolean as_set_check_last   (ASSet          *set,
                                     gunichar        c);
static gboolean as_set_match        (ASSet          *set,
                                     const char     *text,
                                     ASMatch        *match);

static void     as_match_destroy    (ASMatch        *match);
static void     as_string_info_free (ASStringInfo   *info);
static ASString *as_string_new      (guint           n_wildcards);
static void     as_string_free      (ASString       *s);


static int
cmp_ints (gunichar *c1, gunichar *c2)
{
    return *c1 < *c2 ? -1 : (*c1 > *c2 ? 1 : 0);
}


static void
append_options (GString  *pattern,
                ASOptions opts)
{
    if (opts & AS_WORD_BOUNDARY)
        g_string_append (pattern, "\\b");
    else if (opts & AS_NOT_WORD_BOUNDARY)
        g_string_append (pattern, "\\B");
}

static void
append_escaped (GString    *pattern,
                const char *string,
                int         len)
{
    if (len < 0)
        len = strlen (string);

    if (!egg_regex_escape (string, len, pattern))
        g_string_append_len (pattern, string, len);
}


static ASStringInfo *
as_string_get_info (const char *string,
                    ASOptions   start_opts,
                    ASOptions   end_opts,
                    guint       string_no)
{
    char *wc, *p;
    guint rev_len;
    ASStringInfo *info = NULL;
    char *rev = NULL;
    GString *pattern = NULL;

    if (!string || !string[0])
    {
        g_critical ("%s: empty string", G_STRLOC);
        goto error;
    }

    rev = g_utf8_strreverse (string, -1);
    rev_len = strlen (rev);
    info = g_new0 (ASStringInfo, 1);

    if (string[0] == AS_WILDCARD && string[1] != AS_WILDCARD)
    {
        g_critical ("%s: leading '%c' symbol", G_STRLOC, AS_WILDCARD);
        goto error;
    }

    if (rev[0] == AS_WILDCARD && rev[1] != AS_WILDCARD)
    {
        g_critical ("%s: trailing '%c' symbol", G_STRLOC, AS_WILDCARD);
        goto error;
    }

    info->last_char = g_utf8_get_char (rev);
    pattern = g_string_sized_new (rev_len + 16);

    if (end_opts)
        append_options (pattern, end_opts);

    g_string_append_printf (pattern, "(?P<%d>", string_no);

    for (p = rev; *p; )
    {
        wc = strchr (p, AS_WILDCARD);

        if (!wc)
        {
            append_escaped (pattern, p, -1);
            break;
        }

        if (wc[1] == AS_WILDCARD)
        {
            append_escaped (pattern, p, wc - p + 1);
            p = wc + 2;
        }
        else
        {
            append_escaped (pattern, p, wc - p);
            g_string_append_printf (pattern, "(?P<%d_%d>.*)",
                                    string_no, info->n_wildcards);
            info->n_wildcards++;
            p = wc + 1;
        }
    }

    g_string_append (pattern, ")");

    if (start_opts)
        append_options (pattern, start_opts);

    info->pattern_len = pattern->len;
    info->pattern = g_string_free (pattern, FALSE);
    g_free (rev);
    return info;

error:
    as_string_info_free (info);
    g_free (rev);
    if (pattern)
        g_string_free (pattern, TRUE);
    return NULL;
}


static void
as_string_info_free (ASStringInfo *info)
{
    if (info)
    {
        g_free (info->pattern);
        g_free (info);
    }
}


static ASString *
as_string_new (guint n_wildcards)
{
    ASString *s = g_new0 (ASString, 1);

    if (n_wildcards)
    {
        s->n_parens = n_wildcards;
        s->parens = g_new0 (guint, s->n_parens);
    }

    return s;
}


static void
as_string_free (ASString *s)
{
    if (s)
    {
        g_free (s->parens);
        g_free (s);
    }
}


static ASSet*
as_set_new (ASStringInfo **strings,
            guint          n_strings)
{
    ASSet *set = NULL;
    GString *pattern;
    guint i, len;
    GError *error = NULL;
    gunichar *last_chars;
    gunichar min_last = 0, max_last = 0;
    gboolean has_wildcard = FALSE;

    g_return_val_if_fail (strings != NULL, NULL);
    g_return_val_if_fail (n_strings != 0, NULL);

    set = g_new0 (ASSet, 1);
    set->ref_count = 1;
    set->n_strings = n_strings;
    set->strings = g_new (ASString*, n_strings);

    len = n_strings + 2; /* (||||) */
    last_chars = g_new (gunichar, n_strings);

    for (i = 0; i < n_strings; ++i)
    {
        last_chars[i] = strings[i]->last_char;
        min_last = MIN (min_last, last_chars[i]);
        max_last = MAX (max_last, last_chars[i]);

        len += strings[i]->pattern_len;

        set->strings[i] = as_string_new (strings[i]->n_wildcards);

        if (strings[i]->n_wildcards)
            has_wildcard = TRUE;
    }

    set->min_last = min_last;
    set->max_last = max_last;

    pattern = g_string_sized_new (len);

    for (i = 0; i < n_strings; ++i)
    {
        if (i)
            g_string_append_c (pattern, '|');

        g_string_append_len (pattern, strings[i]->pattern, strings[i]->pattern_len);
    }

    set->regex = egg_regex_new (pattern->str, EGG_REGEX_ANCHORED, 0, &error);
    g_print ("pattern: %s\n", pattern->str);

    if (error)
    {
        g_critical ("%s: %s", G_STRLOC, error->message);
        g_error_free (error);
        goto error;
    }

    egg_regex_optimize (set->regex, &error);

    if (error)
    {
        g_critical ("%s: %s", G_STRLOC, error->message);
        g_error_free (error);
    }

    g_qsort_with_data (last_chars, n_strings, sizeof (gunichar),
                       (GCompareDataFunc) cmp_ints, NULL);

    for (i = 1, len = 1; i < n_strings; ++i)
        if (last_chars[i] != last_chars[len-1])
            if (len++ != i)
                last_chars[len-1] = last_chars[i];

    set->last_chars = g_memdup (last_chars, len * sizeof (gunichar));
    set->n_last_chars = len;

    for (i = 0; i < n_strings; ++i)
    {
        int no;
        guint j, n_parens;
        char name[32];

        g_snprintf (name, 32, "%d", i);
        no = egg_regex_get_string_number (set->regex, name);
        g_assert (no > 0);

        set->strings[i]->whole = no;

        n_parens = set->strings[i]->n_parens;
        /* they are inverted */
        for (j = 0; j < n_parens; ++j)
        {
            g_snprintf (name, 32, "%d_%d", i, j);
            no = egg_regex_get_string_number (set->regex, name);
            g_assert (no > 0);
            set->strings[i]->parens[n_parens - j - 1] = no;
        }
    }

    g_free (last_chars);
    g_string_free (pattern, TRUE);

    return set;

error:
    g_free (last_chars);
    g_string_free (pattern, TRUE);
    as_set_unref (set);
    return NULL;
}


static void
as_match_destroy (ASMatch *match)
{
    if (match->n_parens)
        g_free (match->parens);
}


static gboolean
as_set_match (ASSet      *set,
              const char *text,
              ASMatch    *match)
{
    char *reversed;
    int start_pos, end_pos;
    gboolean found = FALSE;
    guint i;

    g_return_val_if_fail (text != NULL, -1);

    reversed = g_utf8_strreverse (text, -1);
    egg_regex_clear (set->regex);

    if (egg_regex_match (set->regex, reversed, -1, 0) <= 0)
        goto out;

    for (i = 0; i < set->n_strings; ++i)
    {
        egg_regex_fetch_pos (set->regex, reversed,
                             set->strings[i]->whole,
                             &start_pos, &end_pos);

        if (start_pos >= 0)
        {
            g_assert (start_pos == 0);
            found = TRUE;
            break;
        }
    }

    if (found)
    {
        match->string_no = i;
        match->start_offset = 0;
        match->end_offset = g_utf8_pointer_to_offset (reversed, reversed + end_pos);

        if (set->strings[i]->n_parens)
        {
            guint j;

            match->n_parens = set->strings[i]->n_parens;
            match->parens = g_new (guint, 2*match->n_parens);

            for (j = 0; j < match->n_parens; ++j)
            {
                egg_regex_fetch_pos (set->regex, reversed,
                                     set->strings[i]->parens[j],
                                     &start_pos, &end_pos);
                g_assert (start_pos >= 0);
                g_assert (end_pos >= 0);

                match->parens[2*j] =
                        g_utf8_pointer_to_offset (reversed, reversed + start_pos);
                match->parens[2*j + 1] = match->parens[2*j] +
                        g_utf8_pointer_to_offset (reversed + start_pos, reversed + end_pos);
            }
        }
        else
        {
            match->n_parens = 0;
            match->parens = NULL;
        }
    }

out:
    g_free (reversed);
    return found;
}


static gboolean
as_set_check_last (ASSet    *set,
                   gunichar  c)
{
    guint i;

    if (c < set->min_last || c > set->max_last)
        return FALSE;

    for (i = 0; i < set->n_last_chars; ++i)
        if (c == set->last_chars[i])
            return TRUE;

    return FALSE;
}


static void
as_set_unref (ASSet *set)
{
    guint i;

    g_return_if_fail (set != NULL);

    if (!--set->ref_count)
    {
        if (set->strings)
            for (i = 0; i < set->n_strings; ++i)
                as_string_free (set->strings[i]);
        g_free (set->strings);
        egg_regex_unref (set->regex);
        g_free (set->last_chars);
        g_free (set);
    }
}


static ASSet *as_set_ref (ASSet *set)
{
    g_return_val_if_fail (set != NULL, NULL);
    set->ref_count++;
    return set;
}


static gboolean
as_plugin_init (ASPlugin   *plugin)
{
#define N_STRINGS 5
    const char *strings[N_STRINGS] = {"\\begin{?}", "\\cite{?}", "\\some{?}", "BB", "FF"};
    ASStringInfo *info[N_STRINGS];
    int i;

    for (i = 0; i < N_STRINGS; ++i)
        info[i] = as_string_get_info (strings[i], 0, 0, i);

    plugin->set = as_set_new (info, N_STRINGS);

    return TRUE;
}


static void
as_plugin_deinit (ASPlugin   *plugin)
{
    as_set_unref (plugin->set);
    plugin->set = NULL;
}


static void
as_plugin_attach (ASPlugin   *plugin,
                  MooEdit    *doc)
{
    ASSet *set = as_set_ref (plugin->set);
    g_object_set_data (G_OBJECT (doc), AS_DATA, set);
    g_signal_connect (doc, "char-inserted",
                      G_CALLBACK (char_inserted_cb), set);
}


static void
as_plugin_detach (G_GNUC_UNUSED ASPlugin *plugin,
                  MooEdit    *doc)
{
    ASSet *set = g_object_get_data (G_OBJECT (doc), AS_DATA);
    g_object_set_data (G_OBJECT (doc), AS_DATA, NULL);
    g_signal_handlers_disconnect_by_func (doc, (gpointer) char_inserted_cb, set);
    as_set_unref (set);
}


static gboolean
char_inserted_cb (MooEdit        *doc,
                  GtkTextIter    *where,
                  guint           character,
                  ASSet          *set)
{
    GtkTextIter iter;
    char *slice;
    gboolean found;
    ASMatch match;

    g_return_val_if_fail (set != NULL, FALSE);

    if (!as_set_check_last (set, character))
        return FALSE;

    iter = *where;
    gtk_text_iter_set_line_offset (&iter, 0);

    /* get extra char here */
    slice = gtk_text_iter_get_slice (&iter, where);
    found = as_set_match (set, slice, &match);
    g_free (slice);

    if (!found)
        return FALSE;

    process_match (doc, where, set, &match);
    as_match_destroy (&match);
    return TRUE;
}


static void
process_match (MooEdit        *doc,
               GtkTextIter    *end,
               ASSet          *set,
               ASMatch        *match)
{
    GtkTextIter start;
    char *full_text = NULL;
    char **parens_text = NULL;
    guint i;
    ASPlugin *plugin;

    start = *end;
    gtk_text_iter_backward_chars (&start, match->end_offset);
    full_text = gtk_text_iter_get_slice (&start, end);

    if (match->n_parens)
    {
        parens_text = g_new (char*, match->n_parens + 1);
        parens_text[match->n_parens] = NULL;

        for (i = 0; i < match->n_parens; ++i)
        {
            GtkTextIter s, e = *end;
            gtk_text_iter_backward_chars (&e, match->parens[2*i]);
            s = e;
            gtk_text_iter_backward_chars (&s, match->parens[2*i + 1] - match->parens[2*i]);
            parens_text[i] = gtk_text_iter_get_slice (&s, &e);
        }
    }

    g_print ("found '%s'", full_text);

    if (match->n_parens)
    {
        g_print (": ");
        for (i = 0; i < match->n_parens; ++i)
            g_print ("'%s' ", parens_text[i]);
    }

    g_print ("\n");

    plugin = moo_doc_plugin_lookup (AS_PLUGIN_ID, doc);
    as_plugin_do_action (plugin, doc, end, set, match,
                         full_text, parens_text);

    g_free (full_text);
    g_strfreev (parens_text);
}


static void
as_plugin_do_action (G_GNUC_UNUSED ASPlugin       *plugin,
                     G_GNUC_UNUSED MooEdit        *doc,
                     G_GNUC_UNUSED GtkTextIter    *insert,
                     G_GNUC_UNUSED ASSet          *set,
                     G_GNUC_UNUSED ASMatch        *match,
                     G_GNUC_UNUSED char           *full_text,
                     G_GNUC_UNUSED char          **parens_text)
{
}


MOO_PLUGIN_DEFINE_INFO (as, AS_PLUGIN_ID,
                        "Active Strings", "Very active",
                        "Yevgen Muntyan <muntyan@tamu.edu>",
                        MOO_VERSION);
MOO_PLUGIN_DEFINE_FULL (AS, as,
                        as_plugin_init, as_plugin_deinit,
                        NULL, NULL,
                        as_plugin_attach, as_plugin_detach,
                        NULL, 0, 0);


gboolean
moo_active_strings_plugin_init (void)
{
    return moo_plugin_register (as_plugin_get_type ());
}
