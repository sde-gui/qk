/*
 *   moolang.c
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
#include "mooedit/moolangmgr.h"
#include "mooedit/moolang-rules.h"
#include "mooedit/moolang-parser.h"
#include "mooedit/moolang-strings.h"
#include "mooedit/mooeditprefs.h"
#include "mooedit/moohighlighter.h"
#include "mooutils/xdgmime/xdgmime.h"
#include "mooutils/mooprefs.h"
#include "mooutils/moomarshals.h"
#include <string.h>


#define LANG_FILE_SUFFIX        ".lang"
#define LANG_FILE_SUFFIX_LEN    5
#define STYLES_FILE_SUFFIX      ".styles"
#define STYLES_FILE_SUFFIX_LEN  7
#define EXTENSION_SEPARATOR     ";"


/*****************************************************************************/
/* MooContext
 */

static MooContext*
moo_context_new (MooLang        *lang,
                 const char     *name,
                 const char     *style)
{
    MooContext *ctx;

    g_return_val_if_fail (lang != NULL, NULL);
    g_return_val_if_fail (name != NULL, NULL);

    ctx = g_new0 (MooContext, 1);
    ctx->lang = lang;
    ctx->name = g_strdup (name);
    ctx->style = g_strdup (style);

    ctx->rules = (MooRuleArray*) g_ptr_array_new ();

    return ctx;
}


static void
moo_context_free (MooContext *ctx)
{
    if (ctx)
    {
        guint i;

        g_free (ctx->name);
        g_free (ctx->style);

        for (i = 0; i < ctx->rules->len; ++i)
            _moo_rule_free (ctx->rules->data[i]);
        g_ptr_array_free ((GPtrArray*) ctx->rules, TRUE);

        g_free (ctx);
    }
}


void
_moo_context_add_rule (MooContext     *ctx,
                       MooRule        *rule)
{
    g_return_if_fail (ctx != NULL && rule != NULL);
    g_return_if_fail (rule->context == NULL);
    rule->context = ctx;
    g_ptr_array_add ((GPtrArray*) ctx->rules, rule);
}


void
_moo_context_set_eol_stay (MooContext *ctx)
{
    g_return_if_fail (ctx != NULL);
    ctx->line_end.type = MOO_CONTEXT_STAY;
    ctx->line_end.u.num = 0;
}


void
_moo_context_set_eol_pop (MooContext     *ctx,
                         guint           num)
{
    g_return_if_fail (ctx != NULL);
    g_return_if_fail (num > 0);
    ctx->line_end.type = MOO_CONTEXT_POP;
    ctx->line_end.u.num = num;
}


void
_moo_context_set_eol_switch (MooContext     *ctx,
                             MooContext     *target)
{
    g_return_if_fail (ctx != NULL && target != NULL);
    ctx->line_end.type = MOO_CONTEXT_SWITCH;
    ctx->line_end.u.ctx = target;
}


/*****************************************************************************/
/* MooLang
 */

static void lang_connect_scheme     (MooLang            *lang,
                                     MooTextStyleScheme *scheme);
static void lang_disconnect_scheme  (MooLang            *lang,
                                     MooTextStyleScheme *scheme);

static void
style_cache_create (MooLang *lang)
{
    g_return_if_fail (lang->style_cache == NULL);
    lang->style_cache = g_hash_table_new_full (g_direct_hash, g_direct_equal,
                                               NULL, (GDestroyNotify) g_hash_table_destroy);
}

static void
scheme_destroyed (MooLang            *lang,
                  MooTextStyleScheme *scheme)
{
    g_hash_table_remove (lang->style_cache, scheme);
}

static void
scheme_changed (MooTextStyleScheme *scheme,
                MooLang            *lang)
{
    g_hash_table_remove (lang->style_cache, scheme);
    lang_disconnect_scheme (lang, scheme);
}

static void
disconnect_scheme (MooTextStyleScheme *scheme,
                   G_GNUC_UNUSED GHashTable *scheme_cache,
                   MooLang            *lang)
{
    lang_disconnect_scheme (lang, scheme);
}

static void
style_cache_destroy (MooLang *lang)
{
    g_hash_table_foreach (lang->style_cache, (GHFunc) disconnect_scheme, lang);
    g_hash_table_destroy (lang->style_cache);
    lang->style_cache = NULL;
}

static MooTextStyle*
style_cache_lookup (MooLang            *lang,
                    const char         *style_name,
                    MooTextStyleScheme *scheme)
{
    GHashTable *scheme_cache = g_hash_table_lookup (lang->style_cache, scheme);
    return scheme_cache ? g_hash_table_lookup (scheme_cache, style_name) : NULL;
}

static void
style_cache_insert (MooLang            *lang,
                    const char         *style_name,
                    MooTextStyleScheme *scheme,
                    MooTextStyle       *style)
{
    GHashTable *scheme_cache = g_hash_table_lookup (lang->style_cache, scheme);

    if (!scheme_cache)
    {
        lang_connect_scheme (lang, scheme);
        scheme_cache = g_hash_table_new_full (g_str_hash, g_str_equal, g_free,
                                              (GDestroyNotify) moo_text_style_free);
        g_hash_table_insert (lang->style_cache, scheme, scheme_cache);
    }

    g_hash_table_insert (scheme_cache, g_strdup (style_name), style);
}

static void
lang_disconnect_scheme (MooLang            *lang,
                        MooTextStyleScheme *scheme)
{
    g_signal_handlers_disconnect_by_func (scheme,
                                          (gpointer) scheme_changed,
                                          lang);
    g_object_weak_unref (G_OBJECT (scheme), (GWeakNotify) scheme_destroyed, lang);
}

static void
lang_connect_scheme (MooLang            *lang,
                     MooTextStyleScheme *scheme)
{
    g_signal_connect (scheme, "changed", G_CALLBACK (scheme_changed), lang);
    g_object_weak_ref (G_OBJECT (scheme), (GWeakNotify) scheme_destroyed, lang);
}


MooLang*
_moo_lang_new (MooLangMgr     *mgr,
               const char     *name,
               const char     *section,
               const char     *version,
               const char     *author)
{
    MooLang *lang;

    g_return_val_if_fail (MOO_IS_LANG_MGR (mgr) && name != NULL && name[0] != 0, NULL);
    g_return_val_if_fail (!g_hash_table_lookup (mgr->lang_names, name), NULL);

    lang = g_new0 (MooLang, 1);

    lang->mgr = mgr;

    lang->context_names = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
    lang->contexts = (MooContextArray*) g_ptr_array_new ();

    lang->style_names = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
    lang->styles = (MooTextStyleArray*) g_ptr_array_new ();
    style_cache_create (lang);

    lang->id = moo_lang_id_from_name (name);
    lang->display_name = g_strdup (name);
    lang->section = section ? g_strdup (section) : g_strdup ("Others");
    lang->version = version ? g_strdup (version) : g_strdup ("");
    lang->author = author ? g_strdup (author) : g_strdup ("");

    _moo_lang_mgr_add_lang (mgr, lang);

    return lang;
}


char *
moo_lang_id_from_name (const char *name)
{
    g_return_val_if_fail (name != NULL, NULL);
    return g_ascii_strdown (name, -1);
}


const char *
moo_lang_id (MooLang *lang)
{
    return lang ? lang->id : MOO_LANG_NONE;
}


void
_moo_lang_add_style (MooLang            *lang,
                     const char         *name,
                     const MooTextStyle *style)
{
    MooTextStyle *copy;

    g_return_if_fail (lang != NULL);
    g_return_if_fail (name != NULL);
    g_return_if_fail (!g_hash_table_lookup (lang->style_names, name));

    if (style)
        copy = moo_text_style_copy (style);
    else
        copy = moo_text_style_new (NULL, NULL, NULL, 0, 0, 0, 0, 0, FALSE);

    g_ptr_array_add ((GPtrArray*) lang->styles, copy);
    g_hash_table_insert (lang->style_names, g_strdup (name), copy);
}


MooLang*
moo_lang_ref (MooLang *lang)
{
    g_return_val_if_fail (lang != NULL && lang->mgr != NULL, NULL);
    g_object_ref (lang->mgr);
    return lang;
}


void
moo_lang_unref (MooLang *lang)
{
    g_return_if_fail (lang != NULL && lang->mgr != NULL);
    g_object_unref (lang->mgr);
}


MooContext*
_moo_lang_add_context (MooLang        *lang,
                       const char     *name,
                       const char     *style)
{
    MooContext *ctx;

    g_return_val_if_fail (lang != NULL, NULL);
    g_return_val_if_fail (name != NULL && name[0] != 0, NULL);
    g_return_val_if_fail (!g_hash_table_lookup (lang->context_names, name), NULL);

    ctx = moo_context_new (lang, name, style);
    g_hash_table_insert (lang->context_names, g_strdup (name), ctx);
    g_ptr_array_add ((GPtrArray*) lang->contexts, ctx);

    return ctx;
}


MooContext*
_moo_lang_get_context (MooLang        *lang,
                       const char     *ctx_name)
{
    g_return_val_if_fail (lang != NULL, NULL);
    g_return_val_if_fail (ctx_name != NULL, NULL);
    return g_hash_table_lookup (lang->context_names, ctx_name);
}


MooContext*
_moo_lang_get_default_context (MooLang *lang)
{
    g_return_val_if_fail (lang != NULL, NULL);
    g_return_val_if_fail (lang->contexts->len != 0, NULL);
    return lang->contexts->data[0];
}


void
_moo_lang_free (MooLang *lang)
{
    if (lang)
    {
        guint i;

        g_hash_table_destroy (lang->context_names);

        for (i = 0; i < lang->contexts->len; ++i)
            moo_context_free (lang->contexts->data[i]);
        g_ptr_array_free ((GPtrArray*) lang->contexts, TRUE);

        g_hash_table_destroy (lang->style_names);
        style_cache_destroy (lang);

        for (i = 0; i < lang->styles->len; ++i)
            moo_text_style_free (lang->styles->data[i]);
        g_ptr_array_free ((GPtrArray*) lang->styles, TRUE);

        g_free (lang->id);
        g_free (lang->display_name);
        g_free (lang->section);
        g_free (lang->version);
        g_free (lang->author);
        g_free (lang->sample);

        g_free (lang->brackets);
        g_free (lang->line_comment);
        g_free (lang->block_comment_start);
        g_free (lang->block_comment_end);

        g_slist_foreach (lang->mime_types, (GFunc) g_free, NULL);
        g_slist_free (lang->mime_types);
        g_slist_foreach (lang->extensions, (GFunc) g_free, NULL);
        g_slist_free (lang->extensions);

        g_free (lang);
    }
}


GType
moo_lang_get_type (void)
{
    static GType type = 0;

    if (!type)
        type = g_boxed_type_register_static ("MooLang",
                                             (GBoxedCopyFunc) moo_lang_ref,
                                             (GBoxedFreeFunc) moo_lang_unref);

    return type;
}



static void
set_tag_style (MooLang       *lang,
               GtkTextTag    *tag,
               const char    *style_name,
               MooTextStyleScheme *scheme)
{
    MooTextStyle *style = NULL;

    g_return_if_fail (lang != NULL && tag != NULL);

    if (!style_name)
        return;

    if (!scheme)
        scheme = moo_lang_mgr_get_active_scheme (lang->mgr);
    g_return_if_fail (MOO_IS_TEXT_STYLE_SCHEME (scheme));

    style = style_cache_lookup (lang, style_name, scheme);

    if (!style)
    {
        style = _moo_lang_mgr_get_style (lang->mgr, lang->id, style_name, scheme);
        g_return_if_fail (style != NULL);

        if (style->default_style)
        {
            MooTextStyle *def_style;

            def_style = _moo_lang_mgr_get_style (lang->mgr, NULL, style->default_style, scheme);

            if (!def_style)
            {
                g_warning ("%s: invalid default style name '%s'",
                           G_STRLOC, style->default_style);
            }
            else
            {
                moo_text_style_compose (def_style, style);
                moo_text_style_free (style);
                style = def_style;
            }
        }

        style_cache_insert (lang, style_name, scheme, style);
    }

    if (style->mask & MOO_TEXT_STYLE_FOREGROUND)
        g_object_set (tag, "foreground-gdk", &style->foreground, NULL);
    if (style->mask & MOO_TEXT_STYLE_BACKGROUND)
        g_object_set (tag, "background-gdk", &style->background, NULL);
    if (style->mask & MOO_TEXT_STYLE_BOLD)
        g_object_set (tag, "weight", style->bold ? PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL, NULL);
    if (style->mask & MOO_TEXT_STYLE_ITALIC)
        g_object_set (tag, "style", style->italic ? PANGO_STYLE_ITALIC : PANGO_STYLE_NORMAL, NULL);
    if (style->mask & MOO_TEXT_STYLE_UNDERLINE)
        g_object_set (tag, "underline", style->underline ? PANGO_UNDERLINE_SINGLE : PANGO_UNDERLINE_NONE, NULL);
    if (style->mask & MOO_TEXT_STYLE_STRIKETHROUGH)
        g_object_set (tag, "strikethrough", (gboolean) (style->strikethrough ? TRUE : FALSE), NULL);

    if (MOO_IS_SYNTAX_TAG (tag) && style->mask != 0)
        MOO_SYNTAX_TAG(tag)->has_style = TRUE;
}


void
_moo_lang_set_tag_style (GtkTextTag         *tag,
                         MooContext         *ctx,
                         MooRule            *rule,
                         MooTextStyleScheme *scheme)
{
    g_return_if_fail (GTK_IS_TEXT_TAG (tag));
    g_return_if_fail (ctx != NULL);

    set_tag_style (ctx->lang, tag, ctx->style, scheme);

    if (rule)
        set_tag_style (rule->context->lang, tag, rule->style, scheme);
}


void
_moo_style_set_tag_style (const MooTextStyle *style,
                          GtkTextTag         *tag)
{
    g_return_if_fail (style != NULL);
    g_return_if_fail (GTK_IS_TEXT_TAG (tag));

    if (style->mask & MOO_TEXT_STYLE_BACKGROUND)
        g_object_set (tag, "background-gdk", &style->background, NULL);
    else
        g_object_set (tag, "background-set", FALSE, NULL);

    if (style->mask & MOO_TEXT_STYLE_FOREGROUND)
        g_object_set (tag, "foreground-gdk", &style->foreground, NULL);
    else
        g_object_set (tag, "foreground-set", FALSE, NULL);

    if (style->mask & MOO_TEXT_STYLE_BOLD)
        g_object_set (tag, "weight", style->bold ? PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL, NULL);
    else
        g_object_set (tag, "weight-set", FALSE, NULL);

    if (style->mask & MOO_TEXT_STYLE_ITALIC)
        g_object_set (tag, "style", style->italic ? PANGO_STYLE_ITALIC : PANGO_STYLE_NORMAL, NULL);
    else
        g_object_set (tag, "style-set", FALSE, NULL);

    if (style->mask & MOO_TEXT_STYLE_UNDERLINE)
        g_object_set (tag, "underline", style->underline ? PANGO_UNDERLINE_SINGLE : PANGO_UNDERLINE_NONE, NULL);
    else
        g_object_set (tag, "underline-set", FALSE, NULL);

    if (style->mask & MOO_TEXT_STYLE_STRIKETHROUGH)
        g_object_set (tag, "strikethrough", (gboolean) (style->strikethrough ? TRUE : FALSE), NULL);
    else
        g_object_set (tag, "strikethrough-set", FALSE, NULL);
}


void
_moo_lang_erase_tag_style (GtkTextTag         *tag)
{
    g_return_if_fail (GTK_IS_TEXT_TAG (tag));

    g_object_set (tag,
                  "background-set", FALSE,
                  "foreground-set", FALSE,
                  "weight-set", FALSE,
                  "style-set", FALSE,
                  "underline-set", FALSE,
                  "strikethrough-set", FALSE,
                  NULL);

    if (MOO_IS_SYNTAX_TAG (tag))
        MOO_SYNTAX_TAG(tag)->has_style = FALSE;
}
