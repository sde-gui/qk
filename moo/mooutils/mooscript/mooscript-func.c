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

#include "mooscript-func.h"


MSValue *
ms_func_call (MSFunc     *func,
              MSValue   **args,
              guint       n_args,
              MSContext  *ctx)
{
    MSFuncCall call;

    g_return_val_if_fail (MS_IS_FUNC (func), NULL);
    g_return_val_if_fail (!n_args || args, NULL);
    g_return_val_if_fail (ctx != NULL, NULL);

    call = MS_FUNC_GET_CLASS(func)->call;
    g_return_val_if_fail (call != NULL, NULL);

    return call (func, args, n_args, ctx);
}


/******************************************************************/
/* MSFunc
 */

G_DEFINE_TYPE (MSFunc, ms_func, G_TYPE_OBJECT)

static void
ms_func_init (G_GNUC_UNUSED MSFunc *func)
{
}

static void
ms_func_class_init (G_GNUC_UNUSED MSFuncClass *klass)
{
    ms_type_init ();
}


/******************************************************************/
/* MSCFunc
 */

G_DEFINE_TYPE (MSCFunc, ms_cfunc, MS_TYPE_FUNC)


static void
ms_cfunc_init (MSCFunc *func)
{
    func->n_args = -1;
}


static MSValue *
ms_cfunc_call (MSFunc     *func_,
               MSValue   **args,
               guint       n_args,
               MSContext  *ctx)
{
    MSCFunc_Var func_var;
    MSCFunc_0 func_0;
    MSCFunc_1 func_1;
    MSCFunc_2 func_2;
    MSCFunc_3 func_3;
    MSCFunc *func = MS_CFUNC (func_);

    if (func->n_args >= 0 && func->n_args != (int) n_args)
    {
        g_warning ("wrong number of arguments");
        return NULL;
    }

    if (func->n_args < 0)
    {
        func_var = (MSCFunc_Var) func->cfunc;
        return func_var (args, n_args, ctx);
    }

    switch (func->n_args)
    {
        case 0:
            func_0 = (MSCFunc_0) func->cfunc;
            return func_0 (ctx);
        case 1:
            func_1 = (MSCFunc_1) func->cfunc;
            return func_1 (args[0], ctx);
        case 2:
            func_2 = (MSCFunc_2) func->cfunc;
            return func_2 (args[0], args[1], ctx);
        case 3:
            func_3 = (MSCFunc_3) func->cfunc;
            return func_3 (args[0], args[1], args[2], ctx);
    }

    g_warning ("don't know how to call function with %d arguments", func->n_args);
    return NULL;
}


static void
ms_cfunc_class_init (MSCFuncClass *klass)
{
    MS_FUNC_CLASS(klass)->call = ms_cfunc_call;
}


static MSFunc *
ms_cfunc_new (int       n_args,
              GCallback cfunc)
{
    MSCFunc *func;

    g_return_val_if_fail (cfunc != NULL, NULL);

    func = g_object_new (MS_TYPE_CFUNC, NULL);
    func->n_args = n_args;
    func->cfunc = cfunc;

    return MS_FUNC (func);
}


MSFunc *
ms_cfunc_new_var (MSCFunc_Var cfunc)
{
    return ms_cfunc_new (-1, G_CALLBACK (cfunc));
}


MSFunc *
ms_cfunc_new_0 (MSCFunc_0 cfunc)
{
    return ms_cfunc_new (0, G_CALLBACK (cfunc));
}


MSFunc *
ms_cfunc_new_1 (MSCFunc_1 cfunc)
{
    return ms_cfunc_new (1, G_CALLBACK (cfunc));
}


MSFunc *
ms_cfunc_new_2 (MSCFunc_2 cfunc)
{
    return ms_cfunc_new (2, G_CALLBACK (cfunc));
}


MSFunc *
ms_cfunc_new_3 (MSCFunc_3 cfunc)
{
    return ms_cfunc_new (3, G_CALLBACK (cfunc));
}
