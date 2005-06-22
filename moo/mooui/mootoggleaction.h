/*
 *   mooui/mootoggleaction.h
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

#ifndef MOOUI_MOOTOGGLEACTION_H
#define MOOUI_MOOTOGGLEACTION_H

#include "mooui/mooaction.h"

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

    guint               active : 1;
    MooToggleActionFunc toggled_callback;
    gpointer            toggled_data;
};

struct _MooToggleActionClass
{
    MooActionClass  parent_class;

    /* signals */
    void            (*toggled)  (MooToggleAction    *action,
                                 gboolean            active);
};


GType            moo_toggle_action_get_type     (void) G_GNUC_CONST;

MooToggleAction *moo_toggle_action_new          (const char         *id,
                                                 const char         *label,
                                                 const char         *tooltip,
                                                 const char         *accel,
                                                 MooToggleActionFunc func,
                                                 gpointer            data);
MooToggleAction *moo_toggle_action_new_stock    (const char         *id,
                                                 const char         *stock_id,
                                                 MooToggleActionFunc func,
                                                 gpointer            data);

void             moo_toggle_action_set_active   (MooToggleAction    *action,
                                                 gboolean            active);


G_END_DECLS

#endif /* MOOUI_MOOTOGGLEACTION_H */

