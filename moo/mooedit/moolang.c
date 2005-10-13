/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   moolang.c
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
#include <config.h>
#endif

#define MOOEDIT_COMPILATION
#include "mooedit/moolang.h"
#include "mooedit/moolang-rules.h"
#include "mooedit/moolang-parser.h"
#include "mooedit/moolang-strings.h"
#include "mooedit/mooeditprefs.h"
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
            moo_rule_free (ctx->rules->data[i]);
        g_ptr_array_free ((GPtrArray*) ctx->rules, TRUE);

        g_free (ctx);
    }
}


void
moo_context_add_rule (MooContext     *ctx,
                      MooRule        *rule)
{
    g_return_if_fail (ctx != NULL && rule != NULL);
    g_return_if_fail (rule->context == NULL);
    rule->context = ctx;
    g_ptr_array_add ((GPtrArray*) ctx->rules, rule);
}


void
moo_context_set_line_end_stay (MooContext *ctx)
{
    g_return_if_fail (ctx != NULL);
    ctx->line_end.type = MOO_CONTEXT_STAY;
    ctx->line_end.num = 0;
}


void
moo_context_set_line_end_pop (MooContext     *ctx,
                              guint           num)
{
    g_return_if_fail (ctx != NULL);
    g_return_if_fail (num > 0);
    ctx->line_end.type = MOO_CONTEXT_POP;
    ctx->line_end.num = num;
}


void
moo_context_set_line_end_switch (MooContext     *ctx,
                                 MooContext     *target)
{
    g_return_if_fail (ctx != NULL && target != NULL);
    ctx->line_end.type = MOO_CONTEXT_SWITCH;
    ctx->line_end.ctx = target;
}


/*****************************************************************************/
/* MooLang
 */

MooLang*
moo_lang_new (MooLangTable   *table,
              const char     *name,
              const char     *section,
              const char     *version,
              const char     *author)
{
    MooLang *lang;

    g_return_val_if_fail (table != NULL && name != NULL && name[0] != 0, NULL);
    g_return_val_if_fail (!g_hash_table_lookup (table->lang_names, name), NULL);

    lang = g_new0 (MooLang, 1);

    lang->table = table;

    lang->context_names = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
    lang->contexts = (MooContextArray*) g_ptr_array_new ();

    lang->style_names = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
    lang->styles = (MooTextStyleArray*) g_ptr_array_new ();
    lang->style_cache = g_hash_table_new_full (g_str_hash, g_str_equal, g_free,
                                               (GDestroyNotify) moo_text_style_free);

    lang->name = g_strdup (name);
    lang->section = section ? g_strdup (section) : g_strdup ("Others");
    lang->version = version ? g_strdup (version) : g_strdup ("");
    lang->author = author ? g_strdup (author) : g_strdup ("");

    table->langs = g_slist_append (table->langs, lang);
    g_hash_table_insert (table->lang_names, g_strdup (name), lang);

    return lang;
}


void
moo_lang_add_style (MooLang            *lang,
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
    g_return_val_if_fail (lang != NULL && lang->table != NULL, NULL);
    g_object_ref (lang->table);
    return lang;
}


void
moo_lang_unref (MooLang *lang)
{
    g_return_if_fail (lang != NULL && lang->table != NULL);
    g_object_unref (lang->table);
}


MooContext*
moo_lang_add_context (MooLang        *lang,
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
moo_lang_get_context (MooLang        *lang,
                      const char     *ctx_name)
{
    g_return_val_if_fail (lang != NULL, NULL);
    g_return_val_if_fail (ctx_name != NULL, NULL);
    return g_hash_table_lookup (lang->context_names, ctx_name);
}


MooContext*
moo_lang_get_default_context (MooLang *lang)
{
    g_return_val_if_fail (lang != NULL, NULL);
    g_return_val_if_fail (lang->contexts->len != 0, NULL);
    return lang->contexts->data[0];
}


static void
moo_lang_free (MooLang *lang)
{
    if (lang)
    {
        guint i;

        g_hash_table_destroy (lang->context_names);

        for (i = 0; i < lang->contexts->len; ++i)
            moo_context_free (lang->contexts->data[i]);
        g_ptr_array_free ((GPtrArray*) lang->contexts, TRUE);

        g_hash_table_destroy (lang->style_names);
        g_hash_table_destroy (lang->style_cache);

        for (i = 0; i < lang->styles->len; ++i)
            moo_text_style_free (lang->styles->data[i]);
        g_ptr_array_free ((GPtrArray*) lang->styles, TRUE);

        g_free (lang->name);
        g_free (lang->section);
        g_free (lang->version);
        g_free (lang->author);

        g_free (lang->brackets);

        g_slist_foreach (lang->mime_types, (GFunc) g_free, NULL);
        g_slist_free (lang->mime_types);
        g_slist_foreach (lang->extensions, (GFunc) g_free, NULL);
        g_slist_free (lang->extensions);

        g_free (lang);
    }
}


static void
moo_lang_parse_mime_types (MooLang      *lang,
                           const char   *mimetypes)
{
    char **pieces, **p;

    g_return_if_fail (lang != NULL && mimetypes != NULL);

    pieces = g_strsplit (mimetypes, EXTENSION_SEPARATOR, 0);

    if (!pieces)
        return;

    for (p = pieces; *p; p++)
        if (**p)
            lang->mime_types = g_slist_prepend (lang->mime_types, g_strdup (*p));

    g_strfreev (pieces);
}


static void
moo_lang_parse_extensions (MooLang      *lang,
                           const char   *extensions)
{
    char **pieces, **p;

    g_return_if_fail (lang != NULL && extensions != NULL);

    pieces = g_strsplit (extensions, EXTENSION_SEPARATOR, 0);

    if (!pieces)
        return;

    for (p = pieces; *p; p++)
        if (**p)
            lang->extensions = g_slist_prepend (lang->extensions, g_strdup (*p));

    g_strfreev (pieces);
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
               const char    *style_name)
{
    MooTextStyle *style;

    g_return_if_fail (lang != NULL && tag != NULL);

    if (!style_name)
        return;

    style = g_hash_table_lookup (lang->style_cache, style_name);

    if (!style)
    {
        style = moo_lang_table_get_style (lang->table, lang->name, style_name);
        g_return_if_fail (style != NULL);

        if (style->default_style)
        {
            MooTextStyle *def_style;

            def_style = moo_lang_table_get_style (lang->table, NULL, style->default_style);

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

        g_hash_table_insert (lang->style_cache, g_strdup (style_name), style);
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
}


void
_moo_lang_set_tag_style (MooLang            *lang,
                         GtkTextTag         *tag,
                         MooContext         *ctx,
                         MooRule            *rule)
{
    g_return_if_fail (lang != NULL && GTK_IS_TEXT_TAG (tag));
    g_return_if_fail (ctx != NULL && ctx->lang == lang);
    g_return_if_fail (!rule || rule->context == ctx);

    set_tag_style (lang, tag, ctx->style);

    if (rule)
        set_tag_style (lang, tag, rule->style);
}


static MooTextStyle*
moo_lang_get_style (MooLang     *lang,
                    const char  *style_name)
{
    MooTextStyle *style;
    g_return_val_if_fail (lang != NULL && style_name != NULL, NULL);
    style = g_hash_table_lookup (lang->style_names, style_name);
    return moo_text_style_copy (style);
}


static gboolean
parse_bool (const char *string,
            gboolean   *val)
{
    g_return_val_if_fail (string != NULL && val != NULL, FALSE);
    if (!g_ascii_strcasecmp (string, "true") ||
         !g_ascii_strcasecmp (string, "yes") ||
         !strcmp (string, "1"))
    {
        *val = TRUE;
        return TRUE;
    }
    else if (!g_ascii_strcasecmp (string, "false") ||
              !g_ascii_strcasecmp (string, "no") ||
              !strcmp (string, "0"))
    {
        *val = FALSE;
        return TRUE;
    }
    else
    {
        g_warning ("%s: could not get boolean value from '%s'",
                   G_STRLOC, string);
        return FALSE;
    }
}


static gboolean
parse_color (const char *string,
             GdkColor   *val)
{
    g_return_val_if_fail (string != NULL && val != NULL, FALSE);

    if (gdk_color_parse (string, val))
        return TRUE;

    g_warning ("%s: could not get color value from '%s'",
               G_STRLOC, string);
    return FALSE;
}


static MooTextStyle*
style_new_from_xml (StyleXML *xml)
{
    GdkColor foreground, background;
    gboolean bold, italic, underline, strikethrough;
    MooTextStyleMask mask = 0;

    g_return_val_if_fail (xml != NULL, NULL);

    if (xml->bold && parse_bool (xml->bold, &bold))
        mask |= MOO_TEXT_STYLE_BOLD;
    if (xml->italic && parse_bool (xml->italic, &italic))
        mask |= MOO_TEXT_STYLE_ITALIC;
    if (xml->underline && parse_bool (xml->underline, &underline))
        mask |= MOO_TEXT_STYLE_UNDERLINE;
    if (xml->strikethrough && parse_bool (xml->strikethrough, &strikethrough))
        mask |= MOO_TEXT_STYLE_STRIKETHROUGH;
    if (xml->foreground && parse_color (xml->foreground, &foreground))
        mask |= MOO_TEXT_STYLE_FOREGROUND;
    if (xml->background && parse_color (xml->background, &background))
        mask |= MOO_TEXT_STYLE_BACKGROUND;

    return moo_text_style_new (xml->default_style,
                               &foreground, &background,
                               bold, italic, underline,
                               strikethrough, mask, FALSE);
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


/*****************************************************************************/
/* MooLangTable
 */

static void     moo_lang_table_finalize     (GObject        *object);


/* MOO_TYPE_LANG_TABLE */
G_DEFINE_TYPE (MooLangTable, moo_lang_table, G_TYPE_OBJECT)

enum {
    LANG_ADDED,
    LANG_REMOVED,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

static void
moo_lang_table_class_init (MooLangTableClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = moo_lang_table_finalize;

    signals[LANG_ADDED] =
            g_signal_new ("lang-added",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooLangTableClass, lang_added),
                          NULL, NULL,
                          _moo_marshal_VOID__STRING,
                          G_TYPE_NONE, 1,
                          G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE);

    signals[LANG_REMOVED] =
            g_signal_new ("lang-removed",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooLangTableClass, lang_removed),
                          NULL, NULL,
                          _moo_marshal_VOID__STRING,
                          G_TYPE_NONE, 1,
                          G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE);
}


static void
moo_lang_table_init (MooLangTable *table)
{
    table->lang_names = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
    table->schemes = g_hash_table_new_full (g_str_hash, g_str_equal,
                                            g_free,
                                            (GDestroyNotify) moo_text_style_scheme_unref);
    table->active_scheme = moo_text_style_scheme_new_default ();
    g_hash_table_insert (table->schemes, g_strdup (table->active_scheme->name),
                         table->active_scheme);
    table->langs = NULL;
    table->dirs_read = FALSE;
}


MooLangTable*
moo_lang_table_new (void)
{
    return g_object_new (MOO_TYPE_LANG_TABLE, NULL);
}


static void
moo_lang_table_finalize (GObject *object)
{
    MooLangTable *table = MOO_LANG_TABLE (object);

    g_slist_foreach (table->lang_dirs, (GFunc) g_free, NULL);
    g_slist_free (table->lang_dirs);

    g_slist_foreach (table->langs, (GFunc) moo_lang_free, NULL);
    g_slist_free (table->langs);

    g_hash_table_destroy (table->lang_names);
    g_hash_table_destroy (table->schemes);
    table->active_scheme = NULL;

    G_OBJECT_CLASS(moo_lang_table_parent_class)->finalize (object);
}


void
moo_lang_table_add_dir (MooLangTable   *table,
                        const char     *dir)
{
    g_return_if_fail (MOO_IS_LANG_TABLE (table));
    g_return_if_fail (dir != NULL);
    g_return_if_fail (!table->dirs_read);
    table->lang_dirs = g_slist_append (table->lang_dirs, g_strdup (dir));
}


static MooTextStyle*
style_new_from_markup (MooMarkupNode *node,
                       gboolean       modified)
{
    GdkColor foreground, background;
    gboolean bold, italic, underline, strikethrough;
    const char *fg_prop, *bg_prop, *bold_prop, *italic_prop;
    const char *underline_prop, *strikethrough_prop;
    MooTextStyleMask mask = 0;

    g_return_val_if_fail (node != NULL, NULL);

    fg_prop = moo_markup_get_prop (node, STYLE_FOREGROUND_PROP);
    bg_prop = moo_markup_get_prop (node, STYLE_BACKGROUND_PROP);
    bold_prop = moo_markup_get_prop (node, STYLE_BOLD_PROP);
    italic_prop = moo_markup_get_prop (node, STYLE_ITALIC_PROP);
    underline_prop = moo_markup_get_prop (node, STYLE_UNDERLINE_PROP);
    strikethrough_prop = moo_markup_get_prop (node, STYLE_STRIKETHROUGH_PROP);

    if (bold_prop && parse_bool (bold_prop, &bold))
        mask |= MOO_TEXT_STYLE_BOLD;
    if (italic_prop && parse_bool (italic_prop, &italic))
        mask |= MOO_TEXT_STYLE_ITALIC;
    if (underline_prop && parse_bool (underline_prop, &underline))
        mask |= MOO_TEXT_STYLE_UNDERLINE;
    if (strikethrough_prop && parse_bool (strikethrough_prop, &strikethrough))
        mask |= MOO_TEXT_STYLE_STRIKETHROUGH;
    if (fg_prop && parse_color (fg_prop, &foreground))
        mask |= MOO_TEXT_STYLE_FOREGROUND;
    if (bg_prop && parse_color (bg_prop, &background))
        mask |= MOO_TEXT_STYLE_BACKGROUND;

    return moo_text_style_new (NULL,
                               &foreground, &background,
                               bold, italic, underline,
                               strikethrough, mask, modified);
}


static void
load_language_node (MooTextStyleScheme  *scheme,
                    MooMarkupNode       *node)
{
    MooMarkupNode *child;
    const char *lang_name;

    g_return_if_fail (node != NULL && node->name != NULL && !strcmp (node->name, LANGUAGE_ELM));
    g_return_if_fail (scheme != NULL);

    lang_name = moo_markup_get_prop (node, LANG_NAME_PROP);
    g_return_if_fail (lang_name && lang_name[0]);

    for (child = node->children; child != NULL; child = child->next)
    {
        MooTextStyle *style;
        const char *style_name;

        if (!MOO_MARKUP_IS_ELEMENT (child))
            continue;

        if (strcmp (child->name, STYLE_ELM))
        {
            g_warning ("%s: invalid element '%s'", G_STRLOC, child->name);
            continue;
        }

        style_name = moo_markup_get_prop (child, STYLE_NAME_PROP);

        if (!style_name || !style_name[0])
        {
            g_warning ("%s: style name absent", G_STRLOC);
            continue;
        }

        style = style_new_from_markup (child, TRUE);

        if (!style)
        {
            g_critical ("%s: could not parse style node", G_STRLOC);
            continue;
        }

        moo_text_style_scheme_compose (scheme, lang_name, style_name, style);
        moo_text_style_free (style);
    }
}


static void
moo_lang_table_choose_scheme (MooLangTable   *table,
                              MooMarkupNode  *node)
{
    const char *name = moo_markup_get_prop (node, DEFAULT_SCHEME_PROP);
    MooTextStyleScheme *scheme;

    if (!name)
        name = SCHEME_DEFAULT;

    scheme = g_hash_table_lookup (table->schemes, name);

    if (!scheme)
        scheme = g_hash_table_lookup (table->schemes, SCHEME_DEFAULT);

    g_return_if_fail (scheme != NULL);
    table->active_scheme = scheme;
}


static void
moo_lang_table_load_styles (MooLangTable   *table)
{
    MooMarkupDoc *xml;
    MooMarkupNode *root, *child;

    xml = moo_prefs_get_markup ();
    g_return_if_fail (xml != NULL);

    root = moo_markup_get_element (MOO_MARKUP_NODE (xml),
                                   MOO_STYLES_PREFS_PREFIX);

    if (!root)
        return;

    moo_lang_table_choose_scheme (table, root);

    for (child = root->children; child != NULL; child = child->next)
    {
        MooMarkupNode *grand_child;
        MooTextStyleScheme *scheme;
        const char *scheme_name;

        if (!MOO_MARKUP_IS_ELEMENT (child))
            continue;

        if (strcmp (child->name, SCHEME_ELM))
        {
            g_warning ("%s: invalid element '%s'", G_STRLOC, child->name);
            continue;
        }

        scheme_name = moo_markup_get_prop (child, SCHEME_NAME_PROP);

        if (!scheme_name || !scheme_name[0])
        {
            g_warning ("%s: scheme name missing", G_STRLOC);
            continue;
        }

        scheme = g_hash_table_lookup (table->schemes, scheme_name);

        if (!scheme)
            continue;

        for (grand_child = child->children; grand_child != NULL; grand_child = grand_child->next)
        {
            if (!MOO_MARKUP_IS_ELEMENT (grand_child))
                continue;

            if (!strcmp (grand_child->name, LANGUAGE_ELM))
            {
                load_language_node (scheme, grand_child);
            }
            else if (!strcmp (grand_child->name, DEFAULT_STYLE_ELM))
            {
                const char *name;
                MooTextStyle *style;

                name = moo_markup_get_prop (grand_child, STYLE_NAME_PROP);

                if (!name || !name[0])
                {
                    g_warning ("%s: style name absent", G_STRLOC);
                    continue;
                }

                style = style_new_from_markup (grand_child, TRUE);

                if (!style)
                {
                    g_critical ("%s: could not parse style node", G_STRLOC);
                    continue;
                }

                moo_text_style_scheme_compose (scheme, NULL, name, style);
                moo_text_style_free (style);
            }
            else
            {
                g_warning ("%s: invalid element '%s'", G_STRLOC, grand_child->name);
            }
        }
    }
}


static GSList*
read_files (MooLangTable   *table,
            GHashTable     *lang_xml_names)
{
    GSList *l;
    GSList *lang_xml_list = NULL;

    g_return_val_if_fail (table != NULL, NULL);
    g_return_val_if_fail (lang_xml_names != NULL, NULL);

    if (!table->lang_dirs)
        return NULL;

    for (l = table->lang_dirs; l != NULL; l = l->next)
    {
        const char *dirname = l->data, *entry;
        GDir *dir;

        dir = g_dir_open (dirname, 0, NULL);

        if (!dir)
            continue;

        while ((entry = g_dir_read_name (dir)))
        {
            guint entry_len = strlen (entry);

            if (entry_len > LANG_FILE_SUFFIX_LEN &&
                !strcmp (entry + (entry_len - LANG_FILE_SUFFIX_LEN), LANG_FILE_SUFFIX))
            {
                char *filename;
                LangXML *lang, *old_lang;

                filename = g_build_filename (dirname, entry, NULL);
                lang = moo_lang_parse_file (filename);
                g_free (filename);

                if (!lang)
                    continue;

                old_lang = g_hash_table_lookup (lang_xml_names, lang->name);

                if (old_lang)
                {
                    g_message ("%s: loading another instance of lang '%s'",
                               G_STRLOC, lang->name);
                    lang_xml_list = g_slist_remove (lang_xml_list, old_lang);
                    moo_lang_xml_free (old_lang);
                }

                lang_xml_list = g_slist_append (lang_xml_list, lang);
                g_hash_table_insert (lang_xml_names, g_strdup (lang->name), lang);
            }
            else if (entry_len > STYLES_FILE_SUFFIX_LEN &&
                     !strcmp (entry + (entry_len - STYLES_FILE_SUFFIX_LEN), STYLES_FILE_SUFFIX))
            {
                char *filename;
                MooTextStyleScheme *scheme;

                filename = g_build_filename (dirname, entry, NULL);
                scheme = moo_text_style_scheme_parse_file (filename);
                g_free (filename);

                if (!scheme)
                    continue;

                if (g_hash_table_lookup (table->schemes, scheme->name))
                {
                    g_message ("%s: loading another instance of scheme '%s'",
                               G_STRLOC, scheme->name);
                    if (!strcmp (table->active_scheme->name, scheme->name))
                        table->active_scheme = scheme;
                }

                g_hash_table_insert (table->schemes, g_strdup (scheme->name), scheme);
            }
        }

        g_dir_close (dir);
    }

    return lang_xml_list;
}


static GSList*
check_external_refs (GSList     *lang_xml_list,
                     GHashTable *lang_xml_names)
{
    gboolean again = TRUE;

    while (again)
    {
        GSList *l;

        again = FALSE;

        for (l = lang_xml_list; l != NULL; l = l->next)
        {
            GSList *ref_link;
            LangXML *xml = l->data;
            gboolean valid = TRUE;

            for (ref_link = xml->external_refs; ref_link != NULL; ref_link = ref_link->next)
            {
                CrossRef *ref = ref_link->data;
                LangXML *ref_lang;

                g_assert (ref->lang != NULL);

                ref_lang = g_hash_table_lookup (lang_xml_names, ref->lang);

                if (!ref_lang)
                {
                    g_warning ("%s: invalid reference to lang '%s' in lang '%s'",
                               G_STRLOC, ref->lang, xml->name);
                    g_hash_table_remove (lang_xml_names, xml->name);
                    lang_xml_list = g_slist_remove (lang_xml_list, xml);
                    moo_lang_xml_free (xml);
                    valid = FALSE;
                    break;
                }

                if (!g_hash_table_lookup (ref_lang->context_names, ref->name))
                {
                    g_warning ("%s: lang '%s' does not contain context '%s', referenced from lang '%s'",
                               G_STRLOC, ref->lang, ref->name, xml->name);
                    g_hash_table_remove (lang_xml_names, xml->name);
                    lang_xml_list = g_slist_remove (lang_xml_list, xml);
                    moo_lang_xml_free (xml);
                    valid = FALSE;
                    break;
                }
            }

            if (!valid)
            {
                again = TRUE;
                break;
            }
        }
    }

    return lang_xml_list;
}


static void
moo_lang_build_contexts (MooLang *lang,
                         LangXML *xml)
{
    GSList *l;

    g_assert (!strcmp (lang->name, xml->name));

    for (l = xml->syntax->contexts; l != NULL; l = l->next)
    {
        ContextXML *ctx_xml = l->data;
        MooContext *ctx = moo_lang_add_context (lang, ctx_xml->name, ctx_xml->style);
        g_assert (ctx != NULL);
    }
}


static void
moo_lang_finish_build (MooLang *lang,
                       LangXML *xml)
{
    GSList *l;

    g_assert (!strcmp (lang->name, xml->name));

    if (xml->style_list)
    {
        for (l = xml->style_list->styles; l != NULL; l = l->next)
        {
            StyleXML *style_xml = l->data;
            MooTextStyle *style = style_new_from_xml (style_xml);
            moo_lang_add_style (lang, style_xml->name, style);
            moo_text_style_free (style);
        }
    }

    if (xml->mimetypes)
        moo_lang_parse_mime_types (lang, xml->mimetypes);

    if (xml->extensions)
        moo_lang_parse_extensions (lang, xml->extensions);

    if (xml->general)
        lang->brackets = g_strdup (xml->general->brackets);

    for (l = xml->syntax->contexts; l != NULL; l = l->next)
    {
        GSList *rule_link;
        ContextXML *ctx_xml;
        MooContext *ctx, *switch_to;

        ctx_xml = l->data;
        ctx = moo_lang_get_context (lang, ctx_xml->name);
        g_assert (ctx != NULL);

        for (rule_link = ctx_xml->rules; rule_link != NULL; rule_link = rule_link->next)
        {
            RuleXML *rule_xml = rule_link->data;
            MooRule *rule = moo_rule_new_from_xml (rule_xml, xml, lang);
            if (rule)
                moo_context_add_rule (ctx, rule);
        }

        switch (ctx_xml->eol_switch_info.type)
        {
            case MOO_CONTEXT_STAY:
                moo_context_set_line_end_stay (ctx);
                break;
            case MOO_CONTEXT_POP:
                moo_context_set_line_end_pop (ctx, ctx_xml->eol_switch_info.num);
                break;
            case MOO_CONTEXT_SWITCH:
                if (ctx_xml->eol_switch_info.ref.lang)
                    switch_to = moo_lang_table_get_context (lang->table,
                                                            ctx_xml->eol_switch_info.ref.lang,
                                                            ctx_xml->eol_switch_info.ref.name);
                else
                    switch_to = moo_lang_get_context (lang, ctx_xml->eol_switch_info.ref.name);

                if (!switch_to)
                    g_critical ("%s: oops", G_STRLOC);
                else
                    moo_context_set_line_end_switch (ctx, switch_to);

                break;
        }
    }
}


void
moo_lang_table_read_dirs (MooLangTable   *table)
{
    GHashTable *lang_xml_names;
    GSList *lang_xml_list, *l;

    g_return_if_fail (MOO_IS_LANG_TABLE (table));
    g_return_if_fail (!table->dirs_read);

    if (!table->lang_dirs)
        return;

    lang_xml_names = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

    lang_xml_list = read_files (table, lang_xml_names);

    if (!lang_xml_list)
        goto out;

    lang_xml_list = check_external_refs (lang_xml_list, lang_xml_names);

    if (!lang_xml_list)
        goto out;

    table->dirs_read = TRUE;

    for (l = lang_xml_list; l != NULL; l = l->next)
    {
        LangXML *xml = l->data;
        MooLang *lang = moo_lang_new (table, xml->name,
                                      xml->section, xml->version,
                                      xml->author);
        lang->hidden = xml->hidden;
        moo_lang_build_contexts (lang, xml);
    }

    for (l = lang_xml_list; l != NULL; l = l->next)
    {
        LangXML *xml = l->data;
        MooLang *lang = moo_lang_table_get_lang (table, xml->name);
        g_assert (lang != NULL);
        moo_lang_finish_build (lang, xml);
    }

out:
    g_hash_table_destroy (lang_xml_names);
    g_slist_foreach (lang_xml_list, (GFunc) moo_lang_xml_free, NULL);
    g_slist_free (lang_xml_list);

    moo_lang_table_load_styles (table);
}


MooContext*
moo_lang_table_get_context (MooLangTable   *table,
                            const char     *lang_name,
                            const char     *ctx_name)
{
    MooLang *lang;
    g_return_val_if_fail (MOO_IS_LANG_TABLE (table), NULL);
    g_return_val_if_fail (lang_name && ctx_name, NULL);
    lang = moo_lang_table_get_lang (table, lang_name);
    g_return_val_if_fail (lang != NULL, NULL);
    return moo_lang_get_context (lang, ctx_name);
}


MooLang*
moo_lang_table_get_lang (MooLangTable   *table,
                         const char     *name)
{
    g_return_val_if_fail (MOO_IS_LANG_TABLE (table), NULL);
    g_return_val_if_fail (name != NULL, NULL);
    return g_hash_table_lookup (table->lang_names, name);
}


GSList*
moo_lang_table_get_available_langs (MooLangTable *table)
{
    g_return_val_if_fail (MOO_IS_LANG_TABLE (table), NULL);
    return g_slist_copy (table->langs);
}


static void
maybe_add_section (MooLang *lang,
                   GSList **list)
{
    if (!g_slist_find_custom (*list, lang->section, (GCompareFunc) strcmp))
        *list = g_slist_prepend (*list, g_strdup (lang->section));
}

GSList*
moo_lang_table_get_sections (MooLangTable *table)
{
    GSList *list = NULL;
    g_return_val_if_fail (MOO_IS_LANG_TABLE (table), NULL);
    g_slist_foreach (table->langs, (GFunc) maybe_add_section, &list);
    return list;
}


MooLang*
moo_lang_table_get_lang_for_file (MooLangTable   *table,
                                  const char     *filename)
{
    MooLang *lang = NULL;
    char *basename, *utf8_basename;
    GSList *l;
    gboolean found = FALSE;

    g_return_val_if_fail (MOO_IS_LANG_TABLE (table), NULL);
    g_return_val_if_fail (filename != NULL, NULL);

    /* TODO: is this right? */
    basename = g_path_get_basename (filename);
    g_return_val_if_fail (basename != NULL, NULL);
    utf8_basename = g_filename_display_name (basename);
    g_return_val_if_fail (utf8_basename != NULL, NULL);

    for (l = table->langs; !found && l != NULL; l = l->next)
    {
        GSList *g;
        lang = l->data;

        for (g = lang->extensions; !found && g != NULL; g = g->next)
        {
            if (g_pattern_match_simple ((char*) g->data, utf8_basename))
            {
                found = TRUE;
                break;
            }
        }
    }

    if (!found)
        lang = NULL;

#ifdef MOO_USE_XDGMIME
    /* XXX: xdgmime wants utf8-encoded filename here. is it a problem? */

    if (!lang)
    {
        const char *mime_type = xdg_mime_get_mime_type_for_file (filename);
        if (!xdg_mime_mime_type_equal (mime_type, XDG_MIME_TYPE_UNKNOWN))
            lang = moo_lang_table_get_lang_for_mime_type (table, mime_type);
    }
#endif /* MOO_USE_XDGMIME */

    if (!lang)
        lang = moo_lang_table_get_lang_for_filename (table, filename);

    g_free (utf8_basename);
    g_free (basename);
    return lang;
}


MooLang*
moo_lang_table_get_lang_for_filename (MooLangTable   *table,
                                      const char     *filename)
{
    MooLang *lang = NULL;
    char *basename, *utf8_basename;
    GSList *l;
    gboolean found = FALSE;

    g_return_val_if_fail (MOO_IS_LANG_TABLE (table), NULL);
    g_return_val_if_fail (filename != NULL, NULL);

    basename = g_path_get_basename (filename);
    g_return_val_if_fail (basename != NULL, NULL);
    utf8_basename = g_filename_display_name (basename);
    g_return_val_if_fail (utf8_basename != NULL, NULL);

    for (l = table->langs; l != NULL && !found; l = l->next)
    {
        GSList *g;

        lang = l->data;

        for (g = lang->extensions; !found && g != NULL; g = g->next)
            if (g_pattern_match_simple (g->data, utf8_basename))
                found = TRUE;
    }

    if (!found)
        lang = NULL;

#ifdef MOO_USE_XDGMIME
    /* XXX: xdgmime wants utf8-encoded filename here. is it a problem? */
    if (!lang)
    {
        const char *mime_type = xdg_mime_get_mime_type_from_file_name (filename);
        if (!xdg_mime_mime_type_equal (mime_type, XDG_MIME_TYPE_UNKNOWN))
            lang = moo_lang_table_get_lang_for_mime_type (table, mime_type);
    }
#endif /* MOO_USE_XDGMIME */

    /* check if it's backup file */
    if (!lang)
    {
        char *base = NULL;
        int len = strlen (utf8_basename);
        guint i;

        static const char *bak_globs[] = {"*~", "*.bak", "*.in"};

        /* XXX this is broken - it passes utf8 filename to get_lang_for_filename */

        for (i = 0; i < G_N_ELEMENTS (bak_globs); ++i)
        {
            int ext_len = strlen (bak_globs[i]) - 1;

            if (len > ext_len && g_pattern_match_simple (bak_globs[i], utf8_basename))
            {
                base = g_strndup (utf8_basename, len - ext_len);
                break;
            }
        }

        if (base)
        {
            lang = moo_lang_table_get_lang_for_filename (table, base);
            g_free (base);
        }
    }

    return lang;
}


MooLang*
moo_lang_table_get_lang_for_mime_type (MooLangTable   *table,
                                       const char     *mime_type)
{
    GSList *l;
    MooLang *lang = NULL;
    gboolean found = FALSE;

    g_return_val_if_fail (MOO_IS_LANG_TABLE (table), NULL);
    g_return_val_if_fail (mime_type != NULL, NULL);

    for (l = table->langs; !found && l != NULL; l = l->next)
    {
        lang = l->data;

        if (g_slist_find_custom (lang->mime_types, mime_type, (GCompareFunc) strcmp))
        {
            found = TRUE;
            break;
        }
    }

    return found ? lang : NULL;
}


MooTextStyle*
moo_lang_table_get_style (MooLangTable   *table,
                          const char     *lang_name,
                          const char     *style_name)
{
    const MooTextStyle *scheme_style = NULL;
    MooTextStyle *lang_style = NULL;

    g_return_val_if_fail (MOO_IS_LANG_TABLE (table), NULL);
    g_return_val_if_fail (style_name != NULL, NULL);
    g_return_val_if_fail (table->active_scheme != NULL, NULL);

    if (lang_name)
    {
        MooLang *lang = moo_lang_table_get_lang (table, lang_name);
        g_return_val_if_fail (lang != NULL, NULL);
        lang_style = moo_lang_get_style (lang, style_name);
    }

    scheme_style = moo_text_style_scheme_get (table->active_scheme,
                                              lang_name, style_name);

    if (lang_style)
    {
        if (scheme_style)
            moo_text_style_compose (lang_style, scheme_style);
        return lang_style;
    }
    else if (scheme_style)
    {
        return moo_text_style_copy (scheme_style);
    }
    else
    {
        return NULL;
    }
}


MooTextStyleScheme*
moo_lang_table_get_active_scheme (MooLangTable *table)
{
    g_return_val_if_fail (MOO_IS_LANG_TABLE (table), NULL);
    g_return_val_if_fail (table->active_scheme != NULL, NULL);
    return table->active_scheme;
}
