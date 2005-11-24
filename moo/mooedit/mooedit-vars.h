/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mooedit.h
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

#ifndef __MOO_EDIT_VARS_H__
#define __MOO_EDIT_VARS_H__

#include "mooedit/mooedit-private.h"

G_BEGIN_DECLS


typedef struct {
    GHashTable *hash;
} VarPool;

struct _StringList {
    GHashTable *hash;
};

struct _VarTable {
    VarPool *pool;
    GHashTable *hash;
};

typedef struct {
    GValue value;
    guint dep : 4;
} Var;


static VarPool     *var_pool_new            (void);
static GParamSpec  *var_pool_get_pspec      (VarPool    *pool,
                                             const char *name);
static const char  *var_pool_find_name      (VarPool    *pool,
                                             const char *alias);
static void         var_pool_add            (VarPool    *pool,
                                             GParamSpec *pspec);
static void         var_pool_add_alias      (VarPool    *pool,
                                             const char *name,
                                             const char *alias);

static StringList  *string_list_new         (void);
static void         string_list_free        (StringList *list);
static void         string_list_add         (StringList *list,
                                             const char *s);

static VarTable    *var_table_new           (VarPool    *pool);
static void         var_table_free          (VarTable   *table);
static Var         *var_table_get           (VarTable   *table,
                                             const char *name);
static void         var_table_insert        (VarTable   *table,
                                             const char *name,
                                             Var        *var);
static void         var_table_remove        (VarTable   *table,
                                             const char *name);
static void         var_table_remove_by_dep (VarTable   *table,
                                             guint       dep,
                                             StringList *changed);

static Var         *var_new                 (guint       dep,
                                             GType       type);
static void         var_free                (Var        *var);


static StringList *
string_list_new (void)
{
    StringList *l = g_new0 (StringList, 1);
    l->hash = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
    return l;
}


static void
string_list_free (StringList *list)
{
    if (list)
    {
        g_hash_table_destroy (list->hash);
        g_free (list);
    }
}


static void
string_list_add (StringList *list,
                 const char *s)
{
    g_return_if_fail (list != NULL);
    g_return_if_fail (s != NULL);
    g_hash_table_insert (list->hash, g_strdup (s), NULL);
}


static VarTable *
var_table_new (VarPool *pool)
{
    VarTable *table = g_new0 (VarTable, 1);
    table->hash = g_hash_table_new_full (g_str_hash, g_str_equal,
                                         g_free, (GDestroyNotify) var_free);
    table->pool = pool;
    return table;
}


static void
var_table_free (VarTable *table)
{
    if (table)
    {
        g_hash_table_destroy (table->hash);
        g_free (table);
    }
}


static Var *
var_table_get (VarTable   *table,
               const char *name)
{
    return g_hash_table_lookup (table->hash, name);
}


static void
var_table_insert (VarTable   *table,
                  const char *name,
                  Var        *var)
{
    if (var)
        g_hash_table_insert (table->hash, g_strdup (name), var);
    else
        g_hash_table_remove (table->hash, name);
}


static void
var_table_remove (VarTable   *table,
                  const char *name)
{
    g_hash_table_remove (table->hash, name);
}


static gboolean
remove_by_dep (const char *name,
               Var        *val,
               gpointer    user_data)
{
    struct {
        StringList *list;
        guint dep;
    } *data = user_data;

    if (val->dep >= data->dep)
    {
        string_list_add (data->list, name);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static void
var_table_remove_by_dep (VarTable   *table,
                         guint       dep,
                         StringList *changed)
{
    struct {
        StringList *list;
        guint dep;
    } data = {changed, dep};

    g_hash_table_foreach_remove (table->hash,
                                 (GHRFunc) remove_by_dep,
                                 &data);
}


static Var *
var_new (guint dep,
         GType type)
{
    Var *var = g_new0 (Var, 1);
    g_value_init (&var->value, type);
    var->dep = dep;
    return var;
}


static void
var_free (Var *var)
{
    if (var)
    {
        g_value_unset (&var->value);
        g_free (var);
    }
}


static VarPool *
var_pool_new (void)
{
    VarPool *pool = g_new0 (VarPool, 1);
    pool->hash = g_hash_table_new_full (g_str_hash, g_str_equal, g_free,
                                        (GDestroyNotify) g_param_spec_unref);
    return pool;
}


static GParamSpec *
var_pool_get_pspec (VarPool    *pool,
                    const char *name)
{
    return g_hash_table_lookup (pool->hash, name);
}


static const char *
var_pool_find_name (VarPool    *pool,
                    const char *alias)
{
    GParamSpec *pspec = var_pool_get_pspec (pool, alias);
    return pspec ? g_param_spec_get_name (pspec) : NULL;
}


static void
var_pool_add (VarPool    *pool,
              GParamSpec *pspec)
{
    const char *name = g_param_spec_get_name (pspec);
    g_param_spec_ref (pspec);
    g_param_spec_sink (pspec);
    g_hash_table_insert (pool->hash, g_strdup (name), pspec);
}


static void
var_pool_add_alias (VarPool    *pool,
                    const char *name,
                    const char *alias)
{
    GParamSpec *pspec = var_pool_get_pspec (pool, name);
    g_return_if_fail (pspec != NULL);
    g_param_spec_ref (pspec);
    g_hash_table_insert (pool->hash, g_strdup (alias), pspec);
}


G_END_DECLS

#endif /* __MOO_EDIT_VARS_H__ */
