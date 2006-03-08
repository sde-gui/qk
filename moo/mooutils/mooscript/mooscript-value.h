/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mooscript-value.h
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

#ifndef __MOO_SCRIPT_VALUE_H__
#define __MOO_SCRIPT_VALUE_H__

#include <glib-object.h>

G_BEGIN_DECLS


typedef struct _MSValue MSValue;
typedef struct _MSValueClass MSValueClass;
typedef struct _MSFunc MSFunc;
typedef struct _MSContext MSContext;

#define MS_VALUE_TYPE(val) ((val)->klass->type)

typedef enum {
    MS_VALUE_NONE,
    MS_VALUE_INT,
    MS_VALUE_STRING,
    MS_VALUE_GVALUE,
    MS_VALUE_LIST,
    MS_VALUE_DICT,
    MS_VALUE_FUNC,
    MS_VALUE_INVALID
} MSValueType;

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
    };
};


void         ms_type_init               (void);
void         _ms_type_init_builtin      (MSValueClass   *types);

void         ms_value_class_add_method  (MSValueClass   *klass,
                                         const char     *name,
                                         MSFunc         *func);
void         ms_value_add_method        (MSValue        *value,
                                         const char     *name,
                                         MSFunc         *func);
MSValue     *ms_value_get_method        (MSValue        *value,
                                         const char     *name);

const char  *ms_binary_op_name          (MSBinaryOp      op);
gpointer     ms_binary_op_cfunc         (MSBinaryOp      op);
const char  *ms_unary_op_name           (MSUnaryOp       op);
gpointer     ms_unary_op_cfunc          (MSUnaryOp       op);

MSValue     *ms_value_none              (void);
MSValue     *ms_value_false             (void);
MSValue     *ms_value_true              (void);
MSValue     *ms_value_bool              (gboolean        val);

gboolean     ms_value_is_none           (MSValue        *value);

MSValue     *ms_value_int               (int             val);
MSValue     *ms_value_string            (const char     *string);
MSValue     *ms_value_string_len        (const char     *string,
                                         int             chars);
MSValue     *ms_value_take_string       (char           *string);
MSValue     *ms_value_gvalue            (const GValue   *gval);

MSValue     *ms_value_list              (guint           n_elms);
void         ms_value_list_set_elm      (MSValue        *list,
                                         guint           index,
                                         MSValue        *elm);

MSValue     *ms_value_dict              (void);
void         ms_value_dict_set_elm      (MSValue        *dict,
                                         const char     *key,
                                         MSValue        *val);
MSValue     *ms_value_dict_get_elm      (MSValue        *dict,
                                         const char     *key);

MSValue     *ms_value_ref               (MSValue        *val);
void         ms_value_unref             (MSValue        *val);

gboolean     ms_value_get_bool          (MSValue        *val);
gboolean     ms_value_get_int           (MSValue        *val,
                                         int            *ival);
char        *ms_value_print             (MSValue        *val);

gboolean     ms_value_equal             (MSValue        *a,
                                         MSValue        *b);
int          ms_value_cmp               (MSValue        *a,
                                         MSValue        *b);

MSValue     *ms_value_func              (MSFunc         *func);
MSValue     *ms_value_meth              (MSFunc         *func);
MSValue     *ms_value_bound_meth        (MSFunc         *func,
                                         MSValue        *obj);
gboolean     ms_value_is_func           (MSValue        *val);
MSValue     *ms_value_call              (MSValue        *func,
                                         MSValue       **args,
                                         guint           n_args,
                                         MSContext      *ctx);


G_END_DECLS

#endif /* __MOO_SCRIPT_VALUE_H__ */
