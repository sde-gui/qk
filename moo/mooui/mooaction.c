/*
 *   mooui/mooaction.c
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

#include "mooui/mooaction.h"
#include "mooui/mooactiongroup.h"
#include "mooutils/moomarshals.h"
#include "mooutils/moocompat.h"
#include <gtk/gtk.h>
#include <string.h>


static void moo_action_class_init       (MooActionClass *klass);

GObject    *moo_action_constructor      (GType                  type,
                                         guint                  n_construct_properties,
                                         GObjectConstructParam *construct_properties);
static void moo_action_init             (MooAction      *action);
static void moo_action_finalize         (GObject        *object);

static void moo_action_set_property     (GObject        *object,
                                         guint           prop_id,
                                         const GValue   *value,
                                         GParamSpec     *pspec);
static void moo_action_get_property     (GObject        *object,
                                         guint           prop_id,
                                         GValue         *value,
                                         GParamSpec     *pspec);

static void moo_action_activate_real    (MooAction      *action);

static void moo_action_set_closure      (MooAction      *action,
                                         MooClosure     *closure);
static void moo_action_set_stock_id     (MooAction      *action,
                                         const char     *stock_id);
static void moo_action_set_label        (MooAction      *action,
                                         const char     *label);
static void moo_action_set_tooltip      (MooAction      *action,
                                         const char     *tooltip);
static void moo_action_set_icon_stock_id(MooAction      *action,
                                         const char     *stock_id);
static void moo_action_set_icon         (MooAction      *action,
                                         GdkPixbuf      *icon);

static void moo_action_set_sensitive_real (MooAction      *action,
                                         gboolean        sensitive);
static void moo_action_set_visible_real (MooAction      *action,
                                         gboolean        visible);

static GtkWidget *moo_action_create_menu_item_real (MooAction      *action,
                                                    GtkMenuShell   *menushell,
                                                    int             position);
static gboolean   moo_action_create_tool_item_real (MooAction      *action,
                                                    GtkToolbar     *toolbar,
                                                    int             position);

static void moo_action_add_proxy        (MooAction      *action,
                                         GtkWidget      *proxy);

#if GTK_MINOR_VERSION < 4
static void tool_item_callback (G_GNUC_UNUSED GtkWidget *widget,
                                MooAction *action)
{
    moo_action_activate (action);
}
#endif /* GTK_MINOR_VERSION < 4 */


enum {
    PROP_0,

    PROP_ID,
    PROP_GROUP_ID,

    PROP_NAME,

    PROP_CLOSURE,

    PROP_STOCK_ID,
    PROP_LABEL,
    PROP_TOOLTIP,

    PROP_NO_ACCEL,
    PROP_ACCEL,
    PROP_DEFAULT_ACCEL,

    PROP_ICON,
    PROP_ICON_STOCK_ID,

    PROP_DEAD,
    PROP_SENSITIVE,
    PROP_VISIBLE
};

enum {
    ACTIVATE,
    SET_SENSITIVE,
    SET_VISIBLE,
    LAST_SIGNAL
};


static guint signals[LAST_SIGNAL] = {0};


/* MOO_TYPE_ACTION */
G_DEFINE_TYPE (MooAction, moo_action, G_TYPE_OBJECT)


static GHashTable *accel_map = NULL;            /* char* -> char* */
static GHashTable *default_accel_map = NULL;    /* char* -> char* */


static void moo_action_class_init (MooActionClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->constructor = moo_action_constructor;
    gobject_class->finalize = moo_action_finalize;
    gobject_class->set_property = moo_action_set_property;
    gobject_class->get_property = moo_action_get_property;

    klass->activate = moo_action_activate_real;
    klass->add_proxy = moo_action_add_proxy;
    klass->set_sensitive = moo_action_set_sensitive_real;
    klass->set_visible = moo_action_set_visible_real;
    klass->create_menu_item = moo_action_create_menu_item_real;
    klass->create_tool_item = moo_action_create_tool_item_real;

    accel_map = g_hash_table_new_full (g_str_hash, g_str_equal,
                                       (GDestroyNotify) g_free,
                                       (GDestroyNotify) g_free);
    default_accel_map = g_hash_table_new_full (g_str_hash, g_str_equal,
                                               (GDestroyNotify) g_free,
                                               (GDestroyNotify) g_free);

    g_object_class_install_property (gobject_class,
                                     PROP_ID,
                                     g_param_spec_string ("id",
                                                          "id",
                                                          "id",
                                                          NULL,
                                                          G_PARAM_READWRITE |
                                                                  G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (gobject_class,
                                     PROP_GROUP_ID,
                                     g_param_spec_string ("group-id",
                                                          "group-id",
                                                          "group-id",
                                                          NULL,
                                                          G_PARAM_READWRITE |
                                                                  G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (gobject_class,
                                     PROP_NAME,
                                     g_param_spec_string ("name",
                                                          "name",
                                                          "name",
                                                          NULL,
                                                          G_PARAM_READWRITE |
                                                                  G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_CLOSURE,
                                     g_param_spec_object ("closure",
                                                          "closure",
                                                          "closure",
                                                          MOO_TYPE_CLOSURE,
                                                          G_PARAM_READWRITE |
                                                                  G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_STOCK_ID,
                                     g_param_spec_string ("stock-id",
                                                          "stock-id",
                                                          "stock-id",
                                                          NULL,
                                                          G_PARAM_READWRITE |
                                                                  G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_LABEL,
                                     g_param_spec_string ("label",
                                                          "label",
                                                          "label",
                                                          NULL,
                                                          G_PARAM_READWRITE |
                                                                  G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_TOOLTIP,
                                     g_param_spec_string ("tooltip",
                                                          "tooltip",
                                                          "tooltip",
                                                          NULL,
                                                          G_PARAM_READWRITE |
                                                                  G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_NO_ACCEL,
                                     g_param_spec_boolean ("no-accel",
                                                           "no-accel",
                                                           "no-accel",
                                                           FALSE,
                                                           G_PARAM_READWRITE |
                                                                   G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_ACCEL,
                                     g_param_spec_string ("accel",
                                                          "accel",
                                                          "accel",
                                                          NULL,
                                                          G_PARAM_READWRITE |
                                                                  G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_DEFAULT_ACCEL,
                                     g_param_spec_string ("default-accel",
                                                          "default-accel",
                                                          "default-accel",
                                                          NULL,
                                                          G_PARAM_READWRITE |
                                                                  G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_SENSITIVE,
                                     g_param_spec_boolean ("sensitive",
                                                           "sensitive",
                                                           "sensitive",
                                                           TRUE,
                                                           G_PARAM_READWRITE |
                                                                   G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_VISIBLE,
                                     g_param_spec_boolean ("visible",
                                                           "visible",
                                                           "visible",
                                                           TRUE,
                                                           G_PARAM_READWRITE |
                                                                   G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_DEAD,
                                     g_param_spec_boolean ("dead",
                                                           "dead",
                                                           "dead",
                                                           FALSE,
                                                           G_PARAM_READWRITE |
                                                                   G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (gobject_class,
                                     PROP_ICON_STOCK_ID,
                                     g_param_spec_string ("icon-stock-id",
                                                          "icon-stock-id",
                                                          "icon-stock-id",
                                                          NULL,
                                                          G_PARAM_READWRITE |
                                                                  G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_ICON,
                                     g_param_spec_object ("icon",
                                                          "icon",
                                                          "icon",
                                                          GDK_TYPE_PIXBUF,
                                                          G_PARAM_READWRITE |
                                                                  G_PARAM_CONSTRUCT));

    signals[ACTIVATE] =
        g_signal_new ("activate",
                      G_OBJECT_CLASS_TYPE (klass),
                      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                      G_STRUCT_OFFSET (MooActionClass, activate),
                      NULL, NULL,
                      _moo_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    signals[SET_SENSITIVE] =
        g_signal_new ("set-sensitive",
                      G_OBJECT_CLASS_TYPE (klass),
                      G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                      G_STRUCT_OFFSET (MooActionClass, set_sensitive),
                      NULL, NULL,
                      _moo_marshal_VOID__BOOL,
                      G_TYPE_NONE, 1,
                      G_TYPE_BOOLEAN);

    signals[SET_VISIBLE] =
        g_signal_new ("set-visible",
                      G_OBJECT_CLASS_TYPE (klass),
                      G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                      G_STRUCT_OFFSET (MooActionClass, set_visible),
                      NULL, NULL,
                      _moo_marshal_VOID__BOOL,
                      G_TYPE_NONE, 1,
                      G_TYPE_BOOLEAN);
}


static void moo_action_init (MooAction *action)
{
    action->constructed = FALSE;

    action->group = NULL;

    action->id = NULL;
    action->group_id = NULL;
    action->name = NULL;

    action->no_accel = FALSE;
    action->accel = NULL;
    action->default_accel = NULL;
    action->dead = FALSE;
    action->visible = TRUE;
    action->sensitive = TRUE;
    action->closure = NULL;
    action->stock_id = NULL;
    action->label = NULL;
    action->tooltip = NULL;
    action->icon_stock_id = NULL;
    action->icon = NULL;
}


GObject    *moo_action_constructor      (GType                  type,
                                         guint                  n_props,
                                         GObjectConstructParam *props)
{
    GObject *object;
    MooAction *action;

    object =
        G_OBJECT_CLASS(moo_action_parent_class)->constructor (type, n_props, props);

    action = MOO_ACTION (object);
    action->constructed = TRUE;

    if (action->dead)
        return object;

    if (!action->id || !action->id[0]) {
        g_critical ("%s: no action id", G_STRLOC);
        if (!action->id)
            action->id = g_strdup ("");
        action->no_accel = TRUE;
    }

    if (!action->name)
        action->name = g_strdup (action->id);

    if (!action->group_id) {
        g_critical ("action doesn't have group id");
        if (!action->group_id)
            action->group_id = g_strdup ("");
    }

    if (!action->accel)
        action->accel = g_strdup ("");
    if (!action->default_accel)
        action->default_accel = g_strdup (action->accel);
    moo_action_set_accel (action, action->accel);
    moo_action_set_default_accel (action, action->default_accel);

    return object;
}


static void moo_action_finalize       (GObject      *object)
{
    MooAction *action = MOO_ACTION (object);

    if (action->closure)
        g_object_unref (action->closure);
    g_free (action->id);
    g_free (action->group_id);
    g_free (action->name);
    g_free (action->stock_id);
    g_free (action->label);
    g_free (action->tooltip);
    g_free (action->icon_stock_id);
    if (action->icon)
        g_object_unref (action->icon);

    G_OBJECT_CLASS (moo_action_parent_class)->finalize (object);
}


static void moo_action_activate_real    (MooAction      *action)
{
    g_return_if_fail (MOO_IS_ACTION (action));
    if (action->closure)
        moo_closure_invoke (action->closure);
}


static void moo_action_get_property     (GObject        *object,
                                         guint           prop_id,
                                         GValue         *value,
                                         GParamSpec     *pspec)
{
    MooAction *action = MOO_ACTION (object);

    switch (prop_id)
    {
        case PROP_ID:
            g_value_set_string (value, action->id);
            break;
        case PROP_GROUP_ID:
            g_value_set_string (value, action->group_id);
            break;
        case PROP_NAME:
            g_value_set_string (value, action->name);
            break;
        case PROP_SENSITIVE:
            g_value_set_boolean (value, action->sensitive);
            break;
        case PROP_VISIBLE:
            g_value_set_boolean (value, action->visible);
            break;
        case PROP_DEAD:
            g_value_set_boolean (value, action->dead);
            break;
        case PROP_CLOSURE:
            g_value_set_object (value, action->closure);
            break;
        case PROP_STOCK_ID:
            g_value_set_string (value, action->stock_id);
            break;
        case PROP_LABEL:
            g_value_set_string (value, action->label);
            break;
        case PROP_TOOLTIP:
            g_value_set_string (value, action->tooltip);
            break;
        case PROP_NO_ACCEL:
            g_value_set_boolean (value, action->no_accel);
            break;
        case PROP_ACCEL:
            g_value_set_string (value, moo_action_get_accel (action));
            break;
        case PROP_DEFAULT_ACCEL:
            g_value_set_string (value, moo_action_get_default_accel (action));
            break;
        case PROP_ICON_STOCK_ID:
            g_value_set_string (value, action->icon_stock_id);
            break;
        case PROP_ICON:
            g_value_set_object (value, action->icon);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void moo_action_set_property     (GObject        *object,
                                         guint           prop_id,
                                         const GValue   *value,
                                         GParamSpec     *pspec)
{
    MooAction *action = MOO_ACTION (object);

    switch (prop_id)
    {
        case PROP_ID:
            g_return_if_fail (action->id == NULL);
            action->id = g_strdup (g_value_get_string (value));
            break;
        case PROP_GROUP_ID:
            g_return_if_fail (action->group_id == NULL);
            action->group_id = g_strdup (g_value_get_string (value));
            break;
        case PROP_NAME:
            g_free (action->name);
            action->name = g_strdup (g_value_get_string (value));
            break;
        case PROP_SENSITIVE:
            moo_action_set_sensitive (action, g_value_get_boolean (value));
            break;
        case PROP_VISIBLE:
            moo_action_set_visible (action, g_value_get_boolean (value));
            break;
        case PROP_DEAD:
            action->dead = g_value_get_boolean (value);
            break;
        case PROP_CLOSURE:
            moo_action_set_closure (action,
                                    MOO_CLOSURE (g_value_get_object (value)));
            break;
        case PROP_STOCK_ID:
            moo_action_set_stock_id (action,
                                     g_value_get_string (value));
            break;
        case PROP_LABEL:
            moo_action_set_label (action,
                                  g_value_get_string (value));
            break;
        case PROP_TOOLTIP:
            moo_action_set_tooltip (action,
                                    g_value_get_string (value));
            break;
        case PROP_NO_ACCEL:
            action->no_accel = g_value_get_boolean (value);
            break;
        case PROP_ACCEL:
            moo_action_set_accel (action,
                                  g_value_get_string (value));
            break;
        case PROP_DEFAULT_ACCEL:
            moo_action_set_default_accel (action,
                                          g_value_get_string (value));
            break;
        case PROP_ICON_STOCK_ID:
            moo_action_set_icon_stock_id (action,
                                          g_value_get_string (value));
            break;
        case PROP_ICON:
            moo_action_set_icon (action, GDK_PIXBUF (g_value_get_object (value)));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void moo_action_set_closure      (MooAction      *action,
                                         MooClosure     *closure)
{
    if (action->closure == closure) return;
    if (action->closure) g_object_unref (G_OBJECT (action->closure));
    action->closure = closure;
    if (action->closure) {
        g_object_ref (G_OBJECT (action->closure));
        gtk_object_sink (GTK_OBJECT (action->closure));
    }
}


static void moo_action_set_stock_id     (MooAction      *action,
                                         const char     *stock_id)
{
    if (!stock_id && !action->stock_id) return;
    if (stock_id && action->stock_id && !strcmp (stock_id, action->stock_id)) return;

    if (action->stock_id) {
        g_free (action->stock_id);
        action->stock_id = NULL;
    }

    if (stock_id) {
        GtkStockItem item;
        char *accel = NULL;

        g_return_if_fail (gtk_stock_lookup (stock_id, &item));
        g_free (action->stock_id);
        action->stock_id = g_strdup (stock_id);
        if (item.keyval || item.modifier) {
            accel = gtk_accelerator_name (item.keyval, item.modifier);
            moo_action_set_accel (action, accel);
            g_free (accel);
        }
        else
            moo_action_set_accel (action, "");
    }
    else {
        moo_action_set_accel (action, "");
    }
    g_object_notify (G_OBJECT (action), "stock_id");
}


static void moo_action_set_label        (MooAction      *action,
                                         const char     *label)
{
    g_free (action->label);
    action->label = g_strdup (label);
}


static void moo_action_set_tooltip      (MooAction      *action,
                                         const char     *tooltip)
{
    g_free (action->tooltip);
    action->tooltip = g_strdup (tooltip);
}


void moo_action_set_accel               (MooAction      *action,
                                         const char     *accel)
{
    const char *accel_path;
    guint accel_key = 0;
    GdkModifierType accel_mods = 0;
    GtkAccelKey old;

    g_return_if_fail (MOO_IS_ACTION (action));

    if (!action->constructed) {
        g_free (action->accel);
        action->accel = g_strdup (accel);
        return;
    }

    g_return_if_fail (accel != NULL);
    accel_path = moo_action_get_accel_path (action);

    if (accel[0])
    {
        gtk_accelerator_parse (accel, &accel_key, &accel_mods);

        if (accel_key || accel_mods) {
            g_hash_table_insert (accel_map,
                                 g_strdup (accel_path),
                                 gtk_accelerator_name (accel_key, accel_mods));
        }
        else {
            g_warning ("could not parse accelerator '%s'", accel);
            g_hash_table_insert (accel_map,
                                 g_strdup (accel_path),
                                 g_strdup (""));
        }
    }
    else {
        g_hash_table_insert (accel_map,
                             g_strdup (accel_path),
                             g_strdup (""));
    }
    g_object_notify (G_OBJECT (action), "accel");

    if (gtk_accel_map_lookup_entry (accel_path, &old))
    {
        if (accel_key == old.accel_key && accel_mods == old.accel_mods)
            return;

        if (accel_key || accel_mods)
        {
            if (!gtk_accel_map_change_entry (accel_path, accel_key,
                                             accel_mods, TRUE))
                g_warning ("could not set accel '%s' for accel_path '%s'",
                           accel, accel_path);
        }
        else
        {
            gtk_accel_map_change_entry (accel_path, 0, 0, TRUE);
        }
    }
    else
    {
        if (accel_key || accel_mods)
        {
            gtk_accel_map_add_entry (accel_path,
                                     accel_key,
                                     accel_mods);
        }
    }
}


void moo_action_set_default_accel       (MooAction      *action,
                                         const char     *accel)
{
    const char *accel_path;
    const char *old_accel;

    g_return_if_fail (MOO_IS_ACTION (action));

    if (!action->constructed) {
        g_free (action->default_accel);
        action->default_accel = g_strdup (accel);
        return;
    }

    g_return_if_fail (accel != NULL);
    accel_path = moo_action_get_accel_path (action);
    old_accel = g_hash_table_lookup (default_accel_map, accel_path);
    if (old_accel && !strcmp (old_accel, accel))
        return;

    if (accel[0]) {
        guint accel_key = 0;
        GdkModifierType accel_mods = 0;

        gtk_accelerator_parse (accel, &accel_key, &accel_mods);

        if (accel_key || accel_mods) {
            g_hash_table_insert (default_accel_map,
                                 g_strdup (accel_path),
                                 gtk_accelerator_name (accel_key, accel_mods));
            g_object_notify (G_OBJECT (action), "default-accel");
        }
        else {
            g_warning ("could not parse accelerator '%s'", accel);
        }
    }
    else {
        g_hash_table_insert (default_accel_map,
                             g_strdup (accel_path),
                             g_strdup (""));
        g_object_notify (G_OBJECT (action), "default-accel");
    }
}


const char  *moo_action_get_accel           (MooAction      *action)
{
    g_return_val_if_fail (MOO_IS_ACTION (action), NULL);
    return g_hash_table_lookup (accel_map,
                                moo_action_get_accel_path (action));
}

const char  *moo_action_get_default_accel   (MooAction      *action)
{
    g_return_val_if_fail (MOO_IS_ACTION (action), NULL);
    return g_hash_table_lookup (default_accel_map,
                                moo_action_get_accel_path (action));
}

char        *moo_action_get_accel_label     (MooAction      *action)
{
    const char *accel;
    guint key;
    GdkModifierType mods;

    g_return_val_if_fail (MOO_IS_ACTION (action), NULL);

    accel = moo_action_get_accel (action);
    if (!accel[0]) return g_strdup ("");
    gtk_accelerator_parse (accel, &key, &mods);
    return gtk_accelerator_get_label (key, mods);
}

char        *moo_action_get_default_accel_label (MooAction      *action)
{
    const char *accel;
    guint key;
    GdkModifierType mods;

    g_return_val_if_fail (MOO_IS_ACTION (action), NULL);

    accel = moo_action_get_default_accel (action);
    if (!accel[0]) return g_strdup ("");
    gtk_accelerator_parse (accel, &key, &mods);
    return gtk_accelerator_get_label (key, mods);
}


void         moo_action_set_no_accel        (MooAction      *action,
                                             gboolean        no_accel)
{
    g_return_if_fail (MOO_IS_ACTION (action));
    action->no_accel = no_accel;
}

gboolean     moo_action_get_no_accel        (MooAction      *action)
{
    g_return_val_if_fail (MOO_IS_ACTION (action), TRUE);
    return action->no_accel;
}


GtkWidget   *moo_action_create_menu_item    (MooAction      *action,
                                             GtkMenuShell   *menushell,
                                             int             position)
{
    MooActionClass *klass;

    g_return_val_if_fail (MOO_IS_ACTION (action), NULL);
    g_return_val_if_fail (!action->dead, NULL);

    klass = MOO_ACTION_GET_CLASS (action);
    g_return_val_if_fail (klass != NULL && klass->create_menu_item != NULL, NULL);
    return klass->create_menu_item (action, menushell, position);
}


gboolean     moo_action_create_tool_item    (MooAction      *action,
                                             GtkToolbar     *toolbar,
                                             int             position)
{
    MooActionClass *klass;

    g_return_val_if_fail (MOO_IS_ACTION (action), FALSE);
    g_return_val_if_fail (!action->dead, FALSE);

    klass = MOO_ACTION_GET_CLASS (action);
    g_return_val_if_fail (klass != NULL && klass->create_tool_item != NULL, FALSE);
    return klass->create_tool_item (action, toolbar, position);
}


const char  *moo_action_get_id        (MooAction      *action)
{
    g_return_val_if_fail (MOO_IS_ACTION (action), NULL);
    return action->id;
}

const char  *moo_action_get_group_id        (MooAction      *action)
{
    g_return_val_if_fail (MOO_IS_ACTION (action), NULL);
    return action->group_id;
}

const char  *moo_action_get_name            (MooAction      *action)
{
    g_return_val_if_fail (MOO_IS_ACTION (action), NULL);
    return action->name;
}


const char  *moo_action_get_accel_path  (MooAction      *action)
{
    g_return_val_if_fail (MOO_IS_ACTION (action), NULL);
    return moo_action_make_accel_path (action->group_id, action->id);
}


const char  *moo_action_make_accel_path (const char     *group_id,
                                         const char     *action_id)
{
    static char *result = NULL;
    g_free (result);
    result = NULL;

    g_return_val_if_fail (action_id != NULL && action_id[0] != 0, NULL);

    if (group_id && group_id[0])
        result = g_strdup_printf ("<MooAction>/%s/%s", group_id, action_id);
    else
        result = g_strdup_printf ("<MooAction>/%s", action_id);

    g_assert (result != NULL);
    return result;
}


static GtkWidget *moo_action_create_menu_item_real (MooAction      *action,
                                                    GtkMenuShell   *menu_shell,
                                                    int             position)
{
    GtkWidget *item = NULL;

    if (action->stock_id)
        item = gtk_image_menu_item_new_from_stock (action->stock_id,
                                                   NULL);
    else
    {
        GtkWidget *icon = NULL;
        if (action->icon_stock_id)
        {
            icon = gtk_image_new_from_stock (action->icon_stock_id, GTK_ICON_SIZE_MENU);
            if (!icon) g_warning ("could not create stock icon '%s'",
                                action->icon_stock_id);
            else gtk_widget_show (icon);
        }
        item = gtk_image_menu_item_new_with_mnemonic (action->label);
        if (icon) gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item), icon);
    }

    if (!action->no_accel)
        gtk_menu_item_set_accel_path (GTK_MENU_ITEM (item),
                                      moo_action_get_accel_path (action));

    moo_action_add_proxy (action, item);

    if (position >= 0)
        gtk_menu_shell_insert (menu_shell, item, position);
    else
        gtk_menu_shell_append (menu_shell, item);

    return item;
}


static gboolean moo_action_create_tool_item_real (MooAction      *action,
                                                  GtkToolbar     *toolbar,
                                                  int             position)
{
#if GTK_MINOR_VERSION >= 4
    GtkToolItem *item = NULL;
    GtkTooltips *tooltips = NULL;

    if (action->stock_id)
    {
        item = gtk_tool_button_new_from_stock (action->stock_id);
    }
    else
    {
        GtkWidget *icon = NULL;
        if (action->icon_stock_id)
        {
            icon = gtk_image_new_from_stock (action->icon_stock_id,
                                             gtk_toolbar_get_icon_size (toolbar));
            if (!icon) g_warning ("could not create stock icon '%s'",
                                  action->icon_stock_id);
            else gtk_widget_show (icon);
        }
        item = gtk_tool_button_new (icon, action->label);
        gtk_tool_button_set_use_underline (GTK_TOOL_BUTTON (item), TRUE);
    }

    if (action->group)
        tooltips = moo_action_group_get_tooltips (MOO_ACTION_GROUP (action->group));
    if (tooltips && action->tooltip)
        gtk_tool_item_set_tooltip (item,
                                   tooltips,
                                   action->tooltip,
                                   action->tooltip);

    gtk_toolbar_insert (toolbar, item, position);
    gtk_container_child_set (GTK_CONTAINER (toolbar), GTK_WIDGET (item),
                             "homogeneous", FALSE, NULL);

#else /*  GTK_MINOR_VERSION < 4 */

    GtkWidget *item = NULL;
    if (action->stock_id)
    {
        item = gtk_toolbar_insert_stock (toolbar,
                                         action->stock_id,
                                         action->tooltip,
                                         action->tooltip,
                                         (GtkSignalFunc)tool_item_callback,
                                         action,
                                         position);
    }
    else
    {
        GtkWidget *icon = NULL;
        if (action->icon_stock_id)
        {
            icon = gtk_image_new_from_stock (action->icon_stock_id,
                                             gtk_toolbar_get_icon_size (toolbar));
            if (!icon) g_warning ("could not create stock icon '%s'",
                                  action->icon_stock_id);
            else gtk_widget_show (icon);
        }
        item = gtk_toolbar_insert_item (toolbar,
                                        action->label,
                                        action->tooltip,
                                        action->tooltip,
                                        icon,
                                        NULL,
                                        NULL,
                                        position);
        gtk_button_set_use_underline (GTK_BUTTON (item), TRUE);
    }

#endif /* GTK_MINOR_VERSION < 4 */

    moo_action_add_proxy (action, GTK_WIDGET (item));

    return TRUE;
}


void         moo_action_activate        (MooAction      *action)
{
    g_return_if_fail (MOO_IS_ACTION (action));
    g_signal_emit (action, signals[ACTIVATE], 0);
}


void         moo_action_set_sensitive   (MooAction      *action,
                                         gboolean        sensitive)
{
    g_return_if_fail (MOO_IS_ACTION (action));
    g_signal_emit (action, signals[SET_SENSITIVE], 0, sensitive);
}

void         moo_action_set_visible    (MooAction      *action,
                                        gboolean        visible)
{
    g_return_if_fail (MOO_IS_ACTION (action));
    g_signal_emit (action, signals[SET_VISIBLE], 0, visible);
}


static void         moo_action_set_sensitive_real   (MooAction      *action,
                                                     gboolean        sensitive)
{
    g_return_if_fail (MOO_IS_ACTION (action));
    action->sensitive = sensitive;
    g_object_notify (G_OBJECT (action), "sensitive");
}

static void         moo_action_set_visible_real     (MooAction      *action,
                                                     gboolean        visible)
{
    g_return_if_fail (MOO_IS_ACTION (action));
    action->visible = visible;
    g_object_notify (G_OBJECT (action), "visible");
}


static void moo_action_set_icon_stock_id(MooAction      *action,
                                         const char     *stock_id)
{
    if (action->icon_stock_id == stock_id) return;
    g_free (action->icon_stock_id);
    action->icon_stock_id = g_strdup (stock_id);
    g_object_notify (G_OBJECT (action), "icon-stock-id");
}


static void moo_action_set_icon         (G_GNUC_UNUSED MooAction      *action,
                                         GdkPixbuf      *icon)
{
    if (icon)
        g_warning ("%s: implement me", G_STRLOC);
}


/****************************************************************************/

static void set_visible_callback     (GtkWidget  *proxy,
                                      gboolean    visible);
static void proxy_destroyed          (MooAction  *action,
                                      gpointer    proxy);
static void action_destroyed         (GtkWidget  *proxy,
                                      gpointer    action);

static void set_visible_callback     (GtkWidget  *proxy,
                                      gboolean    visible)
{
    if (visible)
        gtk_widget_show (proxy);
    else
        gtk_widget_hide (proxy);
}

static void proxy_destroyed          (MooAction  *action,
                                      gpointer    proxy)
{
    g_signal_handlers_disconnect_by_func (action, (gpointer)gtk_widget_set_sensitive, proxy);
    g_signal_handlers_disconnect_by_func (action, (gpointer)set_visible_callback, proxy);
    g_object_weak_unref (G_OBJECT (action), (GWeakNotify)action_destroyed, proxy);
}

static void action_destroyed         (GtkWidget  *proxy,
                                      gpointer    action)
{
    g_signal_handlers_disconnect_by_func (proxy, (gpointer)moo_action_activate, action);
    g_object_weak_unref (G_OBJECT (proxy), (GWeakNotify)proxy_destroyed, action);
}


static void moo_action_add_proxy        (MooAction      *action,
                                         GtkWidget      *proxy)
{
    gtk_widget_set_sensitive (proxy, action->sensitive);

    if (action->visible)
        gtk_widget_show (proxy);
    else
        gtk_widget_hide (proxy);

    if (GTK_IS_MENU_ITEM (proxy))
        g_signal_connect_swapped (proxy, "activate",
                                  G_CALLBACK (moo_action_activate),
                                  action);
    else if (
#if GTK_MINOR_VERSION >= 4
        GTK_IS_TOOL_BUTTON (proxy) ||
#endif /* GTK_MINOR_VERSION >= 4 */
        GTK_IS_BUTTON (proxy))
    {
        g_signal_connect_swapped (proxy, "clicked",
                                  G_CALLBACK (moo_action_activate),
                                  action);
    }
    else
    {
        g_critical ("unknown proxy type");
    }

    g_signal_connect_swapped (action, "set-sensitive",
                              G_CALLBACK (gtk_widget_set_sensitive),
                              proxy);
    g_signal_connect_swapped (action, "set-visible",
                              G_CALLBACK (set_visible_callback),
                              proxy);

    g_object_weak_ref (G_OBJECT (proxy),
                       (GWeakNotify)proxy_destroyed,
                       action);
    g_object_weak_ref (G_OBJECT (action),
                       (GWeakNotify)action_destroyed,
                       proxy);
}


const char  *moo_action_get_path            (MooAction      *action)
{
    static char *path = NULL;

    g_return_val_if_fail (MOO_IS_ACTION (action), NULL);

    g_free (path);
    path = g_strdup_printf ("%s::%s", action->group_id, action->id);
    return path;
}
