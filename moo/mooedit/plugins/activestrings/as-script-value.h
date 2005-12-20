/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   as-script-value.h
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

#ifndef __AS_SCRIPT_VALUE_H__
#define __AS_SCRIPT_VALUE_H__

#include <glib-object.h>

G_BEGIN_DECLS


typedef struct _ASValue ASValue;

typedef enum {
    AS_VALUE_NONE,
    AS_VALUE_INT,
    AS_VALUE_STRING,
    AS_VALUE_OBJECT,
    AS_VALUE_GVALUE,
    AS_VALUE_LIST
} ASValueType;

typedef enum {
    AS_OP_PLUS,
    AS_OP_MINUS,
    AS_OP_MULT,
    AS_OP_DIV,
    AS_OP_AND,
    AS_OP_OR,
    AS_OP_EQ,
    AS_OP_NEQ,
    AS_OP_LT,
    AS_OP_GT,
    AS_OP_LE,
    AS_OP_GE,
    AS_OP_PRINT,
    AS_BINARY_OP_LAST
} ASBinaryOp;

typedef enum {
    AS_OP_UMINUS,
    AS_OP_NOT,
    AS_UNARY_OP_LAST
} ASUnaryOp;

struct _ASValue {
    guint ref_count;
    ASValueType type;
    union {
        int ival;
        char *str;
        gpointer ptr;
        GValue *gval;
        struct {
            ASValue **elms;
            guint n_elms;
        } list;
    };
};


const char  *as_binary_op_name      (ASBinaryOp  op);
gpointer     as_binary_op_cfunc     (ASBinaryOp  op);
const char  *as_unary_op_name       (ASUnaryOp   op);
gpointer     as_unary_op_cfunc      (ASUnaryOp   op);

ASValue     *as_value_none          (void);
ASValue     *as_value_false         (void);
ASValue     *as_value_true          (void);
ASValue     *as_value_bool          (gboolean    val);

ASValue     *as_value_int           (int         val);
ASValue     *as_value_string        (const char *string);
ASValue     *as_value_take_string   (char       *string);
ASValue     *as_value_object        (gpointer    object);
ASValue     *as_value_gvalue        (GValue     *gval);

ASValue     *as_value_list          (guint       n_elms);
void         as_value_list_set_elm  (ASValue    *list,
                                     guint       index,
                                     ASValue    *elm);

ASValue     *as_value_ref           (ASValue    *val);
void         as_value_unref         (ASValue    *val);

gboolean     as_value_get_bool      (ASValue    *val);
gboolean     as_value_get_int       (ASValue    *val,
                                     int        *ival);
char        *as_value_print         (ASValue    *val);

gboolean     as_value_equal         (ASValue    *a,
                                     ASValue    *b);
int          as_value_cmp           (ASValue    *a,
                                     ASValue    *b);


G_END_DECLS

#endif /* __AS_SCRIPT_VALUE_H__ */
