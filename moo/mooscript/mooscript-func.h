/*
 *   mooscript-func.h
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

#ifndef MOO_SCRIPT_FUNC_H
#define MOO_SCRIPT_FUNC_H

#include <mooscript/mooscript-value.h>
#include <mooutils/moocobject.h>

G_BEGIN_DECLS


@class MSContext;

@interface MSFunc : MooCObject
- (MSValue*) call:(MSValue**)args
                 :(guint) n_args
                 :(MSContext*) ctx;
@end


typedef MSValue* (*MSCFunc_Var) (MSValue   **args,
                                 guint       n_args,
                                 MSContext  *ctx);
typedef MSValue* (*MSCFunc_0)   (MSContext  *ctx);
typedef MSValue* (*MSCFunc_1)   (MSValue    *arg1,
                                 MSContext  *ctx);
typedef MSValue* (*MSCFunc_2)   (MSValue    *arg1,
                                 MSValue    *arg2,
                                 MSContext  *ctx);
typedef MSValue* (*MSCFunc_3)   (MSValue    *arg1,
                                 MSValue    *arg2,
                                 MSValue    *arg3,
                                 MSContext  *ctx);
typedef MSValue* (*MSCFunc_4)   (MSValue    *arg1,
                                 MSValue    *arg2,
                                 MSValue    *arg3,
                                 MSValue    *arg4,
                                 MSContext  *ctx);

@interface MSCFunc : MSFunc {
@private
    int n_args;
    void (*cfunc) (void);
}

+ (MSFunc*) newVar:(MSCFunc_Var) cfunc;
+ (MSFunc*) new0  :(MSCFunc_0) cfunc;
+ (MSFunc*) new1  :(MSCFunc_1) cfunc;
+ (MSFunc*) new2  :(MSCFunc_2) cfunc;
+ (MSFunc*) new3  :(MSCFunc_3) cfunc;
+ (MSFunc*) new4  :(MSCFunc_4) cfunc;
@end


G_END_DECLS

#endif /* MOO_SCRIPT_FUNC_H */
