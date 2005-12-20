/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   as-script-context.c
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

#include "as-script-context.h"

#define N_POS_VARS 20

G_DEFINE_TYPE (ASContext, as_context, G_TYPE_OBJECT)


static void
default_print_func (const char  *string,
                    G_GNUC_UNUSED ASContext *ctx)
{
    g_print ("%s", string);
}


static ASValue*
print_func (ASValue   **args,
            guint       n_args,
            ASContext  *ctx)
{
    guint i;

    for (i = 0; i < n_args; ++i)
    {
        char *s = as_value_print (args[i]);
        ctx->print_func (s, ctx);
        g_free (s);
    }

    ctx->print_func ("\n", ctx);
    return as_value_none ();
}


static void
add_builtin_funcs (ASContext *ctx)
{
    guint i;
    ASFunc *func;

    for (i = 0; i < AS_BINARY_OP_LAST; ++i)
    {
        func = as_cfunc_new_2 (as_binary_op_cfunc (i));
        as_context_set_func (ctx, as_binary_op_name (i), func);
        g_object_unref (func);
    }

    for (i = 0; i < AS_UNARY_OP_LAST; ++i)
    {
        func = as_cfunc_new_1 (as_unary_op_cfunc (i));
        as_context_set_func (ctx, as_unary_op_name (i), func);
        g_object_unref (func);
    }

    func = as_cfunc_new_var (print_func);
    as_context_set_func (ctx, "print", func);
    g_object_unref (func);
}


static void
as_context_init (ASContext *ctx)
{
    ctx->funcs = g_hash_table_new_full (g_str_hash, g_str_equal,
                                        g_free, g_object_unref);
    ctx->named_vars = g_hash_table_new_full (g_str_hash, g_str_equal, g_free,
                                             (GDestroyNotify) as_value_unref);
    ctx->positional_vars = g_new0 (ASValue*, N_POS_VARS);

    ctx->print_func = default_print_func;
    add_builtin_funcs (ctx);
}


static void
as_context_finalize (GObject *object)
{
    guint i;
    ASContext *ctx = AS_CONTEXT (object);

    g_hash_table_destroy (ctx->funcs);
    g_hash_table_destroy (ctx->named_vars);

    for (i = 0; i < N_POS_VARS; ++i)
        if (ctx->positional_vars[i])
            as_value_unref (ctx->positional_vars[i]);
    g_free (ctx->positional_vars);

    g_free (ctx->error_msg);

    G_OBJECT_CLASS(as_context_parent_class)->finalize (object);
}


static void
as_context_class_init (ASContextClass *klass)
{
    G_OBJECT_CLASS(klass)->finalize = as_context_finalize;
}


ASContext *
as_context_new (void)
{
    ASContext *ctx = g_object_new (AS_TYPE_CONTEXT, NULL);
    return ctx;
}


ASValue *
as_context_get_positional_var (ASContext  *ctx,
                               guint       num)
{
    g_return_val_if_fail (AS_IS_CONTEXT (ctx), NULL);
    g_return_val_if_fail (num < N_POS_VARS, NULL);
    return ctx->positional_vars[num] ?
            as_value_ref (ctx->positional_vars[num]) : NULL;
}


ASValue *
as_context_get_named_var (ASContext  *ctx,
                          const char *name)
{
    ASValue *value;
    g_return_val_if_fail (AS_IS_CONTEXT (ctx), NULL);
    g_return_val_if_fail (name != NULL, NULL);
    value = g_hash_table_lookup (ctx->named_vars, name);
    return value ? as_value_ref (value) : NULL;
}


gboolean
as_context_assign_positional (ASContext  *ctx,
                              guint       num,
                              ASValue    *value)
{
    g_return_val_if_fail (AS_IS_CONTEXT (ctx), FALSE);
    g_return_val_if_fail (num < N_POS_VARS, FALSE);

    if (value != ctx->positional_vars[num])
    {
        if (ctx->positional_vars[num])
            as_value_unref (ctx->positional_vars[num]);
        ctx->positional_vars[num] = value;
        if (value)
            as_value_ref (value);
    }

    return TRUE;
}


gboolean
as_context_assign_named (ASContext  *ctx,
                         const char *name,
                         ASValue    *value)
{
    ASValue *old;

    g_return_val_if_fail (AS_IS_CONTEXT (ctx), FALSE);
    g_return_val_if_fail (name != NULL, FALSE);

    old = g_hash_table_lookup (ctx->named_vars, name);

    if (value != old)
    {
        if (value)
            g_hash_table_insert (ctx->named_vars, g_strdup (name),
                                 as_value_ref (value));
        else
            g_hash_table_remove (ctx->named_vars, name);
    }

    return TRUE;
}


ASFunc *
as_context_get_func (ASContext  *ctx,
                     const char *name)
{
    ASFunc *func;
    g_return_val_if_fail (AS_IS_CONTEXT (ctx), FALSE);
    g_return_val_if_fail (name != NULL, FALSE);
    func = g_hash_table_lookup (ctx->funcs, name);
    return func ? g_object_ref (func) : NULL;
}


gboolean
as_context_set_func (ASContext  *ctx,
                     const char *name,
                     ASFunc     *func)
{
    ASFunc *old;

    g_return_val_if_fail (AS_IS_CONTEXT (ctx), FALSE);
    g_return_val_if_fail (name != NULL, FALSE);
    g_return_val_if_fail (!func || AS_IS_FUNC (func), FALSE);

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


ASValue *
as_context_set_error (ASContext  *ctx,
                      ASError     error,
                      const char *message)
{
    g_return_val_if_fail (AS_IS_CONTEXT (ctx), NULL);
    g_return_val_if_fail (!ctx->error && error, NULL);
    g_return_val_if_fail (!ctx->error_msg, NULL);
    ctx->error = error;
    ctx->error_msg = g_strdup (message);
    return NULL;
}


void
as_context_clear_error (ASContext *ctx)
{
    g_return_if_fail (AS_IS_CONTEXT (ctx));
    ctx->error = AS_ERROR_NONE;
    g_free (ctx->error_msg);
    ctx->error_msg = NULL;
}


const char *
as_context_get_error_msg (ASContext *ctx)
{
    static const char *msgs[AS_ERROR_LAST] = {
        NULL, "Type error", "Value error"
    };

    g_return_val_if_fail (AS_IS_CONTEXT (ctx), NULL);
    g_return_val_if_fail (ctx->error < AS_ERROR_LAST, NULL);

    if (ctx->error_msg)
        return ctx->error_msg;
    else
        return msgs[ctx->error];
}
