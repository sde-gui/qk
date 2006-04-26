/*
 *   mooui/mootoggleaction.c
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

#include "mooutils/mootoggleaction.h"
#include "mooutils/mooactiongroup.h"
#include "mooutils/moocompat.h"
#include "mooutils/mooaccel.h"
#include "mooutils/moomarshals.h"
#include "mooutils/mooutils-gobject.h"
#include "mooutils/mooutils-misc.h"
#include <gtk/gtk.h>
#include <string.h>


#define PEEK_DATA(action)   (action->object ? MOO_OBJECT_PTR_GET((MooObjectPtr*)action->data) : action->data)
#define PEEK_OBJECT(action) (action->object ? MOO_OBJECT_PTR_GET((MooObjectPtr*)action->data) : NULL)
#define ACTION_DEAD(action) (action->object ? (MOO_OBJECT_PTR_GET((MooObjectPtr*)action->data) == NULL) : FALSE)


static void         moo_toggle_action_set_property  (GObject            *object,
                                                     guint               prop_id,
                                                     const GValue       *value,
                                                     GParamSpec         *pspec);
static void         moo_toggle_action_get_property  (GObject            *object,
                                                     guint               prop_id,
                                                     GValue             *value,
                                                     GParamSpec         *pspec);
static void         moo_toggle_action_finalize      (GObject            *object);

static GtkWidget   *create_menu_item                (MooAction          *action);
static GtkWidget   *create_tool_item                (MooAction          *action,
                                                     GtkWidget          *toolbar,
                                                     int                 position,
                                                     MooToolItemFlags    flags);

static void         moo_toggle_action_toggled       (MooToggleAction    *action,
                                                     gboolean            active);

static void         moo_toggle_action_add_proxy     (MooAction          *action,
                                                     GtkWidget          *proxy);


enum {
    PROP_0,
    PROP_ACTIVE,
    PROP_TOGGLED_CALLBACK,
    PROP_TOGGLED_OBJECT,
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
    gobject_class->finalize = moo_toggle_action_finalize;

    action_class->add_proxy = moo_toggle_action_add_proxy;
    action_class->create_menu_item = create_menu_item;
    action_class->create_tool_item = create_tool_item;

    klass->toggled = moo_toggle_action_toggled;

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

    g_object_class_install_property (gobject_class,
                                     PROP_TOGGLED_OBJECT,
                                     g_param_spec_object ("toggled-object",
                                             "toggled-object",
                                             "toggled-object",
                                             G_TYPE_OBJECT,
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


static void moo_toggle_action_init (G_GNUC_UNUSED MooToggleAction *action)
{
}


static void
moo_toggle_action_get_property (GObject     *object,
                                guint        prop_id,
                                GValue      *value,
                                GParamSpec  *pspec)
{
    MooToggleAction *action = MOO_TOGGLE_ACTION (object);

    switch (prop_id)
    {
        case PROP_ACTIVE:
            g_value_set_boolean (value, action->active ? TRUE : FALSE);
            break;

        case PROP_TOGGLED_CALLBACK:
            g_value_set_pointer (value, action->callback);
            break;
        case PROP_TOGGLED_DATA:
            g_value_set_pointer (value, PEEK_DATA (action));
            break;
        case PROP_TOGGLED_OBJECT:
            g_value_set_pointer (value, PEEK_OBJECT (action));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
object_died (MooToggleAction *action)
{
    g_return_if_fail (action->object);
    moo_object_ptr_free (action->data);
    action->data = NULL;
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
            action->callback = g_value_get_pointer (value);
            break;

        case PROP_TOGGLED_DATA:
            if (action->object)
                moo_object_ptr_free (action->data);
            action->data = g_value_get_pointer (value);
            action->object = FALSE;
            break;

        case PROP_TOGGLED_OBJECT:
            if (action->object)
                moo_object_ptr_free (action->data);
            if (G_VALUE_HOLDS_OBJECT (value))
                action->data = moo_object_ptr_new (g_value_get_object (value),
                                                   (GWeakNotify) object_died,
                                                   action);
            else
                action->data = NULL;
            action->object = TRUE;
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
moo_toggle_action_finalize (GObject *object)
{
    MooToggleAction *action = MOO_TOGGLE_ACTION (object);

    if (action->object)
        moo_object_ptr_free (action->data);

    G_OBJECT_CLASS(moo_toggle_action_parent_class)->finalize (object);
}


static void
moo_toggle_action_toggled (MooToggleAction *action,
                           gboolean         active)
{
    if ((action->active && !active) || (!action->active && active))
    {
        active = active ? TRUE : FALSE;
        action->active = active;

        if (action->callback && !ACTION_DEAD (action))
            action->callback (PEEK_DATA (action), active);

        g_object_notify (G_OBJECT (action), "active");
    }
}


void
moo_toggle_action_set_active (MooToggleAction *action,
                              gboolean         active)
{
    g_return_if_fail (MOO_IS_TOGGLE_ACTION (action));
    g_signal_emit (action, signals[TOGGLED], 0, active);
}


static GtkWidget*
create_menu_item (MooAction *action)
{
    GtkWidget *item = NULL;

    if (action->stock_id)
    {
        GtkStockItem stock_item;

        if (gtk_stock_lookup (action->stock_id, &stock_item))
            item = gtk_check_menu_item_new_with_mnemonic (stock_item.label);
        else
            g_warning ("could not find stock item '%s'", action->stock_id);
    }

    if (!item)
        item = gtk_check_menu_item_new_with_label (action->label);

    gtk_menu_item_set_accel_path (GTK_MENU_ITEM (item),
                                  _moo_action_get_accel_path (action));

    if (action->force_accel_label)
    {
        GtkWidget *accel_label = gtk_bin_get_child (GTK_BIN (item));

        if (GTK_IS_ACCEL_LABEL (accel_label))
            moo_accel_label_set_action (accel_label, action);
        else
            g_critical ("%s: oops", G_STRLOC);
    }

    moo_toggle_action_add_proxy (action, item);

    return item;
}


static GtkWidget*
create_tool_item (MooAction *action,
                  GtkWidget *toolbar,
                  int        position,
                  G_GNUC_UNUSED MooToolItemFlags flags)
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
        moo_widget_set_tooltip (GTK_WIDGET (item), action->tooltip);

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


static void
moo_toggle_action_add_proxy (MooAction *action,
                             GtkWidget *proxy)
{
    if (GTK_IS_CHECK_MENU_ITEM (proxy) ||
#if GTK_CHECK_VERSION(2,4,0)
        GTK_IS_TOGGLE_TOOL_BUTTON (proxy) ||
#endif /* GTK_CHECK_VERSION(2,4,0) */
        GTK_IS_TOGGLE_BUTTON (proxy))
    {
        moo_sync_bool_property (proxy, "active", action, "active", FALSE);
    }
    else
    {
        g_critical ("unknown proxy type");
    }

    MOO_ACTION_CLASS(moo_toggle_action_parent_class)->add_proxy (action, proxy);
}
