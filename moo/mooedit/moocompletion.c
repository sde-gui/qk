/*
 *   moocompletion.c
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

#include "mooedit/moocompletion.h"
#include "mooedit/mootextpopup.h"
#include "mooutils/moomarshals.h"
#include "mooutils/eggregex.h"
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>


struct _MooCompletionPrivate {
    GtkListStore *store;
    GSList *groups;
    MooCompletionGroup *active_group;

    GtkTextView *doc;
    GtkTextBuffer *buffer;
    GtkTextMark *start;
    GtkTextMark *end;

    MooTextPopup *popup;
    guint working : 1;
    guint in_update : 1;
};

struct _MooCompletionGroup {
    guint ref_count;

    EggRegex *regex;
    guint *parens;
    guint n_parens;

    GCompletion *cmpl;
    GList *data;

    MooCompletionStringFunc string_func;
    MooCompletionFreeFunc free_func;
    MooCompletionCmpFunc cmp_func;
};


static void     moo_completion_dispose          (GObject            *object);

static void     moo_completion_update           (MooCompletion      *cmpl);
static void     moo_completion_populate         (MooCompletion      *cmpl,
                                                 MooCompletionGroup *group,
                                                 const char         *text,
                                                 char              **prefix);
static void     moo_completion_complete         (MooCompletion      *cmpl,
                                                 gpointer            data);

static void     moo_completion_connect_popup    (MooCompletion      *cmpl);
static void     moo_completion_disconnect_popup (MooCompletion      *cmpl);

static gboolean moo_completion_empty            (MooCompletion      *cmpl);
static gboolean moo_completion_unique           (MooCompletion      *cmpl,
                                                 GtkTreeIter        *iter);
static gpointer moo_completion_get_data         (MooCompletion      *cmpl,
                                                 GtkTreeIter        *iter);

static gboolean moo_completion_group_find       (MooCompletionGroup *group,
                                                 const char         *line,
                                                 int                *start_pos,
                                                 int                *end_pos);
static GList   *moo_completion_group_complete   (MooCompletionGroup *group,
                                                 const char         *text,
                                                 char              **prefix);


G_DEFINE_TYPE (MooCompletion, moo_completion, G_TYPE_OBJECT)


static void
moo_completion_dispose (GObject *object)
{
    MooCompletion *cmpl = MOO_COMPLETION (object);

    if (cmpl->priv)
    {
        moo_completion_set_doc (cmpl, NULL);

        g_object_unref (cmpl->priv->store);
        g_slist_foreach (cmpl->priv->groups, (GFunc) moo_completion_group_unref, NULL);
        g_slist_free (cmpl->priv->groups);

        if (cmpl->priv->popup)
        {
            moo_completion_disconnect_popup (cmpl);
            g_object_unref (cmpl->priv->popup);
        }

        g_free (cmpl->priv);
        cmpl->priv = NULL;
    }

    G_OBJECT_CLASS(moo_completion_parent_class)->dispose (object);
}


static void
moo_completion_class_init (MooCompletionClass *klass)
{
    G_OBJECT_CLASS(klass)->dispose = moo_completion_dispose;
}


static void
moo_completion_init (MooCompletion *cmpl)
{
    cmpl->priv = g_new0 (MooCompletionPrivate, 1);
    cmpl->priv->store = gtk_list_store_new (1, G_TYPE_POINTER);
    cmpl->priv->popup = moo_text_popup_new (NULL);
    moo_completion_connect_popup (cmpl);
}


void
moo_completion_try_complete (MooCompletion *cmpl,
                             gboolean       insert_unique)
{
    GSList *l;
    MooCompletionGroup *group;
    GtkTextIter start, end;
    GtkTreeIter iter;
    gboolean found = FALSE;
    int start_pos, end_pos;
    char *prefix = NULL, *text = NULL, *line;

    g_return_if_fail (MOO_IS_COMPLETION (cmpl));
    g_return_if_fail (cmpl->priv->buffer != NULL);
    g_return_if_fail (!cmpl->priv->working);

    cmpl->priv->working = TRUE;

    gtk_text_buffer_get_iter_at_mark (cmpl->priv->buffer, &end,
                                      gtk_text_buffer_get_insert (cmpl->priv->buffer));
    start = end;

    if (!gtk_text_iter_starts_line (&start))
        gtk_text_iter_set_line_offset (&start, 0);

    line = gtk_text_buffer_get_slice (cmpl->priv->buffer, &start, &end, TRUE);

    for (l = cmpl->priv->groups; !found && l != NULL; l = l->next)
    {
        group = l->data;
        found = moo_completion_group_find (group, line, &start_pos, &end_pos);
    }

    if (found)
    {
        cmpl->priv->active_group = group;

        gtk_text_iter_set_line_index (&start, start_pos);
        gtk_text_iter_set_line_index (&end, end_pos);

        text = gtk_text_buffer_get_slice (cmpl->priv->buffer, &start, &end, TRUE);
        moo_completion_set_region (cmpl, &start, &end);
        moo_completion_populate (cmpl, group, text, &prefix);
    }

    if (!found || moo_completion_empty (cmpl))
    {
        cmpl->priv->active_group = NULL;
        cmpl->priv->working = FALSE;
        goto out;
    }

    if (insert_unique && moo_completion_unique (cmpl, &iter))
    {
        gpointer data = moo_completion_get_data (cmpl, &iter);
        moo_completion_complete (cmpl, data);
        cmpl->priv->working = FALSE;
        goto out;
    }

    if (prefix && strcmp (text, prefix))
    {
        gtk_text_buffer_begin_user_action (cmpl->priv->buffer);
        gtk_text_buffer_delete (cmpl->priv->buffer, &start, &end);
        gtk_text_buffer_insert (cmpl->priv->buffer, &start, prefix, -1);
        gtk_text_buffer_end_user_action (cmpl->priv->buffer);
    }

    moo_completion_get_region (cmpl, &start, NULL);
    moo_text_popup_show (cmpl->priv->popup, &start);

out:
    g_free (text);
    g_free (prefix);
    g_free (line);
}


static void
moo_completion_update (MooCompletion *cmpl)
{
    GtkTextIter start, end;
    GList *list;
    char *text;

    g_return_if_fail (cmpl->priv->working);
    g_return_if_fail (cmpl->priv->active_group != NULL);

    moo_completion_get_region (cmpl, &start, &end);
    text = gtk_text_buffer_get_slice (cmpl->priv->buffer, &start, &end, TRUE);

    list = moo_completion_group_complete (cmpl->priv->active_group, text, NULL);
    gtk_list_store_clear (cmpl->priv->store);

    if (list)
    {
        while (list)
        {
            GtkTreeIter iter;
            gtk_list_store_append (cmpl->priv->store, &iter);
            gtk_list_store_set (cmpl->priv->store, &iter, 0, list->data, -1);
            list = list->next;
        }

        moo_text_popup_update (cmpl->priv->popup);
    }
    else
    {
        moo_completion_hide (cmpl);
    }

    g_free (text);
}


static void
moo_completion_complete (MooCompletion *cmpl,
                         gpointer       data)
{
    char *text, *old_text;
    GtkTextIter start, end;

    g_return_if_fail (cmpl->priv->active_group != NULL);

    text = cmpl->priv->active_group->string_func ?
            cmpl->priv->active_group->string_func (data) : data;
    g_return_if_fail (text != NULL);

    moo_completion_get_region (cmpl, &start, &end);
    old_text = gtk_text_buffer_get_slice (cmpl->priv->buffer,
                                          &start, &end, TRUE);

    if (strcmp (text, old_text))
    {
        gtk_text_buffer_begin_user_action (cmpl->priv->buffer);
        gtk_text_buffer_delete (cmpl->priv->buffer, &start, &end);
        gtk_text_buffer_insert (cmpl->priv->buffer, &start, text, -1);
        gtk_text_buffer_end_user_action (cmpl->priv->buffer);
    }

    moo_completion_hide (cmpl);

    g_free (old_text);
}


static void
moo_completion_populate (MooCompletion      *cmpl,
                         MooCompletionGroup *group,
                         const char         *text,
                         char              **prefix)
{
    GList *list;

    gtk_list_store_clear (cmpl->priv->store);
    list = moo_completion_group_complete (group, text, prefix);

    while (list)
    {
        GtkTreeIter iter;
        gtk_list_store_append (cmpl->priv->store, &iter);
        gtk_list_store_set (cmpl->priv->store, &iter, 0, list->data, -1);
        list = list->next;
    }
}


static gboolean
moo_completion_empty (MooCompletion *cmpl)
{
    return !gtk_tree_model_iter_n_children (GTK_TREE_MODEL (cmpl->priv->store), NULL);
}


static gboolean
moo_completion_unique (MooCompletion  *cmpl,
                       GtkTreeIter    *iter)
{
    return gtk_tree_model_iter_n_children (GTK_TREE_MODEL (cmpl->priv->store), NULL) == 1 &&
            gtk_tree_model_get_iter_first (GTK_TREE_MODEL (cmpl->priv->store), iter);
}


static gpointer
moo_completion_get_data (MooCompletion *cmpl,
                         GtkTreeIter   *iter)
{
    gpointer data = NULL;
    gtk_tree_model_get (GTK_TREE_MODEL (cmpl->priv->store), iter, 0, &data, -1);
    return data;
}


static void
on_popup_activate (MooCompletion    *cmpl,
                   GtkTreeModel     *model,
                   GtkTreeIter      *iter)
{
    gpointer data = NULL;
    gtk_tree_model_get (model, iter, 0, &data, -1);
    moo_completion_complete (cmpl, data);
}


static void
moo_completion_connect_popup (MooCompletion *cmpl)
{
    moo_text_popup_set_model (cmpl->priv->popup,
                              GTK_TREE_MODEL (cmpl->priv->store));
    g_signal_connect_swapped (cmpl->priv->popup, "activate",
                              G_CALLBACK (on_popup_activate), cmpl);
    g_signal_connect_swapped (cmpl->priv->popup, "text-changed",
                              G_CALLBACK (moo_completion_update), cmpl);
    g_signal_connect_swapped (cmpl->priv->popup, "hide",
                              G_CALLBACK (moo_completion_hide), cmpl);
}


static void
moo_completion_disconnect_popup (MooCompletion *cmpl)
{
    moo_text_popup_set_model (cmpl->priv->popup, NULL);
    g_signal_handlers_disconnect_by_func (cmpl->priv->popup,
                                          (gpointer) on_popup_activate,
                                          cmpl);
    g_signal_handlers_disconnect_by_func (cmpl->priv->popup,
                                          (gpointer) moo_completion_update,
                                          cmpl);
    g_signal_handlers_disconnect_by_func (cmpl->priv->popup,
                                          (gpointer) moo_completion_hide,
                                          cmpl);
}


void
moo_completion_hide (MooCompletion *cmpl)
{
    g_return_if_fail (MOO_IS_COMPLETION (cmpl));

    if (cmpl->priv->working)
    {
        cmpl->priv->working = FALSE;
        moo_text_popup_hide (cmpl->priv->popup);
        gtk_list_store_clear (cmpl->priv->store);
    }
}


void
moo_completion_set_doc (MooCompletion  *cmpl,
                             GtkTextView        *doc)
{
    g_return_if_fail (MOO_IS_COMPLETION (cmpl));
    g_return_if_fail (!doc || GTK_IS_TEXT_VIEW (doc));

    if (cmpl->priv->doc == doc)
        return;

    moo_completion_hide (cmpl);

    if (cmpl->priv->doc)
    {
        if (cmpl->priv->start)
            gtk_text_buffer_delete_mark (cmpl->priv->buffer, cmpl->priv->start);
        if (cmpl->priv->end)
            gtk_text_buffer_delete_mark (cmpl->priv->buffer, cmpl->priv->end);
        g_object_unref (cmpl->priv->doc);
        cmpl->priv->start = cmpl->priv->end = NULL;
        cmpl->priv->buffer = NULL;
        cmpl->priv->doc = NULL;
    }

    if (doc)
    {
        cmpl->priv->doc = g_object_ref (doc);
        cmpl->priv->buffer = gtk_text_view_get_buffer (doc);
    }

    moo_text_popup_set_doc (cmpl->priv->popup, doc);
}


GtkTextView *
moo_completion_get_doc (MooCompletion *cmpl)
{
    g_return_val_if_fail (MOO_IS_COMPLETION (cmpl), NULL);
    return cmpl->priv->doc;
}


MooTextPopup *
moo_completion_get_popup (MooCompletion *cmpl)
{
    g_return_val_if_fail (MOO_IS_COMPLETION (cmpl), NULL);
    return cmpl->priv->popup;
}


gboolean
moo_completion_get_region (MooCompletion  *cmpl,
                                GtkTextIter        *start,
                                GtkTextIter        *end)
{
    g_return_val_if_fail (MOO_IS_COMPLETION (cmpl), FALSE);

    if (cmpl->priv->start)
    {
        if (start)
            gtk_text_buffer_get_iter_at_mark (cmpl->priv->buffer, start,
                                              cmpl->priv->start);
        if (end)
            gtk_text_buffer_get_iter_at_mark (cmpl->priv->buffer, end,
                                              cmpl->priv->end);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


void
moo_completion_set_region (MooCompletion      *cmpl,
                           const GtkTextIter  *start,
                           const GtkTextIter  *end)
{
    g_return_if_fail (MOO_IS_COMPLETION (cmpl));
    g_return_if_fail (start && end);
    g_return_if_fail (cmpl->priv->buffer != NULL);

    if (!cmpl->priv->start)
    {
        cmpl->priv->start = gtk_text_buffer_create_mark (cmpl->priv->buffer, NULL, start, TRUE);
        cmpl->priv->end = gtk_text_buffer_create_mark (cmpl->priv->buffer, NULL, end, FALSE);
    }
    else
    {
        gtk_text_buffer_move_mark (cmpl->priv->buffer, cmpl->priv->start, start);
        gtk_text_buffer_move_mark (cmpl->priv->buffer, cmpl->priv->end, end);
    }

    moo_text_popup_set_position (cmpl->priv->popup, start);
    moo_completion_update (cmpl);
}


MooCompletion *
moo_completion_new (void)
{
    return g_object_new (MOO_TYPE_COMPLETION, NULL);
}


static void
text_cell_data_func (G_GNUC_UNUSED GtkTreeViewColumn *tree_column,
                     GtkCellRenderer    *cell,
                     GtkTreeModel       *tree_model,
                     GtkTreeIter        *iter,
                     MooCompletionGroup *group)
{
    gpointer data = NULL;
    char *text;

    g_return_if_fail (group != NULL);

    gtk_tree_model_get (tree_model, iter, 0, &data, -1);
    text = group->string_func ? group->string_func (data) : data;
    g_return_if_fail (text != NULL);

    g_object_set (cell, "text", text, NULL);
}


MooCompletion *
moo_completion_new_text (GList    *words,
                         gboolean  sorted)
{
    MooCompletion *cmpl;
    MooCompletionGroup *group;
    GtkCellRenderer *cell;

    cmpl = moo_completion_new ();
    group = moo_completion_group_new_text (sorted);
    moo_completion_group_add_data (group, words);
    moo_completion_group_set_pattern (group, "\\w*", NULL, 0);
    moo_completion_add_group (cmpl, group);
    moo_completion_group_unref (group);

    cell = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (cmpl->priv->popup->column, cell, TRUE);
    gtk_tree_view_column_set_cell_data_func (cmpl->priv->popup->column, cell,
                                             (GtkTreeCellDataFunc) text_cell_data_func,
                                             moo_completion_group_ref (group),
                                             (GtkDestroyNotify) moo_completion_group_unref);

    return cmpl;
}


void
moo_completion_add_group (MooCompletion      *cmpl,
                          MooCompletionGroup *group)
{
    g_return_if_fail (MOO_IS_COMPLETION (cmpl));
    g_return_if_fail (group != NULL);
    g_return_if_fail (g_slist_find (cmpl->priv->groups, group) == NULL);
    cmpl->priv->groups = g_slist_append (cmpl->priv->groups,
                                         moo_completion_group_ref (group));
}


/****************************************************************************/
/* MooCompletionGroup
 */

MooCompletionGroup *
moo_completion_group_new (MooCompletionStringFunc string_func,
                          MooCompletionFreeFunc   free_func,
                          MooCompletionCmpFunc    cmp_func)
{
    MooCompletionGroup *group = g_new0 (MooCompletionGroup, 1);

    group->ref_count = 1;
    group->cmpl = g_completion_new (string_func);
    group->string_func = string_func;
    group->free_func = free_func;
    group->cmp_func = cmp_func;

    return group;
}


MooCompletionGroup *
moo_completion_group_new_text (gboolean sorted)
{
    return moo_completion_group_new (NULL, g_free,
                                     sorted ? (MooCompletionCmpFunc) strcmp : NULL);
}


void
moo_completion_group_set_data (MooCompletionGroup *group,
                               GList              *data)
{
    g_return_if_fail (group != NULL);

    g_completion_clear_items (group->cmpl);

    if (group->free_func)
        g_list_foreach (group->data, (GFunc) group->free_func, NULL);
    g_list_free (group->data);

    group->data = data;

    if (group->cmp_func)
        group->data = g_list_sort (group->data, (GCompareFunc) group->cmp_func);

    g_completion_add_items (group->cmpl, group->data);
}


void
moo_completion_group_add_data (MooCompletionGroup *group,
                               GList              *data)
{
    g_return_if_fail (group != NULL);

    if (!data)
        return;

    g_completion_clear_items (group->cmpl);

    group->data = g_list_concat (group->data, data);

    if (group->cmp_func)
        group->data = g_list_sort (group->data, (GCompareFunc) group->cmp_func);

    g_completion_add_items (group->cmpl, group->data);
}


static gboolean
moo_completion_group_find (MooCompletionGroup *group,
                           const char         *line,
                           int                *start_pos_p,
                           int                *end_pos_p)
{
    g_return_val_if_fail (group != NULL, FALSE);
    g_return_val_if_fail (line != NULL, FALSE);
    g_return_val_if_fail (group->regex != NULL, FALSE);

    egg_regex_clear (group->regex);

    if (egg_regex_match (group->regex, line, -1, 0) >= 1)
    {
        guint i;

        for (i = 0; i < group->n_parens; ++i)
        {
            int start_pos = -1, end_pos = -1;

            egg_regex_fetch_pos (group->regex, line, group->parens[i],
                                 &start_pos, &end_pos);

            if (start_pos >= 0 && end_pos >= 0)
            {
                if (start_pos_p)
                    *start_pos_p = start_pos;
                if (end_pos_p)
                    *end_pos_p = end_pos;
                return TRUE;
            }
        }
    }

    return FALSE;
}


static GList *
moo_completion_group_complete (MooCompletionGroup *group,
                               const char         *text,
                               char              **prefix)
{
    char *dummy = NULL;
    GList *retval;

    if (!prefix)
        prefix = &dummy;

    retval = g_completion_complete_utf8 (group->cmpl, text, prefix);

    g_free (dummy);
    return retval;
}


void
moo_completion_group_set_pattern (MooCompletionGroup *group,
                                  const char         *pattern,
                                  const guint        *parens,
                                  guint               n_parens)
{
    EggRegex *regex;
    GError *error = NULL;
    char *real_pattern;

    g_return_if_fail (group != NULL);
    g_return_if_fail (pattern && pattern[0]);
    g_return_if_fail (!parens || n_parens);

    real_pattern = g_strdup_printf ("%s$", pattern);
    regex = egg_regex_new (real_pattern, 0, 0, &error);

    if (error)
    {
        g_warning ("%s: %s", G_STRLOC, error->message);
        goto err;
    }

    egg_regex_optimize (regex, &error);

    if (error)
    {
        g_warning ("%s: %s", G_STRLOC, error->message);
        g_error_free (error);
    }

    egg_regex_unref (group->regex);
    group->regex = regex;

    g_free (group->parens);

    if (!parens)
    {
        group->parens = g_new0 (guint, 1);
        group->n_parens = 1;
    }
    else
    {
        group->parens = g_memdup (parens, n_parens * sizeof (guint));
        group->n_parens = n_parens;
    }

    return;

err:
    if (error)
        g_error_free (error);
    g_free (real_pattern);
    egg_regex_unref (regex);
}


GType
moo_completion_group_get_type (void)
{
    static GType type;

    if (!type)
        type = g_boxed_type_register_static ("MooCompletionGroup",
                                             (GBoxedCopyFunc) moo_completion_group_ref,
                                             (GBoxedFreeFunc) moo_completion_group_unref);

    return type;
}


MooCompletionGroup *
moo_completion_group_ref (MooCompletionGroup *group)
{
    g_return_val_if_fail (group != NULL, NULL);
    group->ref_count++;
    return group;
}


void
moo_completion_group_unref (MooCompletionGroup *group)
{
    g_return_if_fail (group != NULL);

    if (!(--group->ref_count))
    {
        egg_regex_unref (group->regex);
        g_free (group->parens);
        g_completion_free (group->cmpl);

        if (group->free_func)
            g_list_foreach (group->data, (GFunc) group->free_func, NULL);
        g_list_free (group->data);

        g_free (group);
    }
}
