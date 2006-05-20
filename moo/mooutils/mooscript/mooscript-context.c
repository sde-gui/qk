/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mooscript-context.c
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

#include "mooscript-context.h"
#include "mooscript-parser.h"
#include <glib/gprintf.h>
#include <gtk/gtkwindow.h>

#define N_POS_VARS 20

enum {
    PROP_0,
    PROP_WINDOW,
    PROP_NAME,
    PROP_ARGV
};

G_DEFINE_TYPE (MSContext, ms_context, G_TYPE_OBJECT)


#if GLIB_CHECK_VERSION(2,10,0)
#define ms_variable_alloc() g_slice_new0 (MSVariable)
#define ms_variable_free(v) g_slice_free (MSVariable, v)
#else
#define ms_variable_alloc() g_new0 (MSVariable, 1)
#define ms_variable_free(v) g_free (v)
#endif

static void
ms_context_set_property (GObject        *object,
                         guint           prop_id,
                         const GValue   *value,
                         GParamSpec     *pspec)
{
    MSContext *ctx = MS_CONTEXT (object);

    switch (prop_id)
    {
        case PROP_WINDOW:
            ctx->window = g_value_get_object (value);
            g_object_notify (object, "window");
            break;

        case PROP_NAME:
            g_free (ctx->name);
            ctx->window = g_strdup (g_value_get_string (value));
            g_object_notify (object, "name");
            break;

        case PROP_ARGV:
            g_strfreev (ctx->argv);
            ctx->argv = g_strdupv (g_value_get_pointer (value));
            ctx->argc = ctx->argv ? g_strv_length (ctx->argv) : 0;
            g_object_notify (object, "argv");
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
ms_context_get_property (GObject        *object,
                         guint           prop_id,
                         GValue         *value,
                         GParamSpec     *pspec)
{
    MSContext *ctx = MS_CONTEXT (object);

    switch (prop_id)
    {
        case PROP_WINDOW:
            g_value_set_object (value, ctx->window);
            break;

        case PROP_NAME:
            g_value_set_string (value, ctx->name);
            break;

        case PROP_ARGV:
            g_value_set_pointer (value, ctx->argv);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
default_print_func (const char  *string,
                    G_GNUC_UNUSED MSContext *ctx)
{
    g_print ("%s", string);
}


static GObject *
ms_context_constructor (GType                  type,
                        guint                  n_props,
                        GObjectConstructParam *props)
{
    GObject *obj;
    MSContext *ctx;

    obj = G_OBJECT_CLASS(ms_context_parent_class)->constructor (type, n_props, props);
    ctx = MS_CONTEXT (obj);

    if (!ctx->name)
    {
        if (ctx->argv && ctx->argc)
            ctx->name = g_strdup (ctx->argv[0]);
        else
            ctx->name = g_strdup ("script");
    }

    if (!ctx->argv || !ctx->argc)
    {
        g_strfreev (ctx->argv);
        ctx->argv = g_new0 (char*, 2);
        ctx->argv[0] = g_strdup (ctx->name);
        ctx->argc = 1;
    }

    return obj;
}


static void
ms_context_init (MSContext *ctx)
{
    ctx->vars = g_hash_table_new_full (g_str_hash, g_str_equal, g_free,
                                       (GDestroyNotify) ms_variable_unref);

    if (moo_python_running ())
        ctx->py_dict = moo_py_get_script_dict ("__moo_script__");

    ctx->print_func = default_print_func;

    _ms_context_add_builtin (ctx);
}


static void
ms_context_finalize (GObject *object)
{
    MSContext *ctx = MS_CONTEXT (object);

    if (moo_python_running ())
        moo_Py_DECREF (ctx->py_dict);

    g_hash_table_destroy (ctx->vars);
    g_free (ctx->error_msg);

    ms_value_unref (ctx->return_val);

    g_free (ctx->name);
    g_strfreev (ctx->argv);

    G_OBJECT_CLASS(ms_context_parent_class)->finalize (object);
}


static void
ms_context_class_init (MSContextClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = ms_context_finalize;
    object_class->set_property = ms_context_set_property;
    object_class->get_property = ms_context_get_property;
    object_class->constructor = ms_context_constructor;

    ms_type_init ();

    g_object_class_install_property (object_class,
                                     PROP_WINDOW,
                                     g_param_spec_object ("window",
                                             "window",
                                             "window",
                                             GTK_TYPE_WINDOW,
                                             G_PARAM_READWRITE));
}


MSContext *
ms_context_new (gpointer window)
{
    MSContext *ctx = g_object_new (MS_TYPE_CONTEXT,
                                   "window", window, NULL);
    return ctx;
}


MSValue *
ms_context_eval_variable (MSContext  *ctx,
                          const char *name)
{
    MSVariable *var;

    g_return_val_if_fail (MS_IS_CONTEXT (ctx), NULL);
    g_return_val_if_fail (name != NULL, NULL);

    var = ms_context_lookup_var (ctx, name);

    if (!var)
        return ms_context_format_error (ctx, MS_ERROR_NAME,
                                        "no variable named '%s'",
                                        name);

    if (var->value)
        return ms_value_ref (var->value);

    return ms_func_call (var->func, NULL, 0, ctx);
}


gboolean
ms_context_assign_variable (MSContext  *ctx,
                            const char *name,
                            MSValue    *value)
{
    MSVariable *var;

    g_return_val_if_fail (MS_IS_CONTEXT (ctx), FALSE);
    g_return_val_if_fail (name != NULL, FALSE);

    var = ms_context_lookup_var (ctx, name);

    if (value)
    {
        if (var)
        {
            if (var->func)
            {
                g_object_unref (var->func);
                var->func = NULL;
            }

            if (var->value != value)
            {
                ms_value_unref (var->value);
                var->value = ms_value_ref (value);
            }
        }
        else
        {
            var = ms_variable_new_value (value);
            ms_context_set_var (ctx, name, var);
            ms_variable_unref (var);
        }
    }
    else if (var)
    {
        ms_context_set_var (ctx, name, NULL);
    }

    return TRUE;
}


gboolean
ms_context_assign_string (MSContext  *ctx,
                          const char *name,
                          const char *str_value)
{
    MSValue *value = NULL;
    gboolean retval;

    g_return_val_if_fail (MS_IS_CONTEXT (ctx), FALSE);
    g_return_val_if_fail (name != NULL, FALSE);

    if (str_value)
        value = ms_value_string (str_value);

    retval = ms_context_assign_variable (ctx, name, value);

    ms_value_unref (value);
    return retval;
}


gboolean
ms_context_assign_positional (MSContext  *ctx,
                              guint       n,
                              MSValue    *value)
{
    char *name;
    gboolean result;

    g_return_val_if_fail (MS_IS_CONTEXT (ctx), FALSE);

    name = g_strdup_printf ("_%d", n);
    result = ms_context_assign_variable (ctx, name, value);

    g_free (name);
    return result;
}


MSVariable *
ms_context_lookup_var (MSContext  *ctx,
                       const char *name)
{
    g_return_val_if_fail (MS_IS_CONTEXT (ctx), NULL);
    g_return_val_if_fail (name != NULL, NULL);
    return g_hash_table_lookup (ctx->vars, name);
}


gboolean
ms_context_set_var (MSContext  *ctx,
                    const char *name,
                    MSVariable *var)
{
    MSVariable *old;

    g_return_val_if_fail (MS_IS_CONTEXT (ctx), FALSE);
    g_return_val_if_fail (name != NULL, FALSE);

    old = g_hash_table_lookup (ctx->vars, name);

    if (var != old)
    {
        if (var)
            g_hash_table_insert (ctx->vars, g_strdup (name),
                                 ms_variable_ref (var));
        else
            g_hash_table_remove (ctx->vars, name);
    }

    return TRUE;
}


gboolean
ms_context_set_func (MSContext  *ctx,
                     const char *name,
                     MSFunc     *func)
{
    MSValue *vfunc;
    gboolean ret;

    g_return_val_if_fail (MS_IS_CONTEXT (ctx), FALSE);
    g_return_val_if_fail (name != NULL, FALSE);
    g_return_val_if_fail (!func || MS_IS_FUNC (func), FALSE);

    vfunc = ms_value_func (func);
    ret = ms_context_assign_variable (ctx, name, vfunc);
    ms_value_unref (vfunc);

    return ret;
}


MSValue *
ms_context_set_error (MSContext  *ctx,
                      MSError     error,
                      const char *message)
{
    const char *errname;

    g_return_val_if_fail (MS_IS_CONTEXT (ctx), NULL);
    g_return_val_if_fail (!ctx->error && error, NULL);
    g_return_val_if_fail (!ctx->error_msg, NULL);

    ctx->error = error;
    errname = ms_context_get_error_msg (ctx);

    if (message && *message)
        ctx->error_msg = g_strdup_printf ("%s: %s", errname, message);
    else
        ctx->error_msg = g_strdup (message);

    return NULL;
}


MSValue *
ms_context_format_error (MSContext  *ctx,
                         MSError     error,
                         const char *format,
                         ...)
{
    va_list args;
    char *string;

    g_return_val_if_fail (MS_IS_CONTEXT (ctx), NULL);
    g_return_val_if_fail (!ctx->error && error, NULL);
    g_return_val_if_fail (!ctx->error_msg, NULL);

    if (!format || !format[0])
    {
        ms_context_set_error (ctx, error, NULL);
        return NULL;
    }

    va_start (args, format);
    string = ms_vaprintf (format, args);
    va_end (args);

    ms_context_set_error (ctx, error, string);
    g_free (string);

    return NULL;
}


void
ms_context_clear_error (MSContext *ctx)
{
    g_return_if_fail (MS_IS_CONTEXT (ctx));
    ctx->error = MS_ERROR_NONE;
    g_free (ctx->error_msg);
    ctx->error_msg = NULL;
}


const char *
ms_context_get_error_msg (MSContext *ctx)
{
    static const char *msgs[MS_ERROR_LAST] = {
        NULL, "Type error", "Value error", "Name error",
        "Runtime error"
    };

    g_return_val_if_fail (MS_IS_CONTEXT (ctx), NULL);
    g_return_val_if_fail (ctx->error < MS_ERROR_LAST, NULL);
    g_return_val_if_fail (ctx->error != MS_ERROR_NONE, "ERROR");

    if (ctx->error_msg)
        return ctx->error_msg;
    else
        return msgs[ctx->error];
}


static MSVariable *
ms_variable_new (void)
{
    MSVariable *var = ms_variable_alloc ();
    var->ref_count = 1;
    return var;
}


MSVariable *
ms_variable_new_value (MSValue *value)
{
    MSVariable *var;

    g_return_val_if_fail (value != NULL, NULL);

    var = ms_variable_new ();
    var->value = ms_value_ref (value);

    return var;
}


MSVariable *
ms_variable_new_func (MSFunc *func)
{
    MSVariable *var;

    g_return_val_if_fail (MS_IS_FUNC (func), NULL);

    var = ms_variable_new ();
    var->func = g_object_ref (func);

    return var;
}


MSVariable *
ms_variable_ref (MSVariable *var)
{
    g_return_val_if_fail (var != NULL, NULL);
    var->ref_count++;
    return var;
}


void
ms_variable_unref (MSVariable *var)
{
    g_return_if_fail (var != NULL);

    if (!--var->ref_count)
    {
        ms_value_unref (var->value);
        if (var->func)
            g_object_unref (var->func);
        ms_variable_free (var);
    }
}


void
ms_context_set_return (MSContext  *ctx,
                       MSValue    *val)
{
    g_return_if_fail (MS_IS_CONTEXT (ctx));
    g_return_if_fail (!ctx->return_set);
    ctx->return_set = TRUE;
    ctx->return_val = val ? ms_value_ref (val) : ms_value_none ();
}


void
ms_context_set_break (MSContext  *ctx)
{
    g_return_if_fail (MS_IS_CONTEXT (ctx));
    g_return_if_fail (!ctx->break_set);
    ctx->break_set = TRUE;
}


void
ms_context_set_continue (MSContext *ctx)
{
    g_return_if_fail (MS_IS_CONTEXT (ctx));
    g_return_if_fail (!ctx->continue_set);
    ctx->continue_set = TRUE;
}


void
ms_context_unset_return (MSContext *ctx)
{
    g_return_if_fail (MS_IS_CONTEXT (ctx));
    g_return_if_fail (ctx->return_set);
    ctx->return_set = FALSE;
    ms_value_unref (ctx->return_val);
    ctx->return_val = NULL;
}


void
ms_context_unset_break (MSContext  *ctx)
{
    g_return_if_fail (MS_IS_CONTEXT (ctx));
    g_return_if_fail (ctx->break_set);
    ctx->break_set = FALSE;
}


void
ms_context_unset_continue (MSContext *ctx)
{
    g_return_if_fail (MS_IS_CONTEXT (ctx));
    g_return_if_fail (ctx->continue_set);
    ctx->continue_set = FALSE;
}


MSValue *
ms_context_run_script (MSContext  *ctx,
                       const char *script)
{
    MSNode *node;
    MSValue *result;

    g_return_val_if_fail (MS_IS_CONTEXT (ctx), NULL);
    g_return_val_if_fail (script != NULL, NULL);

    node = ms_script_parse (script);
    g_return_val_if_fail (node != NULL, NULL);

    result = ms_top_node_eval (node, ctx);

    ms_node_unref (node);
    return result;
}


/************************************************************************/
/* Python
 */

void
ms_context_assign_py_var (MSContext  *ctx,
                          const char *var,
                          MooPyObject *obj)
{
    g_return_if_fail (MS_IS_CONTEXT (ctx));
    g_return_if_fail (var != NULL);
    g_return_if_fail (obj != NULL);
    g_return_if_fail (moo_python_running ());

    if (!ctx->py_dict)
        ctx->py_dict = moo_py_get_script_dict ("__moo_script__");

    moo_py_dict_set_item (ctx->py_dict, var, obj);
}


void
ms_context_assign_py_object (MSContext  *ctx,
                             const char *var,
                             gpointer    gobj)
{
    MooPyObject *pyobj;

    g_return_if_fail (moo_python_running ());
    g_return_if_fail (MS_IS_CONTEXT (ctx));
    g_return_if_fail (var != NULL);
    g_return_if_fail (!gobj || G_IS_OBJECT (gobj));

    pyobj = moo_py_object_from_gobject (gobj);
    g_return_if_fail (pyobj != NULL);
    ms_context_assign_py_var (ctx, var, pyobj);
    moo_Py_DECREF (pyobj);
}


MSValue *
ms_context_run_python (MSContext  *ctx,
                       const char *script)
{
    MooPyObject *py_ret;

    g_return_val_if_fail (MS_IS_CONTEXT (ctx), NULL);
    g_return_val_if_fail (script != NULL, NULL);

    if (!moo_python_running())
        return ms_context_format_error (ctx, MS_ERROR_RUNTIME,
                                        "Python support not available");

    py_ret = moo_python_run_string (script, ctx->py_dict, ctx->py_dict);

    if (py_ret)
    {
        moo_Py_DECREF (py_ret);
        return ms_value_none ();
    }
    else
    {
        moo_PyErr_Print ();
        return ms_context_format_error (ctx, MS_ERROR_RUNTIME,
                                        "python script raised exception");
    }
}
