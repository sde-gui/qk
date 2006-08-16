/*
 *   moocommand-script.c
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

#include "mooedit/moocommand-script.h"
#include "mooedit/mooedit-script.h"
#include "mooscript/mooscript-parser.h"


struct _MooCommandScriptPrivate {
    MSNode *script;
};


G_DEFINE_TYPE (MooCommandScript, moo_command_script, MOO_TYPE_COMMAND)


static void
set_variable (const char   *name,
              const GValue *value,
              gpointer      data)
{
    MSContext *ctx = data;
    MSValue *ms_value;

    ms_value = ms_value_from_gvalue (value);
    g_return_if_fail (ms_value != NULL);

    ms_context_assign_variable (ctx, name, ms_value);
    ms_value_unref (ms_value);
}


static void
moo_command_script_run (MooCommand        *cmd_base,
                        MooCommandContext *ctx)
{
    MSValue *ret;
    MSContext *script_ctx;
    MooCommandScript *cmd = MOO_COMMAND_SCRIPT (cmd_base);

    g_return_if_fail (cmd->priv->script != NULL);

    script_ctx = moo_edit_script_context_new (moo_command_context_get_doc (ctx),
                                              moo_command_context_get_window (ctx));
    g_return_if_fail (script_ctx != NULL);

    moo_command_context_foreach (ctx, set_variable, script_ctx);

    ret = ms_top_node_eval (cmd->priv->script, script_ctx);

    if (!ret)
    {
        g_print ("%s\n", ms_context_get_error_msg (script_ctx));
        ms_context_clear_error (script_ctx);
    }

    ms_value_unref (ret);
    g_object_unref (script_ctx);
}


static void
moo_command_script_dispose (GObject *object)
{
    MooCommandScript *cmd = MOO_COMMAND_SCRIPT (object);

    if (cmd->priv)
    {
        ms_node_unref (cmd->priv->script);
        g_free (cmd->priv);
        cmd->priv = NULL;
    }

    G_OBJECT_CLASS(moo_command_script_parent_class)->dispose (object);
}


static MooCommand *
factory_func (MooCommandData *data,
              G_GNUC_UNUSED gpointer user_data)
{
    MooCommand *cmd;
    const char *code;

    code = moo_command_data_get (data, "code");

    g_return_val_if_fail (code && *code, NULL);

    cmd = moo_command_script_new (code);
    g_return_val_if_fail (cmd != NULL, NULL);

    moo_command_load_data (cmd, data);

    return cmd;
}


static void
moo_command_script_class_init (MooCommandScriptClass *klass)
{
    G_OBJECT_CLASS(klass)->dispose = moo_command_script_dispose;
    MOO_COMMAND_CLASS(klass)->run = moo_command_script_run;

    moo_command_register ("MooScript", factory_func, NULL, NULL);
}


static void
moo_command_script_init (MooCommandScript *cmd)
{
    cmd->priv = g_new0 (MooCommandScriptPrivate, 1);
}


MooCommand *
moo_command_script_new (const char *script)
{
    MooCommandScript *cmd;
    MSNode *node;

    g_return_val_if_fail (script != NULL, NULL);

    node = ms_script_parse (script);
    g_return_val_if_fail (node != NULL, NULL);

    cmd = g_object_new (MOO_TYPE_COMMAND_SCRIPT, NULL);
    cmd->priv->script = node;

    return MOO_COMMAND (cmd);
}
