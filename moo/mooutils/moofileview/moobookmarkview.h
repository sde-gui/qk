/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *   moobookmarkview.h
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

#ifndef __MOO_BOOKMARK_VIEW_H__
#define __MOO_BOOKMARK_VIEW_H__

#include <mooutils/moofileview/moobookmarkmgr.h>
#include <gtk/gtktreeview.h>

G_BEGIN_DECLS


#define MOO_TYPE_BOOKMARK_VIEW                (moo_bookmark_view_get_type ())
#define MOO_BOOKMARK_VIEW(object)             (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_BOOKMARK_VIEW, MooBookmarkView))
#define MOO_BOOKMARK_VIEW_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_BOOKMARK_VIEW, MooBookmarkViewClass))
#define MOO_IS_BOOKMARK_VIEW(object)          (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_BOOKMARK_VIEW))
#define MOO_IS_BOOKMARK_VIEW_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_BOOKMARK_VIEW))
#define MOO_BOOKMARK_VIEW_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_BOOKMARK_VIEW, MooBookmarkViewClass))


typedef struct _MooBookmarkView          MooBookmarkView;
typedef struct _MooBookmarkViewPrivate   MooBookmarkViewPrivate;
typedef struct _MooBookmarkViewClass     MooBookmarkViewClass;

struct _MooBookmarkView
{
    GtkTreeView tree_view;

    MooBookmarkMgr *mgr;
};

struct _MooBookmarkViewClass
{
    GtkTreeViewClass tree_view_class;

    void (*bookmark_activated) (MooBookmarkView *view,
                                MooBookmark     *bookmark);
};


GType           moo_bookmark_view_get_type      (void) G_GNUC_CONST;

GtkWidget      *moo_bookmark_view_new           (MooBookmarkMgr     *mgr);

void            moo_bookmark_view_set_mgr       (MooBookmarkView    *view,
                                                 MooBookmarkMgr     *mgr);

MooBookmark    *moo_bookmark_view_get_bookmark  (MooBookmarkView    *view,
                                                 GtkTreePath        *path);


G_END_DECLS

#endif /* __MOO_BOOKMARK_VIEW_H__ */
