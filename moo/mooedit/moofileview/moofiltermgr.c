/*
 *   moofiltermgr.c
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

#include "moofiltermgr.h"
#include <string.h>

#define NUM_USER_FILTERS 5

typedef struct {
    GtkFileFilter   *filter;
    GtkFileFilter   *aux;
    char            *description;
    char            *glob;
    gboolean         user;
} Filter;

static Filter   *filter_new     (const char *description,
                                 const char *glob,
                                 gboolean    user);
static void      filter_free    (Filter     *filter);

static const char       *filter_get_glob        (Filter *filter);
static const char       *filter_get_description (Filter *filter);


typedef struct _RecentStuff RecentStuff;

struct _MooFilterMgrPrivate {
    GtkListStore    *filters;
    Filter          *last_filter;
    Filter          *null_filter;
    guint            num_user_filters;
};

enum {
    COLUMN_DESCRIPTION,
    COLUMN_FILTER,
    NUM_COLUMNS
};


static void     moo_filter_mgr_class_init   (MooFilterMgrClass    *klass);
static void     moo_filter_mgr_init         (MooFilterMgr         *mgr);
static void     moo_filter_mgr_finalize     (GObject                *object);

static void     mgr_load_filter_prefs       (MooFilterMgr   *mgr);
static void     mgr_save_filter_prefs       (MooFilterMgr   *mgr);
static Filter  *mgr_new_user_filter         (MooFilterMgr   *mgr,
                                             const char     *glob);
static Filter  *mgr_get_last_filter         (MooFilterMgr   *mgr);
static Filter  *mgr_get_null_filter         (MooFilterMgr   *mgr);
static void     list_store_init             (MooFilterMgr   *mgr);
static void     list_store_destroy          (MooFilterMgr   *mgr);
static void     list_store_append_filter    (MooFilterMgr   *mgr,
                                             Filter         *filter);
static Filter  *list_store_find_filter      (MooFilterMgr   *mgr,
                                             const char     *text);


/* MOO_TYPE_FILTER_MGR */
G_DEFINE_TYPE (MooFilterMgr, moo_filter_mgr, G_TYPE_OBJECT)

static void moo_filter_mgr_class_init    (MooFilterMgrClass   *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    gobject_class->finalize = moo_filter_mgr_finalize;
}


static void moo_filter_mgr_init          (MooFilterMgr        *mgr)
{
    mgr->priv = g_new0 (MooFilterMgrPrivate, 1);
    list_store_init (mgr);
}


static void moo_filter_mgr_finalize      (GObject            *object)
{
    MooFilterMgr *mgr = MOO_FILTER_MGR (object);
    list_store_destroy (mgr);
    g_free (mgr->priv);
    G_OBJECT_CLASS (moo_filter_mgr_parent_class)->finalize (object);
}


MooFilterMgr  *moo_filter_mgr_new              (void)
{
    return MOO_FILTER_MGR (g_object_new (MOO_TYPE_FILTER_MGR, NULL));
}


static gboolean row_is_separator (GtkTreeModel  *model,
                                  GtkTreeIter   *iter,
                                  G_GNUC_UNUSED gpointer data)
{
    Filter *filter;
    gtk_tree_model_get (model, iter, COLUMN_FILTER, &filter, -1);
    return filter == NULL;
}


void             moo_filter_mgr_init_filter_combo   (MooFilterMgr *mgr,
                                                     GtkComboBox    *combo)
{
    g_return_if_fail (MOO_IS_FILTER_MGR (mgr));
    g_return_if_fail (GTK_IS_COMBO_BOX (combo));

    gtk_combo_box_set_model (combo, GTK_TREE_MODEL (mgr->priv->filters));
    gtk_combo_box_set_row_separator_func (combo, row_is_separator,
                                          NULL, NULL);

    gtk_combo_box_entry_set_text_column (GTK_COMBO_BOX_ENTRY (combo),
                                         COLUMN_DESCRIPTION);
}


static Filter   *mgr_get_null_filter        (MooFilterMgr     *mgr)
{
    if (!mgr->priv->null_filter)
    {
        Filter *filter = filter_new ("All Files", "*", FALSE);
        list_store_append_filter (mgr, filter);
        mgr->priv->null_filter = filter;
    }
    return mgr->priv->null_filter;
}


GtkFileFilter   *moo_filter_mgr_get_null_filter  (MooFilterMgr *mgr)
{
    Filter *filter;
    g_return_val_if_fail (MOO_IS_FILTER_MGR (mgr), NULL);
    filter = mgr_get_null_filter (mgr);
    return filter->filter;
}


static Filter   *mgr_get_last_filter        (MooFilterMgr     *mgr)
{
    mgr_load_filter_prefs (mgr);
    if (!mgr->priv->last_filter)
        mgr->priv->last_filter = mgr_get_null_filter (mgr);
    return mgr->priv->last_filter;
}


GtkFileFilter   *moo_filter_mgr_get_last_filter  (MooFilterMgr *mgr)
{
    Filter *filter;
    g_return_val_if_fail (MOO_IS_FILTER_MGR (mgr), NULL);
    filter = mgr_get_last_filter (mgr);
    if (filter)
        return filter->filter;
    else
        return NULL;
}


static void      list_store_init            (MooFilterMgr     *mgr)
{
    mgr->priv->filters = gtk_list_store_new (NUM_COLUMNS,
                                             G_TYPE_STRING,
                                             G_TYPE_POINTER);
    mgr_get_null_filter (mgr);
}


static gboolean filter_free_func (GtkTreeModel  *model,
                                  G_GNUC_UNUSED GtkTreePath *path,
                                  GtkTreeIter   *iter,
                                  G_GNUC_UNUSED gpointer data)
{
    Filter *filter;
    gtk_tree_model_get (model, iter, COLUMN_FILTER, &filter, -1);
    if (filter)
        filter_free (filter);
    return FALSE;
}

static void      list_store_destroy         (MooFilterMgr     *mgr)
{
    gtk_tree_model_foreach (GTK_TREE_MODEL (mgr->priv->filters),
                            filter_free_func, NULL);
    g_object_unref (mgr->priv->filters);
    mgr->priv->filters = NULL;
    mgr->priv->last_filter = NULL;
    mgr->priv->null_filter = NULL;
}


static void      list_store_append_filter   (MooFilterMgr     *mgr,
                                             Filter             *filter)
{
    GtkTreeIter iter;
    gtk_list_store_append (mgr->priv->filters, &iter);
    if (filter)
        gtk_list_store_set (mgr->priv->filters, &iter,
                            COLUMN_DESCRIPTION, filter_get_description (filter),
                            COLUMN_FILTER, filter, -1);
}


#define NEGATE_CHAR     '!'
#define GLOB_SEPARATOR  ";"

static gboolean neg_filter_func (const GtkFileFilterInfo *filter_info,
                                 Filter *filter)
{
    return !gtk_file_filter_filter (filter->aux, filter_info);
}


static Filter   *filter_new     (const char *description,
                                 const char *glob,
                                 gboolean    user)
{
    Filter *filter;
    char **globs, **p;
    gboolean negative;

    g_return_val_if_fail (description != NULL, NULL);
    g_return_val_if_fail (glob != NULL && glob[0] != 0, NULL);
    g_return_val_if_fail (glob[0] != NEGATE_CHAR || glob[1] != 0, NULL);

    if (glob[0] == NEGATE_CHAR)
    {
        negative = TRUE;
        globs = g_strsplit (glob + 1, GLOB_SEPARATOR, 0);
    }
    else
    {
        negative = FALSE;
        globs = g_strsplit (glob, GLOB_SEPARATOR, 0);
    }

    g_return_val_if_fail (globs != NULL, NULL);

    filter = g_new0 (Filter, 1);

    filter->description = g_strdup (description);
    filter->glob = g_strdup (glob);
    filter->user = user;

    filter->filter = gtk_file_filter_new ();
    gtk_object_sink (GTK_OBJECT (g_object_ref (filter->filter)));
    gtk_file_filter_set_name (filter->filter, description);

    if (negative)
    {
        filter->aux = gtk_file_filter_new ();
        gtk_object_sink (GTK_OBJECT (g_object_ref (filter->aux)));

        for (p = globs; *p != NULL; p++)
            gtk_file_filter_add_pattern (filter->aux, *p);

        gtk_file_filter_add_custom (filter->filter,
                                    gtk_file_filter_get_needed (filter->aux),
                                    (GtkFileFilterFunc) neg_filter_func,
                                    filter, NULL);
    }
    else
    {
        for (p = globs; *p != NULL; p++)
            gtk_file_filter_add_pattern (filter->filter, *p);
    }

    g_strfreev (globs);
    return filter;
}


static void      filter_free    (Filter *filter)
{
    if (filter)
    {
        if (filter->filter)
            g_object_unref (filter->filter);
        filter->filter = NULL;
        if (filter->aux)
            g_object_unref (filter->aux);
        filter->aux = NULL;
        g_free (filter->description);
        filter->description = NULL;
        g_free (filter->glob);
        filter->glob = NULL;
        g_free (filter);
    }
}


static const char       *filter_get_glob        (Filter *filter)
{
    return filter->glob;
}

static const char       *filter_get_description (Filter *filter)
{
    return filter->description;
}


typedef gboolean (*ListFoundFunc) (GtkTreeModel *model,
                                   GtkTreeIter  *iter,
                                   gpointer      data);

typedef struct {
    GtkTreeIter     *iter;
    ListFoundFunc    func;
    gpointer         user_data;
    gboolean         found;
} ListStoreFindData;

static gboolean list_store_find_check_func (GtkTreeModel        *model,
                                            G_GNUC_UNUSED GtkTreePath *path,
                                            GtkTreeIter         *iter,
                                            ListStoreFindData   *data)
{
    if (data->func (model, iter, data->user_data))
    {
        data->found = TRUE;
        if (data->iter)
            *data->iter = *iter;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static gboolean list_store_find     (GtkListStore   *store,
                                     GtkTreeIter    *iter,
                                     ListFoundFunc   func,
                                     gpointer        user_data)
{
    ListStoreFindData data = {iter, func, user_data, FALSE};
    gtk_tree_model_foreach (GTK_TREE_MODEL (store),
                            (GtkTreeModelForeachFunc) list_store_find_check_func,
                            &data);
    return data.found;
}


static gboolean check_filter_match (GtkTreeModel   *model,
                                    GtkTreeIter    *iter,
                                    const char     *text)
{
    Filter *filter = NULL;

    gtk_tree_model_get (model, iter, COLUMN_FILTER, &filter, -1);

    if (!filter)
        return FALSE;

    if (!strcmp (text, filter_get_description (filter)) ||
         !strcmp (text, filter_get_glob (filter)))
            return TRUE;

    return FALSE;
}

static Filter   *list_store_find_filter     (MooFilterMgr *mgr,
                                             const char     *text)
{
    GtkTreeIter iter;

    g_return_val_if_fail (text != NULL, NULL);

    if (list_store_find (mgr->priv->filters, &iter,
                         (ListFoundFunc)check_filter_match,
                         (gpointer)text))
    {
        Filter *filter;
        gtk_tree_model_get (GTK_TREE_MODEL (mgr->priv->filters),
                            &iter, COLUMN_FILTER, &filter, -1);
        return filter;
    }
    else
    {
        return NULL;
    }
}


static void      mgr_load_filter_prefs      (MooFilterMgr *mgr)
{
//     guint i;
//     char *key;
//     char *glob;
//     Filter *filter;

//     for (i = 0; i < NUM_USER_FILTERS; ++i)
//     {
//         key = g_strdup_printf (MOO_EDIT_PREFS_PREFIX "/"
//                                PREFS_FILTERS "/" PREFS_USER "%d", i);
//
//         glob = g_strdup (moo_prefs_get_string (key));
//
//         if (glob && glob[0])
//             mgr_new_user_filter (mgr, glob);
//
//         g_free (key);
//         g_free (glob);
//     }

//     glob = g_strdup (moo_prefs_get_string (moo_edit_setting (PREFS_LAST_FILTER)));
//
//     if (glob && glob[0])
//     {
//         filter = mgr_new_user_filter (mgr, glob);
//
//         if (filter)
//             mgr_set_last_filter (mgr, filter);
//     }
//
//     g_free (glob);
}


static void      mgr_save_filter_prefs      (MooFilterMgr *mgr)
{
//     GtkTreeIter iter;
//     gboolean user_present;
//     guint i = 0;
//
//     user_present = list_store_find (mgr->priv->filters, &iter,
//                                     row_is_separator, NULL);
//
//     if (user_present)
//     {
//         while (i < NUM_USER_FILTERS &&
//                gtk_tree_model_iter_next (GTK_TREE_MODEL (mgr->priv->filters), &iter))
//         {
//             Filter *filter = NULL;
//             gtk_tree_model_get (GTK_TREE_MODEL (mgr->priv->filters), &iter,
//                                 COLUMN_FILTER, &filter, -1);
//             g_assert (filter != NULL);
//             set_user_filter_prefs (i, filter_get_glob (filter));
//             i++;
//         }
//     }
//
//     for ( ; i < NUM_USER_FILTERS; ++i)
//         set_user_filter_prefs (i, NULL);
}


static Filter   *mgr_new_user_filter        (MooFilterMgr *mgr,
                                             const char     *glob)
{
    Filter *filter;

    g_return_val_if_fail (glob && glob[0], NULL);

    filter = list_store_find_filter (mgr, glob);

    if (filter && !filter->user)
        return filter;

    if (filter)
    {
        GtkTreeIter iter;
        gboolean user_present;

        user_present = list_store_find (mgr->priv->filters, &iter,
                                        row_is_separator, NULL);

        g_return_val_if_fail (user_present, filter);
        g_return_val_if_fail (gtk_tree_model_iter_next
                (GTK_TREE_MODEL (mgr->priv->filters), &iter), filter);
        gtk_list_store_move_before (mgr->priv->filters, &iter, NULL);
    }
    else
    {
        filter = filter_new (glob, glob, TRUE);

        if (filter)
        {
            GtkTreeIter iter;
            gboolean user_present;

            user_present = list_store_find (mgr->priv->filters, &iter,
                                            row_is_separator, NULL);

            if (!user_present)
                gtk_list_store_append (mgr->priv->filters, &iter);

            if (mgr->priv->num_user_filters == NUM_USER_FILTERS)
            {
                Filter *old = NULL;

                --mgr->priv->num_user_filters;

                if (!gtk_tree_model_iter_next (GTK_TREE_MODEL (mgr->priv->filters),
                                                &iter))
                {
                    filter_free (filter);
                    g_return_val_if_reached (NULL);
                }

                gtk_tree_model_get (GTK_TREE_MODEL (mgr->priv->filters),
                                    &iter, COLUMN_FILTER, &old, -1);
                g_assert (old != NULL && old != mgr->priv->last_filter);
                gtk_list_store_remove (mgr->priv->filters, &iter);
                filter_free (old);
            }

            gtk_list_store_append (mgr->priv->filters, &iter);
            gtk_list_store_set (mgr->priv->filters, &iter,
                                COLUMN_FILTER, filter,
                                COLUMN_DESCRIPTION, filter_get_description (filter),
                                -1);
            ++mgr->priv->num_user_filters;
        }
    }

    mgr_save_filter_prefs (mgr);

    return filter;
}


GtkFileFilter   *moo_filter_mgr_get_filter          (MooFilterMgr *mgr,
                                                     GtkTreeIter    *iter)
{
    Filter *filter = NULL;

    g_return_val_if_fail (MOO_IS_FILTER_MGR (mgr), NULL);
    g_return_val_if_fail (iter != NULL, NULL);

    gtk_tree_model_get (GTK_TREE_MODEL (mgr->priv->filters), iter,
                        COLUMN_FILTER, &filter, -1);
    g_return_val_if_fail (filter != NULL, NULL);

    return filter->filter;
}


GtkFileFilter   *moo_filter_mgr_new_user_filter     (MooFilterMgr *mgr,
                                                     const char     *text)
{
    Filter *filter;

    g_return_val_if_fail (MOO_IS_FILTER_MGR (mgr), NULL);
    g_return_val_if_fail (text && text[0], NULL);

    filter = mgr_new_user_filter (mgr, text);

    if (filter)
        return filter->filter;
    else
        return NULL;
}
