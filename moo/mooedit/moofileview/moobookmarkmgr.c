/*
 *   moobookmarkmgr.c
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

#include "moobookmarkmgr.h"
#include "moofileentry.h"
#include MOO_MARSHALS_H
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <glib/gstdio.h>
#include <glade/glade.h>


#define COLUMN_BOOKMARK MOO_BOOKMARK_MGR_COLUMN_BOOKMARK

struct _MooBookmarkMgrPrivate {
    GtkListStore *store;
    GSList *menus;
    guint update_idle;
    GtkWidget *editor;
};


static void moo_bookmark_mgr_finalize   (GObject        *object);
static void moo_bookmark_mgr_changed    (MooBookmarkMgr *mgr);
static void emit_changed                (MooBookmarkMgr *mgr);
static void moo_bookmark_mgr_update_menu(GtkMenuShell   *menu,
                                         MooBookmarkMgr *mgr);


/* MOO_TYPE_BOOKMARK_MGR */
G_DEFINE_TYPE (MooBookmarkMgr, moo_bookmark_mgr, G_TYPE_OBJECT)

enum {
    PROP_0,
};

enum {
    CHANGED,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

static void moo_bookmark_mgr_class_init (MooBookmarkMgrClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = moo_bookmark_mgr_finalize;

    klass->changed = moo_bookmark_mgr_changed;

    signals[CHANGED] =
            g_signal_new ("changed",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooBookmarkMgrClass, changed),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE,0);
}


static void moo_bookmark_mgr_init      (MooBookmarkMgr *mgr)
{
    mgr->priv = g_new0 (MooBookmarkMgrPrivate, 1);

    mgr->priv->store = gtk_list_store_new (1, MOO_TYPE_BOOKMARK);

    g_signal_connect_swapped (mgr->priv->store, "row-changed",
                              G_CALLBACK (emit_changed), mgr);
    g_signal_connect_swapped (mgr->priv->store, "rows-reordered",
                              G_CALLBACK (emit_changed), mgr);
    g_signal_connect_swapped (mgr->priv->store, "row-inserted",
                              G_CALLBACK (emit_changed), mgr);
    g_signal_connect_swapped (mgr->priv->store, "row-deleted",
                              G_CALLBACK (emit_changed), mgr);
}


static void moo_bookmark_mgr_finalize  (GObject      *object)
{
    MooBookmarkMgr *mgr = MOO_BOOKMARK_MGR (object);

    g_object_unref (mgr->priv->store);

    if (mgr->priv->update_idle)
        g_source_remove (mgr->priv->update_idle);

    if (mgr->priv->editor)
    {
        gtk_widget_destroy (mgr->priv->editor);
        g_object_unref (mgr->priv->editor);
    }

    g_free (mgr->priv);
    mgr->priv = NULL;

    G_OBJECT_CLASS (moo_bookmark_mgr_parent_class)->finalize (object);
}


static void emit_changed                (MooBookmarkMgr *mgr)
{
    g_signal_emit (mgr, signals[CHANGED], 0);
}


static gboolean real_update             (MooBookmarkMgr *mgr)
{
    g_slist_foreach (mgr->priv->menus,
                     (GFunc) moo_bookmark_mgr_update_menu,
                     mgr);

    mgr->priv->update_idle = 0;
    return FALSE;
}

static void moo_bookmark_mgr_changed    (MooBookmarkMgr *mgr)
{
    if (!mgr->priv->update_idle)
        mgr->priv->update_idle =
                g_idle_add ((GSourceFunc) real_update, mgr);
}


void            moo_bookmark_mgr_add        (MooBookmarkMgr *mgr,
                                             MooBookmark    *bookmark)
{
    GtkTreeIter iter;

    g_return_if_fail (MOO_IS_BOOKMARK_MGR (mgr));
    g_return_if_fail (bookmark != NULL);

    gtk_list_store_append (mgr->priv->store, &iter);
    gtk_list_store_set (mgr->priv->store, &iter,
                        COLUMN_BOOKMARK, bookmark, -1);
}


MooBookmarkMgr *moo_bookmark_mgr_new        (void)
{
    return g_object_new (MOO_TYPE_BOOKMARK_MGR, NULL);
}


MooBookmark    *moo_bookmark_new            (const char     *label,
                                             const char     *path,
                                             const char     *icon)
{
    MooBookmark *bookmark;

    bookmark = g_new0 (MooBookmark, 1);

    bookmark->path = g_strdup (path);
    bookmark->display_path = path ? g_filename_display_name (path) : NULL;
    bookmark->label = g_strdup (label);
    bookmark->icon_stock_id = g_strdup (icon);

    return bookmark;
}


MooBookmark    *moo_bookmark_copy           (MooBookmark    *bookmark)
{
    MooBookmark *copy;

    g_return_val_if_fail (bookmark != NULL, NULL);

    copy = g_new0 (MooBookmark, 1);

    copy->path = g_strdup (bookmark->path);
    copy->display_path = g_strdup (bookmark->display_path);
    copy->description = g_strdup (bookmark->description);
    copy->label = g_strdup (bookmark->label);
    copy->icon_stock_id = g_strdup (bookmark->icon_stock_id);
    if (bookmark->pixbuf)
        copy->pixbuf = g_object_ref (bookmark->pixbuf);

    return copy;
}


void            moo_bookmark_free           (MooBookmark    *bookmark)
{
    if (bookmark)
    {
        g_free (bookmark->path);
        g_free (bookmark->display_path);
        g_free (bookmark->description);
        g_free (bookmark->label);
        g_free (bookmark->icon_stock_id);
        if (bookmark->pixbuf)
            g_object_unref (bookmark->pixbuf);
        g_free (bookmark);
    }
}


GType           moo_bookmark_get_type       (void)
{
    static GType type = 0;
    if (!type)
        type = g_boxed_type_register_static ("MooBookmark",
                                             (GBoxedCopyFunc) moo_bookmark_copy,
                                             (GBoxedFreeFunc) moo_bookmark_free);
    return type;
}


void            moo_bookmark_set_path       (MooBookmark    *bookmark,
                                             const char     *path)
{
    char *display_path;
    g_return_if_fail (bookmark != NULL);
    g_return_if_fail (path != NULL);

    display_path = g_filename_to_utf8 (path, -1, NULL, NULL, NULL);
    g_return_if_fail (display_path != NULL);

    g_free (bookmark->path);
    g_free (bookmark->display_path);
    bookmark->display_path = display_path;
    bookmark->path = g_strdup (path);
}


void            moo_bookmark_set_display_path (MooBookmark  *bookmark,
                                               const char   *display_path)
{
    char *path;
    g_return_if_fail (bookmark != NULL);
    g_return_if_fail (display_path != NULL);

    path = g_filename_from_utf8 (display_path, -1, NULL, NULL, NULL);
    g_return_if_fail (path != NULL);

    g_free (bookmark->path);
    g_free (bookmark->display_path);
    bookmark->path = path;
    bookmark->display_path = g_strdup (display_path);
}


/***************************************************************************/
/* Bookmarks menu
 */

static void     menu_item_activated         (GtkMenuItem    *item,
                                             MooBookmarkMgr *mgr)
{
    MooBookmark *bookmark;
    MooBookmarkFunc func;
    gpointer data;

    g_return_if_fail (g_object_get_data (G_OBJECT (item), "moo-bookmark-mgr") == mgr);

    bookmark = g_object_get_data (G_OBJECT (item), "moo-bookmark");
    g_return_if_fail (bookmark != NULL);

    func = g_object_get_data (G_OBJECT (item), "moo-bookmark-func");
    data = g_object_get_data (G_OBJECT (item), "moo-bookmark-data");

    if (func)
        func (bookmark, data);
}


static void     create_menu_items           (MooBookmarkMgr *mgr,
                                             GtkMenuShell   *menu,
                                             int             position,
                                             MooBookmarkFunc func,
                                             gpointer        data)
{
    GtkTreeIter iter;
    GtkTreeModel *model = GTK_TREE_MODEL (mgr->priv->store);

    if (!gtk_tree_model_get_iter_first (model, &iter))
        return;

    do
    {
        MooBookmark *bookmark = NULL;
        GtkWidget *item, *icon;

        gtk_tree_model_get (model, &iter, COLUMN_BOOKMARK, &bookmark, -1);

        if (!bookmark)
        {
            item = gtk_separator_menu_item_new ();
        }
        else
        {
            if (bookmark->label)
                item = gtk_image_menu_item_new_with_label (bookmark->label);
            else
                item = gtk_image_menu_item_new ();

            if (bookmark->pixbuf)
                icon = gtk_image_new_from_pixbuf (bookmark->pixbuf);
            else if (bookmark->icon_stock_id)
                icon = gtk_image_new_from_stock (bookmark->icon_stock_id,
                    GTK_ICON_SIZE_MENU);
            else
                icon = NULL;

            if (icon)
            {
                gtk_widget_show (icon);
                gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item), icon);
            }

            g_signal_connect (item, "activate",
                              G_CALLBACK (menu_item_activated), mgr);
        }

        g_object_set_data_full (G_OBJECT (item), "moo-bookmark-mgr",
                                g_object_ref (mgr), g_object_unref);
        g_object_set_data_full (G_OBJECT (item), "moo-bookmark",
                                bookmark, (GDestroyNotify) moo_bookmark_free);
        g_object_set_data (G_OBJECT (item), "moo-bookmark-func", func);
        g_object_set_data (G_OBJECT (item), "moo-bookmark-data", data);

        gtk_widget_show (item);
        gtk_menu_shell_insert (menu, item, position);
        if (position >= 0)
            position++;
    }
    while (gtk_tree_model_iter_next (model, &iter));
}


static void     menu_destroyed              (MooBookmarkMgr *mgr,
                                             gpointer        menu)
{
    g_slist_remove (mgr->priv->menus, menu);
}

void            moo_bookmark_mgr_make_menu  (MooBookmarkMgr *mgr,
                                             GtkMenuShell   *menu,
                                             int             position,
                                             MooBookmarkFunc func,
                                             gpointer        data)
{
    g_return_if_fail (MOO_IS_BOOKMARK_MGR (mgr));
    g_return_if_fail (GTK_IS_MENU_SHELL (menu));

    g_return_if_fail (g_slist_find (mgr->priv->menus, menu) == NULL);
    g_return_if_fail (g_object_get_data (G_OBJECT (menu), "moo-bookmark-mgr") == NULL);

    g_object_set_data_full (G_OBJECT (menu), "moo-bookmark-mgr",
                            g_object_ref (mgr), g_object_unref);
    g_object_set_data (G_OBJECT (menu), "moo-bookmark-func", func);
    g_object_set_data (G_OBJECT (menu), "moo-bookmark-data", data);

    g_object_weak_ref (G_OBJECT (menu), (GWeakNotify) menu_destroyed, mgr);
    mgr->priv->menus = g_slist_prepend (mgr->priv->menus, menu);

    create_menu_items (mgr, menu, position, func, data);
}


static void moo_bookmark_mgr_update_menu(GtkMenuShell   *menu,
                                         MooBookmarkMgr *mgr)
{
    MooBookmarkFunc func;
    gpointer data;
    GList *items;
    int position = 0;

    g_return_if_fail (MOO_IS_BOOKMARK_MGR (mgr));
    g_return_if_fail (GTK_IS_MENU_SHELL (menu));

    g_return_if_fail (g_object_get_data (G_OBJECT (menu), "moo-bookmark-mgr") == mgr);

    func = g_object_get_data (G_OBJECT (menu), "moo-bookmark-func");
    data = g_object_get_data (G_OBJECT (menu), "moo-bookmark-data");

    items = gtk_container_get_children (GTK_CONTAINER (menu));

    if (items)
    {
        GList *l;
        GList *my_items = NULL;

        for (l = items; l != NULL; l = l->next)
        {
            if (g_object_get_data (G_OBJECT (l->data), "moo-bookmark-mgr") == mgr)
            {
                my_items = g_list_prepend (my_items, l->data);
                if (position < 0)
                    position = g_list_position (items, l);
            }
        }

        for (l = my_items; l != NULL; l = l->next)
            gtk_container_remove (GTK_CONTAINER (menu), l->data);

        g_list_free (items);
        g_list_free (my_items);
    }

    create_menu_items (mgr, menu, position, func, data);
}


/***************************************************************************/
/* Loading and saving
 */
#ifndef __MOO__

gboolean        moo_bookmark_mgr_load       (MooBookmarkMgr *mgr,
                                             const char     *file,
                                             gboolean        add,
                                             GError       **error)
{
    GKeyFile *key_file;
    char **groups, **p;

    g_return_val_if_fail (MOO_IS_BOOKMARK_MGR (mgr), FALSE);
    g_return_val_if_fail (file != NULL, FALSE);

    key_file = g_key_file_new ();

    if (!g_key_file_load_from_file (key_file, file, 0, error))
    {
        g_key_file_free (key_file);
        return FALSE;
    }

    if (!add)
        gtk_list_store_clear (mgr->priv->store);

    groups = g_key_file_get_groups (key_file, NULL);

    for (p = groups; *p != NULL; p++)
    {
        GtkTreeIter iter;
        MooBookmark *bookmark;
        char *group = *p;
        char *val;

        if (!strncmp (group, "Separator", strlen ("Separator")))
        {
            gtk_list_store_append (mgr->priv->store, &iter);
            continue;
        }
        else if (strncmp (group, "Bookmark", strlen ("Bookmark")))
        {
            g_warning ("%s: invalid group name %s", G_STRLOC, group);
        }

        bookmark = moo_bookmark_new (NULL, NULL, NULL);

        val = g_key_file_get_value (key_file, group, "path", NULL);
        if (val && val[0])
        {
            char *path = g_filename_from_utf8 (val, -1, NULL, NULL, NULL);

            if (!path)
            {
                g_warning ("%s: invalid path %s", G_STRLOC, val);
                moo_bookmark_free (bookmark);
                g_free (val);
                continue;
            }

            g_free (bookmark->path);
            g_free (bookmark->display_path);
            bookmark->path = path;
            bookmark->display_path = val;
        }
        else
        {
            g_free (val);
        }

        val = g_key_file_get_value (key_file, group, "description", NULL);
        if (val && val[0])
        {
            g_free (bookmark->description);
            bookmark->description = val;
        }
        else
        {
            g_free (val);
        }

        val = g_key_file_get_value (key_file, group, "label", NULL);
        if (val && val[0])
        {
            g_free (bookmark->label);
            bookmark->label = val;
        }
        else
        {
            g_free (val);
        }

        val = g_key_file_get_value (key_file, group, "icon", NULL);
        if (val && val[0])
        {
            g_free (bookmark->icon_stock_id);
            bookmark->icon_stock_id = val;
        }
        else
        {
            g_free (val);
        }

        gtk_list_store_append (mgr->priv->store, &iter);
        gtk_list_store_set (mgr->priv->store, &iter,
                            COLUMN_BOOKMARK, bookmark, -1);
        moo_bookmark_free (bookmark);
    }

    g_strfreev (groups);
    g_key_file_free (key_file);
    return TRUE;
}


gboolean        moo_bookmark_mgr_save       (MooBookmarkMgr *mgr,
                                             const char     *file,
                                             GError       **error)
{
    guint bk_count = 0, sep_count = 0;
    GKeyFile *key_file;
    GtkTreeIter iter;
    GtkTreeModel *model;
    GIOStatus status;
    GIOChannel *io_channel;
    char *content;

    g_return_val_if_fail (MOO_IS_BOOKMARK_MGR (mgr), FALSE);
    g_return_val_if_fail (file != NULL, FALSE);

    model = GTK_TREE_MODEL (mgr->priv->store);
    if (!gtk_tree_model_get_iter_first (model, &iter))
    {
        if (g_unlink (file))
        {
            int saved_errno = errno;
            char *display_name = g_filename_display_name (file);
            g_set_error (error, G_FILE_ERROR,
                         g_file_error_from_errno (errno),
                         "could not delete file %s: %s",
                         display_name, g_strerror (saved_errno));
            g_free (display_name);
            return FALSE;
        }
        else
        {
            return TRUE;
        }
    }

    key_file = g_key_file_new ();

    do
    {
        MooBookmark *bookmark = NULL;
        char *group;

        gtk_tree_model_get (model, &iter, COLUMN_BOOKMARK, &bookmark, -1);

        if (!bookmark)
        {
            if (!sep_count)
                group = g_strdup ("Separator");
            else
                group = g_strdup_printf ("Separator %d", sep_count);
            ++sep_count;

            g_key_file_set_value (key_file, group, "dummy", "dummy");
            g_free (group);
            continue;
        }

        if (!bk_count)
            group = g_strdup ("Bookmark");
        else
            group = g_strdup_printf ("Bookmark %d", bk_count);
        ++bk_count;

        g_key_file_set_value (key_file, group, "label",
                              bookmark->label ? bookmark->label : "");
        g_key_file_set_value (key_file, group, "path",
                              bookmark->display_path ? bookmark->display_path : "");
        g_key_file_set_value (key_file, group, "description",
                              bookmark->description ? bookmark->description : "");
        g_key_file_set_value (key_file, group, "icon",
                              bookmark->icon_stock_id ? bookmark->icon_stock_id : "");

        g_free (group);
        moo_bookmark_free (bookmark);
    }
    while (gtk_tree_model_iter_next (model, &iter));

    content = g_key_file_to_data (key_file, NULL, error);
    if (!content)
    {
        g_key_file_free (key_file);
        return FALSE;
    }

    g_key_file_free (key_file);

    io_channel = g_io_channel_new_file (file, "w", error);
    if (!io_channel)
    {
        g_free (content);
        return FALSE;
    }

    status = g_io_channel_write_chars (io_channel, content, -1, NULL, error);
    g_free (content);
    g_io_channel_shutdown (io_channel, TRUE, NULL);
    g_io_channel_unref (io_channel);

    return status == G_IO_STATUS_NORMAL;
}
#endif


/***************************************************************************/
/* Bookmark editor
 */

#ifndef MOO_BOOKMARK_EDITOR_GLADE_FILE
#define MOO_BOOKMARK_EDITOR_GLADE_FILE "bookmark_editor.glade"
#endif

static GtkTreeModel *copy_bookmarks         (GtkListStore   *store);
static void          copy_bookmarks_back    (GtkListStore   *store,
                                             GtkTreeModel   *model);
static void          init_editor_dialog     (GtkTreeView    *treeview,
                                             GladeXML       *xml);
static void          dialog_response        (GtkWidget      *dialog,
                                             int             response,
                                             MooBookmarkMgr *mgr);
static void          dialog_show            (GtkWidget      *dialog,
                                             MooBookmarkMgr *mgr);

GtkWidget      *moo_bookmark_mgr_get_editor (MooBookmarkMgr *mgr)
{
    GtkWidget *dialog;
    GladeXML *xml;
    GtkTreeView *treeview;

    if (mgr->priv->editor)
        return mgr->priv->editor;

    xml = glade_xml_new (MOO_BOOKMARK_EDITOR_GLADE_FILE, NULL, NULL);

    if (!xml)
        g_error ("Yes, glade is great usually, but not always");

    dialog = glade_xml_get_widget (xml, "dialog");
    g_assert (dialog != NULL);

    g_object_set_data_full (G_OBJECT (dialog), "dialog-glade-xml",
                            xml, g_object_unref);

    gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

    treeview = GTK_TREE_VIEW (glade_xml_get_widget (xml, "treeview"));
    init_editor_dialog (treeview, xml);

    g_signal_connect (dialog, "response",
                      G_CALLBACK (dialog_response), mgr);
    g_signal_connect (dialog, "delete-event",
                      G_CALLBACK (gtk_widget_hide_on_delete), NULL);
    g_signal_connect (dialog, "show",
                      G_CALLBACK (dialog_show), mgr);

    mgr->priv->editor = dialog;
    gtk_object_sink (GTK_OBJECT (g_object_ref (dialog)));

    return dialog;
}


static void          dialog_show            (GtkWidget      *dialog,
                                             MooBookmarkMgr *mgr)
{
    GladeXML *xml;
    GtkTreeView *treeview;
    GtkTreeModel *model;

    xml = g_object_get_data (G_OBJECT (dialog), "dialog-glade-xml");
    g_return_if_fail (xml != NULL);

    model = copy_bookmarks (mgr->priv->store);
    treeview = GTK_TREE_VIEW (glade_xml_get_widget (xml, "treeview"));
    gtk_tree_view_set_model (treeview, model);
    g_object_unref (model);
}


static void          dialog_response        (GtkWidget      *dialog,
                                             int             response,
                                             MooBookmarkMgr *mgr)
{
    GladeXML *xml;
    GtkTreeView *treeview;
    GtkTreeModel *model;

    if (response != GTK_RESPONSE_OK)
    {
        gtk_widget_hide (dialog);
        return;
    }

    xml = g_object_get_data (G_OBJECT (dialog), "dialog-glade-xml");
    g_return_if_fail (xml != NULL);

    treeview = GTK_TREE_VIEW (glade_xml_get_widget (xml, "treeview"));
    model = gtk_tree_view_get_model (treeview);
    copy_bookmarks_back (mgr->priv->store, model);
    gtk_widget_hide (dialog);
}


static gboolean copy_value (GtkTreeModel    *src,
                            G_GNUC_UNUSED GtkTreePath *path,
                            GtkTreeIter     *iter,
                            GtkListStore    *dest)
{
    GtkTreeIter dest_iter;
    MooBookmark *bookmark;

    gtk_tree_model_get (src, iter, COLUMN_BOOKMARK, &bookmark, -1);
    gtk_list_store_append (dest, &dest_iter);
    gtk_list_store_set (dest, &dest_iter, COLUMN_BOOKMARK, bookmark, -1);
    moo_bookmark_free (bookmark);

    return FALSE;
}

static GtkTreeModel *copy_bookmarks         (GtkListStore   *store)
{
    GtkListStore *copy;
    copy = gtk_list_store_new (1, MOO_TYPE_BOOKMARK);
    gtk_tree_model_foreach (GTK_TREE_MODEL (store),
                            (GtkTreeModelForeachFunc) copy_value,
                            copy);
    return GTK_TREE_MODEL (copy);
}


static void          copy_bookmarks_back    (GtkListStore   *store,
                                             GtkTreeModel   *model)
{
    gtk_list_store_clear (store);
    gtk_tree_model_foreach (model,
                            (GtkTreeModelForeachFunc) copy_value,
                            store);
}


static void     icon_data_func      (GtkTreeViewColumn  *column,
                                     GtkCellRenderer    *cell,
                                     GtkTreeModel       *model,
                                     GtkTreeIter        *iter);
static void     label_data_func     (GtkTreeViewColumn  *column,
                                     GtkCellRenderer    *cell,
                                     GtkTreeModel       *model,
                                     GtkTreeIter        *iter);
static void     path_data_func      (GtkTreeViewColumn  *column,
                                     GtkCellRenderer    *cell,
                                     GtkTreeModel       *model,
                                     GtkTreeIter        *iter);
// static gboolean separator_func      (GtkTreeModel       *model,
//                                      GtkTreeIter        *iter);

static void     selection_changed   (GtkTreeSelection   *selection,
                                     GladeXML           *xml);
static void     new_clicked         (GladeXML           *xml);
static void     delete_clicked      (GladeXML           *xml);
static void     separator_clicked   (GladeXML           *xml);

static void     label_edited        (GtkCellRenderer    *cell,
                                     char               *path,
                                     char               *text,
                                     GladeXML           *xml);
static void     path_edited         (GtkCellRenderer    *cell,
                                     char               *path,
                                     char               *text,
                                     GladeXML           *xml);
static void     path_editing_started(GtkCellRenderer    *cell,
                                     GtkCellEditable    *editable);
static void     path_entry_realize  (GtkWidget          *entry);
static void     path_entry_unrealize(GtkWidget          *entry);

static void     init_icon_combo     (GtkComboBox        *combo,
                                     GladeXML           *xml);
static void     combo_update_icon   (GtkComboBox        *combo,
                                     GladeXML           *xml);


static void          init_editor_dialog     (GtkTreeView    *treeview,
                                             GladeXML       *xml)
{
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    GtkTreeSelection *selection;
    GtkWidget *button, *icon_combo;
    MooFileEntryCompletion *completion;

    icon_combo = glade_xml_get_widget (xml, "icon_combo");
    init_icon_combo (GTK_COMBO_BOX (icon_combo), xml);

//     gtk_tree_view_set_row_separator_func (treeview,
//                                           (GtkTreeViewRowSeparatorFunc) separator_func,
//                                           NULL, NULL);

    selection = gtk_tree_view_get_selection (treeview);
    gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);
    g_signal_connect (selection, "changed",
                      G_CALLBACK (selection_changed), xml);
    selection_changed (selection, xml);

    button = glade_xml_get_widget (xml, "delete_button");
    g_signal_connect_swapped (button, "clicked",
                              G_CALLBACK (delete_clicked), xml);

    button = glade_xml_get_widget (xml, "new_button");
    g_signal_connect_swapped (button, "clicked",
                              G_CALLBACK (new_clicked), xml);

    button = glade_xml_get_widget (xml, "separator_button");
    g_signal_connect_swapped (button, "clicked",
                              G_CALLBACK (separator_clicked), xml);

    /* Icon */
    cell = gtk_cell_renderer_pixbuf_new ();
    column = gtk_tree_view_column_new_with_attributes ("Icon", cell, NULL);
    gtk_tree_view_column_set_cell_data_func (column, cell,
                                             (GtkTreeCellDataFunc) icon_data_func,
                                             NULL, NULL);
    gtk_tree_view_append_column (treeview, column);
    g_object_set_data (G_OBJECT (treeview),
                       "moo-bookmarks-icon-column",
                       column);

    /* Label */
    cell = gtk_cell_renderer_text_new ();
    g_object_set (cell, "editable", TRUE, NULL);
    g_signal_connect (cell, "edited",
                      G_CALLBACK (label_edited), xml);

    column = gtk_tree_view_column_new_with_attributes ("Label", cell, NULL);
    gtk_tree_view_column_set_cell_data_func (column, cell,
                                             (GtkTreeCellDataFunc) label_data_func,
                                             NULL, NULL);
    gtk_tree_view_append_column (treeview, column);
    g_object_set_data (G_OBJECT (treeview),
                       "moo-bookmarks-label-column",
                       column);

    /* Path */
    cell = gtk_cell_renderer_text_new ();
    g_object_set (cell, "editable", TRUE, NULL);
    g_signal_connect (cell, "edited",
                      G_CALLBACK (path_edited), xml);
    g_signal_connect (cell, "editing-started",
                      G_CALLBACK (path_editing_started), NULL);

    column = gtk_tree_view_column_new_with_attributes ("Path", cell, NULL);
    gtk_tree_view_column_set_cell_data_func (column, cell,
                                             (GtkTreeCellDataFunc) path_data_func,
                                             NULL, NULL);
    gtk_tree_view_append_column (treeview, column);
    g_object_set_data (G_OBJECT (treeview),
                       "moo-bookmarks-path-column",
                       column);

    completion = g_object_new (MOO_TYPE_FILE_ENTRY_COMPLETION,
                               "directories-only", TRUE,
                               "show-hidden", FALSE,
                               NULL);
    g_object_set_data_full (G_OBJECT (cell), "moo-file-entry-completion",
                            completion, g_object_unref);
}


static MooBookmark *get_bookmark(GtkTreeModel       *model,
                                 GtkTreeIter        *iter)
{
    MooBookmark *bookmark = NULL;
    gtk_tree_model_get (model, iter, COLUMN_BOOKMARK, &bookmark, -1);
    return bookmark;
}


static void     set_bookmark    (GtkListStore       *store,
                                 GtkTreeIter        *iter,
                                 MooBookmark        *bookmark)
{
    gtk_list_store_set (store, iter, COLUMN_BOOKMARK, bookmark, -1);
}


// static gboolean separator_func  (GtkTreeModel       *model,
//                                  GtkTreeIter        *iter)
// {
//     MooBookmark *bookmark = get_bookmark (model, iter);
//
//     if (bookmark)
//     {
//         moo_bookmark_free (bookmark);
//         return FALSE;
//     }
//     else
//     {
//         return TRUE;
//     }
// }


static void     icon_data_func  (G_GNUC_UNUSED GtkTreeViewColumn *column,
                                 GtkCellRenderer    *cell,
                                 GtkTreeModel       *model,
                                 GtkTreeIter        *iter)
{
    MooBookmark *bookmark = get_bookmark (model, iter);

    if (!bookmark)
        g_object_set (cell,
                      "pixbuf", NULL,
                      "stock-id", NULL,
                      NULL);
    else
        g_object_set (cell,
                      "pixbuf", bookmark->pixbuf,
                      "stock-id", bookmark->icon_stock_id,
                      "stock-size", GTK_ICON_SIZE_MENU,
                      NULL);

    moo_bookmark_free (bookmark);
}


static void     label_data_func (G_GNUC_UNUSED GtkTreeViewColumn *column,
                                 GtkCellRenderer    *cell,
                                 GtkTreeModel       *model,
                                 GtkTreeIter        *iter)
{
    MooBookmark *bookmark = get_bookmark (model, iter);

    if (!bookmark)
        g_object_set (cell,
                      "text", "-------",
                      "editable", FALSE,
                      NULL);
    else
        g_object_set (cell,
                      "text", bookmark->label,
                      "editable", TRUE,
                      NULL);

    moo_bookmark_free (bookmark);
}


static void     path_data_func  (G_GNUC_UNUSED GtkTreeViewColumn *column,
                                 GtkCellRenderer    *cell,
                                 GtkTreeModel       *model,
                                 GtkTreeIter        *iter)
{
    MooBookmark *bookmark = get_bookmark (model, iter);

    if (!bookmark)
        g_object_set (cell,
                      "text", NULL,
                      "editable", FALSE,
                      NULL);
    else
        g_object_set (cell,
                      "text", bookmark->display_path,
                      "editable", TRUE,
                      NULL);

    moo_bookmark_free (bookmark);
}


static void     selection_changed   (GtkTreeSelection   *selection,
                                     GladeXML           *xml)
{
    GtkWidget *button, *selected_hbox;
    int selected;

    button = glade_xml_get_widget (xml, "delete_button");
    selected_hbox = glade_xml_get_widget (xml, "selected_hbox");

    selected = gtk_tree_selection_count_selected_rows (selection);

    gtk_widget_set_sensitive (button, selected);

    if (selected == 1)
    {
        GtkTreeIter iter;
        GtkTreeModel *model;
        MooBookmark *bookmark;
        GList *rows = gtk_tree_selection_get_selected_rows (selection, &model);
        g_return_if_fail (rows != NULL);
        gtk_tree_model_get_iter (model, &iter, rows->data);
        bookmark = get_bookmark (model, &iter);
        if (bookmark)
        {
            GtkWidget *icon_combo;
            gtk_widget_set_sensitive (selected_hbox, TRUE);
            icon_combo = glade_xml_get_widget (xml, "icon_combo");
            combo_update_icon (GTK_COMBO_BOX (icon_combo), xml);
            moo_bookmark_free (bookmark);
        }
        else
        {
            gtk_widget_set_sensitive (selected_hbox, FALSE);
        }
    }
    else
    {
        gtk_widget_set_sensitive (selected_hbox, FALSE);
    }
}


static void     new_clicked         (GladeXML           *xml)
{
    GtkTreeIter iter;
    GtkTreePath *path;
    GtkWidget *treeview;
    GtkTreeViewColumn *column;
    GtkListStore *store;
    MooBookmark *bookmark;

    treeview = glade_xml_get_widget (xml, "treeview");
    store = GTK_LIST_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (treeview)));

    bookmark = moo_bookmark_new ("New bookmark", NULL,
                                 GTK_STOCK_DIRECTORY);
    gtk_list_store_append (store, &iter);
    set_bookmark (store, &iter, bookmark);

    column = g_object_get_data (G_OBJECT (treeview),
                                "moo-bookmarks-label-column");
    path = gtk_tree_model_get_path (GTK_TREE_MODEL (store), &iter);
    gtk_tree_view_set_cursor (GTK_TREE_VIEW (treeview), path, column, TRUE);

    g_object_set_data (G_OBJECT (store),
                       "moo-bookmarks-modified",
                       GINT_TO_POINTER (TRUE));

    gtk_tree_path_free (path);
    moo_bookmark_free (bookmark);
}


static void     delete_clicked      (GladeXML           *xml)
{
    GtkTreeIter iter;
    GtkTreePath *path;
    GtkWidget *treeview;
    GtkTreeSelection *selection;
    GtkListStore *store;
    GList *paths, *rows = NULL, *l;

    treeview = glade_xml_get_widget (xml, "treeview");
    store = GTK_LIST_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (treeview)));

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
    paths = gtk_tree_selection_get_selected_rows (selection, NULL);

    if (!paths)
        return;

    for (l = paths; l != NULL; l = l->next)
        rows = g_list_prepend (rows,
                               gtk_tree_row_reference_new (GTK_TREE_MODEL (store),
                                                           l->data));

    for (l = rows; l != NULL; l = l->next)
    {
        if (gtk_tree_row_reference_valid (l->data))
        {
            path = gtk_tree_row_reference_get_path (l->data);
            gtk_tree_model_get_iter (GTK_TREE_MODEL (store), &iter, path);
            gtk_list_store_remove (store, &iter);
            gtk_tree_path_free (path);
        }
    }

    g_object_set_data (G_OBJECT (store),
                       "moo-bookmarks-modified",
                       GINT_TO_POINTER (TRUE));

    g_list_foreach (paths, (GFunc) gtk_tree_path_free, NULL);
    g_list_foreach (rows, (GFunc) gtk_tree_row_reference_free, NULL);
    g_list_free (paths);
    g_list_free (rows);
}


static void     separator_clicked   (GladeXML           *xml)
{
    GtkTreeIter iter;
    GtkWidget *treeview;
    GtkListStore *store;

    treeview = glade_xml_get_widget (xml, "treeview");
    store = GTK_LIST_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (treeview)));
    gtk_list_store_append (store, &iter);
}


static void     label_edited        (G_GNUC_UNUSED GtkCellRenderer *cell,
                                     char               *path_string,
                                     char               *text,
                                     GladeXML           *xml)
{
    GtkTreeIter iter;
    GtkTreePath *path;
    GtkWidget *treeview;
    GtkListStore *store;
    MooBookmark *bookmark;

    treeview = glade_xml_get_widget (xml, "treeview");
    store = GTK_LIST_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (treeview)));

    path = gtk_tree_path_new_from_string (path_string);

    if (!gtk_tree_model_get_iter (GTK_TREE_MODEL (store), &iter, path))
    {
        gtk_tree_path_free (path);
        return;
    }

    bookmark = get_bookmark (GTK_TREE_MODEL (store), &iter);
    g_return_if_fail (bookmark != NULL);

    if (!bookmark->label || strcmp (bookmark->label, text))
    {
        g_free (bookmark->label);
        bookmark->label = g_strdup (text);
        set_bookmark (store, &iter, bookmark);
        g_object_set_data (G_OBJECT (store),
                           "moo-bookmarks-modified",
                           GINT_TO_POINTER (TRUE));
    }

    moo_bookmark_free (bookmark);
    gtk_tree_path_free (path);
}


static void     path_edited         (G_GNUC_UNUSED GtkCellRenderer *cell,
                                     char               *path_string,
                                     char               *text,
                                     GladeXML           *xml)
{
    GtkTreeIter iter;
    GtkTreePath *path;
    GtkWidget *treeview;
    GtkListStore *store;
    MooBookmark *bookmark;
    MooFileEntryCompletion *cmpl;

    treeview = glade_xml_get_widget (xml, "treeview");
    store = GTK_LIST_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (treeview)));

    path = gtk_tree_path_new_from_string (path_string);

    if (!gtk_tree_model_get_iter (GTK_TREE_MODEL (store), &iter, path))
    {
        gtk_tree_path_free (path);
        return;
    }

    bookmark = get_bookmark (GTK_TREE_MODEL (store), &iter);
    g_return_if_fail (bookmark != NULL);

    if (!bookmark->display_path || strcmp (bookmark->display_path, text))
    {
        moo_bookmark_set_display_path (bookmark, text);
        set_bookmark (store, &iter, bookmark);
        g_object_set_data (G_OBJECT (store),
                           "moo-bookmarks-modified",
                           GINT_TO_POINTER (TRUE));
    }

    moo_bookmark_free (bookmark);
    gtk_tree_path_free (path);

    cmpl = g_object_get_data (G_OBJECT (cell), "moo-file-entry-completion");
    g_return_if_fail (cmpl != NULL);
    g_object_set (cmpl, "entry", NULL, NULL);
}


static void     path_editing_started(GtkCellRenderer    *cell,
                                     GtkCellEditable    *editable)
{
    MooFileEntryCompletion *cmpl =
            g_object_get_data (G_OBJECT (cell), "moo-file-entry-completion");

    g_return_if_fail (cmpl != NULL);
    g_return_if_fail (GTK_IS_ENTRY (editable));

    g_object_set (cmpl, "entry", editable, NULL);

//     if (!g_object_get_data (G_OBJECT (editable), "moo-stupid-entry-workaround"))
//     {
//         g_signal_connect (editable, "realize",
//                           G_CALLBACK (path_entry_realize), NULL);
//         g_signal_connect (editable, "unrealize",
//                           G_CALLBACK (path_entry_unrealize), NULL);
//         g_object_set_data (G_OBJECT (editable), "moo-stupid-entry-workaround",
//                            GINT_TO_POINTER (TRUE));
//     }
}


static void     path_entry_realize  (GtkWidget          *entry)
{
    GtkSettings *settings;
    gboolean value;

    g_return_if_fail (gtk_widget_has_screen (entry));
    settings = gtk_settings_get_for_screen (gtk_widget_get_screen (entry));
    g_return_if_fail (settings != NULL);

    g_object_get (settings, "gtk-entry-select-on-focus", &value, NULL);
    g_object_set (settings, "gtk-entry-select-on-focus", FALSE, NULL);
    g_object_set_data (G_OBJECT (settings),
                       "moo-stupid-entry-workaround",
                       GINT_TO_POINTER (TRUE));
    g_object_set_data (G_OBJECT (settings),
                       "moo-stupid-entry-workaround-value",
                       GINT_TO_POINTER (value));

    gtk_editable_set_position (GTK_EDITABLE (entry), -1);
}


static void     path_entry_unrealize(GtkWidget          *entry)
{
    GtkSettings *settings;
    gboolean value;

    g_return_if_fail (gtk_widget_has_screen (entry));
    settings = gtk_settings_get_for_screen (gtk_widget_get_screen (entry));
    g_return_if_fail (settings != NULL);

    if (g_object_get_data (G_OBJECT (settings), "moo-stupid-entry-workaround"))
    {
        value = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (settings),
                                 "moo-stupid-entry-workaround-value"));
        g_object_set (settings, "gtk-entry-select-on-focus", value, NULL);
        g_object_set_data (G_OBJECT (settings), "moo-stupid-entry-workaround", NULL);
    }

    g_signal_handlers_disconnect_by_func (entry,
                                          (gpointer) path_entry_realize,
                                          NULL);
    g_signal_handlers_disconnect_by_func (entry,
                                          (gpointer) path_entry_unrealize,
                                          NULL);
    g_object_set_data (G_OBJECT (entry), "moo-stupid-entry-workaround", NULL);
}


static void combo_icon_data_func    (GtkCellLayout      *cell_layout,
                                     GtkCellRenderer    *cell,
                                     GtkTreeModel       *model,
                                     GtkTreeIter        *iter);
static void combo_label_data_func   (GtkCellLayout      *cell_layout,
                                     GtkCellRenderer    *cell,
                                     GtkTreeModel       *model,
                                     GtkTreeIter        *iter);
static void fill_icon_store         (GtkListStore       *store,
                                     GtkStyle           *style);
static void icon_store_find_pixbuf  (GtkListStore       *store,
                                     GtkTreeIter        *iter,
                                     GdkPixbuf          *pixbuf);
static void icon_store_find_stock   (GtkListStore       *store,
                                     GtkTreeIter        *iter,
                                     const char         *stock);
static void icon_store_find_empty   (GtkListStore       *store,
                                     GtkTreeIter        *iter);
static void icon_combo_changed      (GtkComboBox        *combo,
                                     GladeXML           *xml);

static void     init_icon_combo     (GtkComboBox        *combo,
                                     GladeXML           *xml)
{
    static GtkListStore *icon_store;
    GtkCellRenderer *cell;

    if (!icon_store)
    {
        GtkWidget *dialog = glade_xml_get_widget (xml, "dialog");
        gtk_widget_ensure_style (dialog);
        icon_store = gtk_list_store_new (3, GDK_TYPE_PIXBUF,
                                         G_TYPE_STRING, G_TYPE_STRING);
        fill_icon_store (icon_store, dialog->style);
    }

    gtk_cell_layout_clear (GTK_CELL_LAYOUT (combo));
    gtk_combo_box_set_model (combo, GTK_TREE_MODEL (icon_store));
    gtk_combo_box_set_wrap_width (combo, 3);

    cell = gtk_cell_renderer_pixbuf_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), cell, FALSE);
    gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (combo), cell,
                                        (GtkCellLayoutDataFunc) combo_icon_data_func,
                                        NULL, NULL);

    cell = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), cell, FALSE);
    gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (combo), cell,
                                        (GtkCellLayoutDataFunc) combo_label_data_func,
                                        NULL, NULL);

    g_signal_connect (combo, "changed",
                      G_CALLBACK (icon_combo_changed), xml);
}


static void     combo_update_icon   (GtkComboBox        *combo,
                                     GladeXML           *xml)
{
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkWidget *treeview;
    GList *rows;
    MooBookmark *bookmark;
    GtkListStore *icon_store;

    treeview = glade_xml_get_widget (xml, "treeview");
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
    rows = gtk_tree_selection_get_selected_rows (selection, &model);
    g_return_if_fail (rows != NULL && rows->next == NULL);

    gtk_tree_model_get_iter (model, &iter, rows->data);
    bookmark = get_bookmark (model, &iter);
    g_return_if_fail (bookmark != NULL);

    icon_store = GTK_LIST_STORE (gtk_combo_box_get_model (combo));

    if (bookmark->pixbuf)
        icon_store_find_pixbuf (icon_store, &iter, bookmark->pixbuf);
    else if (bookmark->icon_stock_id)
        icon_store_find_stock (icon_store, &iter, bookmark->icon_stock_id);
    else
        icon_store_find_empty (icon_store, &iter);

    g_signal_handlers_block_by_func (combo, (gpointer) icon_combo_changed, xml);
    gtk_combo_box_set_active_iter (combo, &iter);
    g_signal_handlers_unblock_by_func (combo, (gpointer) icon_combo_changed, xml);

    moo_bookmark_free (bookmark);
    gtk_tree_path_free (rows->data);
    g_list_free (rows);
}


enum {
    ICON_COLUMN_PIXBUF = 0,
    ICON_COLUMN_STOCK  = 1,
    ICON_COLUMN_LABEL  = 2
};

static void combo_icon_data_func    (G_GNUC_UNUSED GtkCellLayout *cell_layout,
                                     GtkCellRenderer    *cell,
                                     GtkTreeModel       *model,
                                     GtkTreeIter        *iter)
{
    char *stock = NULL;
    GdkPixbuf *pixbuf = NULL;

    gtk_tree_model_get (model, iter, ICON_COLUMN_PIXBUF, &pixbuf, -1);
    g_object_set (cell, "pixbuf", pixbuf, NULL);
    if (pixbuf)
    {
        g_object_unref (pixbuf);
        return;
    }

    gtk_tree_model_get (model, iter, ICON_COLUMN_STOCK, &stock, -1);
    g_object_set (cell, "stock-id", stock,
                  "stock-size", GTK_ICON_SIZE_MENU, NULL);
    g_free (stock);
}


static void combo_label_data_func   (G_GNUC_UNUSED GtkCellLayout *cell_layout,
                                     GtkCellRenderer    *cell,
                                     GtkTreeModel       *model,
                                     GtkTreeIter        *iter)
{
    char *label = NULL;
    gtk_tree_model_get (model, iter, ICON_COLUMN_LABEL, &label, -1);
    g_object_set (cell, "text", label, NULL);
    g_free (label);
}


static void fill_icon_store         (GtkListStore       *store,
                                     GtkStyle           *style)
{
    GtkTreeIter iter;
    GSList *stock_ids, *l;

    stock_ids = gtk_stock_list_ids ();

    for (l = stock_ids; l != NULL; l = l->next)
    {
        GtkStockItem item;

        if (!gtk_style_lookup_icon_set (style, l->data))
            continue;

        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter, ICON_COLUMN_STOCK,
                            l->data, -1);

        if (gtk_stock_lookup (l->data, &item))
        {
            char *label = g_strdup (item.label);
            char *und = strchr (label, '_');

            if (und)
            {
                if (und[1] == 0)
                    *und = 0;
                else
                    memmove (und, und + 1, strlen (label) - (und - label));
            }

            gtk_list_store_set (store, &iter, ICON_COLUMN_LABEL,
                                label, -1);
            g_free (label);
        }
        else
        {
            gtk_list_store_set (store, &iter, ICON_COLUMN_LABEL,
                                l->data, -1);
        }
    }

    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter, ICON_COLUMN_LABEL, "None", -1);

    g_slist_foreach (stock_ids, (GFunc) g_free, NULL);
    g_slist_free (stock_ids);
}


static void icon_combo_changed      (GtkComboBox        *combo,
                                     GladeXML           *xml)
{
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter, icon_iter;
    GtkWidget *treeview;
    GList *rows;
    MooBookmark *bookmark;
    GtkTreeModel *icon_model;
    GdkPixbuf *pixbuf = NULL;
    char *stock = NULL;

    treeview = glade_xml_get_widget (xml, "treeview");
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
    rows = gtk_tree_selection_get_selected_rows (selection, &model);
    g_return_if_fail (rows != NULL && rows->next == NULL);

    gtk_tree_model_get_iter (model, &iter, rows->data);
    bookmark = get_bookmark (model, &iter);
    g_return_if_fail (bookmark != NULL);

    gtk_combo_box_get_active_iter (combo, &icon_iter);
    icon_model = gtk_combo_box_get_model (combo);

    if (bookmark->pixbuf)
        g_object_unref (bookmark->pixbuf);
    bookmark->pixbuf = NULL;
    g_free (bookmark->icon_stock_id);
    bookmark->icon_stock_id = NULL;

    gtk_tree_model_get (icon_model, &icon_iter, ICON_COLUMN_PIXBUF,
                        &pixbuf, -1);

    if (pixbuf)
        bookmark->pixbuf = pixbuf;

    if (!pixbuf)
    {
        gtk_tree_model_get (icon_model, &icon_iter, ICON_COLUMN_STOCK,
                            &stock, -1);
        bookmark->icon_stock_id = stock;
    }

    set_bookmark (GTK_LIST_STORE (model), &iter, bookmark);

    moo_bookmark_free (bookmark);
    gtk_tree_path_free (rows->data);
    g_list_free (rows);
}


static void icon_store_find_pixbuf  (GtkListStore       *store,
                                     GtkTreeIter        *iter,
                                     GdkPixbuf          *pixbuf)
{
    GtkTreeModel *model = GTK_TREE_MODEL (store);

    g_return_if_fail (GDK_IS_PIXBUF (pixbuf));

    if (gtk_tree_model_get_iter_first (model, iter)) do
    {
        GdkPixbuf *pix = NULL;
        gtk_tree_model_get (model, iter, ICON_COLUMN_PIXBUF, &pix, -1);

        if (pix == pixbuf)
        {
            if (pix)
                g_object_unref (pix);
            return;
        }

        if (pix)
            g_object_unref (pix);
    }
    while (gtk_tree_model_iter_next (model, iter));

    gtk_list_store_append (store, iter);
    gtk_list_store_set (store, iter, ICON_COLUMN_PIXBUF, pixbuf, -1);
}


static void icon_store_find_stock   (GtkListStore       *store,
                                     GtkTreeIter        *iter,
                                     const char         *stock)
{
    GtkTreeModel *model = GTK_TREE_MODEL (store);

    g_return_if_fail (stock != NULL);

    if (gtk_tree_model_get_iter_first (model, iter)) do
    {
        char *id = NULL;
        gtk_tree_model_get (model, iter, ICON_COLUMN_STOCK, &id, -1);

        if (id && !strcmp (id, stock))
        {
            g_free (id);
            return;
        }

        g_free (id);
    }
    while (gtk_tree_model_iter_next (model, iter));

    gtk_list_store_append (store, iter);
    gtk_list_store_set (store, iter, ICON_COLUMN_STOCK, stock, -1);
}


static void icon_store_find_empty   (GtkListStore       *store,
                                     GtkTreeIter        *iter)
{
    GtkTreeModel *model = GTK_TREE_MODEL (store);

    if (gtk_tree_model_get_iter_first (model, iter)) do
    {
        char *id = NULL;
        GdkPixbuf *pixbuf = NULL;

        gtk_tree_model_get (model, iter, ICON_COLUMN_STOCK, &id,
                            ICON_COLUMN_PIXBUF, &pixbuf, -1);

        if (!id && !pixbuf)
            return;

        g_free (id);
        if (pixbuf)
            g_object_unref (pixbuf);
    }
    while (gtk_tree_model_iter_next (model, iter));

    gtk_list_store_append (store, iter);
    gtk_list_store_set (store, iter, ICON_COLUMN_LABEL, "None", -1);
}
