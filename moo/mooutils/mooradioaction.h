/*
 *   mooradioaction.h
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

#ifndef __MOO_RADIO_ACTION_H__
#define __MOO_RADIO_ACTION_H__

#include <mooutils/mootoggleaction.h>

G_BEGIN_DECLS


#define MOO_TYPE_RADIO_ACTION              (moo_radio_action_get_type ())
#define MOO_RADIO_ACTION(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_RADIO_ACTION, MooRadioAction))
#define MOO_RADIO_ACTION_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_RADIO_ACTION, MooRadioActionClass))
#define MOO_IS_RADIO_ACTION(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_RADIO_ACTION))
#define MOO_IS_RADIO_ACTION_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_RADIO_ACTION))
#define MOO_RADIO_ACTION_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_RADIO_ACTION, MooRadioActionClass))

#define MOO_TYPE_RADIO_ACTION_GROUP              (moo_radio_action_group_get_type ())
#define MOO_RADIO_ACTION_GROUP(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_RADIO_ACTION_GROUP, MooRadioActionGroup))
#define MOO_RADIO_ACTION_GROUP_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_RADIO_ACTION_GROUP, MooRadioActionGroupClass))
#define MOO_IS_RADIO_ACTION_GROUP(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_RADIO_ACTION_GROUP))
#define MOO_IS_RADIO_ACTION_GROUP_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_RADIO_ACTION_GROUP))
#define MOO_RADIO_ACTION_GROUP_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_RADIO_ACTION_GROUP, MooRadioActionGroupClass))


typedef struct _MooRadioAction           MooRadioAction;
typedef struct _MooRadioActionClass      MooRadioActionClass;
typedef struct _MooRadioActionGroup      MooRadioActionGroup;
typedef struct _MooRadioActionGroupClass MooRadioActionGroupClass;

struct _MooRadioAction
{
    MooToggleAction parent;
    MooRadioActionGroup *group;
};

struct _MooRadioActionClass
{
    MooToggleActionClass parent_class;
};

struct _MooRadioActionGroup
{
    GObject parent;
    GSList *actions;
    MooRadioAction *active;
};

struct _MooRadioActionGroupClass
{
    GObjectClass parent_class;
    void (*changed) (MooRadioActionGroup *group);
};


GType                moo_radio_action_get_type          (void) G_GNUC_CONST;
GType                moo_radio_action_group_get_type    (void) G_GNUC_CONST;

MooRadioActionGroup *moo_radio_action_group_new         (void);
MooRadioAction      *moo_radio_action_group_get_active  (MooRadioActionGroup    *group);

MooRadioAction      *moo_radio_action_new               (MooRadioActionGroup    *group);
void                 moo_radio_action_set_group         (MooRadioAction         *action,
                                                         MooRadioActionGroup    *group);
MooRadioActionGroup *moo_radio_action_get_group         (MooRadioAction         *action);


G_END_DECLS

#endif /* __MOO_RADIO_ACTION_H__ */

