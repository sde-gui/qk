/*
 *   as-plugin.c
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
#include "mooedit/plugins/mooeditplugins.h"
#include "mooedit/mooedit-script.h"
#include "mooutils/eggregex.h"
#include "mooutils/mooconfig.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/mooutils-fs.h"
#include "as-plugin-script.h"
#include "as-plugin.h"
#include "mooscript/mooscript-parser.h"
#include <string.h>

#define AS_DATA "moo-active-strings"
#define AS_WILDCARD '?'

#define AS_DEBUG 0

typedef struct _ASPlugin ASPlugin;
typedef struct _ASSet ASSet;
typedef struct _ASString ASString;
typedef struct _ASStringInfo ASStringInfo;
typedef struct _ASMatch ASMatch;
typedef struct _ASInfo ASInfo;


typedef enum {
    AS_WORD_BOUNDARY        = 1 << 0
} ASOptions;

struct _ASInfo {
    char *pattern;
    char *script;
    char *lang;
    guint enabled : 1;
    ASOptions start_opts : 1;
    ASOptions end_opts : 1;
};

struct _ASPlugin {
    MooPlugin parent;
    MSContext *ctx;
    GHashTable *lang_sets;
    ASSet *any_lang;
    GSList *docs;
};

struct _ASStringInfo {
    guint n_wildcards;
    char *pattern;
    guint pattern_len;
    gunichar last_char;
    char *script;
};

struct _ASString {
    guint whole;
    guint *parens;
    guint n_parens;
};

struct _ASSet {
    ASString **strings;
    char **scripts;
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
                                     GSList         *sets);
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

static void     as_string_info_set_script (ASStringInfo *info,
                                     const char     *script);
static void     as_string_info_free (ASStringInfo   *info);

static void     as_match_destroy    (ASMatch        *match);
static ASString *as_string_new      (guint           n_wildcards);
static void     as_string_free      (ASString       *s);


static ASInfo *
as_info_new (const char *pattern,
             const char *script,
             const char *lang,
             ASOptions   start_opts,
             ASOptions   end_opts,
             gboolean    enabled)
{
    ASInfo *info;

    g_return_val_if_fail (pattern && pattern[0], NULL);

    info = g_new0 (ASInfo, 1);
    info->pattern = (pattern && pattern[0]) ? g_strdup (pattern) : NULL;
    info->script = (script && script[0]) ? g_strdup (script) : NULL;
    info->lang = (lang && lang[0]) ? g_ascii_strdown (lang, -1) : NULL;
    info->enabled = enabled != 0;
    info->start_opts = start_opts;
    info->end_opts = end_opts;

    return info;
}


static void
as_info_free (ASInfo *info)
{
    if (info)
    {
        g_free (info->pattern);
        g_free (info->script);
        g_free (info->lang);
        g_free (info);
    }
}


static int
cmp_ints (gunichar *c1, gunichar *c2)
{
    return *c1 < *c2 ? -1 : (*c1 > *c2 ? 1 : 0);
}


static void
append_options (GString  *pattern,
                ASOptions opts,
                gunichar  ch)
{
    if ((opts & AS_WORD_BOUNDARY) &&
         (ch == '_' || g_unichar_isalpha (ch)))
            g_string_append (pattern, "\\b");
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
    gunichar first, last;

    if (!string || !string[0])
    {
        g_critical ("%s: empty string", G_STRLOC);
        goto error;
    }

    rev = g_utf8_strreverse (string, -1);
    rev_len = strlen (rev);
    info = g_new0 (ASStringInfo, 1);
    first = g_utf8_get_char (string);
    last = g_utf8_get_char (rev);

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
        append_options (pattern, end_opts, last);

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
        append_options (pattern, start_opts, first);

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
        g_free (info->script);
        g_free (info);
    }
}


static void
as_string_info_set_script (ASStringInfo *info,
                           const char   *script)
{
    g_return_if_fail (info != NULL);
    g_free (info->script);
    info->script = g_strdup (script);
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
#if AS_DEBUG
    g_print ("pattern: %s\n", pattern->str);
#endif

    if (!set->regex)
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

    set->scripts = g_new0 (char*, n_strings);

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

#if AS_DEBUG
    g_print ("last chars:");
    for (i = 0; i < set->n_last_chars; ++i)
        g_print (" %c", set->last_chars[i]);
    g_print ("\n");
#endif

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

    if (egg_regex_match (set->regex, reversed, 0) <= 0)
        goto out;

    for (i = 0; i < set->n_strings; ++i)
    {
        egg_regex_fetch_pos (set->regex,
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
                egg_regex_fetch_pos (set->regex,
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
    if (set && !--set->ref_count)
    {
        guint i;

        for (i = 0; i < set->n_strings; ++i)
        {
            as_string_free (set->strings[i]);
            g_free (set->scripts[i]);
        }

        g_free (set->strings);
        g_free (set->scripts);
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


static void
as_string_info_array_free (GPtrArray *ar)
{
    g_ptr_array_foreach (ar, (GFunc) as_string_info_free, NULL);
    g_ptr_array_free (ar, TRUE);
}


static void
add_lang_sets (const char *lang,
               GPtrArray  *strings,
               ASPlugin   *plugin)
{
    ASSet *set;

    g_return_if_fail (strings && strings->len);

    set = as_set_new ((ASStringInfo**) strings->pdata, strings->len);

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
as_plugin_load_info (ASPlugin *plugin,
                     GSList   *list)
{
    GPtrArray *any_lang;
    GHashTable *lang_strings;
    GSList *l;

    if (!list)
        return;

    any_lang = g_ptr_array_new ();
    lang_strings = g_hash_table_new_full (g_str_hash, g_str_equal, g_free,
                                          (GDestroyNotify) as_string_info_array_free);

    for (l = list; l != NULL; l = l->next)
    {
        GPtrArray *ar;
        ASStringInfo *sinfo;
        ASInfo *info = l->data;

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

        sinfo = as_string_get_info (info->pattern,
                                    info->start_opts,
                                    info->end_opts,
                                    ar->len);

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
                as_string_info_set_script (sinfo, info->script);
                ms_node_unref (script);
            }
        }

        g_ptr_array_add (ar, sinfo);
    }

    if (any_lang->len)
        plugin->any_lang = as_set_new ((ASStringInfo**) any_lang->pdata,
                                        any_lang->len);

    g_hash_table_foreach (lang_strings, (GHFunc) add_lang_sets, plugin);

    as_string_info_array_free (any_lang);
    g_hash_table_destroy (lang_strings);
}


static GSList *
as_plugin_parse_config (MooConfig *config)
{
    guint n_items, i;
    GSList *list = NULL;

    g_return_val_if_fail (config != NULL, NULL);

    n_items = moo_config_n_items (config);

    for (i = 0; i < n_items; ++i)
    {
        const char *pattern, *lang, *script;
        gboolean enabled, word_boundary;
        ASInfo *info;
        MooConfigItem *item = moo_config_nth_item (config, i);

        pattern = moo_config_item_get (item, AS_KEY_PATTERN);
        lang = moo_config_item_get (item, AS_KEY_LANG);
        script = moo_config_item_get_content (item);
        enabled = moo_config_get_bool (config, item, AS_KEY_ENABLED);
        word_boundary = moo_config_get_bool (config, item, AS_KEY_WORD_BOUNDARY);

        if (!pattern)
        {
            g_warning ("%s: patern missing", G_STRLOC);
            continue;
        }

        if (!script || !script[0])
            script = NULL;

        info = as_info_new (pattern, script, lang,
                            word_boundary ? AS_WORD_BOUNDARY : 0,
                            0, enabled);

        if (info)
            list = g_slist_prepend (list, info);
    }

    return g_slist_reverse (list);
}


static void
as_plugin_load (ASPlugin *plugin)
{
    GSList *info = NULL;
    MooConfig *config;

    config = _as_plugin_load_config ();

    if (!config)
        return;

    info = as_plugin_parse_config (config);

    if (info)
        as_plugin_load_info (plugin, info);

    g_slist_foreach (info, (GFunc) as_info_free, NULL);
    g_slist_free (info);
    g_object_unref (config);
}


static gboolean
as_plugin_init (ASPlugin *plugin)
{
    plugin->lang_sets = g_hash_table_new_full (g_str_hash, g_str_equal, g_free,
                                               (GDestroyNotify) as_set_unref);
    plugin->ctx = moo_edit_script_context_new (NULL, NULL);

    as_plugin_load (plugin);

    return TRUE;
}


static void
as_plugin_deinit (ASPlugin   *plugin)
{
    as_set_unref (plugin->any_lang);
    g_hash_table_destroy (plugin->lang_sets);
    g_object_unref (plugin->ctx);
    plugin->any_lang = NULL;
    plugin->lang_sets = NULL;
    plugin->ctx = NULL;
}


static GSList *
as_plugin_get_doc_sets (ASPlugin   *plugin,
                        MooEdit    *doc)
{
    char *lang = NULL;
    GSList *list = NULL;

    if (plugin->any_lang)
        list = g_slist_prepend (list, as_set_ref (plugin->any_lang));

    moo_edit_config_get (doc->config, "lang", &lang, NULL);

    if (lang)
    {
        ASSet *set = g_hash_table_lookup (plugin->lang_sets, lang);

        if (set)
            list = g_slist_prepend (list, as_set_ref (set));
    }

    g_free (lang);
    return list;
}


static void
free_sets_list (GSList *list)
{
    g_slist_foreach (list, (GFunc) as_set_unref, NULL);
    g_slist_free (list);
}


static void
as_plugin_connect_doc (ASPlugin   *plugin,
                       MooEdit    *doc)
{
    GSList *sets = as_plugin_get_doc_sets (plugin, doc);

    if (sets)
    {
        g_object_set_data_full (G_OBJECT (doc), AS_DATA, sets,
                                (GDestroyNotify) free_sets_list);
        g_signal_connect (doc, "char-inserted",
                          G_CALLBACK (char_inserted_cb), sets);
    }
}


static void
as_plugin_disconnect_doc (G_GNUC_UNUSED ASPlugin *plugin,
                          MooEdit    *doc)
{
    GSList *sets = g_object_get_data (G_OBJECT (doc), AS_DATA);

    if (sets)
    {
        g_signal_handlers_disconnect_by_func (doc, (gpointer) char_inserted_cb, sets);
        g_object_set_data (G_OBJECT (doc), AS_DATA, NULL);
    }
}


static void
as_plugin_reconnect_doc (MooEdit  *doc,
                         ASPlugin *plugin)
{
    as_plugin_disconnect_doc (plugin, doc);
    as_plugin_connect_doc (plugin, doc);
}


void
_as_plugin_reload (MooPlugin *mplugin)
{
    ASPlugin *plugin = (ASPlugin*) mplugin;
    as_plugin_deinit (plugin);
    as_plugin_init (plugin);
    g_slist_foreach (plugin->docs,
                     (GFunc) as_plugin_reconnect_doc,
                     plugin);
}


static void
lang_changed (MooEdit    *doc,
              G_GNUC_UNUSED guint var_id,
              G_GNUC_UNUSED GParamSpec *pspec,
              ASPlugin   *plugin)
{
    as_plugin_reconnect_doc (doc, plugin);
}


static void
as_plugin_attach (ASPlugin   *plugin,
                  MooEdit    *doc)
{
    as_plugin_connect_doc (plugin, doc);
    g_signal_connect (doc, "config_notify::lang",
                      G_CALLBACK (lang_changed), plugin);
    plugin->docs = g_slist_prepend (plugin->docs, doc);
}


static void
as_plugin_detach (ASPlugin   *plugin,
                  MooEdit    *doc)
{
    plugin->docs = g_slist_remove (plugin->docs, doc);
    as_plugin_disconnect_doc (plugin, doc);
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
    ASMatch match;
    GSList *l;

    g_return_val_if_fail (sets != NULL, FALSE);

    for (l = sets; l != NULL; l = l->next)
    {
        ASSet *set = l->data;

        if (!as_set_check_last (set, character))
            continue;

        iter = *where;
        gtk_text_iter_set_line_offset (&iter, 0);

        /* get extra char here */
        slice = gtk_text_iter_get_slice (&iter, where);
        found = as_set_match (set, slice, &match);
        g_free (slice);

        if (!found)
            continue;

        process_match (doc, where, set, &match);
        as_match_destroy (&match);
        return TRUE;
    }

    return FALSE;
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

#if AS_DEBUG
    g_print ("found '%s'", full_text);

    if (match->n_parens)
    {
        g_print (": ");
        for (i = 0; i < match->n_parens; ++i)
            g_print ("'%s' ", parens_text[i]);
    }

    g_print ("\n");
#endif

    plugin = moo_plugin_lookup (AS_PLUGIN_ID);
    g_return_if_fail (plugin != NULL);
    as_plugin_do_action (plugin, doc, end, set, match,
                         full_text, parens_text);

    g_free (full_text);
    g_strfreev (parens_text);
}


static void
as_plugin_do_action (ASPlugin       *plugin,
                     MooEdit        *doc,
                     GtkTextIter    *insert,
                     ASSet          *set,
                     ASMatch        *match,
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

    _as_plugin_context_exec (plugin->ctx, script, doc, insert,
                             full_text, parens_text, match->n_parens);

    ms_node_unref (script);
}


static char *
as_plugin_get_user_file (gboolean *is_default)
{
    char *file;

    moo_prefs_new_key_string (AS_FILE_PREFS_KEY, NULL);
    file = g_strdup (moo_prefs_get_filename (AS_FILE_PREFS_KEY));

    if (!file)
    {
        if (is_default)
            *is_default = TRUE;
        file = moo_get_user_data_file (AS_FILE);
    }
    else if (is_default)
    {
        *is_default = FALSE;
    }

    return file;
}


MooConfig *
_as_plugin_load_config (void)
{
    MooConfig *config;
    char *file;
    GError *error = NULL;

    file = as_plugin_get_user_file (NULL);

    if (!file || !g_file_test (file, G_FILE_TEST_EXISTS))
    {
        char **files;
        guint n_files;
        int i;

        g_free (file);
        file = NULL;
        files = moo_get_data_files (AS_FILE, MOO_DATA_SHARE, &n_files);

        for (i = n_files - 1; !file && i >= 0; --i)
            if (g_file_test (files[i], G_FILE_TEST_EXISTS))
                file = g_strdup (files[i]);

        g_strfreev (files);
    }

    if (!file)
        return NULL;

    config = moo_config_new_from_file (file, FALSE, &error);

    if (!config)
    {
        g_critical ("%s: %s", G_STRLOC, error->message);
        g_error_free (error);
    }
    else
    {
        moo_config_set_default_bool (config, AS_KEY_ENABLED, TRUE);
        moo_config_set_default_bool (config, AS_KEY_WORD_BOUNDARY, TRUE);
    }

    g_free (file);
    return config;
}


gboolean
_as_plugin_save_config (MooConfig *config,
                        GError   **error)
{
    char *file, *string;
    gboolean is_default, result;

    g_return_val_if_fail (MOO_IS_CONFIG (config), FALSE);

    file = as_plugin_get_user_file (&is_default);
    g_return_val_if_fail (file != NULL, FALSE);

    string = moo_config_format (config);

    if (is_default)
        result = moo_save_user_data_file (AS_FILE, string, -1, error);
    else
        result = _moo_save_file_utf8 (file, string, -1, error);

    g_free (string);
    return result;
}


MOO_PLUGIN_DEFINE_INFO (as,
                        "Active Strings", "Very active",
                        "Yevgen Muntyan <muntyan@tamu.edu>",
                        MOO_VERSION, NULL);
MOO_PLUGIN_DEFINE_FULL (AS, as,
                        as_plugin_init, as_plugin_deinit,
                        NULL, NULL,
                        as_plugin_attach, as_plugin_detach,
                        _as_plugin_prefs_page, 0, 0);


gboolean
_moo_active_strings_plugin_init (void)
{
    return moo_plugin_register (AS_PLUGIN_ID,
                                as_plugin_get_type (),
                                &as_plugin_info,
                                NULL);
}
