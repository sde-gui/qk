/*
 *   mooeditaction-factory.c
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

#define MOOEDIT_COMPILATION
#include "mooedit/mooeditaction-factory.h"
#include "mooedit/mooeditaction.h"
#include "mooedit/mooedit-private.h"
#include "mooutils/mooutils-gobject.h"
#include "mooutils/mooactionfactory.h"
#include "mooutils/mooactionbase.h"
#include "mooutils/moocompat.h"
#include <string.h>
#include <gobject/gvaluecollector.h>


static void moo_edit_add_action                 (MooEdit            *edit,
                                                 GtkAction          *action);
static void moo_edit_remove_action              (MooEdit            *edit,
                                                 const char         *action_id);

static void moo_edit_class_new_actionv          (MooEditClass       *klass,
                                                 const char         *id,
                                                 const char         *first_prop_name,
                                                 va_list             props);
static void moo_edit_class_new_action_custom    (MooEditClass       *klass,
                                                 const char         *id,
                                                 MooEditActionFunc   func,
                                                 gpointer            data,
                                                 GDestroyNotify      notify);


#define MOO_EDIT_ACTIONS_QUARK (moo_edit_get_actions_quark ())

typedef struct {
    MooActionFactory *action;
    char **conditions;
} ActionInfo;


static GQuark
moo_edit_get_actions_quark (void)
{
    static GQuark q;

    if (!q)
        q = g_quark_from_static_string ("moo-edit-actions");

    return q;
}


static ActionInfo*
action_info_new (MooActionFactory  *action,
                 char             **conditions)
{
    ActionInfo *info;

    g_return_val_if_fail (MOO_IS_ACTION_FACTORY (action), NULL);

    info = g_new0 (ActionInfo, 1);
    info->action = g_object_ref (action);
    info->conditions = g_strdupv (conditions);

    return info;
}


static void
action_info_free (ActionInfo *info)
{
    if (info)
    {
        g_object_unref (info->action);
        g_strfreev (info->conditions);
        g_free (info);
    }
}


static void
moo_edit_add_action (MooEdit   *edit,
                     GtkAction *action)
{
    GtkActionGroup *group;

    g_return_if_fail (MOO_IS_EDIT (edit));
    g_return_if_fail (GTK_IS_ACTION (action));

    group = moo_edit_get_actions (edit);
    gtk_action_group_add_action (group, action);
}


static void
moo_edit_remove_action (MooEdit    *edit,
                        const char *action_id)
{
    GtkActionGroup *group;
    GtkAction *action;

    g_return_if_fail (MOO_IS_EDIT (edit));
    g_return_if_fail (action_id != NULL);

    group = moo_edit_get_actions (edit);
    action = gtk_action_group_get_action (group, action_id);
    gtk_action_group_remove_action (group, action);
}


static GtkAction*
create_action (const char *action_id,
               ActionInfo *info,
               MooEdit    *edit)
{
    GtkAction *action;

    g_return_val_if_fail (info != NULL, NULL);
    g_return_val_if_fail (MOO_IS_ACTION_FACTORY (info->action), NULL);
    g_return_val_if_fail (action_id && action_id[0], NULL);

    if (g_type_is_a (info->action->action_type, MOO_TYPE_ACTION))
        action = moo_action_factory_create_action (info->action, edit,
                                                   "closure-object", edit,
                                                   "name", action_id,
                                                   NULL);
    else if (g_type_is_a (info->action->action_type, MOO_TYPE_TOGGLE_ACTION))
        action = moo_action_factory_create_action (info->action, edit,
                                                   "toggled-object", edit,
                                                   "name", action_id,
                                                   NULL);
    else
        action = moo_action_factory_create_action (info->action, edit,
                                                   "name", action_id,
                                                   NULL);

    g_return_val_if_fail (action != NULL, NULL);

    if (g_type_is_a (info->action->action_type, MOO_TYPE_EDIT_ACTION))
        g_object_set (action, "doc", edit, NULL);

    if (info->conditions)
    {
        char **p;

        for (p = info->conditions; *p != NULL; p += 2)
        {
            if (p[1][0] == '!')
                moo_bind_bool_property (action, p[0], edit, p[1] + 1, TRUE);
            else
                moo_bind_bool_property (action, p[0], edit, p[1], FALSE);
        }
    }

    return action;
}


void
moo_edit_class_new_action (MooEditClass       *klass,
                           const char         *id,
                           const char         *first_prop_name,
                           ...)
{
    va_list args;
    va_start (args, first_prop_name);
    moo_edit_class_new_actionv (klass, id, first_prop_name, args);
    va_end (args);
}


static void
moo_edit_class_install_action (MooEditClass     *klass,
                               const char         *action_id,
                               MooActionFactory   *action,
                               char              **conditions)
{
    GHashTable *actions;
    ActionInfo *info;
    GType type;
    GSList *l;

    g_return_if_fail (MOO_IS_EDIT_CLASS (klass));
    g_return_if_fail (MOO_IS_ACTION_FACTORY (action));
    g_return_if_fail (action_id && action_id[0]);

    type = G_OBJECT_CLASS_TYPE (klass);
    actions = g_type_get_qdata (type, MOO_EDIT_ACTIONS_QUARK);

    if (!actions)
    {
        actions = g_hash_table_new_full (g_str_hash, g_str_equal,
                                         g_free, (GDestroyNotify) action_info_free);
        g_type_set_qdata (type, MOO_EDIT_ACTIONS_QUARK, actions);
    }

    if (g_hash_table_lookup (actions, action_id))
        moo_edit_class_remove_action (klass, action_id);

    info = action_info_new (action, conditions);
    g_hash_table_insert (actions, g_strdup (action_id), info);

    for (l = _moo_edit_instances; l != NULL; l = l->next)
    {
        if (g_type_is_a (G_OBJECT_TYPE (l->data), type))
        {
            GtkAction *action = create_action (action_id, info, l->data);

            if (action)
                moo_edit_add_action (l->data, action);
        }
    }
}


static void
moo_edit_class_new_actionv (MooEditClass       *klass,
                            const char         *action_id,
                            const char         *first_prop_name,
                            va_list             var_args)
{
    const char *name;
    GType action_type = 0;
    GObjectClass *action_class = NULL;
    GArray *action_params = NULL;
    GPtrArray *conditions = NULL;

    g_return_if_fail (MOO_IS_EDIT_CLASS (klass));
    g_return_if_fail (first_prop_name != NULL);
    g_return_if_fail (action_id != NULL);

    action_params = g_array_new (FALSE, TRUE, sizeof (GParameter));
    conditions = g_ptr_array_new ();

    name = first_prop_name;
    while (name)
    {
        GParameter param = {NULL, {0, {{0}, {0}}}};
        GParamSpec *pspec;
        char *err = NULL;

        /* ignore id property */
        if (!strcmp (name, "id") || !strcmp (name, "name"))
        {
            g_critical ("%s: id property specified", G_STRLOC);
            goto error;
        }

        if (!strcmp (name, "action-type::") || !strcmp (name, "action_type::"))
        {
            g_value_init (&param.value, MOO_TYPE_GTYPE);
            G_VALUE_COLLECT (&param.value, var_args, 0, &err);

            if (err)
            {
                g_warning ("%s: %s", G_STRLOC, err);
                g_free (err);
                goto error;
            }

            action_type = _moo_value_get_gtype (&param.value);

            if (!g_type_is_a (action_type, MOO_TYPE_ACTION_BASE))
            {
                g_warning ("%s: invalid action type %s", G_STRLOC, g_type_name (action_type));
                goto error;
            }

            action_class = g_type_class_ref (action_type);
        }
        else if (!strncmp (name, "condition::", strlen ("condition::")))
        {
            const char *suffix = strstr (name, "::");

            if (!suffix || !suffix[1] || !suffix[2])
            {
                g_warning ("%s: invalid condition name '%s'", G_STRLOC, name);
                goto error;
            }

            g_ptr_array_add (conditions, g_strdup (suffix + 2));

            name = va_arg (var_args, gchar*);

            if (!name)
            {
                g_warning ("%s: unterminated '%s' property",
                           G_STRLOC,
                           (char*) g_ptr_array_index (conditions, conditions->len - 1));
                goto error;
            }

            g_ptr_array_add (conditions, g_strdup (name));
        }
        else
        {
            if (!action_class)
            {
                if (!action_type)
                    action_type = MOO_TYPE_EDIT_ACTION;
                action_class = g_type_class_ref (action_type);
            }

            pspec = g_object_class_find_property (action_class, name);

            if (!pspec)
            {
                g_warning ("%s: no property '%s' in class '%s'",
                           G_STRLOC, name, g_type_name (action_type));
                goto error;
            }

            g_value_init (&param.value, G_PARAM_SPEC_VALUE_TYPE (pspec));
            G_VALUE_COLLECT (&param.value, var_args, 0, &err);

            if (err)
            {
                g_warning ("%s: %s", G_STRLOC, err);
                g_free (err);
                g_value_unset (&param.value);
                goto error;
            }

            param.name = g_strdup (name);
            g_array_append_val (action_params, param);
        }

        name = va_arg (var_args, gchar*);
    }

    G_STMT_START
    {
        MooActionFactory *action_factory = NULL;

        action_factory = moo_action_factory_new_a (action_type,
                                                   (GParameter*) action_params->data,
                                                   action_params->len);

        if (!action_factory)
        {
            g_warning ("%s: error in moo_object_factory_new_a()", G_STRLOC);
            goto error;
        }

        _moo_param_array_free ((GParameter*) action_params->data, action_params->len);
        g_array_free (action_params, FALSE);
        action_params = NULL;

        g_ptr_array_add (conditions, NULL);

        moo_edit_class_install_action (klass,
                                       action_id,
                                       action_factory,
                                       (char**) conditions->pdata);

        g_strfreev ((char**) conditions->pdata);
        g_ptr_array_free (conditions, FALSE);

        if (action_class)
            g_type_class_unref (action_class);
        if (action_factory)
            g_object_unref (action_factory);

        return;
    }
    G_STMT_END;

error:
    if (action_params)
    {
        guint i;
        GParameter *params = (GParameter*) action_params->data;

        for (i = 0; i < action_params->len; ++i)
        {
            g_value_unset (&params[i].value);
            g_free ((char*) params[i].name);
        }

        g_array_free (action_params, TRUE);
    }

    if (conditions)
    {
        guint i;
        for (i = 0; i < conditions->len; ++i)
            g_free (g_ptr_array_index (conditions, i));
        g_ptr_array_free (conditions, TRUE);
    }

    if (action_class)
        g_type_class_unref (action_class);
}


static GtkAction *
custom_action_factory_func (MooEdit          *edit,
                            MooActionFactory *factory)
{
    const char *action_id;
    MooEditActionFunc func;
    gpointer func_data;

    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);

    action_id = g_object_get_data (G_OBJECT (factory), "moo-edit-class-action-id");
    func = g_object_get_data (G_OBJECT (factory), "moo-edit-class-action-func");
    func_data = g_object_get_data (G_OBJECT (factory), "moo-edit-class-action-func-data");

    g_return_val_if_fail (action_id != NULL, NULL);
    g_return_val_if_fail (func != NULL, NULL);

    return func (edit, func_data);
}


static void
moo_edit_class_new_action_custom (MooEditClass       *klass,
                                  const char         *action_id,
                                  MooEditActionFunc   func,
                                  gpointer            data,
                                  GDestroyNotify      notify)
{
    MooActionFactory *action_factory;

    g_return_if_fail (MOO_IS_EDIT_CLASS (klass));
    g_return_if_fail (action_id && action_id[0]);
    g_return_if_fail (func != NULL);

    action_factory = moo_action_factory_new_func ((MooActionFactoryFunc) custom_action_factory_func, NULL);
    g_object_set_data (G_OBJECT (action_factory), "moo-edit-class", klass);
    g_object_set_data_full (G_OBJECT (action_factory), "moo-edit-class-action-id",
                            g_strdup (action_id), g_free);
    g_object_set_data (G_OBJECT (action_factory), "moo-edit-class-action-func", func);
    g_object_set_data_full (G_OBJECT (action_factory), "moo-edit-class-action-func-data",
                            data, notify);

    moo_edit_class_install_action (klass, action_id, action_factory, NULL);
    g_object_unref (action_factory);
}


static GtkAction *
type_action_func (MooEdit  *edit,
                  gpointer  klass)
{
    GQuark quark = g_quark_from_static_string ("moo-edit-class-action-id");
    const char *id = g_type_get_qdata (G_TYPE_FROM_CLASS (klass), quark);
    return g_object_new (G_TYPE_FROM_CLASS (klass),
                         "doc", edit, "name", id, NULL);
}


void
moo_edit_class_new_action_type (MooEditClass *edit_klass,
                                const char   *id,
                                GType         type)
{
    gpointer klass;
    GQuark quark;

    g_return_if_fail (g_type_is_a (type, MOO_TYPE_EDIT_ACTION));

    klass = g_type_class_ref (type);
    g_return_if_fail (klass != NULL);

    quark = g_quark_from_static_string ("moo-edit-class-action-id");
    g_free (g_type_get_qdata (type, quark));
    g_type_set_qdata (type, quark, g_strdup (id));
    moo_edit_class_new_action_custom (edit_klass, id, type_action_func,
                                      klass, g_type_class_unref);
}


void
moo_edit_class_remove_action (MooEditClass *klass,
                              const char   *action_id)
{
    GHashTable *actions;
    GType type;
    GSList *l;

    g_return_if_fail (MOO_IS_EDIT_CLASS (klass));

    type = G_OBJECT_CLASS_TYPE (klass);
    actions = g_type_get_qdata (type, MOO_EDIT_ACTIONS_QUARK);

    if (actions)
        g_hash_table_remove (actions, action_id);

    for (l = _moo_edit_instances; l != NULL; l = l->next)
        if (g_type_is_a (G_OBJECT_TYPE (l->data), type))
            moo_edit_remove_action (l->data, action_id);
}


GtkActionGroup *
moo_edit_get_actions (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    return moo_action_collection_get_group (edit->priv->actions, NULL);
}


GtkAction *
moo_edit_get_action_by_id (MooEdit    *edit,
                           const char *action_id)
{
    GtkActionGroup *actions;

    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    g_return_val_if_fail (action_id != NULL, NULL);

    actions = moo_edit_get_actions (edit);
    return gtk_action_group_get_action (actions, action_id);
}


static void
add_action (const char *id,
            ActionInfo *info,
            MooEdit    *edit)
{
    GtkAction *action = create_action (id, info, edit);

    if (action)
        moo_edit_add_action (edit, action);
}

void
_moo_edit_add_class_actions (MooEdit *edit)
{
    GType type;

    g_return_if_fail (MOO_IS_EDIT (edit));

    type = G_OBJECT_TYPE (edit);

    while (TRUE)
    {
        GHashTable *actions;

        actions = g_type_get_qdata (type, MOO_EDIT_ACTIONS_QUARK);

        if (actions)
            g_hash_table_foreach (actions, (GHFunc) add_action, edit);

        if (type == MOO_TYPE_EDIT)
            break;

        type = g_type_parent (type);
    }
}


void
_moo_edit_class_init_actions (MooEditClass *klass)
{
    moo_edit_class_new_action (klass, "Undo",
                               "display-name", "Undo",
                               "label", "_Undo",
                               "tooltip", "Undo",
                               "stock-id", GTK_STOCK_UNDO,
                               "closure-signal", "undo",
                               "condition::sensitive", "can-undo",
                               NULL);

    moo_edit_class_new_action (klass, "Redo",
                               "display-name", "Redo",
                               "label", "_Redo",
                               "tooltip", "Redo",
                               "stock-id", GTK_STOCK_REDO,
                               "closure-signal", "redo",
                               "condition::sensitive", "can-redo",
                               NULL);

    moo_edit_class_new_action (klass, "Cut",
                               "display-name", "Cut",
                               "stock-id", GTK_STOCK_CUT,
                               "closure-signal", "cut-clipboard",
                               "condition::sensitive", "has-selection",
                               NULL);

    moo_edit_class_new_action (klass, "Copy",
                               "display-name", "Copy",
                               "stock-id", GTK_STOCK_COPY,
                               "closure-signal", "copy-clipboard",
                               "condition::sensitive", "has-selection",
                               NULL);

    moo_edit_class_new_action (klass, "Paste",
                               "display-name", "Paste",
                               "stock-id", GTK_STOCK_PASTE,
                               "closure-signal", "paste-clipboard",
                               NULL);

    moo_edit_class_new_action (klass, "SelectAll",
                               "display-name", "Select All",
                               "label", "Select _All",
                               "tooltip", "Select all",
                               "stock-id", GTK_STOCK_SELECT_ALL,
                               "closure-callback", moo_text_view_select_all,
                               "condition::sensitive", "has-text",
                               NULL);
}
