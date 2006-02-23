/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mootextbtree.h
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

#ifndef __MOO_TEXT_BTREE_H__
#define __MOO_TEXT_BTREE_H__

#ifndef MOOEDIT_COMPILATION
#error "This file may not be included"
#endif

#include <glib.h>

G_BEGIN_DECLS


#define BTREE_NODE_EXP 4
#define BTREE_NODE_MAX_CAPACITY (1 << BTREE_NODE_EXP)
#define BTREE_NODE_MIN_CAPACITY (BTREE_NODE_MAX_CAPACITY >> 1)
#define BTREE_MAX_DEPTH 9 /* 2^(3*(9-1)) == 2^24 > 16,777,216 - more than enough */
#define BTREE_MAX_DEPTH_EXP 4 /* 2^4 > 8 */

typedef struct _BTNode BTNode;
typedef struct _BTData BTData;
typedef struct _BTIter BTIter;
typedef struct _BTree BTree;

typedef struct _HLInfo HLInfo;
struct _MooLineMark;


struct _BTNode {
    BTNode *parent;
    guint n_marks;

    union {
        BTNode *children[BTREE_NODE_MAX_CAPACITY];
        BTData *data[BTREE_NODE_MAX_CAPACITY];
    };

    guint n_children : BTREE_NODE_EXP + 1;
    guint is_bottom : 1;
    guint count : (30 - BTREE_NODE_EXP);
};

struct _BTData {
    BTNode *parent;
    guint n_marks;

    HLInfo *hl_info;
    struct _MooLineMark **marks;
};

struct _BTree {
    BTNode *root;
    guint depth;
    guint stamp;
};


BTree      *moo_text_btree_new              (void);
void        moo_text_btree_free             (BTree      *tree);

guint       moo_text_btree_size             (BTree      *tree);

BTData     *moo_text_btree_get_data         (BTree      *tree,
                                             guint       index_);

BTData     *moo_text_btree_insert           (BTree      *tree,
                                             guint       index_,
                                             gpointer    tag);
void        moo_text_btree_delete           (BTree      *tree,
                                             guint       index_,
                                             GSList    **removed_marks);

void        moo_text_btree_insert_range     (BTree      *tree,
                                             int         first,
                                             int         num,
                                             gpointer    tag);
void        moo_text_btree_delete_range     (BTree      *tree,
                                             int         first,
                                             int         num,
                                             GSList    **removed_marks);

void        moo_text_btree_update_n_marks   (BTree      *tree,
                                             BTData     *data,
                                             int         add);


G_END_DECLS

#endif /* __MOO_TEXT_BTREE_H__ */
