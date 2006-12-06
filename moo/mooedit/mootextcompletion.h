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
#include <gtk/gtkliststore.h>

G_BEGIN_DECLS


#define MOO_TYPE_TEXT_COMPLETION             (moo_text_completion_get_type ())
#define MOO_TEXT_COMPLETION(object)          (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_TEXT_COMPLETION, MooTextCompletion))
#define MOO_TEXT_COMPLETION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_TEXT_COMPLETION, MooTextCompletionClass))
#define MOO_IS_TEXT_COMPLETION(object)       (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_TEXT_COMPLETION))
#define MOO_IS_TEXT_COMPLETION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_TEXT_COMPLETION))
#define MOO_TEXT_COMPLETION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_TEXT_COMPLETION, MooTextCompletionClass))


typedef struct _MooTextCompletion         MooTextCompletion;
typedef struct _MooTextCompletionPrivate  MooTextCompletionPrivate;
typedef struct _MooTextCompletionClass    MooTextCompletionClass;

struct _MooTextCompletion
{
    GObject parent;
    MooTextCompletionPrivate *priv;
};

struct _MooTextCompletionClass
{
    GObjectClass parent_class;

    void  (*try_complete)   (MooTextCompletion  *cmpl,
                             gboolean            insert_unique);
    void  (*finish)         (MooTextCompletion  *cmpl);
    void  (*update)         (MooTextCompletion  *cmpl);

    void  (*complete)       (MooTextCompletion  *cmpl,
                             GtkTreeModel       *model,
                             GtkTreeIter        *iter);
    void  (*populate)       (MooTextCompletion  *cmpl,
                             GtkTreeModel       *model,
                             GtkTextIter        *cursor,
                             const char         *text);
};

typedef char *(*MooTextCompletionTextFunc) (GtkTreeModel *model,
                                            GtkTreeIter  *iter,
                                            gpointer      data);


GType           moo_text_completion_get_type        (void) G_GNUC_CONST;

void            moo_text_completion_try_complete    (MooTextCompletion  *cmpl,
                                                     gboolean            insert_unique);
void            moo_text_completion_hide            (MooTextCompletion  *cmpl);
void            moo_text_completion_show            (MooTextCompletion  *cmpl);

void            moo_text_completion_set_doc         (MooTextCompletion  *cmpl,
                                                     GtkTextView        *doc);
GtkTextView    *moo_text_completion_get_doc         (MooTextCompletion  *cmpl);
GtkTextBuffer  *moo_text_completion_get_buffer      (MooTextCompletion  *cmpl);

void            moo_text_completion_set_region      (MooTextCompletion  *cmpl,
                                                     const GtkTextIter  *start,
                                                     const GtkTextIter  *end);
gboolean        moo_text_completion_get_region      (MooTextCompletion  *cmpl,
                                                     GtkTextIter        *start,
                                                     GtkTextIter        *end);

void            moo_text_completion_set_model       (MooTextCompletion  *cmpl,
                                                     GtkTreeModel       *model);
GtkTreeModel   *moo_text_completion_get_model       (MooTextCompletion  *cmpl);
void            moo_text_completion_set_text_column (MooTextCompletion  *cmpl,
                                                     int                 column);
void            moo_text_completion_set_text_func   (MooTextCompletion  *cmpl,
                                                     MooTextCompletionTextFunc func,
                                                     gpointer            data,
                                                     GDestroyNotify      notify);

MooTextPopup   *moo_text_completion_get_popup       (MooTextCompletion  *cmpl);


G_END_DECLS

#endif /* __MOO_TEXT_COMPLETION_H__ */
