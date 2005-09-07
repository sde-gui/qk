/*
 *   mooui/moomenuaction.c
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

#include "mooui/moomenuaction.h"
#include "mooui/mooactiongroup.h"
#include "mooutils/moocompat.h"
#include "mooutils/moomarshals.h"
#include <gtk/gtk.h>
#include <string.h>


static void moo_menu_action_class_init      (MooMenuActionClass *klass);

static void moo_menu_action_init            (MooMenuAction      *action);

static void moo_menu_action_set_property    (GObject            *object,
                                             guint               prop_id,
                                             const GValue       *value,
                                             GParamSpec         *pspec);
static void moo_menu_action_get_property    (GObject            *object,
                                             guint               prop_id,
                                             GValue             *value,
                                             GParamSpec         *pspec);

static void moo_menu_action_add_proxy       (MooAction          *action,
                                             GtkWidget          *proxy);

static GtkWidget   *moo_menu_action_create_menu_item    (MooAction      *action,
                                                         GtkMenuShell   *menushell,
                                                         int             position);
static gboolean     moo_menu_action_create_tool_item    (MooAction      *action,
                                                         GtkToolbar     *toolbar,
                                                         int             position);


enum {
    PROP_0,
    PROP_CREATE_MENU_FUNC,
    PROP_CREATE_MENU_DATA
};



/* MOO_TYPE_MENU_ACTION */
G_DEFINE_TYPE (MooMenuAction, moo_menu_action, MOO_TYPE_ACTION)


static void moo_menu_action_class_init (MooMenuActionClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    MooActionClass *action_class = MOO_ACTION_CLASS (klass);

    gobject_class->set_property = moo_menu_action_set_property;
    gobject_class->get_property = moo_menu_action_get_property;

    action_class->add_proxy = moo_menu_action_add_proxy;
    action_class->create_menu_item = moo_menu_action_create_menu_item;
    action_class->create_tool_item = moo_menu_action_create_tool_item;

    g_object_class_install_property (gobject_class,
                                     PROP_CREATE_MENU_FUNC,
                                     g_param_spec_pointer ("create-menu-func",
                                                           "create-menu-func",
                                                           "create-menu-func",
                                                           G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_CREATE_MENU_DATA,
                                     g_param_spec_pointer ("create-menu-data",
                                                           "create-menu-data",
                                                           "create-menu-data",
                                                           G_PARAM_READWRITE));
}


static void moo_menu_action_init (MooMenuAction *action)
{
    g_return_if_fail (MOO_IS_ACTION (action));

    action->create_menu_func = NULL;
    action->create_menu_data = NULL;
}


static void moo_menu_action_get_property    (GObject        *object,
                                             guint           prop_id,
                                             GValue         *value,
                                             GParamSpec     *pspec)
{
    MooMenuAction *action = MOO_MENU_ACTION (object);

    switch (prop_id)
    {
        case PROP_CREATE_MENU_FUNC:
            g_value_set_pointer (value, (gpointer)action->create_menu_func);
            break;

        case PROP_CREATE_MENU_DATA:
            g_value_set_pointer (value, action->create_menu_data);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void moo_menu_action_set_property     (GObject        *object,
                                              guint           prop_id,
                                              const GValue   *value,
                                              GParamSpec     *pspec)
{
    MooMenuAction *action = MOO_MENU_ACTION (object);

    switch (prop_id)
    {
        case PROP_CREATE_MENU_FUNC:
            action->create_menu_func = (MooMenuCreationFunc)g_value_get_pointer (value);
            break;

        case PROP_CREATE_MENU_DATA:
            action->create_menu_data = g_value_get_pointer (value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void moo_menu_action_add_proxy       (MooAction          *action,
                                             GtkWidget          *proxy)
{
    MOO_ACTION_CLASS (moo_menu_action_parent_class)->add_proxy (action, proxy);
}


static GtkWidget   *moo_menu_action_create_menu_item    (MooAction      *action,
                                                         GtkMenuShell   *menushell,
                                                         int             position)
{
    MooMenuAction *menu_action;
    GtkMenuItem *item;

    menu_action = MOO_MENU_ACTION (action);
    g_return_val_if_fail (menu_action->create_menu_func != NULL, NULL);

    item = menu_action->create_menu_func (menu_action->create_menu_data,
                                          action);
    g_return_val_if_fail (item != NULL, NULL);

    gtk_menu_shell_insert (menushell, GTK_WIDGET (item), position);
    moo_menu_action_add_proxy (action, GTK_WIDGET (item));
    return GTK_WIDGET (item);
}


static gboolean     moo_menu_action_create_tool_item    (G_GNUC_UNUSED MooAction      *action,
                                                         G_GNUC_UNUSED GtkToolbar     *toolbar,
                                                         G_GNUC_UNUSED int             position)
{
    g_warning ("%s: should not be called", G_STRLOC);
    return FALSE;
}


MooAction       *moo_menu_action_new      (const char         *id,
                                           const char         *group_id,
                                           MooMenuCreationFunc create_menu,
                                           gpointer            data)
{
    return MOO_ACTION (g_object_new (MOO_TYPE_MENU_ACTION,
                                     "id", id,
                                     "group-id", group_id,
                                     "create-menu-func", create_menu,
                                     "create-menu-data", data,
                                     "no-accel", TRUE,
                                     NULL));
}
