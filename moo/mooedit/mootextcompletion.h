/*
 *   mootextcompletion.h
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

#ifndef __MOO_TEXT_COMPLETION_H__
#define __MOO_TEXT_COMPLETION_H__

#include "mooedit/mootextpopup.h"

G_BEGIN_DECLS


#define MOO_TYPE_TEXT_COMPLETION            (moo_text_completion_get_type ())
#define MOO_TEXT_COMPLETION(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_TEXT_COMPLETION, MooTextCompletion))
#define MOO_TEXT_COMPLETION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_TEXT_COMPLETION, MooTextCompletionClass))
#define MOO_IS_TEXT_COMPLETION(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_TEXT_COMPLETION))
#define MOO_IS_TEXT_COMPLETION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_TEXT_COMPLETION))
#define MOO_TEXT_COMPLETION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_TEXT_COMPLETION, MooTextCompletionClass))


typedef struct _MooTextCompletion         MooTextCompletion;
typedef struct _MooTextCompletionPrivate  MooTextCompletionPrivate;
typedef struct _MooTextCompletionClass    MooTextCompletionClass;

typedef gboolean (*MooTextCompletionFilterFunc) (MooTextCompletion *cmpl,
                                                 GtkTreeModel      *model,
                                                 GtkTreeIter       *iter,
                                                 gpointer           data);

struct _MooTextCompletion
{
    GObject parent;
    MooTextCompletionPrivate *priv;
};

struct _MooTextCompletionClass
{
    GObjectClass parent_class;

    void        (*populate) (MooTextCompletion  *cmpl,
                             GtkTextIter        *match_start,
                             GtkTextIter        *match_end,
                             const char         *match,
                             guint               paren);

    gboolean    (*update)   (MooTextCompletion  *cmpl);
    gboolean    (*complete) (MooTextCompletion  *cmpl,
                             GtkTreeModel       *model,
                             GtkTreeIter        *iter);
};


GType           moo_text_completion_get_type        (void) G_GNUC_CONST;

MooTextCompletion *moo_text_completion_new          (void);
MooTextCompletion *moo_text_completion_new_text     (GtkTextView        *doc,
                                                     GtkTreeModel       *model,
                                                     int                 text_column);

void            moo_text_completion_try_complete    (MooTextCompletion  *cmpl);
void            moo_text_completion_complete        (MooTextCompletion  *cmpl);
void            moo_text_completion_hide            (MooTextCompletion  *cmpl);

void            moo_text_completion_set_doc         (MooTextCompletion  *cmpl,
                                                     GtkTextView        *doc);
GtkTextView    *moo_text_completion_get_doc         (MooTextCompletion  *cmpl);

void            moo_text_completion_set_model       (MooTextCompletion  *cmpl,
                                                     GtkTreeModel       *model);
GtkTreeModel   *moo_text_completion_get_model       (MooTextCompletion  *cmpl);

void            moo_text_completion_set_filter_func (MooTextCompletion  *cmpl,
                                                     MooTextCompletionFilterFunc func,
                                                     gpointer            data,
                                                     GDestroyNotify      notify);
void            moo_text_completion_set_regex       (MooTextCompletion  *cmpl,
                                                     const char         *pattern,
                                                     const guint        *parens,
                                                     guint               n_parens);
void            moo_text_completion_set_text_column (MooTextCompletion  *cmpl,
                                                     int                 column);

MooTextPopup   *moo_text_completion_get_popup       (MooTextCompletion  *cmpl);

gboolean        moo_text_completion_get_region      (MooTextCompletion  *cmpl,
                                                     GtkTextIter        *start,
                                                     GtkTextIter        *end);
void            moo_text_completion_set_region      (MooTextCompletion  *cmpl,
                                                     const GtkTextIter  *start,
                                                     const GtkTextIter  *end);


G_END_DECLS

#endif /* __MOO_TEXT_COMPLETION_H__ */
