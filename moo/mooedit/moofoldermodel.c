/*
 *   mooedit/moofoldermodel.c
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

#define MOO_FILE_SYSTEM_COMPILATION
#include "mooedit/moofoldermodel.h"
#include <string.h>


static void moo_folder_model_class_init         (MooFolderModelClass *klass);
static void moo_folder_model_init               (MooFolderModel *model);
static void moo_folder_model_tree_iface_init    (GtkTreeModelIface *iface);
static void moo_folder_model_finalize           (GObject *object);
static void moo_folder_model_set_property       (GObject *object,
                                                 guint property_id,
                                                 const GValue *value,
                                                 GParamSpec *pspec);
static void moo_folder_model_get_property       (GObject *object,
                                                 guint property_id,
                                                 GValue *value,
                                                 GParamSpec *pspec);

static gpointer moo_folder_model_parent_class = NULL;

GType            moo_folder_model_get_type  (void)
{
    static GType type = 0;

    if (!type)
    {
        static GTypeInfo info = {
            /* interface types, classed types, instantiated types */
            sizeof (MooFolderClass),
            NULL, /* base_init; */
            NULL, /* base_finalize; */
            (GClassInitFunc) moo_folder_model_class_init,
            NULL, /* class_finalize; */
            NULL, /* class_data; */
            sizeof (MooFolder),
            0, /* n_preallocs; */
            (GInstanceInitFunc) moo_folder_model_init,
            NULL /* value_table; */
        };

        static GInterfaceInfo iface_info = {
            (GInterfaceInitFunc) moo_folder_model_tree_iface_init,
            NULL, /* interface_finalize; */
            NULL /* interface_data; */
        };

        type = g_type_register_static (G_TYPE_OBJECT,
                                       "MooFolderModel",
                                       &info, 0);
        g_type_add_interface_static (type, GTK_TYPE_TREE_MODEL,
                                     &iface_info);
    }

    return type;
}

enum {
    PROP_0,
    PROP_FOLDER
};

static void moo_folder_model_class_init         (MooFolderModelClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    moo_folder_model_parent_class = g_type_class_peek_parent (klass);

    gobject_class->finalize = moo_folder_model_finalize;
    gobject_class->set_property = moo_folder_model_set_property;
    gobject_class->get_property = moo_folder_model_get_property;

    g_object_class_install_property (gobject_class,
                                     PROP_FOLDER,
                                     g_param_spec_object ("folder",
                                             "folder",
                                             "folder",
                                             MOO_TYPE_FOLDER,
                                             G_PARAM_READWRITE));
}


static void         moo_folder_model_add_files      (MooFolderModel *model,
                                                     GSList         *files);
static void         moo_folder_model_change_files   (MooFolderModel *model,
                                                     GSList         *files);
static void         moo_folder_model_remove_files   (MooFolderModel *model,
                                                     GSList         *files);

static GtkTreeModelFlags moo_folder_model_get_flags (GtkTreeModel *tree_model);
static gint         moo_folder_model_get_n_columns  (GtkTreeModel *tree_model);
static GType        moo_folder_model_get_column_type(GtkTreeModel *tree_model,
                                                     gint          index_);
static gboolean     moo_folder_model_get_iter_impl  (GtkTreeModel *tree_model,
                                                     GtkTreeIter  *iter,
                                                     GtkTreePath  *path);
static GtkTreePath *moo_folder_model_get_path       (GtkTreeModel *tree_model,
                                                     GtkTreeIter  *iter);
static void         moo_folder_model_get_value      (GtkTreeModel *tree_model,
                                                     GtkTreeIter  *iter,
                                                     gint          column,
                                                     GValue       *value);
static gboolean     moo_folder_model_iter_next      (GtkTreeModel *tree_model,
                                                     GtkTreeIter  *iter);
static gboolean     moo_folder_model_iter_children  (GtkTreeModel *tree_model,
                                                     GtkTreeIter  *iter,
                                                     GtkTreeIter  *parent);
static gboolean     moo_folder_model_iter_has_child (GtkTreeModel *tree_model,
                                                     GtkTreeIter  *iter);
static gint         moo_folder_model_iter_n_children(GtkTreeModel *tree_model,
                                                     GtkTreeIter  *iter);
static gboolean     moo_folder_model_iter_nth_child (GtkTreeModel *tree_model,
                                                     GtkTreeIter  *iter,
                                                     GtkTreeIter  *parent,
                                                     gint          n);
static gboolean     moo_folder_model_iter_parent    (GtkTreeModel *tree_model,
                                                     GtkTreeIter  *iter,
                                                     GtkTreeIter  *child);


static void moo_folder_model_tree_iface_init    (GtkTreeModelIface *iface)
{
    iface->get_flags = moo_folder_model_get_flags;
    iface->get_n_columns = moo_folder_model_get_n_columns;
    iface->get_column_type = moo_folder_model_get_column_type;
    iface->get_iter = moo_folder_model_get_iter_impl;
    iface->get_path = moo_folder_model_get_path;
    iface->get_value = moo_folder_model_get_value;
    iface->iter_next = moo_folder_model_iter_next;
    iface->iter_children = moo_folder_model_iter_children;
    iface->iter_has_child = moo_folder_model_iter_has_child;
    iface->iter_n_children = moo_folder_model_iter_n_children;
    iface->iter_nth_child = moo_folder_model_iter_nth_child;
    iface->iter_parent = moo_folder_model_iter_parent;
}


static void moo_folder_model_set_property       (GObject *object,
                                                 guint property_id,
                                                 const GValue *value,
                                                 GParamSpec *pspec)
{
    MooFolderModel *model = MOO_FOLDER_MODEL (object);

    switch (property_id)
    {
        case PROP_FOLDER:
            moo_folder_model_set_folder (model,
                                         g_value_get_object (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}


static void moo_folder_model_get_property       (GObject *object,
                                                 guint property_id,
                                                 GValue *value,
                                                 GParamSpec *pspec)
{
    MooFolderModel *model = MOO_FOLDER_MODEL (object);

    switch (property_id)
    {
        case PROP_FOLDER:
            g_value_set_object (value,
                                moo_folder_model_get_folder (model));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}


struct _MooFolderModelPrivate {
    MooFolder   *folder;

    GSList      *dirs;                  /* MooFile*, sorted by name */
    int          n_dirs;
    GHashTable  *name_to_dir;           /* char* -> GSList* */
    GHashTable  *display_name_to_dir;   /* char* -> GSList* */
    GHashTable  *dir_to_link;           /* MooFile* -> GSList* */

    GSList      *files;
    int          n_files;
    GHashTable  *name_to_file;
    GHashTable  *display_name_to_file;
    GHashTable  *file_to_link;
};


void             moo_folder_model_set_folder    (MooFolderModel *model,
                                                 MooFolder      *folder)
{
    GSList *files;

    g_return_if_fail (!folder || MOO_IS_FOLDER (folder));

    if (model->priv->folder == folder)
        return;

    if (model->priv->folder)
    {
        g_signal_handlers_disconnect_by_func (model->priv->folder,
                                              (gpointer) moo_folder_model_add_files,
                                              model);
        g_signal_handlers_disconnect_by_func (model->priv->folder,
                                              (gpointer) moo_folder_model_remove_files,
                                              model);
        g_signal_handlers_disconnect_by_func (model->priv->folder,
                                              (gpointer) moo_folder_model_change_files,
                                              model);

        files = moo_folder_list_files (model->priv->folder);
        moo_folder_model_remove_files (model, files);
        g_slist_foreach (files, (GFunc) moo_file_unref, NULL);
        g_slist_free (files);

        g_object_unref (model->priv->folder);
        model->priv->folder = NULL;
    }

    if (folder)
    {
        model->priv->folder = g_object_ref (folder);

        g_signal_connect_swapped (folder, "files_added",
                                  G_CALLBACK (moo_folder_model_add_files),
                                  model);
        g_signal_connect_swapped (folder, "files_removed",
                                  G_CALLBACK (moo_folder_model_remove_files),
                                  model);
        g_signal_connect_swapped (folder, "files_changed",
                                  G_CALLBACK (moo_folder_model_change_files),
                                  model);

        files = moo_folder_list_files (folder);
        moo_folder_model_add_files (model, files);
        g_slist_foreach (files, (GFunc) moo_file_unref, NULL);
        g_slist_free (files);
    }

    g_object_notify (G_OBJECT (model), "folder");
}


static void moo_folder_model_init               (MooFolderModel *model)
{
    model->priv = g_new0 (MooFolderModelPrivate, 1);

    model->priv->name_to_dir =
            g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
    model->priv->display_name_to_dir =
            g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
    model->priv->dir_to_link = g_hash_table_new (g_direct_hash, g_direct_equal);

    model->priv->name_to_file =
            g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
    model->priv->display_name_to_file =
            g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
    model->priv->file_to_link = g_hash_table_new (g_direct_hash, g_direct_equal);
}


static void moo_folder_model_finalize           (GObject *object)
{
    MooFolderModel *model = MOO_FOLDER_MODEL (object);

    if (model->priv->folder)
    {
        g_signal_handlers_disconnect_by_func (model->priv->folder,
                                              (gpointer) moo_folder_model_add_files,
                                              model);
        g_signal_handlers_disconnect_by_func (model->priv->folder,
                                              (gpointer) moo_folder_model_remove_files,
                                              model);
        g_signal_handlers_disconnect_by_func (model->priv->folder,
                                              (gpointer) moo_folder_model_change_files,
                                              model);
        g_object_unref (model->priv->folder);
        model->priv->folder = NULL;
    }

    g_hash_table_destroy (model->priv->display_name_to_dir);
    g_hash_table_destroy (model->priv->name_to_dir);
    g_hash_table_destroy (model->priv->dir_to_link);
    g_hash_table_destroy (model->priv->display_name_to_file);
    g_hash_table_destroy (model->priv->name_to_file);
    g_hash_table_destroy (model->priv->file_to_link);

    g_slist_foreach (model->priv->dirs, (GFunc)moo_file_unref, NULL);
    g_slist_foreach (model->priv->files, (GFunc)moo_file_unref, NULL);
    g_slist_free (model->priv->dirs);
    g_slist_free (model->priv->files);

    g_free (model->priv);
    model->priv = NULL;

    G_OBJECT_CLASS(moo_folder_model_parent_class)->finalize (object);
}


/*****************************************************************************/
/* Files
 */

static void     model_add_moo_file      (MooFile        *file,
                                         MooFolderModel *model);
static void     model_change_moo_file   (MooFile        *file,
                                         MooFolderModel *model);
static void     model_remove_moo_file   (MooFile        *file,
                                         MooFolderModel *model);
#if 0
static void     model_change_from_file_to_dir   (MooFolderModel *model,
                                                 MooFile        *file);
static void     model_change_from_dir_to_file   (MooFolderModel *model,
                                                 MooFile        *file);
#endif
static gboolean model_contains_file     (MooFolderModel *model,
                                         MooFile        *file);
static gboolean model_is_dir            (MooFolderModel *model,
                                         MooFile        *file);

static GSList  *list_insert_sorted      (GSList        **list,
                                         int            *list_len,
                                         MooFile        *file,
                                         int            *position);
static void     list_delete_link        (GSList        **list,
                                         GSList         *link,
                                         int            *list_len);
#if 0
static void     list_remove_link        (GSList        **list,
                                         GSList         *link,
                                         int            *list_len);
static int      list_insert_link_sorted (GSList        **list,
                                         int            *list_len,
                                         GSList         *link);
#endif
static GSList  *list_find_dir           (MooFolderModel *model,
                                         MooFile        *file,
                                         int            *index);
static GSList  *list_find_file          (MooFolderModel *model,
                                         MooFile        *file,
                                         int            *index);

static void     hash_table_insert_dir   (MooFolderModel *model,
                                         MooFile        *file,
                                         GSList         *link);
static void     hash_table_insert_file  (MooFolderModel *model,
                                         MooFile        *file,
                                         GSList         *link);
static void     hash_table_remove_dir   (MooFolderModel *model,
                                         MooFile        *file);
static void     hash_table_remove_file  (MooFolderModel *model,
                                         MooFile        *file);


#define ITER_MODEL(ip)      ((ip)->user_data)
#define ITER_FILE(ip)       ((ip)->user_data2)
#define ITER_DIR(ip)        ((ip)->user_data3)
#define ITER_GET_MODEL(ip)  ((MooFolderModel*) ITER_MODEL (ip))
#define ITER_GET_FILE(ip)   ((MooFile*) ITER_FILE (ip))
#define ITER_GET_DIR(ip)    (GPOINTER_TO_INT (ITER_DIR (ip)))
#define ITER_SET_DIR(ip,d)  ITER_DIR (ip) = GINT_TO_POINTER (d)

#define ITER_INIT(ip,model,file,is_dir) \
G_STMT_START {                          \
    ITER_MODEL(ip) = model;             \
    ITER_FILE(ip) = file;               \
    ITER_SET_DIR(ip, is_dir);           \
} G_STMT_END

#ifdef DEBUG
#if 0
#define DEFINE_CHECK_INTEGRITY
static void CHECK_INTEGRITY (MooFolderModel *model);
#endif
#if 0
#define DEFINE_CHECK_ITER
static void CHECK_ITER (MooFolderModel *model, GtkTreeIter *iter);
#endif
#endif /* DEBUG */

#ifndef DEFINE_CHECK_INTEGRITY
#define CHECK_INTEGRITY(model)
#endif
#ifndef DEFINE_CHECK_ITER
#define CHECK_ITER(model,iter)
#endif


static void         moo_folder_model_add_files      (MooFolderModel *model,
                                                     GSList         *files)
{
    g_slist_foreach (files, (GFunc) model_add_moo_file, model);
}

static void         moo_folder_model_change_files   (MooFolderModel *model,
                                                     GSList         *files)
{
    g_slist_foreach (files, (GFunc) model_change_moo_file, model);
}

static void         moo_folder_model_remove_files   (MooFolderModel *model,
                                                     GSList         *files)
{
    g_slist_foreach (files, (GFunc) model_remove_moo_file, model);
}


static void     model_add_moo_file  (MooFile        *file,
                                     MooFolderModel *model)
{
    GSList *link;
    int index;
    GtkTreePath *path;
    GtkTreeIter iter;
    gboolean is_dir;

    g_assert (!model_contains_file (model, file));
    g_assert (file != NULL);

    moo_file_ref (file);

    if (moo_file_test (file, MOO_FILE_IS_FOLDER))
    {
        is_dir = TRUE;
        link = list_insert_sorted (&model->priv->dirs,
                                   &model->priv->n_dirs,
                                   moo_file_ref (file),
                                   &index);
        g_assert (g_slist_nth (model->priv->dirs, index) == link);
        hash_table_insert_dir (model, file, link);
    }
    else
    {
        is_dir = FALSE;
        link = list_insert_sorted (&model->priv->files,
                                   &model->priv->n_files,
                                   moo_file_ref (file),
                                   &index);
        g_assert (g_slist_nth (model->priv->files, index) == link);
        index += model->priv->n_dirs;
        hash_table_insert_file (model, file, link);
    }

    CHECK_INTEGRITY (model);

    ITER_INIT (&iter, model, file, is_dir);
    CHECK_ITER (model, &iter);

    path = gtk_tree_path_new_from_indices (index, -1);
    gtk_tree_model_row_inserted (GTK_TREE_MODEL (model), path, &iter);
    gtk_tree_path_free (path);
}


static void     model_change_moo_file   (MooFile        *file,
                                         MooFolderModel *model)
{
    int index;
    GSList *link;
    GtkTreePath *path;
    GtkTreeIter iter;
    gboolean is_dir;

    g_assert (model_contains_file (model, file));
    g_assert (file != NULL);

    if (model_is_dir (model, file))
    {
        if (!moo_file_test (file, MOO_FILE_IS_FOLDER))
        {
            moo_file_ref (file);
            model_remove_moo_file (file, model);
            model_add_moo_file (file, model);
            moo_file_unref (file);
            return;
        }

        link = g_hash_table_lookup (model->priv->dir_to_link, file);
        g_assert (link != NULL);
        index = g_slist_position (model->priv->dirs, link);
        g_assert (index >= 0);
        is_dir = TRUE;
    }
    else
    {
        if (moo_file_test (file, MOO_FILE_IS_FOLDER))
        {
            moo_file_ref (file);
            model_remove_moo_file (file, model);
            model_add_moo_file (file, model);
            moo_file_unref (file);
            return;
        }

        link = g_hash_table_lookup (model->priv->file_to_link, file);
        g_assert (link != NULL);
        index = g_slist_position (model->priv->files, link);
        g_assert (index >= 0);
        index += model->priv->n_dirs;
        is_dir = FALSE;
    }

    CHECK_INTEGRITY (model);

    ITER_INIT (&iter, model, file, is_dir);
    CHECK_ITER (model, &iter);

    path = gtk_tree_path_new_from_indices (index, -1);
    gtk_tree_model_row_changed (GTK_TREE_MODEL (model), path, &iter);
    gtk_tree_path_free (path);
}


static void     model_remove_moo_file   (MooFile        *file,
                                         MooFolderModel *model)
{
    GSList *link;
    int index;
    GtkTreePath *path;

    g_assert (model_contains_file (model, file));
    g_assert (file != NULL);

    if (model_is_dir (model, file))
    {
        link = list_find_dir (model, file, &index);
        g_assert (link != NULL);
        hash_table_remove_dir (model, file);
        list_delete_link (&model->priv->dirs, link,
                           &model->priv->n_dirs);
    }
    else
    {
        link = list_find_file (model, file, &index);
        index += model->priv->n_dirs;
        g_assert (link != NULL);
        hash_table_remove_file (model, file);
        list_delete_link (&model->priv->files, link,
                           &model->priv->n_files);
    }

    CHECK_INTEGRITY (model);

    path = gtk_tree_path_new_from_indices (index, -1);
    gtk_tree_model_row_deleted (GTK_TREE_MODEL (model), path);

    gtk_tree_path_free (path);
    moo_file_unref (file);
}


#ifdef DEBUG
#if 1
#define CHECK_ORDER(order,n)                \
G_STMT_START {                              \
    int *check = g_new0 (int, n);           \
    for (i = 0; i < n; ++i)                 \
    {                                       \
        g_assert (0 <= order[i]);           \
        g_assert (order[i] < n);            \
        g_assert (check[order[i]] == 0);    \
        check[order[i]] = 1;                \
    }                                       \
    g_free (check);                         \
} G_STMT_END
#endif
#endif /* DEBUG */
#ifndef CHECK_ORDER
#define CHECK_ORDER(order,n)
#endif


#if 0
static void     model_change_from_file_to_dir   (MooFolderModel *model,
                                                 MooFile        *file)
{
    GSList *link;
    int file_index, dir_index;
    int total, i;
    int *new_order;
    GtkTreeIter iter;
    GtkTreePath *path;

    g_assert (!model_is_dir (model, file));
    g_assert (moo_file_test (file, MOO_FILE_IS_FOLDER));

    link = list_find_file (model, file, &file_index);
    g_assert (link != NULL);
    file_index += model->priv->n_dirs;

    hash_table_remove_file (model, file);
    list_remove_link (&model->priv->files, link,
                      &model->priv->n_files);

    dir_index = list_insert_link_sorted (&model->priv->dirs,
                                         &model->priv->n_dirs,
                                         link);
    hash_table_insert_dir (model, file, link);

    g_assert (dir_index <= file_index);

    if (file_index != dir_index)
    {
        int *tmp;

        total = model->priv->n_files + model->priv->n_dirs;
        new_order = g_new (int, total);

        for (i = 0; i < dir_index; ++i)
            new_order[i] = i;
        for (i = dir_index; i < file_index; ++i)
            new_order[i] = i+1;
        for (i = file_index + 1; i < total; ++i)
            new_order[i] = i;

        new_order[file_index] = dir_index;

        CHECK_ORDER (new_order, total);

        tmp = g_new (int, total);
        for (i = 0; i < total; ++i)
            tmp[new_order[i]] = i;
        g_free (new_order);
        new_order = tmp;

        CHECK_ORDER (new_order, total);

        gtk_tree_model_rows_reordered (GTK_TREE_MODEL (model), NULL, NULL,
                                       new_order);
        g_free (new_order);
    }

    CHECK_INTEGRITY (model);

    ITER_INIT (&iter, model, file, TRUE);
    CHECK_ITER (model, &iter);

    path = gtk_tree_path_new_from_indices (dir_index, -1);
    gtk_tree_model_row_changed (GTK_TREE_MODEL (model), path, &iter);
    gtk_tree_path_free (path);
}


static void     model_change_from_dir_to_file   (MooFolderModel *model,
                                                 MooFile        *file)
{
    GSList *link;
    int file_index, dir_index;
    int total, i;
    int *new_order;
    GtkTreeIter iter;
    GtkTreePath *path;

    g_assert (model_is_dir (model, file));
    g_assert (!moo_file_test (file, MOO_FILE_IS_FOLDER));

    link = list_find_dir (model, file, &dir_index);
    g_assert (link != NULL);

    hash_table_remove_dir (model, file);
    list_remove_link (&model->priv->dirs, link,
                       &model->priv->n_dirs);

    file_index = list_insert_link_sorted (&model->priv->files,
                                          &model->priv->n_files,
                                          link);
    hash_table_insert_file (model, file, link);
    file_index += model->priv->n_dirs;

    g_assert (dir_index <= file_index);

    if (file_index != dir_index)
    {
        int *tmp;

        total = model->priv->n_files + model->priv->n_dirs;
        new_order = g_new (int, total);

        for (i = 0; i < dir_index; ++i)
            new_order[i] = i;
        for (i = dir_index + 1; i <= file_index; ++i)
            new_order[i] = i - 1;
        for (i = file_index + 1; i < total; ++i)
            new_order[i] = i;

        new_order[dir_index] = file_index;

        CHECK_ORDER (new_order, total);

        tmp = g_new (int, total);
        for (i = 0; i < total; ++i)
            tmp[new_order[i]] = i;
        g_free (new_order);
        new_order = tmp;

        CHECK_ORDER (new_order, total);

        gtk_tree_model_rows_reordered (GTK_TREE_MODEL (model), NULL, NULL,
                                       new_order);
        g_free (new_order);
    }

    CHECK_INTEGRITY (model);

    ITER_INIT (&iter, model, file, FALSE);
    CHECK_ITER (model, &iter);

    path = gtk_tree_path_new_from_indices (file_index, -1);
    gtk_tree_model_row_changed (GTK_TREE_MODEL (model), path, &iter);
    gtk_tree_path_free (path);
}
#endif


static gboolean model_contains_file     (MooFolderModel *model,
                                         MooFile        *file)
{
    return g_hash_table_lookup (model->priv->dir_to_link, file) != NULL ||
            g_hash_table_lookup (model->priv->file_to_link, file) != NULL;
}


static gboolean model_is_dir            (MooFolderModel *model,
                                         MooFile        *file)
{
    return g_hash_table_lookup (model->priv->dir_to_link, file) != NULL;
}


static void     hash_table_insert_dir   (MooFolderModel *model,
                                         MooFile        *file,
                                         GSList         *link)
{
    g_hash_table_insert (model->priv->dir_to_link, file, link);
    g_hash_table_insert (model->priv->name_to_dir,
                         g_strdup (moo_file_get_basename (file)),
                         file);
    g_hash_table_insert (model->priv->display_name_to_dir,
                         g_strdup (moo_file_get_display_basename (file)),
                         file);
}

static void     hash_table_insert_file  (MooFolderModel *model,
                                         MooFile        *file,
                                         GSList         *link)
{
    g_hash_table_insert (model->priv->file_to_link, file, link);
    g_hash_table_insert (model->priv->name_to_file,
                         g_strdup (moo_file_get_basename (file)),
                         file);
    g_hash_table_insert (model->priv->display_name_to_file,
                         g_strdup (moo_file_get_display_basename (file)),
                         file);
}

static void     hash_table_remove_dir   (MooFolderModel *model,
                                         MooFile        *file)
{
    g_hash_table_remove (model->priv->dir_to_link, file);
    g_hash_table_remove (model->priv->name_to_dir,
                         moo_file_get_basename (file));
    g_hash_table_remove (model->priv->display_name_to_dir,
                         moo_file_get_display_basename (file));
}

static void     hash_table_remove_file  (MooFolderModel *model,
                                         MooFile        *file)
{
    g_hash_table_remove (model->priv->file_to_link, file);
    g_hash_table_remove (model->priv->name_to_file,
                         moo_file_get_basename (file));
    g_hash_table_remove (model->priv->display_name_to_file,
                         moo_file_get_display_basename (file));
}


static void     list_delete_link        (GSList        **list,
                                         GSList         *link,
                                         int            *list_len)
{
    g_assert (*list_len == (int)g_slist_length (*list));
    *list = g_slist_delete_link (*list, link);
    (*list_len)--;
    g_assert (*list_len == (int)g_slist_length (*list));
}


#if 0
static void     list_remove_link        (GSList        **list,
                                         GSList         *link,
                                         int            *list_len)
{
    *list = g_slist_remove_link (*list, link);
    (*list_len)--;
    g_assert (*list_len == (int)g_slist_length (*list));
}
#endif


static GSList  *list_find_dir           (MooFolderModel *model,
                                         MooFile        *file,
                                         int            *index)
{
    GSList *link = g_hash_table_lookup (model->priv->dir_to_link, file);
    g_return_val_if_fail (link != NULL, NULL);
    *index = g_slist_position (model->priv->dirs, link);
    g_return_val_if_fail (*index >= 0, NULL);
    return link;
}


static GSList  *list_find_file          (MooFolderModel *model,
                                         MooFile        *file,
                                         int            *index)
{
    GSList *link = g_hash_table_lookup (model->priv->file_to_link, file);
    g_return_val_if_fail (link != NULL, NULL);
    *index = g_slist_position (model->priv->files, link);
    g_return_val_if_fail (*index >= 0, NULL);
    return link;
}


static int      cmp_filenames   (MooFile *f1, MooFile *f2)
{
    return strcmp (moo_file_get_basename (f1),
                   moo_file_get_basename (f2));
}

/* TODO */
static GSList  *list_insert_sorted      (GSList        **list,
                                         int            *list_len,
                                         MooFile        *file,
                                         int            *position)
{
    GSList *link;
    g_assert (*list_len == (int)g_slist_length (*list));
    g_assert (g_slist_find (*list, file) == NULL);
    *list = g_slist_insert_sorted (*list, file, (GCompareFunc) cmp_filenames);
    link = g_slist_find (*list, file);
    g_assert (link != NULL);
    *position = g_slist_position (*list, link);
    g_assert (*position >= 0);
    (*list_len)++;
    g_assert (*list_len == (int)g_slist_length (*list));
    return link;
}


#if 0
static int      list_insert_link_sorted (GSList        **list,
                                         int            *list_len,
                                         GSList         *link)
{
    int position;
    GSList *tmp_link;

    g_assert (*list_len == (int)g_slist_length (*list));
    g_assert (g_slist_find (*list, link->data) == NULL);
    tmp_link = list_insert_sorted (list, list_len, link->data, &position);

    if (position == 0)
    {
        link->next = tmp_link->next;
        *list = link;
        g_slist_free_1 (tmp_link);
    }
    else
    {
        GSList *prev = g_slist_nth (*list, position - 1);
        prev->next = link;
        link->next = tmp_link->next;
        g_slist_free_1 (tmp_link);
    }

    g_assert (g_slist_nth (*list, position) == link);
    g_assert (*list_len == (int)g_slist_length (*list));
    g_assert (g_slist_find (*list, link->data) == link);
    return position;
}
#endif


/***************************************************************************/
/* GtkTreeModel
 */

static GtkTreeModelFlags moo_folder_model_get_flags (G_GNUC_UNUSED GtkTreeModel *tree_model)
{
    return GTK_TREE_MODEL_ITERS_PERSIST | GTK_TREE_MODEL_LIST_ONLY;
}


static gint         moo_folder_model_get_n_columns  (G_GNUC_UNUSED GtkTreeModel *tree_model)
{
    g_assert (MOO_FOLDER_MODEL_N_COLUMNS == 1);
    return MOO_FOLDER_MODEL_N_COLUMNS;
}


static GType        moo_folder_model_get_column_type(G_GNUC_UNUSED GtkTreeModel *tree_model,
                                                     gint          index_)
{
    g_return_val_if_fail (index_ == 0, 0);
    return MOO_TYPE_FILE;
}


static gboolean     moo_folder_model_get_iter_impl  (GtkTreeModel *tree_model,
                                                     GtkTreeIter  *iter,
                                                     GtkTreePath  *path)
{
    MooFolderModel *model;
    int index;
    GSList *link;
    gboolean is_dir;

    g_return_val_if_fail (MOO_IS_FOLDER_MODEL (tree_model), FALSE);
    g_return_val_if_fail (iter != NULL && path != NULL, FALSE);

    model = MOO_FOLDER_MODEL (tree_model);

    if (gtk_tree_path_get_depth (path) != 1)
        g_return_val_if_reached (FALSE);

    index = gtk_tree_path_get_indices(path)[0];

    g_return_val_if_fail (index >= 0, FALSE);

    if (index >= model->priv->n_files + model->priv->n_dirs)
        return FALSE;

    if (index < model->priv->n_dirs)
    {
        is_dir = TRUE;
        link = g_slist_nth (model->priv->dirs, index);
    }
    else
    {
        is_dir = FALSE;
        index -= model->priv->n_dirs;
        link = g_slist_nth (model->priv->files, index);
    }

    g_assert (link != NULL);
    ITER_INIT (iter, model, link->data, is_dir);
    CHECK_ITER (model, iter);

    return TRUE;
}


static GtkTreePath *moo_folder_model_get_path       (GtkTreeModel *tree_model,
                                                     GtkTreeIter  *iter)
{
    MooFolderModel *model;
    int index;
    GSList *link;

    g_return_val_if_fail (MOO_IS_FOLDER_MODEL (tree_model), NULL);
    g_return_val_if_fail (iter != NULL, NULL);

    model = MOO_FOLDER_MODEL (tree_model);
    g_return_val_if_fail (ITER_MODEL (iter) == model, NULL);
    g_return_val_if_fail (ITER_FILE (iter) != NULL, NULL);

    if (ITER_DIR (iter))
        link = g_hash_table_lookup (model->priv->dir_to_link,
                                    ITER_FILE (iter));
    else
        link = g_hash_table_lookup (model->priv->file_to_link,
                                    ITER_FILE (iter));

    g_return_val_if_fail (link != NULL, NULL);

    if (ITER_DIR (iter))
        index = g_slist_position (model->priv->dirs, link);
    else
        index = g_slist_position (model->priv->files, link) +
                model->priv->n_dirs;

    g_return_val_if_fail (index >= 0, NULL);

    return gtk_tree_path_new_from_indices (index, -1);
}


static void         moo_folder_model_get_value      (GtkTreeModel *tree_model,
                                                     GtkTreeIter  *iter,
                                                     gint          column,
                                                     GValue       *value)
{
    g_return_if_fail (MOO_IS_FOLDER_MODEL (tree_model));
    g_return_if_fail (iter != NULL);
    g_return_if_fail (column == 0);
    g_return_if_fail (value != NULL);
    g_return_if_fail (ITER_MODEL (iter) == tree_model);
    g_return_if_fail (ITER_FILE (iter) != NULL);

    g_value_init (value, MOO_TYPE_FILE);
    g_value_set_boxed (value, ITER_FILE (iter));
}


static gboolean     moo_folder_model_iter_next      (GtkTreeModel *tree_model,
                                                     GtkTreeIter  *iter)
{
    MooFolderModel *model;
    GSList *link;

    g_return_val_if_fail (MOO_IS_FOLDER_MODEL (tree_model), FALSE);
    g_return_val_if_fail (iter != NULL, FALSE);

    model = MOO_FOLDER_MODEL (tree_model);
    g_return_val_if_fail (ITER_MODEL (iter) == model, FALSE);
    g_return_val_if_fail (ITER_FILE (iter) != NULL, FALSE);

    if (ITER_DIR (iter))
    {
        link = g_hash_table_lookup (model->priv->dir_to_link,
                                    ITER_FILE (iter));
        g_return_val_if_fail (link != NULL, FALSE);

        if (link->next)
        {
            ITER_INIT (iter, model, link->next->data, TRUE);
            CHECK_ITER (model, iter);
            return TRUE;
        }
        else if (model->priv->files)
        {
            ITER_INIT (iter, model, model->priv->files->data, FALSE);
            CHECK_ITER (model, iter);
            return TRUE;
        }
        else
        {
            ITER_INIT (iter, NULL, NULL, FALSE);
            return FALSE;
        }
    }
    else
    {
        link = g_hash_table_lookup (model->priv->file_to_link,
                                    ITER_FILE (iter));
        g_return_val_if_fail (link != NULL, FALSE);

        if (link->next)
        {
            ITER_INIT (iter, model, link->next->data, FALSE);
            CHECK_ITER (model, iter);
            return TRUE;
        }
        else
        {
            ITER_INIT (iter, NULL, NULL, FALSE);
            return FALSE;
        }
    }
}


static gboolean     moo_folder_model_iter_children  (GtkTreeModel *tree_model,
                                                     GtkTreeIter  *iter,
                                                     GtkTreeIter  *parent)
{
    MooFolderModel *model;

    g_return_val_if_fail (MOO_IS_FOLDER_MODEL (tree_model), FALSE);

    model = MOO_FOLDER_MODEL (tree_model);

    if (!parent)
    {
        if (model->priv->dirs)
        {
            ITER_INIT (iter, model, model->priv->dirs->data, TRUE);
            CHECK_ITER (model, iter);
            return TRUE;
        }
        else if (model->priv->files)
        {
            ITER_INIT (iter, model, model->priv->files->data, FALSE);
            CHECK_ITER (model, iter);
            return TRUE;
        }
        else
        {
            ITER_INIT (iter, NULL, NULL, FALSE);
            return FALSE;
        }
    }
    else
    {
        g_return_val_if_fail (ITER_MODEL (parent) == model, FALSE);
        g_return_val_if_fail (ITER_FILE (parent) != NULL, FALSE);
        ITER_INIT (iter, NULL, NULL, FALSE);
        return FALSE;
    }
}


static gboolean     moo_folder_model_iter_has_child (GtkTreeModel *tree_model,
                                                     GtkTreeIter  *iter)
{
    g_return_val_if_fail (MOO_IS_FOLDER_MODEL (tree_model), FALSE);
    g_return_val_if_fail (iter != NULL, FALSE);
    g_return_val_if_fail (ITER_MODEL (iter) == tree_model, FALSE);
    g_return_val_if_fail (ITER_FILE (iter) != NULL, FALSE);
    return FALSE;
}


static gint         moo_folder_model_iter_n_children(GtkTreeModel *tree_model,
                                                     GtkTreeIter  *iter)
{
    MooFolderModel *model;

    g_return_val_if_fail (MOO_IS_FOLDER_MODEL (tree_model), 0);

    if (iter)
    {
        g_return_val_if_fail (ITER_MODEL (iter) == tree_model, 0);
        g_return_val_if_fail (ITER_FILE (iter) != NULL, 0);
        return 0;
    }

    model = MOO_FOLDER_MODEL (tree_model);
    return model->priv->n_dirs + model->priv->n_files;
}


static gboolean     moo_folder_model_iter_nth_child (GtkTreeModel *tree_model,
                                                     GtkTreeIter  *iter,
                                                     GtkTreeIter  *parent,
                                                     gint          n)
{
    MooFolderModel *model;
    GtkTreePath *path;
    gboolean result;

    g_return_val_if_fail (MOO_IS_FOLDER_MODEL (tree_model), FALSE);

    if (parent)
    {
        g_return_val_if_fail (ITER_MODEL (parent) == tree_model, FALSE);
        g_return_val_if_fail (ITER_FILE (parent) != NULL, FALSE);
        return FALSE;
    }

    /* TODO */
    model = MOO_FOLDER_MODEL (tree_model);
    path = gtk_tree_path_new_from_indices (n, -1);
    result = gtk_tree_model_get_iter (tree_model, iter, path);
    gtk_tree_path_free (path);
    return result;
}


static gboolean     moo_folder_model_iter_parent    (GtkTreeModel *tree_model,
                                                     GtkTreeIter  *iter,
                                                     GtkTreeIter  *child)
{
    g_return_val_if_fail (MOO_IS_FOLDER_MODEL (tree_model), FALSE);
    g_return_val_if_fail (child != NULL, FALSE);
    g_return_val_if_fail (ITER_MODEL (child) == tree_model, FALSE);
    g_return_val_if_fail (ITER_FILE (child) != NULL, FALSE);
    ITER_INIT (iter, NULL, NULL, FALSE);
    return FALSE;
}


GtkTreeModel    *moo_folder_model_new           (MooFolder      *folder)
{
    g_return_val_if_fail (!folder || MOO_IS_FOLDER (folder), NULL);
    return GTK_TREE_MODEL (g_object_new (MOO_TYPE_FOLDER_MODEL,
                                         "folder", folder, NULL));
}


MooFolder       *moo_folder_model_get_folder    (MooFolderModel *model)
{
    g_return_val_if_fail (MOO_IS_FOLDER_MODEL (model), NULL);
    return model->priv->folder;
}


gboolean         moo_folder_model_get_iter      (MooFolderModel *model,
                                                 MooFile        *file,
                                                 GtkTreeIter    *iter)
{
    g_return_val_if_fail (MOO_IS_FOLDER_MODEL (model), FALSE);
    g_return_val_if_fail (file != NULL, FALSE);

    if (g_hash_table_lookup (model->priv->dir_to_link, file))
    {
        ITER_INIT (iter, model, file, TRUE);
        CHECK_ITER (model, iter);
        return TRUE;
    }
    else if (g_hash_table_lookup (model->priv->file_to_link, file))
    {
        ITER_INIT (iter, model, file, FALSE);
        CHECK_ITER (model, iter);
        return TRUE;
    }
    else
    {
        ITER_INIT (iter, NULL, NULL, FALSE);
        return FALSE;
    }
}


gboolean         moo_folder_model_get_iter_by_name  (MooFolderModel *model,
                                                     const char     *name,
                                                     GtkTreeIter    *iter)
{
    MooFile *file;

    g_return_val_if_fail (MOO_IS_FOLDER_MODEL (model), FALSE);
    g_return_val_if_fail (name != NULL, FALSE);
    g_return_val_if_fail (iter != NULL, FALSE);

    if ((file = g_hash_table_lookup (model->priv->name_to_dir, name)))
    {
        ITER_INIT (iter, model, file, TRUE);
        CHECK_ITER (model, iter);
        return TRUE;
    }
    else if ((file = g_hash_table_lookup (model->priv->name_to_file, name)))
    {
        ITER_INIT (iter, model, file, FALSE);
        CHECK_ITER (model, iter);
        return TRUE;
    }
    else
    {
        ITER_INIT (iter, NULL, NULL, FALSE);
        return FALSE;
    }
}


gboolean    moo_folder_model_get_iter_by_display_name   (MooFolderModel *model,
                                                         const char     *name,
                                                         GtkTreeIter    *iter)
{
    MooFile *file;

    g_return_val_if_fail (MOO_IS_FOLDER_MODEL (model), FALSE);
    g_return_val_if_fail (name != NULL, FALSE);
    g_return_val_if_fail (iter != NULL, FALSE);

    if ((file = g_hash_table_lookup (model->priv->display_name_to_dir, name)))
    {
        ITER_INIT (iter, model, file, TRUE);
        CHECK_ITER (model, iter);
        return TRUE;
    }
    else if ((file = g_hash_table_lookup (model->priv->display_name_to_file, name)))
    {
        ITER_INIT (iter, model, file, FALSE);
        CHECK_ITER (model, iter);
        return TRUE;
    }
    else
    {
        ITER_INIT (iter, NULL, NULL, FALSE);
        return FALSE;
    }
}


#ifdef DEFINE_CHECK_INTEGRITY
static void CHECK_INTEGRITY (MooFolderModel *model)
{
    GSList *link;

    g_assert ((int)g_slist_length (model->priv->dirs) == model->priv->n_dirs);
    g_assert ((int)g_hash_table_size (model->priv->name_to_dir) == model->priv->n_dirs);
    g_assert ((int)g_hash_table_size (model->priv->display_name_to_dir) == model->priv->n_dirs);
    g_assert ((int)g_hash_table_size (model->priv->dir_to_link) == model->priv->n_dirs);

    for (link = model->priv->dirs; link != NULL; link = link->next)
    {
        MooFile *file = link->data;
        g_assert (file != NULL);
        g_assert (file == g_hash_table_lookup (model->priv->name_to_dir,
                  moo_file_get_basename (file)));
        g_assert (file == g_hash_table_lookup (model->priv->display_name_to_dir,
                  moo_file_get_display_basename (file)));
        g_assert (link == g_hash_table_lookup (model->priv->dir_to_link, file));
    }

    g_assert ((int)g_slist_length (model->priv->files) == model->priv->n_files);
    g_assert ((int)g_hash_table_size (model->priv->name_to_file) == model->priv->n_files);
    g_assert ((int)g_hash_table_size (model->priv->display_name_to_file) == model->priv->n_files);
    g_assert ((int)g_hash_table_size (model->priv->file_to_link) == model->priv->n_files);

    for (link = model->priv->files; link != NULL; link = link->next)
    {
        MooFile *file = link->data;
        g_assert (file != NULL);
        g_assert (file == g_hash_table_lookup (model->priv->name_to_file,
                  moo_file_get_basename (file)));
        g_assert (file == g_hash_table_lookup (model->priv->display_name_to_file,
                  moo_file_get_display_basename (file)));
        g_assert (link == g_hash_table_lookup (model->priv->file_to_link, file));
    }
}
#endif /* DEFINE_CHECK_INTEGRITY */


#ifdef DEFINE_CHECK_ITER
static gboolean NO_CHECK_ITER = FALSE;

static void CHECK_ITER (MooFolderModel *model, GtkTreeIter *iter)
{
    GtkTreePath *path1, *path2;
    GtkTreeIter iter2;

    if (NO_CHECK_ITER)
    {
        NO_CHECK_ITER = FALSE;
        return;
    }
    else
    {
        NO_CHECK_ITER = TRUE;
    }

    g_assert (iter != NULL);
    g_assert (ITER_MODEL (iter) == model);
    g_assert (ITER_FILE (iter) != NULL);
    g_assert (model_contains_file (model, ITER_FILE (iter)));

    if (ITER_DIR (iter))
        g_assert (model_is_dir (model, ITER_FILE (iter)));
    else
        g_assert (!model_is_dir (model, ITER_FILE (iter)));

    path1 = gtk_tree_model_get_path (GTK_TREE_MODEL (model), iter);
    g_assert (path1 != NULL);
    g_assert (gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &iter2, path1));

    g_assert (ITER_MODEL (&iter2) == model);
    g_assert (ITER_FILE (&iter2) == ITER_FILE (iter));
    g_assert (ITER_DIR (&iter2) == ITER_DIR (iter));

    path2 = gtk_tree_model_get_path (GTK_TREE_MODEL (model), &iter2);
    g_assert (path2 != NULL);
    g_assert (!gtk_tree_path_compare (path1, path2));

    gtk_tree_path_free (path1);
    gtk_tree_path_free (path2);
}
#endif /* DEFINE_CHECK_ITER */
