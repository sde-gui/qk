/*
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

#include <mooscript/mooscript-value.h>

G_BEGIN_DECLS


#define MS_TYPE_FUNC                    (ms_func_get_type ())
#define MS_FUNC(object)                 (G_TYPE_CHECK_INSTANCE_CAST ((object), MS_TYPE_FUNC, MSFunc))
#define MS_FUNC_CLASS(klass)            (G_TYPE_CHECK_CLASS_CAST ((klass), MS_TYPE_FUNC, MSFuncClass))
#define MS_IS_FUNC(object)              (G_TYPE_CHECK_INSTANCE_TYPE ((object), MS_TYPE_FUNC))
#define MS_IS_FUNC_CLASS(klass)         (G_TYPE_CHECK_CLASS_TYPE ((klass), MS_TYPE_FUNC))
#define MS_FUNC_GET_CLASS(obj)          (G_TYPE_INSTANCE_GET_CLASS ((obj), MS_TYPE_FUNC, MSFuncClass))

#define MS_TYPE_CFUNC                   (ms_cfunc_get_type ())
#define MS_CFUNC(object)                (G_TYPE_CHECK_INSTANCE_CAST ((object), MS_TYPE_CFUNC, MSCFunc))
#define MS_CFUNC_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), MS_TYPE_CFUNC, MSCFuncClass))
#define MS_IS_CFUNC(object)             (G_TYPE_CHECK_INSTANCE_TYPE ((object), MS_TYPE_CFUNC))
#define MS_IS_CFUNC_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), MS_TYPE_CFUNC))
#define MS_CFUNC_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), MS_TYPE_CFUNC, MSCFuncClass))

typedef struct _MSContext MSContext;
typedef struct _MSFuncClass MSFuncClass;
typedef struct _MSCFunc MSCFunc;
typedef struct _MSCFuncClass MSCFuncClass;

struct _MSFunc {
    GObject object;
};

typedef MSValue* (*MSFuncCall)  (MSFunc     *func,
                                 MSValue   **args,
                                 guint       n_args,
                                 MSContext  *ctx);

struct _MSFuncClass {
    GObjectClass object_class;
    MSFuncCall call;
};


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

struct _MSCFunc {
    MSFunc func;
    int n_args;
    void (*cfunc) (void);
};

struct _MSCFuncClass {
    MSFuncClass func_class;
};


GType           ms_func_get_type    (void) G_GNUC_CONST;
GType           ms_cfunc_get_type   (void) G_GNUC_CONST;

MSFunc         *ms_cfunc_new_var    (MSCFunc_Var cfunc);
MSFunc         *ms_cfunc_new_0      (MSCFunc_0   cfunc);
MSFunc         *ms_cfunc_new_1      (MSCFunc_1   cfunc);
MSFunc         *ms_cfunc_new_2      (MSCFunc_2   cfunc);
MSFunc         *ms_cfunc_new_3      (MSCFunc_3   cfunc);
MSFunc         *ms_cfunc_new_4      (MSCFunc_4   cfunc);


G_END_DECLS

#endif /* __MOO_SCRIPT_FUNC_H__ */
