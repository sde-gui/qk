/*
 *   moouseractions.c
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

#include "mooutils/moouseractions.h"
#include "mooscript/mooscript-parser.h"
#include "mooutils/moocompat.h"
#include "mooutils/mooconfig.h"
#include <string.h>

typedef MooUserActionSetup SetupFunc;

typedef struct {
    char *name;
    char *label;
    char *accel;
    MooCommand *cmd;
    SetupFunc setup;
} Action;

typedef struct {
    MooAction base;
    MooWindow *window;
    MooCommand *cmd;
    SetupFunc setup;
} MooUserAction;

typedef struct {
    MooActionClass base_class;
} MooUserActionClass;


GType   _moo_user_action_get_type   (void) G_GNUC_CONST;


G_DEFINE_TYPE (MooUserAction, _moo_user_action, MOO_TYPE_ACTION)


static void
action_free (Action *action)
{
    if (action)
    {
        g_free (action->name);
        g_free (action->label);
        g_free (action->accel);
        if (action->cmd)
            g_object_unref (action->cmd);
        g_free (action);
    }
}


static char *
parse_accel (const char *string)
{
    if (!string || !string[0])
        return NULL;

    return g_strdup (string);
}


static Action *
action_new (const char *name,
            const char *label,
            const char *accel,
            MooCommandType cmd_type,
            const char *code,
            MooUserActionSetup setup)
{
    Action *action;
    MooCommand *cmd = NULL;

    g_return_val_if_fail (name != NULL, NULL);

    cmd = moo_command_new (cmd_type, code);
    g_return_val_if_fail (cmd != NULL, NULL);

    action = g_new0 (Action, 1);
    action->name = g_strdup (name);
    action->label = label ? g_strdup (label) : g_strdup (name);
    action->accel = parse_accel (accel);
    action->cmd = cmd;
    action->setup = setup;

    return action;
}


static MooAction *
create_action (MooWindow *window,
               Action    *data)
{
    MooUserAction *action;

    g_return_val_if_fail (data != NULL, NULL);

    action = g_object_new (_moo_user_action_get_type (),
                           "name", data->name,
                           "accel", data->accel,
                           "label", data->label,
                           NULL);

    action->window = window;
    action->cmd = g_object_ref (data->cmd);
    action->setup = data->setup;

    return MOO_ACTION (action);
}


void
moo_parse_user_actions (const char             *filename,
                        MooUserActionSetup      setup)
{
    guint n_items, i;
    MooConfig *config;
    MooWindowClass *klass;

    g_return_if_fail (filename != NULL);

    config = moo_config_parse_file (filename);

    if (!config)
        return;

    n_items = moo_config_n_items (config);
    klass = g_type_class_ref (MOO_TYPE_WINDOW);

    for (i = 0; i < n_items; ++i)
    {
        Action *action;
        const char *name, *label, *accel, *code, *type;
        MooCommandType cmd_type = 0;
        MooConfigItem *item = moo_config_nth_item (config, i);

        name = moo_config_item_get_value (item, "action");
        label = moo_config_item_get_value (item, "label");
        accel = moo_config_item_get_value (item, "accel");
        type = moo_config_item_get_value (item, "command");
        code = moo_config_item_get_content (item);

        if (!name)
        {
            g_warning ("%s: action name missing", G_STRLOC);
            continue;
        }

        if (!label)
            label = name;

        if (code)
        {
            if (!type)
                cmd_type = MOO_COMMAND_SCRIPT;
            else
                cmd_type = moo_command_type_parse (type);

            if (!cmd_type)
            {
                g_warning ("%s: unknown command type '%s'", G_STRLOC, type);
                continue;
            }
        }

        action = action_new (name, label, accel,
                             cmd_type, code, setup);

        if (action)
            moo_window_class_new_action_custom (klass, action->name,
                                                (MooWindowActionFunc) create_action,
                                                action, (GDestroyNotify) action_free);
    }

    moo_config_free (config);
    g_type_class_unref (klass);
}


static void
_moo_user_action_init (G_GNUC_UNUSED MooUserAction *action)
{
}


static void
moo_user_action_finalize (GObject *object)
{
    MooUserAction *action = (MooUserAction*) object;
    g_object_unref (action->cmd);
    G_OBJECT_CLASS(_moo_user_action_parent_class)->finalize (object);
}


static void
moo_user_action_activate (MooAction *_action)
{
    MooUserAction *action = (MooUserAction*) _action;

    g_return_if_fail (action->cmd != NULL);

    moo_command_set_window (action->cmd, action->window);

    if (action->setup)
        action->setup (action->cmd, action->window);

    moo_command_run (action->cmd);

    moo_command_set_context (action->cmd, NULL);
    moo_command_set_py_dict (action->cmd, NULL);
    moo_command_set_shell_env (action->cmd, NULL);
    moo_command_set_window (action->cmd, NULL);
}


static void
_moo_user_action_class_init (MooUserActionClass *klass)
{
    G_OBJECT_CLASS(klass)->finalize = moo_user_action_finalize;
    MOO_ACTION_CLASS(klass)->activate = moo_user_action_activate;
}
