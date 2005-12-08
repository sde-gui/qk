/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   moolangmgr.c
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
#include "mooedit/moolangmgr.h"
#include "mooedit/moolang-strings.h"
#include "mooedit/moolang-parser.h"
#include "mooedit/mooeditprefs.h"
#include "mooutils/xdgmime/xdgmime.h"
#include "mooutils/mooprefs.h"
#include "mooutils/moomarshals.h"
#include "mooutils/moocompat.h"
#include <string.h>


#define LANG_FILE_SUFFIX        ".lang"
#define LANG_FILE_SUFFIX_LEN    5
#define STYLES_FILE_SUFFIX      ".styles"
#define STYLES_FILE_SUFFIX_LEN  7
#define EXTENSION_SEPARATOR     ";"


static void     moo_lang_mgr_finalize     (GObject        *object);


/* MOO_TYPE_LANG_MGR */
G_DEFINE_TYPE (MooLangMgr, moo_lang_mgr, G_TYPE_OBJECT)

enum {
    LANG_ADDED,
    LANG_REMOVED,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

static void
moo_lang_mgr_class_init (MooLangMgrClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = moo_lang_mgr_finalize;

    signals[LANG_ADDED] =
            g_signal_new ("lang-added",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooLangMgrClass, lang_added),
                          NULL, NULL,
                          _moo_marshal_VOID__STRING,
                          G_TYPE_NONE, 1,
                          G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE);

    signals[LANG_REMOVED] =
            g_signal_new ("lang-removed",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooLangMgrClass, lang_removed),
                          NULL, NULL,
                          _moo_marshal_VOID__STRING,
                          G_TYPE_NONE, 1,
                          G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE);
}


static void
moo_lang_mgr_init (MooLangMgr *mgr)
{
    mgr->lang_names = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
    mgr->schemes = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
    mgr->active_scheme = moo_text_style_scheme_new_default ();
    g_hash_table_insert (mgr->schemes, g_strdup (mgr->active_scheme->name),
                         mgr->active_scheme);
    mgr->langs = NULL;
    mgr->dirs_read = FALSE;
}


MooLangMgr*
moo_lang_mgr_new (void)
{
    return g_object_new (MOO_TYPE_LANG_MGR, NULL);
}


static void
moo_lang_mgr_finalize (GObject *object)
{
    MooLangMgr *mgr = MOO_LANG_MGR (object);

    g_slist_foreach (mgr->lang_dirs, (GFunc) g_free, NULL);
    g_slist_free (mgr->lang_dirs);

    g_slist_foreach (mgr->langs, (GFunc) _moo_lang_free, NULL);
    g_slist_free (mgr->langs);

    g_hash_table_destroy (mgr->lang_names);
    g_hash_table_destroy (mgr->schemes);
    mgr->active_scheme = NULL;

    G_OBJECT_CLASS(moo_lang_mgr_parent_class)->finalize (object);
}


void
moo_lang_mgr_add_dir (MooLangMgr   *mgr,
                      const char   *dir)
{
    g_return_if_fail (MOO_IS_LANG_MGR (mgr));
    g_return_if_fail (dir != NULL);
    g_return_if_fail (!mgr->dirs_read);
    mgr->lang_dirs = g_slist_append (mgr->lang_dirs, g_strdup (dir));
}


static gboolean
parse_bool (const char         *string,
            gboolean           *val)
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


void
moo_lang_mgr_set_active_scheme (MooLangMgr *mgr,
                                const char *name)
{
    MooTextStyleScheme *scheme;

    g_return_if_fail (MOO_IS_LANG_MGR (mgr));

    if (!name)
        name = SCHEME_DEFAULT;

    scheme = g_hash_table_lookup (mgr->schemes, name);

    if (!scheme)
        scheme = g_hash_table_lookup (mgr->schemes, SCHEME_DEFAULT);

    g_return_if_fail (scheme != NULL);
    mgr->active_scheme = scheme;
}


static void
moo_lang_mgr_choose_scheme (MooLangMgr     *mgr,
                            MooMarkupNode  *node)
{
    const char *name = moo_markup_get_prop (node, DEFAULT_SCHEME_PROP);
    moo_lang_mgr_set_active_scheme (mgr, name);
}


static void
moo_lang_mgr_load_styles (MooLangMgr   *mgr)
{
    MooMarkupDoc *xml;
    MooMarkupNode *root, *child;

    xml = moo_prefs_get_markup ();
    g_return_if_fail (xml != NULL);

    root = moo_markup_get_element (MOO_MARKUP_NODE (xml),
                                   MOO_STYLES_PREFS_PREFIX);

    if (!root)
        return;

    moo_lang_mgr_choose_scheme (mgr, root);

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

        scheme = g_hash_table_lookup (mgr->schemes, scheme_name);

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
read_files (MooLangMgr   *mgr,
            GHashTable   *lang_xml_names)
{
    GSList *l;
    GSList *lang_xml_list = NULL;

    g_return_val_if_fail (mgr != NULL, NULL);
    g_return_val_if_fail (lang_xml_names != NULL, NULL);

    if (!mgr->lang_dirs)
        return NULL;

    for (l = mgr->lang_dirs; l != NULL; l = l->next)
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
                scheme = moo_text_style_scheme_parse_file (filename, NULL); /* XXX */
                g_free (filename);

                if (!scheme)
                    continue;

                if (g_hash_table_lookup (mgr->schemes, scheme->name))
                {
                    g_message ("%s: loading another instance of scheme '%s'",
                               G_STRLOC, scheme->name);
                    if (!strcmp (mgr->active_scheme->name, scheme->name))
                        mgr->active_scheme = scheme;
                }

                /* XXX base scheme */

                g_hash_table_insert (mgr->schemes, g_strdup (scheme->name), scheme);
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

    g_assert (!strcmp (lang->display_name, xml->name));

    for (l = xml->syntax->contexts; l != NULL; l = l->next)
    {
        ContextXML *ctx_xml = l->data;
        MooContext *ctx = moo_lang_add_context (lang, ctx_xml->name, ctx_xml->style);
        g_assert (ctx != NULL);
    }
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
        if (**p)/*XXX*/
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


static void
moo_lang_finish_build (MooLang *lang,
                       LangXML *xml)
{
    GSList *l;

    g_assert (!strcmp (lang->display_name, xml->name));

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
    lang->sample = g_strdup (xml->sample);

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
                moo_context_set_eol_stay (ctx);
                break;
            case MOO_CONTEXT_POP:
                moo_context_set_eol_pop (ctx, ctx_xml->eol_switch_info.num);
                break;
            case MOO_CONTEXT_SWITCH:
                if (ctx_xml->eol_switch_info.ref.lang)
                    switch_to = moo_lang_mgr_get_context (lang->mgr,
                                                          ctx_xml->eol_switch_info.ref.lang,
                                                          ctx_xml->eol_switch_info.ref.name);
                else
                    switch_to = moo_lang_get_context (lang, ctx_xml->eol_switch_info.ref.name);

                if (!switch_to)
                    g_critical ("%s: oops", G_STRLOC);
                else
                    moo_context_set_eol_switch (ctx, switch_to);

                break;
        }
    }
}


void
moo_lang_mgr_read_dirs (MooLangMgr   *mgr)
{
    GHashTable *lang_xml_names;
    GSList *lang_xml_list, *l;

    g_return_if_fail (MOO_IS_LANG_MGR (mgr));
    g_return_if_fail (!mgr->dirs_read);

    if (!mgr->lang_dirs)
        return;

    lang_xml_names = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

    lang_xml_list = read_files (mgr, lang_xml_names);

    if (!lang_xml_list)
        goto out;

    lang_xml_list = check_external_refs (lang_xml_list, lang_xml_names);

    if (!lang_xml_list)
        goto out;

    mgr->dirs_read = TRUE;

    for (l = lang_xml_list; l != NULL; l = l->next)
    {
        LangXML *xml = l->data;
        MooLang *lang = moo_lang_new (mgr, xml->name,
                                      xml->section, xml->version,
                                      xml->author);
        lang->hidden = xml->hidden;
        moo_lang_build_contexts (lang, xml);
    }

    for (l = lang_xml_list; l != NULL; l = l->next)
    {
        LangXML *xml = l->data;
        MooLang *lang = moo_lang_mgr_get_lang (mgr, xml->name);
        g_assert (lang != NULL);
        moo_lang_finish_build (lang, xml);
    }

out:
    g_hash_table_destroy (lang_xml_names);
    g_slist_foreach (lang_xml_list, (GFunc) moo_lang_xml_free, NULL);
    g_slist_free (lang_xml_list);

    moo_lang_mgr_load_styles (mgr);
}


MooContext*
moo_lang_mgr_get_context (MooLangMgr     *mgr,
                          const char     *lang_name,
                          const char     *ctx_name)
{
    MooLang *lang;
    g_return_val_if_fail (MOO_IS_LANG_MGR (mgr), NULL);
    g_return_val_if_fail (lang_name && ctx_name, NULL);
    lang = moo_lang_mgr_get_lang (mgr, lang_name);
    g_return_val_if_fail (lang != NULL, NULL);
    return moo_lang_get_context (lang, ctx_name);
}


MooLang*
moo_lang_mgr_get_lang (MooLangMgr     *mgr,
                       const char     *name)
{
    char *id;
    MooLang *lang;
    g_return_val_if_fail (MOO_IS_LANG_MGR (mgr), NULL);
    g_return_val_if_fail (name != NULL, NULL);
    id = g_ascii_strdown (name, -1);
    lang = g_hash_table_lookup (mgr->lang_names, id);
    g_free (id);
    return lang;
}


GSList*
moo_lang_mgr_get_available_langs (MooLangMgr *mgr)
{
    g_return_val_if_fail (MOO_IS_LANG_MGR (mgr), NULL);
    return g_slist_copy (mgr->langs);
}


static void
maybe_add_section (MooLang *lang,
                   GSList **list)
{
    if (!g_slist_find_custom (*list, lang->section, (GCompareFunc) strcmp))
        *list = g_slist_prepend (*list, g_strdup (lang->section));
}

GSList*
moo_lang_mgr_get_sections (MooLangMgr *mgr)
{
    GSList *list = NULL;
    g_return_val_if_fail (MOO_IS_LANG_MGR (mgr), NULL);
    g_slist_foreach (mgr->langs, (GFunc) maybe_add_section, &list);
    return list;
}


static MooLang *
lang_mgr_get_lang_by_extension (MooLangMgr  *mgr,
                                const char  *filename)
{
    MooLang *lang = NULL;
    char *basename, *utf8_basename;
    GSList *l;
    gboolean found = FALSE;

    g_return_val_if_fail (filename != NULL, NULL);

    /* TODO: is this right? */
    basename = g_path_get_basename (filename);
    g_return_val_if_fail (basename != NULL, NULL);
    utf8_basename = g_filename_display_name (basename);
    g_return_val_if_fail (utf8_basename != NULL, NULL);

    for (l = mgr->langs; !found && l != NULL; l = l->next)
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

    g_free (utf8_basename);
    g_free (basename);
    return lang;
}


static MooLang *
lang_mgr_get_lang_for_bak_filename (MooLangMgr *mgr,
                                    const char *filename)
{
    MooLang *lang = NULL;
    char *base = NULL;
    int len;
    guint i;
    char *utf8_name;

    static const char *bak_globs[] = {"*~", "*.bak", "*.in"};

    utf8_name = g_filename_to_utf8 (filename, -1, NULL, NULL, NULL);
    g_return_val_if_fail (utf8_name != NULL, NULL);

    len = strlen (utf8_name);

    for (i = 0; i < G_N_ELEMENTS (bak_globs); ++i)
    {
        int ext_len = strlen (bak_globs[i]) - 1;

        if (len > ext_len && g_pattern_match_simple (bak_globs[i], utf8_name))
        {
            base = g_strndup (utf8_name, len - ext_len);
            break;
        }
    }

    if (base)
    {
        char *opsys_name = g_filename_from_utf8 (base, len, NULL, NULL, NULL);

        if (opsys_name)
            lang = moo_lang_mgr_get_lang_for_filename (mgr, opsys_name);

        g_free (opsys_name);
        g_free (base);
    }

    g_free (utf8_name);
    return lang;
}


MooLang*
moo_lang_mgr_get_lang_for_file (MooLangMgr     *mgr,
                                const char     *filename)
{
    MooLang *lang = NULL;
    const char *mime_type;

    g_return_val_if_fail (MOO_IS_LANG_MGR (mgr), NULL);
    g_return_val_if_fail (filename != NULL, NULL);

    lang = lang_mgr_get_lang_by_extension (mgr, filename);

    if (lang)
        return lang;

#ifdef MOO_USE_XDGMIME
    /* XXX: xdgmime wants utf8-encoded filename here. is it a problem? */
    /* It's a big problem! */

    mime_type = xdg_mime_get_mime_type_for_file (filename, NULL);

    if (!xdg_mime_mime_type_equal (mime_type, XDG_MIME_TYPE_UNKNOWN))
        lang = moo_lang_mgr_get_lang_for_mime_type (mgr, mime_type);

    if (lang)
        return lang;
#endif /* MOO_USE_XDGMIME */

    lang = lang_mgr_get_lang_for_bak_filename (mgr, filename);

    if (lang)
        return lang;

    return NULL;
}


MooLang*
moo_lang_mgr_get_lang_for_filename (MooLangMgr *mgr,
                                    const char *filename)
{
    MooLang *lang = NULL;
    const char *mime_type;

    g_return_val_if_fail (MOO_IS_LANG_MGR (mgr), NULL);
    g_return_val_if_fail (filename != NULL, NULL);

    lang = lang_mgr_get_lang_by_extension (mgr, filename);

    if (lang)
        return lang;

#ifdef MOO_USE_XDGMIME
    /* XXX: xdgmime wants utf8-encoded filename here. is it a problem? */
    /* It's a big problem! */

    mime_type = xdg_mime_get_mime_type_from_file_name (filename);

    if (!xdg_mime_mime_type_equal (mime_type, XDG_MIME_TYPE_UNKNOWN))
        lang = moo_lang_mgr_get_lang_for_mime_type (mgr, mime_type);

    if (lang)
        return lang;
#endif /* MOO_USE_XDGMIME */

    lang = lang_mgr_get_lang_for_bak_filename (mgr, filename);

    if (lang)
        return lang;

    return NULL;
}


#ifdef MOO_USE_XDGMIME
static int
check_mime_subclass (const char *base,
                     const char *mime)
{
    return !xdg_mime_mime_type_subclass (mime, base);
}


MooLang*
moo_lang_mgr_get_lang_for_mime_type (MooLangMgr *mgr,
                                     const char *mime)
{
    GSList *l;
    MooLang *lang = NULL;
    gboolean found = FALSE;

    g_return_val_if_fail (MOO_IS_LANG_MGR (mgr), NULL);
    g_return_val_if_fail (mime != NULL, NULL);

    for (l = mgr->langs; l != NULL; l = l->next)
    {
        lang = l->data;

        if (g_slist_find_custom (lang->mime_types, mime, (GCompareFunc) strcmp))
        {
            found = TRUE;
            break;
        }
    }

    if (found)
        return lang;

    for (l = mgr->langs; l != NULL; l = l->next)
    {
        lang = l->data;

        if (g_slist_find_custom (lang->mime_types, mime, (GCompareFunc) check_mime_subclass))
        {
            found = TRUE;
            break;
        }
    }

    return found ? lang : NULL;
}
#endif /* MOO_USE_XDGMIME */


static MooTextStyle*
moo_lang_get_style (MooLang     *lang,
                    const char  *style_name)
{
    MooTextStyle *style;
    g_return_val_if_fail (lang != NULL && style_name != NULL, NULL);
    style = g_hash_table_lookup (lang->style_names, style_name);
    return moo_text_style_copy (style);
}


MooTextStyle*
moo_lang_mgr_get_style (MooLangMgr     *mgr,
                        const char     *lang_name,
                        const char     *style_name,
                        MooTextStyleScheme *scheme)
{
    const MooTextStyle *scheme_style = NULL;
    MooTextStyle *lang_style = NULL;

    g_return_val_if_fail (MOO_IS_LANG_MGR (mgr), NULL);
    g_return_val_if_fail (style_name != NULL, NULL);
    g_return_val_if_fail (mgr->active_scheme != NULL, NULL);

    if (!scheme)
        scheme = mgr->active_scheme;

    if (lang_name)
    {
        MooLang *lang = moo_lang_mgr_get_lang (mgr, lang_name);
        g_return_val_if_fail (lang != NULL, NULL);
        lang_style = moo_lang_get_style (lang, style_name);
    }

    scheme_style = moo_text_style_scheme_get (scheme, lang_name, style_name);

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
moo_lang_mgr_get_active_scheme (MooLangMgr *mgr)
{
    g_return_val_if_fail (MOO_IS_LANG_MGR (mgr), NULL);
    g_return_val_if_fail (mgr->active_scheme != NULL, NULL);
    return mgr->active_scheme;
}


void
_moo_lang_mgr_add_lang (MooLangMgr         *mgr,
                        MooLang            *lang)
{
    g_return_if_fail (MOO_IS_LANG_MGR (mgr));
    g_return_if_fail (lang != NULL);
    g_return_if_fail (lang->id != NULL);
    g_return_if_fail (!g_hash_table_lookup (mgr->lang_names, lang->id));

    mgr->langs = g_slist_append (mgr->langs, lang);
    g_hash_table_insert (mgr->lang_names, g_strdup (lang->id), lang);
}


static void
prepend_scheme (G_GNUC_UNUSED const char *scheme_name,
                gpointer    scheme,
                GSList    **list)
{
    *list = g_slist_prepend (*list, scheme);
}

GSList*
moo_lang_mgr_list_schemes (MooLangMgr         *mgr)
{
    GSList *list = NULL;
    g_return_val_if_fail (MOO_IS_LANG_MGR (mgr), NULL);
    g_hash_table_foreach (mgr->schemes, (GHFunc) prepend_scheme, &list);
    return list;
}
