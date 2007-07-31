/* -*- objc -*-
 *   mooindenter-settings.m
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#define MOOEDIT_COMPILATION
#include "mooedit/mooindenter-regex.h"
#include "mooutils/moocobject.h"
#include "mooutils/moomarkup.h"
#include "mooutils/mooutils-misc.h"
#include <glib/gregex.h>


static GHashTable *regex_hash;


@protocol MooIndenterElm
- (BOOL) newline: (MooIndenter*)indenter
            text: (const char*)text
          cursor: (GtkTextIter*)cursor
           start: (GtkTextIter*)start
             end: (GtkTextIter*)end;
@end

typedef MooCObject<MooIndenterElm> MooIndenterElm;

@interface MooIndenterShift : MooCObject <MooIndenterElm>
{
    GRegex *re;
}

- init: (const char*)pattern;
@end;

@interface MooIndenterRegex : MooCObject
{
    char *id_;
    GRegex *re_all;
    guint n_elms;
    MooIndenterElm **elms;
}

- initWithMarkup: (MooMarkupNode*)node
         lang_id: (const char*)lang_id
        filename: (const char*)filename;

- (BOOL) newline: (MooIndenter*)indenter
           where: (GtkTextIter*)where;

+ (void) loadSettings;
@end


@implementation MooIndenterRegex

+ (void) loadSettings
{
    char *filename = NULL;
    MooMarkupDoc *doc;
    MooMarkupNode *root, *node;
    const char *version;
    GError *error = NULL;
    MooIndenterRegex *re;

    if (regex_hash)
        return;

    regex_hash = g_hash_table_new_full (g_str_hash, g_str_equal, g_free,
                                        (GDestroyNotify) _moo_indenter_regex_unref);

    filename = moo_get_data_file ("indent.xml");

    if (!filename)
        return;

    doc = moo_markup_parse_file (filename, &error);

    if (!doc)
    {
        g_warning ("%s: %s", G_STRLOC, error->message);
        g_free (error);
        goto out;
    }

    if (!(root = moo_markup_get_root_element (doc, "indent-settings")))
    {
        g_warning ("%s: 'indent-settings' element missing", G_STRLOC);
        goto out;
    }

    version = moo_markup_get_prop (root, "version");
    if (!version || strcmp (version, "1") != 0)
    {
        g_warning ("%s: invalid version '%s'", G_STRLOC,
                   version ? version : "(null)");
        goto out;
    }

    for (node = root->children; node != NULL; node = node->next)
    {
        const char *lang_id;

        if (!MOO_MARKUP_IS_ELEMENT (node))
            continue;

        if (strcmp (node->name, "lang") != 0)
        {
            g_warning ("%s: invalid element '%s'", G_STRLOC, node->name);
            goto out;
        }

        lang_id = moo_markup_get_prop (node, "id");
        if (!lang_id)
        {
            g_warning ("%s: 'id' attribute missing in file '%s'", G_STRLOC, filename);
            goto out;
        }

        re = [[self alloc] initWithMarkup:node lang_id:lang_id filename:filename];

        if (!re)
            goto out;

        g_hash_table_insert (regex_hash, g_strdup (re->id_), re);
    }

out:
    g_free (filename);
    if (doc)
        moo_markup_doc_unref (doc);
}


- initWithMarkup: (MooMarkupNode*)elm
         lang_id: (const char*)lang_id
        filename: (const char*)filename
{
    MooMarkupNode *node;
    GSList *elements = NULL;
    GString *pattern_all;
    GError *error = NULL;
    guint i;

    [super init];

    id_ = g_strdup (lang_id);
    pattern_all = NULL;

    for (node = elm->children; node != NULL; node = node->next)
    {
        if (!MOO_MARKUP_IS_ELEMENT (node))
            continue;

        if (!strcmp (node->name, "shift"))
        {
            const char *pattern = moo_markup_get_prop (node, "pattern");
            MooIndenterElm *elm = [[MooIndenterShift alloc] init:pattern];

            if (!elm)
                goto error;

            elements = g_slist_prepend (elements, elm);
            if (!pattern_all)
            {
                pattern_all = g_string_new ("(");
                g_string_append (pattern_all, pattern);
            }
            else
            {
                g_string_append_printf (pattern_all, "|%s", pattern);
            }
        }
        else
        {
            g_warning ("in file %s: invalid element '%s' in lang '%s'",
                       filename, node->name, lang_id);
            goto error;
        }
    }

    if (!elements)
    {
        g_warning ("in file %s: empty lang '%s' node",
                   filename, lang_id);
        goto error;
    }

    g_string_append (pattern_all, ")");
    re_all = g_regex_new (pattern_all->str, G_REGEX_DUPNAMES | G_REGEX_OPTIMIZE,
                          0, &error);
    if (!re_all)
    {
        g_warning ("in file %s: could not compile resulting regex '%s': %s",
                   filename, pattern_all->str, error->message);
        g_error_free (error);
        goto error;
    }

    elements = g_slist_reverse (elements);
    n_elms = g_slist_length (elements);
    elms = g_new (MooIndenterElm*, n_elms);
    for (i = 0; i < n_elms; ++i)
    {
        elms[i] = elements->data;
        elements = g_slist_delete_link (elements, elements);
    }

    g_string_free (pattern_all, TRUE);
    return self;

error:
    while (elements)
    {
        [(id) elements->data release];
        elements = g_slist_delete_link (elements, elements);
    }

    g_string_free (pattern_all, TRUE);
    [self release];
    return nil;
}

- (void) dealloc
{
    guint i;

    g_free (id_);

    if (re_all)
        g_regex_unref (re_all);

    for (i = 0; i < n_elms; ++i)
        [elms[i] release];
    g_free (elms);

    [super dealloc];
}


- (BOOL) newline: (MooIndenter*)indenter
           where: (GtkTextIter*)where
{
    guint i;
    GtkTextIter start, end;
    char *text;
    BOOL handled;

    start = *where;
    gtk_text_iter_backward_line (&start);

    if (gtk_text_iter_ends_line (&start))
        return NO;

    end = start;
    gtk_text_iter_forward_to_line_end (&end);
    text = gtk_text_iter_get_slice (&start, &end);

    if (!g_regex_match (re_all, text, 0, NULL))
    {
        g_free (text);
        return NO;
    }

    for (i = 0, handled = NO; !handled && i < n_elms; ++i)
        handled = [elms[i] newline:indenter text:text
                            cursor:where start:&start end:&end];

    g_free (text);
    return handled;
}

@end


MooIndenterRegex *
_moo_indenter_regex_ref (MooIndenterRegex *regex)
{
    g_return_val_if_fail (regex != nil, NULL);
    return [regex retain];
}

void
_moo_indenter_regex_unref (MooIndenterRegex *regex)
{
    g_return_if_fail (regex != nil);
    [regex release];
}

MooIndenterRegex *
_moo_indenter_get_regex (const char *id_)
{
    g_return_val_if_fail (id_ != NULL, nil);
    [MooIndenterRegex loadSettings];
    return g_hash_table_lookup (regex_hash, id_);
}

gboolean
_moo_indenter_regex_newline (MooIndenterRegex *regex,
                             MooIndenter      *indenter,
                             GtkTextIter      *where)
{
    g_return_val_if_fail (regex != NULL, FALSE);
    return [regex newline:indenter where:where];
}


@implementation MooIndenterShift

- init: (const char*)pattern
{
    GError *error = NULL;

    [super init];

    re = g_regex_new (pattern, G_REGEX_DUPNAMES | G_REGEX_OPTIMIZE, 0, &error);
    if (!re)
    {
        g_warning ("%s: %s", G_STRFUNC, error->message);
        g_error_free (error);
        [self release];
        return nil;
    }

    return self;
}

- (void) dealloc
{
    if (re)
        g_regex_unref (re);
    [super dealloc];
}

- (BOOL) newline: (MooIndenter*)indenter
            text: (const char*)text
          cursor: (GtkTextIter*)cursor
           start: (GtkTextIter*)start
             end: (GtkTextIter*)end
{
    GtkTextIter iter;
    guint offset;
    char *insert;
    MOO_UNUSED_VAR (end);

    if (!g_regex_match (re, text, 0, NULL))
        return NO;

    iter = *start;
    if (!_moo_indenter_compute_line_offset (indenter, &iter, &offset))
    {
        g_critical ("%s: oops", G_STRFUNC);
        return NO;
    }

    offset = _moo_indenter_compute_next_offset (indenter, offset);
    insert = moo_indenter_make_space (indenter, offset, 0);
    gtk_text_buffer_insert (gtk_text_iter_get_buffer (start), cursor, insert, -1);
    g_free (insert);

    return YES;
}

@end;
