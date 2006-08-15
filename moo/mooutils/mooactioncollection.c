/*
 *   mooactioncollection.c
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

#include "mooutils/mooactiongroup.h"
#include "mooutils/mooactionbase.h"
#include <string.h>


struct _MooActionCollectionPrivate {
    MooActionGroup *default_group;
    GHashTable *groups; /* name -> MooActionGroup* */
    GSList *groups_list;
    char *name;
};

enum {
    PROP_0,
    PROP_NAME,
    PROP_DISPLAY_NAME
};


G_DEFINE_TYPE (MooActionCollection, moo_action_collection, G_TYPE_OBJECT)


static void
moo_action_collection_init (MooActionCollection *coll)
{
    coll->priv = g_new0 (MooActionCollectionPrivate, 1);
    coll->priv->groups = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
    coll->priv->default_group = _moo_action_group_new (coll, NULL, NULL);
    coll->priv->groups_list = g_slist_prepend (NULL, coll->priv->default_group);
}


static void
moo_action_collection_dispose (GObject *object)
{
    MooActionCollection *coll = MOO_ACTION_COLLECTION (object);

    if (coll->priv)
    {
        g_object_unref (coll->priv->default_group);
        g_hash_table_destroy (coll->priv->groups);
        g_slist_free (coll->priv->groups_list);
        g_free (coll->priv);
    }

    G_OBJECT_CLASS (moo_action_collection_parent_class)->dispose (object);
}


static void
moo_action_collection_set_property (GObject      *object,
                                    guint         property_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
    MooActionCollection *coll = MOO_ACTION_COLLECTION (object);
    char *tmp;

    switch (property_id)
    {
        case PROP_NAME:
            tmp = coll->priv->name;
            coll->priv->name = g_strdup (g_value_get_string (value));
            g_free (tmp);
            break;
        case PROP_DISPLAY_NAME:
            moo_action_collection_set_display_name (coll, g_value_get_string (value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}


static void
moo_action_collection_get_property (GObject    *object,
                                    guint       property_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
    MooActionCollection *coll = MOO_ACTION_COLLECTION (object);

    switch (property_id)
    {
        case PROP_NAME:
            g_value_set_string (value, moo_action_collection_get_name (coll));
            break;
        case PROP_DISPLAY_NAME:
            g_value_set_string (value, moo_action_collection_get_display_name (coll));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}


static void
moo_action_collection_class_init (MooActionCollectionClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = moo_action_collection_dispose;
    object_class->set_property = moo_action_collection_set_property;
    object_class->get_property = moo_action_collection_get_property;

    g_object_class_install_property (object_class, PROP_NAME,
                                     g_param_spec_string ("name", "name", "name",
                                                          NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (object_class, PROP_DISPLAY_NAME,
                                     g_param_spec_string ("display-name", "display-name", "display-name",
                                                          NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
}


MooActionCollection *
moo_action_collection_new (const char *name,
                           const char *display_name)
{
    return g_object_new (MOO_TYPE_ACTION_COLLECTION,
                         "name", name,
                         "display-name", display_name,
                         NULL);
}


const char *
moo_action_collection_get_name (MooActionCollection *coll)
{
    g_return_val_if_fail (MOO_IS_ACTION_COLLECTION (coll), NULL);
    return coll->priv->name;
}


void
moo_action_collection_set_display_name (MooActionCollection *coll,
                                        const char          *name)
{
    g_return_if_fail (MOO_IS_ACTION_COLLECTION (coll));
    _moo_action_group_set_display_name (coll->priv->default_group, name);
}


const char *
moo_action_collection_get_display_name (MooActionCollection *coll)
{
    g_return_val_if_fail (MOO_IS_ACTION_COLLECTION (coll), NULL);
    return _moo_action_group_get_display_name (coll->priv->default_group);
}


static GtkActionGroup *
get_group (MooActionCollection *coll,
           const char          *name)
{
    if (name)
        return g_hash_table_lookup (coll->priv->groups, name);
    else
        return GTK_ACTION_GROUP (coll->priv->default_group);
}


GtkActionGroup *
moo_action_collection_add_group (MooActionCollection *coll,
                                 const char          *name,
                                 const char          *display_name)
{
    MooActionGroup *group;

    g_return_val_if_fail (MOO_IS_ACTION_COLLECTION (coll), NULL);
    g_return_val_if_fail (name != NULL, NULL);
    g_return_val_if_fail (get_group (coll, name) == NULL, NULL);

    group = _moo_action_group_new (coll, name, display_name);
    g_hash_table_insert (coll->priv->groups, g_strdup (name), group);
    coll->priv->groups_list = g_slist_prepend (coll->priv->groups_list, group);

    return GTK_ACTION_GROUP (group);
}


void
moo_action_collection_remove_group (MooActionCollection *coll,
                                    GtkActionGroup      *group)
{
    const char *name;

    g_return_if_fail (MOO_IS_ACTION_COLLECTION (coll));
    g_return_if_fail (MOO_IS_ACTION_GROUP (group));

    name = gtk_action_group_get_name (group);
    g_return_if_fail (name != NULL);
    g_return_if_fail (group == get_group (coll, name));

    _moo_action_group_set_collection (MOO_ACTION_GROUP (group), NULL);
    g_hash_table_remove (coll->priv->groups, name);
    coll->priv->groups_list = g_slist_remove (coll->priv->groups_list, group);
}


GtkActionGroup *
moo_action_collection_get_group (MooActionCollection *coll,
                                 const char          *name)
{
    g_return_val_if_fail (MOO_IS_ACTION_COLLECTION (coll), NULL);
    return get_group (coll, name);
}


const GSList *
moo_action_collection_get_groups (MooActionCollection *coll)
{
    g_return_val_if_fail (MOO_IS_ACTION_COLLECTION (coll), NULL);
    return coll->priv->groups_list;
}


GtkAction *
moo_action_collection_get_action (MooActionCollection *coll,
                                  const char          *name)
{
    GSList *l;

    g_return_val_if_fail (MOO_IS_ACTION_COLLECTION (coll), NULL);
    g_return_val_if_fail (name != NULL, NULL);

    for (l = coll->priv->groups_list; l != NULL; l = l->next)
    {
        GtkActionGroup *group = l->data;
        GtkAction *action = gtk_action_group_get_action (group, name);
        if (name)
            return action;
    }

    return NULL;
}


void
moo_action_collection_remove_action (MooActionCollection *coll,
                                     GtkAction           *action)
{
    GtkActionGroup *group = NULL;

    g_return_if_fail (MOO_IS_ACTION_COLLECTION (coll));
    g_return_if_fail (GTK_IS_ACTION (action));

    g_object_get (action, "action-group", &group, NULL);
    g_return_if_fail (group != NULL);
    g_return_if_fail (g_slist_find (coll->priv->groups_list, group) != NULL);

    gtk_action_group_remove_action (group, action);
}


GList *
moo_action_collection_list_actions (MooActionCollection *coll)
{
    GList *list = NULL;
    GSList *l;

    g_return_val_if_fail (MOO_IS_ACTION_COLLECTION (coll), NULL);

    for (l = coll->priv->groups_list; l != NULL; l = l->next)
    {
        GtkActionGroup *group = l->data;
        list = g_list_concat (list, gtk_action_group_list_actions (group));
    }

    return list;
}
