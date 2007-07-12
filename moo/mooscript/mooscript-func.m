/*
 *   mooscript-func.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#include "mooscript-context.h"
#include "mooscript-func-private.h"


@implementation MSFunc : MooCObject

- (MSValue*) call:(MSValue**)args
                 :(guint) n_args
                 :(MSContext*) ctx
{
    g_return_val_if_fail (!n_args || args, NULL);
    g_return_val_if_fail (ctx != NULL, NULL);
    g_return_val_if_reached (NULL);
}

@end


@implementation MSCFunc : MSFunc

- init
{
    [super init];
    n_args = -1;
    return self;
}

- (MSValue*) call:(MSValue**) call_args
                 :(guint) n_call_args
                 :(MSContext*) ctx
{
    MSCFunc_Var func_var;
    MSCFunc_0 func_0;
    MSCFunc_1 func_1;
    MSCFunc_2 func_2;
    MSCFunc_3 func_3;
    MSCFunc_4 func_4;

    if (n_args >= 0 && n_args != (int) n_call_args)
    {
        [ctx formatError:MS_ERROR_TYPE
                        :"function takes %d arguments, but %d given",
                         n_args, n_call_args];
        return NULL;
    }

    if (n_args < 0)
    {
        func_var = (MSCFunc_Var) cfunc;
        return func_var (call_args, n_call_args, ctx);
    }

    switch (n_args)
    {
        case 0:
            func_0 = (MSCFunc_0) cfunc;
            return func_0 (ctx);
        case 1:
            func_1 = (MSCFunc_1) cfunc;
            return func_1 (call_args[0], ctx);
        case 2:
            func_2 = (MSCFunc_2) cfunc;
            return func_2 (call_args[0], call_args[1], ctx);
        case 3:
            func_3 = (MSCFunc_3) cfunc;
            return func_3 (call_args[0], call_args[1], call_args[2], ctx);
        case 4:
            func_4 = (MSCFunc_4) cfunc;
            return func_4 (call_args[0], call_args[1], call_args[2], call_args[3], ctx);
    }

    g_warning ("don't know how to call function with %d arguments", n_args);
    return NULL;
}


+ (MSFunc*) new:(int) n_args
               :(GCallback) cfunc
{
    MSCFunc *func;

    g_return_val_if_fail (cfunc != NULL, NULL);

    func = [[self alloc] init];
    func->n_args = n_args;
    func->cfunc = cfunc;

    return func;
}

+ (MSFunc*) newVar:(MSCFunc_Var) cfunc
{
    return [self new:-1 :G_CALLBACK (cfunc)];
}

+ (MSFunc*) new0  :(MSCFunc_0) cfunc
{
    return [self new:0 :G_CALLBACK (cfunc)];
}

+ (MSFunc*) new1  :(MSCFunc_1) cfunc
{
    return [self new:1 :G_CALLBACK (cfunc)];
}

+ (MSFunc*) new2  :(MSCFunc_2) cfunc
{
    return [self new:2 :G_CALLBACK (cfunc)];
}

+ (MSFunc*) new3  :(MSCFunc_3) cfunc
{
    return [self new:3 :G_CALLBACK (cfunc)];
}

+ (MSFunc*) new4  :(MSCFunc_4) cfunc
{
    return [self new:4 :G_CALLBACK (cfunc)];
}

@end


/* -*- objc -*- */
