/*
 *   mooui/moomenuaction.c
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

#include "mooutils/moomenuaction.h"
#include "mooutils/mooactiongroup.h"
#include "mooutils/moocompat.h"
#include "mooutils/moomarshals.h"
#include <gtk/gtk.h>
#include <string.h>


static void moo_menu_action_class_init      (MooMenuActionClass *klass);

static void moo_menu_action_init            (MooMenuAction      *action);
static void moo_menu_action_finalize        (GObject            *object);

static void moo_menu_action_set_property    (GObject            *object,
                                             guint               prop_id,
                                             const GValue       *value,
                                             GParamSpec         *pspec);
static void moo_menu_action_get_property    (GObject            *object,
                                             guint               prop_id,
                                             GValue             *value,
                                             GParamSpec         *pspec);

static GtkWidget   *moo_menu_action_create_menu_item    (MooAction      *action);

static void data_destroyed                  (MooMenuAction      *action,
                                             gpointer            data);

enum {
    PROP_0,
    PROP_MENU_MGR
};



/* MOO_TYPE_MENU_ACTION */
G_DEFINE_TYPE (MooMenuAction, moo_menu_action, MOO_TYPE_ACTION)


static void
moo_menu_action_class_init (MooMenuActionClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    MooActionClass *action_class = MOO_ACTION_CLASS (klass);

    gobject_class->set_property = moo_menu_action_set_property;
    gobject_class->get_property = moo_menu_action_get_property;
    gobject_class->finalize = moo_menu_action_finalize;

    action_class->create_menu_item = moo_menu_action_create_menu_item;

    g_object_class_install_property (gobject_class,
                                     PROP_MENU_MGR,
                                     g_param_spec_object ("menu-mgr",
                                             "menu-mgr",
                                             "menu-mgr",
                                             MOO_TYPE_MENU_MGR,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
}


static void
moo_menu_action_init (MooMenuAction *action)
{
    action->mgr = moo_menu_mgr_new ();
}


static void
moo_menu_action_get_property (GObject        *object,
                              guint           prop_id,
                              GValue         *value,
                              GParamSpec     *pspec)
{
    MooMenuAction *action = MOO_MENU_ACTION (object);

    switch (prop_id)
    {
        case PROP_MENU_MGR:
            g_value_set_object (value, action->mgr);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
moo_menu_action_set_property (GObject        *object,
                              guint           prop_id,
                              const GValue   *value,
                              GParamSpec     *pspec)
{
    MooMenuAction *action = MOO_MENU_ACTION (object);

    switch (prop_id)
    {
        case PROP_MENU_MGR:
            moo_menu_action_set_mgr (action, g_value_get_object (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static GtkWidget*
moo_menu_action_create_menu_item (MooAction *action)
{
    MooMenuAction *menu_action;
    GtkWidget *item;
    gpointer data = NULL;
    GDestroyNotify destroy = NULL;

    menu_action = MOO_MENU_ACTION (action);

    if (menu_action->data && menu_action->is_object)
    {
        data = g_object_ref (menu_action->data);
        destroy = g_object_unref;
    }
    else
    {
        data = menu_action->data;
        destroy = NULL;
    }

    item = moo_menu_mgr_create_item (menu_action->mgr, menu_action->label,
                                     0, data, destroy);
    g_return_val_if_fail (item != NULL, NULL);

    MOO_ACTION_CLASS(G_OBJECT_GET_CLASS (action))->add_proxy (action, item);
    return item;
}


MooAction*
moo_menu_action_new (const char     *id)
{
    g_return_val_if_fail (id != NULL, NULL);
    return g_object_new (MOO_TYPE_MENU_ACTION, "id", id, NULL);
}


MooMenuMgr*
moo_menu_action_get_mgr (MooMenuAction *action)
{
    g_return_val_if_fail (MOO_IS_MENU_ACTION (action), NULL);
    return action->mgr;
}


void
moo_menu_action_set_mgr (MooMenuAction  *action,
                         MooMenuMgr     *mgr)
{
    g_return_if_fail (MOO_IS_MENU_ACTION (action));
    g_return_if_fail (!mgr || MOO_IS_MENU_MGR (mgr));

    if (mgr == action->mgr)
        return;

    g_object_unref (action->mgr);

    if (mgr)
        action->mgr = g_object_ref (mgr);
    else
        action->mgr = moo_menu_mgr_new ();

    g_object_notify (G_OBJECT (action), "menu-mgr");
}


static void
moo_menu_action_finalize (GObject *object)
{
    MooMenuAction *action = MOO_MENU_ACTION (object);

    if (action->data && action->is_object)
        g_object_weak_unref (action->data, (GWeakNotify) data_destroyed, action);
    g_free (action->label);

    G_OBJECT_CLASS(moo_menu_action_parent_class)->finalize (object);
}


static void
data_destroyed (MooMenuAction      *action,
                gpointer            data)
{
    g_return_if_fail (action->data == data);
    action->data = NULL;
}


void
moo_menu_action_set_menu_data (MooMenuAction  *action,
                               gpointer        data,
                               gboolean        is_object)
{
    g_return_if_fail (MOO_IS_MENU_ACTION (action));

    if (action->data && action->is_object)
        g_object_weak_unref (action->data, (GWeakNotify) data_destroyed, action);

    action->data = data;
    action->is_object = is_object;

    if (action->data && action->is_object)
        g_object_weak_ref (action->data, (GWeakNotify) data_destroyed, action);
}


void
moo_menu_action_set_menu_label (MooMenuAction  *action,
                                const char     *label)
{
    g_return_if_fail (MOO_IS_MENU_ACTION (action));
    g_free (action->label);
    action->label = g_strdup (label);
}
