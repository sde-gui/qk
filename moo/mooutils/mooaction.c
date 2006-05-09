/*
 *   mooaction.c
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

#include "mooutils/mooaction.h"
#include "mooutils/mooutils-gobject.h"
#include <gtk/gtktoggleaction.h>
#include <string.h>


static void
set_bool (gpointer    object,
          const char *key,
          gboolean    value)
{
    g_object_set_data (object, key, GINT_TO_POINTER (value));
}


static gboolean
get_bool (gpointer    object,
          const char *key)
{
    return g_object_get_data (object, key) != NULL;
}


static void
set_string (gpointer    object,
            const char *key,
            const char *value)
{
    if (value)
        g_object_set_data_full (object, key, g_strdup (value), g_free);
    else
        g_object_set_data (object, key, NULL);
}


static const char *
get_string (gpointer    object,
            const char *key)
{
    return g_object_get_data (object, key);
}


void
moo_action_set_no_accel (GtkAction *action,
                         gboolean   no_accel)
{
    g_return_if_fail (GTK_IS_ACTION (action));
    set_bool (action, "moo-action-no-accel", no_accel);
}


gboolean
moo_action_get_no_accel (GtkAction *action)
{
    g_return_val_if_fail (GTK_IS_ACTION (action), FALSE);
    return get_bool (action, "moo-action-no-accel");
}


void
_moo_action_set_force_accel_label (GtkAction *action,
                                   gboolean   force)
{
    g_return_if_fail (GTK_IS_ACTION (action));
    set_bool (action, "moo-action-force-accel-label", force);
}


gboolean
_moo_action_get_force_accel_label (GtkAction *action)
{
    g_return_val_if_fail (GTK_IS_ACTION (action), FALSE);
    return get_bool (action, "moo-action-force-accel-label");
}


const char *
moo_action_get_default_accel (GtkAction *action)
{
    const char *accel;

    g_return_val_if_fail (GTK_IS_ACTION (action), "");

    accel = get_string (action, "moo-action-default-accel");
    return accel ? accel : "";
}


void
moo_action_set_default_accel (GtkAction  *action,
                              const char *accel)
{
    g_return_if_fail (GTK_IS_ACTION (action));
    set_string (action, "moo-action-default-accel", accel);
}


void
_moo_action_set_accel_path (GtkAction  *action,
                            const char *accel_path)
{
    g_return_if_fail (GTK_IS_ACTION (action));
    set_string (action, "moo-action-accel-path", accel_path);

    if (!moo_action_get_no_accel (action))
        gtk_action_set_accel_path (action, accel_path);
}


const char *
_moo_action_get_accel_path (GtkAction *action)
{
    g_return_val_if_fail (GTK_IS_ACTION (action), NULL);
    return get_string (action, "moo-action-accel-path");
}


const char *
_moo_action_get_accel (GtkAction *action)
{
    const char *string;

    g_return_val_if_fail (GTK_IS_ACTION (action), "");

    if ((string = get_string (action, "moo-action-accel-path")))
        return string;
    else if ((string = get_string (action, "moo-action-default-accel")))
        return string;
    else
        return "";
}


const char *
_moo_action_make_accel_path (const char *group_id,
                             const char *action_id)
{
    static GString *path = NULL;

    g_return_val_if_fail (action_id != NULL && action_id[0] != 0, NULL);

    if (!path)
        path = g_string_new (NULL);

    if (group_id && group_id[0])
        g_string_printf (path, "<MooAction>/%s/%s", group_id, action_id);
    else
        g_string_printf (path, "<MooAction>/%s", action_id);

    return path->str;
}


const char *
moo_action_get_display_name (GtkAction *action)
{
    const char *name;
    g_return_val_if_fail (GTK_IS_ACTION (action), NULL);
    name = get_string (action, "moo-action-display-name");
    return name ? name : gtk_action_get_name (action);
}


void
moo_action_set_display_name (GtkAction  *action,
                             const char *name)
{
    g_return_if_fail (GTK_IS_ACTION (action));
    set_string (action, "moo-action-display-name", name);
}


gboolean
_moo_action_get_dead (GtkAction *action)
{
    g_return_val_if_fail (GTK_IS_ACTION (action), TRUE);
    return get_bool (action, "moo-action-dead");
}


void
_moo_action_set_dead (GtkAction *action,
                      gboolean   dead)
{
    g_return_if_fail (GTK_IS_ACTION (action));
    set_bool (action, "moo-action-dead", dead);
}


gboolean
_moo_action_get_has_submenu (GtkAction *action)
{
    g_return_val_if_fail (GTK_IS_ACTION (action), TRUE);
    return get_bool (action, "moo-action-has-submenu");
}


void
_moo_action_set_has_submenu (GtkAction *action,
                             gboolean   has_submenu)
{
    g_return_if_fail (GTK_IS_ACTION (action));
    set_bool (action, "moo-action-has-submenu", has_submenu);
}


static void
moo_action_activate (GtkAction *action)
{
    MooClosure *closure = _moo_action_get_closure (action);

    if (closure)
        moo_closure_invoke (closure);
}


void
_moo_action_set_closure (GtkAction  *action,
                         MooClosure *closure)
{
    g_return_if_fail (GTK_IS_ACTION (action));

    if (closure)
    {
        moo_closure_ref_sink (closure);
        g_object_set_data_full (G_OBJECT (action), "moo-action-closure",
                                closure, (GDestroyNotify) moo_closure_unref);

        if (!g_signal_handler_find (action, G_SIGNAL_MATCH_FUNC,
                                    0, 0, 0, moo_action_activate, NULL))
            g_signal_connect (action, "activate",
                              G_CALLBACK (moo_action_activate),
                              NULL);
    }
    else
    {
        g_object_set_data (G_OBJECT (action), "moo-action-closure", NULL);
    }
}


MooClosure *
_moo_action_get_closure (GtkAction *action)
{
    g_return_val_if_fail (GTK_IS_ACTION (action), NULL);
    return g_object_get_data (G_OBJECT (action), "moo-action-closure");
}


/**************************************************************************/
/* moo_sync_toggle_action
 */

typedef struct {
    MooObjectWatch parent;
    GParamSpec *pspec;
    gboolean invert;
} ToggleWatch;

static void action_toggled          (ToggleWatch    *watch);
static void prop_changed            (ToggleWatch    *watch);
static void toggle_watch_destroy    (MooObjectWatch *watch);

static MooObjectWatchClass ToggleWatchClass = {NULL, NULL, toggle_watch_destroy};


static ToggleWatch *
toggle_watch_new (GObject    *master,
                  const char *prop,
                  GtkAction  *action,
                  gboolean    invert)
{
    ToggleWatch *watch;
    GObjectClass *klass;
    GParamSpec *pspec;
    char *signal;

    g_return_val_if_fail (G_IS_OBJECT (master), NULL);
    g_return_val_if_fail (GTK_IS_TOGGLE_ACTION (action), NULL);
    g_return_val_if_fail (prop != NULL, NULL);

    klass = g_type_class_peek (G_OBJECT_TYPE (master));
    pspec = g_object_class_find_property (klass, prop);

    if (!pspec)
    {
        g_warning ("%s: no property '%s' in class '%s'",
                   G_STRLOC, prop, g_type_name (G_OBJECT_TYPE (master)));
        return NULL;
    }

    watch = moo_object_watch_new (ToggleWatch, &ToggleWatchClass,
                                  master, action, NULL, NULL);

    watch->pspec = pspec;
    watch->invert = invert;

    signal = g_strdup_printf ("notify::%s", prop);

    g_signal_connect_swapped (master, signal,
                              G_CALLBACK (prop_changed),
                              watch);
    g_signal_connect_swapped (action, "toggled",
                              G_CALLBACK (action_toggled),
                              watch);

    g_free (signal);
    return watch;
}


static void
toggle_watch_destroy (MooObjectWatch *watch)
{
    if (MOO_OBJECT_PTR_GET (watch->source))
    {
        g_signal_handlers_disconnect_by_func (MOO_OBJECT_PTR_GET (watch->source),
                                              (gpointer) prop_changed,
                                              watch);
    }

    if (MOO_OBJECT_PTR_GET (watch->target))
    {
        g_assert (GTK_IS_TOGGLE_ACTION (MOO_OBJECT_PTR_GET (watch->target)));
        g_signal_handlers_disconnect_by_func (MOO_OBJECT_PTR_GET (watch->target),
                                              (gpointer) action_toggled,
                                              watch);
    }
}


void
moo_sync_toggle_action (GtkAction  *action,
                        gpointer    master,
                        const char *prop,
                        gboolean    invert)
{
    ToggleWatch *watch;

    g_return_if_fail (GTK_IS_TOGGLE_ACTION (action));
    g_return_if_fail (G_IS_OBJECT (master));
    g_return_if_fail (prop != NULL);

    watch = toggle_watch_new (master, prop, action, invert);
    g_return_if_fail (watch != NULL);

    prop_changed (watch);
}


static void
prop_changed (ToggleWatch *watch)
{
    gboolean value, active, equal;
    gpointer action;

    g_object_get (MOO_OBJECT_PTR_GET (watch->parent.source),
                  watch->pspec->name, &value, NULL);

    action = MOO_OBJECT_PTR_GET (watch->parent.target);
    g_assert (GTK_IS_TOGGLE_ACTION (action));
    active = gtk_toggle_action_get_active (action);

    if (!watch->invert)
        equal = ((value && active) || (!value && !active));
    else
        equal = ((!value && active) || (value && !active));

    if (!equal)
        gtk_toggle_action_set_active (action, watch->invert ? !value : value);
}


static void
action_toggled (ToggleWatch *watch)
{
    gboolean value, active, equal;
    gpointer action;

    g_object_get (MOO_OBJECT_PTR_GET (watch->parent.source),
                  watch->pspec->name, &value, NULL);

    action = MOO_OBJECT_PTR_GET (watch->parent.target);
    g_assert (GTK_IS_TOGGLE_ACTION (action));
    active = gtk_toggle_action_get_active (action);

    if (!watch->invert)
        equal = ((value && active) || (!value && !active));
    else
        equal = ((!value && active) || (value && !active));

    if (!equal)
        g_object_set (MOO_OBJECT_PTR_GET (watch->parent.source),
                      watch->pspec->name,
                      watch->invert ? !active : active, NULL);
}
