/*
 *   mooui/mooactiongroup.c
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

#include "mooui/mooactiongroup.h"
#include "mooutils/moocompat.h"


struct _MooActionGroupPrivate {
    GHashTable      *actions;       /* char* -> MooAction* */
    GtkAccelGroup   *accel_group;
    char            *name;
};


static void moo_action_group_class_init     (MooActionGroupClass    *klass);

static void moo_action_group_init           (MooActionGroup         *group);
static void moo_action_group_finalize       (GObject                *object);

static void moo_action_group_add_action_priv (MooActionGroup        *group,
                                              MooAction             *action);


enum {
    PROP_0
};

enum {
    LAST_SIGNAL
};


/* MOO_TYPE_ACTION_GROUP */
G_DEFINE_TYPE (MooActionGroup, moo_action_group, G_TYPE_OBJECT)


static void moo_action_group_class_init (MooActionGroupClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    gobject_class->finalize = moo_action_group_finalize;
}


static void moo_action_group_init (MooActionGroup *group)
{
    group->priv = g_new0 (MooActionGroupPrivate, 1);
    group->priv->actions =
        g_hash_table_new_full (g_str_hash, g_str_equal,
                               g_free, g_object_unref);
    group->priv->accel_group = NULL;
    group->priv->name = NULL;
}


static void moo_action_group_finalize       (GObject      *object)
{
    MooActionGroup *group = MOO_ACTION_GROUP (object);
    g_hash_table_destroy (group->priv->actions);
    g_free (group->priv->name);
    g_free (group->priv);
    group->priv = NULL;
    G_OBJECT_CLASS (moo_action_group_parent_class)->finalize (object);
}


#if 0
static void moo_action_group_set_property   (GObject        *object,
                                             guint           prop_id,
                                             const GValue   *value,
                                             GParamSpec     *pspec)
{
    g_return_if_fail (MOO_IS_ACTION_GROUP (object));
    MooActionGroup *group = MOO_ACTION_GROUP (object);

    switch (prop_id)
    {
        case PROP_ID:
            moo_action_group_set_id (group, g_value_get_string (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void moo_action_group_get_property   (GObject        *object,
                                             guint           prop_id,
                                             GValue         *value,
                                             GParamSpec     *pspec)
{
    g_return_if_fail (MOO_IS_ACTION_GROUP (object));
    MooActionGroup *group = MOO_ACTION_GROUP (object);
    g_return_if_fail (group->priv != NULL);

    switch (prop_id)
    {
        case PROP_ID:
            g_value_set_string (value, group->priv->id().c_str());
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


void moo_action_group_set_id         (MooActionGroup *group,
                                             const char     *id)
{
    if (!group->priv) group->priv = new MooActionGroupPrivate (group, id);
    else {
        group->priv->set_id (id);
        g_object_notify (G_OBJECT (group), "id");
    }
}
#endif /* 0 */


void             moo_action_group_add_action    (MooActionGroup *group,
                                                 MooAction      *action)
{
    g_return_if_fail (MOO_IS_ACTION_GROUP (group) && group->priv != NULL);
    g_return_if_fail (MOO_IS_ACTION (action));
    moo_action_group_add_action_priv (group, action);
}


MooActionGroup  *moo_action_group_new           (const char *name)
{
    MooActionGroup *group;

    g_return_val_if_fail (name != NULL, NULL);
    group = MOO_ACTION_GROUP (g_object_new (MOO_TYPE_ACTION_GROUP, NULL));
    group->priv->name = g_strdup (name);

    return group;
}


MooAction       *moo_action_group_get_action    (MooActionGroup *group,
                                                 const char     *action_id)
{
    g_return_val_if_fail (MOO_IS_ACTION_GROUP (group) && action_id != NULL, NULL);
    g_return_val_if_fail (group->priv != NULL, NULL);
    return g_hash_table_lookup (group->priv->actions, action_id);
}


void             moo_action_group_set_accel_group (MooActionGroup *group,
                                                   GtkAccelGroup  *accel_group)
{
    g_return_if_fail (MOO_IS_ACTION_GROUP (group) && group->priv != NULL);
    g_return_if_fail (!accel_group || GTK_IS_ACCEL_GROUP (accel_group));

    if (accel_group == group->priv->accel_group)
        return;
    if (group->priv->accel_group)
        g_object_unref (G_OBJECT (group->priv->accel_group));
    group->priv->accel_group = accel_group;
    if (accel_group)
        g_object_ref (G_OBJECT (accel_group));
}


typedef struct {
    MooActionGroupForeachFunc   func;
    gpointer                    data;
    gboolean                    stop;
    MooActionGroup             *group;
} ForeachData;

static void foreach_func (G_GNUC_UNUSED const char *action_id,
                          MooAction     *action,
                          ForeachData   *data)
{
    if (!data->stop)
        data->stop = data->func (data->group, action, data->data);
}

void             moo_action_group_foreach       (MooActionGroup             *group,
                                                 MooActionGroupForeachFunc   func,
                                                 gpointer                    data)
{
    ForeachData d = {func, data, FALSE, group};

    g_return_if_fail (MOO_IS_ACTION_GROUP (group) && group->priv != NULL);
    g_return_if_fail (func != NULL);

    g_hash_table_foreach (group->priv->actions,
                          (GHFunc) foreach_func, &d);
}


void             moo_action_group_set_name      (MooActionGroup *group,
                                                 const char     *name)
{
    g_return_if_fail (MOO_IS_ACTION_GROUP (group) && name != NULL);
    g_free (group->priv->name);
    group->priv->name = g_strdup (name);
}


const char      *moo_action_group_get_name      (MooActionGroup *group)
{
    g_return_val_if_fail (MOO_IS_ACTION_GROUP (group), NULL);
    return group->priv->name;
}


static void moo_action_group_add_action_priv (MooActionGroup    *group,
                                              MooAction         *action)
{
    const char *id = moo_action_get_id (action);
    if (g_hash_table_lookup (group->priv->actions, id)) {
        g_warning ("action with id '%s' already exists in action group '%s'\n",
                   id, group->priv->name);
    }

    g_hash_table_insert (group->priv->actions, g_strdup (id),
                         g_object_ref (action));
    /*TODO???*/
    action->group = group;
}


void
moo_action_group_remove_action (MooActionGroup *group,
                                const char     *action_id)
{
    g_return_if_fail (MOO_IS_ACTION_GROUP (group));
    g_return_if_fail (action_id != NULL);

    g_hash_table_remove (group->priv->actions, action_id);
}
