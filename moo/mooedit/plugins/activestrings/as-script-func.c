/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   as-script-func.c
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

#include "as-script-func.h"


ASValue *
as_func_call (ASFunc     *func,
              ASValue   **args,
              guint       n_args,
              ASContext  *ctx)
{
    ASFuncCall call;

    g_return_val_if_fail (AS_IS_FUNC (func), NULL);
    g_return_val_if_fail (!n_args || args, NULL);
    g_return_val_if_fail (ctx != NULL, NULL);

    call = AS_FUNC_GET_CLASS(func)->call;
    g_return_val_if_fail (call != NULL, NULL);

    return call (func, args, n_args, ctx);
}


/******************************************************************/
/* ASFunc
 */

G_DEFINE_TYPE (ASFunc, as_func, G_TYPE_OBJECT)

static void
as_func_init (G_GNUC_UNUSED ASFunc *func)
{
}

static void
as_func_class_init (G_GNUC_UNUSED ASFuncClass *klass)
{
}


/******************************************************************/
/* ASCFunc
 */

G_DEFINE_TYPE (ASCFunc, as_cfunc, AS_TYPE_FUNC)


static void
as_cfunc_init (ASCFunc *func)
{
    func->n_args = -1;
}


static ASValue *
as_cfunc_call (ASFunc     *func_,
               ASValue   **args,
               guint       n_args,
               ASContext  *ctx)
{
    ASCFunc_Var func_var;
    ASCFunc_0 func_0;
    ASCFunc_1 func_1;
    ASCFunc_2 func_2;
    ASCFunc_3 func_3;
    ASCFunc *func = AS_CFUNC (func_);

    if (func->n_args >= 0 && func->n_args != (int) n_args)
    {
        g_warning ("wrong number of arguments");
        return NULL;
    }

    if (func->n_args < 0)
    {
        func_var = (ASCFunc_Var) func->cfunc;
        return func_var (args, n_args, ctx);
    }

    switch (func->n_args)
    {
        case 0:
            func_0 = (ASCFunc_0) func->cfunc;
            return func_0 (ctx);
        case 1:
            func_1 = (ASCFunc_1) func->cfunc;
            return func_1 (args[0], ctx);
        case 2:
            func_2 = (ASCFunc_2) func->cfunc;
            return func_2 (args[0], args[1], ctx);
        case 3:
            func_3 = (ASCFunc_3) func->cfunc;
            return func_3 (args[0], args[1], args[2], ctx);
    }

    g_warning ("don't know how to call function with %d arguments", func->n_args);
    return NULL;
}


static void
as_cfunc_class_init (ASCFuncClass *klass)
{
    AS_FUNC_CLASS(klass)->call = as_cfunc_call;
}


static ASFunc *
as_cfunc_new (int       n_args,
              GCallback cfunc)
{
    ASCFunc *func;

    g_return_val_if_fail (cfunc != NULL, NULL);

    func = g_object_new (AS_TYPE_CFUNC, NULL);
    func->n_args = n_args;
    func->cfunc = cfunc;

    return AS_FUNC (func);
}


ASFunc *
as_cfunc_new_var (ASCFunc_Var cfunc)
{
    return as_cfunc_new (-1, G_CALLBACK (cfunc));
}


ASFunc *
as_cfunc_new_0 (ASCFunc_0 cfunc)
{
    return as_cfunc_new (0, G_CALLBACK (cfunc));
}


ASFunc *
as_cfunc_new_1 (ASCFunc_1 cfunc)
{
    return as_cfunc_new (1, G_CALLBACK (cfunc));
}


ASFunc *
as_cfunc_new_2 (ASCFunc_2 cfunc)
{
    return as_cfunc_new (2, G_CALLBACK (cfunc));
}


ASFunc *
as_cfunc_new_3 (ASCFunc_3 cfunc)
{
    return as_cfunc_new (3, G_CALLBACK (cfunc));
}
