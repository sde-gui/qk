/*
 *   mooactiongroup.h
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

#ifndef __MOO_ACTION_GROUP_H__
#define __MOO_ACTION_GROUP_H__

#include <mooutils/mooaction.h>

G_BEGIN_DECLS


#define MOO_TYPE_ACTION_GROUP              (moo_action_group_get_type ())
#define MOO_ACTION_GROUP(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_ACTION_GROUP, MooActionGroup))
#define MOO_ACTION_GROUP_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_ACTION_GROUP, MooActionGroupClass))
#define MOO_IS_ACTION_GROUP(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_ACTION_GROUP))
#define MOO_IS_ACTION_GROUP_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_ACTION_GROUP))
#define MOO_ACTION_GROUP_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_ACTION_GROUP, MooActionGroupClass))


typedef struct _MooActionGroup        MooActionGroup;
typedef struct _MooActionGroupPrivate MooActionGroupPrivate;
typedef struct _MooActionGroupClass   MooActionGroupClass;

struct _MooActionGroup
{
    GObject                object;
    MooActionGroupPrivate *priv;
};

struct _MooActionGroupClass
{
    GObjectClass parent_class;
};


GType            moo_action_group_get_type      (void) G_GNUC_CONST;

MooActionGroup  *moo_action_group_new           (const char     *name);

MooAction       *moo_action_group_add_action    (MooActionGroup *group,
                                                 const char     *first_prop_name,
                                                 ...);

void             moo_action_group_add           (MooActionGroup *group,
                                                 MooAction      *action);
MooAction       *moo_action_group_get_action    (MooActionGroup *group,
                                                 const char     *action_id);
void             moo_action_group_remove_action (MooActionGroup *group,
                                                 const char     *action_id);

const char      *moo_action_group_get_name      (MooActionGroup *group);
void             moo_action_group_set_name      (MooActionGroup *group,
                                                 const char     *name);

typedef gboolean (*MooActionGroupForeachFunc)   (MooActionGroup *group,
                                                 MooAction      *action,
                                                 gpointer        data);

void             moo_action_group_foreach       (MooActionGroup             *group,
                                                 MooActionGroupForeachFunc   func,
                                                 gpointer                    data);

G_END_DECLS

#endif /* __MOO_ACTION_GROUP_H__ */

