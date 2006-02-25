/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   as-script-context.c
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
#include <glib/gprintf.h>

#define N_POS_VARS 20

G_DEFINE_TYPE (MSContext, ms_context, G_TYPE_OBJECT)


static void
default_print_func (const char  *string,
                    G_GNUC_UNUSED MSContext *ctx)
{
    g_print ("%s", string);
}


static MSValue*
print_func (MSValue   **args,
            guint       n_args,
            MSContext  *ctx)
{
    guint i;

    for (i = 0; i < n_args; ++i)
    {
        char *s = ms_value_print (args[i]);
        ctx->print_func (s, ctx);
        g_free (s);
    }

    ctx->print_func ("\n", ctx);
    return ms_value_none ();
}


static void
add_builtin_funcs (MSContext *ctx)
{
    guint i;
    MSFunc *func;

    for (i = 0; i < MS_BINARY_OP_LMST; ++i)
    {
        func = ms_cfunc_new_2 (ms_binary_op_cfunc (i));
        ms_context_set_func (ctx, ms_binary_op_name (i), func);
        g_object_unref (func);
    }

    for (i = 0; i < MS_UNARY_OP_LMST; ++i)
    {
        func = ms_cfunc_new_1 (ms_unary_op_cfunc (i));
        ms_context_set_func (ctx, ms_unary_op_name (i), func);
        g_object_unref (func);
    }

    func = ms_cfunc_new_var (print_func);
    ms_context_set_func (ctx, "print", func);
    g_object_unref (func);
}


static void
ms_context_init (MSContext *ctx)
{
    ctx->funcs = g_hash_table_new_full (g_str_hash, g_str_equal,
                                        g_free, g_object_unref);
    ctx->vars = g_hash_table_new_full (g_str_hash, g_str_equal, g_free,
                                       (GDestroyNotify) ms_variable_unref);

    ctx->print_func = default_print_func;
    add_builtin_funcs (ctx);
}


static void
ms_context_finalize (GObject *object)
{
    MSContext *ctx = MS_CONTEXT (object);

    g_hash_table_destroy (ctx->funcs);
    g_hash_table_destroy (ctx->vars);

    g_free (ctx->error_msg);

    G_OBJECT_CLASS(ms_context_parent_class)->finalize (object);
}


static void
ms_context_class_init (MSContextClass *klass)
{
    G_OBJECT_CLASS(klass)->finalize = ms_context_finalize;
}


MSContext *
ms_context_new (void)
{
    MSContext *ctx = g_object_new (MS_TYPE_CONTEXT, NULL);
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
        return ms_value_none ();

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


MSFunc *
ms_context_lookup_func (MSContext  *ctx,
                        const char *name)
{
    g_return_val_if_fail (MS_IS_CONTEXT (ctx), FALSE);
    g_return_val_if_fail (name != NULL, FALSE);
    return g_hash_table_lookup (ctx->funcs, name);
}


gboolean
ms_context_set_func (MSContext  *ctx,
                     const char *name,
                     MSFunc     *func)
{
    MSFunc *old;

    g_return_val_if_fail (MS_IS_CONTEXT (ctx), FALSE);
    g_return_val_if_fail (name != NULL, FALSE);
    g_return_val_if_fail (!func || MS_IS_FUNC (func), FALSE);

    old = g_hash_table_lookup (ctx->funcs, name);

    if (func != old)
    {
        if (func)
            g_hash_table_insert (ctx->funcs, g_strdup (name),
                                 g_object_ref (func));
        else
            g_hash_table_remove (ctx->funcs, name);
    }

    return TRUE;
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


static void
ms_context_format_error_valist (MSContext  *ctx,
                                MSError     error,
                                const char *format,
                                va_list     args)
{
    char *buffer = NULL;
    g_vasprintf (&buffer, format, args);
    ms_context_set_error (ctx, error, buffer);
    g_free (buffer);
}


MSValue *
ms_context_format_error (MSContext  *ctx,
                         MSError     error,
                         const char *format,
                         ...)
{
    va_list args;

    g_return_val_if_fail (MS_IS_CONTEXT (ctx), NULL);
    g_return_val_if_fail (!ctx->error && error, NULL);
    g_return_val_if_fail (!ctx->error_msg, NULL);

    if (!format || !format[0])
        ms_context_set_error (ctx, error, NULL);

    va_start (args, format);
    ms_context_format_error_valist (ctx, error, format, args);
    va_end (args);

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
    static const char *msgs[MS_ERROR_LMST] = {
        NULL, "Type error", "Value error", "Name error"
    };

    g_return_val_if_fail (MS_IS_CONTEXT (ctx), NULL);
    g_return_val_if_fail (ctx->error < MS_ERROR_LMST, NULL);

    if (ctx->error_msg)
        return ctx->error_msg;
    else
        return msgs[ctx->error];
}


static MSVariable *
ms_variable_new (void)
{
    MSVariable *var = g_new0 (MSVariable, 1);
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
        if (var->value)
            ms_value_unref (var->value);
        if (var->func)
            g_object_unref (var->func);
        g_free (var);
    }
}
