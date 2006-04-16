/*
 *   moocompletion.h
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

#ifndef __MOO_COMPLETION_H__
#define __MOO_COMPLETION_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS


#define MOO_TYPE_COMPLETION            (moo_completion_get_type ())
#define MOO_COMPLETION(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_COMPLETION, MooCompletion))
#define MOO_COMPLETION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_COMPLETION, MooCompletionClass))
#define MOO_IS_COMPLETION(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_COMPLETION))
#define MOO_IS_COMPLETION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_COMPLETION))
#define MOO_COMPLETION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_COMPLETION, MooCompletionClass))


typedef struct _MooCompletion         MooCompletion;
typedef struct _MooCompletionPrivate  MooCompletionPrivate;
typedef struct _MooCompletionClass    MooCompletionClass;

struct _MooCompletion
{
    GObject parent;
    MooCompletionPrivate *priv;
};

struct _MooCompletionClass
{
    GObjectClass parent_class;

    void        (*update)   (MooCompletion     *cmpl,
                             const GtkTextIter *start,
                             const GtkTextIter *end);
    gboolean    (*complete) (MooCompletion     *cmpl,
                             GtkTreeModel      *model,
                             GtkTreeIter       *iter);
};


GType           moo_completion_get_type     (void) G_GNUC_CONST;

MooCompletion  *moo_completion_new          (void);

void            moo_completion_set_doc      (MooCompletion      *cmpl,
                                             GtkTextView        *doc);
GtkTextView    *moo_completion_get_doc      (MooCompletion      *cmpl);

void            moo_completion_set_model    (MooCompletion      *cmpl,
                                             GtkTreeModel       *model,
                                             int                 text_column);
GtkTreeModel   *moo_completion_get_model    (MooCompletion      *cmpl,
                                             int                *text_column);

gboolean        moo_completion_show         (MooCompletion      *cmpl,
                                             const GtkTextIter  *start,
                                             const GtkTextIter  *end);
void            moo_completion_update       (MooCompletion      *cmpl);
void            moo_completion_complete     (MooCompletion      *cmpl);
void            moo_completion_hide         (MooCompletion      *cmpl);

gboolean        moo_completion_get_region   (MooCompletion      *cmpl,
                                             GtkTextIter        *start,
                                             GtkTextIter        *end);
void            moo_completion_set_region   (MooCompletion      *cmpl,
                                             const GtkTextIter  *start,
                                             const GtkTextIter  *end);


G_END_DECLS

#endif /* __MOO_COMPLETION_H__ */
