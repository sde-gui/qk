/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mootextbtree.c
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

#define MOOEDIT_COMPILATION
#include "mooedit/mootext-private.h"
#include <string.h>


#define USE_MEM_CHUNK 0


static BTNode  *bt_node_new         (BTNode     *parent,
                                     guint       n_children,
                                     guint       count,
                                     guint       n_marks);
static BTData  *bt_data_new         (BTNode     *parent,
                                     gpointer    tag);
static void     bt_node_free_rec    (BTNode     *node,
                                     GSList    **removed_marks);
static void     bt_data_free        (BTData     *data,
                                     GSList    **removed_marks);

#define NODE_IS_ROOT(node__) (!(node__)->parent)

#if defined(MOO_DEBUG) && 0
#define WANT_CHECK_INTEGRITY
static void CHECK_INTEGRITY (BTree *tree, gboolean check_capacity);
#else
#define CHECK_INTEGRITY(tree,check_capacity)
#endif


#if USE_MEM_CHUNK
static GMemChunk *btree_data_chunk = NULL;
static GMemChunk *btree_node_chunk = NULL;
static GMemChunk *btree_hl_info_chunk = NULL;
#endif

#if !USE_MEM_CHUNK
#undef g_chunk_free
#undef g_chunk_new0
#define g_chunk_free(m,c) g_free (m)
#define g_chunk_new0(T,c) g_new0 (T, 1)
#endif

inline static void
data_free__ (BTData *data)
{
    g_chunk_free (data, btree_data_chunk);
}

inline static BTData*
data_new__ (void)
{
    return g_chunk_new0 (BTData, btree_data_chunk);
}

inline static void
node_free__ (BTNode *node)
{
    g_chunk_free (node, btree_node_chunk);
}

inline static BTNode*
node_new__ (void)
{
    return g_chunk_new0 (BTNode, btree_node_chunk);
}

inline static void
hl_info_free__ (HLInfo *info)
{
    g_chunk_free (info, btree_hl_info_chunk);
}

inline static HLInfo*
hl_info_new__ (void)
{
    return g_chunk_new0 (HLInfo, btree_hl_info_chunk);
}


static void
init_mem_chunk (void)
{
#if USE_MEM_CHUNK
    if (!btree_node_chunk)
    {
        btree_node_chunk = g_mem_chunk_create (BTNode, 512, G_ALLOC_AND_FREE);
        btree_data_chunk = g_mem_chunk_create (BTData, 512, G_ALLOC_AND_FREE);
        btree_hl_info_chunk = g_mem_chunk_create (HLInfo, 512, G_ALLOC_AND_FREE);
    }
#endif
}


BTree*
moo_text_btree_new (void)
{
    BTree *tree = g_new0 (BTree, 1);

    init_mem_chunk ();

    tree->stamp = 1;
    tree->depth = 0;
    tree->root = bt_node_new (NULL, 1, 1, 0);
    tree->root->is_bottom = TRUE;
    tree->root->data[0] = bt_data_new (tree->root, NULL);

    CHECK_INTEGRITY (tree, TRUE);

    return tree;
}


guint
moo_text_btree_size (BTree *tree)
{
    g_return_val_if_fail (tree != 0, 0);
    return tree->root->count;
}


void
moo_text_btree_free (BTree *tree)
{
    if (tree)
    {
        bt_node_free_rec (tree->root, NULL);
        g_free (tree);
    }
}


BTData*
moo_text_btree_get_data (BTree          *tree,
                         guint           index_)
{
    BTNode *node;

    g_assert (tree != NULL);
    g_assert (index_ < tree->root->count);

    node = tree->root;

    while (!node->is_bottom)
    {
        BTNode **child;

        for (child = node->children; index_ >= (*child)->count; child++)
            index_ -= (*child)->count;

        node = *child;
    }

    return node->data[index_];
}


static BTNode*
bt_node_new (BTNode *parent,
             guint   n_children,
             guint   count,
             guint   n_marks)
{
    BTNode *node = node_new__ ();

    node->parent = parent;
    node->count = count;
    node->n_children = n_children;
    node->n_marks = n_marks;

    return node;
}


static void
bt_node_free_rec (BTNode  *node,
                  GSList **removed_marks)
{
    if (node)
    {
        guint i;

        if (node->is_bottom)
        {
            for (i = 0; i < node->n_children; ++i)
                bt_data_free (node->data[i], removed_marks);
        }
        else
        {
            for (i = 0; i < node->n_children; ++i)
                bt_node_free_rec (node->children[i], removed_marks);
        }

        node_free__ (node);
    }
}


inline static HLInfo*
hl_info_new (void)
{
#ifdef __MOO__
    HLInfo *info = hl_info_new__ ();
    return info;
#else
    return NULL;
#endif
}


inline static void
hl_info_free (HLInfo *info)
{
    if (info)
    {
#ifdef __MOO__
        g_free (info->segments);
        g_slist_foreach (info->tags, (GFunc) g_object_unref, NULL);
        g_slist_free (info->tags);
        hl_info_free__ (info);
#endif
    }
}


static BTData*
bt_data_new (BTNode  *parent,
             gpointer tag)
{
    BTData *data = data_new__ ();

    data->parent = parent;
    data->hl_info = hl_info_new ();

    if (tag)
        data->hl_info->tags = g_slist_prepend (NULL, g_object_ref (tag));

    return data;
}


static void
bt_data_free (BTData  *data,
              GSList **removed_marks)
{
    if (data)
    {
        hl_info_free (data->hl_info);

        if (data->n_marks)
        {
            guint i;

            for (i = 0; i < data->n_marks; ++i)
            {
                if (removed_marks)
                {
                    *removed_marks = g_slist_prepend (*removed_marks, data->marks[i]);
                    _moo_line_mark_set_line (data->marks[i], NULL, -1, 0);
                }
                else
                {
                    _moo_line_mark_deleted (data->marks[i]);
                    g_object_unref (data->marks[i]);
                }
            }
        }

        g_free (data->marks);
        data_free__ (data);
    }
}


static guint
node_get_index (BTNode *node, gpointer child)
{
    guint i;
    for (i = 0; ; ++i)
        if (node->children[i] == child)
            return i;
}


static void
node_insert__ (BTNode *node, gpointer data, guint index_)
{
    g_assert (node != NULL);
    g_assert (node->n_children < BTREE_NODE_MAX_CAPACITY);
    g_assert (index_ <= node->n_children);

    if (index_ < node->n_children)
        memmove (node->children + index_ + 1,
                 node->children + index_,
                  (node->n_children - index_) * sizeof (BTData*));

    node->children[index_] = data;
    node->n_children++;
}


static void
node_remove__ (BTNode *node, gpointer data)
{
    guint index;

    g_assert (node != NULL);
    g_assert (node->n_children > 0);

    index = node_get_index (node, data);

    if (index + 1 < node->n_children)
        memmove (node->data + index,
                 node->data + index + 1,
                 (node->n_children - index - 1) * sizeof (BTData*));

    node->n_children--;
}


BTData*
moo_text_btree_insert (BTree          *tree,
                       guint           index_,
                       gpointer        tag)
{
    BTNode *node, *tmp;
    BTData *data;
    G_GNUC_UNUSED guint index_orig = index_;

    g_assert (tree != NULL);
    g_assert (index_ <= tree->root->count);

    tree->stamp++;

    node = tree->root;

    while (!node->is_bottom)
    {
        BTNode **child;

        for (child = node->children; index_ > (*child)->count; child++)
            index_ -= (*child)->count;

        node = *child;
    }

    g_assert (node->n_children < BTREE_NODE_MAX_CAPACITY);
    g_assert (index_ <= node->n_children);

    data = bt_data_new (node, tag);
    node_insert__ (node, data, index_);

    for (tmp = node; tmp != NULL; tmp = tmp->parent)
        tmp->count++;

    while (node->n_children == BTREE_NODE_MAX_CAPACITY)
    {
        BTNode *new_node;
        guint new_count, i, node_index;

        if (NODE_IS_ROOT (node))
        {
            tree->depth++;
            node->parent = bt_node_new (NULL, 1, node->count, node->n_marks);
            node->parent->children[0] = node;
            tree->root = node->parent;
        }

        if (node->is_bottom)
            new_count = BTREE_NODE_MIN_CAPACITY;
        else
            for (new_count = 0, i = BTREE_NODE_MIN_CAPACITY; i < BTREE_NODE_MAX_CAPACITY; ++i)
                new_count += node->children[i]->count;

        node_index = node_get_index (node->parent, node);
        g_assert (node_index < node->parent->n_children);

        new_node = bt_node_new (node->parent, BTREE_NODE_MIN_CAPACITY, new_count, 0);
        new_node->is_bottom = node->is_bottom;
        node->count -= new_count;
        node_insert__ (node->parent, new_node, node_index + 1);
        g_assert (node_get_index (node->parent, new_node) == node_index + 1);
        g_assert (node_get_index (node->parent, node) == node_index);
        memcpy (new_node->children,
                node->children + BTREE_NODE_MIN_CAPACITY,
                BTREE_NODE_MIN_CAPACITY * sizeof (BTNode*));
        node->n_children = BTREE_NODE_MIN_CAPACITY;
        for (i = 0; i < new_node->n_children; ++i)
            new_node->children[i]->parent = new_node;

        if (node->n_marks)
        {
            node->n_marks = 0;
            new_node->n_marks = 0;

            for (i = 0; i < node->n_children; ++i)
                node->n_marks += node->children[i]->n_marks;
            for (i = 0; i < new_node->n_children; ++i)
                new_node->n_marks += new_node->children[i]->n_marks;
        }

        node = node->parent;
    }

    CHECK_INTEGRITY (tree, TRUE);
    g_assert (data == moo_text_btree_get_data (tree, index_orig));

    return data;
}


static void
merge_nodes (BTNode *parent, guint first)
{
    BTNode *node, *next;
    int i;

    g_assert (first + 1 < parent->n_children);

    node = parent->children[first];
    next = parent->children[first+1];
    g_assert (node->n_children + next->n_children < BTREE_NODE_MAX_CAPACITY);

    memcpy (node->children + node->n_children,
            next->children,
            next->n_children * sizeof (BTNode*));

    for (i = node->n_children; i < node->n_children + next->n_children; ++i)
        node->children[i]->parent = node;

    node->n_children += next->n_children;
    node->count += next->count;
    node->n_marks += next->n_marks;

    node_remove__ (parent, next);
    node_free__ (next);
}


void
moo_text_btree_delete (BTree          *tree,
                       guint           index_,
                       GSList        **deleted_marks)
{
    BTNode *node, *tmp;
    BTData *data;

    g_assert (tree != NULL);
    g_assert (tree->root->count > 1);
    g_assert (index_ < tree->root->count);

    tree->stamp++;

    data = moo_text_btree_get_data (tree, index_);
    g_assert (data != NULL);

    node = data->parent;
    g_assert (node->count == node->n_children);
    node_remove__ (node, data);

    for (tmp = node; tmp != NULL; tmp = tmp->parent)
    {
        tmp->count--;
        g_assert (tmp->n_marks >= data->n_marks);
        tmp->n_marks -= data->n_marks;
    }

    bt_data_free (data, deleted_marks);

    while (node->n_children < BTREE_NODE_MIN_CAPACITY)
    {
        guint node_index;
        BTNode *parent = node->parent;

        if (!parent)
        {
            if (node->n_children > 1 || node->is_bottom)
                break;

            tree->depth--;
            tree->root = node->children[0];
            tree->root->parent = NULL;
            node_free__ (node);
            break;
        }
        else if (parent->n_children == 1)
        {
            g_assert (NODE_IS_ROOT (parent));
            tree->depth--;
            tree->root = node;
            node->parent = NULL;
            node_free__ (parent);
            break;
        }
        else
        {
            node_index = node_get_index (parent, node);

            if (node_index)
            {
                BTNode *sib = parent->children[node_index-1];

                if (sib->n_children > BTREE_NODE_MIN_CAPACITY)
                {
                    BTNode *child = sib->children[sib->n_children-1];
                    node_insert__ (node, child, 0);
                    node_remove__ (sib, child);
                    child->parent = node;

                    node->n_marks += child->n_marks;
                    sib->n_marks -= child->n_marks;

                    if (!node->is_bottom)
                    {
                        node->count += child->count;
                        sib->count -= child->count;
                    }
                    else
                    {
                        node->count++;
                        sib->count--;
                    }
                }
                else
                {
                    merge_nodes (parent, node_index-1);
                }
            }
            else
            {
                BTNode *sib = parent->children[1];

                if (sib->n_children > BTREE_NODE_MIN_CAPACITY)
                {
                    BTNode *child = sib->children[0];
                    node_insert__ (node, child, node->n_children);
                    node_remove__ (sib, child);
                    g_assert (node->n_children == BTREE_NODE_MIN_CAPACITY);
                    child->parent = node;

                    node->n_marks += child->n_marks;
                    sib->n_marks -= child->n_marks;

                    if (!node->is_bottom)
                    {
                        node->count += child->count;
                        sib->count -= child->count;
                    }
                    else
                    {
                        node->count++;
                        sib->count--;
                    }
                }
                else
                {
                    merge_nodes (parent, 0);
                }
            }
        }

        node = parent;
    }

    CHECK_INTEGRITY (tree, TRUE);
}


/* XXX */
void
moo_text_btree_insert_range (BTree      *tree,
                             int         first,
                             int         num,
                             gpointer    tag)
{
    int i;

    g_assert (tree != NULL);
    g_assert (first >= 0 && first <= (int) tree->root->count);
    g_assert (num > 0);

    for (i = 0; i < num; ++i)
        moo_text_btree_insert (tree, first, tag);
}


/* XXX */
void
moo_text_btree_delete_range (BTree      *tree,
                             int         first,
                             int         num,
                             GSList    **deleted_marks)
{
    int i;

    g_assert (tree != NULL);
    g_assert (first >= 0 && first < (int) tree->root->count);
    g_assert (num > 0 && first + num <= (int) tree->root->count);

    for (i = 0; i < num; ++i)
        moo_text_btree_delete (tree, first, deleted_marks);
}


void
moo_text_btree_update_n_marks (G_GNUC_UNUSED BTree *tree,
                               BTData     *data,
                               int         add)
{
    BTNode *node;

    for (node = (BTNode*) data; node != NULL; node = node->parent)
    {
        g_assert (add > 0 || (add < 0 && (int) node->n_marks >= -add));
        node->n_marks += add;
    }

    CHECK_INTEGRITY (tree, FALSE);
}


#ifdef WANT_CHECK_INTEGRITY

static void
node_check_count (BTNode *node)
{
    guint real_count = 0, mark_count = 0, i;

    if (node->is_bottom)
    {
        real_count = node->n_children;
        mark_count = node->n_marks;
    }
    else
    {
        for (i = 0; i < node->n_children; ++i)
        {
            real_count += node->children[i]->count;
            mark_count += node->children[i]->n_marks;
        }
    }

    g_assert (real_count == node->count);
    g_assert (mark_count == node->n_marks);
}


static void
node_check (BTNode *node, gboolean is_root, gboolean check_capacity)
{
    guint i;

    if (is_root)
    {
        g_assert (node->parent == NULL);
        g_assert (node->n_children >= 1);
    }
    else
    {
        g_assert (node->parent != NULL);
        if (check_capacity)
            g_assert (node->n_children >= BTREE_NODE_MIN_CAPACITY);
        else
            g_assert (node->n_children >= 1);
    }

    if (check_capacity)
        g_assert (node->n_children < BTREE_NODE_MAX_CAPACITY);

    g_assert (node->count >= node->n_children);

    if (!is_root)
    {
        guint index = node_get_index (node->parent, node);
        g_assert (index < node->parent->n_children);
    }

    node_check_count (node);

    if (!node->is_bottom)
        for (i = 0; i < node->n_children; ++i)
            node_check (node->children[i], FALSE, check_capacity);
}


static void CHECK_INTEGRITY (BTree *tree, gboolean check_capacity)
{
    guint i, p;
    BTNode *node;

    g_assert (tree != NULL);
    g_assert (tree->root != NULL);
    g_assert (tree->root->count != 0);

    for (i = 0, p = 1; i < BTREE_MAX_DEPTH - 1; ++i, p *= BTREE_NODE_MIN_CAPACITY) ;
    g_assert (p > 10000000);

    for (i = 0, node = tree->root; i < tree->depth; ++i, node = node->children[0])
        g_assert (!node->is_bottom);
    g_assert (node->is_bottom);

    node_check (tree->root, TRUE, check_capacity);
}

#endif /* WANT_CHECK_INTEGRITY */
