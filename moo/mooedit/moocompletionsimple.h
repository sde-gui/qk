/*
 *   moocompletionsimple.h
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

#ifndef MOO_COMPLETION_SIMPLE_H
#define MOO_COMPLETION_SIMPLE_H

#include "mooedit/mootextcompletion.h"
#include <gtk/gtkliststore.h>

G_BEGIN_DECLS


#define MOO_TYPE_COMPLETION_SIMPLE             (moo_completion_simple_get_type ())
#define MOO_COMPLETION_SIMPLE(object)          (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_COMPLETION_SIMPLE, MooCompletionSimple))
#define MOO_COMPLETION_SIMPLE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_COMPLETION_SIMPLE, MooCompletionSimpleClass))
#define MOO_IS_COMPLETION_SIMPLE(object)       (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_COMPLETION_SIMPLE))
#define MOO_IS_COMPLETION_SIMPLE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_COMPLETION_SIMPLE))
#define MOO_COMPLETION_SIMPLE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_COMPLETION_SIMPLE, MooCompletionSimpleClass))


typedef struct _MooCompletionSimple         MooCompletionSimple;
typedef struct _MooCompletionSimplePrivate  MooCompletionSimplePrivate;
typedef struct _MooCompletionSimpleClass    MooCompletionSimpleClass;
typedef struct _MooCompletionGroup    MooCompletionGroup;

struct _MooCompletionSimple
{
    MooTextCompletion base;
    MooCompletionSimplePrivate *priv;
};

struct _MooCompletionSimpleClass
{
    MooTextCompletionClass base_class;
};


GType           moo_completion_simple_get_type          (void) G_GNUC_CONST;

/* steals data */
void            moo_completion_group_add_data           (MooCompletionGroup     *group,
                                                         GList                  *data);

void            moo_completion_group_set_pattern        (MooCompletionGroup     *group,
                                                         const char             *pattern,
                                                         const guint            *parens,
                                                         guint                   n_parens);
void            moo_completion_group_set_suffix         (MooCompletionGroup     *group,
                                                         const char             *suffix);
void            moo_completion_group_set_script         (MooCompletionGroup     *group,
                                                         const char             *script);

/* steals words */
MooTextCompletion *moo_completion_simple_new_text       (GList                  *words);

MooCompletionGroup *moo_completion_simple_new_group     (MooCompletionSimple    *cmpl,
                                                         const char             *name);


G_END_DECLS

#endif /* MOO_COMPLETION_SIMPLE_H */
