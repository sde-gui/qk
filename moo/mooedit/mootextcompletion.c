/*
 *   mootextcompletion.c
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

#include "mooedit/mootextcompletion.h"
#include "mooedit/mootextpopup.h"
#include "mooutils/moomarshals.h"
#include "mooutils/eggregex.h"
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>


struct _MooTextCompletionPrivate {
    GtkTreeModel *model;
    GtkTreeModelFilter *filter;
    int text_column;
    MooTextCompletionFilterFunc filter_func;
    gpointer data;
    GDestroyNotify notify;

    EggRegex *regex;
    guint *parens;
    guint n_parens;

    char *line;
    char *text;
    guint text_len;

    GtkTextView *doc;
    GtkTextBuffer *buffer;
    GtkTextMark *start;
    GtkTextMark *end;

    MooTextPopup *popup;
    guint working : 1;
    guint in_update : 1;
};


static void     moo_text_completion_dispose         (GObject            *object);

static gboolean moo_text_completion_update_real     (MooTextCompletion  *cmpl);
static gboolean moo_text_completion_complete_real   (MooTextCompletion  *cmpl,
                                                     GtkTreeModel       *model,
                                                     GtkTreeIter        *iter);

static void     moo_text_completion_populate        (MooTextCompletion  *cmpl,
                                                     GtkTextIter        *match_start,
                                                     GtkTextIter        *match_end,
                                                     const char         *match,
                                                     guint               paren);
static void     moo_text_completion_emit_complete   (MooTextCompletion  *cmpl,
                                                     GtkTreeModel       *model,
                                                     GtkTreeIter        *iter);

static void     moo_text_completion_connect_popup   (MooTextCompletion  *cmpl);
static void     moo_text_completion_disconnect_popup(MooTextCompletion  *cmpl);

static gboolean moo_text_completion_empty           (MooTextCompletion  *cmpl);
static gboolean moo_text_completion_unique          (MooTextCompletion  *cmpl,
                                                     GtkTreeIter        *iter);

static gboolean filter_model_visible_func           (GtkTreeModel       *model,
                                                     GtkTreeIter        *iter,
                                                     gpointer            data);
static gboolean default_filter_func                 (MooTextCompletion *cmpl,
                                                     GtkTreeModel      *model,
                                                     GtkTreeIter       *iter,
                                                     gpointer           data);


G_DEFINE_TYPE (MooTextCompletion, moo_text_completion, G_TYPE_OBJECT)

enum {
    POPULATE,
    UPDATE,
    COMPLETE,
    NUM_SIGNALS
};

static guint signals[NUM_SIGNALS];


static void
moo_text_completion_dispose (GObject *object)
{
    MooTextCompletion *cmpl = MOO_TEXT_COMPLETION (object);

    if (cmpl->priv)
    {
        moo_text_completion_set_doc (cmpl, NULL);
        moo_text_completion_set_model (cmpl, NULL);

        if (cmpl->priv->notify)
            cmpl->priv->notify (cmpl->priv->data);

        if (cmpl->priv->regex)
            egg_regex_unref (cmpl->priv->regex);

        if (cmpl->priv->popup)
        {
            moo_text_completion_disconnect_popup (cmpl);
            g_object_unref (cmpl->priv->popup);
        }

        g_free (cmpl->priv->parens);
        g_free (cmpl->priv->text);
        g_free (cmpl->priv->line);
        g_free (cmpl->priv);
        cmpl->priv = NULL;
    }

    G_OBJECT_CLASS(moo_text_completion_parent_class)->dispose (object);
}


static void
moo_text_completion_class_init (MooTextCompletionClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->dispose = moo_text_completion_dispose;
    klass->complete = moo_text_completion_complete_real;
    klass->update = moo_text_completion_update_real;

    signals[POPULATE] =
            g_signal_new ("populate",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTextCompletionClass, populate),
                          NULL, NULL,
                          _moo_marshal_VOID__BOXED_BOXED_STRING_UINT,
                          G_TYPE_NONE, 4,
                          GTK_TYPE_TEXT_ITER | G_SIGNAL_TYPE_STATIC_SCOPE,
                          GTK_TYPE_TEXT_ITER | G_SIGNAL_TYPE_STATIC_SCOPE,
                          G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE,
                          G_TYPE_UINT);

    signals[UPDATE] =
            g_signal_new ("update",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTextCompletionClass, update),
                          g_signal_accumulator_true_handled, NULL,
                          _moo_marshal_BOOLEAN__VOID,
                          G_TYPE_BOOLEAN, 0);

    signals[COMPLETE] =
            g_signal_new ("complete",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTextCompletionClass, complete),
                          g_signal_accumulator_true_handled, NULL,
                          _moo_marshal_BOOLEAN__OBJECT_BOXED,
                          G_TYPE_BOOLEAN, 2,
                          GTK_TYPE_TREE_MODEL,
                          GTK_TYPE_TREE_ITER | G_SIGNAL_TYPE_STATIC_SCOPE);
}


static void
moo_text_completion_init (MooTextCompletion *cmpl)
{
    cmpl->priv = g_new0 (MooTextCompletionPrivate, 1);
    cmpl->priv->text_column = -1;
    cmpl->priv->filter_func = default_filter_func;
    cmpl->priv->popup = moo_text_popup_new (NULL);
    moo_text_completion_connect_popup (cmpl);
    cmpl->priv->regex = egg_regex_new ("\\b\\w+$", 0, 0, NULL);
    cmpl->priv->parens = g_new0 (guint, 1);
    cmpl->priv->n_parens = 1;
}


static gboolean
filter_model_visible_func (GtkTreeModel       *model,
                           GtkTreeIter        *iter,
                           gpointer            data)
{
    MooTextCompletion *cmpl = data;
    return cmpl->priv->filter_func (cmpl, model, iter,
                                    cmpl->priv->data);
}


static gboolean
default_filter_func (MooTextCompletion *cmpl,
                     GtkTreeModel      *model,
                     GtkTreeIter       *iter,
                     G_GNUC_UNUSED gpointer dummy)
{
    gboolean visible = TRUE;
    char *text = NULL;

    if (!cmpl->priv->text)
        return TRUE;

    g_return_val_if_fail (cmpl->priv->text_column >= 0, TRUE);

    gtk_tree_model_get (model, iter, cmpl->priv->text_column, &text, -1);
    g_return_val_if_fail (text != NULL, TRUE);

    if (cmpl->priv->text_len)
        visible = !strncmp (text, cmpl->priv->text, cmpl->priv->text_len);

    g_free (text);
    return visible;
}


MooTextCompletion *
moo_text_completion_new (void)
{
    return g_object_new (MOO_TYPE_TEXT_COMPLETION, NULL);
}


void
moo_text_completion_try_complete (MooTextCompletion *cmpl)
{
    GtkTextIter start, end;
    GtkTreeIter iter;
    gboolean found = FALSE;
    guint paren = 0;

    g_return_if_fail (MOO_IS_TEXT_COMPLETION (cmpl));
    g_return_if_fail (cmpl->priv->buffer != NULL);
    g_return_if_fail (cmpl->priv->model != NULL);
    g_return_if_fail (!cmpl->priv->working);

    cmpl->priv->working = TRUE;

    gtk_text_buffer_get_iter_at_mark (cmpl->priv->buffer, &end,
                                      gtk_text_buffer_get_insert (cmpl->priv->buffer));
    start = end;

    if (!gtk_text_iter_starts_line (&start))
        gtk_text_iter_set_line_offset (&start, 0);

    g_free (cmpl->priv->line);
    cmpl->priv->line = gtk_text_buffer_get_slice (cmpl->priv->buffer, &start, &end, TRUE);

    egg_regex_clear (cmpl->priv->regex);

    if (egg_regex_match (cmpl->priv->regex, cmpl->priv->line, -1, 0) >= 1)
    {
        guint i;

        for (i = 0; i < cmpl->priv->n_parens; ++i)
        {
            int start_pos = -1, end_pos = -1;

            egg_regex_fetch_pos (cmpl->priv->regex, cmpl->priv->line,
                                 cmpl->priv->parens[i], &start_pos, &end_pos);

            if (start_pos >= 0 && end_pos >= 0)
            {
                found = TRUE;
                paren = i;
                gtk_text_iter_set_line_index (&start, start_pos);
                gtk_text_iter_set_line_index (&end, end_pos);
                break;
            }
        }
    }

    g_free (cmpl->priv->text);

    if (found)
    {
        cmpl->priv->text = gtk_text_buffer_get_slice (cmpl->priv->buffer, &start, &end, TRUE);
        cmpl->priv->text_len = strlen (cmpl->priv->text);
        moo_text_completion_set_region (cmpl, &start, &end);
        moo_text_completion_populate (cmpl, &start, &end, cmpl->priv->text, paren);
    }
    else
    {
        cmpl->priv->text = NULL;
        cmpl->priv->text_len = 0;
        moo_text_completion_set_region (cmpl, &end, &end);
        moo_text_completion_populate (cmpl, &end, &end, NULL, 0);
    }

    if (moo_text_completion_empty (cmpl))
    {
        cmpl->priv->working = FALSE;
        return;
    }

    if (moo_text_completion_unique (cmpl, &iter))
    {
        moo_text_completion_emit_complete (cmpl, GTK_TREE_MODEL (cmpl->priv->filter), &iter);
        cmpl->priv->working = FALSE;
        return;
    }

    moo_text_popup_show (cmpl->priv->popup, &start);
}


static gboolean
moo_text_completion_update_real (MooTextCompletion *cmpl)
{
    GtkTextIter start, end;

    g_return_val_if_fail (cmpl->priv->working, TRUE);

    moo_text_completion_get_region (cmpl, &start, &end);
    g_free (cmpl->priv->text);
    cmpl->priv->text = gtk_text_buffer_get_slice (cmpl->priv->buffer, &start, &end, TRUE);
    cmpl->priv->text_len = strlen (cmpl->priv->text);

    gtk_tree_model_filter_refilter (cmpl->priv->filter);

    return TRUE;
}


static gboolean
moo_text_completion_complete_real (MooTextCompletion  *cmpl,
                                   GtkTreeModel       *model,
                                   GtkTreeIter        *iter)
{
    char *text = NULL;
    GtkTextIter start, end;

    g_return_val_if_fail (cmpl->priv->text_column >= 0, FALSE);

    gtk_tree_model_get (model, iter, cmpl->priv->text_column, &text, -1);
    g_return_val_if_fail (text != 0, FALSE);

    moo_text_completion_get_region (cmpl, &start, &end);
    gtk_text_buffer_begin_user_action (cmpl->priv->buffer);
    gtk_text_buffer_delete (cmpl->priv->buffer, &start, &end);
    gtk_text_buffer_insert (cmpl->priv->buffer, &start, text, -1);
    gtk_text_buffer_end_user_action (cmpl->priv->buffer);

    g_free (text);
    return TRUE;
}


static void
moo_text_completion_populate (MooTextCompletion  *cmpl,
                              GtkTextIter        *match_start,
                              GtkTextIter        *match_end,
                              const char         *match,
                              guint               paren)
{
    g_signal_emit (cmpl, signals[POPULATE], 0, match_start, match_end, match, paren);
}


static void
moo_text_completion_emit_complete (MooTextCompletion  *cmpl,
                                   GtkTreeModel       *model,
                                   GtkTreeIter        *iter)
{
    gboolean retval;
    g_signal_emit (cmpl, signals[COMPLETE], 0, model, iter, &retval);
}


static gboolean
moo_text_completion_empty (MooTextCompletion *cmpl)
{
    return !cmpl->priv->filter ||
            !gtk_tree_model_iter_n_children (GTK_TREE_MODEL (cmpl->priv->filter), NULL);
}


static gboolean
moo_text_completion_unique (MooTextCompletion  *cmpl,
                            GtkTreeIter        *iter)
{
    return cmpl->priv->filter &&
            gtk_tree_model_iter_n_children (GTK_TREE_MODEL (cmpl->priv->filter), NULL) == 1 &&
            gtk_tree_model_get_iter_first (GTK_TREE_MODEL (cmpl->priv->filter), iter);
}


static void
moo_text_completion_update (MooTextCompletion *cmpl)
{
    gboolean retval;

    if (!cmpl->priv->working || cmpl->priv->in_update)
        return;

    cmpl->priv->in_update = TRUE;
    g_signal_emit (cmpl, signals[UPDATE], 0, &retval);
    cmpl->priv->in_update = FALSE;

    if (moo_text_completion_empty (cmpl))
        moo_text_completion_hide (cmpl);
    else
        moo_text_popup_update (cmpl->priv->popup);
}


static void
on_popup_activate (MooTextCompletion *cmpl,
                   GtkTreeModel      *model,
                   GtkTreeIter       *iter)
{
    moo_text_completion_emit_complete (cmpl, model, iter);
}


static void
moo_text_completion_connect_popup (MooTextCompletion *cmpl)
{
    g_signal_connect_swapped (cmpl->priv->popup, "activate",
                              G_CALLBACK (on_popup_activate), cmpl);
    g_signal_connect_swapped (cmpl->priv->popup, "text-changed",
                              G_CALLBACK (moo_text_completion_update), cmpl);
    g_signal_connect_swapped (cmpl->priv->popup, "hide",
                              G_CALLBACK (moo_text_completion_hide), cmpl);
}


static void
moo_text_completion_disconnect_popup (MooTextCompletion *cmpl)
{
    g_signal_handlers_disconnect_by_func (cmpl->priv->popup,
                                          (gpointer) on_popup_activate,
                                          cmpl);
    g_signal_handlers_disconnect_by_func (cmpl->priv->popup,
                                          (gpointer) moo_text_completion_update,
                                          cmpl);
    g_signal_handlers_disconnect_by_func (cmpl->priv->popup,
                                          (gpointer) moo_text_completion_hide,
                                          cmpl);
}


void
moo_text_completion_complete (MooTextCompletion *cmpl)
{
    GtkTreeIter iter;

    g_return_if_fail (MOO_IS_TEXT_COMPLETION (cmpl));
    g_return_if_fail (cmpl->priv->working);

    if (moo_text_completion_unique (cmpl, &iter))
        moo_text_completion_emit_complete (cmpl, GTK_TREE_MODEL (cmpl->priv->filter), &iter);
    else
        moo_text_popup_activate (cmpl->priv->popup);

    moo_text_completion_hide (cmpl);
}


void
moo_text_completion_hide (MooTextCompletion *cmpl)
{
    g_return_if_fail (MOO_IS_TEXT_COMPLETION (cmpl));

    if (cmpl->priv->working)
    {
        cmpl->priv->working = FALSE;
        moo_text_popup_hide (cmpl->priv->popup);
    }
}


void
moo_text_completion_set_doc (MooTextCompletion  *cmpl,
                             GtkTextView        *doc)
{
    g_return_if_fail (MOO_IS_TEXT_COMPLETION (cmpl));
    g_return_if_fail (!doc || GTK_IS_TEXT_VIEW (doc));

    if (cmpl->priv->doc == doc)
        return;

    moo_text_completion_hide (cmpl);

    g_free (cmpl->priv->line);
    g_free (cmpl->priv->text);
    cmpl->priv->line = NULL;
    cmpl->priv->text = NULL;
    cmpl->priv->text_len = 0;

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
moo_text_completion_get_doc (MooTextCompletion *cmpl)
{
    g_return_val_if_fail (MOO_IS_TEXT_COMPLETION (cmpl), NULL);
    return cmpl->priv->doc;
}


void
moo_text_completion_set_model (MooTextCompletion  *cmpl,
                               GtkTreeModel       *model)
{
    g_return_if_fail (MOO_IS_TEXT_COMPLETION (cmpl));
    g_return_if_fail (!model || GTK_IS_TREE_MODEL (model));

    if (cmpl->priv->model == model)
        return;

    moo_text_completion_hide (cmpl);

    if (cmpl->priv->model)
    {
        g_object_unref (cmpl->priv->filter);
        g_object_unref (cmpl->priv->model);
        cmpl->priv->filter = NULL;
        cmpl->priv->model = NULL;
    }

    if (model)
    {
        cmpl->priv->model = g_object_ref (model);
        cmpl->priv->filter = GTK_TREE_MODEL_FILTER (
                gtk_tree_model_filter_new (cmpl->priv->model, NULL));
        gtk_tree_model_filter_set_visible_func (cmpl->priv->filter,
                                                filter_model_visible_func,
                                                cmpl, NULL);
    }

    moo_text_popup_set_model (cmpl->priv->popup,
                              GTK_TREE_MODEL (cmpl->priv->filter));
}


GtkTreeModel *
moo_text_completion_get_model (MooTextCompletion *cmpl)
{
    g_return_val_if_fail (MOO_IS_TEXT_COMPLETION (cmpl), NULL);
    return cmpl->priv->model;
}


void
moo_text_completion_set_filter_func (MooTextCompletion  *cmpl,
                                     MooTextCompletionFilterFunc func,
                                     gpointer            data,
                                     GDestroyNotify      notify)
{
    g_return_if_fail (MOO_IS_TEXT_COMPLETION (cmpl));

    if (!func)
        return moo_text_completion_set_filter_func (cmpl, default_filter_func, NULL, NULL);

    if (cmpl->priv->notify)
        cmpl->priv->notify (cmpl->priv->data);

    cmpl->priv->filter_func = func;
    cmpl->priv->notify = notify;
    cmpl->priv->data = data;

    moo_text_completion_update (cmpl);
}


void
moo_text_completion_set_regex (MooTextCompletion  *cmpl,
                               const char         *pattern,
                               const guint        *parens,
                               guint               n_parens)
{
    EggRegex *regex;
    GError *error = NULL;
    char *real_pattern;

    g_return_if_fail (MOO_IS_TEXT_COMPLETION (cmpl));
    g_return_if_fail (pattern && pattern[0]);
    g_return_if_fail (!parens || n_parens);

    moo_text_completion_hide (cmpl);

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

    egg_regex_unref (cmpl->priv->regex);
    g_free (cmpl->priv->parens);

    if (!parens)
    {
        cmpl->priv->parens = g_new0 (guint, 1);
        cmpl->priv->n_parens = 1;
    }
    else
    {
        cmpl->priv->parens = g_memdup (parens, n_parens * sizeof (guint));
        cmpl->priv->n_parens = n_parens;
    }

    return;

err:
    if (error)
        g_error_free (error);
    g_free (real_pattern);
    egg_regex_unref (regex);
}


void
moo_text_completion_set_text_column (MooTextCompletion  *cmpl,
                                     int                 column)
{
    g_return_if_fail (MOO_IS_TEXT_COMPLETION (cmpl));
    cmpl->priv->text_column = column;
}


MooTextPopup *
moo_text_completion_get_popup (MooTextCompletion *cmpl)
{
    g_return_val_if_fail (MOO_IS_TEXT_COMPLETION (cmpl), NULL);
    return cmpl->priv->popup;
}


gboolean
moo_text_completion_get_region (MooTextCompletion  *cmpl,
                                GtkTextIter        *start,
                                GtkTextIter        *end)
{
    g_return_val_if_fail (MOO_IS_TEXT_COMPLETION (cmpl), FALSE);

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
moo_text_completion_set_region (MooTextCompletion  *cmpl,
                                const GtkTextIter  *start,
                                const GtkTextIter  *end)
{
    g_return_if_fail (MOO_IS_TEXT_COMPLETION (cmpl));
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
    moo_text_completion_update (cmpl);
}


MooTextCompletion *
moo_text_completion_new_text (GtkTextView  *doc,
                              GtkTreeModel *model,
                              int           text_column)
{
    MooTextCompletion *cmpl;
    GtkCellRenderer *cell;

    g_return_val_if_fail (GTK_IS_TREE_MODEL (model), NULL);
    g_return_val_if_fail (gtk_tree_model_get_column_type (model, text_column) == G_TYPE_STRING, NULL);

    cmpl = moo_text_completion_new ();
    moo_text_completion_set_model (cmpl, model);
    moo_text_completion_set_text_column (cmpl, text_column);

    cell = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (cmpl->priv->popup->column, cell, TRUE);
    gtk_tree_view_column_set_attributes (cmpl->priv->popup->column, cell,
                                         "text", text_column, NULL);

    if (doc)
        moo_text_completion_set_doc (cmpl, doc);

    return cmpl;
}
