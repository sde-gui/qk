/*
 *   moofold.h
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

#ifndef MOOEDIT_COMPILATION
#error "This file may not be included"
#endif

#ifndef __MOO_FOLD_H__
#define __MOO_FOLD_H__

#include <mooedit/moolinemark.h>

G_BEGIN_DECLS


#define MOO_FOLD_TAG "moo-fold-invisible"

typedef struct _MooFoldTree     MooFoldTree;

struct _MooFoldTree
{
    MooFold *folds;
    guint n_folds;
    MooTextBuffer *buffer;
    guint consistent : 1;
};

struct _MooFold
{
    GObject object;

    MooFold *parent;
    MooFold *prev;
    MooFold *next;
    MooFold *children;  /* chlidren are sorted by line */
    guint n_children;

    MooLineMark *start;
    MooLineMark *end;   /* may be NULL */

    guint collapsed : 1;
    guint deleted : 1;  /* alive just because of reference count */
};

struct _MooFoldClass
{
    GObjectClass object_class;
};


int          _moo_fold_get_start    (MooFold        *fold);
int          _moo_fold_get_end      (MooFold        *fold);

gboolean     _moo_fold_is_deleted   (MooFold        *fold);

MooFoldTree *_moo_fold_tree_new     (MooTextBuffer  *buffer);
void         _moo_fold_tree_free    (MooFoldTree    *tree);

MooFold     *_moo_fold_tree_add     (MooFoldTree    *tree,
                                     int             first_line,
                                     int             last_line);
void         _moo_fold_tree_remove  (MooFoldTree    *tree,
                                     MooFold        *fold);
GSList      *_moo_fold_tree_get     (MooFoldTree    *tree,
                                     int             first_line,
                                     int             last_line);

void         _moo_fold_tree_expand  (MooFoldTree    *tree,
                                     MooFold        *fold);
void         _moo_fold_tree_collapse(MooFoldTree    *tree,
                                     MooFold        *fold);


G_END_DECLS

#endif /* __MOO_FOLD_H__ */
