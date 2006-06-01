/*
 *   mooeditsession.c
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
#include "mooedit/mooeditsession-private.h"


typedef struct _SWindow SWindow;
typedef struct _SDoc SDoc;

struct _SWindow {
    GSList *docs;
    SDoc *active;
};

struct _SDoc {
    char *filename;
    char *encoding;
};

struct _MooEditSession {
    GSList *windows;    /* MooEditSessionWindow* */
};


static SWindow  *session_window_new     (void);
static SWindow  *session_window_copy    (SWindow    *win);
static void      session_window_free    (SWindow    *win);
static SDoc     *session_doc_new        (const char *filename,
                                         const char *encoding);
static SDoc     *session_doc_copy       (SDoc       *doc);
static void      session_doc_free       (SDoc       *doc);


GType
moo_edit_session_get_type (void)
{
    static GType type;

    if (G_UNLIKELY (!type))
        type = g_boxed_type_register_static ("MooEditSession",
                                             (GBoxedCopyFunc) moo_edit_session_copy,
                                             (GBoxedFreeFunc) moo_edit_session_free);

    return type;
}


MooEditSession *
_moo_edit_session_new (void)
{
    return g_new0 (MooEditSession, 1);
}


MooEditSession *
moo_edit_session_copy (MooEditSession *session)
{
    MooEditSession *copy;
    GSList *l;

    g_return_val_if_fail (session != NULL, NULL);

    copy = _moo_edit_session_new ();

    for (l = session->windows; l != NULL; l = l->next)
        copy->windows = g_slist_prepend (copy->windows,
                                         session_window_copy (l->data));
    copy->windows = g_slist_reverse (copy->windows);

    return copy;
}


void
moo_edit_session_free (MooEditSession *session)
{
    if (session)
    {
        g_slist_foreach (session->windows, (GFunc) session_window_free, NULL);
        g_slist_free (session->windows);
        g_free (session);
    }
}


static SWindow *
session_window_new (void)
{
    return g_new0 (SWindow, 1);
}


static SWindow *
session_window_copy (SWindow *win)
{
    SWindow *copy;
    GSList *l;

    g_return_val_if_fail (win != NULL, NULL);

    copy = session_window_new ();

    for (l = win->docs; l != NULL; l = l->next)
    {
        SDoc *doc_copy = session_doc_copy (l->data);
        copy->docs = g_slist_prepend (copy->docs, doc_copy);
        if (l->data == win->active)
            copy->active = doc_copy;
    }

    copy->docs = g_slist_reverse (copy->docs);

    return copy;
}


static void
session_window_free (SWindow *win)
{
    if (win)
    {
        g_slist_foreach (win->docs, (GFunc) session_doc_free, NULL);
        g_slist_free (win->docs);
        g_free (win);
    }
}


static SDoc *
session_doc_new (const char *filename,
                 const char *encoding)
{
    SDoc *doc;

    g_return_val_if_fail (filename != NULL, NULL);

    doc = g_new0 (SDoc, 1);
    doc->filename = g_strdup (filename);
    doc->encoding = g_strdup (encoding);

    return doc;
}


static SDoc *
session_doc_copy (SDoc *doc)
{
    g_return_val_if_fail (doc != NULL, NULL);
    return session_doc_new (doc->filename, doc->encoding);
}


static void
session_doc_free (SDoc *doc)
{
    if (doc)
    {
        g_free (doc->filename);
        g_free (doc->encoding);
        g_free (doc);
    }
}


MooEditSession  *moo_edit_session_parse_markup  (const char     *markup);
char            *moo_edit_session_get_markup    (MooEditSession *session);
