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

typedef MooUserActionCtxFunc CtxFunc;

typedef struct {
    char *name;
    char *label;
    char *accel;
    MSNode *script;
    CtxFunc ctx_func;
} Action;

typedef struct {
    MooAction base;
    MooWindow *window;
    MSNode *script;
    CtxFunc ctx_func;
} MooUserAction;

typedef struct {
    MooActionClass base_class;
} MooUserActionClass;


GType       _moo_user_action_get_type   (void) G_GNUC_CONST;


G_DEFINE_TYPE (MooUserAction, _moo_user_action, MOO_TYPE_ACTION)


static void
action_free (Action *action)
{
    if (action)
    {
        g_free (action->name);
        g_free (action->label);
        g_free (action->accel);
        ms_node_unref (action->script);
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


static MSNode *
parse_code (const char *code)
{
    g_return_val_if_fail (code != NULL, NULL);
    return ms_script_parse (code);
}

static Action *
action_new (const char *name,
            const char *label,
            const char *accel,
            const char *code,
            CtxFunc     ctx_func)
{
    Action *action;
    MSNode *script;

    g_return_val_if_fail (name != NULL, NULL);
    g_return_val_if_fail (code != NULL, NULL);

    script = parse_code (code);

    if (!script)
    {
        g_warning ("could not parse script\n%s\n", code);
        return NULL;
    }

    action = g_new0 (Action, 1);
    action->name = g_strdup (name);
    action->label = label ? g_strdup (label) : g_strdup (name);
    action->accel = parse_accel (accel);
    action->script = script;
    action->ctx_func = ctx_func;

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
    action->script = ms_node_ref (data->script);
    action->ctx_func = data->ctx_func;

    return MOO_ACTION (action);
}


void
moo_parse_user_actions (const char          *filename,
                        MooUserActionCtxFunc ctx_func)
{
    guint n_items, i;
    MooConfig *config;
    MooWindowClass *klass;

    g_return_if_fail (filename != NULL);
    g_return_if_fail (ctx_func != NULL);

    config = moo_config_parse_file (filename);

    if (!config)
        return;

    n_items = moo_config_n_items (config);
    klass = g_type_class_ref (MOO_TYPE_WINDOW);

    for (i = 0; i < n_items; ++i)
    {
        Action *action;
        const char *name, *label, *accel, *code;
        MooConfigItem *item = moo_config_nth_item (config, i);

        name = moo_config_item_get_value (item, "action");
        label = moo_config_item_get_value (item, "label");
        accel = moo_config_item_get_value (item, "accel");
        code = moo_config_item_get_content (item);

        if (!name)
        {
            g_warning ("%s: action name missing", G_STRLOC);
            continue;
        }

        if (!label)
            label = name;

        if (!code)
        {
            g_warning ("%s: code missing", G_STRLOC);
            continue;
        }

        action = action_new (name, label, accel, code, ctx_func);

        if (action)
            moo_window_class_new_action_custom (klass, action->name,
                                                (MooWindowActionFunc) create_action,
                                                action, (GDestroyNotify) action_free);
    }

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
    ms_node_unref (action->script);
    G_OBJECT_CLASS(_moo_user_action_parent_class)->finalize (object);
}


static void
moo_user_action_activate (MooAction *_action)
{
    MSContext *ctx;
    MSValue *value;
    MooUserAction *action = (MooUserAction*) _action;

    ctx = action->ctx_func (action->window);
    value = ms_top_node_eval (action->script, ctx);

    if (!value)
        g_warning ("%s: %s", G_STRLOC, ms_context_get_error_msg (ctx));
    else
        ms_value_unref (value);

    g_object_unref (ctx);
}


static void
_moo_user_action_class_init (MooUserActionClass *klass)
{
    G_OBJECT_CLASS(klass)->finalize = moo_user_action_finalize;
    MOO_ACTION_CLASS(klass)->activate = moo_user_action_activate;
}
