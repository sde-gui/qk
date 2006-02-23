/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   as-plugin-xml.c
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

#include "as-plugin-xml.h"
#include "mooutils/moomarkup.h"
#include "mooutils/mooutils-misc.h"
#include <string.h>

#define AS_ROOT         "active-strings"
#define AS_ELM          "as"
#define AS_PROP_PATTERN "pattern"
#define AS_PROP_LANG    "lang"


ASInfo *
_as_info_new (const char *pattern,
              const char *script,
              const char *lang)
{
    ASInfo *info;

    g_return_val_if_fail (pattern && pattern[0], NULL);

    info = g_new0 (ASInfo, 1);
    info->pattern = g_strdup (pattern);
    info->script = g_strdup (script);
    info->lang = lang ? g_ascii_strdown (lang, -1) : NULL;
    return info;
}


void
_as_info_free (ASInfo *info)
{
    if (info)
    {
        g_free (info->pattern);
        g_free (info->script);
        g_free (info->lang);
        g_free (info);
    }
}


static ASInfo *
parse_as_elm (MooMarkupNode *node)
{
    const char *pattern;
    const char *script;
    const char *lang;

    pattern = moo_markup_get_prop (node, AS_PROP_PATTERN);
    lang = moo_markup_get_prop (node, AS_PROP_LANG);
    script = moo_markup_get_content (node);

    if (!pattern || !pattern[0])
    {
        g_warning ("%s: '%s' attribute missing",
                   G_STRLOC, AS_PROP_PATTERN);
        return NULL;
    }

    return _as_info_new (pattern, script, lang);
}


gboolean
_as_load_file (const char *filename,
               GSList    **list)
{
    MooMarkupDoc *doc;
    MooMarkupNode *root, *node;

    g_return_val_if_fail (filename != NULL, FALSE);
    g_return_val_if_fail (list != NULL, FALSE);

    doc = moo_markup_parse_file (filename, NULL);
    *list = NULL;

    if (!doc)
        return FALSE;

    root = moo_markup_get_root_element (doc, AS_ROOT);

    if (!root)
        root = MOO_MARKUP_NODE (doc);

    for (node = root->children; node != NULL; node = node->next)
    {
        ASInfo *info;

        if (!MOO_MARKUP_IS_ELEMENT (node))
            continue;

        if (strcmp (node->name, AS_ELM))
        {
            g_warning ("%s: unknown element '%s'", G_STRLOC, node->name);
            continue;
        }

        info = parse_as_elm (node);

        if (info)
            *list = g_slist_prepend (*list, info);
    }

    *list = g_slist_reverse (*list);
    return TRUE;
}


char *
_as_format_xml (GSList *list)
{
    GString *xml;
    GSList *l;

    if (!list)
        return NULL;

    xml = g_string_sized_new (1024);

    g_string_append (xml, "<" AS_ROOT ">\n");

    for (l = list; l != NULL; l = l->next)
    {
        ASInfo *info = l->data;
        char *tmp;

        if (!info || !info->pattern)
        {
            g_critical ("%s: oops", G_STRLOC);
            continue;
        }

        tmp = g_markup_printf_escaped ("<" AS_ELM "pattern=\"%s\"", info->pattern);
        g_string_append (xml, tmp);
        g_free (tmp);

        if (info->lang)
        {
            tmp = g_markup_printf_escaped (" lang=\"%s\"", info->lang);
            g_string_append (xml, tmp);
            g_free (tmp);
        }

        if (info->script)
        {
            tmp = g_markup_printf_escaped (">%s</" AS_ELM ">\n", info->script);
            g_string_append (xml, tmp);
            g_free (tmp);
        }
        else
        {
            g_string_append (xml, ">\n");
        }
    }

    g_string_append (xml, "</" AS_ROOT ">\n");

    return g_string_free (xml, FALSE);
}


gboolean
_as_save (const char *filename,
          GSList     *info)
{
    char *xml;
    gboolean ret;
    GError *error = NULL;

    g_return_val_if_fail (filename != NULL, FALSE);

    xml = _as_format_xml (info);

    if (!xml)
        return moo_unlink (filename) == 0;

    ret = moo_save_file_utf8 (filename, xml, -1, &error);

    if (!ret)
    {
        g_warning ("%s: could not save file '%s'", G_STRLOC, filename);
        g_warning ("%s: %s", G_STRLOC, error->message);
        g_error_free (error);
    }

    g_free (xml);
    return ret;
}
