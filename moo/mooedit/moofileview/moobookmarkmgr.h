/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *   moobookmarkmgr.h
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

#ifndef __MOO_BOOKMARK_MGR_H__
#define __MOO_BOOKMARK_MGR_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS


#define MOO_TYPE_BOOKMARK_MGR                (moo_bookmark_mgr_get_type ())
#define MOO_BOOKMARK_MGR(object)             (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_BOOKMARK_MGR, MooBookmarkMgr))
#define MOO_BOOKMARK_MGR_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_BOOKMARK_MGR, MooBookmarkMgrClass))
#define MOO_IS_BOOKMARK_MGR(object)          (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_BOOKMARK_MGR))
#define MOO_IS_BOOKMARK_MGR_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_BOOKMARK_MGR))
#define MOO_BOOKMARK_MGR_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_BOOKMARK_MGR, MooBookmarkMgrClass))

#define MOO_TYPE_BOOKMARK                    (moo_bookmark_get_type ())

typedef enum {
    MOO_BOOKMARK_MGR_COLUMN_BOOKMARK = 0,
    MOO_BOOKMARK_MGR_NUM_COLUMNS
} MooBookmarkMgrModelColumn;

typedef struct _MooBookmark             MooBookmark;
typedef struct _MooBookmarkMgr          MooBookmarkMgr;
typedef struct _MooBookmarkMgrPrivate   MooBookmarkMgrPrivate;
typedef struct _MooBookmarkMgrClass     MooBookmarkMgrClass;

struct _MooBookmark {
    char *path;
    char *display_path;
    char *label;
    char *icon_stock_id;
    GdkPixbuf *pixbuf;
};

struct _MooBookmarkMgr
{
    GObject parent;
    MooBookmarkMgrPrivate *priv;
};

struct _MooBookmarkMgrClass
{
    GObjectClass parent_class;

    void    (*changed)  (MooBookmarkMgr *watch);
};

typedef void   (*MooBookmarkFunc)           (MooBookmark    *bookmark,
                                             gpointer        data);

GType           moo_bookmark_get_type       (void) G_GNUC_CONST;
GType           moo_bookmark_mgr_get_type   (void) G_GNUC_CONST;

MooBookmark    *moo_bookmark_new            (const char     *name,
                                             const char     *path,
                                             const char     *icon);
MooBookmark    *moo_bookmark_copy           (MooBookmark    *bookmark);
void            moo_bookmark_free           (MooBookmark    *bookmark);

void            moo_bookmark_set_path       (MooBookmark    *bookmark,
                                             const char     *path);
void            moo_bookmark_set_display_path (MooBookmark  *bookmark,
                                             const char     *display_path);


MooBookmarkMgr *moo_bookmark_mgr_new        (void);
GtkTreeModel   *moo_bookmark_mgr_get_model  (MooBookmarkMgr *mgr);

void            moo_bookmark_mgr_add        (MooBookmarkMgr *mgr,
                                             MooBookmark    *bookmark);

void            moo_bookmark_mgr_fill_menu  (MooBookmarkMgr *mgr,
                                             GtkMenuShell   *menu,
                                             int             position,
                                             MooBookmarkFunc func,
                                             gpointer        data);
gboolean        moo_bookmark_mgr_is_empty   (MooBookmarkMgr *mgr);

#ifndef __MOO__
gboolean        moo_bookmark_mgr_load       (MooBookmarkMgr *mgr,
                                             const char     *file,
                                             gboolean        add,
                                             GError       **error);
gboolean        moo_bookmark_mgr_save       (MooBookmarkMgr *mgr,
                                             const char     *file,
                                             GError       **error);
#endif

GtkWidget      *moo_bookmark_mgr_get_editor (MooBookmarkMgr *mgr);


G_END_DECLS

#endif /* __MOO_BOOKMARK_MGR_H__ */
