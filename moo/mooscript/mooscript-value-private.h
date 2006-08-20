/*
 *   mooscript-value-private.h
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

#ifndef __MOO_SCRIPT_VALUE_PRIVATE_H__
#define __MOO_SCRIPT_VALUE_PRIVATE_H__

#include "mooscript-value.h"
#include "mooscript-context.h"

G_BEGIN_DECLS


#define MS_TYPE_VALUE (_ms_value_get_type ())

typedef struct _MSValueClass MSValueClass;

#undef MS_VALUE_TYPE
#define MS_VALUE_TYPE(val) ((val)->klass->type)

typedef enum {
    MS_OP_PLUS,
    MS_OP_MINUS,
    MS_OP_MULT,
    MS_OP_DIV,
    MS_OP_AND,
    MS_OP_OR,
    MS_OP_EQ,
    MS_OP_NEQ,
    MS_OP_LT,
    MS_OP_GT,
    MS_OP_LE,
    MS_OP_GE,
    MS_OP_FORMAT,
    MS_OP_IN,
    MS_BINARY_OP_LAST
} MSBinaryOp;

typedef enum {
    MS_OP_UMINUS,
    MS_OP_NOT,
    MS_OP_LEN,
    MS_UNARY_OP_LAST
} MSUnaryOp;

struct _MSValueClass {
    MSValueType type;
    GHashTable *methods;
};

struct _MSValue {
    guint ref_count;
    MSValueClass *klass;
    GHashTable *methods;

    union {
        int ival;
        char *str;
        GValue *gval;
        GHashTable *hash;

        struct {
            MSValue **elms;
            guint n_elms;
        } list;

        struct {
            MSFunc *func;
            MSValue *obj;
            guint meth : 1;
        } func;
    } u;
};


GType        _ms_value_get_type         (void) G_GNUC_CONST;
void         _ms_type_init_builtin      (MSValueClass   *types);

void         _ms_value_class_add_method (MSValueClass   *klass,
                                         const char     *name,
                                         MSFunc         *func);
MSValue     *_ms_value_get_method       (MSValue        *value,
                                         const char     *name);

char        *_ms_printf                 (const char     *format,
                                         ...) G_GNUC_PRINTF (1, 2);
char        *_ms_vaprintf               (const char     *format,
                                         va_list         args);

gboolean     _ms_value_is_func          (MSValue        *val);
MSValue     *_ms_value_call             (MSValue        *func,
                                         MSValue       **args,
                                         guint           n_args,
                                         MSContext      *ctx);


G_END_DECLS

#endif /* __MOO_SCRIPT_VALUE_PRIVATE_H__ */
