/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mooscript-node.h
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

#ifndef __MOO_SCRIPT_NODE_H__
#define __MOO_SCRIPT_NODE_H__

#include "mooscript-func.h"

G_BEGIN_DECLS


#define AS_TYPE_NODE                    (as_node_get_type ())
#define AS_NODE(object)                 (G_TYPE_CHECK_INSTANCE_CAST ((object), AS_TYPE_NODE, ASNode))
#define AS_NODE_CLASS(klass)            (G_TYPE_CHECK_CLASS_CAST ((klass), AS_TYPE_NODE, ASNodeClass))
#define AS_IS_NODE(object)              (G_TYPE_CHECK_INSTANCE_TYPE ((object), AS_TYPE_NODE))
#define AS_IS_NODE_CLASS(klass)         (G_TYPE_CHECK_CLASS_TYPE ((klass), AS_TYPE_NODE))
#define AS_NODE_GET_CLASS(obj)          (G_TYPE_INSTANCE_GET_CLASS ((obj), AS_TYPE_NODE, ASNodeClass))

#define AS_TYPE_NODE_LIST               (as_node_list_get_type ())
#define AS_NODE_LIST(object)            (G_TYPE_CHECK_INSTANCE_CAST ((object), AS_TYPE_NODE_LIST, ASNodeList))
#define AS_NODE_LIST_CLASS(klass)       (G_TYPE_CHECK_CLASS_CAST ((klass), AS_TYPE_NODE_LIST, ASNodeListClass))
#define AS_IS_NODE_LIST(object)         (G_TYPE_CHECK_INSTANCE_TYPE ((object), AS_TYPE_NODE_LIST))
#define AS_IS_NODE_LIST_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE ((klass), AS_TYPE_NODE_LIST))
#define AS_NODE_LIST_GET_CLASS(obj)     (G_TYPE_INSTANCE_GET_CLASS ((obj), AS_TYPE_NODE_LIST, ASNodeListClass))

#define AS_TYPE_NODE_VAR                (as_node_var_get_type ())
#define AS_NODE_VAR(object)             (G_TYPE_CHECK_INSTANCE_CAST ((object), AS_TYPE_NODE_VAR, ASNodeVar))
#define AS_NODE_VAR_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), AS_TYPE_NODE_VAR, ASNodeVarClass))
#define AS_IS_NODE_VAR(object)          (G_TYPE_CHECK_INSTANCE_TYPE ((object), AS_TYPE_NODE_VAR))
#define AS_IS_NODE_VAR_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), AS_TYPE_NODE_VAR))
#define AS_NODE_VAR_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), AS_TYPE_NODE_VAR, ASNodeVarClass))

#define AS_TYPE_NODE_COMMAND            (as_node_command_get_type ())
#define AS_NODE_COMMAND(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), AS_TYPE_NODE_COMMAND, ASNodeCommand))
#define AS_NODE_COMMAND_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), AS_TYPE_NODE_COMMAND, ASNodeCommandClass))
#define AS_IS_NODE_COMMAND(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), AS_TYPE_NODE_COMMAND))
#define AS_IS_NODE_COMMAND_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), AS_TYPE_NODE_COMMAND))
#define AS_NODE_COMMAND_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), AS_TYPE_NODE_COMMAND, ASNodeCommandClass))

#define AS_TYPE_NODE_IF_ELSE            (as_node_if_else_get_type ())
#define AS_NODE_IF_ELSE(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), AS_TYPE_NODE_IF_ELSE, ASNodeIfElse))
#define AS_NODE_IF_ELSE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), AS_TYPE_NODE_IF_ELSE, ASNodeIfElseClass))
#define AS_IS_NODE_IF_ELSE(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), AS_TYPE_NODE_IF_ELSE))
#define AS_IS_NODE_IF_ELSE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), AS_TYPE_NODE_IF_ELSE))
#define AS_NODE_IF_ELSE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), AS_TYPE_NODE_IF_ELSE, ASNodeIfElseClass))

#define AS_TYPE_NODE_LOOP               (as_node_loop_get_type ())
#define AS_NODE_LOOP(object)            (G_TYPE_CHECK_INSTANCE_CAST ((object), AS_TYPE_NODE_LOOP, ASNodeLoop))
#define AS_NODE_LOOP_CLASS(klass)       (G_TYPE_CHECK_CLASS_CAST ((klass), AS_TYPE_NODE_LOOP, ASNodeLoopClass))
#define AS_IS_NODE_LOOP(object)         (G_TYPE_CHECK_INSTANCE_TYPE ((object), AS_TYPE_NODE_LOOP))
#define AS_IS_NODE_LOOP_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE ((klass), AS_TYPE_NODE_LOOP))
#define AS_NODE_LOOP_GET_CLASS(obj)     (G_TYPE_INSTANCE_GET_CLASS ((obj), AS_TYPE_NODE_LOOP, ASNodeLoopClass))

#define AS_TYPE_NODE_ASSIGN             (as_node_assign_get_type ())
#define AS_NODE_ASSIGN(object)          (G_TYPE_CHECK_INSTANCE_CAST ((object), AS_TYPE_NODE_ASSIGN, ASNodeAssign))
#define AS_NODE_ASSIGN_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), AS_TYPE_NODE_ASSIGN, ASNodeAssignClass))
#define AS_IS_NODE_ASSIGN(object)       (G_TYPE_CHECK_INSTANCE_TYPE ((object), AS_TYPE_NODE_ASSIGN))
#define AS_IS_NODE_ASSIGN_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), AS_TYPE_NODE_ASSIGN))
#define AS_NODE_ASSIGN_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), AS_TYPE_NODE_ASSIGN, ASNodeAssignClass))

#define AS_TYPE_NODE_VALUE              (as_node_value_get_type ())
#define AS_NODE_VALUE(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), AS_TYPE_NODE_VALUE, ASNodeValue))
#define AS_NODE_VALUE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), AS_TYPE_NODE_VALUE, ASNodeValueClass))
#define AS_IS_NODE_VALUE(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), AS_TYPE_NODE_VALUE))
#define AS_IS_NODE_VALUE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), AS_TYPE_NODE_VALUE))
#define AS_NODE_VALUE_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), AS_TYPE_NODE_VALUE, ASNodeValueClass))

#define AS_TYPE_NODE_VAL_LIST            (as_node_val_list_get_type ())
#define AS_NODE_VAL_LIST(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), AS_TYPE_NODE_VAL_LIST, ASNodeValList))
#define AS_NODE_VAL_LIST_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), AS_TYPE_NODE_VAL_LIST, ASNodeValListClass))
#define AS_IS_NODE_VAL_LIST(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), AS_TYPE_NODE_VAL_LIST))
#define AS_IS_NODE_VAL_LIST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), AS_TYPE_NODE_VAL_LIST))
#define AS_NODE_VAL_LIST_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), AS_TYPE_NODE_VAL_LIST, ASNodeValListClass))

typedef struct _ASNode ASNode;
typedef struct _ASNodeClass ASNodeClass;
typedef struct _ASNodeList ASNodeList;
typedef struct _ASNodeListClass ASNodeListClass;
typedef struct _ASNodeVar ASNodeVar;
typedef struct _ASNodeVarClass ASNodeVarClass;
typedef struct _ASNodeCommand ASNodeCommand;
typedef struct _ASNodeCommandClass ASNodeCommandClass;
typedef struct _ASNodeIfElse ASNodeIfElse;
typedef struct _ASNodeIfElseClass ASNodeIfElseClass;
typedef struct _ASNodeLoop ASNodeLoop;
typedef struct _ASNodeLoopClass ASNodeLoopClass;
typedef struct _ASNodeAssign ASNodeAssign;
typedef struct _ASNodeAssignClass ASNodeAssignClass;
typedef struct _ASNodeValue ASNodeValue;
typedef struct _ASNodeValueClass ASNodeValueClass;
typedef struct _ASNodeValList ASNodeValList;
typedef struct _ASNodeValListClass ASNodeValListClass;


typedef ASValue* (*ASNodeEval) (ASNode *node, ASContext *ctx);


struct _ASNode {
    GObject object;
};

struct _ASNodeClass {
    GObjectClass object_class;
    ASNodeEval eval;
};


struct _ASNodeList {
    ASNode node;
    ASNode **nodes;
    guint n_nodes;
    guint n_nodes_allocd__;
};

struct _ASNodeListClass {
    ASNodeClass node_class;
};


typedef enum {
    AS_VAR_POSITIONAL,
    AS_VAR_NAMED
} ASNodeVarType;

struct _ASNodeVar {
    ASNode node;
    ASNodeVarType type;
    union {
        guint num;
        char *name;
    };
};

struct _ASNodeVarClass {
    ASNodeClass node_class;
};


struct _ASNodeCommand {
    ASNode node;
    char *name;
    ASNodeList *args;
};

struct _ASNodeCommandClass {
    ASNodeClass node_class;
};


typedef enum {
    AS_LOOP_TIMES,
    AS_LOOP_WHILE
} ASLoopType;

struct _ASNodeLoop {
    ASNode node;
    ASLoopType type;
    ASNode *condition;
    ASNode *what;
};

struct _ASNodeLoopClass {
    ASNodeClass node_class;
};


struct _ASNodeIfElse {
    ASNode node;
    ASNode *condition;
    ASNode *then_;
    ASNode *else_;
};

struct _ASNodeIfElseClass {
    ASNodeClass node_class;
};


struct _ASNodeAssign {
    ASNode node;
    ASNodeVar *var;
    ASNode *val;
};

struct _ASNodeAssignClass {
    ASNodeClass node_class;
};


struct _ASNodeValue {
    ASNode node;
    ASValue *value;
};

struct _ASNodeValueClass {
    ASNodeClass node_class;
};


struct _ASNodeValList {
    ASNode node;
    ASNodeList *elms;
};

struct _ASNodeValListClass {
    ASNodeClass node_class;
};


GType           as_node_get_type            (void) G_GNUC_CONST;
GType           as_node_list_get_type       (void) G_GNUC_CONST;
GType           as_node_var_get_type        (void) G_GNUC_CONST;
GType           as_node_command_get_type    (void) G_GNUC_CONST;
GType           as_node_if_else_get_type    (void) G_GNUC_CONST;
GType           as_node_loop_get_type       (void) G_GNUC_CONST;
GType           as_node_assign_get_type     (void) G_GNUC_CONST;
GType           as_node_val_list_get_type   (void) G_GNUC_CONST;
GType           as_node_value_get_type      (void) G_GNUC_CONST;

ASValue        *as_node_eval                (ASNode     *node,
                                             ASContext  *ctx);

ASNodeList     *as_node_list_new            (void);
void            as_node_list_add            (ASNodeList *list,
                                             ASNode     *node);

ASNodeCommand  *as_node_command_new         (const char *name,
                                             ASNodeList *args);
ASNodeCommand  *as_node_binary_op_new       (ASBinaryOp  op,
                                             ASNode     *lval,
                                             ASNode     *rval);
ASNodeCommand  *as_node_unary_op_new        (ASUnaryOp   op,
                                             ASNode     *val);

ASNodeIfElse   *as_node_if_else_new         (ASNode     *condition,
                                             ASNode     *then_,
                                             ASNode     *else_);

ASNodeLoop     *as_node_loop_new            (ASLoopType  type,
                                             ASNode     *times_or_cond,
                                             ASNode     *what);

ASNodeAssign   *as_node_assign_new          (ASNodeVar  *var,
                                             ASNode     *val);

ASNodeValue    *as_node_value_new           (ASValue    *value);
ASNodeValList  *as_node_val_list_new        (ASNodeList *list);

ASNodeVar      *as_node_var_new_positional  (guint       n);
ASNodeVar      *as_node_var_new_named       (const char *name);


G_END_DECLS

#endif /* __MOO_SCRIPT_NODE_H__ */
