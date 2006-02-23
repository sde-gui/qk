/*
 *   mootoggleaction.h
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

#ifndef __MOO_TOGGLE_ACTION_H__
#define __MOO_TOGGLE_ACTION_H__

#include <mooutils/mooaction.h>

G_BEGIN_DECLS


#define MOO_TYPE_TOGGLE_ACTION              (moo_toggle_action_get_type ())
#define MOO_TOGGLE_ACTION(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_TOGGLE_ACTION, MooToggleAction))
#define MOO_TOGGLE_ACTION_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_TOGGLE_ACTION, MooToggleActionClass))
#define MOO_IS_TOGGLE_ACTION(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_TOGGLE_ACTION))
#define MOO_IS_TOGGLE_ACTION_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_TOGGLE_ACTION))
#define MOO_TOGGLE_ACTION_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_TOGGLE_ACTION, MooToggleActionClass))


typedef struct _MooToggleAction        MooToggleAction;
typedef struct _MooToggleActionClass   MooToggleActionClass;

typedef void (*MooToggleActionFunc) (gpointer data, gboolean active);

struct _MooToggleAction
{
    MooAction           parent;
    MooToggleActionFunc callback;
    gpointer            data;
    guint               object : 1;
    guint               active : 1;
};

struct _MooToggleActionClass
{
    MooActionClass  parent_class;

    /* signals */
    void            (*toggled)  (MooToggleAction    *action,
                                 gboolean            active);
};


GType       moo_toggle_action_get_type      (void) G_GNUC_CONST;

void        moo_toggle_action_set_active    (MooToggleAction    *action,
                                             gboolean            active);


G_END_DECLS

#endif /* __MOO_TOGGLE_ACTION_H__ */

