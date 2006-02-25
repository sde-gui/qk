/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mooscript-func.h
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

#ifndef __MOO_SCRIPT_FUNC_H__
#define __MOO_SCRIPT_FUNC_H__

#include "mooscript-value.h"

G_BEGIN_DECLS


#define AS_TYPE_FUNC                    (as_func_get_type ())
#define AS_FUNC(object)                 (G_TYPE_CHECK_INSTANCE_CAST ((object), AS_TYPE_FUNC, ASFunc))
#define AS_FUNC_CLASS(klass)            (G_TYPE_CHECK_CLASS_CAST ((klass), AS_TYPE_FUNC, ASFuncClass))
#define AS_IS_FUNC(object)              (G_TYPE_CHECK_INSTANCE_TYPE ((object), AS_TYPE_FUNC))
#define AS_IS_FUNC_CLASS(klass)         (G_TYPE_CHECK_CLASS_TYPE ((klass), AS_TYPE_FUNC))
#define AS_FUNC_GET_CLASS(obj)          (G_TYPE_INSTANCE_GET_CLASS ((obj), AS_TYPE_FUNC, ASFuncClass))

#define AS_TYPE_CFUNC                   (as_cfunc_get_type ())
#define AS_CFUNC(object)                (G_TYPE_CHECK_INSTANCE_CAST ((object), AS_TYPE_CFUNC, ASCFunc))
#define AS_CFUNC_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), AS_TYPE_CFUNC, ASCFuncClass))
#define AS_IS_CFUNC(object)             (G_TYPE_CHECK_INSTANCE_TYPE ((object), AS_TYPE_CFUNC))
#define AS_IS_CFUNC_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), AS_TYPE_CFUNC))
#define AS_CFUNC_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), AS_TYPE_CFUNC, ASCFuncClass))

typedef struct _ASContext ASContext;

typedef struct _ASFunc ASFunc;
typedef struct _ASFuncClass ASFuncClass;
typedef struct _ASCFunc ASCFunc;
typedef struct _ASCFuncClass ASCFuncClass;

struct _ASFunc {
    GObject object;
};

typedef ASValue* (*ASFuncCall)  (ASFunc     *func,
                                 ASValue   **args,
                                 guint       n_args,
                                 ASContext  *ctx);

struct _ASFuncClass {
    GObjectClass object_class;
    ASFuncCall call;
};


typedef ASValue* (*ASCFunc_Var) (ASValue   **args,
                                 guint       n_args,
                                 ASContext  *ctx);
typedef ASValue* (*ASCFunc_0)   (ASContext  *ctx);
typedef ASValue* (*ASCFunc_1)   (ASValue    *arg1,
                                 ASContext  *ctx);
typedef ASValue* (*ASCFunc_2)   (ASValue    *arg1,
                                 ASValue    *arg2,
                                 ASContext  *ctx);
typedef ASValue* (*ASCFunc_3)   (ASValue    *arg1,
                                 ASValue    *arg2,
                                 ASValue    *arg3,
                                 ASContext  *ctx);

struct _ASCFunc {
    ASFunc func;
    int n_args;
    void (*cfunc) (void);
};

struct _ASCFuncClass {
    ASFuncClass func_class;
};


GType           as_func_get_type    (void) G_GNUC_CONST;
GType           as_cfunc_get_type   (void) G_GNUC_CONST;

ASValue        *as_func_call        (ASFunc     *func,
                                     ASValue   **args,
                                     guint       n_args,
                                     ASContext  *ctx);

ASFunc         *as_cfunc_new_var    (ASCFunc_Var cfunc);
ASFunc         *as_cfunc_new_0      (ASCFunc_0   cfunc);
ASFunc         *as_cfunc_new_1      (ASCFunc_1   cfunc);
ASFunc         *as_cfunc_new_2      (ASCFunc_2   cfunc);
ASFunc         *as_cfunc_new_3      (ASCFunc_3   cfunc);


G_END_DECLS

#endif /* __MOO_SCRIPT_FUNC_H__ */
