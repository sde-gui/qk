/*
 *   mooscript-node.c
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

#include "mooscript-node-private.h"
#include "mooscript-func-private.h"
#include "mooscript-context-private.h"
#include <string.h>


static gpointer
node_ref (gpointer node)
{
    return ms_node_ref (node ? MS_NODE (node) : NULL);
}

static void
node_unref (gpointer node)
{
    ms_node_unref (node ? MS_NODE (node) : NULL);
}


static MSValue *
_ms_node_eval (MSNode     *node,
               MSContext  *ctx)
{
    g_return_val_if_fail (node != NULL, NULL);
    g_return_val_if_fail (ctx != nil, NULL);
    g_return_val_if_fail (node->eval != NULL, NULL);
    return node->eval (node, ctx);
}


MSValue *
ms_top_node_eval (MSNode     *node,
                  MSContext  *ctx)
{
    MSValue *ret;

    g_return_val_if_fail (node != NULL, NULL);
    g_return_val_if_fail (ctx != nil, NULL);
    g_return_val_if_fail (node->eval != NULL, NULL);

    ret = _ms_node_eval (node, ctx);

    if ([ctx returnSet])
        [ctx unsetReturn];
    if ([ctx breakSet])
        [ctx unsetBreak];
    if ([ctx continueSet])
        [ctx unsetContinue];

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


MSNode *
ms_node_ref (MSNode *node)
{
    if (node)
        node->ref_count++;
    return node;
}


void
ms_node_unref (MSNode *node)
{
    if (node && !--node->ref_count)
    {
        if (node->destroy)
            node->destroy (node);
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

        if ([ctx returnSet])
        {
            ms_value_unref (ret);
            ret = [ctx getReturn];
            break;
        }

        if ([ctx breakSet] || [ctx continueSet])
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
        node_unref (list->nodes[i]);
    g_free (list->nodes);
}


MSNodeList *
_ms_node_list_new (void)
{
    return NODE_NEW (MSNodeList,
                     MS_TYPE_NODE_LIST,
                     ms_node_list_eval,
                     ms_node_list_destroy);
}


void
_ms_node_list_add (MSNodeList *list,
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

    list->nodes[list->n_nodes++] = node_ref (node);
}


/****************************************************************************/
/* MSNodeEnvVar
 */

static void
ms_node_env_var_destroy (MSNode *node)
{
    MSNodeEnvVar *var = MS_NODE_ENV_VAR (node);
    node_unref (var->name);
}


static MSValue *
ms_node_env_var_eval (MSNode    *node,
                      MSContext *ctx)
{
    MSValue *name, *ret;
    MSNodeEnvVar *var = MS_NODE_ENV_VAR (node);

    name = _ms_node_eval (var->name, ctx);

    if (!name)
        return NULL;

    if (MS_VALUE_TYPE (name) != MS_VALUE_STRING || !name->u.str)
    {
        [ctx formatError:MS_ERROR_TYPE
                        :"in $(%v): variable name must be a string",
                         name];
        ms_value_unref (name);
        return NULL;
    }

    ret = [ctx getEnvVariable:name->u.str];
    ms_value_unref (name);

    if (ret || [ctx errorSet])
        return ret;

    return var->dflt ? _ms_node_eval (var->dflt, ctx) : ms_value_none ();
}


MSNodeEnvVar *
_ms_node_env_var_new (MSNode *name,
                      MSNode *dflt)
{
    MSNodeEnvVar *var;

    g_return_val_if_fail (name != NULL, NULL);

    var = NODE_NEW (MSNodeEnvVar,
                    MS_TYPE_NODE_ENV_VAR,
                    ms_node_env_var_eval,
                    ms_node_env_var_destroy);

    var->name = node_ref (name);
    var->dflt = dflt ? node_ref (dflt) : NULL;

    return var;
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
    return [ctx evalVariable:var->name];
}


MSNodeVar *
_ms_node_var_new (const char *name)
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
    node_unref (MS_NODE_FUNCTION(node)->func);
    node_unref (MS_NODE_FUNCTION(node)->args);
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

    if (!_ms_value_is_func (func))
    {
        [ctx formatError:MS_ERROR_TYPE
                        :"object <%r> is not a function",
                         func];
        ms_value_unref (func);
        return NULL;
    }

    n_args = node->args ? node->args->n_nodes : 0;
    args = NULL;
    ret = NULL;

    if (n_args)
    {
#if GLIB_CHECK_VERSION(2,10,0)
        args = g_slice_alloc (n_args * sizeof (MSValue*));
#else
        args = g_new0 (MSValue*, n_args);
#endif

        for (i = 0; i < n_args; ++i)
        {
            args[i] = _ms_node_eval (node->args->nodes[i], ctx);

            if (!args[i])
                goto out;
        }
    }

    ret = _ms_value_call (func, args, n_args, ctx);

out:
    for (i = 0; i < n_args; ++i)
        ms_value_unref (args[i]);
#if GLIB_CHECK_VERSION(2,10,0)
    g_slice_free1 (n_args * sizeof (MSValue*), args);
#else
    g_free (args);
#endif
    ms_value_unref (func);
    return ret;
}


MSNodeFunction *
_ms_node_function_new (MSNode     *func,
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

    node->func = node_ref (func);
    node->args = node_ref (args);

    return node;
}


MSNodeFunction *
_ms_node_binary_op_new (MSBinaryOp  op,
                        MSNode     *a,
                        MSNode     *b)
{
    MSNodeFunction *node;
    MSNodeList *args;
    const char *name;
    MSNodeVar *func;

    g_return_val_if_fail (a && b, NULL);

    name = _ms_binary_op_name (op);
    g_return_val_if_fail (name != NULL, NULL);
    func = _ms_node_var_new (name);

    args = _ms_node_list_new ();
    _ms_node_list_add (args, a);
    _ms_node_list_add (args, b);

    node = _ms_node_function_new (MS_NODE (func), args);

    node_unref (args);
    node_unref (func);
    return node;
}


MSNodeFunction *
_ms_node_unary_op_new (MSUnaryOp   op,
                       MSNode     *val)
{
    MSNodeFunction *node;
    MSNodeList *args;
    const char *name;
    MSNodeVar *func;

    g_return_val_if_fail (val != NULL, NULL);

    name = _ms_unary_op_name (op);
    g_return_val_if_fail (name != NULL, NULL);
    func = _ms_node_var_new (name);

    args = _ms_node_list_new ();
    _ms_node_list_add (args, val);

    node = _ms_node_function_new (MS_NODE (func), args);

    node_unref (args);
    node_unref (func);
    return node;
}


/****************************************************************************/
/* MSNodeIfElse
 */

static void
ms_node_if_else_destroy (MSNode *node_)
{
    MSNodeIfElse *node = MS_NODE_IF_ELSE (node_);
    node_unref (node->list);
}


static MSValue *
ms_node_if_else_eval (MSNode    *node_,
                      MSContext *ctx)
{
    MSValue *ret = NULL;
    MSNodeIfElse *node = MS_NODE_IF_ELSE (node_);
    MSNode *node_action = NULL;
    guint i = 0;

    g_return_val_if_fail (node->list != NULL, NULL);

    while (!node_action && i < node->list->n_nodes)
    {
        MSNode *node_cond = node->list->nodes[i++];
        MSNode *node_eval = node->list->nodes[i++];

        if (node_cond)
        {
            MSValue *cond = _ms_node_eval (node_cond, ctx);

            if (!cond)
                return NULL;

            if (ms_value_get_bool (cond))
                node_action = node_eval;

            ms_value_unref (cond);
        }
        else
        {
            node_action = node_eval;
        }
    }

    if (!node_action)
        return ms_value_none ();

    ret = _ms_node_eval (node_action, ctx);

    if ([ctx returnSet])
    {
        ms_value_unref (ret);
        ret = [ctx getReturn];
    }

    return ret;
}


MSNodeIfElse *
_ms_node_if_else_new (MSNode     *condition,
                      MSNode     *then_,
                      MSNodeList *elif_,
                      MSNode     *else_)
{
    MSNodeIfElse *node;
    MSNodeList *list;
    guint i;

    g_return_val_if_fail (condition && then_, NULL);

    list = _ms_node_list_new ();

    _ms_node_list_add (list, condition);
    _ms_node_list_add (list, then_);

    if (elif_)
    {
        for (i = 0; i < elif_->n_nodes; ++i)
        {
            g_return_val_if_fail (MS_IS_NODE_LIST (elif_->nodes[i]), NULL);
            g_return_val_if_fail (MS_NODE_LIST(elif_->nodes[i])->n_nodes == 2, NULL);
            _ms_node_list_add (list, MS_NODE_LIST(elif_->nodes[i])->nodes[0]);
            _ms_node_list_add (list, MS_NODE_LIST(elif_->nodes[i])->nodes[1]);
        }
    }

    if (else_)
    {
        MSValue *v = ms_value_true ();
        MSNode *n = MS_NODE (_ms_node_value_new (v));
        _ms_node_list_add (list, n);
        _ms_node_list_add (list, else_);
        node_unref (n);
        ms_value_unref (v);
    }

    node = NODE_NEW (MSNodeIfElse,
                     MS_TYPE_NODE_IF_ELSE,
                     ms_node_if_else_eval,
                     ms_node_if_else_destroy);

    node->list = list;

    return node;
}


/****************************************************************************/
/* MSNodeWhile
 */

static void
ms_node_while_destroy (MSNode *node)
{
    MSNodeWhile *wh = MS_NODE_WHILE (node);
    node_unref (wh->condition);
    node_unref (wh->what);
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

            if ([ctx returnSet])
            {
                ms_value_unref (ret);
                ret = [ctx getReturn];
                break;
            }

            if ([ctx breakSet])
            {
                [ctx unsetBreak];
                break;
            }

            if ([ctx continueSet])
                [ctx unsetContinue];
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

        if ([ctx returnSet])
        {
            ms_value_unref (ret);
            ret = [ctx getReturn];
            break;
        }

        if ([ctx breakSet])
        {
            [ctx unsetBreak];
            break;
        }

        if ([ctx continueSet])
            [ctx unsetContinue];

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
_ms_node_while_new (MSCondType  type,
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
    wh->condition = node_ref (cond);
    wh->what = what ? node_ref (what) : NULL;

    return wh;
}


/****************************************************************************/
/* MSNodeFor
 */

static void
ms_node_for_destroy (MSNode *node)
{
    MSNodeFor *loop = MS_NODE_FOR (node);
    node_unref (loop->variable);
    node_unref (loop->list);
    node_unref (loop->what);
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
        return [ctx formatError:MS_ERROR_TYPE
                               :"illegal loop variable <%r>",
                                loop->variable];

    vallist = _ms_node_eval (loop->list, ctx);

    if (!vallist)
        return NULL;

    if (MS_VALUE_TYPE (vallist) != MS_VALUE_LIST)
        return [ctx formatError:MS_ERROR_TYPE
                               :"illegal loop list <%r>",
                                vallist];

    var = MS_NODE_VAR (loop->variable);

    for (i = 0; i < vallist->u.list.n_elms; ++i)
    {
        if (![ctx assignVariable:var->name :vallist->u.list.elms[i]])
            goto error;

        ms_value_unref (ret);

        if (loop->what)
            ret = _ms_node_eval (loop->what, ctx);
        else
            ret = ms_value_none ();

        if (!ret)
            goto error;

        if ([ctx returnSet])
        {
            ms_value_unref (ret);
            ret = [ctx getReturn];
            break;
        }

        if ([ctx breakSet])
        {
            [ctx unsetBreak];
            break;
        }

        if ([ctx continueSet])
            [ctx unsetContinue];
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
_ms_node_for_new (MSNode     *var,
                  MSNode     *list,
                  MSNode     *what)
{
    MSNodeFor *loop;

    g_return_val_if_fail (var && list, NULL);

    loop = NODE_NEW (MSNodeFor,
                     MS_TYPE_NODE_FOR,
                     ms_node_for_eval,
                     ms_node_for_destroy);

    loop->variable = node_ref (var);
    loop->list = node_ref (list);
    loop->what = what ? node_ref (what) : NULL;

    return loop;
}


/****************************************************************************/
/* MSNodeAssign
 */

static void
ms_node_assign_destroy (MSNode *node_)
{
    MSNodeAssign *node = MS_NODE_ASSIGN (node_);
    node_unref (node->var);
    node_unref (node->val);
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

    if (![ctx assignVariable:node->var->name :value])
    {
        ms_value_unref (value);
        return NULL;
    }

    return value;
}


MSNodeAssign *
_ms_node_assign_new (MSNodeVar  *var,
                     MSNode     *val)
{
    MSNodeAssign *node;

    g_return_val_if_fail (MS_IS_NODE_VAR (var), NULL);
    g_return_val_if_fail (val != NULL, NULL);

    node = NODE_NEW (MSNodeAssign,
                     MS_TYPE_NODE_ASSIGN,
                     ms_node_assign_eval,
                     ms_node_assign_destroy);

    node->var = node_ref (var);
    node->val = node_ref (val);

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
_ms_node_value_new (MSValue *value)
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
    node_unref (node->elms);
    node_unref (node->first);
    node_unref (node->last);
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
        [ctx formatError:MS_ERROR_TYPE
                        :"illegal list bounds <%r> and <%r>",
                         vfirst, vlast];
        ms_value_unref (vfirst);
        ms_value_unref (vlast);
        return NULL;
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
_ms_node_val_list_new (MSNodeList *list)
{
    MSNodeValList *node;

    g_return_val_if_fail (!list || MS_IS_NODE_LIST (list), NULL);

    if (list && !list->n_nodes)
        list = NULL;

    node = ms_node_val_list_new_ ();
    node->type = MS_VAL_LIST;
    node->elms = list ? node_ref (list) : NULL;

    return node;
}


MSNodeValList *
_ms_node_val_range_new (MSNode *first,
                        MSNode *last)
{
    MSNodeValList *node;

    g_return_val_if_fail (first && last, NULL);

    node = ms_node_val_list_new_ ();
    node->type = MS_VAL_RANGE;
    node->first = node_ref (first);
    node->last = node_ref (last);

    return node;
}


/****************************************************************************/
/* MSNodeGetItem
 */

static void
ms_node_get_item_destroy (MSNode *node)
{
    node_unref (MS_NODE_GET_ITEM(node)->obj);
    node_unref (MS_NODE_GET_ITEM(node)->key);
}


static MSValue *
ms_node_get_item_eval (MSNode    *node_,
                       MSContext *ctx)
{
    MSNodeGetItem *node = MS_NODE_GET_ITEM (node_);
    MSValue *obj = NULL, *key = NULL, *val = NULL;
    guint len;
    int index;

    obj = _ms_node_eval (node->obj, ctx);

    if (!obj)
        goto error;

    key = _ms_node_eval (node->key, ctx);

    if (!key)
        goto error;

    if (MS_VALUE_TYPE (obj) == MS_VALUE_DICT)
    {
        if (MS_VALUE_TYPE (key) == MS_VALUE_STRING)
        {
            val = ms_value_dict_get_elm (obj, key->u.str);

            if (!val)
            {
                [ctx formatError:MS_ERROR_VALUE
                                :"no key '%s'",
                                 key->u.str];
                goto error;
            }

            goto out;
        }
        else
        {
            [ctx formatError:MS_ERROR_VALUE
                            :"invalid dict key <%r>",
                             key];
            goto error;
        }
    }

    if (!ms_value_get_int (key, &index))
    {
        [ctx formatError:MS_ERROR_VALUE
                        :"invalid list index <%r>",
                         key];
        goto error;
    }

    if (index < 0)
    {
        [ctx formatError:MS_ERROR_VALUE
                        :"index out of range"];
        goto error;
    }

    switch (MS_VALUE_TYPE (obj))
    {
        case MS_VALUE_STRING:
            len = g_utf8_strlen (obj->u.str, -1);
            break;

        case MS_VALUE_LIST:
            len = obj->u.list.n_elms;
            break;

        default:
            [ctx formatError:MS_ERROR_VALUE
                            :"<%r> is not subscriptable",
                             obj];
            goto error;
    }

    if ((guint) index >= len)
    {
        [ctx formatError:MS_ERROR_VALUE
                        :"index out of range"];
        goto error;
    }

    switch (MS_VALUE_TYPE (obj))
    {
        case MS_VALUE_STRING:
            val = ms_value_string_len (g_utf8_offset_to_pointer (obj->u.str, index), 1);
            break;

        case MS_VALUE_LIST:
            val = ms_value_ref (obj->u.list.elms[index]);
            break;

        default:
            g_assert_not_reached ();
    }

out:
    ms_value_unref (obj);
    ms_value_unref (key);
    return val;

error:
    ms_value_unref (obj);
    ms_value_unref (key);
    return NULL;
}


MSNodeGetItem *
_ms_node_get_item_new (MSNode     *obj,
                       MSNode     *key)
{
    MSNodeGetItem *node;

    g_return_val_if_fail (obj && key, NULL);

    node = NODE_NEW (MSNodeGetItem,
                     MS_TYPE_NODE_GET_ITEM,
                     ms_node_get_item_eval,
                     ms_node_get_item_destroy);

    node->obj = node_ref (obj);
    node->key = node_ref (key);

    return node;
}


/****************************************************************************/
/* MSNodeSetItem
 */

static void
ms_node_set_item_destroy (MSNode *node)
{
    node_unref (MS_NODE_SET_ITEM(node)->obj);
    node_unref (MS_NODE_SET_ITEM(node)->key);
    node_unref (MS_NODE_SET_ITEM(node)->val);
}


static MSValue *
ms_node_set_item_eval (MSNode    *node_,
                       MSContext *ctx)
{
    MSNodeSetItem *node = MS_NODE_SET_ITEM (node_);
    MSValue *obj = NULL, *key = NULL, *val = NULL;
    guint len;
    int index;

    obj = _ms_node_eval (node->obj, ctx);

    if (!obj)
        goto error;

    key = _ms_node_eval (node->key, ctx);

    if (!key)
        goto error;

    val = _ms_node_eval (node->val, ctx);

    if (!val)
        goto error;

    if (MS_VALUE_TYPE (obj) == MS_VALUE_DICT)
    {
        if (MS_VALUE_TYPE (key) == MS_VALUE_STRING)
        {
            ms_value_dict_set_elm (obj, key->u.str, val);
            goto out;
        }
        else
        {
            [ctx formatError:MS_ERROR_VALUE
                            :"invalid dict key <%r>",
                             key];
            goto error;
        }
    }

    if (!ms_value_get_int (key, &index))
    {
        [ctx formatError:MS_ERROR_VALUE
                        :"invalid list index <%r>",
                         key];
        goto error;
    }

    if (index < 0)
    {
        [ctx formatError:MS_ERROR_VALUE
                        :"index out of range"];
        goto error;
    }

    switch (MS_VALUE_TYPE (obj))
    {
        case MS_VALUE_LIST:
            len = obj->u.list.n_elms;
            break;

        default:
            [ctx formatError:MS_ERROR_VALUE
                            :"invalid list assignment for <%r>",
                             obj];
            goto error;
    }

    if ((guint) index >= len)
    {
        [ctx formatError:MS_ERROR_VALUE
                        :"index out of range"];
        goto error;
    }

    switch (MS_VALUE_TYPE (obj))
    {
        case MS_VALUE_LIST:
            ms_value_list_set_elm (obj, index, val);
            break;

        default:
            g_assert_not_reached ();
    }

out:
    ms_value_unref (obj);
    ms_value_unref (key);
    return val;

error:
    ms_value_unref (obj);
    node_unref (key);
    ms_value_unref (val);
    return NULL;
}


MSNodeSetItem *
_ms_node_set_item_new (MSNode     *obj,
                       MSNode     *key,
                       MSNode     *val)
{
    MSNodeSetItem *node;

    g_return_val_if_fail (obj && key && val, NULL);

    node = NODE_NEW (MSNodeSetItem,
                     MS_TYPE_NODE_SET_ITEM,
                     ms_node_set_item_eval,
                     ms_node_set_item_destroy);

    node->obj = node_ref (obj);
    node->key = node_ref (key);
    node->val = node_ref (val);

    return node;
}


/****************************************************************************/
/* MSNodeReturn
 */

static void
ms_node_return_destroy (MSNode *node)
{
    node_unref (MS_NODE_RETURN(node)->val);
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

    [ctx setReturn:ret];

    return ret;
}


MSNodeReturn *
_ms_node_return_new (MSNode *val)
{
    MSNodeReturn *node;

    node = NODE_NEW (MSNodeReturn,
                     MS_TYPE_NODE_RETURN,
                     ms_node_return_eval,
                     ms_node_return_destroy);

    node->val = node_ref (val);

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
            [ctx setBreak];
            break;
        case MS_BREAK_CONTINUE:
            [ctx setContinue];
            break;
        default:
            g_assert_not_reached ();
    }

    return ms_value_none ();
}


MSNodeBreak *
_ms_node_break_new (MSBreakType type)
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
    node_unref (MS_NODE_DICT_ELM(node_)->dict);
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
        val = _ms_value_get_method (obj, node->key);

    ms_value_unref (obj);

    if (!val)
        return [ctx formatError:MS_ERROR_VALUE
                               :"no key '%s'",
                                node->key];
    else
        return val;
}


MSNodeDictElm *
_ms_node_dict_elm_new (MSNode     *dict,
                       const char *key)
{
    MSNodeDictElm *node;

    g_return_val_if_fail (dict != NULL, NULL);
    g_return_val_if_fail (key != NULL, NULL);

    node = NODE_NEW (MSNodeDictElm,
                     MS_TYPE_NODE_DICT_ELM,
                     ms_node_dict_elm_eval,
                     ms_node_dict_elm_destroy);

    node->dict = node_ref (dict);
    node->key = g_strdup (key);

    return node;
}


/****************************************************************************/
/* MSNodeDictAssign
 */

static void
ms_node_dict_assign_destroy (MSNode *node_)
{
    node_unref (MS_NODE_DICT_ASSIGN(node_)->dict);
    g_free (MS_NODE_DICT_ASSIGN(node_)->key);
    node_unref (MS_NODE_DICT_ASSIGN(node_)->val);
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
        [ctx formatError:MS_ERROR_TYPE
                        :"<%r> is not a dict object",
                         obj];
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
_ms_node_dict_assign_new (MSNode     *dict,
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

    node->dict = node_ref (dict);
    node->key = g_strdup (key);
    node->val = node_ref (val);

    return node;
}


/****************************************************************************/
/* MSNodeDictEntry
 */

static void
ms_node_dict_entry_destroy (MSNode *node_)
{
    node_unref (MS_NODE_DICT_ENTRY(node_)->val);
    g_free (MS_NODE_DICT_ENTRY(node_)->key);
}


MSNodeDictEntry *
_ms_node_dict_entry_new (const char *key,
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
    node->val = node_ref (val);

    return node;
}


/****************************************************************************/
/* MSNodeDict
 */

static void
ms_node_dict_destroy (MSNode *node_)
{
    node_unref (MS_NODE_DICT(node_)->entries);
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
_ms_node_dict_new (MSNodeList *entries)
{
    MSNodeDict *node;

    g_return_val_if_fail (!entries || MS_IS_NODE_LIST (entries), NULL);

    node = NODE_NEW (MSNodeDict,
                     MS_TYPE_NODE_DICT,
                     ms_node_dict_eval,
                     ms_node_dict_destroy);

    node->entries = node_ref (entries);

    return node;
}

/* -*- objc -*- */
