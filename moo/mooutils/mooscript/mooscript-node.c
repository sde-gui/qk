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


G_DEFINE_TYPE(MSNode, ms_node, G_TYPE_OBJECT)


static void
ms_node_class_init (G_GNUC_UNUSED MSNodeClass *klass)
{
}

static void
ms_node_init (G_GNUC_UNUSED MSNode *node)
{
}


MSValue *
ms_node_eval (MSNode     *node,
              MSContext  *ctx)
{
    MSNodeEval eval;

    g_return_val_if_fail (MS_IS_NODE (node), NULL);
    g_return_val_if_fail (ctx != NULL, NULL);

    eval = MS_NODE_GET_CLASS(node)->eval;

    if (!eval)
    {
        g_critical ("eval method missing");
        return NULL;
    }

    return eval (node, ctx);
}


/****************************************************************************/
/* MSNodeList
 */

G_DEFINE_TYPE(MSNodeList, ms_node_list, MS_TYPE_NODE)


static MSValue*
ms_node_list_eval (MSNode    *node,
                   MSContext *ctx)
{
    guint i;
    MSValue *ret = NULL;
    MSNodeList *list = MS_NODE_LIST (node);

    if (!list->n_nodes)
        return ms_value_none ();

    for (i = 0; i < list->n_nodes; ++i)
    {
        ret = ms_node_eval (list->nodes[i], ctx);

        if (!ret)
            return NULL;

        if (i + 1 < list->n_nodes)
            ms_value_unref (ret);
    }

    return ret;
}


static void
ms_node_list_finalize (GObject *object)
{
    guint i;
    MSNodeList *list = MS_NODE_LIST (object);

    for (i = 0; i < list->n_nodes; ++i)
        g_object_unref (list->nodes[i]);
    g_free (list->nodes);

    G_OBJECT_CLASS(ms_node_list_parent_class)->finalize (object);
}


static void
ms_node_list_class_init (MSNodeListClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    MSNodeClass *node_class = MS_NODE_CLASS (klass);

    object_class->finalize = ms_node_list_finalize;
    node_class->eval = ms_node_list_eval;
}


static void
ms_node_list_init (G_GNUC_UNUSED MSNodeList *list)
{
}


MSNodeList *
ms_node_list_new (void)
{
    return g_object_new (MS_TYPE_NODE_LIST, NULL);
}


void
ms_node_list_add (MSNodeList *list,
                  MSNode     *node)
{
    g_return_if_fail (MS_IS_NODE_LIST (list));
    g_return_if_fail (MS_IS_NODE (node));

    if (list->n_nodes_allocd__ == list->n_nodes)
    {
        MSNode **tmp;
        list->n_nodes_allocd__ = MAX (list->n_nodes_allocd__ + 2,
                                      1.2 * list->n_nodes_allocd__);
        tmp = g_new0 (MSNode*, list->n_nodes_allocd__);
        if (list->n_nodes)
            memcpy (tmp, list->nodes, list->n_nodes * sizeof(MSNode*));
        g_free (list->nodes);
        list->nodes = tmp;
    }

    list->nodes[list->n_nodes++] = g_object_ref (node);
}


/****************************************************************************/
/* MSNodeVar
 */

G_DEFINE_TYPE(MSNodeVar, ms_node_var, MS_TYPE_NODE)


static void
ms_node_var_finalize (GObject *object)
{
    MSNodeVar *var = MS_NODE_VAR (object);

    if (var->type == MS_VAR_NAMED)
        g_free (var->name);

    G_OBJECT_CLASS(ms_node_var_parent_class)->finalize (object);
}


static MSValue *
ms_node_var_eval (MSNode    *node,
                  MSContext *ctx)
{
    MSNodeVar *var = MS_NODE_VAR (node);

    switch (var->type)
    {
        case MS_VAR_POSITIONAL:
            return ms_context_eval_positional (ctx, var->num);
        case MS_VAR_NAMED:
            return ms_context_eval_named (ctx, var->name);
    }

    g_return_val_if_reached (NULL);
}


static void
ms_node_var_class_init (MSNodeVarClass *klass)
{
    G_OBJECT_CLASS(klass)->finalize = ms_node_var_finalize;
    MS_NODE_CLASS(klass)->eval = ms_node_var_eval;
}


static void
ms_node_var_init (MSNodeVar *var)
{
    var->type = MS_VAR_NAMED;
}


MSNodeVar *
ms_node_var_new_positional (guint n)
{
    MSNodeVar *var;

    var = g_object_new (MS_TYPE_NODE_VAR, NULL);
    var->type = MS_VAR_POSITIONAL;
    var->num = n;

    return var;
}


MSNodeVar *
ms_node_var_new_named (const char *name)
{
    MSNodeVar *var;

    g_return_val_if_fail (name && name[0], NULL);

    var = g_object_new (MS_TYPE_NODE_VAR, NULL);
    var->type = MS_VAR_NAMED;
    var->name = g_strdup (name);

    return var;
}


/****************************************************************************/
/* MSNodeCommand
 */

G_DEFINE_TYPE(MSNodeCommand, ms_node_command, MS_TYPE_NODE)


static void
ms_node_command_finalize (GObject *object)
{
    MSNodeCommand *cmd = MS_NODE_COMMAND (object);

    g_free (cmd->name);

    if (cmd->args)
        g_object_unref (cmd->args);

    G_OBJECT_CLASS(ms_node_command_parent_class)->finalize (object);
}


static MSValue *
ms_node_command_eval (MSNode    *node,
                      MSContext *ctx)
{
    guint i, n_args;
    MSValue **args;
    MSValue *ret;
    MSNodeCommand *cmd = MS_NODE_COMMAND (node);
    MSFunc *func;

    g_return_val_if_fail (cmd->name != NULL, NULL);

    func = ms_context_lookup_func (ctx, cmd->name);

    if (!func)
        return ms_context_format_error (ctx, MS_ERROR_NAME,
                                        "unknown command '%s'",
                                        cmd->name);

    g_object_ref (func);
    n_args = cmd->args ? cmd->args->n_nodes : 0;
    args = NULL;
    ret = NULL;

    if (n_args)
    {
        args = g_new0 (MSValue*, n_args);

        for (i = 0; i < n_args; ++i)
        {
            args[i] = ms_node_eval (cmd->args->nodes[i], ctx);

            if (!args[i])
                goto out;
        }
    }

    ret = ms_func_call (func, args, n_args, ctx);

out:
    for (i = 0; i < n_args; ++i)
        if (args[i])
            ms_value_unref (args[i]);
    g_free (args);
    g_object_unref (func);
    return ret;
}


static void
ms_node_command_class_init (MSNodeCommandClass *klass)
{
    G_OBJECT_CLASS(klass)->finalize = ms_node_command_finalize;
    MS_NODE_CLASS(klass)->eval = ms_node_command_eval;
}


static void
ms_node_command_init (G_GNUC_UNUSED MSNodeCommand *cmd)
{
}


MSNodeCommand *
ms_node_command_new (const char *name,
                     MSNodeList *args)
{
    MSNodeCommand *cmd;

    g_return_val_if_fail (name && name[0], NULL);
    g_return_val_if_fail (!args || MS_IS_NODE_LIST (args), NULL);

    if (args && !args->n_nodes)
        args = NULL;

    cmd = g_object_new (MS_TYPE_NODE_COMMAND, NULL);

    cmd->name = g_strdup (name);
    cmd->args = args ? g_object_ref (args) : NULL;

    return cmd;
}


MSNodeCommand *
ms_node_binary_op_new (MSBinaryOp  op,
                       MSNode     *lval,
                       MSNode     *rval)
{
    MSNodeCommand *cmd;
    MSNodeList *args;
    const char *name;

    g_return_val_if_fail (MS_IS_NODE (lval), NULL);
    g_return_val_if_fail (MS_IS_NODE (rval), NULL);

    name = ms_binary_op_name (op);
    g_return_val_if_fail (name != NULL, NULL);

    args = ms_node_list_new ();
    ms_node_list_add (args, lval);
    ms_node_list_add (args, rval);

    cmd = ms_node_command_new (name, args);

    g_object_unref (args);
    return cmd;
}


MSNodeCommand *
ms_node_unary_op_new (MSUnaryOp   op,
                      MSNode     *val)
{
    MSNodeCommand *cmd;
    MSNodeList *args;
    const char *name;

    g_return_val_if_fail (MS_IS_NODE (val), NULL);

    name = ms_unary_op_name (op);
    g_return_val_if_fail (name != NULL, NULL);

    args = ms_node_list_new ();
    ms_node_list_add (args, val);

    cmd = ms_node_command_new (name, args);

    g_object_unref (args);
    return cmd;
}


/****************************************************************************/
/* MSNodeIfElse
 */

G_DEFINE_TYPE(MSNodeIfElse, ms_node_if_else, MS_TYPE_NODE)


static void
ms_node_if_else_finalize (GObject *object)
{
    MSNodeIfElse *node = MS_NODE_IF_ELSE (object);

    g_object_unref (node->condition);
    g_object_unref (node->then_);
    if (node->else_)
        g_object_unref (node->else_);

    G_OBJECT_CLASS(ms_node_if_else_parent_class)->finalize (object);
}


static MSValue *
ms_node_if_else_eval (MSNode    *node_,
                      MSContext *ctx)
{
    MSValue *ret = NULL, *cond;
    MSNodeIfElse *node = MS_NODE_IF_ELSE (node_);

    g_return_val_if_fail (node->condition != NULL, NULL);
    g_return_val_if_fail (node->then_ != NULL, NULL);

    cond = ms_node_eval (node->condition, ctx);

    if (!cond)
        return NULL;

    if (ms_value_get_bool (cond))
        ret = ms_node_eval (node->then_, ctx);
    else if (node->else_)
        ret = ms_node_eval (node->else_, ctx);
    else
        ret = ms_value_none ();

    ms_value_unref (cond);
    return ret;
}


static void
ms_node_if_else_class_init (MSNodeIfElseClass *klass)
{
    G_OBJECT_CLASS(klass)->finalize = ms_node_if_else_finalize;
    MS_NODE_CLASS(klass)->eval = ms_node_if_else_eval;
}


static void
ms_node_if_else_init (G_GNUC_UNUSED MSNodeIfElse *node)
{
}


MSNodeIfElse *
ms_node_if_else_new (MSNode     *condition,
                     MSNode     *then_,
                     MSNode     *else_)
{
    MSNodeIfElse *node;

    g_return_val_if_fail (MS_IS_NODE (condition), NULL);
    g_return_val_if_fail (MS_IS_NODE (then_), NULL);
    g_return_val_if_fail (!else_ || MS_IS_NODE (else_), NULL);

    node = g_object_new (MS_TYPE_NODE_IF_ELSE, NULL);
    node->condition = g_object_ref (condition);
    node->then_ = g_object_ref (then_);
    node->else_ = else_ ? g_object_ref (else_) : NULL;

    return node;
}


/****************************************************************************/
/* MSNodeLoop
 */

G_DEFINE_TYPE(MSNodeLoop, ms_node_loop, MS_TYPE_NODE)


static void
ms_node_loop_finalize (GObject *object)
{
    MSNodeLoop *loop = MS_NODE_LOOP (object);

    g_object_unref (loop->condition);
    if (loop->what)
        g_object_unref (loop->what);

    G_OBJECT_CLASS(ms_node_loop_parent_class)->finalize (object);
}


static MSValue *
ms_node_loop_times (MSNodeLoop *loop,
                    MSContext  *ctx)
{
    MSValue *times, *ret;
    int n_times, i;

    times = ms_node_eval (loop->condition, ctx);
    ret = NULL;

    if (!times)
        return NULL;

    if (!ms_value_get_int (times, &n_times))
    {
        ms_context_format_error (ctx, MS_ERROR_TYPE,
                                 "could not convert value to int");
        ms_value_unref (times);
        return NULL;
    }

    ms_value_unref (times);

    if (n_times < 0)
    {
        ms_context_format_error (ctx, MS_ERROR_VALUE,
                                 "can not repeat negative number of times");
        return NULL;
    }

    for (i = 0; i < n_times; ++i)
    {
        ret = ms_node_eval (loop->what, ctx);

        if (!ret)
            return NULL;

        if (i + 1 < n_times)
            ms_value_unref (ret);
    }

    return ret ? ret : ms_value_none ();
}


static MSValue *
ms_node_loop_while (MSNodeLoop *loop,
                    MSContext  *ctx)
{
    MSValue *ret;

    ret = NULL;

    while (TRUE)
    {
        MSValue *cond = ms_node_eval (loop->condition, ctx);
        gboolean doit;

        if (!cond)
            return NULL;

        doit = ms_value_get_bool (cond);
        ms_value_unref (cond);

        if (doit)
        {
            if (ret)
                ms_value_unref (ret);

            ret = ms_node_eval (loop->what, ctx);

            if (!ret)
                return NULL;
        }
        else
        {
            return ret ? ret : ms_value_none ();
        }
    }
}


static MSValue *
ms_node_loop_eval (MSNode    *node,
                   MSContext *ctx)
{
    MSNodeLoop *loop = MS_NODE_LOOP (node);

    g_return_val_if_fail (loop->condition != NULL, NULL);

    switch (loop->type)
    {
        case MS_LOOP_TIMES:
            return ms_node_loop_times (loop, ctx);
        case MS_LOOP_WHILE:
            return ms_node_loop_while (loop, ctx);
    }

    g_return_val_if_reached (NULL);
}


static void
ms_node_loop_class_init (MSNodeLoopClass *klass)
{
    G_OBJECT_CLASS(klass)->finalize = ms_node_loop_finalize;
    MS_NODE_CLASS(klass)->eval = ms_node_loop_eval;
}


static void
ms_node_loop_init (MSNodeLoop *loop)
{
    loop->type = -1;
}


MSNodeLoop *
ms_node_loop_new (MSLoopType  type,
                  MSNode     *condition,
                  MSNode     *what)
{
    MSNodeLoop *loop;

    g_return_val_if_fail (MS_IS_NODE (condition), NULL);
    g_return_val_if_fail (!what || MS_IS_NODE (what), NULL);

    loop = g_object_new (MS_TYPE_NODE_LOOP, NULL);
    loop->type = type;
    loop->condition = g_object_ref (condition);
    loop->what = what ? g_object_ref (what) : NULL;

    return loop;
}


/****************************************************************************/
/* MSNodeAssign
 */

G_DEFINE_TYPE(MSNodeAssign, ms_node_assign, MS_TYPE_NODE)


static void
ms_node_assign_finalize (GObject *object)
{
    MSNodeAssign *node = MS_NODE_ASSIGN (object);

    g_object_unref (node->var);
    g_object_unref (node->val);

    G_OBJECT_CLASS(ms_node_assign_parent_class)->finalize (object);
}


static MSValue *
ms_node_assign_eval (MSNode    *node_,
                     MSContext *ctx)
{
    MSValue *value;
    gboolean success = FALSE;
    MSNodeAssign *node = MS_NODE_ASSIGN (node_);

    g_return_val_if_fail (node->var != NULL, NULL);
    g_return_val_if_fail (node->val != NULL, NULL);

    value = ms_node_eval (node->val, ctx);

    if (!value)
        return NULL;

    switch (node->var->type)
    {
        case MS_VAR_POSITIONAL:
            success = ms_context_assign_positional (ctx, node->var->num, value);
            break;
        case MS_VAR_NAMED:
            success = ms_context_assign_named (ctx, node->var->name, value);
            break;
    }

    if (!success)
    {
        ms_value_unref (value);
        return NULL;
    }

    return value;
}


static void
ms_node_assign_class_init (MSNodeAssignClass *klass)
{
    G_OBJECT_CLASS(klass)->finalize = ms_node_assign_finalize;
    MS_NODE_CLASS(klass)->eval = ms_node_assign_eval;
}


static void
ms_node_assign_init (G_GNUC_UNUSED MSNodeAssign *node)
{
}


MSNodeAssign *
ms_node_assign_new (MSNodeVar  *var,
                    MSNode     *val)
{
    MSNodeAssign *node;

    g_return_val_if_fail (MS_IS_NODE_VAR (var), NULL);
    g_return_val_if_fail (MS_IS_NODE (val), NULL);

    node = g_object_new (MS_TYPE_NODE_ASSIGN, NULL);
    node->var = g_object_ref (var);
    node->val = g_object_ref (val);

    return node;
}


/****************************************************************************/
/* MSNodeValue
 */

G_DEFINE_TYPE(MSNodeValue, ms_node_value, MS_TYPE_NODE)


static void
ms_node_value_finalize (GObject *object)
{
    MSNodeValue *val = MS_NODE_VALUE (object);

    ms_value_unref (val->value);

    G_OBJECT_CLASS(ms_node_value_parent_class)->finalize (object);
}


static MSValue *
ms_node_value_eval (MSNode    *node,
                    G_GNUC_UNUSED MSContext *ctx)
{
    return ms_value_ref (MS_NODE_VALUE(node)->value);
}


static void
ms_node_value_class_init (MSNodeValueClass *klass)
{
    G_OBJECT_CLASS(klass)->finalize = ms_node_value_finalize;
    MS_NODE_CLASS(klass)->eval = ms_node_value_eval;
}


static void
ms_node_value_init (G_GNUC_UNUSED MSNodeValue *node)
{
}


MSNodeValue *
ms_node_value_new (MSValue *value)
{
    MSNodeValue *node;

    g_return_val_if_fail (value != NULL, NULL);

    node = g_object_new (MS_TYPE_NODE_VALUE, NULL);
    node->value = ms_value_ref (value);

    return node;
}


/****************************************************************************/
/* MSNodeValList
 */

G_DEFINE_TYPE(MSNodeValList, ms_node_val_list, MS_TYPE_NODE)


static void
ms_node_val_list_finalize (GObject *object)
{
    MSNodeValList *node = MS_NODE_VAL_LIST (object);

    if (node->elms)
        g_object_unref (node->elms);

    G_OBJECT_CLASS(ms_node_val_list_parent_class)->finalize (object);
}


static MSValue *
ms_node_val_list_eval (MSNode    *node_,
                       MSContext *ctx)
{
    MSValue *ret;
    guint n_elms, i;
    MSNodeValList *node = MS_NODE_VAL_LIST (node_);

    n_elms = node->elms ? node->elms->n_nodes : 0;
    ret = ms_value_list (n_elms);

    for (i = 0; i < n_elms; ++i)
    {
        MSValue *elm = ms_node_eval (node->elms->nodes[i], ctx);

        if (!elm)
        {
            ms_value_unref (ret);
            return NULL;
        }

        ms_value_list_set_elm (ret, i, elm);
        ms_value_unref (elm);
    }

    return ret;
}


static void
ms_node_val_list_class_init (MSNodeValListClass *klass)
{
    G_OBJECT_CLASS(klass)->finalize = ms_node_val_list_finalize;
    MS_NODE_CLASS(klass)->eval = ms_node_val_list_eval;
}


static void
ms_node_val_list_init (G_GNUC_UNUSED MSNodeValList *node)
{
}


MSNodeValList *
ms_node_val_list_new (MSNodeList *list)
{
    MSNodeValList *node;

    g_return_val_if_fail (!list || MS_IS_NODE_LIST (list), NULL);

    if (list && !list->n_nodes)
        list = NULL;

    node = g_object_new (MS_TYPE_NODE_VAL_LIST, NULL);
    node->elms = list ? g_object_ref (list) : NULL;

    return node;
}
