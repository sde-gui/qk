/*
 *   mooui/mootoggleaction.c
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

#include "mooutils/mootoggleaction.h"
#include "mooutils/mooactiongroup.h"
#include "mooutils/moocompat.h"
#include "mooutils/moomarshals.h"
#include <gtk/gtk.h>
#include <string.h>


static void moo_toggle_action_class_init        (MooToggleActionClass   *klass);

static void moo_toggle_action_init              (MooToggleAction        *action);

static void moo_toggle_action_set_property      (GObject                *object,
                                                 guint                   prop_id,
                                                 const GValue           *value,
                                                 GParamSpec             *pspec);
static void moo_toggle_action_get_property      (GObject                *object,
                                                 guint                   prop_id,
                                                 GValue                 *value,
                                                 GParamSpec             *pspec);

static GtkWidget *moo_toggle_action_create_menu_item (MooAction      *action);
static GtkWidget *moo_toggle_action_create_tool_item (MooAction      *action,
                                                      GtkWidget      *toolbar,
                                                      int             position);

static void moo_action_toggled                  (MooToggleAction     *action,
                                                 gboolean             active);

static void moo_toggle_action_add_proxy         (MooAction               *action,
                                                 GtkWidget               *proxy);


enum {
    PROP_0,
    PROP_ACTIVE,
    PROP_TOGGLED_CALLBACK,
    PROP_TOGGLED_DATA
};

enum {
    TOGGLED,
    LAST_SIGNAL
};


static guint signals[LAST_SIGNAL] = {0};


/* MOO_TYPE_TOGGLE_ACTION */
G_DEFINE_TYPE (MooToggleAction, moo_toggle_action, MOO_TYPE_ACTION)


static void moo_toggle_action_class_init (MooToggleActionClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    MooActionClass *action_class = MOO_ACTION_CLASS (klass);

    gobject_class->set_property = moo_toggle_action_set_property;
    gobject_class->get_property = moo_toggle_action_get_property;

    action_class->add_proxy = moo_toggle_action_add_proxy;
    action_class->create_menu_item = moo_toggle_action_create_menu_item;
    action_class->create_tool_item = moo_toggle_action_create_tool_item;

    klass->toggled = moo_action_toggled;

    g_object_class_install_property (gobject_class,
                                     PROP_ACTIVE,
                                     g_param_spec_boolean ("active",
                                                           "active",
                                                           "active",
                                                           TRUE,
                                                           G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_TOGGLED_CALLBACK,
                                     g_param_spec_pointer ("toggled-callback",
                                                           "toggled-callback",
                                                           "toggled-callback",
                                                           G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_TOGGLED_DATA,
                                     g_param_spec_pointer ("toggled-data",
                                                           "toggled-data",
                                                           "toggled-data",
                                                           G_PARAM_READWRITE));

    signals[TOGGLED] =
        g_signal_new ("toggled",
                      G_OBJECT_CLASS_TYPE (klass),
                      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                      G_STRUCT_OFFSET (MooToggleActionClass, toggled),
                      NULL, NULL,
                      _moo_marshal_VOID__BOOL,
                      G_TYPE_NONE, 1,
                      G_TYPE_BOOLEAN);
}


static void moo_toggle_action_init (MooToggleAction *action)
{
    g_return_if_fail (MOO_IS_ACTION (action));

    action->active = TRUE;
    action->toggled_callback = NULL;
    action->toggled_data = NULL;
}


static void moo_toggle_action_get_property     (GObject        *object,
                                         guint           prop_id,
                                         GValue         *value,
                                         GParamSpec     *pspec)
{
    MooToggleAction *action = MOO_TOGGLE_ACTION (object);

    switch (prop_id)
    {
        case PROP_ACTIVE:
            g_value_set_boolean (value, action->active);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void moo_toggle_action_set_property     (GObject        *object,
                                         guint           prop_id,
                                         const GValue   *value,
                                         GParamSpec     *pspec)
{
    MooToggleAction *action = MOO_TOGGLE_ACTION (object);

    switch (prop_id)
    {
        case PROP_ACTIVE:
            moo_toggle_action_set_active (action, g_value_get_boolean (value));
            break;

        case PROP_TOGGLED_CALLBACK:
            action->toggled_callback = (void(*)(gpointer,gboolean))g_value_get_pointer (value);
            break;

        case PROP_TOGGLED_DATA:
            action->toggled_data = g_value_get_pointer (value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void moo_action_toggled                  (MooToggleAction     *action,
                                                 gboolean             active)
{
    action->active = active;
    if (action->toggled_callback)
        action->toggled_callback (action->toggled_data, active);
}

void        moo_toggle_action_set_active        (MooToggleAction     *action,
                                                 gboolean             active)
{
    g_return_if_fail (MOO_IS_TOGGLE_ACTION (action));
    g_signal_emit (action, signals[TOGGLED], 0, active);
}


static GtkWidget*
moo_toggle_action_create_menu_item (MooAction         *action)
{
    GtkWidget *item = NULL;

    if (action->stock_id) {
        GtkStockItem stock_item;
        if (gtk_stock_lookup (action->stock_id, &stock_item))
        {
            item = gtk_check_menu_item_new_with_mnemonic (stock_item.label);
        }
        else
            g_warning ("could not find stock item '%s'", action->stock_id);
    }
    if (!item)
        item = gtk_check_menu_item_new_with_label (action->label);

    gtk_menu_item_set_accel_path (GTK_MENU_ITEM (item),
                                  _moo_action_get_accel_path (action));

    moo_toggle_action_add_proxy (action, item);

    return item;
}


static GtkWidget*
moo_toggle_action_create_tool_item (MooAction      *action,
                                    GtkWidget      *toolbar,
                                    int             position)
{
#if GTK_CHECK_VERSION(2,4,0)
    GtkToolItem *item = NULL;

    if (action->stock_id)
    {
        item = gtk_toggle_tool_button_new_from_stock (action->stock_id);
    }
    else
    {
        GtkWidget *icon = NULL;
        if (action->icon_stock_id)
        {
            icon = gtk_image_new_from_stock (action->icon_stock_id,
                                             gtk_toolbar_get_icon_size (GTK_TOOLBAR (toolbar)));
            if (!icon) g_warning ("could not create stock icon '%s'",
                                  action->icon_stock_id);
            else gtk_widget_show (icon);
        }
        item = gtk_toggle_tool_button_new ();
        gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON (item), icon);
        gtk_tool_button_set_use_underline (GTK_TOOL_BUTTON (item), TRUE);
    }

    if (action->tooltip)
    {
        GtkTooltips *tooltips = gtk_tooltips_new ();
        gtk_tool_item_set_tooltip (item,
                                   tooltips,
                                   action->tooltip,
                                   action->tooltip);
        g_object_set_data_full (G_OBJECT (item), "moo-tooltips",
                                tooltips, g_object_unref);
    }

    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, position);
    gtk_container_child_set (GTK_CONTAINER (toolbar), GTK_WIDGET (item),
                             "homogeneous", FALSE, NULL);

#else /* !GTK_CHECK_VERSION(2,4,0) */

    GtkWidget *item = NULL;
    GtkWidget *icon = NULL;
    if (action->stock_id || action->icon_stock_id)
    {
        icon = gtk_image_new_from_stock (action->stock_id ? action->stock_id : action->icon_stock_id,
                                         gtk_toolbar_get_icon_size (toolbar));
        if (!icon) g_warning ("could not create stock icon '%s'",
                              action->stock_id ? action->stock_id : action->icon_stock_id);
        else gtk_widget_show (icon);
    }
    item = gtk_toolbar_insert_element (toolbar,
                                        GTK_TOOLBAR_CHILD_TOGGLEBUTTON,
                                        NULL,
                                        action->label,
                                        action->tooltip,
                                        action->tooltip,
                                        icon,
                                        NULL,
                                        action,
                                        position);

    gtk_button_set_use_underline (GTK_BUTTON (item), TRUE);

#endif /* !GTK_CHECK_VERSION(2,4,0) */

    moo_toggle_action_add_proxy (action, GTK_WIDGET (item));

    return GTK_WIDGET (item);
}


/****************************************************************************/

static void proxy_set_active    (GtkWidget          *proxy,
                                 gboolean            active);
static gboolean proxy_get_active(GtkWidget          *proxy);
static void proxy_destroyed     (MooAction          *action,
                                 gpointer            proxy);
static void action_destroyed    (GtkWidget          *proxy,
                                 gpointer            action);
static void proxy_toggled       (GtkWidget          *proxy,
                                 MooToggleAction    *action);


static void proxy_set_active      (GtkWidget    *proxy,
                                   gboolean      active)
{
    if (GTK_IS_CHECK_MENU_ITEM (proxy))
        gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (proxy), active);
#if GTK_CHECK_VERSION(2,4,0)
    else if (GTK_IS_TOGGLE_TOOL_BUTTON (proxy))
        gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (proxy), active);
#endif /* GTK_CHECK_VERSION(2,4,0) */
    else if (GTK_IS_TOGGLE_BUTTON (proxy))
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (proxy), active);
    else
        g_critical ("unknown proxy type");
}

static gboolean proxy_get_active      (GtkWidget    *proxy)
{
    if (GTK_IS_CHECK_MENU_ITEM (proxy))
        return gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (proxy));
#if GTK_CHECK_VERSION(2,4,0)
    else if (GTK_IS_TOGGLE_TOOL_BUTTON (proxy))
        return gtk_toggle_tool_button_get_active (GTK_TOGGLE_TOOL_BUTTON (proxy));
#endif /* GTK_CHECK_VERSION(2,4,0) */
    else if (GTK_IS_TOGGLE_BUTTON (proxy))
        return gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (proxy));
    else
        g_critical ("unknown proxy type");
    return FALSE;
}


static void proxy_destroyed          (MooAction  *action,
                                      gpointer    proxy)
{
    g_signal_handlers_disconnect_by_func (action, (gpointer)proxy_set_active, proxy);
    g_object_weak_unref (G_OBJECT (action), (GWeakNotify)action_destroyed, proxy);
}

static void action_destroyed         (GtkWidget  *proxy,
                                      gpointer    action)
{
    g_signal_handlers_disconnect_by_func (proxy, (gpointer)moo_action_activate, action);
    g_object_weak_unref (G_OBJECT (proxy), (GWeakNotify)proxy_destroyed, action);
}


static void proxy_toggled   (GtkWidget          *proxy,
                             MooToggleAction    *action)
{
    g_signal_handlers_block_by_func (action, (gpointer)proxy_set_active, proxy);
    moo_toggle_action_set_active (action, proxy_get_active (proxy));
    g_signal_handlers_unblock_by_func (action, (gpointer)proxy_set_active, proxy);
}

static void moo_toggle_action_add_proxy         (MooAction               *action,
                                                 GtkWidget               *proxy)
{
    proxy_set_active (proxy, MOO_TOGGLE_ACTION(action)->active);

    if (GTK_IS_CHECK_MENU_ITEM (proxy) ||
#if GTK_CHECK_VERSION(2,4,0)
        GTK_IS_TOGGLE_TOOL_BUTTON (proxy) ||
#endif /* GTK_CHECK_VERSION(2,4,0) */
        GTK_IS_TOGGLE_BUTTON (proxy))
    {
        g_signal_connect_swapped (action, "toggled",
                                  G_CALLBACK (proxy_set_active), proxy);

        g_signal_connect (proxy, "toggled", G_CALLBACK (proxy_toggled), action);
    }
    else
    {
        g_critical ("unknown proxy type");
    }

    g_object_weak_ref (G_OBJECT (proxy), (GWeakNotify)proxy_destroyed, action);
    g_object_weak_ref (G_OBJECT (action), (GWeakNotify)action_destroyed, proxy);

    MOO_ACTION_CLASS(moo_toggle_action_parent_class)->add_proxy (action, proxy);
}
