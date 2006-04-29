/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   moolang-parser.c
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

#define MOOEDIT_COMPILATION
#include "mooedit/moolang-parser.h"
#include "mooedit/moolang-strings.h"
#include "mooedit/moolangmgr.h"
#include "mooutils/moomarkup.h"
#include <string.h>
#ifdef MOO_USE_XML
#include <libxml/tree.h>
#include <libxml/parser.h>
#endif


#ifdef MOO_USE_XML
static LangXML      *lang_xml_parse         (xmlNode        *node);
static GeneralXML   *general_xml_parse      (xmlNode        *node);
static SyntaxXML    *syntax_xml_parse       (LangXML        *lang_xml,
                                             xmlNode        *node);
static KeywordXML   *keyword_xml_parse      (xmlNode        *node);
static ContextXML   *context_xml_parse      (LangXML        *lang_xml,
                                             xmlNode        *node);
static StyleListXML *style_list_xml_parse   (LangXML        *lang_xml,
                                             xmlNode        *node);
static StyleXML     *style_xml_parse        (xmlNode        *node);
static RuleXML      *rule_xml_parse         (LangXML        *xml,
                                             xmlNode        *node);

static void          general_xml_free       (GeneralXML     *xml);
static void          syntax_xml_free        (SyntaxXML      *xml);
static void          keyword_xml_free       (KeywordXML     *xml);
static void          context_xml_free       (ContextXML     *xml);
static void          style_xml_free         (StyleXML       *xml);
static void          style_list_xml_free    (StyleListXML   *xml);
static void          rule_xml_free          (RuleXML        *xml);

static CrossRef     *cross_ref_new          (const char     *lang,
                                             const char     *name);
static void          cross_ref_free         (CrossRef       *ref);
static void          ctx_switch_info_destroy(CtxSwitchInfo  *switch_info);

static void          lang_xml_add_cross_ref     (LangXML        *xml,
                                                 const char     *lang,
                                                 const char     *name);
static void          lang_xml_add_keyword_ref   (LangXML        *xml,
                                                 const xmlChar  *keyword);
static void          lang_xml_add_style_ref     (LangXML        *xml,
                                                 const char     *style);
static gboolean      lang_xml_check_internal_refs (LangXML      *xml);
#endif /* MOO_USE_XML */



LangXML*
_moo_lang_parse_file (const char *file)
{
#ifdef MOO_USE_XML
    xmlDoc *doc;
    xmlNode *root;
    LangXML *xml;

    g_return_val_if_fail (file != NULL, NULL);

    doc = xmlReadFile (file, "UTF8",
                       XML_PARSE_NOENT | XML_PARSE_NOBLANKS |
                               XML_PARSE_NONET);
    g_return_val_if_fail (doc != NULL, NULL);

    root = xmlDocGetRootElement (doc);
    xml = lang_xml_parse (root);

    xmlFreeDoc (doc);
    xmlCleanupParser ();

    return xml;
#else
    g_return_val_if_fail (file != NULL, NULL);
    return NULL;
#endif
}


LangXML*
_moo_lang_parse_memory (const char *buffer,
                        int         size)
{
#ifdef MOO_USE_XML
    xmlDoc *doc;
    xmlNode *root;
    LangXML *xml;

    g_return_val_if_fail (buffer != NULL, NULL);

    if (size < 0)
        size = strlen (buffer);

    doc = xmlReadMemory (buffer, size, NULL, "UTF8",
                         XML_PARSE_NOENT | XML_PARSE_NOBLANKS);
    g_return_val_if_fail (doc != NULL, NULL);

    root = xmlDocGetRootElement (doc);
    xml = lang_xml_parse (root);

    xmlFreeDoc (doc);
    xmlCleanupParser ();

    return xml;
#else
    g_return_val_if_fail (buffer != NULL, NULL);
    g_return_val_if_fail (size != 0, NULL);
    return NULL;
#endif
}


#ifndef MOO_USE_XML

/* Stubs for functions used elsewhere */
void
_moo_lang_xml_free (G_GNUC_UNUSED LangXML *xml)
{
    g_return_if_reached ();
}

MooRule*
_moo_rule_new_from_xml (G_GNUC_UNUSED RuleXML    *xml,
                        G_GNUC_UNUSED LangXML    *lang_xml,
                        G_GNUC_UNUSED MooLang    *lang)
{
    g_return_val_if_reached (NULL);
}

#else /* MOO_USE_XML */

/****************************************************************************/
/* Auxiliary stuff
 */

#define ELM_FOREACH(node__,var__)                                       \
G_STMT_START {                                                          \
    xmlNode *var__;                                                     \
    for (var__ = node__->children; var__ != NULL; var__ = var__->next)    \
        if (var__->type == XML_ELEMENT_NODE)                            \

#define ELM_FOREACH_END                                                 \
} G_STMT_END


#if 0
#define GET_PROP(node__,prop__) GET_PROP (node__, (xmlChar*) prop__)
#define STRING_FREE(s__) if (s__) xmlFree (s__)
#define STRDUP(s__) g_strdup ((char*) s__)
#else
inline static xmlChar *GET_PROP (xmlNode *node, const char *prop_name)
{
    return xmlGetProp (node, (xmlChar*) prop_name);
}

inline static char *STRDUP (const xmlChar *s)
{
    return g_strdup ((char*) s);
}

inline static void STRING_FREE (xmlChar *s)
{
    if (s) xmlFree (s);
}
#endif


#define IS_ELEMENT_NODE(node_)      (node_ != NULL && node_->type == XML_ELEMENT_NODE)
#define NODE_NAME_IS_(node_, name_) (node_->name && !strcmp ((char*)node_->name, name_))

#define IS_LANGUAGE_NODE(node_)         (IS_ELEMENT_NODE(node_) && NODE_NAME_IS_(node_, LANGUAGE_ELM))
#define IS_KEYWORD_LIST_NODE(node_)     (IS_ELEMENT_NODE(node_) && NODE_NAME_IS_(node_, KEYWORD_LIST_ELM))
#define IS_KEYWORD_NODE(node_)          (IS_ELEMENT_NODE(node_) && NODE_NAME_IS_(node_, KEYWORD_ELM))
#define IS_CONTEXT_NODE(node_)          (IS_ELEMENT_NODE(node_) && NODE_NAME_IS_(node_, CONTEXT_ELM))
#define IS_SYNTAX_NODE(node_)           (IS_ELEMENT_NODE(node_) && NODE_NAME_IS_(node_, SYNTAX_ELM))
#define IS_STYLE_NODE(node_)            (IS_ELEMENT_NODE(node_) && NODE_NAME_IS_(node_, STYLE_ELM))
#define IS_STYLE_LIST_NODE(node_)       (IS_ELEMENT_NODE(node_) && NODE_NAME_IS_(node_, STYLE_LIST_ELM))
#define IS_GENERAL_NODE(node_)          (IS_ELEMENT_NODE(node_) && NODE_NAME_IS_(node_, GENERAL_ELM))
#define IS_BRACKETS_NODE(node_)         (IS_ELEMENT_NODE(node_) && NODE_NAME_IS_(node_, BRACKETS_ELM))
#define IS_COMMENTS_NODE(node_)         (IS_ELEMENT_NODE(node_) && NODE_NAME_IS_(node_, COMMENTS_ELM))
#define IS_SINGLE_LINE_NODE(node_)      (IS_ELEMENT_NODE(node_) && NODE_NAME_IS_(node_, SINGLE_LINE_ELM))
#define IS_MULTI_LINE_NODE(node_)       (IS_ELEMENT_NODE(node_) && NODE_NAME_IS_(node_, MULTI_LINE_ELM))
#define IS_SCHEME_NODE(node_)           (IS_ELEMENT_NODE(node_) && NODE_NAME_IS_(node_, SCHEME_ELM))
#define IS_DEFAULT_STYLE_NODE(node_)    (IS_ELEMENT_NODE(node_) && NODE_NAME_IS_(node_, DEFAULT_STYLE_ELM))
#define IS_BRACKET_MATCH_NODE(node_)    (IS_ELEMENT_NODE(node_) && NODE_NAME_IS_(node_, BRACKET_MATCH_ELM))
#define IS_BRACKET_MISMATCH_NODE(node_) (IS_ELEMENT_NODE(node_) && NODE_NAME_IS_(node_, BRACKET_MISMATCH_ELM))
#define IS_SAMPLE_CODE_NODE(node_)      (IS_ELEMENT_NODE(node_) && NODE_NAME_IS_(node_, SAMPLE_CODE_ELM))


#if VERBOSE
#define DEBUG_PRINT(x) (x)
#else
#define DEBUG_PRINT(x)
#endif


static gboolean
parse_bool (const xmlChar *string,
            gboolean       default_val)
{
    if (!string)
        return default_val;

    if (!g_ascii_strcasecmp ((const char*) string, "true") ||
         !strcmp ((const char*) string, "1") ||
         !g_ascii_strcasecmp ((const char*) string, "yes"))
    {
        return TRUE;
    }
    else if (!g_ascii_strcasecmp ((const char*) string, "false") ||
              !strcmp ((const char*) string, "0") ||
              !g_ascii_strcasecmp ((const char*) string, "no"))
    {
        return FALSE;
    }
    else
    {
        g_warning ("%s: invalid string '%s' for boolean value",
                   G_STRLOC, string);
        return default_val;
    }
}


static gboolean
prop_equal (const xmlChar *str1, const char *str2)
{
    while (*str1 && *str2)
    {
        if (*str1 == '-' || *str1 == '_')
        {
            if (*str2 != '-' && *str2 != '_')
                return FALSE;
        }
        else if (*str1 != *str2)
        {
            return FALSE;
        }

        str1++;
        str2++;
    }

    return !*str1 && !*str2;
}


static CrossRef*
cross_ref_new (const char     *lang,
               const char     *name)
{
    CrossRef *ref;

    g_return_val_if_fail (name != NULL, NULL);

    ref = g_new (CrossRef, 1);
    ref->lang = g_strdup (lang);
    ref->name = g_strdup (name);

    return ref;
}


static void
cross_ref_free (CrossRef       *ref)
{
    if (ref)
    {
        g_free (ref->name);
        g_free (ref->lang);
        g_free (ref);
    }
}


static void
ctx_switch_info_destroy (CtxSwitchInfo *switch_info)
{
    g_return_if_fail (switch_info != NULL);

    switch (switch_info->type)
    {
        case MOO_CONTEXT_SWITCH:
#if 0
        case MOO_CONTEXT_JUMP:
#endif
            g_free (switch_info->ref.name);
            g_free (switch_info->ref.lang);
            break;
        case MOO_CONTEXT_STAY:
        case MOO_CONTEXT_POP:
            break;
    }
}


#define LANG_REF_SEP     "##"
#define LANG_REF_SEP_LEN 2

static gboolean
parse_name_ref (const xmlChar *string,
                char         **lang_p,
                char         **name_p)
{
    const char *s;
    char *lang, *name;

    g_return_val_if_fail (string && string[0], FALSE);

    if (strncmp ((char*) string, LANG_REF_SEP, LANG_REF_SEP_LEN))
    {
        if (lang_p)
            *lang_p = NULL;
        if (name_p)
            *name_p = STRDUP (string);
        return TRUE;
    }

    if (!string[LANG_REF_SEP_LEN])
        return FALSE;

    s = strstr ((char*) string + LANG_REF_SEP_LEN, LANG_REF_SEP);

    if (s == (char*) string + LANG_REF_SEP_LEN)
        return FALSE;

    if (s && !s[LANG_REF_SEP_LEN])
        return FALSE;

    if (!s)
        return FALSE;

    lang = g_strndup ((char*) string + LANG_REF_SEP_LEN, (s - (char*) string) - LANG_REF_SEP_LEN);
    name = g_strdup (s + LANG_REF_SEP_LEN);

    if (lang_p)
        *lang_p = lang;
    else
        g_free (lang);

    if (name_p)
        *name_p = name;
    else
        g_free (name);

    return TRUE;
}


static gboolean
parse_context_ref (const xmlChar *string,
                   CtxSwitchInfo *switch_info)
{
    g_return_val_if_fail (switch_info != NULL, FALSE);
    g_return_val_if_fail (!string || string[0], FALSE);

    if (!string || !strcmp ((char*) string, CONTEXT_STAY))
    {
        switch_info->type = MOO_CONTEXT_STAY;
        switch_info->num = 0;
    }
    else if (!strncmp ((char*) string, CONTEXT_POP, strlen (CONTEXT_POP)))
    {
        guint n;

        for (n = 1, string += strlen (CONTEXT_POP);
             *string; ++n, string += strlen (CONTEXT_POP))
        {
            if (strncmp ((char*) string, CONTEXT_POP, strlen (CONTEXT_POP)))
                return FALSE;
        }

        switch_info->type = MOO_CONTEXT_POP;
        switch_info->num = n;
    }
    else
    {
        char *name = NULL, *lang = NULL;

        if (!parse_name_ref (string, &lang, &name))
            return FALSE;

        switch_info->type = MOO_CONTEXT_SWITCH;
        switch_info->ref.lang = lang;
        switch_info->ref.name = name;
    }

    return TRUE;
}


/****************************************************************************/
/* language node
 */

static LangXML*
lang_xml_parse (xmlNode *node)
{
    LangXML *xml = NULL;
    xmlChar *name = NULL, *version = NULL, *author = NULL, *section = NULL;
    xmlChar *mimetypes = NULL, *extensions = NULL, *hidden = NULL;

    g_return_val_if_fail (IS_LANGUAGE_NODE (node), NULL);

    name = GET_PROP (node, LANG_NAME_PROP);
    version = GET_PROP (node, LANG_VERSION_PROP);
    section = GET_PROP (node, LANG_SECTION_PROP);
    mimetypes = GET_PROP (node, LANG_MIME_TYPES_PROP);
    extensions = GET_PROP (node, LANG_EXTENSIONS_PROP);
    author = GET_PROP (node, LANG_AUTHOR_PROP);
    hidden = GET_PROP (node, LANG_HIDDEN_PROP);

    if (!name || !name[0])
    {
        g_warning ("%s: no name in language node", G_STRLOC);
        goto error;
    }

    xml = g_new0 (LangXML, 1);
    xml->name = STRDUP (name);
    xml->section = STRDUP (section);
    xml->version = STRDUP (version);
    xml->mimetypes = STRDUP (mimetypes);
    xml->extensions = STRDUP (extensions);
    xml->author = STRDUP (author);
    xml->hidden = parse_bool (hidden, FALSE);

    xml->context_names = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
    xml->style_names = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
    xml->keyword_names = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
    xml->style_refs = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

    DEBUG_PRINT ({
        g_print ("language: %s\n", name);
        if (section) g_print ("  section: %s\n", section);
        if (version) g_print ("  version: %s\n", version);
        if (author) g_print ("  author: %s\n", author);
        if (mimetypes) g_print ("  mimetypes: %s\n", mimetypes);
        if (extensions) g_print ("  extensions: %s\n", extensions);
        g_print ("\n");
    });

    ELM_FOREACH (node, child)
    {
        if (IS_SYNTAX_NODE (child))
        {
            if (xml->syntax)
            {
                g_warning ("%s: duplicated '" SYNTAX_ELM "' tag", G_STRLOC);
                goto error;
            }

            xml->syntax = syntax_xml_parse (xml, child);

            if (!xml->syntax)
                goto error;
        }
        else if (IS_STYLE_LIST_NODE (child))
        {
            if (xml->style_list)
            {
                g_warning ("%s: duplicated '" STYLE_LIST_ELM "' tag", G_STRLOC);
                goto error;
            }

            xml->style_list = style_list_xml_parse (xml, child);

            if (!xml->style_list)
                goto error;
        }
        else if (IS_GENERAL_NODE (child))
        {
            if (xml->general)
            {
                g_warning ("%s: duplicated '" GENERAL_ELM "' tag", G_STRLOC);
                goto error;
            }

            xml->general = general_xml_parse (child);

            if (!xml->general)
                goto error;
        }
        else if (IS_SAMPLE_CODE_NODE (child))
        {
            xmlChar *content;

            if (xml->sample)
            {
                g_warning ("%s: duplicated '" SAMPLE_CODE_ELM "' element", G_STRLOC);
                goto error;
            }

            content = xmlNodeGetContent (child);
            xml->sample = STRDUP (content);

            STRING_FREE (content);
        }
        else
        {
            g_warning ("%s: unknown tag '%s'", G_STRLOC, child->name);
            goto error;
        }
    }
    ELM_FOREACH_END;

    if (!xml->syntax)
    {
        g_warning ("%s: no contexts defined in lang '%s'", G_STRLOC, xml->name);
        goto error;
    }

    if (!lang_xml_check_internal_refs (xml))
        goto error;

    STRING_FREE (name);
    STRING_FREE (version);
    STRING_FREE (section);
    STRING_FREE (mimetypes);
    STRING_FREE (extensions);
    STRING_FREE (author);
    STRING_FREE (hidden);
    return xml;

error:
    STRING_FREE (name);
    STRING_FREE (version);
    STRING_FREE (section);
    STRING_FREE (mimetypes);
    STRING_FREE (extensions);
    STRING_FREE (author);
    STRING_FREE (hidden);
    _moo_lang_xml_free (xml);
    return NULL;
}


void
_moo_lang_xml_free (LangXML *xml)
{
    if (xml)
    {
        g_free (xml->name);
        g_free (xml->section);
        g_free (xml->version);
        g_free (xml->mimetypes);
        g_free (xml->extensions);
        g_free (xml->author);
        g_free (xml->sample);
        general_xml_free (xml->general);
        syntax_xml_free (xml->syntax);
        style_list_xml_free (xml->style_list);

        if (xml->context_names)
            g_hash_table_destroy (xml->context_names);
        if (xml->style_names)
            g_hash_table_destroy (xml->style_names);
        if (xml->keyword_names)
            g_hash_table_destroy (xml->keyword_names);

        g_slist_foreach (xml->internal_refs, (GFunc) cross_ref_free, NULL);
        g_slist_free (xml->internal_refs);
        g_slist_foreach (xml->external_refs, (GFunc) cross_ref_free, NULL);
        g_slist_free (xml->external_refs);

        g_slist_foreach (xml->keyword_refs, (GFunc) g_free, NULL);
        g_slist_free (xml->keyword_refs);
        g_hash_table_destroy (xml->style_refs);

        g_free (xml);
    }
}


static void
lang_xml_add_cross_ref (LangXML        *xml,
                        const char     *lang,
                        const char     *name)
{
    CrossRef *ref;

    g_return_if_fail (xml != NULL && name != NULL);
    g_return_if_fail (!lang || lang[0]);

    if (!lang || !strcmp (lang, xml->name))
    {
        ref = cross_ref_new (NULL, name);
        xml->internal_refs = g_slist_prepend (xml->internal_refs, ref);
    }
    else
    {
        ref = cross_ref_new (lang, name);
        xml->external_refs = g_slist_prepend (xml->external_refs, ref);
    }
}


static void
lang_xml_add_keyword_ref (LangXML        *xml,
                          const xmlChar  *keyword)
{
    g_return_if_fail (xml != NULL && keyword != NULL);
    xml->keyword_refs = g_slist_prepend (xml->keyword_refs, STRDUP (keyword));
}


static void
lang_xml_add_style_ref (LangXML        *xml,
                        const char     *style)
{
    if (style)
        g_hash_table_insert (xml->style_refs, g_strdup (style), NULL);
}


static void
check_style (const char *style_name,
             G_GNUC_UNUSED gpointer ptr,
             gpointer    user_data)
{
    struct {
        LangXML *xml;
        gboolean valid;
    } *data = user_data;

    if (!g_hash_table_lookup (data->xml->style_names, style_name))
    {
        g_warning ("%s: invalid style '%s'", G_STRLOC, style_name);
        data->valid = FALSE;
    }
}

static gboolean
lang_xml_check_internal_refs (LangXML *xml)
{
    GSList *l;
    struct {
        LangXML *xml;
        gboolean valid;
    } data;

    for (l = xml->internal_refs; l != NULL; l = l->next)
    {
        CrossRef *ref = l->data;

        g_assert (ref->lang == NULL);

        if (!g_hash_table_lookup (xml->context_names, ref->name))
        {
            g_warning ("%s: invalid context '%s'", G_STRLOC, ref->name);
            return FALSE;
        }
    }

    for (l = xml->keyword_refs; l != NULL; l = l->next)
    {
        char *kw = l->data;

        if (!g_hash_table_lookup (xml->keyword_names, kw))
        {
            g_warning ("%s: invalid keyword list '%s'", G_STRLOC, kw);
            return FALSE;
        }
    }

    data.xml = xml;
    data.valid = TRUE;
    g_hash_table_foreach (xml->style_refs, (GHFunc) check_style, &data);

    if (!data.valid)
        return FALSE;

    return TRUE;
}


/****************************************************************************/
/* general node
 */

static GeneralXML*
general_xml_parse (xmlNode *node)
{
    GeneralXML *xml = NULL;

    g_assert (IS_GENERAL_NODE (node));

    xml = g_new0 (GeneralXML, 1);

    g_assert (IS_GENERAL_NODE (node));

    ELM_FOREACH (node, child)
    {
        if (IS_BRACKETS_NODE (child))
        {
            xmlChar *content;

            if (xml->brackets)
            {
                g_warning ("%s: duplicated '" BRACKETS_ELM "' element", G_STRLOC);
                goto error;
            }

            content = xmlNodeGetContent (node);
            xml->brackets = STRDUP (content);

            STRING_FREE (content);
        }
        else if (IS_COMMENTS_NODE (child))
        {
            ELM_FOREACH (child, comm_node)
            {
                if (IS_SINGLE_LINE_NODE (comm_node))
                {
                    xmlChar *start;

                    if (xml->single_line_start)
                    {
                        g_warning ("%s: duplicated '" SINGLE_LINE_ELM "' element", G_STRLOC);
                        goto error;
                    }

                    start = GET_PROP (comm_node, COMMENT_START_PROP);

                    if (!start)
                    {
                        g_warning ("%s: '" COMMENT_START_PROP "' attribute missing in '"
                                   SINGLE_LINE_ELM "' element", G_STRLOC);
                        goto error;
                    }

                    xml->single_line_start = STRDUP (start);
                    STRING_FREE (start);
                }
                else if (IS_MULTI_LINE_NODE (comm_node))
                {
                    xmlChar *start = NULL;
                    xmlChar *end = NULL;

                    if (xml->multi_line_start)
                    {
                        g_warning ("%s: duplicated '" MULTI_LINE_ELM "' element", G_STRLOC);
                        goto error;
                    }

                    start = GET_PROP (comm_node, COMMENT_START_PROP);
                    end = GET_PROP (comm_node, COMMENT_END_PROP);

                    if (!start)
                    {
                        g_warning ("%s: '" COMMENT_START_PROP "' attribute missing in '"
                                   MULTI_LINE_ELM "' element", G_STRLOC);
                        STRING_FREE (end);
                        goto error;
                    }
                    else if (!end)
                    {
                        g_warning ("%s: '" COMMENT_END_PROP "' attribute missing in '"
                                   MULTI_LINE_ELM "' element", G_STRLOC);
                        STRING_FREE (start);
                        goto error;
                    }

                    xml->multi_line_start = STRDUP (start);
                    xml->multi_line_end = STRDUP (end);
                    STRING_FREE (start);
                    STRING_FREE (end);
                }
                else
                {
                    g_warning ("%s: invalid node '%s' in '" COMMENTS_ELM "' node",
                               G_STRLOC, comm_node->name);
                    goto error;
                }
            }
            ELM_FOREACH_END;
        }
        else
        {
            g_warning ("%s: invalid node '%s' in '" GENERAL_ELM "' node",
                       G_STRLOC, child->name);
            goto error;
        }
    }
    ELM_FOREACH_END;

    return xml;

error:
    general_xml_free (xml);
    return NULL;
}


static void
general_xml_free (GeneralXML *xml)
{
    if (xml)
    {
        g_free (xml->brackets);
        g_free (xml->single_line_start);
        g_free (xml->multi_line_start);
        g_free (xml->multi_line_end);
        g_free (xml);
    }
}


/****************************************************************************/
/* syntax node
 */

static SyntaxXML*
syntax_xml_parse (LangXML *lang_xml,
                  xmlNode *node)
{
    SyntaxXML *xml = NULL;

    g_assert (IS_SYNTAX_NODE (node));

    xml = g_new0 (SyntaxXML, 1);

    DEBUG_PRINT ({
        g_print ("syntax\n");
        g_print ("\n");
    });

    ELM_FOREACH (node, child)
    {
        if (IS_KEYWORD_LIST_NODE (child))
        {
            KeywordXML *kw = keyword_xml_parse (child);

            if (!kw)
                goto error;

            if (g_hash_table_lookup (lang_xml->keyword_names, kw->name))
            {
                g_warning ("%s: duplicated keyword list '%s'",
                           G_STRLOC, kw->name);
                keyword_xml_free (kw);
                goto error;
            }

            g_hash_table_insert (lang_xml->keyword_names,
                                 g_strdup (kw->name), kw);
            xml->keywords = g_slist_prepend (xml->keywords, kw);
            xml->n_keywords++;
        }
        else if (IS_CONTEXT_NODE (child))
        {
            ContextXML *ctx = context_xml_parse (lang_xml, child);

            if (!ctx)
                goto error;

            if (g_hash_table_lookup (lang_xml->context_names, ctx->name))
            {
                g_warning ("%s: duplicated context '%s'",
                           G_STRLOC, ctx->name);
                context_xml_free (ctx);
                goto error;
            }

            g_hash_table_insert (lang_xml->context_names,
                                 g_strdup (ctx->name), ctx);
            xml->contexts = g_slist_prepend (xml->contexts, ctx);
            xml->n_contexts++;
        }
        else
        {
            g_warning ("%s: invalid tag '%s' inside of '" SYNTAX_ELM "' tag",
                       G_STRLOC, child->name);
            goto error;
        }
    }
    ELM_FOREACH_END;

    if (!xml->contexts)
    {
        g_warning ("%s: no contexts defined in lang '%s'",
                   G_STRLOC, lang_xml->name);
    }

    xml->contexts = g_slist_reverse (xml->contexts);
    xml->keywords = g_slist_reverse (xml->keywords);
    return xml;

error:
    syntax_xml_free (xml);
    return NULL;
}


static void
syntax_xml_free (SyntaxXML *xml)
{
    if (xml)
    {
        g_slist_foreach (xml->keywords, (GFunc) keyword_xml_free, NULL);
        g_slist_foreach (xml->contexts, (GFunc) context_xml_free, NULL);
        g_slist_free (xml->keywords);
        g_slist_free (xml->contexts);
        g_free (xml);
    }
}


/****************************************************************************/
/* keyword-list node
 */

static KeywordXML*
keyword_xml_parse (xmlNode *node)
{
    KeywordXML *xml = NULL;
    xmlChar *name = NULL, *word_boundary = NULL, *prefix = NULL, *suffix = NULL;

    g_assert (IS_KEYWORD_LIST_NODE (node));

    name = GET_PROP (node, KEYWORD_NAME_PROP);
    word_boundary = GET_PROP (node, KEYWORD_WBNDRY_PROP);
    prefix = GET_PROP (node, KEYWORD_PREFIX_PROP);
    suffix = GET_PROP (node, KEYWORD_SUFFIX_PROP);

    if (!name || !name[0])
    {
        g_warning ("%s: no " KEYWORD_NAME_PROP " attribute in "
                   KEYWORD_LIST_ELM " node", G_STRLOC);
        goto error;
    }

    xml = g_new0 (KeywordXML, 1);
    xml->name = STRDUP (name);
    xml->word_boundary = parse_bool (word_boundary, TRUE);
    xml->prefix = STRDUP (prefix);
    xml->suffix = STRDUP (suffix);

    DEBUG_PRINT ({
        g_print ("keyword-list: %s\n", name);
        g_print ("  word-boundary: %s\n", xml->word_boundary ? "TRUE" : "FALSE");
        g_print ("  prefix: %s\n", xml->prefix ? xml->prefix : "<NULL>");
        g_print ("  suffix: %s\n", xml->suffix ? xml->suffix : "<NULL>");
        g_print ("\n");
    });

    ELM_FOREACH (node, child)
    {
        if (IS_KEYWORD_NODE (child))
        {
            xmlChar *content = xmlNodeGetContent (child);

            if (!content || !content[0])
            {
                g_warning ("%s: empty " KEYWORD_ELM " tag", G_STRLOC);
                STRING_FREE (content);
                goto error;
            }

            DEBUG_PRINT ({
                g_print ("  %s\n", content);
            });

            xml->words = g_slist_prepend (xml->words, STRDUP (content));
            xml->n_words++;
            STRING_FREE (content);
        }
        else
        {
            g_warning ("%s: invalid tag '%s' inside of " KEYWORD_LIST_ELM " tag",
                       G_STRLOC, child->name);
            goto error;
        }
    }
    ELM_FOREACH_END;

    DEBUG_PRINT ({
        g_print ("\n");
    });

    xml->words = g_slist_reverse (xml->words);

    STRING_FREE (name);
    STRING_FREE (prefix);
    STRING_FREE (suffix);
    STRING_FREE (word_boundary);
    return xml;

error:
    STRING_FREE (name);
    STRING_FREE (prefix);
    STRING_FREE (suffix);
    STRING_FREE (word_boundary);
    keyword_xml_free (xml);
    return NULL;
}


static void
keyword_xml_free (KeywordXML *xml)
{
    if (xml)
    {
        g_slist_foreach (xml->words, (GFunc) g_free, NULL);
        g_slist_free (xml->words);
        g_free (xml->name);
        g_free (xml->suffix);
        g_free (xml->prefix);
        g_free (xml);
    }
}


/****************************************************************************/
/* context node
 */

static ContextXML*
context_xml_parse (LangXML *lang_xml,
                   xmlNode *node)
{
    ContextXML *xml = NULL;
    xmlChar *name = NULL, *eol_context = NULL, *style = NULL;

    g_assert (IS_CONTEXT_NODE (node));

    name = GET_PROP (node, CONTEXT_NAME_PROP);
    eol_context = GET_PROP (node, CONTEXT_EOL_CTX_PROP);
    style = GET_PROP (node, CONTEXT_STYLE_PROP);

    if (!name || !name[0])
    {
        g_warning ("%s: no name attribute in " CONTEXT_ELM " node", G_STRLOC);
        goto error;
    }

    xml = g_new0 (ContextXML, 1);
    xml->name = STRDUP (name);
    xml->eol_context = STRDUP (eol_context);
    xml->style = STRDUP (style);

    if (!parse_context_ref (eol_context, &xml->eol_switch_info))
    {
        g_warning ("%s: invalid '" CONTEXT_EOL_CTX_PROP "' '%s' in " CONTEXT_ELM " node",
                   G_STRLOC, xml->eol_context ? xml->eol_context : "(null)");
        goto error;
    }

    switch (xml->eol_switch_info.type)
    {
        case MOO_CONTEXT_SWITCH:
#if 0
        case MOO_CONTEXT_JUMP:
#endif
            lang_xml_add_cross_ref (lang_xml,
                                    xml->eol_switch_info.ref.lang,
                                    xml->eol_switch_info.ref.name);
            break;
        case MOO_CONTEXT_STAY:
        case MOO_CONTEXT_POP:
            break;
    }

    DEBUG_PRINT ({
        g_print ("context: %s\n", name);
        if (eol_context) g_print ("  eol_context: %s\n", eol_context);
        if (style) g_print ("  style: %s\n", style);
        g_print ("\n");
    });

    ELM_FOREACH (node, child)
    {
        RuleXML *rule = rule_xml_parse (lang_xml, child);

        if (!rule)
            goto error;

        xml->rules = g_slist_prepend (xml->rules, rule);
        xml->n_rules++;
    }
    ELM_FOREACH_END;

    xml->rules = g_slist_reverse (xml->rules);
    lang_xml_add_style_ref (lang_xml, xml->style);

    STRING_FREE (name);
    STRING_FREE (eol_context);
    STRING_FREE (style);
    return xml;

error:
    STRING_FREE (name);
    STRING_FREE (eol_context);
    STRING_FREE (style);
    context_xml_free (xml);
    return NULL;
}


static void
context_xml_free (ContextXML *xml)
{
    if (xml)
    {
        g_free (xml->name);
        g_free (xml->eol_context);
        g_free (xml->style);
        g_slist_foreach (xml->rules, (GFunc) rule_xml_free, NULL);
        g_slist_free (xml->rules);
        g_free (xml);
    }
}


/****************************************************************************/
/* Styles nodes
 */

static StyleListXML*
style_list_xml_parse (LangXML *lang_xml,
                      xmlNode *node)
{
    StyleListXML *xml = NULL;

    g_assert (IS_STYLE_LIST_NODE (node));

    xml = g_new0 (StyleListXML, 1);

    ELM_FOREACH (node, child)
    {
        if (IS_STYLE_NODE (child))
        {
            StyleXML *style = style_xml_parse (child);

            if (!style)
                goto error;

            if (g_hash_table_lookup (lang_xml->style_names, style->name))
            {
                g_warning ("%s: duplicated style '%s'", G_STRLOC, style->name);
                style_xml_free (style);
                goto error;
            }

            g_hash_table_insert (lang_xml->style_names,
                                 g_strdup (style->name), style);
            xml->styles = g_slist_prepend (xml->styles, style);
            xml->n_styles++;
        }
        else
        {
            g_warning ("%s: unknown tag '%s'", G_STRLOC, child->name);
            goto error;
        }
    }
    ELM_FOREACH_END;

    xml->styles = g_slist_reverse (xml->styles);
    return xml;

error:
    style_list_xml_free (xml);
    return NULL;
}


static StyleXML*
style_xml_parse (xmlNode *node)
{
    StyleXML *xml = NULL;
    xmlChar *name = NULL, *default_style = NULL;
    xmlChar *bold = NULL, *italic = NULL, *underline = NULL, *strikethrough = NULL;
    xmlChar *foreground = NULL, *background = NULL;

    g_assert (IS_STYLE_NODE (node));

    name = GET_PROP (node, STYLE_NAME_PROP);
    default_style = GET_PROP (node, STYLE_DEF_STYLE_PROP);
    bold = GET_PROP (node, STYLE_BOLD_PROP);
    italic = GET_PROP (node, STYLE_ITALIC_PROP);
    underline = GET_PROP (node, STYLE_UNDERLINE_PROP);
    strikethrough = GET_PROP (node, STYLE_STRIKETHROUGH_PROP);
    foreground = GET_PROP (node, STYLE_FOREGROUND_PROP);
    background = GET_PROP (node, STYLE_BACKGROUND_PROP);

    if (!name || !name[0])
    {
        g_warning ("%s: no name in style node", G_STRLOC);
        goto error;
    }

    xml = g_new0 (StyleXML, 1);
    xml->name = STRDUP (name);
    xml->default_style = STRDUP (default_style);
    xml->bold = STRDUP (bold);
    xml->italic = STRDUP (italic);
    xml->underline = STRDUP (underline);
    xml->strikethrough = STRDUP (strikethrough);
    xml->foreground = STRDUP (foreground);
    xml->background = STRDUP (background);

    DEBUG_PRINT ({
        g_print ("style: %s\n", name);
        if (default_style) g_print ("  default_style: %s\n", default_style);
        if (bold) g_print ("  bold: %s\n", bold);
        if (italic) g_print ("  italic: %s\n", italic);
        if (underline) g_print ("  underline: %s\n", underline);
        if (strikethrough) g_print ("  strikethrough: %s\n", strikethrough);
        if (foreground) g_print ("  foreground: %s\n", foreground);
        if (background) g_print ("  background: %s\n", background);
    });

    STRING_FREE (name);
    STRING_FREE (default_style);
    STRING_FREE (bold);
    STRING_FREE (italic);
    STRING_FREE (underline);
    STRING_FREE (strikethrough);
    STRING_FREE (foreground);
    STRING_FREE (background);

    return xml;

error:
    STRING_FREE (name);
    STRING_FREE (default_style);
    STRING_FREE (bold);
    STRING_FREE (italic);
    STRING_FREE (underline);
    STRING_FREE (strikethrough);
    STRING_FREE (foreground);
    STRING_FREE (background);
    style_xml_free (xml);
    return NULL;
}


static void
style_xml_free (StyleXML *xml)
{
    if (xml)
    {
        g_free (xml->name);
        g_free (xml->default_style);
        g_free (xml->bold);
        g_free (xml->italic);
        g_free (xml->underline);
        g_free (xml->strikethrough);
        g_free (xml->foreground);
        g_free (xml->background);
        g_free (xml);
    }
}


static void
style_list_xml_free (StyleListXML *xml)
{
    if (xml)
    {
        g_slist_foreach (xml->styles, (GFunc) style_xml_free, NULL);
        g_slist_free (xml->styles);
        g_free (xml);
    }
}


/****************************************************************************/
/* Rules
 */

static RuleXML  *rule_string_xml_parse      (xmlNode            *node);
static RuleXML  *rule_regex_xml_parse       (xmlNode            *node);
static RuleXML  *rule_char_xml_parse        (xmlNode            *node);
static RuleXML  *rule_2char_xml_parse       (xmlNode            *node);
static RuleXML  *rule_any_char_xml_parse    (xmlNode            *node);
static RuleXML  *rule_special_xml_parse     (xmlNode            *node);
static RuleXML  *rule_keywords_xml_parse    (LangXML            *lang_xml,
                                             xmlNode            *node);
static RuleXML  *rule_include_xml_parse     (LangXML            *lang_xml,
                                             xmlNode            *node);

static void      rule_string_xml_free       (RuleStringXML      *xml);
static void      rule_regex_xml_free        (RuleRegexXML       *xml);
static void      rule_any_char_xml_free     (RuleAnyCharXML     *xml);
static void      rule_keywords_xml_free     (RuleKeywordsXML    *xml);
static void      rule_include_xml_free      (RuleIncludeXML     *xml);

static MooRule  *rule_string_xml_create_rule    (RuleStringXML      *xml);
static MooRule  *rule_regex_xml_create_rule     (RuleRegexXML       *xml);
static MooRule  *rule_any_char_xml_create_rule  (RuleAnyCharXML     *xml);
static MooRule  *rule_keywords_xml_create_rule  (RuleKeywordsXML    *xml,
                                                 LangXML            *lang_xml);
static MooRule  *rule_include_xml_create_rule   (RuleIncludeXML     *xml,
                                                 MooLang            *lang);
static MooRule  *rule_char_xml_create_rule      (RuleCharXML        *xml);
static MooRule  *rule_2char_xml_create_rule     (Rule2CharXML       *xml);
static MooRule  *rule_special_xml_create_rule   (RuleXML            *xml);

#define rule_char_xml_free g_free
#define rule_2char_xml_free g_free
#define rule_special_xml_free g_free

#define RULE_IS__(node__,name__)            (!g_ascii_strcasecmp ((char*)node__->name, name__))
#define RULE_IS_STRING_NODE(node__)         (RULE_IS__(node__, RULE_ASCII_STRING_ELM))
#define RULE_IS_REGEX_NODE(node__)          (RULE_IS__(node__, RULE_REGEX_ELM))
#define RULE_IS_CHAR_NODE(node__)           (RULE_IS__(node__, RULE_ASCII_CHAR_ELM))
#define RULE_IS_2CHAR_NODE(node__)          (RULE_IS__(node__, RULE_ASCII_2CHAR_ELM))
#define RULE_IS_ANY_CHAR_NODE(node__)       (RULE_IS__(node__, RULE_ASCII_ANY_CHAR_ELM))
#define RULE_IS_KEYWORDS_NODE(node__)       (RULE_IS__(node__, RULE_KEYWORDS_ELM))
#define RULE_IS_INCLUDE_NODE(node__)        (RULE_IS__(node__, RULE_INCLUDE_RULES_ELM))
#define RULE_IS_INT_NODE(node__)            (RULE_IS__(node__, RULE_INT_ELM))
#define RULE_IS_HEX_NODE(node__)            (RULE_IS__(node__, RULE_HEX_ELM))
#define RULE_IS_OCTAL_NODE(node__)          (RULE_IS__(node__, RULE_OCTAL_ELM))
#define RULE_IS_FLOAT_NODE(node__)          (RULE_IS__(node__, RULE_FLOAT_ELM))
#define RULE_IS_C_CHAR_NODE(node__)         (RULE_IS__(node__, RULE_C_CHAR_ELM))
#define RULE_IS_ESCAPED_CHAR_NODE(node__)   (RULE_IS__(node__, RULE_ESCAPED_CHAR_ELM))
#define RULE_IS_WHITESPACE_NODE(node__)     (RULE_IS__(node__, RULE_WHITESPACE_ELM))
#define RULE_IS_IDENTIFIER_NODE(node__)     (RULE_IS__(node__, RULE_IDENTIFIER_ELM))
#define RULE_IS_LINE_CONTINUE_NODE(node__)  (RULE_IS__(node__, RULE_LINE_CONTINUE_ELM))


static RuleXML*
rule_xml_parse (LangXML *lang_xml,
                xmlNode *node)
{
    RuleXML *xml = NULL;
    xmlAttr *attr;

    g_assert (IS_ELEMENT_NODE (node));

    if (!node->name || !node->name[0])
    {
        g_warning ("%s: no name in rule node", G_STRLOC);
        goto error;
    }

    if (RULE_IS_STRING_NODE (node))
    {
        xml = rule_string_xml_parse (node);
        if (xml) xml->type = RULE_STRING;
    }
    else if (RULE_IS_REGEX_NODE (node))
    {
        xml = rule_regex_xml_parse (node);
        if (xml) xml->type = RULE_REGEX;
    }
    else if (RULE_IS_CHAR_NODE (node))
    {
        xml = rule_char_xml_parse (node);
        if (xml) xml->type = RULE_CHAR;
    }
    else if (RULE_IS_2CHAR_NODE (node))
    {
        xml = rule_2char_xml_parse (node);
        if (xml) xml->type = RULE_2CHAR;
    }
    else if (RULE_IS_ANY_CHAR_NODE (node))
    {
        xml = rule_any_char_xml_parse (node);
        if (xml) xml->type = RULE_ANY_CHAR;
    }
    else if (RULE_IS_KEYWORDS_NODE (node))
    {
        xml = rule_keywords_xml_parse (lang_xml, node);
        if (xml) xml->type = RULE_KEYWORDS;
    }
    else if (RULE_IS_INCLUDE_NODE (node))
    {
        xml = rule_include_xml_parse (lang_xml, node);
        if (xml) xml->type = RULE_INCLUDE;
    }
    else if (RULE_IS_INT_NODE (node))
    {
        xml = rule_special_xml_parse (node);
        if (xml) xml->type = RULE_INT;
    }
    else if (RULE_IS_FLOAT_NODE (node))
    {
        xml = rule_special_xml_parse (node);
        if (xml) xml->type = RULE_FLOAT;
    }
    else if (RULE_IS_OCTAL_NODE (node))
    {
        xml = rule_special_xml_parse (node);
        if (xml) xml->type = RULE_OCTAL;
    }
    else if (RULE_IS_HEX_NODE (node))
    {
        xml = rule_special_xml_parse (node);
        if (xml) xml->type = RULE_HEX;
    }
    else if (RULE_IS_C_CHAR_NODE (node))
    {
        xml = rule_special_xml_parse (node);
        if (xml) xml->type = RULE_C_CHAR;
    }
    else if (RULE_IS_ESCAPED_CHAR_NODE (node))
    {
        xml = rule_special_xml_parse (node);
        if (xml) xml->type = RULE_ESCAPED_CHAR;
    }
    else if (RULE_IS_WHITESPACE_NODE (node))
    {
        xml = rule_special_xml_parse (node);
        if (xml) xml->type = RULE_WHITESPACE;
    }
    else if (RULE_IS_IDENTIFIER_NODE (node))
    {
        xml = rule_special_xml_parse (node);
        if (xml) xml->type = RULE_IDENTIFIER;
    }
    else if (RULE_IS_LINE_CONTINUE_NODE (node))
    {
        xml = rule_special_xml_parse (node);
        if (xml) xml->type = RULE_LINE_CONTINUE;
    }
    else
    {
        g_warning ("%s: invalid rule type '%s'", G_STRLOC, node->name);
        goto error;
    }

    if (!xml)
        goto error;

    for (attr = node->properties; attr != NULL; attr = attr->next)
    {
        xmlChar *val;

        val = xmlGetProp (node, attr->name);

        if (!val)
        {
            g_warning ("%s: oops", G_STRLOC);
            goto error;
        }

        if (prop_equal (attr->name, RULE_STYLE_PROP))
        {
            if (!val[0])
            {
                g_warning ("%s: empty style name in rule '%s'", G_STRLOC, node->name);
                xmlFree (val);
                goto error;
            }
            else if (xml->style)
            {
                g_warning ("%s: duplicated style tag in rule '%s'", G_STRLOC, node->name);
                xmlFree (val);
                goto error;
            }
            else
            {
                xml->style = STRDUP (val);
                lang_xml_add_style_ref (lang_xml, xml->style);
            }
        }
        else if (prop_equal (attr->name, RULE_CTX_PROP))
        {
            if (!val[0])
            {
                g_warning ("%s: empty context name in rule '%s'", G_STRLOC, node->name);
                xmlFree (val);
                goto error;
            }
            else if (xml->context)
            {
                g_warning ("%s: duplicated context tag in rule '%s'", G_STRLOC, node->name);
                xmlFree (val);
                goto error;
            }
            else
            {
                if (!parse_context_ref (val, &xml->end_switch_info))
                {
                    g_warning ("%s: invalid context value in rule '%s'", G_STRLOC, node->name);
                    xmlFree (val);
                    goto error;
                }

                switch (xml->end_switch_info.type)
                {
                    case MOO_CONTEXT_SWITCH:
#if 0
                    case MOO_CONTEXT_JUMP:
#endif
                        lang_xml_add_cross_ref (lang_xml,
                                xml->end_switch_info.ref.lang,
                                xml->end_switch_info.ref.name);
                        break;
                    case MOO_CONTEXT_STAY:
                    case MOO_CONTEXT_POP:
                        break;
                }
            }
        }
        else if (prop_equal (attr->name, RULE_INCLUDE_EOL_PROP))
        {
            xml->include_eol = parse_bool (val, TRUE);
        }
        else if (prop_equal (attr->name, RULE_INCLUDE_PROP))
        {
            xml->includes_into_next = parse_bool (val, FALSE);
        }
        else if (prop_equal (attr->name, RULE_BOL_PROP))
        {
            xml->bol_only = parse_bool (val, FALSE);
        }
        else if (prop_equal (attr->name, RULE_FIRST_NON_BLANK_PROP))
        {
            xml->first_non_blank_only = parse_bool (val, FALSE);
        }
        else if (prop_equal (attr->name, RULE_FIRST_LINE_PROP))
        {
            xml->first_line_only = parse_bool (val, FALSE);
        }
        else if (prop_equal (attr->name, RULE_CASELESS_PROP))
        {
            xml->caseless = parse_bool (val, FALSE);
        }

        xmlFree (val);
    }

    DEBUG_PRINT ({
        g_print ("rule: %s\n", node->name);
    });

    ELM_FOREACH (node, child)
    {
        RuleXML *child_rule = rule_xml_parse (lang_xml, child);

        if (!child_rule)
            goto error;

        xml->child_rules = g_slist_prepend (xml->child_rules, child_rule);
    }
    ELM_FOREACH_END;

    xml->child_rules = g_slist_reverse (xml->child_rules);
    return xml;

error:
    rule_xml_free (xml);
    return NULL;
}


inline static MooRuleFlags
rule_xml_get_flags (gpointer xmlptr)
{
    RuleXML *xml = xmlptr;
    MooRuleFlags flags = 0;

    if (xml->includes_into_next)
        flags |= MOO_RULE_INCLUDE_INTO_NEXT;
    if (xml->bol_only)
        flags |= MOO_RULE_MATCH_FIRST_CHAR;
    if (xml->first_non_blank_only)
        flags |= MOO_RULE_MATCH_FIRST_NON_EMPTY_CHAR;
    if (xml->first_line_only)
        flags |= MOO_RULE_MATCH_FIRST_LINE;
    if (xml->caseless)
        flags |= MOO_RULE_MATCH_CASELESS;

    return flags;
}


inline static const char*
rule_xml_get_style (gpointer xmlptr)
{
    return ((RuleXML*)xmlptr)->style;
}


MooRule*
_moo_rule_new_from_xml (RuleXML    *xml,
                        LangXML    *lang_xml,
                        MooLang    *lang)
{
    MooRule *rule = NULL;
    MooContext *switch_to;
    GSList *l;

    g_return_val_if_fail (xml != NULL && lang != NULL, NULL);

    switch (xml->type)
    {
        case RULE_STRING:
            rule = rule_string_xml_create_rule ((RuleStringXML*) xml);
            break;
        case RULE_REGEX:
            rule = rule_regex_xml_create_rule ((RuleRegexXML*) xml);
            break;
        case RULE_CHAR:
            rule = rule_char_xml_create_rule ((RuleCharXML*) xml);
            break;
        case RULE_2CHAR:
            rule = rule_2char_xml_create_rule ((Rule2CharXML*) xml);
            break;
        case RULE_ANY_CHAR:
            rule = rule_any_char_xml_create_rule ((RuleAnyCharXML*) xml);
            break;
        case RULE_KEYWORDS:
            rule = rule_keywords_xml_create_rule ((RuleKeywordsXML*) xml, lang_xml);
            break;
        case RULE_INCLUDE:
            rule = rule_include_xml_create_rule ((RuleIncludeXML*) xml, lang);
            break;
        case RULE_INT:
        case RULE_FLOAT:
        case RULE_HEX:
        case RULE_OCTAL:
        case RULE_ESCAPED_CHAR:
        case RULE_C_CHAR:
        case RULE_WHITESPACE:
        case RULE_IDENTIFIER:
        case RULE_LINE_CONTINUE:
            rule = rule_special_xml_create_rule (xml);
            break;
    }

    if (!rule)
        return NULL;

    switch (xml->end_switch_info.type)
    {
        case MOO_CONTEXT_STAY:
            _moo_rule_set_end_stay (rule);
            break;
        case MOO_CONTEXT_POP:
            _moo_rule_set_end_pop (rule, xml->end_switch_info.num);
            break;
        case MOO_CONTEXT_SWITCH:
#if 0
        case MOO_CONTEXT_JUMP:
#endif
            if (xml->end_switch_info.ref.lang)
                switch_to = moo_lang_mgr_get_context (lang->mgr,
                                                      xml->end_switch_info.ref.lang,
                                                      xml->end_switch_info.ref.name);
            else
                switch_to = moo_lang_get_context (lang, xml->end_switch_info.ref.name);

            if (!switch_to)
            {
                g_critical ("%s: oops", G_STRLOC);
                _moo_rule_free (rule);
                return NULL;
            }
            else
            {
#if 0
                if (xml->end_switch_info.type == MOO_CONTEXT_SWITCH)
#endif
                    _moo_rule_set_end_switch (rule, switch_to);
#if 0
                else
                    _moo_rule_set_end_jump (rule, switch_to);
#endif
            }
            break;
    }

    if (xml->include_eol)
        rule->include_eol = TRUE;

    for (l = xml->child_rules; l != NULL; l = l->next)
    {
        RuleXML *child_xml = l->data;
        MooRule *child_rule = _moo_rule_new_from_xml (child_xml, lang_xml, lang);

        if (!child_rule)
        {
            _moo_rule_free (rule);
            return NULL;
        }

        _moo_rule_add_child_rule (rule, child_rule);
    }

    return rule;
}


static void
rule_xml_free (RuleXML        *xml)
{
    if (xml)
    {
        g_free (xml->style);
        g_free (xml->context);

        g_slist_foreach (xml->child_rules, (GFunc) rule_xml_free, NULL);
        g_slist_free (xml->child_rules);

        ctx_switch_info_destroy (&xml->end_switch_info);

        switch (xml->type)
        {
            case RULE_STRING:
                rule_string_xml_free ((RuleStringXML*) xml);
                break;
            case RULE_REGEX:
                rule_regex_xml_free ((RuleRegexXML*) xml);
                break;
            case RULE_CHAR:
                rule_char_xml_free ((RuleCharXML*) xml);
                break;
            case RULE_2CHAR:
                rule_2char_xml_free ((Rule2CharXML*) xml);
                break;
            case RULE_ANY_CHAR:
                rule_any_char_xml_free ((RuleAnyCharXML*) xml);
                break;
            case RULE_KEYWORDS:
                rule_keywords_xml_free ((RuleKeywordsXML*) xml);
                break;
            case RULE_INCLUDE:
                rule_include_xml_free ((RuleIncludeXML*) xml);
                break;
            case RULE_INT:
            case RULE_FLOAT:
            case RULE_HEX:
            case RULE_OCTAL:
            case RULE_ESCAPED_CHAR:
            case RULE_C_CHAR:
            case RULE_WHITESPACE:
            case RULE_IDENTIFIER:
            case RULE_LINE_CONTINUE:
                rule_special_xml_free (xml);
                break;
        }
    }
}


static RuleXML*
rule_string_xml_parse (xmlNode *node)
{
    RuleStringXML *xml;
    xmlChar *string;

    g_assert (RULE_IS_STRING_NODE (node));

    string = GET_PROP (node, RULE_STRING_STRING_PROP);

    if (!string)
    {
        g_warning ("%s: '" RULE_STRING_STRING_PROP "' attribute missing in rule %s",
                   G_STRLOC, node->name);
        return NULL;
    }

    xml = g_new0 (RuleStringXML, 1);
    xml->string = STRDUP (string);

    xmlFree (string);
    return (RuleXML*) xml;
}


static MooRule*
rule_string_xml_create_rule (RuleStringXML *xml)
{
    return _moo_rule_string_new (xml->string,
                                 rule_xml_get_flags (xml),
                                 rule_xml_get_style (xml));
}


static void
rule_string_xml_free (RuleStringXML *xml)
{
    g_free (xml->string);
    g_free (xml);
}


static RuleXML*
rule_regex_xml_parse (xmlNode            *node)
{
    RuleRegexXML *xml;
    xmlChar *pattern, *non_empty;

    g_assert (RULE_IS_REGEX_NODE (node));

    pattern = GET_PROP (node, RULE_REGEX_PATTERN_PROP);
    non_empty = GET_PROP (node, RULE_REGEX_NON_EMPTY_PROP);

    if (!pattern)
    {
        g_warning ("%s: '" RULE_REGEX_PATTERN_PROP "' attribute missing in rule %s",
                   G_STRLOC, node->name);
        STRING_FREE (non_empty);
        return NULL;
    }

    xml = g_new0 (RuleRegexXML, 1);
    xml->pattern = STRDUP (pattern);
    xml->non_empty = parse_bool (non_empty, TRUE);

    STRING_FREE (pattern);
    STRING_FREE (non_empty);
    return (RuleXML*) xml;
}


static MooRule*
rule_regex_xml_create_rule (RuleRegexXML *xml)
{
    return _moo_rule_regex_new (xml->pattern, xml->non_empty,
                                0, 0,
                                rule_xml_get_flags (xml),
                                rule_xml_get_style (xml));
}


static void
rule_regex_xml_free (RuleRegexXML       *xml)
{
    g_free (xml->pattern);
    g_free (xml);
}


static RuleXML*
rule_char_xml_parse (xmlNode            *node)
{
    RuleCharXML *xml;
    xmlChar *ch;

    g_assert (RULE_IS_CHAR_NODE (node));

    ch = GET_PROP (node, RULE_CHAR_CHAR_PROP);

    if (!ch || !ch[0])
    {
        g_warning ("%s: '" RULE_CHAR_CHAR_PROP "' attribute missing in rule %s",
                   G_STRLOC, node->name);
        return NULL;
    }

    if (ch[1] != 0)
    {
        g_warning ("%s: invalid '" RULE_CHAR_CHAR_PROP "' attribute '%s' in rule %s",
                   G_STRLOC, ch, node->name);
        goto error;
    }

    xml = g_new0 (RuleCharXML, 1);
    xml->ch = ch[0];

    xmlFree (ch);
    return (RuleXML*) xml;

error:
    STRING_FREE (ch);
    return NULL;
}


static MooRule*
rule_char_xml_create_rule (RuleCharXML        *xml)
{
    return _moo_rule_char_new (xml->ch,
                               rule_xml_get_flags (xml),
                               rule_xml_get_style (xml));
}


static RuleXML*
rule_special_xml_parse (G_GNUC_UNUSED xmlNode *node)
{
    return g_new0 (RuleXML, 1);
}


static MooRule*
rule_special_xml_create_rule (RuleXML *xml)
{
    MooRuleFlags flags = rule_xml_get_flags (xml);
    const char *style = rule_xml_get_style (xml);

    switch (xml->type)
    {
        case RULE_INT:
            return _moo_rule_int_new (flags, style);
        case RULE_WHITESPACE:
            return _moo_rule_whitespace_new (flags, style);
        case RULE_IDENTIFIER:
            return _moo_rule_identifier_new (flags, style);
        case RULE_FLOAT:
            return _moo_rule_float_new (flags, style);
        case RULE_HEX:
            return _moo_rule_hex_new (flags, style);
        case RULE_OCTAL:
            return _moo_rule_octal_new (flags, style);
        case RULE_ESCAPED_CHAR:
            return _moo_rule_escaped_char_new (flags, style);
        case RULE_C_CHAR:
            return _moo_rule_c_char_new (flags, style);
        case RULE_LINE_CONTINUE:
            return _moo_rule_line_continue_new (flags, style);
        default:
            g_return_val_if_reached (NULL);
    }
}


static RuleXML*
rule_2char_xml_parse (xmlNode            *node)
{
    Rule2CharXML *xml;
    xmlChar *ch1, *ch2;

    g_assert (RULE_IS_2CHAR_NODE (node));

    ch1 = GET_PROP (node, RULE_2CHAR_CHAR1_PROP);
    ch2 = GET_PROP (node, RULE_2CHAR_CHAR2_PROP);

    if (!ch1 || !ch1[0])
    {
        g_warning ("%s: '" RULE_2CHAR_CHAR1_PROP "' attribute missing in rule %s",
                   G_STRLOC, node->name);
        goto error;
    }

    if (!ch2 || !ch2[0])
    {
        g_warning ("%s: '" RULE_2CHAR_CHAR2_PROP "' attribute missing in rule %s",
                   G_STRLOC, node->name);
        goto error;
    }

    if (ch1[1] != 0)
    {
        g_warning ("%s: invalid '" RULE_2CHAR_CHAR1_PROP "' attribute '%s' in rule %s",
                   G_STRLOC, ch1, node->name);
        goto error;
    }

    if (ch2[1] != 0)
    {
        g_warning ("%s: invalid '" RULE_2CHAR_CHAR2_PROP "' attribute '%s' in rule %s",
                   G_STRLOC, ch2, node->name);
        goto error;
    }

    xml = g_new0 (Rule2CharXML, 1);
    xml->chars[0] = ch1[0];
    xml->chars[1] = ch2[0];

    xmlFree (ch1);
    xmlFree (ch2);
    return (RuleXML*) xml;

error:
    STRING_FREE (ch1);
    STRING_FREE (ch2);
    return NULL;
}


static MooRule*
rule_2char_xml_create_rule (Rule2CharXML *xml)
{
    return _moo_rule_2char_new (xml->chars[0], xml->chars[1],
                                rule_xml_get_flags (xml),
                                rule_xml_get_style (xml));
}


static RuleXML*
rule_any_char_xml_parse (xmlNode            *node)
{
    RuleAnyCharXML *xml;
    xmlChar *range;

    g_assert (RULE_IS_ANY_CHAR_NODE (node));

    range = GET_PROP (node, RULE_ANY_CHAR_CHARS_PROP);

    if (!range)
    {
        g_warning ("%s: '" RULE_ANY_CHAR_CHARS_PROP "' attribute missing in rule %s",
                   G_STRLOC, node->name);
        return NULL;
    }

    xml = g_new0 (RuleAnyCharXML, 1);
    xml->range = STRDUP (range);

    xmlFree (range);
    return (RuleXML*) xml;
}


static MooRule*
rule_any_char_xml_create_rule (RuleAnyCharXML       *xml)
{
    return _moo_rule_any_char_new (xml->range,
                                   rule_xml_get_flags (xml),
                                   rule_xml_get_style (xml));
}


static void
rule_any_char_xml_free (RuleAnyCharXML       *xml)
{
    g_free (xml->range);
    g_free (xml);
}


static RuleXML*
rule_keywords_xml_parse (LangXML *lang_xml,
                         xmlNode *node)
{
    RuleKeywordsXML *xml;
    xmlChar *keyword;

    g_assert (RULE_IS_KEYWORDS_NODE (node));

    keyword = GET_PROP (node, RULE_KEYWORDS_KEYWORD_PROP);

    if (!keyword)
    {
        g_warning ("%s: '" RULE_KEYWORDS_KEYWORD_PROP "' attribute missing in rule %s",
                   G_STRLOC, node->name);
        return NULL;
    }

    xml = g_new0 (RuleKeywordsXML, 1);
    xml->keywords = STRDUP (keyword);

    lang_xml_add_keyword_ref (lang_xml, keyword);

    xmlFree (keyword);
    return (RuleXML*) xml;
}


static MooRule*
rule_keywords_xml_create_rule (RuleKeywordsXML    *xml,
                               LangXML            *lang_xml)
{
    KeywordXML *kw_xml;

    kw_xml = g_hash_table_lookup (lang_xml->keyword_names, xml->keywords);
    g_return_val_if_fail (kw_xml != NULL, NULL);

    return _moo_rule_keywords_new (kw_xml->words,
                                   rule_xml_get_flags (xml),
                                   kw_xml->prefix, kw_xml->suffix,
                                   kw_xml->word_boundary,
                                   rule_xml_get_style (xml));
}


static void
rule_keywords_xml_free (RuleKeywordsXML    *xml)
{
    g_free (xml->keywords);
    g_free (xml);
}


static RuleXML*
rule_include_xml_parse (LangXML            *lang_xml,
                        xmlNode            *node)
{
    RuleIncludeXML *xml;
    xmlChar *from = NULL;
    char *lang = NULL, *name = NULL;

    g_assert (RULE_IS_INCLUDE_NODE (node));

    from = GET_PROP (node, RULE_INCLUDE_FROM_PROP);

    if (!from || !from[0])
    {
        g_warning ("%s: '" RULE_INCLUDE_FROM_PROP "' attribute missing in rule %s",
                   G_STRLOC, node->name);
        goto error;
    }

    if (!parse_name_ref (from, &lang, &name))
    {
        g_warning ("%s: invalid value '%s' of attribute '" RULE_INCLUDE_FROM_PROP "' in rule %s",
                   G_STRLOC, from, node->name);
        goto error;
    }

    xml = g_new0 (RuleIncludeXML, 1);
    xml->from_context = name;
    xml->from_lang = lang;

    lang_xml_add_cross_ref (lang_xml, lang, name);

    xmlFree (from);
    return (RuleXML*) xml;

error:
    STRING_FREE (from);
    g_free (lang);
    g_free (name);
    return NULL;
}


static MooRule*
rule_include_xml_create_rule (RuleIncludeXML *xml,
                              MooLang        *lang)
{
    MooContext *ctx;

    if (xml->from_lang)
        ctx = moo_lang_mgr_get_context (lang->mgr, xml->from_lang, xml->from_context);
    else
        ctx = moo_lang_get_context (lang, xml->from_context);

    g_return_val_if_fail (ctx != NULL, NULL);

    return _moo_rule_include_new (ctx);
}


static void
rule_include_xml_free (RuleIncludeXML    *xml)
{
    g_free (xml->from_lang);
    g_free (xml->from_context);
    g_free (xml);
}

#endif /* MOO_USE_XML */


/****************************************************************************/
/* Style schemes
 * Style schemes are stored in simplish XML files without fancy entities or
 * something, so GMarkup will do fine
 */

static gboolean
parse_bool_check (const char *string,
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
parse_color_check (const char *string,
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
style_from_xml (MooMarkupNode *node)
{
    const char *default_style_prop = NULL;
    const char *bold_prop = NULL, *italic_prop = NULL;
    const char *underline_prop = NULL, *strikethrough_prop = NULL;
    const char *foreground_prop = NULL, *background_prop = NULL;
    GdkColor foreground, background;
    gboolean bold, italic, underline, strikethrough;
    MooTextStyleMask mask = 0;

    g_return_val_if_fail (node != NULL, NULL);

    default_style_prop = moo_markup_get_prop (node, STYLE_DEF_STYLE_PROP);
    bold_prop = moo_markup_get_prop (node, STYLE_BOLD_PROP);
    italic_prop = moo_markup_get_prop (node, STYLE_ITALIC_PROP);
    underline_prop = moo_markup_get_prop (node, STYLE_UNDERLINE_PROP);
    strikethrough_prop = moo_markup_get_prop (node, STYLE_STRIKETHROUGH_PROP);
    foreground_prop = moo_markup_get_prop (node, STYLE_FOREGROUND_PROP);
    background_prop = moo_markup_get_prop (node, STYLE_BACKGROUND_PROP);

    if (bold_prop && parse_bool_check (bold_prop, &bold))
        mask |= MOO_TEXT_STYLE_BOLD;
    if (italic_prop && parse_bool_check (italic_prop, &italic))
        mask |= MOO_TEXT_STYLE_ITALIC;
    if (underline_prop && parse_bool_check (underline_prop, &underline))
        mask |= MOO_TEXT_STYLE_UNDERLINE;
    if (strikethrough_prop && parse_bool_check (strikethrough_prop, &strikethrough))
        mask |= MOO_TEXT_STYLE_STRIKETHROUGH;
    if (foreground_prop && parse_color_check (foreground_prop, &foreground))
        mask |= MOO_TEXT_STYLE_FOREGROUND;
    if (background_prop && parse_color_check (background_prop, &background))
        mask |= MOO_TEXT_STYLE_BACKGROUND;

    return moo_text_style_new (default_style_prop,
                               &foreground, &background,
                               bold, italic, underline,
                               strikethrough, mask, FALSE);
}


#define M_NODE_NAME_IS_(node_, name_)       (node_->name && !strcmp (node_->name, name_))
#define M_IS_LANGUAGE_NODE(node_)           (MOO_MARKUP_IS_ELEMENT(node_) && M_NODE_NAME_IS_(node_, LANGUAGE_ELM))
#define M_IS_STYLE_NODE(node_)              (MOO_MARKUP_IS_ELEMENT(node_) && M_NODE_NAME_IS_(node_, STYLE_ELM))
#define M_IS_SCHEME_NODE(node_)             (MOO_MARKUP_IS_ELEMENT(node_) && M_NODE_NAME_IS_(node_, SCHEME_ELM))
#define M_IS_DEFAULT_STYLE_NODE(node_)      (MOO_MARKUP_IS_ELEMENT(node_) && M_NODE_NAME_IS_(node_, DEFAULT_STYLE_ELM))
#define M_IS_BRACKET_MATCH_NODE(node_)      (MOO_MARKUP_IS_ELEMENT(node_) && M_NODE_NAME_IS_(node_, BRACKET_MATCH_ELM))
#define M_IS_BRACKET_MISMATCH_NODE(node_)   (MOO_MARKUP_IS_ELEMENT(node_) && M_NODE_NAME_IS_(node_, BRACKET_MISMATCH_ELM))

static gboolean
style_scheme_xml_parse_lang (MooTextStyleScheme *scheme,
                             MooMarkupNode      *node)
{
    const char *lang_name = NULL;
    MooMarkupNode *child;

    g_return_val_if_fail (M_IS_LANGUAGE_NODE (node), FALSE);

    lang_name = moo_markup_get_prop (node, LANG_NAME_PROP);

    if (!lang_name || !lang_name[0])
    {
        g_warning ("%s: no name in '" LANGUAGE_ELM "' node", G_STRLOC);
        goto error;
    }

    for (child = node->children; child != NULL; child = child->next)
    {
        if (!MOO_MARKUP_IS_ELEMENT (child))
            continue;

        if (M_IS_STYLE_NODE (child))
        {
            const char *style_name = moo_markup_get_prop (child, STYLE_NAME_PROP);
            MooTextStyle *style;

            if (!style_name)
            {
                g_warning ("%s: style name absent in '" STYLE_ELM "' node",
                           G_STRLOC);
                goto error;
            }

            style = style_from_xml (child);

            if (!style)
                goto error;

            moo_text_style_scheme_set (scheme, lang_name, style_name, style);
            moo_text_style_free (style);
        }
        else
        {
            g_warning ("%s: unknown tag '%s'", G_STRLOC, child->name);
            goto error;
        }
    }

    return TRUE;

error:
    return FALSE;
}


static MooTextStyleScheme*
style_scheme_xml_parse (MooMarkupNode  *node,
                        char          **base_scheme)
{
    MooTextStyleScheme *scheme = NULL;
    MooMarkupNode *child;
    const char *name = NULL, *foreground = NULL, *background = NULL, *base = NULL;
    const char *selected_foreground = NULL, *selected_background = NULL, *current_line = NULL;

    g_return_val_if_fail (M_IS_SCHEME_NODE (node), NULL);

    name = moo_markup_get_prop (node, SCHEME_NAME_PROP);
    foreground = moo_markup_get_prop (node, SCHEME_FOREGROUND_PROP);
    background = moo_markup_get_prop (node, SCHEME_BACKGROUND_PROP);
    selected_foreground = moo_markup_get_prop (node, SCHEME_SEL_FOREGROUND_PROP);
    selected_background = moo_markup_get_prop (node, SCHEME_SEL_BACKGROUND_PROP);
    current_line = moo_markup_get_prop (node, SCHEME_CURRENT_LINE_PROP);
    base = moo_markup_get_prop (node, SCHEME_BASE_SCHEME_PROP);

    if (!name || !name[0])
    {
        g_warning ("%s: no name in '" SCHEME_ELM "' node", G_STRLOC);
        goto error;
    }

    scheme = moo_text_style_scheme_new_empty (name, NULL);

    scheme->text_colors[MOO_TEXT_COLOR_FG] = g_strdup (foreground);
    scheme->text_colors[MOO_TEXT_COLOR_BG] = g_strdup (background);
    scheme->text_colors[MOO_TEXT_COLOR_SEL_FG] = g_strdup (selected_foreground);
    scheme->text_colors[MOO_TEXT_COLOR_SEL_BG] = g_strdup (selected_background);
    scheme->text_colors[MOO_TEXT_COLOR_CUR_LINE] = g_strdup (current_line);

    for (child = node->children; child != NULL; child = child->next)
    {
        if (!MOO_MARKUP_IS_ELEMENT (child))
            continue;

        if (M_IS_DEFAULT_STYLE_NODE (child))
        {
            const char *style_name = moo_markup_get_prop (child, STYLE_NAME_PROP);
            MooTextStyle *style;

            if (!style_name)
            {
                g_warning ("%s: style name absent in '" DEFAULT_STYLE_ELM "' node",
                           G_STRLOC);
                goto error;
            }

            style = style_from_xml (child);

            if (!style)
                goto error;

            moo_text_style_scheme_set (scheme, NULL, style_name, style);
            moo_text_style_free (style);
        }
        else if (M_IS_LANGUAGE_NODE (child))
        {
            if (!style_scheme_xml_parse_lang (scheme, child))
                goto error;
        }
        else if (M_IS_BRACKET_MATCH_NODE (child))
        {
            if (scheme->bracket_match)
            {
                g_warning ("%s: duplicated '" BRACKET_MATCH_ELM "'", G_STRLOC);
                goto error;
            }

            scheme->bracket_match = style_from_xml (child);

            if (!scheme->bracket_match)
                goto error;
        }
        else if (M_IS_BRACKET_MISMATCH_NODE (child))
        {
            if (scheme->bracket_mismatch)
            {
                g_warning ("%s: duplicated '" BRACKET_MISMATCH_ELM "'", G_STRLOC);
                goto error;
            }

            scheme->bracket_mismatch = style_from_xml (child);

            if (!scheme->bracket_mismatch)
                goto error;
        }
        else
        {
            g_warning ("%s: unknown tag '%s'", G_STRLOC, child->name);
            goto error;
        }
    }

    if (base_scheme)
        *base_scheme = g_strdup (base);

    return scheme;

error:
    if (scheme)
        g_object_unref (scheme);
    return NULL;
}


MooTextStyleScheme*
_moo_text_style_scheme_parse_file (const char *file,
                                   char      **base_scheme)
{
    MooMarkupDoc *doc;
    MooMarkupNode *root;
    MooTextStyleScheme *scheme;
    GError *error = NULL;

    g_return_val_if_fail (file != NULL, NULL);

    doc = moo_markup_parse_file (file, &error);

    if (!doc)
    {
        g_warning ("%s: could not parse file '%s'", G_STRLOC, file);
        if (error)
        {
            g_warning ("%s: %s", G_STRLOC, error->message);
            g_error_free (error);
        }
        return NULL;
    }

    root = moo_markup_get_root_element (doc, NULL);
    scheme = style_scheme_xml_parse (root, base_scheme);

    moo_markup_doc_unref (doc);
    return scheme;
}


MooTextStyleScheme*
_moo_text_style_scheme_parse_memory (const char     *buffer,
                                     int             size,
                                     char          **base_scheme)
{
    MooMarkupDoc *doc;
    MooMarkupNode *root;
    MooTextStyleScheme *scheme;
    GError *error = NULL;

    g_return_val_if_fail (buffer != NULL, NULL);

    if (size < 0)
        size = strlen (buffer);

    doc = moo_markup_parse_memory (buffer, size, &error);

    if (!doc)
    {
        g_warning ("%s: could not parse string", G_STRLOC);
        if (error)
        {
            g_warning ("%s: %s", G_STRLOC, error->message);
            g_error_free (error);
        }
        return NULL;
    }

    root = moo_markup_get_root_element (doc, NULL);
    scheme = style_scheme_xml_parse (root, base_scheme);

    moo_markup_doc_unref (doc);
    return scheme;
}
