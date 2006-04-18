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

#include "mooedit/mootextpopup.h"
#include <gtk/gtkliststore.h>

G_BEGIN_DECLS

#define MOO_TYPE_COMPLETION_GROUP       (moo_completion_group_get_type ())
#define MOO_TYPE_COMPLETION             (moo_completion_get_type ())
#define MOO_COMPLETION(object)          (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_COMPLETION, MooCompletion))
#define MOO_COMPLETION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_COMPLETION, MooCompletionClass))
#define MOO_IS_COMPLETION(object)       (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_COMPLETION))
#define MOO_IS_COMPLETION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_COMPLETION))
#define MOO_COMPLETION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_COMPLETION, MooCompletionClass))


typedef struct _MooCompletion         MooCompletion;
typedef struct _MooCompletionPrivate  MooCompletionPrivate;
typedef struct _MooCompletionClass    MooCompletionClass;
typedef struct _MooCompletionGroup    MooCompletionGroup;

struct _MooCompletion
{
    GObject parent;
    MooCompletionPrivate *priv;
};

struct _MooCompletionClass
{
    GObjectClass parent_class;
};


/* same as GCompletionFunc - must not allocate a new string;
   but, it's safe to return static string and rewrite it on each call */
typedef char* (*MooCompletionStringFunc)    (gpointer       data);
typedef void  (*MooCompletionFreeFunc)      (gpointer       data);
typedef int   (*MooCompletionCmpFunc)       (gpointer       data1,
                                             gpointer       data2);


GType           moo_completion_get_type             (void) G_GNUC_CONST;
GType           moo_completion_group_get_type       (void) G_GNUC_CONST;

MooCompletionGroup *moo_completion_group_new_text   (gboolean            sorted);
MooCompletionGroup *moo_completion_group_new        (MooCompletionStringFunc string_func,
                                                     MooCompletionFreeFunc free_func,
                                                     MooCompletionCmpFunc cmp_func);
/* these two steal data */
void            moo_completion_group_set_data       (MooCompletionGroup *group,
                                                     GList              *data);
void            moo_completion_group_add_data       (MooCompletionGroup *group,
                                                     GList              *data);

void            moo_completion_group_set_pattern    (MooCompletionGroup *group,
                                                     const char         *pattern,
                                                     const guint        *parens,
                                                     guint               n_parens);

MooCompletionGroup *moo_completion_group_ref        (MooCompletionGroup *group);
void            moo_completion_group_unref          (MooCompletionGroup *group);

MooCompletion  *moo_completion_new                  (void);
/* steals words */
MooCompletion  *moo_completion_new_text             (GList              *words,
                                                     gboolean            sorted);

void            moo_completion_try_complete         (MooCompletion      *cmpl,
                                                     gboolean            insert_unique);
void            moo_completion_hide                 (MooCompletion      *cmpl);

void            moo_completion_set_doc              (MooCompletion      *cmpl,
                                                     GtkTextView        *doc);
GtkTextView    *moo_completion_get_doc              (MooCompletion      *cmpl);

GtkListStore   *moo_completion_get_store            (MooCompletion      *cmpl);

void            moo_completion_add_group            (MooCompletion      *cmpl,
                                                     MooCompletionGroup *group);

MooTextPopup   *moo_completion_get_popup            (MooCompletion      *cmpl);

gboolean        moo_completion_get_region           (MooCompletion      *cmpl,
                                                     GtkTextIter        *start,
                                                     GtkTextIter        *end);
void            moo_completion_set_region           (MooCompletion      *cmpl,
                                                     const GtkTextIter  *start,
                                                     const GtkTextIter  *end);


G_END_DECLS

#endif /* __MOO_COMPLETION_H__ */
