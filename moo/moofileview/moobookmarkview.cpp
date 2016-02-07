/*
 *   moobookmarkview.c
 *
 *   Copyright (C) 2004-2010 by Yevgen Muntyan <emuntyan@users.sourceforge.net>
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

#include "moofileview/moobookmarkview.h"
#include "moocpp/moocpp.h"
#include "marshals.h"
#include <gtk/gtk.h>
#include <string.h>

using namespace moo;

#define COLUMN_BOOKMARK MOO_BOOKMARK_MGR_COLUMN_BOOKMARK


static void moo_bookmark_view_finalize      (GObject            *object);

static void moo_bookmark_view_set_property  (GObject            *object,
                                             guint               prop_id,
                                             const GValue       *value,
                                             GParamSpec         *pspec);
static void moo_bookmark_view_get_property  (GObject            *object,
                                             guint               prop_id,
                                             GValue             *value,
                                             GParamSpec         *pspec);

static void row_activated                   (GtkTreeView        *treeview,
                                             GtkTreePath        *path,
                                             GtkTreeViewColumn  *column);

static void icon_data_func                  (gtk::TreeViewColumn column,
                                             gtk::CellRenderer   cell,
                                             gtk::TreeModel      model,
                                             const GtkTreeIter&  iter);
static void label_data_func                 (gtk::TreeViewColumn column,
                                             gtk::CellRenderer   cell,
                                             gtk::TreeModel      model,
                                             const GtkTreeIter&  iter);


/* MOO_TYPE_BOOKMARK_VIEW */
G_DEFINE_TYPE (MooBookmarkView, _moo_bookmark_view, GTK_TYPE_TREE_VIEW)

enum {
    PROP_0,
    PROP_MGR
};

enum {
    BOOKMARK_ACTIVATED,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

static void
_moo_bookmark_view_class_init (MooBookmarkViewClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GtkTreeViewClass *treeview_class = GTK_TREE_VIEW_CLASS (klass);

    gobject_class->finalize = moo_bookmark_view_finalize;
    gobject_class->set_property = moo_bookmark_view_set_property;
    gobject_class->get_property = moo_bookmark_view_get_property;

    treeview_class->row_activated = row_activated;

    g_object_class_install_property (gobject_class,
                                     PROP_MGR,
                                     g_param_spec_object ("mgr",
                                             "mgr",
                                             "mgr",
                                             MOO_TYPE_BOOKMARK_MGR,
                                             (GParamFlags) G_PARAM_READWRITE));

    signals[BOOKMARK_ACTIVATED] =
            g_signal_new ("bookmark-activated",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                          G_STRUCT_OFFSET (MooBookmarkViewClass, bookmark_activated),
                          NULL, NULL,
                          _moo_marshal_VOID__BOXED,
                          G_TYPE_NONE, 1,
                          MOO_TYPE_BOOKMARK | G_SIGNAL_TYPE_STATIC_SCOPE);
}


static void
_moo_bookmark_view_init (MooBookmarkView *view)
{
    gtk::TreeView treeview = *GTK_TREE_VIEW (view);

    treeview.set_headers_visible (FALSE);

    GtkTreeSelection *selection = treeview.get_selection ();
    gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);

    gtk::TreeViewColumnPtr column = gtk::TreeViewColumn::create ();
    treeview.append_column (*column);

    /* Icon */
    gtk::CellRendererPtr cell = gtk::CellRendererPixbuf::create ();
    column->pack_start (*cell, FALSE);
    column->set_cell_data_func (*cell, icon_data_func);

    /* Label */
    cell = gtk::CellRendererText::create ();
    cell->set ("xpad", 6);
    column->pack_start (*cell, FALSE);
    column->set_cell_data_func (*cell, label_data_func);
}


static void
moo_bookmark_view_finalize (GObject *object)
{
    MooBookmarkView *view = MOO_BOOKMARK_VIEW (object);

    if (view->mgr)
        g_object_unref (view->mgr);

    gtk_tree_view_set_model (GTK_TREE_VIEW (view), NULL);

    G_OBJECT_CLASS (_moo_bookmark_view_parent_class)->finalize (object);
}


static void
moo_bookmark_view_set_property (GObject        *object,
                                guint           prop_id,
                                const GValue   *value,
                                GParamSpec     *pspec)
{
    MooBookmarkView *view = MOO_BOOKMARK_VIEW (object);

    switch (prop_id)
    {
        case PROP_MGR:
            _moo_bookmark_view_set_mgr (view, MOO_BOOKMARK_MGR_OPT (g_value_get_object (value)));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static void
moo_bookmark_view_get_property (GObject        *object,
                                guint           prop_id,
                                GValue         *value,
                                GParamSpec     *pspec)
{
    MooBookmarkView *view = MOO_BOOKMARK_VIEW (object);

    switch (prop_id)
    {
        case PROP_MGR:
            g_value_set_object (value, view->mgr);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


GtkWidget *
_moo_bookmark_view_new (MooBookmarkMgr *mgr)
{
    g_return_val_if_fail (!mgr || MOO_IS_BOOKMARK_MGR (mgr), NULL);
    return GTK_WIDGET (g_object_new (MOO_TYPE_BOOKMARK_VIEW, "mgr", mgr, (const char*) NULL));
}


void
_moo_bookmark_view_set_mgr (MooBookmarkView    *view,
                            MooBookmarkMgr     *mgr)
{
    g_return_if_fail (MOO_IS_BOOKMARK_VIEW (view));
    g_return_if_fail (!mgr || MOO_IS_BOOKMARK_MGR (mgr));

    if (mgr == view->mgr)
        return;

    if (view->mgr)
        g_object_unref (view->mgr);
    if (mgr)
        g_object_ref (mgr);
    view->mgr = mgr;

    gtk_tree_view_set_model (GTK_TREE_VIEW (view),
                             mgr ? _moo_bookmark_mgr_get_model (mgr).gobj () : NULL);

    g_object_notify (G_OBJECT (view), "mgr");
}


objp<MooBookmark>
get_bookmark (gtk::TreeModel     model,
              const GtkTreeIter& iter)
{
    objp<MooBookmark> bookmark;
    model.get (iter, COLUMN_BOOKMARK, bookmark);
    return bookmark;
}

objp<MooBookmark>
get_bookmark (GtkTreeModel *model,
              GtkTreeIter  *iter)
{
    g_return_val_if_fail (GTK_IS_TREE_MODEL (model), nullptr);
    g_return_val_if_fail (iter != nullptr, nullptr);
    return get_bookmark (*wrap (model), *iter);
}

#if 0
static void
set_bookmark (GtkListStore *store,
              GtkTreeIter  *iter,
              MooBookmark  *bookmark)
{
    gtk_list_store_set (store, iter, COLUMN_BOOKMARK, bookmark, -1);
}
#endif


static void
icon_data_func (gtk::TreeViewColumn,
                gtk::CellRenderer   cell,
                gtk::TreeModel      model,
                const GtkTreeIter&  iter)
{
    objp<MooBookmark> bookmark = get_bookmark (model, iter);

    if (!bookmark)
        cell.set ("pixbuf", nullptr,
                  "stock-id", nullptr);
    else
        cell.set ("pixbuf", bookmark->pixbuf,
                  "stock-id", bookmark->icon_stock_id,
                  "stock-size", GTK_ICON_SIZE_LARGE_TOOLBAR);
}


static void
label_data_func (gtk::TreeViewColumn,
                 gtk::CellRenderer   cell,
                 gtk::TreeModel      model,
                 const GtkTreeIter&  iter)
{
    objp<MooBookmark> bookmark = get_bookmark (model, iter);

    if (!bookmark)
        cell.set ("text", "-------");
    else
        cell.set ("text", bookmark->label);
}


static objp<MooBookmark>
_moo_bookmark_view_get_bookmark (MooBookmarkView    *view,
                                 GtkTreePath        *path)
{
    GtkTreeModel *model;
    GtkTreeIter iter;

    g_return_val_if_fail (MOO_IS_BOOKMARK_VIEW (view), NULL);
    g_return_val_if_fail (path != NULL, NULL);

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (view));
    g_return_val_if_fail (model != NULL, NULL);

    if (!gtk_tree_model_get_iter (model, &iter, path))
        return NULL;

    return get_bookmark (model, &iter);
}


static void
row_activated (GtkTreeView        *treeview,
               GtkTreePath        *path,
               G_GNUC_UNUSED GtkTreeViewColumn *column)
{
    objp<MooBookmark> bookmark;
    bookmark = _moo_bookmark_view_get_bookmark (MOO_BOOKMARK_VIEW (treeview), path);
    g_signal_emit (treeview, signals[BOOKMARK_ACTIVATED], 0, bookmark.get ());
}
