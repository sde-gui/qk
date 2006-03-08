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
#include "mooutils/moopython.h"
#include <string.h>


MSValue *
_ms_node_eval (MSNode     *node,
               MSContext  *ctx)
{
    g_return_val_if_fail (node != NULL, NULL);
    g_return_val_if_fail (MS_IS_CONTEXT (ctx), NULL);
    g_return_val_if_fail (node->eval != NULL, NULL);
    return node->eval (node, ctx);
}


MSValue *
ms_top_node_eval (MSNode     *node,
                  MSContext  *ctx)
{
    MSValue *ret;

    g_return_val_if_fail (node != NULL, NULL);
    g_return_val_if_fail (MS_IS_CONTEXT (ctx), NULL);
    g_return_val_if_fail (node->eval != NULL, NULL);

    ret = _ms_node_eval (node, ctx);

    if (ctx->return_set)
        ms_context_unset_return (ctx);
    if (ctx->break_set)
        ms_context_unset_break (ctx);
    if (ctx->continue_set)
        ms_context_unset_continue (ctx);

    return ret;
}


static MSNode *
ms_node_new (gsize         node_size,
             MSNodeType    type,
             MSNodeEval    eval,
             MSNodeDestroy destroy)
{
    MSNode *node;

    g_assert (node_size >= sizeof (MSNode));
    g_assert (type && type < MS_TYPE_NODE_LAST);

    node = g_malloc0 (node_size);
    node->ref_count = 1;
    node->type = type;
    node->eval = eval;
    node->destroy = destroy;

    return node;
}

#define NODE_NEW(Type_,type_,eval_,destroy_)                        \
    ((Type_*) ms_node_new (sizeof (Type_), type_, eval_, destroy_))


gpointer
ms_node_ref (gpointer node)
{
    if (node)
        MS_NODE(node)->ref_count++;
    return node;
}


void
ms_node_unref (gpointer node)
{
    if (node && !--MS_NODE(node)->ref_count)
    {
        if (MS_NODE(node)->destroy)
            MS_NODE(node)->destroy (node);
        g_free (node);
    }
}


/****************************************************************************/
/* MSNodeList
 */

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
        ret = _ms_node_eval (list->nodes[i], ctx);

        if (!ret)
            return NULL;

        if (ctx->return_set)
        {
            ms_value_unref (ret);
            ret = ms_value_ref (ctx->return_val);
            break;
        }

        if (ctx->break_set || ctx->continue_set)
            break;

        if (i + 1 < list->n_nodes)
            ms_value_unref (ret);
    }

    return ret;
}


static void
ms_node_list_destroy (MSNode *node)
{
    guint i;
    MSNodeList *list = MS_NODE_LIST (node);
    for (i = 0; i < list->n_nodes; ++i)
        ms_node_unref (list->nodes[i]);
    g_free (list->nodes);
}


MSNodeList *
ms_node_list_new (void)
{
    return NODE_NEW (MSNodeList,
                     MS_TYPE_NODE_LIST,
                     ms_node_list_eval,
                     ms_node_list_destroy);
}


void
ms_node_list_add (MSNodeList *list,
                  MSNode     *node)
{
    g_return_if_fail (list != NULL && node != NULL);
    g_return_if_fail (MS_NODE_TYPE (list) == MS_TYPE_NODE_LIST);

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

    list->nodes[list->n_nodes++] = ms_node_ref (node);
}


/****************************************************************************/
/* MSNodeVar
 */

static void
ms_node_var_destroy (MSNode *node)
{
    MSNodeVar *var = MS_NODE_VAR (node);
    g_free (var->name);
}


static MSValue *
ms_node_var_eval (MSNode    *node,
                  MSContext *ctx)
{
    MSNodeVar *var = MS_NODE_VAR (node);
    return ms_context_eval_variable (ctx, var->name);
}


MSNodeVar *
ms_node_var_new (const char *name)
{
    MSNodeVar *var;

    g_return_val_if_fail (name && name[0], NULL);

    var = NODE_NEW (MSNodeVar,
                    MS_TYPE_NODE_VAR,
                    ms_node_var_eval,
                    ms_node_var_destroy);

    var->name = g_strdup (name);

    return var;
}


/****************************************************************************/
/* MSNodeFunction
 */

static void
ms_node_function_destroy (MSNode *node)
{
    ms_node_unref (MS_NODE_FUNCTION(node)->func);
    ms_node_unref (MS_NODE_FUNCTION(node)->args);
}


static MSValue *
ms_node_function_eval (MSNode    *node_,
                       MSContext *ctx)
{
    guint i, n_args;
    MSValue **args;
    MSValue *ret, *func;
    MSNodeFunction *node = MS_NODE_FUNCTION (node_);

    g_return_val_if_fail (node->func != NULL, NULL);

    func = _ms_node_eval (node->func, ctx);

    if (!func)
        return NULL;

    if (!ms_value_is_func (func))
    {
        char *str = ms_value_print (func);
        ms_value_unref (func);
        ms_context_format_error (ctx, MS_ERROR_TYPE,
                                 "object <%s> is not a function",
                                 str);
        g_free (str);
        return NULL;
    }

    n_args = node->args ? node->args->n_nodes : 0;
    args = NULL;
    ret = NULL;

    if (n_args)
    {
        args = g_new0 (MSValue*, n_args);

        for (i = 0; i < n_args; ++i)
        {
            args[i] = _ms_node_eval (node->args->nodes[i], ctx);

            if (!args[i])
                goto out;
        }
    }

    ret = ms_value_call (func, args, n_args, ctx);

out:
    for (i = 0; i < n_args; ++i)
        ms_value_unref (args[i]);
    g_free (args);
    ms_value_unref (func);
    return ret;
}


MSNodeFunction *
ms_node_function_new (MSNode     *func,
                      MSNodeList *args)
{
    MSNodeFunction *node;

    g_return_val_if_fail (func != NULL, NULL);
    g_return_val_if_fail (!args || MS_IS_NODE_LIST (args), NULL);

    if (args && !args->n_nodes)
        args = NULL;

    node = NODE_NEW (MSNodeFunction,
                     MS_TYPE_NODE_FUNCTION,
                     ms_node_function_eval,
                     ms_node_function_destroy);

    node->func = ms_node_ref (func);
    node->args = ms_node_ref (args);

    return node;
}


MSNodeFunction *
ms_node_binary_op_new (MSBinaryOp  op,
                       MSNode     *a,
                       MSNode     *b)
{
    MSNodeFunction *node;
    MSNodeList *args;
    const char *name;
    MSNodeVar *func;

    g_return_val_if_fail (a && b, NULL);

    name = ms_binary_op_name (op);
    g_return_val_if_fail (name != NULL, NULL);
    func = ms_node_var_new (name);

    args = ms_node_list_new ();
    ms_node_list_add (args, a);
    ms_node_list_add (args, b);

    node = ms_node_function_new (MS_NODE (func), args);

    ms_node_unref (args);
    ms_node_unref (func);
    return node;
}


MSNodeFunction *
ms_node_unary_op_new (MSUnaryOp   op,
                      MSNode     *val)
{
    MSNodeFunction *node;
    MSNodeList *args;
    const char *name;
    MSNodeVar *func;

    g_return_val_if_fail (val != NULL, NULL);

    name = ms_unary_op_name (op);
    g_return_val_if_fail (name != NULL, NULL);
    func = ms_node_var_new (name);

    args = ms_node_list_new ();
    ms_node_list_add (args, val);

    node = ms_node_function_new (MS_NODE (func), args);

    ms_node_unref (args);
    ms_node_unref (func);
    return node;
}


/****************************************************************************/
/* MSNodeIfElse
 */

static void
ms_node_if_else_destroy (MSNode *node_)
{
    MSNodeIfElse *node = MS_NODE_IF_ELSE (node_);
    ms_node_unref (node->condition);
    ms_node_unref (node->then_);
    ms_node_unref (node->else_);
}


static MSValue *
ms_node_if_else_eval (MSNode    *node_,
                      MSContext *ctx)
{
    MSValue *ret = NULL, *cond;
    MSNodeIfElse *node = MS_NODE_IF_ELSE (node_);

    g_return_val_if_fail (node->condition != NULL, NULL);
    g_return_val_if_fail (node->then_ != NULL, NULL);

    cond = _ms_node_eval (node->condition, ctx);

    if (!cond)
        return NULL;

    if (ms_value_get_bool (cond))
        ret = _ms_node_eval (node->then_, ctx);
    else if (node->else_)
        ret = _ms_node_eval (node->else_, ctx);
    else
        ret = ms_value_none ();

    if (ctx->return_set)
    {
        ms_value_unref (ret);
        ret = ms_value_ref (ctx->return_val);
    }

    ms_value_unref (cond);
    return ret;
}


MSNodeIfElse *
ms_node_if_else_new (MSNode     *condition,
                     MSNode     *then_,
                     MSNode     *else_)
{
    MSNodeIfElse *node;

    g_return_val_if_fail (condition && then_, NULL);

    node = NODE_NEW (MSNodeIfElse,
                     MS_TYPE_NODE_IF_ELSE,
                     ms_node_if_else_eval,
                     ms_node_if_else_destroy);

    node->condition = ms_node_ref (condition);
    node->then_ = ms_node_ref (then_);
    node->else_ = else_ ? ms_node_ref (else_) : NULL;

    return node;
}


/****************************************************************************/
/* MSNodeWhile
 */

static void
ms_node_while_destroy (MSNode *node)
{
    MSNodeWhile *wh = MS_NODE_WHILE (node);
    ms_node_unref (wh->condition);
    ms_node_unref (wh->what);
}


static MSValue *
ms_loop_while (MSContext  *ctx,
               MSNode     *condition,
               MSNode     *what)
{
    MSValue *ret;

    g_return_val_if_fail (condition != NULL, NULL);

    ret = NULL;

    while (TRUE)
    {
        MSValue *cond = _ms_node_eval (condition, ctx);
        gboolean doit;

        if (!cond)
            return NULL;

        doit = ms_value_get_bool (cond);
        ms_value_unref (cond);

        if (doit)
        {
            ms_value_unref (ret);

            if (what)
                ret = _ms_node_eval (what, ctx);
            else
                ret = ms_value_none ();

            if (!ret)
                return NULL;

            if (ctx->return_set)
            {
                ms_value_unref (ret);
                ret = ms_value_ref (ctx->return_val);
                break;
            }

            if (ctx->break_set)
            {
                ms_context_unset_break (ctx);
                break;
            }

            if (ctx->continue_set)
                ms_context_unset_continue (ctx);
        }
        else
        {
            break;
        }
    }

    return ret ? ret : ms_value_none ();
}


static MSValue *
ms_loop_do_while (MSContext  *ctx,
                  MSNode     *condition,
                  MSNode     *what)
{
    MSValue *ret;

    g_return_val_if_fail (condition != NULL, NULL);

    ret = NULL;

    while (TRUE)
    {
        MSValue *cond;
        gboolean stop;

        ms_value_unref (ret);

        if (what)
            ret = _ms_node_eval (what, ctx);
        else
            ret = ms_value_none ();

        if (!ret)
            return NULL;

        if (ctx->return_set)
        {
            ms_value_unref (ret);
            ret = ms_value_ref (ctx->return_val);
            break;
        }

        if (ctx->break_set)
        {
            ms_context_unset_break (ctx);
            break;
        }

        if (ctx->continue_set)
            ms_context_unset_continue (ctx);

        cond = _ms_node_eval (condition, ctx);

        if (!cond)
            return NULL;

        stop = !ms_value_get_bool (cond);
        ms_value_unref (cond);

        if (stop)
            break;
    }

    return ret ? ret : ms_value_none ();
}


static MSValue *
ms_node_while_eval (MSNode    *node,
                    MSContext *ctx)
{
    MSNodeWhile *wh = MS_NODE_WHILE (node);

    g_return_val_if_fail (wh->condition != NULL, NULL);

    switch (wh->type)
    {
        case MS_COND_BEFORE:
            return ms_loop_while (ctx, wh->condition, wh->what);
        case MS_COND_AFTER:
            return ms_loop_do_while (ctx, wh->condition, wh->what);
    }

    g_return_val_if_reached (NULL);
}


MSNodeWhile*
ms_node_while_new (MSCondType  type,
                   MSNode     *cond,
                   MSNode     *what)
{
    MSNodeWhile *wh;

    g_return_val_if_fail (cond != NULL, NULL);

    wh = NODE_NEW (MSNodeWhile,
                   MS_TYPE_NODE_WHILE,
                   ms_node_while_eval,
                   ms_node_while_destroy);

    wh->type = type;
    wh->condition = ms_node_ref (cond);
    wh->what = what ? ms_node_ref (what) : NULL;

    return wh;
}


/****************************************************************************/
/* MSNodeFor
 */

static void
ms_node_for_destroy (MSNode *node)
{
    MSNodeFor *loop = MS_NODE_FOR (node);
    ms_node_unref (loop->variable);
    ms_node_unref (loop->list);
    ms_node_unref (loop->what);
}


static MSValue *
ms_node_for_eval (MSNode    *node,
                  MSContext *ctx)
{
    MSNodeVar *var;
    MSNodeFor *loop = MS_NODE_FOR (node);
    MSValue *vallist = NULL;
    MSValue *ret = NULL;
    guint i;

    g_return_val_if_fail (loop->variable != NULL, NULL);
    g_return_val_if_fail (loop->list != NULL, NULL);

    if (!MS_IS_NODE_VAR (loop->variable))
        return ms_context_format_error (ctx, MS_ERROR_TYPE,
                                        "illegal loop variable");

    if (!MS_IS_NODE_VAL_LIST (loop->list))
        if (!MS_IS_NODE_VALUE (loop->list) ||
             MS_VALUE_TYPE (MS_NODE_VALUE(loop->list)->value) != MS_VALUE_LIST)
            return ms_context_format_error (ctx, MS_ERROR_TYPE,
                                            "illegal loop list");

    var = MS_NODE_VAR (loop->variable);

    if (MS_IS_NODE_VAL_LIST (loop->list))
        vallist = _ms_node_eval (loop->list, ctx);
    else
        vallist = ms_value_ref (MS_NODE_VALUE(loop->list)->value);

    if (!vallist)
        return NULL;

    g_return_val_if_fail (MS_VALUE_TYPE (vallist) == MS_VALUE_LIST, NULL);

    for (i = 0; i < vallist->list.n_elms; ++i)
    {
        if (!ms_context_assign_variable (ctx, var->name, vallist->list.elms[i]))
            goto error;

        ms_value_unref (ret);

        if (loop->what)
            ret = _ms_node_eval (loop->what, ctx);
        else
            ret = ms_value_none ();

        if (!ret)
            goto error;

        if (ctx->return_set)
        {
            ms_value_unref (ret);
            ret = ms_value_ref (ctx->return_val);
            break;
        }

        if (ctx->break_set)
        {
            ms_context_unset_break (ctx);
            break;
        }

        if (ctx->continue_set)
            ms_context_unset_continue (ctx);
    }

    if (!ret)
        ret = ms_value_none ();

    ms_value_unref (vallist);
    return ret;

error:
    ms_value_unref (ret);
    ms_value_unref (vallist);
    return NULL;
}


MSNodeFor*
ms_node_for_new (MSNode     *var,
                 MSNode     *list,
                 MSNode     *what)
{
    MSNodeFor *loop;

    g_return_val_if_fail (var && list, NULL);

    loop = NODE_NEW (MSNodeFor,
                     MS_TYPE_NODE_FOR,
                     ms_node_for_eval,
                     ms_node_for_destroy);

    loop->variable = ms_node_ref (var);
    loop->list = ms_node_ref (list);
    loop->what = what ? ms_node_ref (what) : NULL;

    return loop;
}


/****************************************************************************/
/* MSNodeAssign
 */

static void
ms_node_assign_destroy (MSNode *node_)
{
    MSNodeAssign *node = MS_NODE_ASSIGN (node_);
    ms_node_unref (node->var);
    ms_node_unref (node->val);
}


static MSValue *
ms_node_assign_eval (MSNode    *node_,
                     MSContext *ctx)
{
    MSValue *value;
    MSNodeAssign *node = MS_NODE_ASSIGN (node_);

    g_return_val_if_fail (node->var != NULL, NULL);
    g_return_val_if_fail (node->val != NULL, NULL);

    value = _ms_node_eval (node->val, ctx);

    if (!value)
        return NULL;

    if (!ms_context_assign_variable (ctx, node->var->name, value))
    {
        ms_value_unref (value);
        return NULL;
    }

    return value;
}


MSNodeAssign *
ms_node_assign_new (MSNodeVar  *var,
                    MSNode     *val)
{
    MSNodeAssign *node;

    g_return_val_if_fail (MS_IS_NODE_VAR (var), NULL);
    g_return_val_if_fail (val != NULL, NULL);

    node = NODE_NEW (MSNodeAssign,
                     MS_TYPE_NODE_ASSIGN,
                     ms_node_assign_eval,
                     ms_node_assign_destroy);

    node->var = ms_node_ref (var);
    node->val = ms_node_ref (val);

    return node;
}


/****************************************************************************/
/* MSNodeValue
 */

static void
ms_node_value_destroy (MSNode *node)
{
    MSNodeValue *val = MS_NODE_VALUE (node);
    ms_value_unref (val->value);
}


static MSValue *
ms_node_value_eval (MSNode    *node,
                    G_GNUC_UNUSED MSContext *ctx)
{
    return ms_value_ref (MS_NODE_VALUE(node)->value);
}


MSNodeValue *
ms_node_value_new (MSValue *value)
{
    MSNodeValue *node;

    g_return_val_if_fail (value != NULL, NULL);

    node = NODE_NEW (MSNodeValue,
                     MS_TYPE_NODE_VALUE,
                     ms_node_value_eval,
                     ms_node_value_destroy);

    node->value = ms_value_ref (value);

    return node;
}


/****************************************************************************/
/* MSNodeValList
 */

static void
ms_node_val_list_destroy (MSNode *node_)
{
    MSNodeValList *node = MS_NODE_VAL_LIST (node_);
    ms_node_unref (node->elms);
    ms_node_unref (node->first);
    ms_node_unref (node->last);
}


static MSValue *
ms_node_val_range_eval (MSNodeValList *node,
                        MSContext     *ctx)
{
    MSValue *ret;
    guint n_elms, i;
    MSValue *vfirst, *vlast;
    int first, last;

    g_assert (node->type == MS_VAL_RANGE);

    vfirst = _ms_node_eval (node->first, ctx);

    if (!vfirst)
        return NULL;

    vlast = _ms_node_eval (node->last, ctx);

    if (!vlast)
    {
        ms_value_unref (vfirst);
        return NULL;
    }

    if (!ms_value_get_int (vfirst, &first) || !ms_value_get_int (vlast, &last))
    {
        ms_value_unref (vfirst);
        ms_value_unref (vlast);
        return ms_context_format_error (ctx, MS_ERROR_TYPE,
                                        "illegal list bounds");
    }

    ms_value_unref (vfirst);
    ms_value_unref (vlast);

    if (first > last)
        return ms_value_list (0);

    n_elms = last - first + 1;
    ret = ms_value_list (n_elms);

    for (i = 0; i < n_elms; ++i)
    {
        MSValue *val = ms_value_int (first + i);
        ms_value_list_set_elm (ret, i, val);
        ms_value_unref (val);
    }

    return ret;
}


static MSValue *
ms_node_val_list_eval (MSNode    *node_,
                       MSContext *ctx)
{
    MSValue *ret;
    guint n_elms, i;
    MSNodeValList *node = MS_NODE_VAL_LIST (node_);

    switch (node->type)
    {
        case MS_VAL_LIST:
            n_elms = node->elms ? node->elms->n_nodes : 0;
            ret = ms_value_list (n_elms);

            for (i = 0; i < n_elms; ++i)
            {
                MSValue *elm = _ms_node_eval (node->elms->nodes[i], ctx);

                if (!elm)
                {
                    ms_value_unref (ret);
                    return NULL;
                }

                ms_value_list_set_elm (ret, i, elm);
                ms_value_unref (elm);
            }

            return ret;

        case MS_VAL_RANGE:
            return ms_node_val_range_eval (node, ctx);
    }

    g_return_val_if_reached (NULL);
}


static MSNodeValList *
ms_node_val_list_new_ (void)
{
    return NODE_NEW (MSNodeValList,
                     MS_TYPE_NODE_VAL_LIST,
                     ms_node_val_list_eval,
                     ms_node_val_list_destroy);
}


MSNodeValList *
ms_node_val_list_new (MSNodeList *list)
{
    MSNodeValList *node;

    g_return_val_if_fail (!list || MS_IS_NODE_LIST (list), NULL);

    if (list && !list->n_nodes)
        list = NULL;

    node = ms_node_val_list_new_ ();
    node->type = MS_VAL_LIST;
    node->elms = list ? ms_node_ref (list) : NULL;

    return node;
}


MSNodeValList *
ms_node_val_range_new (MSNode *first,
                       MSNode *last)
{
    MSNodeValList *node;

    g_return_val_if_fail (first && last, NULL);

    node = ms_node_val_list_new_ ();
    node->type = MS_VAL_RANGE;
    node->first = ms_node_ref (first);
    node->last = ms_node_ref (last);

    return node;
}


/****************************************************************************/
/* MSNodePython
 */

static void
ms_node_python_destroy (MSNode *node)
{
    g_free (MS_NODE_PYTHON(node)->script);
}


static MSValue *
ms_node_python_eval (MSNode    *node_,
                     MSContext *ctx)
{
    MSNodePython *node = MS_NODE_PYTHON (node_);
    MooPyObject *ret;

    if (!moo_python_running())
        return ms_context_format_error (ctx, MS_ERROR_RUNTIME,
                                        "Python support not available");

    ret = moo_python_run_string (node->script);

    if (ret)
    {
        moo_Py_DECREF (ret);
        return ms_value_none ();
    }
    else
    {
        moo_PyErr_Print ();
        return ms_context_format_error (ctx, MS_ERROR_RUNTIME,
                                        "python script raised exception");
    }
}


MSNodePython *
ms_node_python_new (const char *script)
{
    MSNodePython *node;

    g_return_val_if_fail (script != NULL, NULL);

    node = NODE_NEW (MSNodePython,
                     MS_TYPE_NODE_PYTHON,
                     ms_node_python_eval,
                     ms_node_python_destroy);

    node->script = g_strdup (script);

    return node;
}


/****************************************************************************/
/* MSNodeListElm
 */

static void
ms_node_list_elm_destroy (MSNode *node)
{
    ms_node_unref (MS_NODE_LIST_ELM(node)->list);
    ms_node_unref (MS_NODE_LIST_ELM(node)->ind);
}


static MSValue *
ms_node_list_elm_eval (MSNode    *node_,
                       MSContext *ctx)
{
    MSNodeListElm *node = MS_NODE_LIST_ELM (node_);
    MSValue *list = NULL, *ind = NULL, *ret;
    guint len;
    int index;

    list = _ms_node_eval (node->list, ctx);

    if (!list)
        goto error;

    ind = _ms_node_eval (node->ind, ctx);

    if (!ind)
        goto error;

    if (!ms_value_get_int (ind, &index))
    {
        ms_context_format_error (ctx, MS_ERROR_VALUE,
                                 "invalid list index");
        goto error;
    }

    if (index < 0)
    {
        ms_context_format_error (ctx, MS_ERROR_VALUE,
                                 "index out of range");
        goto error;
    }

    switch (MS_VALUE_TYPE (list))
    {
        case MS_VALUE_STRING:
            len = g_utf8_strlen (list->str, -1);
            break;

        case MS_VALUE_LIST:
            len = list->list.n_elms;
            break;

        default:
            ms_context_format_error (ctx, MS_ERROR_VALUE,
                                     "invalid subscript");
            goto error;
    }

    if ((guint) index >= len)
    {
        ms_context_format_error (ctx, MS_ERROR_VALUE,
                                 "index out of range");
        goto error;
    }

    switch (MS_VALUE_TYPE (list))
    {
        case MS_VALUE_STRING:
            ret = ms_value_string_len (g_utf8_offset_to_pointer (list->str, index), 1);
            break;

        case MS_VALUE_LIST:
            ret = ms_value_ref (list->list.elms[index]);
            break;

        default:
            g_assert_not_reached ();
    }

    ms_value_unref (list);
    ms_value_unref (ind);
    return ret;

error:
    ms_value_unref (list);
    ms_value_unref (ind);
    return NULL;
}


MSNodeListElm *
ms_node_list_elm_new (MSNode     *list,
                      MSNode     *ind)
{
    MSNodeListElm *node;

    g_return_val_if_fail (list && ind, NULL);

    node = NODE_NEW (MSNodeListElm,
                     MS_TYPE_NODE_LIST_ELM,
                     ms_node_list_elm_eval,
                     ms_node_list_elm_destroy);

    node->list = ms_node_ref (list);
    node->ind = ms_node_ref (ind);

    return node;
}


/****************************************************************************/
/* MSNodeListAssign
 */

static void
ms_node_list_assign_destroy (MSNode *node)
{
    ms_node_unref (MS_NODE_LIST_ASSIGN(node)->list);
    ms_node_unref (MS_NODE_LIST_ASSIGN(node)->ind);
    ms_node_unref (MS_NODE_LIST_ASSIGN(node)->val);
}


static MSValue *
ms_node_list_assign_eval (MSNode    *node_,
                          MSContext *ctx)
{
    MSNodeListAssign *node = MS_NODE_LIST_ASSIGN (node_);
    MSValue *list = NULL, *ind = NULL, *val = NULL;
    guint len;
    int index;

    list = _ms_node_eval (node->list, ctx);

    if (!list)
        goto error;

    ind = _ms_node_eval (node->ind, ctx);

    if (!ind)
        goto error;

    if (!ms_value_get_int (ind, &index))
    {
        ms_context_format_error (ctx, MS_ERROR_VALUE,
                                 "invalid list index");
        goto error;
    }

    if (index < 0)
    {
        ms_context_format_error (ctx, MS_ERROR_VALUE,
                                 "index out of range");
        goto error;
    }

    switch (MS_VALUE_TYPE (list))
    {
        case MS_VALUE_LIST:
            len = list->list.n_elms;
            break;

        default:
            ms_context_format_error (ctx, MS_ERROR_VALUE,
                                     "invalid list assignment");
            goto error;
    }

    if ((guint) index >= len)
    {
        ms_context_format_error (ctx, MS_ERROR_VALUE,
                                 "index out of range");
        goto error;
    }

    val = _ms_node_eval (node->val, ctx);

    if (!val)
        goto error;

    switch (MS_VALUE_TYPE (list))
    {
        case MS_VALUE_LIST:
            ms_value_list_set_elm (list, index, val);
            break;

        default:
            g_assert_not_reached ();
    }

    ms_value_unref (list);
    ms_value_unref (ind);
    return val;

error:
    ms_value_unref (list);
    ms_node_unref (ind);
    ms_value_unref (val);
    return NULL;
}


MSNodeListAssign *
ms_node_list_assign_new (MSNode     *list,
                         MSNode     *ind,
                         MSNode     *val)
{
    MSNodeListAssign *node;

    g_return_val_if_fail (list && ind && val, NULL);

    node = NODE_NEW (MSNodeListAssign,
                     MS_TYPE_NODE_LIST_ASSIGN,
                     ms_node_list_assign_eval,
                     ms_node_list_assign_destroy);

    node->list = ms_node_ref (list);
    node->ind = ms_node_ref (ind);
    node->val = ms_node_ref (val);

    return node;
}


/****************************************************************************/
/* MSNodeReturn
 */

static void
ms_node_return_destroy (MSNode *node)
{
    ms_node_unref (MS_NODE_RETURN(node)->val);
}


static MSValue *
ms_node_return_eval (MSNode    *node_,
                     MSContext *ctx)
{
    MSNodeReturn *node = MS_NODE_RETURN (node_);
    MSValue *ret;

    if (node->val)
    {
        ret = _ms_node_eval (node->val, ctx);

        if (!ret)
            return NULL;
    }
    else
    {
        ret = ms_value_none ();
    }

    ms_context_set_return (ctx, ret);

    return ret;
}


MSNodeReturn *
ms_node_return_new (MSNode *val)
{
    MSNodeReturn *node;

    node = NODE_NEW (MSNodeReturn,
                     MS_TYPE_NODE_RETURN,
                     ms_node_return_eval,
                     ms_node_return_destroy);

    node->val = ms_node_ref (val);

    return node;
}


/****************************************************************************/
/* MSNodeBreak
 */

static MSValue *
ms_node_break_eval (MSNode    *node_,
                    MSContext *ctx)
{
    MSNodeBreak *node = MS_NODE_BREAK (node_);

    switch (node->type)
    {
        case MS_BREAK_BREAK:
            ms_context_set_break (ctx);
            break;
        case MS_BREAK_CONTINUE:
            ms_context_set_continue (ctx);
            break;
        default:
            g_assert_not_reached ();
    }

    return ms_value_none ();
}


MSNodeBreak *
ms_node_break_new (MSBreakType type)
{
    MSNodeBreak *node;

    node = NODE_NEW (MSNodeBreak,
                     MS_TYPE_NODE_BREAK,
                     ms_node_break_eval,
                     NULL);

    node->type = type;

    return node;
}


/****************************************************************************/
/* MSNodeDictElm
 */

static void
ms_node_dict_elm_destroy (MSNode *node_)
{
    ms_node_unref (MS_NODE_DICT_ELM(node_)->dict);
    g_free (MS_NODE_DICT_ELM(node_)->key);
}


static MSValue *
ms_node_dict_elm_eval (MSNode    *node_,
                       MSContext *ctx)
{
    MSValue *obj, *val = NULL;
    MSNodeDictElm *node = MS_NODE_DICT_ELM (node_);

    obj = _ms_node_eval (node->dict, ctx);

    if (!obj)
        return NULL;

    if (MS_VALUE_TYPE (obj) == MS_VALUE_DICT)
        val = ms_value_dict_get_elm (obj, node->key);

    if (!val)
        val = ms_value_get_method (obj, node->key);

    ms_value_unref (obj);

    if (!val)
        return ms_context_format_error (ctx, MS_ERROR_VALUE,
                                        "no key '%s'", node->key);
    else
        return val;
}


MSNodeDictElm *
ms_node_dict_elm_new (MSNode     *dict,
                      const char *key)
{
    MSNodeDictElm *node;

    g_return_val_if_fail (dict != NULL, NULL);
    g_return_val_if_fail (key != NULL, NULL);

    node = NODE_NEW (MSNodeDictElm,
                     MS_TYPE_NODE_DICT_ELM,
                     ms_node_dict_elm_eval,
                     ms_node_dict_elm_destroy);

    node->dict = ms_node_ref (dict);
    node->key = g_strdup (key);

    return node;
}


/****************************************************************************/
/* MSNodeDictAssign
 */

static void
ms_node_dict_assign_destroy (MSNode *node_)
{
    ms_node_unref (MS_NODE_DICT_ASSIGN(node_)->dict);
    g_free (MS_NODE_DICT_ASSIGN(node_)->key);
    ms_node_unref (MS_NODE_DICT_ASSIGN(node_)->val);
}


static MSValue *
ms_node_dict_assign_eval (MSNode    *node_,
                          MSContext *ctx)
{
    MSValue *obj = NULL, *val = NULL;
    MSNodeDictAssign *node = MS_NODE_DICT_ASSIGN (node_);

    obj = _ms_node_eval (node->dict, ctx);

    if (!obj)
        goto error;

    val = _ms_node_eval (node->val, ctx);

    if (!val)
        goto error;

    if (MS_VALUE_TYPE (obj) != MS_VALUE_DICT)
    {
        ms_context_format_error (ctx, MS_ERROR_TYPE,
                                 "not a dict object");
        goto error;
    }

    ms_value_dict_set_elm (obj, node->key, val);

    ms_value_unref (obj);
    return val;

error:
    ms_value_unref (obj);
    ms_value_unref (val);
    return NULL;
}


MSNodeDictAssign *
ms_node_dict_assign_new (MSNode     *dict,
                         const char *key,
                         MSNode     *val)
{
    MSNodeDictAssign *node;

    g_return_val_if_fail (dict != NULL, NULL);
    g_return_val_if_fail (key != NULL, NULL);
    g_return_val_if_fail (val != NULL, NULL);

    node = NODE_NEW (MSNodeDictAssign,
                     MS_TYPE_NODE_DICT_ASSIGN,
                     ms_node_dict_assign_eval,
                     ms_node_dict_assign_destroy);

    node->dict = ms_node_ref (dict);
    node->key = g_strdup (key);
    node->val = ms_node_ref (val);

    return node;
}


/****************************************************************************/
/* MSNodeDictEntry
 */

static void
ms_node_dict_entry_destroy (MSNode *node_)
{
    ms_node_unref (MS_NODE_DICT_ENTRY(node_)->val);
    g_free (MS_NODE_DICT_ENTRY(node_)->key);
}


MSNodeDictEntry *
ms_node_dict_entry_new (const char *key,
                        MSNode     *val)
{
    MSNodeDictEntry *node;

    g_return_val_if_fail (key != NULL, NULL);
    g_return_val_if_fail (val != NULL, NULL);

    node = NODE_NEW (MSNodeDictEntry,
                     MS_TYPE_NODE_DICT_ENTRY,
                     NULL,
                     ms_node_dict_entry_destroy);

    node->key = g_strdup (key);
    node->val = ms_node_ref (val);

    return node;
}


/****************************************************************************/
/* MSNodeDict
 */

static void
ms_node_dict_destroy (MSNode *node_)
{
    ms_node_unref (MS_NODE_DICT(node_)->entries);
}


static MSValue *
ms_node_dict_eval (MSNode     *node_,
                   MSContext  *ctx)
{
    MSValue *ret;
    MSNodeDict *node = MS_NODE_DICT (node_);

    ret = ms_value_dict ();

    if (node->entries)
    {
        guint i;

        for (i = 0; i < node->entries->n_nodes; ++i)
        {
            MSNodeDictEntry *entry = MS_NODE_DICT_ENTRY (node->entries->nodes[i]);
            MSValue *val = _ms_node_eval (entry->val, ctx);

            if (!val)
            {
                ms_value_unref (val);
                return NULL;
            }

            ms_value_dict_set_elm (ret, entry->key, val);
            ms_value_unref (val);
        }
    }

    return ret;
}


MSNodeDict *
ms_node_dict_new (MSNodeList *entries)
{
    MSNodeDict *node;

    g_return_val_if_fail (!entries || MS_IS_NODE_LIST (entries), NULL);

    node = NODE_NEW (MSNodeDict,
                     MS_TYPE_NODE_DICT,
                     ms_node_dict_eval,
                     ms_node_dict_destroy);

    node->entries = ms_node_ref (entries);

    return node;
}
