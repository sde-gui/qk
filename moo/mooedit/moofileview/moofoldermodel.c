/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *   moofoldermodel.c
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
#include "moofoldermodel.h"
#include "moofoldermodel-private.h"


struct _MooFolderModelPrivate {
    MooFolder   *folder;
    FileList    *dirs;
    FileList    *files;
    gboolean     sort_case_sensitive;
    MooFileCmp   cmp_func;
};


static void moo_folder_model_class_init         (MooFolderModelClass *klass);
static void moo_folder_model_init               (MooFolderModel *model);

static void moo_folder_model_tree_iface_init        (GtkTreeModelIface      *iface);
static void moo_folder_model_drag_source_iface_init (GtkTreeDragSourceIface *iface);

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

        static GInterfaceInfo tree_model_info = {
            (GInterfaceInitFunc) moo_folder_model_tree_iface_init,
            NULL, /* interface_finalize; */
            NULL /* interface_data; */
        };

        static GInterfaceInfo drag_source_info = {
            (GInterfaceInitFunc) moo_folder_model_drag_source_iface_init,
            NULL, /* interface_finalize; */
            NULL /* interface_data; */
        };

        type = g_type_register_static (G_TYPE_OBJECT,
                                       "MooFolderModel",
                                       &info, 0);
        g_type_add_interface_static (type, GTK_TYPE_TREE_MODEL,
                                     &tree_model_info);
        g_type_add_interface_static (type, GTK_TYPE_TREE_DRAG_SOURCE,
                                     &drag_source_info);
    }

    return type;
}

enum {
    PROP_0,
    PROP_FOLDER,
    PROP_SORT_CASE_SENSITIVE
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

    g_object_class_install_property (gobject_class,
                                     PROP_SORT_CASE_SENSITIVE,
                                     g_param_spec_boolean ("sort-case-sensitive",
                                             "sort-case-sensitive",
                                             "sort-case-sensitive",
                                             MOO_FOLDER_MODEL_SORT_CASE_SENSITIVE_DEFAULT,
                                             G_PARAM_CONSTRUCT | G_PARAM_READWRITE));
}


static void         moo_folder_model_add_files      (MooFolderModel *model,
                                                     GSList         *files);
static void         moo_folder_model_change_files   (MooFolderModel *model,
                                                     GSList         *files);
static void         moo_folder_model_remove_files   (MooFolderModel *model,
                                                     GSList         *files);
static void         moo_folder_model_folder_deleted (MooFolderModel *model);
static void         moo_folder_model_disconnect_folder (MooFolderModel *model);


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

        case PROP_SORT_CASE_SENSITIVE:
            moo_folder_model_set_sort_case_sensitive (model,
                    g_value_get_boolean (value));
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

        case PROP_SORT_CASE_SENSITIVE:
            g_value_set_boolean (value, model->priv->sort_case_sensitive);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}


void             moo_folder_model_set_folder    (MooFolderModel *model,
                                                 MooFolder      *folder)
{
    GSList *files;

    g_return_if_fail (!folder || MOO_IS_FOLDER (folder));

    if (model->priv->folder == folder)
        return;

    moo_folder_model_disconnect_folder (model);

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
        g_signal_connect_swapped (folder, "deleted",
                                  G_CALLBACK (moo_folder_model_folder_deleted),
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

    model->priv->sort_case_sensitive = MOO_FOLDER_MODEL_SORT_CASE_SENSITIVE_DEFAULT;

    if (model->priv->sort_case_sensitive)
        model->priv->cmp_func = moo_file_cmp;
    else
        model->priv->cmp_func = moo_file_case_cmp;

    model->priv->files = file_list_new (model->priv->cmp_func);
    model->priv->dirs = file_list_new (model->priv->cmp_func);
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

    file_list_destroy (model->priv->dirs);
    file_list_destroy (model->priv->files);

    g_free (model->priv);
    model->priv = NULL;

    G_OBJECT_CLASS(moo_folder_model_parent_class)->finalize (object);
}


void    moo_folder_model_set_sort_case_sensitive    (MooFolderModel *model,
                                                     gboolean        case_sensitive)
{
    g_return_if_fail (MOO_IS_FOLDER_MODEL (model));

    if (model->priv->sort_case_sensitive != case_sensitive)
    {
        int *new_order, *real_new_order;
        guint i;
        guint dir_num = model->priv->dirs->size;
        guint total = model->priv->dirs->size + model->priv->files->size;
        GtkTreePath *path = gtk_tree_path_new ();

        model->priv->sort_case_sensitive = case_sensitive;

        if (model->priv->sort_case_sensitive)
            model->priv->cmp_func = moo_file_cmp;
        else
            model->priv->cmp_func = moo_file_case_cmp;

        real_new_order = g_new (int, total);

        for (i = 0; i < total; ++i)
            real_new_order[i] = i;

        file_list_set_cmp_func (model->priv->dirs, model->priv->cmp_func, &new_order);

        if (dir_num)
        {
            for (i = 0; i < dir_num; ++i)
                real_new_order[i] = new_order[i];
            g_free (new_order);
        }

        file_list_set_cmp_func (model->priv->files, model->priv->cmp_func, &new_order);

        if (model->priv->files->size)
        {
            for (i = dir_num; i < total; ++i)
                real_new_order[i] = new_order[i - dir_num] + dir_num;
            g_free (new_order);
        }

        g_signal_emit_by_name (model, "rows-reordered", path, NULL, real_new_order);
        g_object_notify (G_OBJECT (model), "sort-case-sensitive");

        g_free (real_new_order);
        gtk_tree_path_free (path);
    }
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

static gboolean model_contains_file     (MooFolderModel *model,
                                         MooFile        *file);
static gboolean model_is_dir            (MooFolderModel *model,
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
#define DEFINE_CHECK_ITER
static void CHECK_ITER (MooFolderModel *model, GtkTreeIter *iter);
#endif
#endif /* DEBUG */

#ifndef DEFINE_CHECK_ITER
#define CHECK_ITER(model,iter)
#endif


static void         moo_folder_model_disconnect_folder  (MooFolderModel *model)
{
    GSList *files;

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
        g_signal_handlers_disconnect_by_func (model->priv->folder,
                                              (gpointer) moo_folder_model_folder_deleted,
                                              model);

        files = file_list_get_slist (model->priv->files);
        moo_folder_model_remove_files (model, files);
        g_slist_foreach (files, (GFunc) moo_file_unref, NULL);
        g_slist_free (files);

        files = file_list_get_slist (model->priv->dirs);
        moo_folder_model_remove_files (model, files);
        g_slist_foreach (files, (GFunc) moo_file_unref, NULL);
        g_slist_free (files);

        g_object_unref (model->priv->folder);
        model->priv->folder = NULL;
    }

}


static void         moo_folder_model_folder_deleted (MooFolderModel *model)
{
    moo_folder_model_disconnect_folder (model);
}

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
    int index_;
    GtkTreePath *path;
    GtkTreeIter iter;
    gboolean is_dir;

    g_assert (!model_contains_file (model, file));
    g_assert (file != NULL);

    moo_file_ref (file);

    if (MOO_FILE_IS_DIR (file))
    {
        is_dir = TRUE;
        index_ = file_list_add (model->priv->dirs, file);
    }
    else
    {
        is_dir = FALSE;
        index_ = file_list_add (model->priv->files, file);
        index_ += model->priv->dirs->size;
    }

    ITER_INIT (&iter, model, file, is_dir);
    CHECK_ITER (model, &iter);

    path = gtk_tree_path_new_from_indices (index_, -1);
    gtk_tree_model_row_inserted (GTK_TREE_MODEL (model), path, &iter);
    gtk_tree_path_free (path);
}


static void     model_change_moo_file   (MooFile        *file,
                                         MooFolderModel *model)
{
    int index_;
    GtkTreePath *path;
    GtkTreeIter iter;
    gboolean is_dir;

    g_assert (model_contains_file (model, file));
    g_assert (file != NULL);

    if (model_is_dir (model, file))
    {
        if (!MOO_FILE_IS_DIR (file))
        {
            moo_file_ref (file);
            model_remove_moo_file (file, model);
            model_add_moo_file (file, model);
            moo_file_unref (file);
            return;
        }

        index_ = file_list_position (model->priv->dirs, file);
        g_assert (index_ >= 0);
        is_dir = TRUE;
    }
    else
    {
        if (MOO_FILE_IS_DIR (file))
        {
            moo_file_ref (file);
            model_remove_moo_file (file, model);
            model_add_moo_file (file, model);
            moo_file_unref (file);
            return;
        }

        index_ = file_list_position (model->priv->files, file);
        g_assert (index_ >= 0);
        index_ += model->priv->dirs->size;
        is_dir = FALSE;
    }

    ITER_INIT (&iter, model, file, is_dir);
    CHECK_ITER (model, &iter);

    path = gtk_tree_path_new_from_indices (index_, -1);
    gtk_tree_model_row_changed (GTK_TREE_MODEL (model), path, &iter);
    gtk_tree_path_free (path);
}


static void     model_remove_moo_file   (MooFile        *file,
                                         MooFolderModel *model)
{
    int index_;
    GtkTreePath *path;

    g_assert (model_contains_file (model, file));
    g_assert (file != NULL);

    moo_file_ref (file);

    if (model_is_dir (model, file))
    {
        index_ = file_list_remove (model->priv->dirs, file);
    }
    else
    {
        index_ = file_list_remove (model->priv->files, file);
        index_ += model->priv->dirs->size;
    }

    path = gtk_tree_path_new_from_indices (index_, -1);
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


static gboolean model_contains_file     (MooFolderModel *model,
                                         MooFile        *file)
{
    return file_list_contains (model->priv->dirs, file) ||
            file_list_contains (model->priv->files, file);
}


static gboolean model_is_dir            (MooFolderModel *model,
                                         MooFile        *file)
{
    return file_list_contains (model->priv->dirs, file);
}


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
    int index_;
    MooFile *file;
    gboolean is_dir;

    g_return_val_if_fail (MOO_IS_FOLDER_MODEL (tree_model), FALSE);
    g_return_val_if_fail (iter != NULL && path != NULL, FALSE);

    model = MOO_FOLDER_MODEL (tree_model);

    if (gtk_tree_path_get_depth (path) != 1)
        g_return_val_if_reached (FALSE);

    index_ = gtk_tree_path_get_indices(path)[0];

    g_return_val_if_fail (index_ >= 0, FALSE);

    if (index_ >= model->priv->files->size + model->priv->dirs->size)
        return FALSE;

    if (index_ < model->priv->dirs->size)
    {
        is_dir = TRUE;
        file = file_list_nth (model->priv->dirs, index_);
    }
    else
    {
        is_dir = FALSE;
        index_ -= model->priv->dirs->size;
        file = file_list_nth (model->priv->files, index_);
    }

    g_assert (file != NULL);
    ITER_INIT (iter, model, file, is_dir);
    CHECK_ITER (model, iter);

    return TRUE;
}


static GtkTreePath *moo_folder_model_get_path       (GtkTreeModel *tree_model,
                                                     GtkTreeIter  *iter)
{
    MooFolderModel *model;
    int index_;

    g_return_val_if_fail (MOO_IS_FOLDER_MODEL (tree_model), NULL);
    g_return_val_if_fail (iter != NULL, NULL);

    model = MOO_FOLDER_MODEL (tree_model);
    g_return_val_if_fail (ITER_MODEL (iter) == model, NULL);
    g_return_val_if_fail (ITER_FILE (iter) != NULL, NULL);

    if (ITER_DIR (iter))
    {
        index_ = file_list_position (model->priv->dirs,
                                     ITER_FILE (iter));
        g_return_val_if_fail (index_ >= 0, NULL);
    }
    else
    {
        index_ = file_list_position (model->priv->files,
                                     ITER_FILE (iter));
        g_return_val_if_fail (index_ >= 0, NULL);
        index_ += model->priv->dirs->size;
    }

    return gtk_tree_path_new_from_indices (index_, -1);
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
    MooFile *next;

    g_return_val_if_fail (MOO_IS_FOLDER_MODEL (tree_model), FALSE);
    g_return_val_if_fail (iter != NULL, FALSE);

    model = MOO_FOLDER_MODEL (tree_model);
    g_return_val_if_fail (ITER_MODEL (iter) == model, FALSE);
    g_return_val_if_fail (ITER_FILE (iter) != NULL, FALSE);

    if (ITER_DIR (iter))
    {
        if ((next = file_list_next (model->priv->dirs, ITER_FILE (iter))))
        {
            ITER_INIT (iter, model, next, TRUE);
            CHECK_ITER (model, iter);
            return TRUE;
        }
        else if ((next = file_list_first (model->priv->files)))
        {
            ITER_INIT (iter, model, next, FALSE);
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
        next = file_list_next (model->priv->files, ITER_FILE (iter));

        if (next)
        {
            ITER_INIT (iter, model, next, FALSE);
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
    MooFile *first;

    g_return_val_if_fail (MOO_IS_FOLDER_MODEL (tree_model), FALSE);

    model = MOO_FOLDER_MODEL (tree_model);

    if (!parent)
    {
        if ((first = file_list_first (model->priv->dirs)))
        {
            ITER_INIT (iter, model, first, TRUE);
            CHECK_ITER (model, iter);
            return TRUE;
        }
        else if ((first = file_list_first (model->priv->files)))
        {
            ITER_INIT (iter, model, first, FALSE);
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
    return model->priv->dirs->size + model->priv->files->size;
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

    if (file_list_contains (model->priv->dirs, file))
    {
        ITER_INIT (iter, model, file, TRUE);
        CHECK_ITER (model, iter);
        return TRUE;
    }
    else if (file_list_contains (model->priv->files, file))
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

    if ((file = file_list_find_name (model->priv->dirs, name)))
    {
        ITER_INIT (iter, model, file, TRUE);
        CHECK_ITER (model, iter);
        return TRUE;
    }
    else if ((file = file_list_find_name (model->priv->files, name)))
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

    if ((file = file_list_find_display_name (model->priv->dirs, name)))
    {
        ITER_INIT (iter, model, file, TRUE);
        CHECK_ITER (model, iter);
        return TRUE;
    }
    else if ((file = file_list_find_display_name (model->priv->files, name)))
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


/***********************************************************************/
/* DnD
 */

static GdkAtom uri_atom (void)
{
    static GdkAtom atom = 0;
    if (!atom)
        atom = gdk_atom_intern ("text/uri-list", FALSE);
    return atom;
}


static gboolean source_drag_data_get    (GtkTreeDragSource  *drag_source,
                                         GtkTreePath        *path,
                                         GtkSelectionData   *selection_data)
{
    GtkTreeIter iter;
    MooFolderModel *model;
    char *file_path;
    static char *uris[2];
    gboolean result;

    model = MOO_FOLDER_MODEL (drag_source);

    if (!gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &iter, path))
        return FALSE;

    g_assert (ITER_MODEL (&iter) == model);
    g_assert (ITER_FILE (&iter) != NULL);

    file_path = g_build_filename (moo_folder_get_path (model->priv->folder),
                                  moo_file_name (ITER_FILE (&iter)),
                                  NULL);

    if (selection_data->target != uri_atom())
    {
        result = gtk_selection_data_set_text (selection_data, file_path, -1);
    }
    else
    {
        uris[0] = g_strdup_printf ("file://%s", file_path);
        uris[1] = NULL;
        result = gtk_selection_data_set_uris (selection_data, uris);
        g_free (uris[0]);
    }

    g_free (file_path);
    return result;
}


static gboolean source_drag_data_delete (GtkTreeDragSource  *drag_source,
                                         GtkTreePath        *path)
{
    g_print ("deleting\n");
    return TRUE;
}


static void moo_folder_model_drag_source_iface_init (GtkTreeDragSourceIface *iface)
{
    iface->drag_data_get = source_drag_data_get;
    iface->drag_data_delete = source_drag_data_delete;
}


static void moo_folder_filter_class_init                (MooFolderFilterClass   *klass);


GType            moo_folder_filter_get_type     (void)
{
    static GType type = 0;

    if (!type)
    {
        static GTypeInfo info = {
            /* interface types, classed types, instantiated types */
            sizeof (MooFolderFilterClass),
            NULL, /* base_init; */
            NULL, /* base_finalize; */
            (GClassInitFunc) moo_folder_filter_class_init,
            NULL, /* class_finalize; */
            NULL, /* class_data; */
            sizeof (MooFolderFilter),
            0, /* n_preallocs; */
//             (GInstanceInitFunc) moo_folder_filter_init,
            NULL,
            NULL /* value_table; */
        };

        type = g_type_register_static (GTK_TYPE_TREE_MODEL_FILTER,
                                       "MooFolderFilter",
                                       &info, 0);
    }

    return type;
}


GtkTreeModel    *moo_folder_filter_new              (MooFolderModel     *model)
{
    g_return_val_if_fail (MOO_IS_FOLDER_MODEL (model), NULL);
    return g_object_new (MOO_TYPE_FOLDER_FILTER,
                         "child-model", model, NULL);
}


static void moo_folder_filter_class_init                (MooFolderFilterClass   *klass)
{
}


void             moo_folder_filter_set_folder       (MooFolderFilter    *filter,
                                                     MooFolder          *folder)
{
    GtkTreeModel *model;
    g_return_if_fail (MOO_IS_FOLDER_FILTER (filter));
    model = gtk_tree_model_filter_get_model (GTK_TREE_MODEL_FILTER (filter));
    g_return_if_fail (MOO_IS_FOLDER_MODEL (model));
    moo_folder_model_set_folder (MOO_FOLDER_MODEL (model), folder);
}


MooFolder       *moo_folder_filter_get_folder       (MooFolderFilter    *filter)
{
    GtkTreeModel *model;
    g_return_val_if_fail (MOO_IS_FOLDER_FILTER (filter), NULL);
    model = gtk_tree_model_filter_get_model (GTK_TREE_MODEL_FILTER (filter));
    g_return_val_if_fail (MOO_IS_FOLDER_MODEL (model), NULL);
    return moo_folder_model_get_folder (MOO_FOLDER_MODEL (model));
}
