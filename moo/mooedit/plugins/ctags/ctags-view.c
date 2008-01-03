/*
 *   ctags-view.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *   Copyright (C) 2008      by Christian Dywan <christian@twotoasts.de>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#include "ctags-view.h"
#include "ctags-doc.h"
#include <mooutils/moomarshals.h>
#include <mooutils/mooutils-gobject.h>
#include <gtk/gtk.h>

G_DEFINE_TYPE (MooCtagsView, _moo_ctags_view, GTK_TYPE_TREE_VIEW)

struct _MooCtagsViewPrivate {
    guint nothing;
};


static void
moo_ctags_view_cursor_changed (GtkTreeView *treeview)
{
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;
    MooCtagsEntry *entry;

    if (GTK_TREE_VIEW_CLASS(_moo_ctags_view_parent_class)->cursor_changed)
        GTK_TREE_VIEW_CLASS(_moo_ctags_view_parent_class)->cursor_changed (treeview);

    selection = gtk_tree_view_get_selection (treeview);
    if (!gtk_tree_selection_get_selected (selection, &model, &iter))
        return;

    gtk_tree_model_get (model, &iter, MOO_CTAGS_VIEW_COLUMN_ENTRY, &entry, -1);

    if (entry)
    {
        g_signal_emit_by_name (treeview, "activate-entry", entry);
        _moo_ctags_entry_unref (entry);
    }

}

static void
moo_ctags_view_row_activated (GtkTreeView       *treeview,
                              GtkTreePath       *path,
                              GtkTreeViewColumn *column)
{
    if (GTK_TREE_VIEW_CLASS(_moo_ctags_view_parent_class)->row_activated)
        GTK_TREE_VIEW_CLASS(_moo_ctags_view_parent_class)->row_activated (treeview, path, column);

    if (gtk_tree_view_row_expanded (treeview, path))
        gtk_tree_view_collapse_row (treeview, path);
    else
        gtk_tree_view_expand_row (treeview, path, FALSE);
}

static void
_moo_ctags_view_class_init (MooCtagsViewClass *klass)
{
    GtkTreeViewClass *treeview_class = GTK_TREE_VIEW_CLASS (klass);

    treeview_class->cursor_changed = moo_ctags_view_cursor_changed;
    treeview_class->row_activated = moo_ctags_view_row_activated;

    g_signal_new ("activate-entry",
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_LAST,
                  0, NULL, NULL,
                  _moo_marshal_VOID__BOXED,
                  G_TYPE_NONE, 1,
                  MOO_TYPE_CTAGS_ENTRY);

    g_type_class_add_private (klass, sizeof (MooCtagsViewPrivate));
}


static void
data_func (G_GNUC_UNUSED GtkTreeViewColumn *column,
           GtkCellRenderer   *cell,
           GtkTreeModel      *model,
           GtkTreeIter       *iter)
{
    MooCtagsEntry *entry = NULL;
    char *label = NULL;

    gtk_tree_model_get (model, iter,
                        MOO_CTAGS_VIEW_COLUMN_ENTRY, &entry,
                        MOO_CTAGS_VIEW_COLUMN_LABEL, &label,
                        -1);

    if (label)
    {
        g_object_set (cell, "markup", label, NULL);
    }
    else if (entry)
    {
        char *markup;
        /*if (entry->signature)
            markup = g_strdup_printf ("%s %s", entry->name, entry->signature);
        else
            */markup = g_strdup (entry->name);
        g_object_set (cell, "markup", markup, NULL);
        g_free (markup);
    }
    else
    {
        g_object_set (cell, "markup", NULL, NULL);
    }

    g_free (label);
    _moo_ctags_entry_unref (entry);
}

static void
_moo_ctags_view_init (MooCtagsView *view)
{
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;

    view->priv = G_TYPE_INSTANCE_GET_PRIVATE (view, MOO_TYPE_CTAGS_VIEW,
                                              MooCtagsViewPrivate);

    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (view), FALSE);

#if GTK_CHECK_VERSION(2, 12, 0)
    g_object_set (view,
                  "show-expanders", FALSE,
                  "level-indentation", 6,
                  NULL);
#endif

    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (NULL, cell, NULL);
    gtk_tree_view_column_set_cell_data_func (column, cell,
                                             (GtkTreeCellDataFunc) data_func,
                                             NULL, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (view), column);
}


GtkWidget *
_moo_ctags_view_new (void)
{
    return g_object_new (MOO_TYPE_CTAGS_VIEW, NULL);
}
