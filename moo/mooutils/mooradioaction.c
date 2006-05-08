/*
 *   mooradioaction.c
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

#include "mooutils/mooradioaction.h"
#include "mooutils/mooactiongroup.h"
#include "mooutils/moocompat.h"
#include "mooutils/moomarshals.h"
#include "mooutils/mooutils-gobject.h"
#include "mooutils/mooutils-misc.h"
#include <gtk/gtk.h>
#include <string.h>


static void         moo_radio_action_finalize      (GObject            *object);

static GtkWidget   *create_menu_item                (MooAction          *action);
static void         moo_radio_action_toggled        (MooToggleAction    *action,
                                                     gboolean            active);

static void         moo_radio_action_group_add              (MooRadioActionGroup *group,
                                                             MooRadioAction      *action);
static void         moo_radio_action_group_remove           (MooRadioActionGroup *group,
                                                             MooRadioAction      *action);
static void         moo_radio_action_group_active_changed   (MooRadioActionGroup *group,
                                                             MooRadioAction      *active);


/* MOO_TYPE_RADIO_ACTION */
G_DEFINE_TYPE (MooRadioAction, moo_radio_action, MOO_TYPE_TOGGLE_ACTION)
/* MOO_TYPE_RADIO_ACTION_GROUP */
G_DEFINE_TYPE (MooRadioActionGroup, moo_radio_action_group, G_TYPE_OBJECT)


static void
moo_radio_action_class_init (MooRadioActionClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    MooActionClass *action_class = MOO_ACTION_CLASS (klass);
    MooToggleActionClass *toggle_class = MOO_TOGGLE_ACTION_CLASS (klass);

    gobject_class->finalize = moo_radio_action_finalize;
    action_class->create_menu_item = create_menu_item;
    toggle_class->toggled = moo_radio_action_toggled;
}


static void
moo_radio_action_init (MooRadioAction *action)
{
    MooRadioActionGroup *group;
    action->group = moo_radio_action_group_new ();
    moo_radio_action_group_add (group, action);
}


static void
moo_radio_action_finalize (GObject *object)
{
    MooRadioAction *action = MOO_RADIO_ACTION (object);

    if (action->group)
    {
        moo_radio_action_group_remove (action->group, action);
        g_object_unref (action->group);
    }

    G_OBJECT_CLASS(moo_radio_action_parent_class)->finalize (object);
}


static void
moo_radio_action_toggled (MooToggleAction *action,
                          gboolean         active)
{
    MOO_TOGGLE_ACTION_CLASS(moo_radio_action_parent_class)->toggled (action, active);

    if (action->active)
        moo_radio_action_group_active_changed (MOO_RADIO_ACTION(action)->group,
                                               MOO_RADIO_ACTION(action));
}


static GtkWidget*
create_menu_item (MooAction *action)
{
    GtkWidget *item = MOO_ACTION_CLASS(moo_radio_action_parent_class)->create_menu_item (action);
    g_return_val_if_fail (item != NULL, NULL);
    g_object_set (item, "draw-as-radio", TRUE, NULL);
    return item;
}


/************************************************************************/
/* MooRadioActionGroup
 */

enum {
    CHANGED,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];


static void
moo_radio_action_group_class_init (MooRadioActionGroupClass *klass)
{
    signals[CHANGED] =
            g_signal_new ("changed",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                          G_STRUCT_OFFSET (MooRadioActionGroupClass, changed),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);
}


static void
moo_radio_action_group_init (G_GNUC_UNUSED MooRadioActionGroup *group)
{
}


static void
moo_radio_action_group_add (MooRadioActionGroup *group,
                            MooRadioAction      *action)
{
    g_return_if_fail (!g_slist_find (group->actions, action));
    group->actions = g_slist_prepend (group->actions, action);

    if (!group->active || MOO_TOGGLE_ACTION(action)->active)
    {
        group->active = action;
        moo_toggle_action_set_active (MOO_TOGGLE_ACTION (action), TRUE);
        g_signal_emit (group, signals[CHANGED], 0);
    }
}


static void
moo_radio_action_group_remove (MooRadioActionGroup *group,
                               MooRadioAction      *action)
{
    group->actions = g_slist_remove (group->actions, action);

    if (group->active == action)
    {
        if (group->actions)
        {
            group->active = group->actions->data;
            moo_toggle_action_set_active (MOO_TOGGLE_ACTION (group->active), TRUE);
        }
        else
        {
            group->active = NULL;
        }

        g_signal_emit (group, signals[CHANGED], 0);
    }
}


static void
moo_radio_action_group_active_changed (MooRadioActionGroup *group,
                                       MooRadioAction      *active)
{
    g_return_if_fail (g_slist_find (group->actions, active) != NULL);
    g_return_if_fail (MOO_TOGGLE_ACTION(active)->active);

    if (group->active != active)
    {
        MooRadioAction *old_active = group->active;
        group->active = active;
        moo_toggle_action_set_active (MOO_TOGGLE_ACTION (old_active), FALSE);
        g_signal_emit (group, signals[CHANGED], 0);
    }
}


MooRadioActionGroup *
moo_radio_action_group_new (void)
{
    return g_object_new (MOO_TYPE_RADIO_ACTION_GROUP, NULL);
}
