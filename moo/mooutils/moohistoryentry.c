/*
 *   moohistoryentry.c
 *
 *   Copyright (C) 2004-2006 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#include MOO_MARSHALS_H
#ifndef __MOO__
#include "moohistoryentry.h"
#else
#include "mooutils/moohistoryentry.h"
#endif
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>


struct _MooHistoryEntryPrivate {
    gboolean called_popup;
    guint completion_popup_timeout_id;
    guint completion_popup_timeout;

    MooHistoryList *list;
    GtkTreeModel *model;
    GtkTreeModel *filter;
    char *completion_text;
    MooHistoryEntryFilterFunc filter_func;
    gpointer filter_data;
    gboolean disable_filter;

    gboolean sort_history;
    gboolean sort_completion;
};


static void     moo_history_entry_class_init    (MooHistoryEntryClass  *klass);
static void     moo_history_entry_init          (MooHistoryEntry    *entry);
static void     moo_history_entry_finalize      (GObject            *object);
static void     moo_history_entry_set_property  (GObject            *object,
                                                 guint               prop_id,
                                                 const GValue       *value,
                                                 GParamSpec         *pspec);
static void     moo_history_entry_get_property  (GObject            *object,
                                                 guint               prop_id,
                                                 GValue             *value,
                                                 GParamSpec         *pspec);

static void     moo_history_entry_popup         (MooCombo           *combo);
static void     moo_history_entry_changed       (MooCombo           *combo);

static gboolean completion_visible_func         (GtkTreeModel       *model,
                                                 GtkTreeIter        *iter,
                                                 MooHistoryEntry    *entry);
static gboolean default_filter_func             (const char         *text,
                                                 GtkTreeModel       *model,
                                                 GtkTreeIter        *iter,
                                                 gpointer            data);
static int      default_sort_func               (GtkTreeModel       *model,
                                                 GtkTreeIter        *a,
                                                 GtkTreeIter        *b);
static void     cell_data_func                  (GtkCellLayout      *cell_layout,
                                                 GtkCellRenderer    *cell,
                                                 GtkTreeModel       *tree_model,
                                                 GtkTreeIter        *iter);
static gboolean row_separator_func              (GtkTreeModel       *model,
                                                 GtkTreeIter        *iter,
                                                 gpointer            data);
static char    *get_text_func                   (GtkTreeModel       *model,
                                                 GtkTreeIter        *iter,
                                                 gpointer            data);


/* MOO_TYPE_HISTORY_ENTRY */
G_DEFINE_TYPE (MooHistoryEntry, moo_history_entry, MOO_TYPE_COMBO)


enum {
    PROP_0,
    PROP_LIST,
    PROP_SORT_HISTORY,
    PROP_SORT_COMPLETION
};

enum {
    NUM_SIGNALS
};

// static guint signals[NUM_SIGNALS];

static void
moo_history_entry_class_init (MooHistoryEntryClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    MooComboClass *combo_class = MOO_COMBO_CLASS (klass);

    moo_history_entry_parent_class = g_type_class_peek_parent (klass);

    gobject_class->finalize = moo_history_entry_finalize;
    gobject_class->set_property = moo_history_entry_set_property;
    gobject_class->get_property = moo_history_entry_get_property;

    combo_class->popup = moo_history_entry_popup;
    combo_class->changed = moo_history_entry_changed;

    g_object_class_install_property (gobject_class,
                                     PROP_LIST,
                                     g_param_spec_object ("list",
                                             "list",
                                             "list",
                                             MOO_TYPE_HISTORY_LIST,
                                             G_PARAM_CONSTRUCT | G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_SORT_HISTORY,
                                     g_param_spec_boolean ("sort-history",
                                             "sort-history",
                                             "sort-history",
                                             TRUE,
                                             G_PARAM_CONSTRUCT | G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_SORT_COMPLETION,
                                     g_param_spec_boolean ("sort-completion",
                                             "sort-completion",
                                             "sort-completion",
                                             FALSE,
                                             G_PARAM_CONSTRUCT | G_PARAM_READWRITE));
}


static void
moo_history_entry_init (MooHistoryEntry *entry)
{
    GtkCellRenderer *cell;

    entry->priv = g_new0 (MooHistoryEntryPrivate, 1);

    entry->priv->filter_func = default_filter_func;
    entry->priv->filter_data = entry;
    entry->priv->completion_text = g_strdup ("");

    gtk_cell_layout_clear (GTK_CELL_LAYOUT (entry));
    cell = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (entry), cell, TRUE);
    gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (entry), cell,
                                        (GtkCellLayoutDataFunc) cell_data_func,
                                        entry, NULL);

    entry->priv->list = moo_history_list_new (NULL);
    entry->priv->model = gtk_tree_model_sort_new_with_model (moo_history_list_get_model (entry->priv->list));
    entry->priv->filter = gtk_tree_model_filter_new (entry->priv->model, NULL);
    gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER (entry->priv->filter),
                                            (GtkTreeModelFilterVisibleFunc) completion_visible_func,
                                            entry, NULL);
    moo_combo_set_model (MOO_COMBO (entry), entry->priv->filter);
    moo_combo_set_row_separator_func (MOO_COMBO (entry), row_separator_func, NULL);
    moo_combo_set_get_text_func (MOO_COMBO (entry), get_text_func, NULL);
}


static void
moo_history_entry_set_property (GObject        *object,
                        guint           prop_id,
                        const GValue   *value,
                        GParamSpec     *pspec)
{
    MooHistoryEntry *entry = MOO_HISTORY_ENTRY (object);

    switch (prop_id)
    {
        case PROP_LIST:
            moo_history_entry_set_list (entry, g_value_get_object (value));
            break;

        case PROP_SORT_HISTORY:
            entry->priv->sort_history = g_value_get_boolean (value);
            g_object_notify (object, "sort-history");
            break;

        case PROP_SORT_COMPLETION:
            entry->priv->sort_completion = g_value_get_boolean (value);
            g_object_notify (object, "sort-completion");
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static void
moo_history_entry_get_property (GObject        *object,
                        guint           prop_id,
                        GValue         *value,
                        GParamSpec     *pspec)
{
    MooHistoryEntry *entry = MOO_HISTORY_ENTRY (object);

    switch (prop_id)
    {
        case PROP_LIST:
            g_value_set_object (value, entry->priv->list);
            break;

        case PROP_SORT_HISTORY:
            g_value_set_boolean (value, entry->priv->sort_history);
            break;

        case PROP_SORT_COMPLETION:
            g_value_set_boolean (value, entry->priv->sort_completion);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static void
moo_history_entry_finalize (GObject *object)
{
    MooHistoryEntry *entry = MOO_HISTORY_ENTRY (object);

    g_free (entry->priv->completion_text);

    if (entry->priv->filter)
        g_object_unref (entry->priv->filter);

    g_object_unref (entry->priv->list);

    g_free (entry->priv);
    G_OBJECT_CLASS (moo_history_entry_parent_class)->finalize (object);
}


static gboolean
model_is_empty (GtkTreeModel *model)
{
    GtkTreeIter iter;
    return !gtk_tree_model_get_iter_first (model, &iter);
}


static void
get_entry_text (MooHistoryEntry *entry)
{
    g_free (entry->priv->completion_text);
    entry->priv->completion_text = g_strdup (moo_combo_entry_get_text (MOO_COMBO (entry)));
}


static void
moo_history_entry_popup (MooCombo *combo)
{
    MooHistoryEntry *entry = MOO_HISTORY_ENTRY (combo);
    gboolean do_sort = FALSE;

    if (!entry->priv->called_popup)
    {
        entry->priv->disable_filter = TRUE;
        if (entry->priv->sort_history)
            do_sort = TRUE;
    }
    else
    {
        entry->priv->completion_popup_timeout_id = 0;
        entry->priv->disable_filter = FALSE;
        entry->priv->called_popup = FALSE;

        if (entry->priv->sort_completion)
            do_sort = TRUE;

        if (moo_combo_popup_shown (MOO_COMBO (entry)))
            return;

        if (!entry->priv->filter)
            return;

        get_entry_text (entry);
    }

    if (do_sort)
        gtk_tree_sortable_set_default_sort_func (GTK_TREE_SORTABLE (entry->priv->model),
                                                 (GtkTreeIterCompareFunc) default_sort_func,
                                                 NULL, NULL);
    else
        gtk_tree_model_sort_reset_default_sort_func (GTK_TREE_MODEL_SORT (entry->priv->model));

    gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (entry->priv->filter));
    entry->priv->disable_filter = FALSE;

    MOO_COMBO_CLASS(moo_history_entry_parent_class)->popup (combo);
}


static int
default_sort_func (GtkTreeModel       *model,
                   GtkTreeIter        *a,
                   GtkTreeIter        *b)
{
    MooHistoryListItem *e1 = NULL, *e2 = NULL;
    int result;

    /* XXX 0 is not good */
    gtk_tree_model_get (model, a, 0, &e1, -1);
    gtk_tree_model_get (model, b, 0, &e2, -1);

    if (!e1 && !e2)
        result = 0;
    else if (!e1)
        result = e2->builtin ? 1 : -1;
    else if (!e2)
        result = e1->builtin ? -1 : 1;
    else if (e1->builtin != e2->builtin)
        result = e1->builtin ? -1 : 1;
    else
        result = strcmp (e1->data, e2->data);

    moo_history_list_item_free (e1);
    moo_history_list_item_free (e2);
    return result;
}


static void
refilter (MooHistoryEntry *entry)
{
    get_entry_text (entry);
    gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (entry->priv->filter));

    if (model_is_empty (entry->priv->filter))
        moo_combo_popdown (MOO_COMBO (entry));
    else
        moo_combo_update_popup (MOO_COMBO (entry));
}


static gboolean
popup (MooHistoryEntry *entry)
{
    entry->priv->called_popup = TRUE;
    moo_combo_popup (MOO_COMBO (entry));
    return FALSE;
}


static void
moo_history_entry_changed (MooCombo *combo)
{
    MooHistoryEntry *entry = MOO_HISTORY_ENTRY (combo);

    if (!GTK_WIDGET_MAPPED (combo))
        return;

    get_entry_text (entry);

    if (!entry->priv->completion_text[0] ||
         moo_combo_get_active_iter (combo, NULL))
    {
        if (entry->priv->completion_popup_timeout_id)
            g_source_remove (entry->priv->completion_popup_timeout_id);

        moo_combo_popdown (MOO_COMBO (entry));
    }
    else if (moo_combo_popup_shown (MOO_COMBO (entry)))
    {
        refilter (entry);
    }
    else if (entry->priv->completion_popup_timeout)
    {
        if (entry->priv->completion_popup_timeout_id)
            g_source_remove (entry->priv->completion_popup_timeout_id);
        entry->priv->completion_popup_timeout_id =
                g_timeout_add (entry->priv->completion_popup_timeout,
                               (GSourceFunc) popup,
                               entry);
    }
    else
    {
        popup (entry);
    }
}


// static void
// maybe_refilter (MooHistoryEntry *entry)
// {
//     if (moo_combo_popup_shown (MOO_COMBO (entry)))
//         refilter (entry);
// }


static gboolean
completion_visible_func (GtkTreeModel    *model,
                         GtkTreeIter     *iter,
                         MooHistoryEntry *entry)
{
    if (entry->priv->disable_filter)
        return TRUE;
    else
        return entry->priv->filter_func (entry->priv->completion_text,
                                         model, iter, entry->priv->filter_data);
}


static gboolean
default_filter_func (const char         *entry_text,
                     GtkTreeModel       *model,
                     GtkTreeIter        *iter,
                     G_GNUC_UNUSED gpointer data)
{
    gboolean visible;
    MooHistoryListItem *e = NULL;

    gtk_tree_model_get (model, iter, 0, &e, -1);

    if (entry_text[0])
    {
        if (!e)
            visible = FALSE;
        else
            visible = !strncmp (entry_text, e->data, strlen (entry_text));
    }
    else
    {
        visible = TRUE;
    }

    moo_history_list_item_free (e);
    return visible ? TRUE : FALSE;
}


GtkWidget*
moo_history_entry_new (void)
{
    return g_object_new (MOO_TYPE_HISTORY_ENTRY, NULL);
}


void
moo_history_entry_add_text (MooHistoryEntry *entry,
                            const char      *text)
{
    g_return_if_fail (MOO_IS_HISTORY_ENTRY (entry));
    g_return_if_fail (text != NULL);
    moo_history_list_add (entry->priv->list, text);
}


void
moo_history_entry_set_list (MooHistoryEntry    *entry,
                            MooHistoryList      *list)
{
    g_return_if_fail (MOO_IS_HISTORY_ENTRY (entry));
    g_return_if_fail (!list || MOO_IS_HISTORY_LIST (list));

    if (entry->priv->list == list)
        return;

    if (!list)
    {
        list = moo_history_list_new (NULL);
        moo_history_entry_set_list (entry, list);
        g_object_unref (list);
        return;
    }

    g_object_unref (entry->priv->list);
    moo_combo_set_model (MOO_COMBO (entry), NULL);
    g_object_unref (entry->priv->filter);

    entry->priv->list = list;
    g_object_ref (entry->priv->list);
    entry->priv->model = gtk_tree_model_sort_new_with_model (moo_history_list_get_model (entry->priv->list));
    entry->priv->filter = gtk_tree_model_filter_new (entry->priv->model, NULL);
    gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER (entry->priv->filter),
                                            (GtkTreeModelFilterVisibleFunc) completion_visible_func,
                                            entry, NULL);
    moo_combo_set_model (MOO_COMBO (entry), entry->priv->filter);

    moo_history_list_load (list);

    g_object_notify (G_OBJECT (entry), "list");
}


MooHistoryList*
moo_history_entry_get_list (MooHistoryEntry    *entry)
{
    g_return_val_if_fail (MOO_IS_HISTORY_ENTRY (entry), NULL);
    return entry->priv->list;
}


static void
cell_data_func (G_GNUC_UNUSED GtkCellLayout *cell_layout,
                GtkCellRenderer    *cell,
                GtkTreeModel       *model,
                GtkTreeIter        *iter)
{
    MooHistoryListItem *e = NULL;

    gtk_tree_model_get (model, iter, 0, &e, -1);

    if (e)
    {
        g_object_set (cell, "text", e->display, NULL);
        moo_history_list_item_free (e);
    }
}


static gboolean
row_separator_func (GtkTreeModel       *model,
                    GtkTreeIter        *iter,
                    G_GNUC_UNUSED gpointer data)
{
    MooHistoryListItem *e = NULL;
    gtk_tree_model_get (model, iter, 0, &e, -1);
    moo_history_list_item_free (e);
    return !e;
}


static char*
get_text_func (GtkTreeModel       *model,
               GtkTreeIter        *iter,
               G_GNUC_UNUSED gpointer data)
{
    MooHistoryListItem *e = NULL;
    char *text;

    gtk_tree_model_get (model, iter, 0, &e, -1);

    g_return_val_if_fail (e != NULL, NULL);
    g_return_val_if_fail (g_utf8_validate (e->data, -1, NULL), NULL);

    text = g_strdup (e->data);
    moo_history_list_item_free (e);
    return text;
}
