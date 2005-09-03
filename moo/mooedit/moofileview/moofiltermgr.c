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

#define NUM_USER_FILTERS  5
#define ALL_FILES_GLOB    "*"

typedef struct {
    GtkFileFilter   *filter;
    GtkFileFilter   *aux;
    char            *description;
    char            *glob;
    gboolean         user;
} Filter;

static Filter       *filter_new             (const char *description,
                                             const char *glob,
                                             gboolean    user);
static void          filter_free            (Filter     *filter);

static const char   *filter_get_glob        (Filter     *filter);
static const char   *filter_get_description (Filter     *filter);
static GtkFileFilter *filter_get_gtk_filter (Filter     *filter);

typedef struct {
    GtkListStore    *filters;
    Filter          *last_filter;
    guint            num_user;
    guint            num_builtin;
    gboolean         has_separator;
    guint            max_num_user;
} FilterStore;

static FilterStore  *filter_store_new           (MooFilterMgr   *mgr);
static void          filter_store_free          (FilterStore    *store);

static void          filter_store_add_builtin   (FilterStore    *store,
                                                 Filter         *filter,
                                                 int             position);
static void          filter_store_add_user      (FilterStore    *store,
                                                 Filter         *filter);
static Filter       *filter_store_get_last      (FilterStore    *store);


struct _MooFilterMgrPrivate {
    FilterStore *default_store;
    GHashTable  *named_stores;  /* user_id -> FilterStore* */
    GHashTable  *filters;       /* glob -> Filter* */
};

enum {
    COLUMN_DESCRIPTION,
    COLUMN_FILTER,
    NUM_COLUMNS
};


static void     moo_filter_mgr_class_init   (MooFilterMgrClass    *klass);
static void     moo_filter_mgr_init         (MooFilterMgr         *mgr);
static void     moo_filter_mgr_finalize     (GObject                *object);


static gboolean      combo_row_separator_func   (GtkTreeModel  *model,
                                                 GtkTreeIter   *iter,
                                                 gpointer       data);
static FilterStore  *mgr_get_store              (MooFilterMgr   *mgr,
                                                 const char     *user_id,
                                                 gboolean        create);
static Filter       *mgr_get_null_filter        (MooFilterMgr   *mgr);
static Filter       *mgr_new_filter             (MooFilterMgr   *mgr,
                                                 const char     *description,
                                                 const char     *glob,
                                                 gboolean        user);
static Filter       *mgr_get_last_filter        (MooFilterMgr   *mgr,
                                                 const char     *user_id);
static void          mgr_set_last_filter        (MooFilterMgr   *mgr,
                                                 const char     *user_id,
                                                 Filter         *filter);
static Filter       *mgr_new_user_filter        (MooFilterMgr   *mgr,
                                                 const char     *glob,
                                                 const char     *user_id);


/* MOO_TYPE_FILTER_MGR */
G_DEFINE_TYPE (MooFilterMgr, moo_filter_mgr, G_TYPE_OBJECT)


static void
moo_filter_mgr_class_init (MooFilterMgrClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    gobject_class->finalize = moo_filter_mgr_finalize;
}


static void
moo_filter_mgr_init (MooFilterMgr *mgr)
{
    mgr->priv = g_new0 (MooFilterMgrPrivate, 1);

    mgr->priv->filters =
            g_hash_table_new_full (g_str_hash, g_str_equal,
                                   g_free, (GDestroyNotify) filter_free);
    g_hash_table_insert (mgr->priv->filters,
                         g_strdup (ALL_FILES_GLOB),
                         filter_new ("All Files", ALL_FILES_GLOB, FALSE));

    mgr->priv->default_store = NULL;
    mgr->priv->named_stores =
            g_hash_table_new_full (g_str_hash, g_str_equal,
                                   g_free, (GDestroyNotify) filter_store_free);
}


static void
moo_filter_mgr_finalize (GObject *object)
{
    MooFilterMgr *mgr = MOO_FILTER_MGR (object);

    filter_store_free (mgr->priv->default_store);
    g_hash_table_destroy (mgr->priv->named_stores);
    g_hash_table_destroy (mgr->priv->filters);

    g_free (mgr->priv);

    G_OBJECT_CLASS (moo_filter_mgr_parent_class)->finalize (object);
}


MooFilterMgr*
moo_filter_mgr_new (void)
{
    return g_object_new (MOO_TYPE_FILTER_MGR, NULL);
}


void
moo_filter_mgr_init_filter_combo (MooFilterMgr   *mgr,
                                  GtkComboBox    *combo,
                                  const char     *user_id)
{
    FilterStore *store;

    g_return_if_fail (MOO_IS_FILTER_MGR (mgr));
    g_return_if_fail (GTK_IS_COMBO_BOX (combo));

    store = mgr_get_store (mgr, user_id, TRUE);
    g_return_if_fail (store != NULL);

    gtk_combo_box_set_model (combo, GTK_TREE_MODEL (store->filters));
    gtk_combo_box_set_row_separator_func (combo, combo_row_separator_func,
                                          NULL, NULL);

    if (GTK_IS_COMBO_BOX_ENTRY (combo))
        gtk_combo_box_entry_set_text_column (GTK_COMBO_BOX_ENTRY (combo),
                                             COLUMN_DESCRIPTION);
}


static gboolean
combo_row_separator_func (GtkTreeModel  *model,
                          GtkTreeIter   *iter,
                          G_GNUC_UNUSED gpointer data)
{
    Filter *filter = NULL;
    gtk_tree_model_get (model, iter, COLUMN_FILTER, &filter, -1);
    return !filter;
}


static FilterStore*
mgr_get_store (MooFilterMgr   *mgr,
               const char     *user_id,
               gboolean        create)
{
    FilterStore *store;

    if (!user_id)
    {
        if (!mgr->priv->default_store && create)
            mgr->priv->default_store = filter_store_new (mgr);
        return mgr->priv->default_store;
    }

    store = g_hash_table_lookup (mgr->priv->named_stores, user_id);

    if (!store && create)
    {
        g_return_val_if_fail (g_utf8_validate (user_id, -1, NULL), NULL);
        store = filter_store_new (mgr);
        g_hash_table_insert (mgr->priv->named_stores,
                             g_strdup (user_id),
                             store);
    }

    return store;
}


GtkFileFilter*
moo_filter_mgr_get_filter (MooFilterMgr   *mgr,
                           GtkTreeIter    *iter,
                           const char     *user_id)
{
    FilterStore *store;
    Filter *filter = NULL;

    g_return_val_if_fail (MOO_IS_FILTER_MGR (mgr), NULL);
    g_return_val_if_fail (iter != NULL, NULL);

    store = mgr_get_store (mgr, user_id, FALSE);
    g_return_val_if_fail (store != NULL, NULL);

    gtk_tree_model_get (GTK_TREE_MODEL (store->filters), iter, &filter, -1);

    return filter_get_gtk_filter (filter);
}


GtkFileFilter*
moo_filter_mgr_get_null_filter (MooFilterMgr *mgr)
{
    g_return_val_if_fail (MOO_IS_FILTER_MGR (mgr), NULL);
    return mgr_get_null_filter(mgr)->filter;
}


static Filter*
mgr_get_null_filter (MooFilterMgr *mgr)
{
    return g_hash_table_lookup (mgr->priv->filters, ALL_FILES_GLOB);
}


GtkFileFilter*
moo_filter_mgr_get_last_filter (MooFilterMgr   *mgr,
                                const char     *user_id)
{
    g_return_val_if_fail (MOO_IS_FILTER_MGR (mgr), NULL);
    return filter_get_gtk_filter (mgr_get_last_filter (mgr, user_id));
}


static Filter*
mgr_get_last_filter (MooFilterMgr   *mgr,
                     const char     *user_id)
{
    FilterStore *store;

    g_return_val_if_fail (MOO_IS_FILTER_MGR (mgr), NULL);

    store = mgr_get_store (mgr, user_id, FALSE);

    if (store)
        return filter_store_get_last (store);
    else
        return NULL;
}


static void
mgr_set_last_filter (MooFilterMgr   *mgr,
                     const char     *user_id,
                     Filter         *filter)
{
    FilterStore *store;

    g_return_if_fail (MOO_IS_FILTER_MGR (mgr));

    store = mgr_get_store (mgr, user_id, TRUE);
    store->last_filter = filter;
}


GtkFileFilter*
moo_filter_mgr_new_user_filter (MooFilterMgr   *mgr,
                                const char     *glob,
                                const char     *user_id)
{
    g_return_val_if_fail (MOO_IS_FILTER_MGR (mgr), NULL);
    g_return_val_if_fail (glob != NULL, NULL);
    return filter_get_gtk_filter (mgr_new_user_filter (mgr, glob, user_id));
}


static Filter*
mgr_new_user_filter (MooFilterMgr   *mgr,
                     const char     *glob,
                     const char     *user_id)
{
    FilterStore *store;
    Filter *filter;

    filter = mgr_new_filter (mgr, glob, glob, TRUE);
    g_return_val_if_fail (filter != NULL, NULL);

    store = mgr_get_store (mgr, user_id, TRUE);
    filter_store_add_user (store, filter);
    return filter;
}


GtkFileFilter*
moo_filter_mgr_new_builtin_filter (MooFilterMgr   *mgr,
                                   const char     *description,
                                   const char     *glob,
                                   const char     *user_id,
                                   int             position)
{
    FilterStore *store;
    Filter *filter;

    g_return_val_if_fail (MOO_IS_FILTER_MGR (mgr), NULL);
    g_return_val_if_fail (glob != NULL, NULL);
    g_return_val_if_fail (description != NULL, NULL);

    filter = mgr_new_filter (mgr, description, glob, FALSE);
    g_return_val_if_fail (filter != NULL, NULL);

    store = mgr_get_store (mgr, user_id, TRUE);
    filter_store_add_builtin (store, filter, position);
    return filter->filter;
}


static Filter*
mgr_new_filter (MooFilterMgr   *mgr,
                const char     *description,
                const char     *glob,
                gboolean        user)
{
    Filter *filter;

    g_return_val_if_fail (description != NULL, NULL);
    g_return_val_if_fail (glob != NULL, NULL);

    filter = g_hash_table_lookup (mgr->priv->filters, glob);

    if (!filter)
    {
        filter = filter_new (description, glob, user);
        g_return_val_if_fail (filter != NULL, NULL);
        g_hash_table_insert (mgr->priv->filters,
                             g_strdup (glob), filter);
    }

    return filter;
}


/*****************************************************************************/
/* Filechooser
 */

static void
dialog_set_filter (MooFilterMgr   *mgr,
                   GtkFileChooser *dialog,
                   Filter         *filter)
{
    GtkEntry *entry = g_object_get_data (G_OBJECT (dialog),
                                         "moo-filter-entry");
    gtk_entry_set_text (entry, filter_get_description (filter));
    gtk_file_chooser_set_filter (dialog, filter_get_gtk_filter (filter));
    mgr_set_last_filter (mgr,
                         g_object_get_data (G_OBJECT (dialog), "moo-filter-user-id"),
                         filter);
}


static void
filter_entry_activated (GtkEntry       *entry,
                        GtkFileChooser *dialog)
{
    const char *text;
    Filter *filter = NULL;
    MooFilterMgr *mgr;

    mgr = g_object_get_data (G_OBJECT (dialog), "moo-filter-mgr");
    g_return_if_fail (MOO_IS_FILTER_MGR (mgr));

    text = gtk_entry_get_text (entry);

    if (text && text[0])
        filter = mgr_new_user_filter (mgr, text,
                                      g_object_get_data (G_OBJECT (dialog),
                                              "moo-filter-user-id"));

    if (!filter)
        filter = mgr_get_null_filter (mgr);

    dialog_set_filter (mgr, dialog, filter);
}


static void
combo_changed (GtkComboBox    *combo,
               GtkFileChooser *dialog)
{
    GtkTreeIter iter;
    Filter *filter;
    MooFilterMgr *mgr;
    FilterStore *store;

    if (!gtk_combo_box_get_active_iter (combo, &iter))
        return;

    mgr = g_object_get_data (G_OBJECT (dialog), "moo-filter-mgr");
    g_return_if_fail (MOO_IS_FILTER_MGR (mgr));

    store = mgr_get_store (mgr,
                           g_object_get_data (G_OBJECT (dialog),
                                              "moo-filter-user-id"),
                           FALSE);
    g_return_if_fail (store != NULL);

    gtk_tree_model_get (GTK_TREE_MODEL (store->filters), &iter,
                        COLUMN_FILTER, &filter, -1);
    g_return_if_fail (filter != NULL);

    dialog_set_filter (mgr, dialog, filter);
}


void
moo_filter_mgr_attach (MooFilterMgr   *mgr,
                       GtkFileChooser *dialog,
                       const char     *user_id)
{
    GtkWidget *alignment;
    GtkWidget *hbox;
    GtkWidget *label;
    GtkWidget *combo;
    GtkWidget *entry;
    Filter *filter;

    g_return_if_fail (MOO_IS_FILTER_MGR (mgr));
    g_return_if_fail (GTK_IS_FILE_CHOOSER (dialog));

    alignment = gtk_alignment_new (1, 0.5, 0, 1);
    gtk_widget_show (alignment);
    gtk_file_chooser_set_extra_widget (GTK_FILE_CHOOSER (dialog), alignment);

    hbox = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (hbox);
    gtk_container_add (GTK_CONTAINER (alignment), hbox);

    label = gtk_label_new_with_mnemonic ("_Filter:");
    gtk_widget_show (label);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

    combo = gtk_combo_box_entry_new ();
    moo_filter_mgr_init_filter_combo (mgr, GTK_COMBO_BOX (combo), user_id);
    gtk_widget_show (combo);
    gtk_box_pack_start (GTK_BOX (hbox), combo, TRUE, TRUE, 0);

    entry = GTK_BIN(combo)->child;
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), entry);

    g_signal_connect (entry, "activate",
                      G_CALLBACK (filter_entry_activated),
                      dialog);
    g_signal_connect (combo, "changed",
                      G_CALLBACK (combo_changed),
                      dialog);

    g_object_set_data (G_OBJECT (dialog), "moo-filter-combo", combo);
    g_object_set_data (G_OBJECT (dialog), "moo-filter-entry", entry);
    g_object_set_data_full (G_OBJECT (dialog), "moo-filter-mgr",
                            g_object_ref (mgr), g_object_unref);
    g_object_set_data_full  (G_OBJECT (dialog), "moo-filter-user-id",
                             g_strdup (user_id), g_free);

    filter = mgr_get_last_filter (mgr, user_id);

    if (filter)
        dialog_set_filter (mgr, GTK_FILE_CHOOSER (dialog), filter);
}


/*****************************************************************************/
/* FilterStore
 */

static FilterStore*
filter_store_new (MooFilterMgr *mgr)
{
    FilterStore *store = g_new0 (FilterStore, 1);

    store->filters = gtk_list_store_new (NUM_COLUMNS,
                                         G_TYPE_STRING,
                                         G_TYPE_POINTER);
    store->last_filter = NULL;
    store->num_user = 0;
    store->num_builtin = 0;
    store->has_separator = FALSE;
    store->max_num_user = NUM_USER_FILTERS;

    filter_store_add_builtin (store, mgr_get_null_filter (mgr), -1);

    return store;
}


static void
filter_store_free (FilterStore *store)
{
    if (store)
    {
        g_object_unref (store->filters);
        g_free (store);
    }
}


static void
filter_store_add_builtin (FilterStore    *store,
                          Filter         *filter,
                          int             position)
{
    GtkTreeIter iter;

    g_return_if_fail (store != NULL);
    g_return_if_fail (filter != NULL);

    store->last_filter = filter;

    if (position < 0 || position > (int) store->num_builtin)
        position = store->num_builtin;

    gtk_list_store_insert (store->filters, &iter, position);
    gtk_list_store_set (store->filters, &iter,
                        COLUMN_DESCRIPTION, filter_get_description (filter),
                        COLUMN_FILTER, filter, -1);
    store->num_builtin++;

    if (store->num_user && !store->has_separator)
    {
        gtk_list_store_insert (store->filters, &iter, store->num_builtin);
        store->has_separator = TRUE;
    }
}


static gboolean
filter_store_find_user (FilterStore    *store,
                        Filter         *filter,
                        GtkTreeIter    *iter)
{
    guint i, offset;
    Filter *filt = NULL;

    offset = store->num_builtin + (store->has_separator ? 1 : 0);

    for (i = offset; i < offset + store->num_user; ++i)
    {
        gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (store->filters),
                                       iter, NULL, i);
        gtk_tree_model_get (GTK_TREE_MODEL (store->filters), iter,
                            COLUMN_FILTER, &filt, -1);
        if (filter == filt)
            return TRUE;
    }

    return FALSE;
}


static gboolean
filter_store_find_builtin (FilterStore    *store,
                           Filter         *filter,
                           GtkTreeIter    *iter)
{
    guint i;
    Filter *filt = NULL;

    for (i = 0; i < store->num_builtin; ++i)
    {
        gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (store->filters),
                                       iter, NULL, i);
        gtk_tree_model_get (GTK_TREE_MODEL (store->filters), iter,
                            COLUMN_FILTER, &filt, -1);
        if (filter == filt)
            return TRUE;
    }

    return FALSE;
}


static void
filter_store_add_user (FilterStore    *store,
                       Filter         *filter)
{
    GtkTreeIter iter;

    g_return_if_fail (store != NULL);
    g_return_if_fail (filter != NULL);

    store->last_filter = filter;

    if (filter_store_find_builtin (store, filter, &iter))
        return;

    if (filter_store_find_user (store, filter, &iter))
    {
        gtk_list_store_remove (store->filters, &iter);
        store->num_user--;
    }

    gtk_list_store_insert (store->filters, &iter,
                           store->num_builtin + (store->has_separator ? 1 : 0));
    gtk_list_store_set (store->filters, &iter,
                        COLUMN_DESCRIPTION, filter_get_description (filter),
                        COLUMN_FILTER, filter, -1);
    store->num_user++;

    if (store->num_builtin && !store->has_separator)
    {
        gtk_list_store_insert (store->filters, &iter, store->num_builtin);
        store->has_separator = TRUE;
    }

    if (store->num_user > store->max_num_user)
    {
        gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (store->filters),
                                       &iter, NULL,
                                       store->num_builtin +
                                               (store->has_separator ? 1 : 0) +
                                               store->num_user - 1);
        gtk_list_store_remove (store->filters, &iter);
        store->num_user--;
    }
}


static Filter       *filter_store_get_last      (FilterStore    *store)
{
    if (!store->last_filter)
    {
        GtkTreeIter iter;
        Filter *filter = NULL;

        gtk_tree_model_get_iter_first (GTK_TREE_MODEL (store->filters), &iter);
        gtk_tree_model_get (GTK_TREE_MODEL (store->filters), &iter,
                            COLUMN_FILTER, &filter, -1);
        store->last_filter = filter;
    }

    return store->last_filter;
}


/*****************************************************************************/
/* Filter
 */

#define NEGATE_CHAR     '!'
#define GLOB_SEPARATOR  ";"

static gboolean
neg_filter_func (const GtkFileFilterInfo *filter_info,
                 Filter *filter)
{
    return !gtk_file_filter_filter (filter->aux, filter_info);
}


static Filter*
filter_new (const char *description,
            const char *glob,
            gboolean    user)
{
    Filter *filter;
    char **globs, **p;
    gboolean negative;

    g_return_val_if_fail (description != NULL, NULL);
    g_return_val_if_fail (g_utf8_validate (description, -1, NULL), NULL);
    g_return_val_if_fail (g_utf8_validate (glob, -1, NULL), NULL);
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


static void
filter_free (Filter *filter)
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


static GtkFileFilter *filter_get_gtk_filter (Filter     *filter)
{
    g_return_val_if_fail (filter != NULL, NULL);
    return filter->filter;
}


// static const char*
// filter_get_glob (Filter *filter)
// {
//     g_return_val_if_fail (filter != NULL, NULL);
//     return filter->glob;
// }


static const char*
filter_get_description (Filter *filter)
{
    g_return_val_if_fail (filter != NULL, NULL);
    return filter->description;
}


// void             moo_filter_mgr_init_filter_combo   (MooFilterMgr *mgr,
//                                                      GtkComboBox    *combo)
// {
//     g_return_if_fail (MOO_IS_FILTER_MGR (mgr));
//     g_return_if_fail (GTK_IS_COMBO_BOX (combo));
//
//     gtk_combo_box_set_model (combo, GTK_TREE_MODEL (mgr->priv->filters));
//     gtk_combo_box_set_row_separator_func (combo, row_is_separator,
//                                           NULL, NULL);
//
//     gtk_combo_box_entry_set_text_column (GTK_COMBO_BOX_ENTRY (combo),
//                                          COLUMN_DESCRIPTION);
// }


// static Filter   *mgr_get_null_filter        (MooFilterMgr     *mgr)
// {
//     if (!mgr->priv->null_filter)
//     {
//         Filter *filter = filter_new ("All Files", "*", FALSE);
//         list_store_append_filter (mgr, filter);
//         mgr->priv->null_filter = filter;
//     }
//     return mgr->priv->null_filter;
// }


// GtkFileFilter   *moo_filter_mgr_get_null_filter  (MooFilterMgr *mgr)
// {
//     Filter *filter;
//     g_return_val_if_fail (MOO_IS_FILTER_MGR (mgr), NULL);
//     filter = mgr_get_null_filter (mgr);
//     return filter->filter;
// }


// static Filter   *mgr_get_last_filter        (MooFilterMgr     *mgr)
// {
//     mgr_load_filter_prefs (mgr);
//     if (!mgr->priv->last_filter)
//         mgr->priv->last_filter = mgr_get_null_filter (mgr);
//     return mgr->priv->last_filter;
// }


// GtkFileFilter   *moo_filter_mgr_get_last_filter  (MooFilterMgr *mgr)
// {
//     Filter *filter;
//     g_return_val_if_fail (MOO_IS_FILTER_MGR (mgr), NULL);
//     filter = mgr_get_last_filter (mgr);
//     if (filter)
//         return filter->filter;
//     else
//         return NULL;
// }


// static void      list_store_init            (MooFilterMgr     *mgr)
// {
//     mgr->priv->filters = gtk_list_store_new (NUM_COLUMNS,
//                                              G_TYPE_STRING,
//                                              G_TYPE_POINTER);
//     mgr_get_null_filter (mgr);
// }


// static gboolean filter_free_func (GtkTreeModel  *model,
//                                   G_GNUC_UNUSED GtkTreePath *path,
//                                   GtkTreeIter   *iter,
//                                   G_GNUC_UNUSED gpointer data)
// {
//     Filter *filter;
//     gtk_tree_model_get (model, iter, COLUMN_FILTER, &filter, -1);
//     if (filter)
//         filter_free (filter);
//     return FALSE;
// }

// static void      list_store_destroy         (MooFilterMgr     *mgr)
// {
//     gtk_tree_model_foreach (GTK_TREE_MODEL (mgr->priv->filters),
//                             filter_free_func, NULL);
//     g_object_unref (mgr->priv->filters);
//     mgr->priv->filters = NULL;
//     mgr->priv->last_filter = NULL;
//     mgr->priv->null_filter = NULL;
// }


// static void      list_store_append_filter   (MooFilterMgr     *mgr,
//                                              Filter             *filter)
// {
//     GtkTreeIter iter;
//     gtk_list_store_append (mgr->priv->filters, &iter);
//     if (filter)
//         gtk_list_store_set (mgr->priv->filters, &iter,
//                             COLUMN_DESCRIPTION, filter_get_description (filter),
//                             COLUMN_FILTER, filter, -1);
// }


//
//
//
//
//
// typedef gboolean (*ListFoundFunc) (GtkTreeModel *model,
//                                    GtkTreeIter  *iter,
//                                    gpointer      data);
//
// typedef struct {
//     GtkTreeIter     *iter;
//     ListFoundFunc    func;
//     gpointer         user_data;
//     gboolean         found;
// } ListStoreFindData;
//
// static gboolean list_store_find_check_func (GtkTreeModel        *model,
//                                             G_GNUC_UNUSED GtkTreePath *path,
//                                             GtkTreeIter         *iter,
//                                             ListStoreFindData   *data)
// {
//     if (data->func (model, iter, data->user_data))
//     {
//         data->found = TRUE;
//         if (data->iter)
//             *data->iter = *iter;
//         return TRUE;
//     }
//     else
//     {
//         return FALSE;
//     }
// }
//
// static gboolean list_store_find     (GtkListStore   *store,
//                                      GtkTreeIter    *iter,
//                                      ListFoundFunc   func,
//                                      gpointer        user_data)
// {
//     ListStoreFindData data = {iter, func, user_data, FALSE};
//     gtk_tree_model_foreach (GTK_TREE_MODEL (store),
//                             (GtkTreeModelForeachFunc) list_store_find_check_func,
//                             &data);
//     return data.found;
// }
//
//
// static gboolean check_filter_match (GtkTreeModel   *model,
//                                     GtkTreeIter    *iter,
//                                     const char     *text)
// {
//     Filter *filter = NULL;
//
//     gtk_tree_model_get (model, iter, COLUMN_FILTER, &filter, -1);
//
//     if (!filter)
//         return FALSE;
//
//     if (!strcmp (text, filter_get_description (filter)) ||
//          !strcmp (text, filter_get_glob (filter)))
//             return TRUE;
//
//     return FALSE;
// }
//
// static Filter   *list_store_find_filter     (MooFilterMgr *mgr,
//                                              const char     *text)
// {
//     GtkTreeIter iter;
//
//     g_return_val_if_fail (text != NULL, NULL);
//
//     if (list_store_find (mgr->priv->filters, &iter,
//                          (ListFoundFunc)check_filter_match,
//                          (gpointer)text))
//     {
//         Filter *filter;
//         gtk_tree_model_get (GTK_TREE_MODEL (mgr->priv->filters),
//                             &iter, COLUMN_FILTER, &filter, -1);
//         return filter;
//     }
//     else
//     {
//         return NULL;
//     }
// }
//
//
// static void      mgr_load_filter_prefs      (MooFilterMgr *mgr)
// {
// //     guint i;
// //     char *key;
// //     char *glob;
// //     Filter *filter;
//
// //     for (i = 0; i < NUM_USER_FILTERS; ++i)
// //     {
// //         key = g_strdup_printf (MOO_EDIT_PREFS_PREFIX "/"
// //                                PREFS_FILTERS "/" PREFS_USER "%d", i);
// //
// //         glob = g_strdup (moo_prefs_get_string (key));
// //
// //         if (glob && glob[0])
// //             mgr_new_user_filter (mgr, glob);
// //
// //         g_free (key);
// //         g_free (glob);
// //     }
//
// //     glob = g_strdup (moo_prefs_get_string (moo_edit_setting (PREFS_LAST_FILTER)));
// //
// //     if (glob && glob[0])
// //     {
// //         filter = mgr_new_user_filter (mgr, glob);
// //
// //         if (filter)
// //             mgr_set_last_filter (mgr, filter);
// //     }
// //
// //     g_free (glob);
// }
//
//
// static void      mgr_save_filter_prefs      (MooFilterMgr *mgr)
// {
// //     GtkTreeIter iter;
// //     gboolean user_present;
// //     guint i = 0;
// //
// //     user_present = list_store_find (mgr->priv->filters, &iter,
// //                                     row_is_separator, NULL);
// //
// //     if (user_present)
// //     {
// //         while (i < NUM_USER_FILTERS &&
// //                gtk_tree_model_iter_next (GTK_TREE_MODEL (mgr->priv->filters), &iter))
// //         {
// //             Filter *filter = NULL;
// //             gtk_tree_model_get (GTK_TREE_MODEL (mgr->priv->filters), &iter,
// //                                 COLUMN_FILTER, &filter, -1);
// //             g_assert (filter != NULL);
// //             set_user_filter_prefs (i, filter_get_glob (filter));
// //             i++;
// //         }
// //     }
// //
// //     for ( ; i < NUM_USER_FILTERS; ++i)
// //         set_user_filter_prefs (i, NULL);
// }
//
//
// static Filter   *mgr_new_user_filter        (MooFilterMgr *mgr,
//                                              const char     *glob)
// {
//     Filter *filter;
//
//     g_return_val_if_fail (glob && glob[0], NULL);
//
//     filter = list_store_find_filter (mgr, glob);
//
//     if (filter && !filter->user)
//         return filter;
//
//     if (filter)
//     {
//         GtkTreeIter iter;
//         gboolean user_present;
//
//         user_present = list_store_find (mgr->priv->filters, &iter,
//                                         row_is_separator, NULL);
//
//         g_return_val_if_fail (user_present, filter);
//         g_return_val_if_fail (gtk_tree_model_iter_next
//                 (GTK_TREE_MODEL (mgr->priv->filters), &iter), filter);
//         gtk_list_store_move_before (mgr->priv->filters, &iter, NULL);
//     }
//     else
//     {
//         filter = filter_new (glob, glob, TRUE);
//
//         if (filter)
//         {
//             GtkTreeIter iter;
//             gboolean user_present;
//
//             user_present = list_store_find (mgr->priv->filters, &iter,
//                                             row_is_separator, NULL);
//
//             if (!user_present)
//                 gtk_list_store_append (mgr->priv->filters, &iter);
//
//             if (mgr->priv->num_user_filters == NUM_USER_FILTERS)
//             {
//                 Filter *old = NULL;
//
//                 --mgr->priv->num_user_filters;
//
//                 if (!gtk_tree_model_iter_next (GTK_TREE_MODEL (mgr->priv->filters),
//                                                 &iter))
//                 {
//                     filter_free (filter);
//                     g_return_val_if_reached (NULL);
//                 }
//
//                 gtk_tree_model_get (GTK_TREE_MODEL (mgr->priv->filters),
//                                     &iter, COLUMN_FILTER, &old, -1);
//                 g_assert (old != NULL && old != mgr->priv->last_filter);
//                 gtk_list_store_remove (mgr->priv->filters, &iter);
//                 filter_free (old);
//             }
//
//             gtk_list_store_append (mgr->priv->filters, &iter);
//             gtk_list_store_set (mgr->priv->filters, &iter,
//                                 COLUMN_FILTER, filter,
//                                 COLUMN_DESCRIPTION, filter_get_description (filter),
//                                 -1);
//             ++mgr->priv->num_user_filters;
//         }
//     }
//
//     mgr_save_filter_prefs (mgr);
//
//     return filter;
// }
//
//
// GtkFileFilter   *moo_filter_mgr_get_filter          (MooFilterMgr *mgr,
//                                                      GtkTreeIter    *iter)
// {
//     Filter *filter = NULL;
//
//     g_return_val_if_fail (MOO_IS_FILTER_MGR (mgr), NULL);
//     g_return_val_if_fail (iter != NULL, NULL);
//
//     gtk_tree_model_get (GTK_TREE_MODEL (mgr->priv->filters), iter,
//                         COLUMN_FILTER, &filter, -1);
//     g_return_val_if_fail (filter != NULL, NULL);
//
//     return filter->filter;
// }
//
//
// GtkFileFilter   *moo_filter_mgr_new_user_filter     (MooFilterMgr *mgr,
//                                                      const char     *text)
// {
//     Filter *filter;
//
//     g_return_val_if_fail (MOO_IS_FILTER_MGR (mgr), NULL);
//     g_return_val_if_fail (text && text[0], NULL);
//
//     filter = mgr_new_user_filter (mgr, text);
//
//     if (filter)
//         return filter->filter;
//     else
//         return NULL;
// }
