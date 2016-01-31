/*
 *   moobookmarkmgr.h
 *
 *   Copyright (C) 2004-2016 by Yevgen Muntyan <emuntyan@users.sourceforge.net>
 *
 *   This file is part of medit.  medit is free software; you can
 *   redistribute it and/or modify it under the terms of the
 *   GNU Lesser General Public License as published by the
 *   Free Software Foundation; either version 2.1 of the License,
 *   or (at your option) any later version.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with medit.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <gtk/gtk.h>
#include <mooutils/moouixml.h>
#include <moocpp/moocpp.h>

G_BEGIN_DECLS

typedef struct MooBookmarkMgr MooBookmarkMgr;

MooBookmarkMgr *_moo_bookmark_mgr_new (void);

G_END_DECLS

#ifdef __cplusplus

#define MOO_TYPE_BOOKMARK_MGR                (_moo_bookmark_mgr_get_type ())
#define MOO_BOOKMARK_MGR(object)             (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_BOOKMARK_MGR, MooBookmarkMgr))
#define MOO_BOOKMARK_MGR_OPT(object)         (moo::object_cast_opt<MooBookmarkMgr> (object))
#define MOO_BOOKMARK_MGR_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_BOOKMARK_MGR, MooBookmarkMgrClass))
#define MOO_IS_BOOKMARK_MGR(object)          (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_BOOKMARK_MGR))
#define MOO_IS_BOOKMARK_MGR_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_BOOKMARK_MGR))
#define MOO_BOOKMARK_MGR_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_BOOKMARK_MGR, MooBookmarkMgrClass))

#define MOO_TYPE_BOOKMARK                    (_moo_bookmark_get_type ())

typedef enum {
    MOO_BOOKMARK_MGR_COLUMN_BOOKMARK = 0,
    MOO_BOOKMARK_MGR_NUM_COLUMNS
} MooBookmarkMgrModelColumn;

struct MooBookmark
{
    MooBookmark (const char* label,
                 const char* path,
                 const char* icon);
    ~MooBookmark ();

    MooBookmark (const MooBookmark&) = default;
    MooBookmark& operator=(const MooBookmark&) = delete;
    MooBookmark (MooBookmark&&);
    MooBookmark& operator=(MooBookmark&&);

    moo::gstr path;
    moo::gstr display_path;
    moo::gstr label;
    moo::gstr icon_stock_id;
    moo::gobj_ptr<GdkPixbuf> pixbuf;
};

struct MooBookmarkMgrPrivate;

struct MooBookmarkMgr
{
    GObject parent;
    MooBookmarkMgrPrivate *priv;
};

struct MooBookmarkMgrClass
{
    GObjectClass parent_class;

    void (*changed)   (MooBookmarkMgr *mgr);

    void (*activate)  (MooBookmarkMgr *mgr,
                       MooBookmark    *bookmark,
                       GObject        *user);
};


GType           _moo_bookmark_get_type      (void) G_GNUC_CONST;
GType           _moo_bookmark_mgr_get_type  (void) G_GNUC_CONST;

moo::gtk::TreeModelPtr _moo_bookmark_mgr_get_model (MooBookmarkMgr *mgr);

void            _moo_bookmark_mgr_add       (MooBookmarkMgr *mgr,
                                             moo::objp<MooBookmark> bookmark);

GtkWidget      *_moo_bookmark_mgr_get_editor(MooBookmarkMgr *mgr);

void            _moo_bookmark_mgr_add_user  (MooBookmarkMgr *mgr,
                                             gpointer        user, /* GObject* */
                                             MooActionCollection *actions,
                                             MooUiXml       *xml,
                                             const char     *path);
void            _moo_bookmark_mgr_remove_user(MooBookmarkMgr *mgr,
                                             gpointer        user); /* GObject* */

MOO_DEFINE_GOBJ_TYPE (MooBookmarkMgr, GObject, MOO_TYPE_BOOKMARK_MGR)

#endif // __cplusplus
