/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   moofind.c
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

typedef struct _AS AS;
typedef struct _ASString ASString;
typedef struct _ASRegex ASRegex;
typedef struct _ASSet ASSet;
typedef struct _ASMatch ASMatch;

typedef gboolean (*ASFunc) (AS                *as,
                            GtkTextIter       *match_start,
                            GtkTextIter       *match_end,
                            gpointer           data);

typedef enum {
    AS_STRING = 0,
    AS_REGEX  = 1
} ASType;

struct _ASRegex {
    EggRegex *regex;
};

struct _ASString {
    char *string;
    guint len;
    guint char_len;
    gunichar last_char;
};

struct _AS {
    ASFunc callback;
    gpointer data;
    GDestroyNotify destroy;
    union {
        ASRegex regex;
        ASString str;
    };
    guint16 ref_count;
    ASType type;
};

struct _ASSet {
    GSList *list;
};

typedef struct {
    MooPlugin parent;
    ASSet *set;
} ASPlugin;


static gboolean as_plugin_init      (ASPlugin       *plugin);
static void     as_plugin_deinit    (ASPlugin       *plugin);
static void     as_plugin_attach    (ASPlugin       *plugin,
                                     MooEdit        *doc);
static void     as_plugin_detach    (ASPlugin       *plugin,
                                     MooEdit        *doc);

static ASSet   *as_set_new          (void);
static ASSet   *as_set_copy         (ASSet          *set);
static void     as_set_free         (ASSet          *set);

static void     as_set_add          (ASSet          *set,
                                     AS             *as);

static AS      *as_ref              (AS             *as);
static void     as_unref            (AS             *as);
static AS      *as_new__            (ASType          type,
                                     ASFunc          callback,
                                     gpointer        data,
                                     GDestroyNotify  destroy);
static AS      *as_new_string       (const char     *string,
                                     ASFunc          callback,
                                     gpointer        data,
                                     GDestroyNotify  destroy);

static gboolean char_inserted_cb    (GtkTextView    *view,
                                     GtkTextIter    *where,
                                     guint           character,
                                     ASSet          *set);

static GQuark   as_quark            (void);


static GQuark
as_quark (void)
{
    static GQuark q = 0;
    if (!q)
        q = g_quark_from_static_string ("moo-active-strings");
    return q;
}


static gboolean
magic_cb (G_GNUC_UNUSED AS *as,
          GtkTextIter *start,
          GtkTextIter *end,
          G_GNUC_UNUSED gpointer data)
{
    GtkTextBuffer *buffer = gtk_text_iter_get_buffer (start);
    gtk_text_buffer_begin_user_action (buffer);
    gtk_text_buffer_delete (buffer, start, end);
    gtk_text_buffer_insert (buffer, start, "REAL MAGIC", -1);
    gtk_text_buffer_end_user_action (buffer);
    return TRUE;
}


static gboolean
as_plugin_init (ASPlugin   *plugin)
{
    AS *as;

    plugin->set = as_set_new ();

    as = as_new_string ("MAGIC", magic_cb, NULL, NULL);
    as_set_add (plugin->set, as);
    as_unref (as);

    return TRUE;
}


static void
as_plugin_deinit (ASPlugin   *plugin)
{
    as_set_free (plugin->set);
    plugin->set = NULL;
}


static void
as_plugin_attach (ASPlugin   *plugin,
                  MooEdit    *doc)
{
    ASSet *set = as_set_copy (plugin->set);
    g_object_set_qdata (G_OBJECT (doc), as_quark(), set);
    g_signal_connect (doc, "char-inserted",
                      G_CALLBACK (char_inserted_cb), set);
}


static void
as_plugin_detach (G_GNUC_UNUSED ASPlugin *plugin,
                  MooEdit    *doc)
{
    ASSet *set = g_object_get_qdata (G_OBJECT (doc), as_quark());
    g_object_set_qdata (G_OBJECT (doc), as_quark(), NULL);
    g_signal_handlers_disconnect_by_func (doc, (gpointer) char_inserted_cb, set);
    as_set_free (set);
}


static gboolean
iter_backward_chars (GtkTextIter *iter,
                     guint        how_many)
{
    guint i;
    g_return_val_if_fail (iter != NULL, FALSE);
    g_return_val_if_fail (how_many, FALSE);
    for (i = 0; i < how_many; ++i)
        if (!gtk_text_iter_backward_char (iter))
            return FALSE;
    return TRUE;
}


static gboolean
char_inserted_cb (GtkTextView    *view,
                  GtkTextIter    *where,
                  guint           character,
                  ASSet          *set)
{
    GSList *l;
    GtkTextIter iter;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (view);

    g_return_val_if_fail (set != NULL, FALSE);

    if (!set->list)
        return FALSE;

    for (l = set->list; l != NULL; l = l->next)
    {
        AS *as = l->data;
        char *text;

        if (as->type == AS_REGEX)
        {
            g_warning ("%s: implement me", G_STRLOC);
            continue;
        }

        if (as->str.last_char != character)
            continue;

        iter = *where;
        if (!iter_backward_chars (&iter, as->str.char_len))
            continue;

        text = gtk_text_buffer_get_slice (buffer, &iter, where, TRUE);

        if (!strcmp (text, as->str.string))
        {
            gboolean result = FALSE;

            if (as->callback)
                result = as->callback (as, &iter, where, as->data);

            g_free (text);
            return result;
        }

        g_free (text);
    }

    return FALSE;
}


static ASSet*
as_set_new (void)
{
    return g_new0 (ASSet, 1);
}


static ASSet*
as_set_copy (ASSet          *set)
{
    ASSet *copy;

    g_return_val_if_fail (set != NULL, NULL);

    copy = g_new0 (ASSet, 1);
    copy->list = g_slist_copy (set->list);
    g_slist_foreach (copy->list, (GFunc) as_ref, NULL);

    return copy;
}


static void
as_set_free (ASSet          *set)
{
    if (set)
    {
        g_slist_foreach (set->list, (GFunc) as_unref, NULL);
        g_slist_free (set->list);
        g_free (set);
    }
}


static void
as_set_add (ASSet          *set,
            AS             *as)
{
    g_return_if_fail (set && as);
    set->list = g_slist_append (set->list, as_ref (as));
}


static AS*
as_ref (AS *as)
{
    g_return_val_if_fail (as != NULL, NULL);
    as->ref_count++;
    return as;
}


static void
as_unref (AS *as)
{
    if (as && !(--as->ref_count))
    {
        switch (as->type)
        {
            case AS_REGEX:
                g_warning ("%s: implement me", G_STRLOC);
                break;

            case AS_STRING:
                g_free (as->str.string);
                break;
        }

        if (as->destroy)
            as->destroy (as->data);

        g_free (as);
    }
}


static AS*
as_new__  (ASType         type,
           ASFunc         callback,
           gpointer       data,
           GDestroyNotify destroy)
{
    AS *as;

    switch (type)
    {
        case AS_REGEX:
        case AS_STRING:
            break;
        default:
            g_return_val_if_reached (NULL);
    }

    as = g_new0 (AS, 1);
    as->ref_count = 1;
    as->type = type;
    as->callback = callback;
    as->data = data;
    as->destroy = destroy;

    return as;
}


static AS*
as_new_string (const char     *string,
               ASFunc          callback,
               gpointer        data,
               GDestroyNotify  destroy)
{
    AS *as;

    g_return_val_if_fail (string && string[0], NULL);
    g_return_val_if_fail (g_utf8_validate (string, -1, NULL), NULL);

    as = as_new__ (AS_STRING, callback, data, destroy);
    g_return_val_if_fail (as != NULL, NULL);

    as->str.string = g_strdup (string);
    as->str.len = strlen (string);
    as->str.char_len = g_utf8_strlen (string, -1);
    g_assert (as->str.char_len != 0);

    as->str.last_char = g_utf8_get_char (g_utf8_offset_to_pointer (string, as->str.char_len - 1));

    return as;
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
