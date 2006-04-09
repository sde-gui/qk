/*
 *   moocommand.c
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

#include "mooutils/moocommand.h"
#include "mooutils/moomarshals.h"
#include "mooscript/mooscript-context.h"
#include "mooscript/mooscript-parser.h"
#include <gtk/gtk.h>


static void moo_command_get_property    (GObject        *object,
                                         guint           prop_id,
                                         GValue         *value,
                                         GParamSpec     *pspec);
static void moo_command_set_property    (GObject        *object,
                                         guint           prop_id,
                                         const GValue   *value,
                                         GParamSpec     *pspec);

static void moo_command_finalize        (GObject        *object);
static void moo_command_cleanup         (MooCommand     *cmd);

static void moo_command_run_real        (MooCommand     *cmd);
static void moo_command_run_script      (MooCommand     *cmd);
static void moo_command_run_python      (MooCommand     *cmd);
static void moo_command_run_shell       (MooCommand     *cmd);


G_DEFINE_TYPE(MooCommand, moo_command, G_TYPE_OBJECT)


enum {
    PROP_0,
    PROP_CONTEXT,
    PROP_PY_DICT,
    PROP_SHELL_ENV,
    PROP_WINDOW,
    PROP_SCRIPT,
    PROP_PYTHON_SCRIPT,
    PROP_SHELL_COMMAND
};

enum {
    RUN,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];


static void
moo_command_class_init (MooCommandClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = moo_command_finalize;
    gobject_class->set_property = moo_command_set_property;
    gobject_class->get_property = moo_command_get_property;

    klass->run = moo_command_run_real;

    g_object_class_install_property (gobject_class,
                                     PROP_CONTEXT,
                                     g_param_spec_object ("context",
                                             "context",
                                             "context",
                                             MS_TYPE_CONTEXT,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_PY_DICT,
                                     g_param_spec_pointer ("py-dict",
                                             "py-dict",
                                             "py-dict",
                                             G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_SHELL_ENV,
                                     g_param_spec_boxed ("shell-env",
                                             "shell-env",
                                             "shell-env",
                                             G_TYPE_STRV,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_WINDOW,
                                     g_param_spec_object ("window",
                                             "window",
                                             "window",
                                             GTK_TYPE_WINDOW,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_SCRIPT,
                                     g_param_spec_string ("script",
                                             "script",
                                             "script",
                                             NULL,
                                             G_PARAM_WRITABLE));

    g_object_class_install_property (gobject_class,
                                     PROP_PYTHON_SCRIPT,
                                     g_param_spec_string ("python-script",
                                             "python-script",
                                             "python-script",
                                             NULL,
                                             G_PARAM_WRITABLE));

    g_object_class_install_property (gobject_class,
                                     PROP_SHELL_COMMAND,
                                     g_param_spec_string ("shell-command",
                                             "shell-command",
                                             "shell-command",
                                             NULL,
                                             G_PARAM_WRITABLE));

    signals[RUN] = g_signal_new ("run",
                                 G_TYPE_FROM_CLASS (gobject_class),
                                 G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                                 G_STRUCT_OFFSET (MooCommandClass, run),
                                 NULL, NULL,
                                 _moo_marshal_VOID__VOID,
                                 G_TYPE_NONE, 0);
}


static void
moo_command_init (G_GNUC_UNUSED MooCommand *cmd)
{
}


static void
moo_command_set_property (GObject        *object,
                          guint           prop_id,
                          const GValue   *value,
                          GParamSpec     *pspec)
{
    MooCommand *cmd = MOO_COMMAND (object);

    switch (prop_id)
    {
        case PROP_CONTEXT:
            moo_command_set_context (cmd, g_value_get_object (value));
            break;

        case PROP_PY_DICT:
            moo_command_set_py_dict (cmd, g_value_get_pointer (value));
            break;

        case PROP_SHELL_ENV:
            moo_command_set_shell_env (cmd, g_value_get_boxed (value));
            break;

        case PROP_WINDOW:
            moo_command_set_window (cmd, g_value_get_object (value));
            break;

        case PROP_SCRIPT:
            moo_command_set_script (cmd, g_value_get_string (value));
            break;

        case PROP_PYTHON_SCRIPT:
            moo_command_set_python (cmd, g_value_get_string (value));
            break;

        case PROP_SHELL_COMMAND:
            moo_command_set_shell (cmd, g_value_get_string (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
moo_command_get_property (GObject        *object,
                          guint           prop_id,
                          GValue         *value,
                          GParamSpec     *pspec)
{
    MooCommand *cmd = MOO_COMMAND (object);

    switch (prop_id)
    {
        case PROP_CONTEXT:
            g_value_set_object (value, cmd->context);
            break;

        case PROP_PY_DICT:
            g_value_set_pointer (value, cmd->py_dict);
            break;

        case PROP_SHELL_ENV:
            g_value_set_boxed (value, cmd->shell_env);
            break;

        case PROP_WINDOW:
            g_value_set_object (value, cmd->window);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
moo_command_finalize (GObject *object)
{
    MooCommand *cmd = MOO_COMMAND (object);

    moo_command_cleanup (cmd);

    if (cmd->context)
        g_object_unref (cmd->context);
    if (cmd->py_dict)
        moo_Py_DECREF (cmd->py_dict);
    g_strfreev (cmd->shell_env);

    G_OBJECT_CLASS(moo_command_parent_class)->finalize (object);
}


void
moo_command_run (MooCommand *cmd)
{
    g_return_if_fail (MOO_IS_COMMAND (cmd));
    g_signal_emit (cmd, signals[RUN], 0);
}


static void
moo_command_run_real (MooCommand *cmd)
{
    if (!cmd->type)
    {
        g_message ("%s: no command", G_STRLOC);
        return;
    }

    switch (cmd->type)
    {
        case MOO_COMMAND_SCRIPT:
            return moo_command_run_script (cmd);

        case MOO_COMMAND_PYTHON:
            return moo_command_run_python (cmd);

        case MOO_COMMAND_SHELL:
            return moo_command_run_shell (cmd);
    }

    g_return_if_reached ();
}


static void
moo_command_run_script (MooCommand *cmd)
{
    MSValue *val;

    g_return_if_fail (cmd->script != NULL);

    g_return_if_fail (cmd->context != NULL);

    val = ms_top_node_eval (cmd->script,
                            cmd->context);

    if (!val)
    {
        g_warning ("%s: %s", G_STRLOC,
                   ms_context_get_error_msg (cmd->context));
        ms_context_clear_error (cmd->context);
    }

    ms_value_unref (val);
}


static void
moo_command_run_python (MooCommand *cmd)
{
    MooPyObject *result;

    g_return_if_fail (moo_python_running ());
    g_return_if_fail (cmd->string != NULL);

    if (!cmd->py_dict)
        cmd->py_dict = moo_py_get_script_dict ("__moo_script__");

    result = moo_python_run_string (cmd->string,
                                    cmd->py_dict,
                                    cmd->py_dict);

    if (!result)
        moo_PyErr_Print ();

    moo_Py_DECREF (result);
}


static void
moo_command_run_shell (MooCommand *cmd)
{
    g_return_if_fail (cmd->string);
    g_critical ("%s: implement me", G_STRLOC);
}


void
moo_command_set_window (MooCommand *cmd,
                        gpointer    window)
{
    g_return_if_fail (MOO_IS_COMMAND (cmd));
    g_return_if_fail (!window || GTK_IS_WINDOW (window));

    cmd->window = window;

    if (cmd->context && cmd->window)
        g_object_set (cmd->context, "window", cmd->window, NULL);

    g_object_notify (G_OBJECT (cmd), "window");
}


void
moo_command_set_context (MooCommand *cmd,
                         MSContext  *ctx)
{
    g_return_if_fail (MOO_IS_COMMAND (cmd));
    g_return_if_fail (!ctx || MS_IS_CONTEXT (ctx));

    if (cmd->context != ctx)
    {
        if (cmd->context)
            g_object_unref (cmd->context);
        cmd->context = ctx;
        if (cmd->context)
            g_object_ref (cmd->context);

        if (cmd->context && cmd->window)
            g_object_set (cmd->context, "window", cmd->window, NULL);

        g_object_notify (G_OBJECT (cmd), "context");
    }
}


void
moo_command_set_py_dict (MooCommand *cmd,
                         gpointer    dict)
{
    g_return_if_fail (MOO_IS_COMMAND (cmd));
    g_return_if_fail (moo_python_running ());

    if (cmd->py_dict != dict)
    {
        if (cmd->py_dict)
            moo_Py_DECREF (cmd->py_dict);
        cmd->py_dict = dict;
        if (cmd->py_dict)
            moo_Py_INCREF (cmd->py_dict);
        g_object_notify (G_OBJECT (cmd), "py-dict");
    }
}


void
moo_command_set_shell_env (MooCommand *cmd,
                           char      **env)
{
    g_return_if_fail (MOO_IS_COMMAND (cmd));

    if (cmd->shell_env != env)
    {
        g_strfreev (cmd->shell_env);
        cmd->shell_env = g_strdupv (env);
        g_object_notify (G_OBJECT (cmd), "shell-env");
    }
}


void
moo_command_set_script (MooCommand *cmd,
                        const char *string)
{
    g_return_if_fail (MOO_IS_COMMAND (cmd));

    moo_command_cleanup (cmd);

    cmd->type = MOO_COMMAND_SCRIPT;

    if (string)
        cmd->script = ms_script_parse (string);
}


void
moo_command_set_python (MooCommand *cmd,
                        const char *string)
{
    g_return_if_fail (MOO_IS_COMMAND (cmd));

    if (cmd->type == MOO_COMMAND_PYTHON && cmd->string == string)
        return;

    moo_command_cleanup (cmd);

    cmd->type = MOO_COMMAND_PYTHON;
    cmd->string = g_strdup (string);
}


void
moo_command_set_shell (MooCommand *cmd,
                       const char *string)
{
    g_return_if_fail (MOO_IS_COMMAND (cmd));

    if (cmd->type == MOO_COMMAND_SHELL && cmd->string == string)
            return;

    moo_command_cleanup (cmd);

    cmd->type = MOO_COMMAND_SHELL;
    cmd->string = g_strdup (string);
}


static void
moo_command_cleanup (MooCommand *cmd)
{
    g_free (cmd->string);
    ms_node_unref (cmd->script);
    cmd->string = NULL;
    cmd->script = NULL;
}


MooCommand *
moo_command_new (MooCommandType type)
{
    MooCommand *cmd;

    if (!type)
        return g_object_new (MOO_TYPE_COMMAND, NULL);

    switch (type)
    {
        case MOO_COMMAND_SCRIPT:
        case MOO_COMMAND_PYTHON:
        case MOO_COMMAND_SHELL:
            cmd = g_object_new (MOO_TYPE_COMMAND, NULL);
            cmd->type = type;
            return cmd;
    }

    g_return_val_if_reached (NULL);
}
