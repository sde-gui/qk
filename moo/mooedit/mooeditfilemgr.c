/*
 *   mooedit/mooeditfilemgr.c
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

#include "mooedit/mooeditfilemgr.h"
#include "mooedit/mooeditprefs.h"


typedef struct {
    MooEditFileInfo *info;
} RecentEntry;

static RecentEntry  *recent_entry_new   (const char     *filename);
static RecentEntry  *recent_entry_copy  (RecentEntry    *entry);
static void          recent_entry_free  (RecentEntry    *entry);


typedef struct {
    GtkFileFilter   *filter;
    char            *description;
    gboolean         user;
    guint            ref_count;
} Filter;

static Filter   *filter_new     (void);
static Filter   *filter_ref     (Filter *filter);
static void      filter_unref   (Filter *filter);


struct _MooEditFileMgrPrivate {
    GSList          *recent_files;  /* RecentEntry* */
    GtkListStore    *filters;
    Filter          *last_filter;
    Filter          *null_filter;
};

enum {
    COLUMN_DESCRIPTION,
    COLUMN_FILTER,
    NUM_COLUMNS
};


static void moo_edit_file_mgr_class_init    (MooEditFileMgrClass    *klass);
static void moo_edit_file_mgr_init          (MooEditFileMgr         *mgr);
static void moo_edit_file_mgr_finalize      (GObject                *object);

static Filter   *mgr_get_last_filter        (MooEditFileMgr *mgr);
static Filter   *mgr_get_null_filter        (MooEditFileMgr *mgr);
static void      list_store_init            (MooEditFileMgr *mgr);
static void      list_store_destroy         (MooEditFileMgr *mgr);
static void      list_store_append_filter   (MooEditFileMgr *mgr,
                                             Filter         *filter);
static Filter   *list_store_find_filter     (MooEditFileMgr *mgr,
                                             const char     *text);
static void      list_store_add_user        (MooEditFileMgr *mgr,
                                             Filter         *filter);


/* MOO_TYPE_EDIT_FILE_MGR */
G_DEFINE_TYPE (MooEditFileMgr, moo_edit_file_mgr, G_TYPE_OBJECT)


static void moo_edit_file_mgr_class_init    (MooEditFileMgrClass   *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    gobject_class->finalize = moo_edit_file_mgr_finalize;
}


static void moo_edit_file_mgr_init          (MooEditFileMgr        *mgr)
{
    mgr->priv = g_new0 (MooEditFileMgrPrivate, 1);
    list_store_init (mgr);
}


static void moo_edit_file_mgr_finalize      (GObject            *object)
{
    MooEditFileMgr *mgr = MOO_EDIT_FILE_MGR (object);
    list_store_destroy (mgr);
    G_OBJECT_CLASS (moo_edit_file_mgr_parent_class)->finalize (object);
}


MooEditFileMgr  *moo_edit_file_mgr_new              (void)
{
    return MOO_EDIT_FILE_MGR (g_object_new (MOO_TYPE_EDIT_FILE_MGR, NULL));
}


static gboolean row_is_separator (GtkTreeModel  *model,
                                  GtkTreeIter   *iter,
                                  G_GNUC_UNUSED gpointer data)
{
    Filter *filter;
    gtk_tree_model_get (model, iter, COLUMN_FILTER, &filter, -1);
    return filter == NULL;
}


static void     filter_entry_activated  (GtkEntry       *entry,
                                         GtkFileChooser *dialog);

static GtkWidget *create_open_dialog (MooEditFileMgr    *mgr,
                                      const gchar       *title,
                                      GtkWindow         *parent)
{
    GtkWidget *dialog;
    GtkWidget *alignment;
    GtkWidget *hbox;
    GtkWidget *label;
    GtkWidget *combo;
    GtkWidget *entry;
    Filter *filter;

    dialog = gtk_file_chooser_dialog_new (title, parent,
                                          GTK_FILE_CHOOSER_ACTION_OPEN,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_OPEN, GTK_RESPONSE_OK,
                                          NULL);

    alignment = gtk_alignment_new (1, 0.5, 0, 1);
    gtk_widget_show (alignment);
    gtk_file_chooser_set_extra_widget (GTK_FILE_CHOOSER (dialog), alignment);

    hbox = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (hbox);
    gtk_container_add (GTK_CONTAINER (alignment), hbox);

    label = gtk_label_new ("Filter:");
    gtk_widget_show (label);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

    combo = gtk_combo_box_entry_new_with_model (GTK_TREE_MODEL (mgr->priv->filters),
                                                COLUMN_DESCRIPTION);
    gtk_combo_box_set_row_separator_func (GTK_COMBO_BOX (combo),
                                          row_is_separator,
                                          NULL, NULL);
    gtk_widget_show (combo);
    gtk_box_pack_start (GTK_BOX (hbox), combo, TRUE, TRUE, 0);

    entry = GTK_WIDGET (GTK_BIN(combo)->child);

    g_signal_connect (entry, "activate",
                      G_CALLBACK (filter_entry_activated),
                      dialog);

    g_object_set_data (G_OBJECT (dialog), "filter-combo", combo);
    g_object_set_data (G_OBJECT (dialog), "file-mgr", mgr);

    filter = mgr_get_last_filter (mgr);

    if (filter)
    {
        gtk_entry_set_text (GTK_ENTRY (entry), filter->description);
        gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (dialog),
                                     filter->filter);
    }

    return dialog;
}


MooEditFileInfo *moo_edit_file_mgr_open_dialog      (MooEditFileMgr *mgr,
                                                     GtkWidget      *parent)
{
    const char *filename = NULL;
    const char *title = "Open File";
    const char *start = NULL;
    MooEditFileInfo *file = NULL;
    GtkWindow *parent_window = NULL;
    GtkWidget *dialog = NULL;

    g_return_val_if_fail (MOO_IS_EDIT_FILE_MGR (mgr), NULL);

    start = moo_prefs_get_string (moo_edit_setting (MOO_EDIT_PREFS_DIALOGS_OPEN));

    if (parent)
        parent_window = GTK_WINDOW (gtk_widget_get_toplevel (parent));

    dialog = create_open_dialog (mgr, title, parent_window);

#if GTK_CHECK_VERSION(2,6,0)
    gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                             GTK_RESPONSE_OK,
                                             GTK_RESPONSE_CANCEL,
                                             -1);
#endif /* GTK_CHECK_VERSION(2,6,0) */

    if (start)
        gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(dialog),
                                             start);

    gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

    if (gtk_dialog_run (GTK_DIALOG(dialog)) == GTK_RESPONSE_OK)
    {
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER(dialog));
        gtk_widget_destroy (dialog);
    }
    else
    {
        gtk_widget_destroy (dialog);
        filename = NULL;
    }

    if (filename)
    {
        char *new_start = g_path_get_dirname (filename);
        moo_prefs_set_string (moo_edit_setting (MOO_EDIT_PREFS_DIALOGS_OPEN),
                              new_start);
        g_free (new_start);
        file = moo_edit_file_info_new (filename, NULL);
    }

    return file;
}


static Filter   *mgr_get_null_filter        (MooEditFileMgr     *mgr)
{
    if (!mgr->priv->null_filter)
    {
        Filter *filter = filter_new ();
        filter->description = g_strdup ("All Files");
        filter->filter = gtk_file_filter_new ();
        gtk_file_filter_add_pattern (filter->filter, "*");
        g_object_ref (filter->filter);
        gtk_object_sink (GTK_OBJECT (filter->filter));
        list_store_append_filter (mgr, filter);
        mgr->priv->null_filter = filter;
    }
    return mgr->priv->null_filter;
}

static Filter   *mgr_get_last_filter        (MooEditFileMgr     *mgr)
{
    if (!mgr->priv->last_filter)
        mgr->priv->last_filter = mgr_get_null_filter (mgr);
    return mgr->priv->last_filter;
}


static void      list_store_init            (MooEditFileMgr     *mgr)
{
    mgr->priv->filters = gtk_list_store_new (NUM_COLUMNS,
                                             G_TYPE_STRING,
                                             G_TYPE_POINTER);
}


static gboolean filter_free_func (GtkTreeModel  *model,
                                  G_GNUC_UNUSED GtkTreePath *path,
                                  GtkTreeIter   *iter,
                                  G_GNUC_UNUSED gpointer data)
{
    Filter *filter;
    gtk_tree_model_get (model, iter, COLUMN_FILTER, &filter, -1);
    if (filter)
        filter_unref (filter);
    return FALSE;
}

static void      list_store_destroy         (MooEditFileMgr     *mgr)
{
    gtk_tree_model_foreach (GTK_TREE_MODEL (mgr->priv->filters),
                            filter_free_func, NULL);
    g_object_unref (mgr->priv->filters);
    mgr->priv->filters = NULL;
    mgr->priv->last_filter = NULL;
}


static void      list_store_append_filter   (MooEditFileMgr     *mgr,
                                             Filter             *filter)
{
    GtkTreeIter iter;
    gtk_list_store_append (mgr->priv->filters, &iter);
    if (filter)
        gtk_list_store_set (mgr->priv->filters, &iter,
                            COLUMN_DESCRIPTION, filter->description,
                            COLUMN_FILTER, filter, -1);
}


static Filter   *filter_new     (void)
{
    Filter *filter = g_new0 (Filter, 1);
    filter->ref_count = 1;
    return filter;
}


static Filter   *filter_ref     (Filter *filter)
{
    g_return_val_if_fail (filter != NULL, NULL);
    filter->ref_count++;
    return filter;
}


static void      filter_unref   (Filter *filter)
{
    if (filter && --filter->ref_count == 0)
    {
        if (filter->filter)
            g_object_unref (filter->filter);
        g_free (filter->description);
        g_free (filter);
    }
}


static void     filter_entry_activated  (GtkEntry       *entry,
                                         GtkFileChooser *dialog)
{
    const char *text;
    Filter *filter;
    MooEditFileMgr *mgr;

    mgr = g_object_get_data (G_OBJECT (dialog), "file-mgr");
    g_return_if_fail (mgr != NULL);

    text = gtk_entry_get_text (entry);

    if (!text || !text[0])
    {
        filter = mgr_get_null_filter (mgr);
    }
    else
    {
        filter = list_store_find_filter (mgr, text);

        if (!filter)
        {
            filter = filter_new ();
            filter->description = g_strdup (text);
            filter->user = TRUE;
            filter->filter = gtk_file_filter_new ();
            gtk_object_sink (gtk_object_ref (GTK_OBJECT (filter->filter)));
            gtk_file_filter_add_pattern (filter->filter, text);
            list_store_add_user (mgr, filter);
        }
    }

    gtk_entry_set_text (entry, filter->description);
    gtk_file_chooser_set_filter (dialog, filter->filter);
    mgr->priv->last_filter = filter;
}
