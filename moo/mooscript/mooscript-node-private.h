/*
 *   mooscript-node-private.h
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

#ifndef __MOO_SCRIPT_NODE_PRIVATE_H__
#define __MOO_SCRIPT_NODE_PRIVATE_H__

#include "mooscript-node.h"
#include "mooscript-value-private.h"

G_BEGIN_DECLS


typedef enum {
    MS_TYPE_NODE_0,
    MS_TYPE_NODE_LIST,
    MS_TYPE_NODE_VAR,
    MS_TYPE_NODE_ENV_VAR,
    MS_TYPE_NODE_FUNCTION,
    MS_TYPE_NODE_IF_ELSE,
    MS_TYPE_NODE_WHILE,
    MS_TYPE_NODE_FOR,
    MS_TYPE_NODE_ASSIGN,
    MS_TYPE_NODE_VALUE,
    MS_TYPE_NODE_VAL_LIST,
    MS_TYPE_NODE_RETURN,
    MS_TYPE_NODE_BREAK,
    MS_TYPE_NODE_DICT_ELM,
    MS_TYPE_NODE_DICT_ASSIGN,
    MS_TYPE_NODE_DICT,
    MS_TYPE_NODE_DICT_ENTRY,
    MS_TYPE_NODE_GET_ITEM,
    MS_TYPE_NODE_SET_ITEM,
    MS_TYPE_NODE_LAST
} MSNodeType;


typedef struct _MSNodeList MSNodeList;
typedef struct _MSNodeVar MSNodeVar;
typedef struct _MSNodeFunction MSNodeFunction;
typedef struct _MSNodeIfElse MSNodeIfElse;
typedef struct _MSNodeWhile MSNodeWhile;
typedef struct _MSNodeFor MSNodeFor;
typedef struct _MSNodeAssign MSNodeAssign;
typedef struct _MSNodeValue MSNodeValue;
typedef struct _MSNodeEnvVar MSNodeEnvVar;
typedef struct _MSNodeValList MSNodeValList;
typedef struct _MSNodeBreak MSNodeBreak;
typedef struct _MSNodeReturn MSNodeReturn;
typedef struct _MSNodeDict MSNodeDict;
typedef struct _MSNodeDictEntry MSNodeDictEntry;
typedef struct _MSNodeDictElm MSNodeDictElm;
typedef struct _MSNodeDictAssign MSNodeDictAssign;
typedef struct _MSNodeGetItem MSNodeGetItem;
typedef struct _MSNodeSetItem MSNodeSetItem;


typedef MSValue* (*MSNodeEval)    (MSNode *node, MSContext *ctx);
typedef void     (*MSNodeDestroy) (MSNode *node);


struct _MSNode {
    guint ref_count;
    MSNodeEval eval;
    MSNodeDestroy destroy;
    MSNodeType type;
};

#if 1
#define MS_NODE(node_) ((MSNode*) node_)
#else
inline static MSNode*
_ms_node_check (gpointer pnode)
{
    MSNode *node = pnode;
    g_assert (node != NULL);
    g_assert (node->type && node->type < MS_TYPE_NODE_LAST);
    return pnode;
}
#define MS_NODE(node_) _ms_node_check (node_)
#endif

#define MS_NODE_TYPE(node_) (MS_NODE(node_)->type)

#if 1
#define MS_NODE_CAST(node_,type_,Type_) ((Type_*) node_)
#else
inline static gpointer
_ms_node_check_type (gpointer   pnode,
                     MSNodeType type)
{
    MSNode *node = pnode;
    g_assert (node != NULL);
    g_assert (MS_NODE_TYPE (node) == type);
    return pnode;
}
#define MS_NODE_CAST(node_,type_,Type_)             \
    ((Type_*) _ms_node_check_type (node_, type_))
#endif

#define MS_NODE_LIST(node_)         MS_NODE_CAST (node_, MS_TYPE_NODE_LIST, MSNodeList)
#define MS_NODE_VAR(node_)          MS_NODE_CAST (node_, MS_TYPE_NODE_VAR, MSNodeVar)
#define MS_NODE_FUNCTION(node_)     MS_NODE_CAST (node_, MS_TYPE_NODE_FUNCTION, MSNodeFunction)
#define MS_NODE_IF_ELSE(node_)      MS_NODE_CAST (node_, MS_TYPE_NODE_IF_ELSE, MSNodeIfElse)
#define MS_NODE_WHILE(node_)        MS_NODE_CAST (node_, MS_TYPE_NODE_WHILE, MSNodeWhile)
#define MS_NODE_FOR(node_)          MS_NODE_CAST (node_, MS_TYPE_NODE_FOR, MSNodeFor)
#define MS_NODE_ASSIGN(node_)       MS_NODE_CAST (node_, MS_TYPE_NODE_ASSIGN, MSNodeAssign)
#define MS_NODE_VALUE(node_)        MS_NODE_CAST (node_, MS_TYPE_NODE_VALUE, MSNodeValue)
#define MS_NODE_ENV_VAR(node_)      MS_NODE_CAST (node_, MS_TYPE_NODE_ENV_VAR, MSNodeEnvVar)
#define MS_NODE_VAL_LIST(node_)     MS_NODE_CAST (node_, MS_TYPE_NODE_VAL_LIST, MSNodeValList)
#define MS_NODE_BREAK(node_)        MS_NODE_CAST (node_, MS_TYPE_NODE_BREAK, MSNodeBreak)
#define MS_NODE_RETURN(node_)       MS_NODE_CAST (node_, MS_TYPE_NODE_RETURN, MSNodeReturn)
#define MS_NODE_DICT(node_)         MS_NODE_CAST (node_, MS_TYPE_NODE_DICT, MSNodeDict)
#define MS_NODE_DICT_ENTRY(node_)   MS_NODE_CAST (node_, MS_TYPE_NODE_DICT_ENTRY, MSNodeDictEntry)
#define MS_NODE_DICT_ELM(node_)     MS_NODE_CAST (node_, MS_TYPE_NODE_DICT_ELM, MSNodeDictElm)
#define MS_NODE_DICT_ASSIGN(node_)  MS_NODE_CAST (node_, MS_TYPE_NODE_DICT_ASSIGN, MSNodeDictAssign)
#define MS_NODE_GET_ITEM(node_)     MS_NODE_CAST (node_, MS_TYPE_NODE_GET_ITEM, MSNodeGetItem)
#define MS_NODE_SET_ITEM(node_)     MS_NODE_CAST (node_, MS_TYPE_NODE_SET_ITEM, MSNodeSetItem)

#define MS_IS_NODE_VAR(node) (node && MS_NODE_TYPE(node) == MS_TYPE_NODE_VAR)
#define MS_IS_NODE_VALUE(node) (node && MS_NODE_TYPE(node) == MS_TYPE_NODE_VALUE)
#define MS_IS_NODE_VAL_LIST(node) (node && MS_NODE_TYPE(node) == MS_TYPE_NODE_VAL_LIST)
#define MS_IS_NODE_LIST(node) (node && MS_NODE_TYPE(node) == MS_TYPE_NODE_LIST)
#define MS_IS_NODE_RETURN(node) (node && MS_NODE_TYPE(node) == MS_TYPE_NODE_RETURN)
#define MS_IS_NODE_BREAK(node) (node && MS_NODE_TYPE(node) == MS_TYPE_NODE_BREAK)


struct _MSNodeList {
    MSNode node;
    MSNode **nodes;
    guint n_nodes;
    guint n_nodes_allocd__;
};


struct _MSNodeVar {
    MSNode node;
    char *name;
};


struct _MSNodeFunction {
    MSNode node;
    MSNode *func;
    MSNodeList *args;
};


typedef enum {
    MS_COND_BEFORE,
    MS_COND_AFTER
} MSCondType;

struct _MSNodeWhile {
    MSNode node;
    MSCondType type;
    MSNode *condition;
    MSNode *what;
};


struct _MSNodeFor {
    MSNode node;
    MSNode *variable;
    MSNode *list;
    MSNode *what;
};


struct _MSNodeIfElse {
    MSNode node;
    MSNodeList *list; /* if a then b; elif c then d; else e -> (a, b, c, d, NULL, e) */
};


struct _MSNodeAssign {
    MSNode node;
    MSNodeVar *var;
    MSNode *val;
};


struct _MSNodeGetItem {
    MSNode node;
    MSNode *obj;
    MSNode *key;
};

struct _MSNodeSetItem {
    MSNode node;
    MSNode *obj;
    MSNode *key;
    MSNode *val;
};


struct _MSNodeValue {
    MSNode node;
    MSValue *value;
};


struct _MSNodeEnvVar {
    MSNode node;
    MSNode *name;
    MSNode *dflt;
};


typedef enum {
    MS_VAL_LIST,
    MS_VAL_RANGE
} MSValListType;

struct _MSNodeValList {
    MSNode node;
    MSValListType type;
    MSNodeList *elms;
    MSNode *first;
    MSNode *last;
};


typedef enum {
    MS_BREAK_BREAK,
    MS_BREAK_CONTINUE
} MSBreakType;

struct _MSNodeBreak {
    MSNode node;
    MSBreakType type;
};


struct _MSNodeReturn {
    MSNode node;
    MSNode *val;
};


struct _MSNodeDict {
    MSNode node;
    MSNodeList *entries;
};

struct _MSNodeDictEntry {
    MSNode node;
    char *key;
    MSNode *val;
};

struct _MSNodeDictElm {
    MSNode node;
    MSNode *dict;
    char *key;
};

struct _MSNodeDictAssign {
    MSNode node;
    MSNode *dict;
    char *key;
    MSNode *val;
};


MSNodeList     *_ms_node_list_new           (void);
void            _ms_node_list_add           (MSNodeList *list,
                                             MSNode     *node);

MSNodeFunction *_ms_node_function_new       (MSNode     *func,
                                             MSNodeList *args);
MSNodeFunction *_ms_node_binary_op_new      (MSBinaryOp  op,
                                             MSNode     *a,
                                             MSNode     *b);
MSNodeFunction *_ms_node_unary_op_new       (MSUnaryOp   op,
                                             MSNode     *val);

MSNodeIfElse   *_ms_node_if_else_new        (MSNode     *condition,
                                             MSNode     *then_,
                                             MSNodeList *elif_,
                                             MSNode     *else_);

MSNodeWhile    *_ms_node_while_new          (MSCondType  type,
                                             MSNode     *cond,
                                             MSNode     *what);
MSNodeFor      *_ms_node_for_new            (MSNode     *var,
                                             MSNode     *list,
                                             MSNode     *what);

MSNodeAssign   *_ms_node_assign_new         (MSNodeVar  *var,
                                             MSNode     *val);

MSNodeValue    *_ms_node_value_new          (MSValue    *value);
MSNodeValList  *_ms_node_val_list_new       (MSNodeList *list);
MSNodeValList  *_ms_node_val_range_new      (MSNode     *first,
                                             MSNode     *last);

MSNodeEnvVar   *_ms_node_env_var_new        (MSNode     *name,
                                             MSNode     *dflt);
MSNodeVar      *_ms_node_var_new            (const char *name);

MSNodeGetItem  *_ms_node_get_item_new       (MSNode     *obj,
                                             MSNode     *key);
MSNodeSetItem  *_ms_node_set_item_new       (MSNode     *obj,
                                             MSNode     *key,
                                             MSNode     *val);

MSNodeBreak    *_ms_node_break_new          (MSBreakType type);
MSNodeReturn   *_ms_node_return_new         (MSNode     *val);

MSNodeDict     *_ms_node_dict_new           (MSNodeList *entries);
MSNodeDictEntry *_ms_node_dict_entry_new    (const char *key,
                                             MSNode     *val);
MSNodeDictElm  *_ms_node_dict_elm_new       (MSNode     *dict,
                                             const char *key);
MSNodeDictAssign *_ms_node_dict_assign_new  (MSNode     *dict,
                                             const char *key,
                                             MSNode     *val);


G_END_DECLS

#endif /* __MOO_SCRIPT_NODE_PRIVATE_H__ */
