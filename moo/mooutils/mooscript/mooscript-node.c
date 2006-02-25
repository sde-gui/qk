/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   as-script-node.c
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

#include "mooscript-node.h"
#include "mooscript-func.h"
#include "mooscript-context.h"
#include <string.h>


G_DEFINE_TYPE(ASNode, as_node, G_TYPE_OBJECT)


static void
as_node_class_init (G_GNUC_UNUSED ASNodeClass *klass)
{
}

static void
as_node_init (G_GNUC_UNUSED ASNode *node)
{
}


ASValue *
as_node_eval (ASNode     *node,
              ASContext  *ctx)
{
    ASNodeEval eval;

    g_return_val_if_fail (AS_IS_NODE (node), NULL);
    g_return_val_if_fail (ctx != NULL, NULL);

    eval = AS_NODE_GET_CLASS(node)->eval;

    if (!eval)
    {
        g_critical ("eval method missing");
        return NULL;
    }

    return eval (node, ctx);
}


/****************************************************************************/
/* ASNodeList
 */

G_DEFINE_TYPE(ASNodeList, as_node_list, AS_TYPE_NODE)


static ASValue*
as_node_list_eval (ASNode    *node,
                   ASContext *ctx)
{
    guint i;
    ASValue *ret = NULL;
    ASNodeList *list = AS_NODE_LIST (node);

    if (!list->n_nodes)
        return as_value_none ();

    for (i = 0; i < list->n_nodes; ++i)
    {
        ret = as_node_eval (list->nodes[i], ctx);

        if (!ret)
            return NULL;

        if (i + 1 < list->n_nodes)
            as_value_unref (ret);
    }

    return ret;
}


static void
as_node_list_finalize (GObject *object)
{
    guint i;
    ASNodeList *list = AS_NODE_LIST (object);

    for (i = 0; i < list->n_nodes; ++i)
        g_object_unref (list->nodes[i]);
    g_free (list->nodes);

    G_OBJECT_CLASS(as_node_list_parent_class)->finalize (object);
}


static void
as_node_list_class_init (ASNodeListClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    ASNodeClass *node_class = AS_NODE_CLASS (klass);

    object_class->finalize = as_node_list_finalize;
    node_class->eval = as_node_list_eval;
}


static void
as_node_list_init (G_GNUC_UNUSED ASNodeList *list)
{
}


ASNodeList *
as_node_list_new (void)
{
    return g_object_new (AS_TYPE_NODE_LIST, NULL);
}


void
as_node_list_add (ASNodeList *list,
                  ASNode     *node)
{
    g_return_if_fail (AS_IS_NODE_LIST (list));
    g_return_if_fail (AS_IS_NODE (node));

    if (list->n_nodes_allocd__ == list->n_nodes)
    {
        ASNode **tmp;
        list->n_nodes_allocd__ = MAX (list->n_nodes_allocd__ + 2,
                                      1.2 * list->n_nodes_allocd__);
        tmp = g_new0 (ASNode*, list->n_nodes_allocd__);
        if (list->n_nodes)
            memcpy (tmp, list->nodes, list->n_nodes * sizeof(ASNode*));
        g_free (list->nodes);
        list->nodes = tmp;
    }

    list->nodes[list->n_nodes++] = g_object_ref (node);
}


/****************************************************************************/
/* ASNodeVar
 */

G_DEFINE_TYPE(ASNodeVar, as_node_var, AS_TYPE_NODE)


static void
as_node_var_finalize (GObject *object)
{
    ASNodeVar *var = AS_NODE_VAR (object);

    if (var->type == AS_VAR_NAMED)
        g_free (var->name);

    G_OBJECT_CLASS(as_node_var_parent_class)->finalize (object);
}


static ASValue *
as_node_var_eval (ASNode    *node,
                  ASContext *ctx)
{
    ASNodeVar *var = AS_NODE_VAR (node);

    switch (var->type)
    {
        case AS_VAR_POSITIONAL:
            return as_context_eval_positional (ctx, var->num);
        case AS_VAR_NAMED:
            return as_context_eval_named (ctx, var->name);
    }

    g_return_val_if_reached (NULL);
}


static void
as_node_var_class_init (ASNodeVarClass *klass)
{
    G_OBJECT_CLASS(klass)->finalize = as_node_var_finalize;
    AS_NODE_CLASS(klass)->eval = as_node_var_eval;
}


static void
as_node_var_init (ASNodeVar *var)
{
    var->type = AS_VAR_NAMED;
}


ASNodeVar *
as_node_var_new_positional (guint n)
{
    ASNodeVar *var;

    var = g_object_new (AS_TYPE_NODE_VAR, NULL);
    var->type = AS_VAR_POSITIONAL;
    var->num = n;

    return var;
}


ASNodeVar *
as_node_var_new_named (const char *name)
{
    ASNodeVar *var;

    g_return_val_if_fail (name && name[0], NULL);

    var = g_object_new (AS_TYPE_NODE_VAR, NULL);
    var->type = AS_VAR_NAMED;
    var->name = g_strdup (name);

    return var;
}


/****************************************************************************/
/* ASNodeCommand
 */

G_DEFINE_TYPE(ASNodeCommand, as_node_command, AS_TYPE_NODE)


static void
as_node_command_finalize (GObject *object)
{
    ASNodeCommand *cmd = AS_NODE_COMMAND (object);

    g_free (cmd->name);

    if (cmd->args)
        g_object_unref (cmd->args);

    G_OBJECT_CLASS(as_node_command_parent_class)->finalize (object);
}


static ASValue *
as_node_command_eval (ASNode    *node,
                      ASContext *ctx)
{
    guint i, n_args;
    ASValue **args;
    ASValue *ret;
    ASNodeCommand *cmd = AS_NODE_COMMAND (node);
    ASFunc *func;

    g_return_val_if_fail (cmd->name != NULL, NULL);

    func = as_context_lookup_func (ctx, cmd->name);

    if (!func)
        return as_context_format_error (ctx, AS_ERROR_NAME,
                                        "unknown command '%s'",
                                        cmd->name);

    g_object_ref (func);
    n_args = cmd->args ? cmd->args->n_nodes : 0;
    args = NULL;
    ret = NULL;

    if (n_args)
    {
        args = g_new0 (ASValue*, n_args);

        for (i = 0; i < n_args; ++i)
        {
            args[i] = as_node_eval (cmd->args->nodes[i], ctx);

            if (!args[i])
                goto out;
        }
    }

    ret = as_func_call (func, args, n_args, ctx);

out:
    for (i = 0; i < n_args; ++i)
        if (args[i])
            as_value_unref (args[i]);
    g_free (args);
    g_object_unref (func);
    return ret;
}


static void
as_node_command_class_init (ASNodeCommandClass *klass)
{
    G_OBJECT_CLASS(klass)->finalize = as_node_command_finalize;
    AS_NODE_CLASS(klass)->eval = as_node_command_eval;
}


static void
as_node_command_init (G_GNUC_UNUSED ASNodeCommand *cmd)
{
}


ASNodeCommand *
as_node_command_new (const char *name,
                     ASNodeList *args)
{
    ASNodeCommand *cmd;

    g_return_val_if_fail (name && name[0], NULL);
    g_return_val_if_fail (!args || AS_IS_NODE_LIST (args), NULL);

    if (args && !args->n_nodes)
        args = NULL;

    cmd = g_object_new (AS_TYPE_NODE_COMMAND, NULL);

    cmd->name = g_strdup (name);
    cmd->args = args ? g_object_ref (args) : NULL;

    return cmd;
}


ASNodeCommand *
as_node_binary_op_new (ASBinaryOp  op,
                       ASNode     *lval,
                       ASNode     *rval)
{
    ASNodeCommand *cmd;
    ASNodeList *args;
    const char *name;

    g_return_val_if_fail (AS_IS_NODE (lval), NULL);
    g_return_val_if_fail (AS_IS_NODE (rval), NULL);

    name = as_binary_op_name (op);
    g_return_val_if_fail (name != NULL, NULL);

    args = as_node_list_new ();
    as_node_list_add (args, lval);
    as_node_list_add (args, rval);

    cmd = as_node_command_new (name, args);

    g_object_unref (args);
    return cmd;
}


ASNodeCommand *
as_node_unary_op_new (ASUnaryOp   op,
                      ASNode     *val)
{
    ASNodeCommand *cmd;
    ASNodeList *args;
    const char *name;

    g_return_val_if_fail (AS_IS_NODE (val), NULL);

    name = as_unary_op_name (op);
    g_return_val_if_fail (name != NULL, NULL);

    args = as_node_list_new ();
    as_node_list_add (args, val);

    cmd = as_node_command_new (name, args);

    g_object_unref (args);
    return cmd;
}


/****************************************************************************/
/* ASNodeIfElse
 */

G_DEFINE_TYPE(ASNodeIfElse, as_node_if_else, AS_TYPE_NODE)


static void
as_node_if_else_finalize (GObject *object)
{
    ASNodeIfElse *node = AS_NODE_IF_ELSE (object);

    g_object_unref (node->condition);
    g_object_unref (node->then_);
    if (node->else_)
        g_object_unref (node->else_);

    G_OBJECT_CLASS(as_node_if_else_parent_class)->finalize (object);
}


static ASValue *
as_node_if_else_eval (ASNode    *node_,
                      ASContext *ctx)
{
    ASValue *ret = NULL, *cond;
    ASNodeIfElse *node = AS_NODE_IF_ELSE (node_);

    g_return_val_if_fail (node->condition != NULL, NULL);
    g_return_val_if_fail (node->then_ != NULL, NULL);

    cond = as_node_eval (node->condition, ctx);

    if (!cond)
        return NULL;

    if (as_value_get_bool (cond))
        ret = as_node_eval (node->then_, ctx);
    else if (node->else_)
        ret = as_node_eval (node->else_, ctx);
    else
        ret = as_value_none ();

    as_value_unref (cond);
    return ret;
}


static void
as_node_if_else_class_init (ASNodeIfElseClass *klass)
{
    G_OBJECT_CLASS(klass)->finalize = as_node_if_else_finalize;
    AS_NODE_CLASS(klass)->eval = as_node_if_else_eval;
}


static void
as_node_if_else_init (G_GNUC_UNUSED ASNodeIfElse *node)
{
}


ASNodeIfElse *
as_node_if_else_new (ASNode     *condition,
                     ASNode     *then_,
                     ASNode     *else_)
{
    ASNodeIfElse *node;

    g_return_val_if_fail (AS_IS_NODE (condition), NULL);
    g_return_val_if_fail (AS_IS_NODE (then_), NULL);
    g_return_val_if_fail (!else_ || AS_IS_NODE (else_), NULL);

    node = g_object_new (AS_TYPE_NODE_IF_ELSE, NULL);
    node->condition = g_object_ref (condition);
    node->then_ = g_object_ref (then_);
    node->else_ = else_ ? g_object_ref (else_) : NULL;

    return node;
}


/****************************************************************************/
/* ASNodeLoop
 */

G_DEFINE_TYPE(ASNodeLoop, as_node_loop, AS_TYPE_NODE)


static void
as_node_loop_finalize (GObject *object)
{
    ASNodeLoop *loop = AS_NODE_LOOP (object);

    g_object_unref (loop->condition);
    if (loop->what)
        g_object_unref (loop->what);

    G_OBJECT_CLASS(as_node_loop_parent_class)->finalize (object);
}


static ASValue *
as_node_loop_times (ASNodeLoop *loop,
                    ASContext  *ctx)
{
    ASValue *times, *ret;
    int n_times, i;

    times = as_node_eval (loop->condition, ctx);
    ret = NULL;

    if (!times)
        return NULL;

    if (!as_value_get_int (times, &n_times))
    {
        as_context_format_error (ctx, AS_ERROR_TYPE,
                                 "could not convert value to int");
        as_value_unref (times);
        return NULL;
    }

    as_value_unref (times);

    if (n_times < 0)
    {
        as_context_format_error (ctx, AS_ERROR_VALUE,
                                 "can not repeat negative number of times");
        return NULL;
    }

    for (i = 0; i < n_times; ++i)
    {
        ret = as_node_eval (loop->what, ctx);

        if (!ret)
            return NULL;

        if (i + 1 < n_times)
            as_value_unref (ret);
    }

    return ret ? ret : as_value_none ();
}


static ASValue *
as_node_loop_while (ASNodeLoop *loop,
                    ASContext  *ctx)
{
    ASValue *ret;

    ret = NULL;

    while (TRUE)
    {
        ASValue *cond = as_node_eval (loop->condition, ctx);
        gboolean doit;

        if (!cond)
            return NULL;

        doit = as_value_get_bool (cond);
        as_value_unref (cond);

        if (doit)
        {
            if (ret)
                as_value_unref (ret);

            ret = as_node_eval (loop->what, ctx);

            if (!ret)
                return NULL;
        }
        else
        {
            return ret ? ret : as_value_none ();
        }
    }
}


static ASValue *
as_node_loop_eval (ASNode    *node,
                   ASContext *ctx)
{
    ASNodeLoop *loop = AS_NODE_LOOP (node);

    g_return_val_if_fail (loop->condition != NULL, NULL);

    switch (loop->type)
    {
        case AS_LOOP_TIMES:
            return as_node_loop_times (loop, ctx);
        case AS_LOOP_WHILE:
            return as_node_loop_while (loop, ctx);
    }

    g_return_val_if_reached (NULL);
}


static void
as_node_loop_class_init (ASNodeLoopClass *klass)
{
    G_OBJECT_CLASS(klass)->finalize = as_node_loop_finalize;
    AS_NODE_CLASS(klass)->eval = as_node_loop_eval;
}


static void
as_node_loop_init (ASNodeLoop *loop)
{
    loop->type = -1;
}


ASNodeLoop *
as_node_loop_new (ASLoopType  type,
                  ASNode     *condition,
                  ASNode     *what)
{
    ASNodeLoop *loop;

    g_return_val_if_fail (AS_IS_NODE (condition), NULL);
    g_return_val_if_fail (!what || AS_IS_NODE (what), NULL);

    loop = g_object_new (AS_TYPE_NODE_LOOP, NULL);
    loop->type = type;
    loop->condition = g_object_ref (condition);
    loop->what = what ? g_object_ref (what) : NULL;

    return loop;
}


/****************************************************************************/
/* ASNodeAssign
 */

G_DEFINE_TYPE(ASNodeAssign, as_node_assign, AS_TYPE_NODE)


static void
as_node_assign_finalize (GObject *object)
{
    ASNodeAssign *node = AS_NODE_ASSIGN (object);

    g_object_unref (node->var);
    g_object_unref (node->val);

    G_OBJECT_CLASS(as_node_assign_parent_class)->finalize (object);
}


static ASValue *
as_node_assign_eval (ASNode    *node_,
                     ASContext *ctx)
{
    ASValue *value;
    gboolean success = FALSE;
    ASNodeAssign *node = AS_NODE_ASSIGN (node_);

    g_return_val_if_fail (node->var != NULL, NULL);
    g_return_val_if_fail (node->val != NULL, NULL);

    value = as_node_eval (node->val, ctx);

    if (!value)
        return NULL;

    switch (node->var->type)
    {
        case AS_VAR_POSITIONAL:
            success = as_context_assign_positional (ctx, node->var->num, value);
            break;
        case AS_VAR_NAMED:
            success = as_context_assign_named (ctx, node->var->name, value);
            break;
    }

    if (!success)
    {
        as_value_unref (value);
        return NULL;
    }

    return value;
}


static void
as_node_assign_class_init (ASNodeAssignClass *klass)
{
    G_OBJECT_CLASS(klass)->finalize = as_node_assign_finalize;
    AS_NODE_CLASS(klass)->eval = as_node_assign_eval;
}


static void
as_node_assign_init (G_GNUC_UNUSED ASNodeAssign *node)
{
}


ASNodeAssign *
as_node_assign_new (ASNodeVar  *var,
                    ASNode     *val)
{
    ASNodeAssign *node;

    g_return_val_if_fail (AS_IS_NODE_VAR (var), NULL);
    g_return_val_if_fail (AS_IS_NODE (val), NULL);

    node = g_object_new (AS_TYPE_NODE_ASSIGN, NULL);
    node->var = g_object_ref (var);
    node->val = g_object_ref (val);

    return node;
}


/****************************************************************************/
/* ASNodeValue
 */

G_DEFINE_TYPE(ASNodeValue, as_node_value, AS_TYPE_NODE)


static void
as_node_value_finalize (GObject *object)
{
    ASNodeValue *val = AS_NODE_VALUE (object);

    as_value_unref (val->value);

    G_OBJECT_CLASS(as_node_value_parent_class)->finalize (object);
}


static ASValue *
as_node_value_eval (ASNode    *node,
                    G_GNUC_UNUSED ASContext *ctx)
{
    return as_value_ref (AS_NODE_VALUE(node)->value);
}


static void
as_node_value_class_init (ASNodeValueClass *klass)
{
    G_OBJECT_CLASS(klass)->finalize = as_node_value_finalize;
    AS_NODE_CLASS(klass)->eval = as_node_value_eval;
}


static void
as_node_value_init (G_GNUC_UNUSED ASNodeValue *node)
{
}


ASNodeValue *
as_node_value_new (ASValue *value)
{
    ASNodeValue *node;

    g_return_val_if_fail (value != NULL, NULL);

    node = g_object_new (AS_TYPE_NODE_VALUE, NULL);
    node->value = as_value_ref (value);

    return node;
}


/****************************************************************************/
/* ASNodeValList
 */

G_DEFINE_TYPE(ASNodeValList, as_node_val_list, AS_TYPE_NODE)


static void
as_node_val_list_finalize (GObject *object)
{
    ASNodeValList *node = AS_NODE_VAL_LIST (object);

    if (node->elms)
        g_object_unref (node->elms);

    G_OBJECT_CLASS(as_node_val_list_parent_class)->finalize (object);
}


static ASValue *
as_node_val_list_eval (ASNode    *node_,
                       ASContext *ctx)
{
    ASValue *ret;
    guint n_elms, i;
    ASNodeValList *node = AS_NODE_VAL_LIST (node_);

    n_elms = node->elms ? node->elms->n_nodes : 0;
    ret = as_value_list (n_elms);

    for (i = 0; i < n_elms; ++i)
    {
        ASValue *elm = as_node_eval (node->elms->nodes[i], ctx);

        if (!elm)
        {
            as_value_unref (ret);
            return NULL;
        }

        as_value_list_set_elm (ret, i, elm);
        as_value_unref (elm);
    }

    return ret;
}


static void
as_node_val_list_class_init (ASNodeValListClass *klass)
{
    G_OBJECT_CLASS(klass)->finalize = as_node_val_list_finalize;
    AS_NODE_CLASS(klass)->eval = as_node_val_list_eval;
}


static void
as_node_val_list_init (G_GNUC_UNUSED ASNodeValList *node)
{
}


ASNodeValList *
as_node_val_list_new (ASNodeList *list)
{
    ASNodeValList *node;

    g_return_val_if_fail (!list || AS_IS_NODE_LIST (list), NULL);

    if (list && !list->n_nodes)
        list = NULL;

    node = g_object_new (AS_TYPE_NODE_VAL_LIST, NULL);
    node->elms = list ? g_object_ref (list) : NULL;

    return node;
}
