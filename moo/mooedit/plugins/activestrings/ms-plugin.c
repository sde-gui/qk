/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   ms-plugin.c
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
#include "ms-plugin-script.h"
#include "ms-plugin-xml.h"
#include "mooutils/mooscript/mooscript-parser.h"
#include <string.h>

#define MS_PLUGIN_ID "ActiveStrings"
#define MS_DATA "moo-active-strings"
#define MS_WILDCARD '?'

#define PREFS_ROOT MOO_PLUGIN_PREFS_ROOT "/" MS_PLUGIN_ID
#define FILE_PREFS_KEY PREFS_ROOT "/file"


typedef struct _MSPlugin MSPlugin;
typedef struct _MSSet MSSet;
typedef struct _MSString MSString;
typedef struct _MSStringInfo MSStringInfo;
typedef struct _MSMatch MSMatch;


struct _MSPlugin {
    MooPlugin parent;
    MSContext *ctx;
    GHashTable *lang_sets;
    MSSet *any_lang;
};

struct _MSStringInfo {
    guint n_wildcards;
    char *pattern;
    guint pattern_len;
    gunichar last_char;
    char *script;
};

struct _MSString {
    guint whole;
    guint *parens;
    guint n_parens;
};

struct _MSSet {
    MSString **strings;
    char **scripts;
    guint n_strings;
    EggRegex *regex;
    guint ref_count;
    gunichar min_last;
    gunichar max_last;
    gunichar *last_chars;
    guint n_last_chars;
};

struct _MSMatch {
    guint string_no;
    /* all offsets are in chars here */
    guint start_offset;
    guint end_offset;
    guint *parens;
    guint n_parens;
};

typedef enum {
    MS_WORD_BOUNDARY        = 1 << 0,
    MS_NOT_WORD_BOUNDARY    = 1 << 1
} MSOptions;


static gboolean ms_plugin_init      (MSPlugin       *plugin);
static void     ms_plugin_deinit    (MSPlugin       *plugin);
static void     ms_plugin_attach    (MSPlugin       *plugin,
                                     MooEdit        *doc);
static void     ms_plugin_detach    (MSPlugin       *plugin,
                                     MooEdit        *doc);

static void     ms_plugin_do_action (MSPlugin       *plugin,
                                     MooEdit        *doc,
                                     GtkTextIter    *insert,
                                     MSSet          *set,
                                     MSMatch        *match,
                                     char           *full_text,
                                     char          **parens_text);

static gboolean char_inserted_cb    (MooEdit        *doc,
                                     GtkTextIter    *where,
                                     guint           character,
                                     GSList         *sets);
static void     process_match       (MooEdit        *doc,
                                     GtkTextIter    *end,
                                     MSSet          *set,
                                     MSMatch        *match);

static MSSet   *ms_set_new          (MSStringInfo  **strings,
                                     guint           n_strings);
static void     ms_set_unref        (MSSet          *set);
static MSSet   *ms_set_ref          (MSSet          *set);
static gboolean ms_set_check_last   (MSSet          *set,
                                     gunichar        c);
static gboolean ms_set_match        (MSSet          *set,
                                     const char     *text,
                                     MSMatch        *match);

static void     ms_string_info_set_script (MSStringInfo *info,
                                     const char     *script);
static void     ms_string_info_free (MSStringInfo   *info);

static void     ms_match_destroy    (MSMatch        *match);
static MSString *ms_string_new      (guint           n_wildcards);
static void     ms_string_free      (MSString       *s);


static int
cmp_ints (gunichar *c1, gunichar *c2)
{
    return *c1 < *c2 ? -1 : (*c1 > *c2 ? 1 : 0);
}


static void
append_options (GString  *pattern,
                MSOptions opts)
{
    if (opts & MS_WORD_BOUNDARY)
        g_string_append (pattern, "\\b");
    else if (opts & MS_NOT_WORD_BOUNDARY)
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


static MSStringInfo *
ms_string_get_info (const char *string,
                    MSOptions   start_opts,
                    MSOptions   end_opts,
                    guint       string_no)
{
    char *wc, *p;
    guint rev_len;
    MSStringInfo *info = NULL;
    char *rev = NULL;
    GString *pattern = NULL;

    if (!string || !string[0])
    {
        g_critical ("%s: empty string", G_STRLOC);
        goto error;
    }

    rev = g_utf8_strreverse (string, -1);
    rev_len = strlen (rev);
    info = g_new0 (MSStringInfo, 1);

    if (string[0] == MS_WILDCARD && string[1] != MS_WILDCARD)
    {
        g_critical ("%s: leading '%c' symbol", G_STRLOC, MS_WILDCARD);
        goto error;
    }

    if (rev[0] == MS_WILDCARD && rev[1] != MS_WILDCARD)
    {
        g_critical ("%s: trailing '%c' symbol", G_STRLOC, MS_WILDCARD);
        goto error;
    }

    info->last_char = g_utf8_get_char (rev);
    pattern = g_string_sized_new (rev_len + 16);

    if (end_opts)
        append_options (pattern, end_opts);

    g_string_append_printf (pattern, "(?P<%d>", string_no);

    for (p = rev; *p; )
    {
        wc = strchr (p, MS_WILDCARD);

        if (!wc)
        {
            append_escaped (pattern, p, -1);
            break;
        }

        if (wc[1] == MS_WILDCARD)
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
    ms_string_info_free (info);
    g_free (rev);
    if (pattern)
        g_string_free (pattern, TRUE);
    return NULL;
}


static void
ms_string_info_free (MSStringInfo *info)
{
    if (info)
    {
        g_free (info->pattern);
        g_free (info->script);
        g_free (info);
    }
}


static void
ms_string_info_set_script (MSStringInfo *info,
                           const char   *script)
{
    g_return_if_fail (info != NULL);
    g_free (info->script);
    info->script = g_strdup (script);
}


static MSString *
ms_string_new (guint n_wildcards)
{
    MSString *s = g_new0 (MSString, 1);

    if (n_wildcards)
    {
        s->n_parens = n_wildcards;
        s->parens = g_new0 (guint, s->n_parens);
    }

    return s;
}


static void
ms_string_free (MSString *s)
{
    if (s)
    {
        g_free (s->parens);
        g_free (s);
    }
}


static MSSet*
ms_set_new (MSStringInfo **strings,
            guint          n_strings)
{
    MSSet *set = NULL;
    GString *pattern;
    guint i, len;
    GError *error = NULL;
    gunichar *last_chars;
    gunichar min_last = 0, max_last = 0;
    gboolean hms_wildcard = FALSE;

    g_return_val_if_fail (strings != NULL, NULL);
    g_return_val_if_fail (n_strings != 0, NULL);

    set = g_new0 (MSSet, 1);
    set->ref_count = 1;
    set->n_strings = n_strings;
    set->strings = g_new (MSString*, n_strings);

    len = n_strings + 2; /* (||||) */
    last_chars = g_new (gunichar, n_strings);

    for (i = 0; i < n_strings; ++i)
    {
        last_chars[i] = strings[i]->last_char;
        min_last = MIN (min_last, last_chars[i]);
        max_last = MAX (max_last, last_chars[i]);

        len += strings[i]->pattern_len;

        set->strings[i] = ms_string_new (strings[i]->n_wildcards);

        if (strings[i]->n_wildcards)
            hms_wildcard = TRUE;
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

    set->scripts = g_new (char*, n_strings);

    for (i = 0; i < n_strings; ++i)
    {
        const char *script = strings[i]->script;

        if (script && script[0])
            set->scripts[i] = g_strdup (script);
    }

    g_qsort_with_data (last_chars, n_strings, sizeof (gunichar),
                       (GCompareDataFunc) cmp_ints, NULL);

    for (i = 1, len = 1; i < n_strings; ++i)
        if (last_chars[i] != last_chars[len-1])
            if (len++ != i)
                last_chars[len-1] = last_chars[i];

    set->last_chars = g_memdup (last_chars, len * sizeof (gunichar));
    set->n_last_chars = len;

    g_print ("last chars:");
    for (i = 0; i < set->n_last_chars; ++i)
        g_print (" %c", set->last_chars[i]);
    g_print ("\n");

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
    ms_set_unref (set);
    return NULL;
}


static void
ms_match_destroy (MSMatch *match)
{
    if (match->n_parens)
        g_free (match->parens);
}


static gboolean
ms_set_match (MSSet      *set,
              const char *text,
              MSMatch    *match)
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
ms_set_check_last (MSSet    *set,
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
ms_set_unref (MSSet *set)
{
    guint i;

    if (!set)
        return;

    if (!--set->ref_count)
    {
        for (i = 0; i < set->n_strings; ++i)
        {
            ms_string_free (set->strings[i]);
            g_free (set->scripts[i]);
        }

        g_free (set->strings);
        g_free (set->scripts);
        egg_regex_unref (set->regex);
        g_free (set->last_chars);
        g_free (set);
    }
}


static MSSet *ms_set_ref (MSSet *set)
{
    g_return_val_if_fail (set != NULL, NULL);
    set->ref_count++;
    return set;
}


static void
ms_string_info_array_free (GPtrArray *ar)
{
    g_ptr_array_foreach (ar, (GFunc) ms_string_info_free, NULL);
    g_ptr_array_free (ar, TRUE);
}


static void
add_lang_sets (const char *lang,
               GPtrArray  *strings,
               MSPlugin   *plugin)
{
    MSSet *set;

    g_return_if_fail (strings && strings->len);

    set = ms_set_new ((MSStringInfo**) strings->pdata, strings->len);

    if (set)
        g_hash_table_insert (plugin->lang_sets, g_strdup (lang), set);
}


static gboolean
is_nonblank_string (const char *string)
{
    if (!string)
        return FALSE;

    while (*string)
    {
        if (!g_ascii_isspace (*string))
            return TRUE;
        string++;
    }

    return FALSE;
}


static void
ms_plugin_load_info (MSPlugin *plugin,
                     GSList   *list)
{
    GPtrArray *any_lang;
    GHashTable *lang_strings;
    GSList *l;

    if (!list)
        return;

    any_lang = g_ptr_array_new ();
    lang_strings = g_hash_table_new_full (g_str_hash, g_str_equal, g_free,
                                          (GDestroyNotify) ms_string_info_array_free);

    for (l = list; l != NULL; l = l->next)
    {
        GPtrArray *ar;
        MSStringInfo *sinfo;
        MSInfo *info = l->data;

        if (info->lang)
        {
            ar = g_hash_table_lookup (lang_strings, info->lang);

            if (!ar)
            {
                ar = g_ptr_array_new ();
                g_hash_table_insert (lang_strings, g_strdup (info->lang), ar);
            }
        }
        else
        {
            ar = any_lang;
        }

        sinfo = ms_string_get_info (info->pattern, 0, 0, ar->len);

        if (!sinfo)
        {
            g_warning ("%s: invalid pattern '%s'", G_STRLOC, info->pattern);
            continue;
        }

        if (is_nonblank_string (info->script))
        {
            MSNode *script = ms_script_parse (info->script);

            if (script)
            {
                ms_string_info_set_script (sinfo, info->script);
                g_object_unref (script);
            }
        }

        g_ptr_array_add (ar, sinfo);
    }

    if (any_lang->len)
        plugin->any_lang = ms_set_new ((MSStringInfo**) any_lang->pdata,
                                        any_lang->len);

    g_hash_table_foreach (lang_strings, (GHFunc) add_lang_sets, plugin);

    ms_string_info_array_free (any_lang);
    g_hash_table_destroy (lang_strings);
}


static void
ms_plugin_load (MSPlugin *plugin)
{
    const char *file;
    GSList *info = NULL;

    moo_prefs_new_key_string (FILE_PREFS_KEY, NULL);

    file = moo_prefs_get_filename (FILE_PREFS_KEY);

    if (file)
        _ms_load_file (file, &info);

    if (info)
        ms_plugin_load_info (plugin, info);

    g_slist_foreach (info, (GFunc) _ms_info_free, NULL);
    g_slist_free (info);
}


static gboolean
ms_plugin_init (MSPlugin *plugin)
{
    plugin->lang_sets = g_hash_table_new_full (g_str_hash, g_str_equal, g_free,
                                               (GDestroyNotify) ms_set_unref);
    plugin->ctx = _ms_plugin_context_new ();

    ms_plugin_load (plugin);

    return TRUE;
}


static void
ms_plugin_deinit (MSPlugin   *plugin)
{
    ms_set_unref (plugin->any_lang);
    g_hash_table_destroy (plugin->lang_sets);
    g_object_unref (plugin->ctx);
    plugin->any_lang = NULL;
    plugin->lang_sets = NULL;
    plugin->ctx = NULL;
}


static void
free_sets_list (GSList *list)
{
    g_slist_foreach (list, (GFunc) ms_set_unref, NULL);
    g_slist_free (list);
}


static GSList *
ms_plugin_get_doc_sets (MSPlugin   *plugin,
                        MooEdit    *doc)
{
    char *lang = NULL;
    GSList *list = NULL;

    if (plugin->any_lang)
        list = g_slist_prepend (list, ms_set_ref (plugin->any_lang));

    moo_edit_config_get (doc->config, "lang", &lang, NULL);

    if (lang)
    {
        MSSet *set = g_hash_table_lookup (plugin->lang_sets, lang);

        if (set)
            list = g_slist_prepend (list, ms_set_ref (set));
    }

    g_free (lang);
    return list;
}


static void
ms_plugin_connect_doc (MSPlugin   *plugin,
                       MooEdit    *doc)
{
    GSList *sets = ms_plugin_get_doc_sets (plugin, doc);

    if (sets)
    {
        g_object_set_data_full (G_OBJECT (doc), MS_DATA, sets,
                                (GDestroyNotify) free_sets_list);
        g_signal_connect (doc, "char-inserted",
                          G_CALLBACK (char_inserted_cb), sets);
    }
}


static void
ms_plugin_disconnect_doc (G_GNUC_UNUSED MSPlugin *plugin,
                          MooEdit    *doc)
{
    GSList *sets = g_object_get_data (G_OBJECT (doc), MS_DATA);

    if (sets)
    {
        g_signal_handlers_disconnect_by_func (doc, (gpointer) char_inserted_cb, sets);
        g_object_set_data (G_OBJECT (doc), MS_DATA, NULL);
    }
}


static void
lang_changed (MooEdit    *doc,
              G_GNUC_UNUSED guint var_id,
              G_GNUC_UNUSED GParamSpec *pspec,
              MSPlugin   *plugin)
{
    ms_plugin_disconnect_doc (plugin, doc);
    ms_plugin_connect_doc (plugin, doc);
}


static void
ms_plugin_attach (MSPlugin   *plugin,
                  MooEdit    *doc)
{
    ms_plugin_connect_doc (plugin, doc);
    g_signal_connect (doc, "config_notify::lang",
                      G_CALLBACK (lang_changed), plugin);
}


static void
ms_plugin_detach (MSPlugin   *plugin,
                  MooEdit    *doc)
{
    ms_plugin_disconnect_doc (plugin, doc);
    g_signal_handlers_disconnect_by_func (doc, (gpointer) lang_changed, plugin);
}


static gboolean
char_inserted_cb (MooEdit        *doc,
                  GtkTextIter    *where,
                  guint           character,
                  GSList         *sets)
{
    GtkTextIter iter;
    char *slice;
    gboolean found;
    MSMatch match;
    GSList *l;

    g_return_val_if_fail (sets != NULL, FALSE);

    for (l = sets; l != NULL; l = l->next)
    {
        MSSet *set = l->data;

        if (!ms_set_check_last (set, character))
            continue;

        iter = *where;
        gtk_text_iter_set_line_offset (&iter, 0);

        /* get extra char here */
        slice = gtk_text_iter_get_slice (&iter, where);
        found = ms_set_match (set, slice, &match);
        g_free (slice);

        if (!found)
            continue;

        process_match (doc, where, set, &match);
        ms_match_destroy (&match);
        return TRUE;
    }

    return FALSE;
}


static void
process_match (MooEdit        *doc,
               GtkTextIter    *end,
               MSSet          *set,
               MSMatch        *match)
{
    GtkTextIter start;
    char *full_text = NULL;
    char **parens_text = NULL;
    guint i;
    MSPlugin *plugin;

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

    plugin = moo_plugin_lookup (MS_PLUGIN_ID);
    g_return_if_fail (plugin != NULL);
    ms_plugin_do_action (plugin, doc, end, set, match,
                         full_text, parens_text);

    g_free (full_text);
    g_strfreev (parens_text);
}


static void
ms_plugin_do_action (MSPlugin       *plugin,
                     MooEdit        *doc,
                     GtkTextIter    *insert,
                     MSSet          *set,
                     MSMatch        *match,
                     char           *full_text,
                     char          **parens_text)
{
    MSNode *script;
    const char *code = set->scripts[match->string_no];

    if (!code)
        return;

    script = ms_script_parse (code);

    if (!script)
    {
        g_critical ("%s: oops", G_STRLOC);
        return;
    }

    _ms_plugin_context_exec (plugin->ctx, script, doc, insert,
                             full_text, parens_text, match->n_parens);

    g_object_unref (script);
}


MOO_PLUGIN_DEFINE_INFO (ms, MS_PLUGIN_ID,
                        "Active Strings", "Very active",
                        "Yevgen Muntyan <muntyan@tamu.edu>",
                        MOO_VERSION);
MOO_PLUGIN_DEFINE_FULL (MS, ms,
                        ms_plugin_init, ms_plugin_deinit,
                        NULL, NULL,
                        ms_plugin_attach, ms_plugin_detach,
                        NULL, 0, 0);


gboolean
_moo_active_strings_plugin_init (void)
{
    return moo_plugin_register (ms_plugin_get_type ());
}
