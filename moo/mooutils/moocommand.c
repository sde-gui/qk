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
#include <string.h>


static void     moo_command_get_property(GObject        *object,
                                         guint           prop_id,
                                         GValue         *value,
                                         GParamSpec     *pspec);
static void     moo_command_set_property(GObject        *object,
                                         guint           prop_id,
                                         const GValue   *value,
                                         GParamSpec     *pspec);

static void     moo_command_finalize    (GObject        *object);
static void     moo_command_cleanup     (MooCommand     *cmd);

static void     moo_command_run_real    (MooCommand     *cmd);
static gboolean moo_command_run_exe     (MooCommand     *cmd,
                                         const char     *cmd_line);

static void     run_script              (MooCommand     *cmd);
static void     run_python              (MooCommand     *cmd);
static void     run_shell               (MooCommand     *cmd);
static void     run_exe                 (MooCommand     *cmd);


G_DEFINE_TYPE(MooCommand, moo_command, G_TYPE_OBJECT)


enum {
    PROP_0,
    PROP_CONTEXT,
    PROP_PY_DICT,
    PROP_SHELL_ENV,
    PROP_WINDOW,
    PROP_SCRIPT,
    PROP_PYTHON_SCRIPT,
    PROP_SHELL_COMMAND,
    PROP_TYPE,
    PROP_CODE
};

enum {
    RUN,
    RUN_EXE,
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
    klass->run_exe = moo_command_run_exe;

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

    g_object_class_install_property (gobject_class,
                                     PROP_TYPE,
                                     g_param_spec_enum ("type",
                                             "type",
                                             "type",
                                             MOO_TYPE_COMMAND_TYPE,
                                             MOO_COMMAND_SCRIPT,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_CODE,
                                     g_param_spec_string ("code",
                                             "code",
                                             "code",
                                             NULL,
                                             G_PARAM_READWRITE));

    signals[RUN] = g_signal_new ("run",
                                 G_TYPE_FROM_CLASS (gobject_class),
                                 G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                                 G_STRUCT_OFFSET (MooCommandClass, run),
                                 NULL, NULL,
                                 _moo_marshal_VOID__VOID,
                                 G_TYPE_NONE, 0);

    signals[RUN_EXE] = g_signal_new ("run-exe",
                                     G_TYPE_FROM_CLASS (gobject_class),
                                     G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                                     G_STRUCT_OFFSET (MooCommandClass, run_exe),
                                     g_signal_accumulator_true_handled, NULL,
                                     _moo_marshal_BOOLEAN__STRING,
                                     G_TYPE_BOOLEAN, 1,
                                     G_TYPE_STRING);
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
    char *tmp;

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

        case PROP_CODE:
            moo_command_set_code (cmd, cmd->type, g_value_get_string (value));
            break;

        case PROP_TYPE:
            tmp = cmd->string;
            cmd->string = NULL;
            moo_command_set_code (cmd, g_value_get_enum (value), tmp);
            g_free (tmp);
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
    if (cmd->shell_vars)
        g_hash_table_destroy (cmd->shell_vars);
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
            return run_script (cmd);

        case MOO_COMMAND_PYTHON:
            return run_python (cmd);

        case MOO_COMMAND_SHELL:
            return run_shell (cmd);

        case MOO_COMMAND_EXE:
            return run_exe (cmd);
    }

    g_return_if_reached ();
}


static void
run_script (MooCommand *cmd)
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
run_python (MooCommand *cmd)
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


#ifdef __WIN32__
#define VAR_CHAR '%'
#else
#define VAR_CHAR '$'
#endif

#define IS_VARIABLE(c) (c == '_' || g_ascii_isalnum (c))


static char *
expand_vars (MooCommand *cmd,
             const char *string)
{
    GString *cmd_line;

    g_return_val_if_fail (string != NULL, NULL);

    cmd_line = g_string_new (NULL);

    while (*string)
    {
        const char *var_start = strchr (string, VAR_CHAR);
        const char *var_end, *value;
        char *variable;

        if (!var_start)
        {
            g_string_append (cmd_line, string);
            break;
        }
        else
        {
            g_string_append_len (cmd_line, string, var_start - string);
        }

        if (!var_start[1])
        {
            g_warning ("%s: trailing %c in '%s'", G_STRLOC,
                       VAR_CHAR, cmd->string);
            g_string_append_c (cmd_line, VAR_CHAR);
            break;
        }

        if (var_start[1] == VAR_CHAR)
        {
            g_string_append_c (cmd_line, VAR_CHAR);
            string = var_start + 2;
            continue;
        }

        var_start++;
        for (var_end = var_start; *var_end && IS_VARIABLE (*var_end); ++var_end) ;

        variable = g_strndup (var_start, var_end - var_start);
        value = moo_command_get_shell_var (cmd, variable);

        if (!value)
        {
            g_warning ("%s: unbound variable '%s' in '%s'",
                       G_STRLOC, variable, cmd->string);
        }
        else
        {
            g_string_append (cmd_line, value);
        }

        g_free (variable);
        string = var_end;
    }

    return g_string_free (cmd_line, FALSE);
}


static void
run_exe (MooCommand *cmd)
{
    char *cmd_line;
    gboolean result;

    g_return_if_fail (cmd->string != NULL);
    g_critical ("%s: implement me", G_STRLOC);

    cmd_line = expand_vars (cmd, cmd->string);

    if (!cmd_line || !cmd_line[0])
        g_warning ("%s: empty command line in '%s'",
                   G_STRLOC, cmd->string);
    else
        g_signal_emit (cmd, signals[RUN_EXE], 0, cmd_line, &result);

    g_free (cmd_line);
}


static void
run_shell (MooCommand *cmd)
{
    char *cmd_line, *sh_command_line;
    gboolean result;

    g_return_if_fail (cmd->string != NULL);
    g_critical ("%s: implement me", G_STRLOC);

    cmd_line = expand_vars (cmd, cmd->string);
    sh_command_line = g_strdup_printf ("sh -c '%s'", cmd_line);

    g_signal_emit (cmd, signals[RUN_EXE], 0, sh_command_line, &result);

    g_free (cmd_line);
    g_free (sh_command_line);
}


static gboolean
moo_command_run_exe (G_GNUC_UNUSED MooCommand *cmd,
                     const char *cmd_line)
{
    GError *error = NULL;

    if (!g_spawn_command_line_async (cmd_line, &error))
    {
        g_warning ("%s: %s", G_STRLOC, error->message);
        g_error_free (error);
    }

    return TRUE;
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
moo_command_set_flags (MooCommand         *cmd,
                       MooCommandFlags     flags)
{
    g_return_if_fail (MOO_IS_COMMAND (cmd));
    cmd->flags = flags;
}


void
moo_command_add_flags (MooCommand         *cmd,
                       MooCommandFlags     flags)
{
    g_return_if_fail (MOO_IS_COMMAND (cmd));
    cmd->flags |= flags;
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


static char *
get_command (const char *string)
{
    guint len;

    g_return_val_if_fail (string && string[0], NULL);

    len = strlen (string);

    if (len && string[len-1] == '\n')
    {
        if (len > 1 && string[len-2] == '\r')
            len -= 2;
        else
            len -= 1;
    }

    return g_strndup (string, len);
}


void
moo_command_set_code (MooCommand     *cmd,
                      MooCommandType  type,
                      const char     *string)
{
    g_return_if_fail (MOO_IS_COMMAND (cmd));

    if (cmd->type == type && cmd->string == string)
        return;

    moo_command_cleanup (cmd);

    cmd->type = type;

    if (string)
    {
        switch (type)
        {
            case MOO_COMMAND_SCRIPT:
                cmd->script = ms_script_parse (string);
            case MOO_COMMAND_PYTHON:
                cmd->string = g_strdup (string);
                break;

            case MOO_COMMAND_EXE:
            case MOO_COMMAND_SHELL:
                cmd->string = get_command (string);
                break;
        }
    }
}


void
moo_command_set_script (MooCommand *cmd,
                        const char *string)
{
    g_return_if_fail (MOO_IS_COMMAND (cmd));
    moo_command_set_code (cmd, MOO_COMMAND_SCRIPT, string);
}


void
moo_command_set_python (MooCommand *cmd,
                        const char *string)
{
    g_return_if_fail (MOO_IS_COMMAND (cmd));
    moo_command_set_code (cmd, MOO_COMMAND_PYTHON, string);
}


void
moo_command_set_shell (MooCommand *cmd,
                       const char *string)
{
    g_return_if_fail (MOO_IS_COMMAND (cmd));
    g_return_if_fail (string && string[0]);
    moo_command_set_code (cmd, MOO_COMMAND_SHELL, string);
}


void
moo_command_set_exe (MooCommand *cmd,
                     const char *string)
{
    g_return_if_fail (MOO_IS_COMMAND (cmd));
    g_return_if_fail (string && string[0]);
    moo_command_set_code (cmd, MOO_COMMAND_EXE, string);
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
moo_command_new (MooCommandType type,
                 const char    *code)
{
    if (!type)
        return g_object_new (MOO_TYPE_COMMAND,
                             "code", code, NULL);

    switch (type)
    {
        case MOO_COMMAND_SCRIPT:
        case MOO_COMMAND_PYTHON:
        case MOO_COMMAND_SHELL:
        case MOO_COMMAND_EXE:
            return g_object_new (MOO_TYPE_COMMAND,
                                 "type", type,
                                 "code", code, NULL);
    }

    g_return_val_if_reached (NULL);
}


void
moo_command_clear_shell_vars (MooCommand *cmd)
{
    g_return_if_fail (MOO_IS_COMMAND (cmd));

    if (cmd->shell_vars)
        g_hash_table_destroy (cmd->shell_vars);

    cmd->shell_vars = NULL;
}


void
moo_command_set_shell_var (MooCommand *cmd,
                           const char *variable,
                           const char *value)
{
    g_return_if_fail (MOO_IS_COMMAND (cmd));
    g_return_if_fail (variable != NULL);

    if (value)
    {
        if (!cmd->shell_vars)
            cmd->shell_vars = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                     g_free, g_free);
        g_hash_table_insert (cmd->shell_vars, g_strdup (variable), g_strdup (value));
    }
    else if (cmd->shell_vars)
    {
        g_hash_table_remove (cmd->shell_vars, variable);
    }
}


const char *
moo_command_get_shell_var (MooCommand *cmd,
                           const char *variable)
{
    g_return_val_if_fail (MOO_IS_COMMAND (cmd), NULL);
    g_return_val_if_fail (variable != NULL, NULL);

    if (cmd->shell_vars)
        return g_hash_table_lookup (cmd->shell_vars, variable);
    else
        return NULL;
}


MooCommandType
moo_command_type_parse (const char *string)
{
    char *norm;
    MooCommandType cmd_type = 0;

    g_return_val_if_fail (string != NULL, 0);

    norm = g_strstrip (g_ascii_strdown (string, -1));

    if (!strcmp (norm, "script"))
        cmd_type = MOO_COMMAND_SCRIPT;
    else if (!strcmp (norm, "shell") || !strcmp (norm, "bat"))
        cmd_type = MOO_COMMAND_SHELL;
    else if (!strcmp (norm, "exe"))
        cmd_type = MOO_COMMAND_EXE;
    else if (!strcmp (norm, "python"))
        cmd_type = MOO_COMMAND_PYTHON;

    g_free (norm);
    return cmd_type;
}


GType
moo_command_type_get_type (void)
{
    static GType type;

    if (!type)
    {
        static GEnumValue values[] = {
            { MOO_COMMAND_SCRIPT, (char*) "MOO_COMMAND_SCRIPT", (char*) "script" },
            { MOO_COMMAND_PYTHON, (char*) "MOO_COMMAND_PYTHON", (char*) "python" },
            { MOO_COMMAND_SHELL, (char*) "MOO_COMMAND_SHELL", (char*) "shell" },
            { MOO_COMMAND_EXE, (char*) "MOO_COMMAND_EXE", (char*) "exe" },
            { 0, NULL, NULL }
        };

        type = g_enum_register_static ("MooCommandType", values);
    }

    return type;
}
