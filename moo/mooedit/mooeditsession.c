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
#include <string.h>


typedef struct _SWindow SWindow;
typedef struct _SDoc SDoc;

struct _SWindow {
    GSList *docs;
    int active;
};

struct _SDoc {
    char *filename;
    char *encoding;
    int line;
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
        copy->docs = g_slist_prepend (copy->docs,
                                      session_doc_copy (l->data));

    copy->active = win->active;
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
    SDoc *copy;

    g_return_val_if_fail (doc != NULL, NULL);

    copy = session_doc_new (doc->filename, doc->encoding);
    copy->line = doc->line;

    return copy;
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


/* XXX docs may be saved/renamed, encoding may change between here
   and the time when they are actually closed */
void
_moo_edit_session_add_window (MooEditSession *session,
                              MooEditWindow  *window)
{
    GSList *docs;
    SWindow *swin;

    g_return_if_fail (session != NULL);
    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));

    swin = session_window_new ();
    docs = moo_edit_window_list_docs (window);

    while (docs)
    {
        MooEdit *edit;
        SDoc *sdoc;
        const char *filename;

        edit = docs->data;
        filename = moo_edit_get_filename (edit);

        if (filename)
        {
            GtkTextBuffer *buffer;
            GtkTextIter iter;

            sdoc = session_doc_new (filename, moo_edit_get_encoding (edit));
            g_return_if_fail (sdoc != NULL);

            swin->docs = g_slist_prepend (swin->docs, sdoc);

            if (edit == moo_edit_window_get_active_doc (window))
                swin->active = g_slist_length (swin->docs) - 1;

            buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (edit));
            gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                              gtk_text_buffer_get_insert (buffer));
            sdoc->line = gtk_text_iter_get_line (&iter);
        }

        docs = g_slist_delete_link (docs, docs);
    }

    swin->docs = g_slist_reverse (swin->docs);
    session->windows = g_slist_append (session->windows, swin);
}


void
_moo_edit_session_load (MooEditSession *session,
                        MooEditor      *editor,
                        MooEditWindow  *window)
{
    GSList *wl;
    gboolean silent;

    g_return_if_fail (session != NULL);
    g_return_if_fail (MOO_IS_EDITOR (editor));

    g_object_get (editor, "silent", &silent, NULL);
    g_object_set (editor, "silent", TRUE, NULL);

    for (wl = session->windows; wl != NULL; wl = wl->next)
    {
        GSList *dl;
        SWindow *swin = wl->data;

        if (!window)
        {
            window = moo_editor_new_window (editor);
            g_return_if_fail (window != NULL);
        }

        for (dl = swin->docs; dl != NULL; dl = dl->next)
        {
            SDoc *sdoc = dl->data;
            MooEdit *edit;

            edit = moo_editor_open_file (editor, window, NULL,
                                         sdoc->filename,
                                         sdoc->encoding);

            if (!edit)
            {
                g_warning ("%s: could not open file '%s'",
                           G_STRLOC, sdoc->filename);
                continue;
            }

            if (sdoc->line >= 0)
                moo_text_view_move_cursor (MOO_TEXT_VIEW (edit), sdoc->line,
                                           0, FALSE, FALSE);
        }

        if (swin->active >= 0 && swin->active < (int) moo_edit_window_num_docs (window))
        {
            MooEdit *edit = moo_edit_window_get_nth_doc (window, swin->active);
            moo_edit_window_set_active_doc (window, edit);
        }

        window = NULL;
    }

    g_object_set (editor, "silent", silent, NULL);
}


/*****************************************************************************/
/* Saving/loading
 */

MooEditSession *
moo_edit_session_parse_markup (const char *markup)
{
    MooMarkupDoc *doc;
    MooMarkupNode *root;
    MooEditSession *session;
    GError *error = NULL;

    g_return_val_if_fail (markup != NULL, NULL);

    doc = moo_markup_parse_memory (markup, -1, &error);

    if (!doc)
    {
        g_warning ("%s: %s", G_STRLOC, error->message);
        g_error_free (error);
        return NULL;
    }

    root = moo_markup_get_root_element (doc, MOO_EDIT_SESSION_ELM_SESSION);

    if (!root)
    {
        g_warning ("%s: no %s element", G_STRLOC, MOO_EDIT_SESSION_ELM_SESSION);
        moo_markup_doc_unref (doc);
        return NULL;
    }

    session = moo_edit_session_parse_node (MOO_MARKUP_NODE (root));
    moo_markup_doc_unref (doc);
    return session;
}


static gboolean
parse_doc_node (SWindow       *window,
                MooMarkupNode *node)
{
    char *filename;
    const char *encoding;
    SDoc *doc;

    g_assert (window != NULL);
    g_assert (MOO_MARKUP_IS_ELEMENT (node));
    g_assert (!strcmp (node->name, MOO_EDIT_SESSION_ELM_DOC));

    filename = moo_markup_get_file_content (node);

    if (!filename)
    {
        g_warning ("%s: could not get filename from '%s' element",
                   G_STRLOC, node->name);
        return FALSE;
    }

    encoding = moo_markup_get_prop (node, MOO_EDIT_SESSION_PROP_ENCODING);

    encoding = encoding && encoding[0] ? encoding : NULL;
    doc = session_doc_new (filename, encoding);
    doc->line = moo_markup_get_int_prop (node, MOO_EDIT_SESSION_PROP_LINE, -1);

    window->docs = g_slist_prepend (window->docs, doc);

    g_free (filename);
    return TRUE;
}


MooEditSession *
moo_edit_session_parse_node (MooMarkupNode *root)
{
    MooMarkupNode *window_node;
    MooEditSession *session = NULL;
    SWindow *window = NULL;

    g_return_val_if_fail (MOO_MARKUP_IS_ELEMENT (root), NULL);

    session = _moo_edit_session_new ();

    for (window_node = root->children; window_node != NULL; window_node = window_node->next)
    {
        MooMarkupNode *child;

        if (!MOO_MARKUP_IS_ELEMENT (window_node))
            continue;

        if (strcmp (window_node->name, MOO_EDIT_SESSION_ELM_WINDOW))
        {
            g_warning ("%s: unexpected element '%s' under %s node",
                       G_STRLOC, window_node->name, root->name);
            goto error;
        }

        window = session_window_new ();
        window->active = moo_markup_get_int_prop (window_node, MOO_EDIT_SESSION_PROP_ACTIVE_DOC, -1);

        for (child = window_node->children; child != NULL; child = child->next)
        {
            if (!MOO_MARKUP_IS_ELEMENT (child))
                continue;

            if (!strcmp (child->name, MOO_EDIT_SESSION_ELM_DOC))
            {
                if (!parse_doc_node (window, child))
                    goto error;
            }
            else
            {
                g_warning ("%s: unexpected element '%s' under %s node",
                           G_STRLOC, child->name, window_node->name);
                goto error;
            }
        }

        window->docs = g_slist_reverse (window->docs);

        if (window->active >= 0 && window->active >= (int) g_slist_length (window->docs))
        {
            g_warning ("%s: invalid active document index %d", G_STRLOC, window->active);
            window->active = -1;
        }

        session->windows = g_slist_prepend (session->windows, window);
        window = NULL;
    }

    session->windows = g_slist_reverse (session->windows);
    return session;

error:
    moo_edit_session_free (session);
    session_window_free (window);
    return NULL;
}


char *
moo_edit_session_get_markup (MooEditSession *session)
{
    MooMarkupDoc *doc;
    MooMarkupNode *root;
    GSList *wl;
    char *string;

    g_return_val_if_fail (session != NULL, NULL);

    doc = moo_markup_doc_new ("MooEditSession");

    root = moo_markup_create_root_element (doc, MOO_EDIT_SESSION_ELM_SESSION);

    for (wl = session->windows; wl != NULL; wl = wl->next)
    {
        GSList *dl;
        MooMarkupNode *window_node;
        SWindow *window = wl->data;

        window_node = moo_markup_create_element (root, MOO_EDIT_SESSION_ELM_WINDOW);

        if (window->active >= 0)
            moo_markup_set_int_prop (window_node,
                                     MOO_EDIT_SESSION_PROP_ACTIVE_DOC,
                                     window->active);

        for (dl = window->docs; dl != NULL; dl = dl->next)
        {
            MooMarkupNode *doc_node;
            SDoc *doc = dl->data;

            doc_node = moo_markup_create_file_element (window_node,
                                                       MOO_EDIT_SESSION_ELM_DOC,
                                                       doc->filename);

            /* XXX */
            if (doc->encoding && doc->encoding[0] &&
                g_ascii_strcasecmp (doc->encoding, "utf8") &&
                g_ascii_strcasecmp (doc->encoding, "utf-8"))
                    moo_markup_set_prop (doc_node,
                                         MOO_EDIT_SESSION_PROP_ENCODING,
                                         doc->encoding);

            if (doc->line > 0)
                moo_markup_set_int_prop (doc_node,
                                         MOO_EDIT_SESSION_PROP_LINE,
                                         doc->line);
        }
    }

    string = moo_markup_node_get_pretty_string (MOO_MARKUP_NODE (doc), 2);
    moo_markup_doc_unref (doc);
    return string;
}
