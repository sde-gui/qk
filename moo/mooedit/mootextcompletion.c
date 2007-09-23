/*
 *   mootextcompletion.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#include "mooedit/mootextcompletion.h"
#include "mooutils/moomarshals.h"
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>


struct _MooTextCompletionPrivate {
    GtkTextView *doc;
    GtkTextBuffer *buffer;
    GtkTextMark *start;
    GtkTextMark *end;

    GtkTreeModel *model;
    MooTextCompletionTextFunc text_func;
    gpointer text_func_data;
    GDestroyNotify text_func_data_notify;

    MooTextPopup *popup;
    guint shown : 1;
    guint working : 1;
};


static void     moo_text_completion_dispose             (GObject            *object);

static void     moo_text_completion_try_complete_real   (MooTextCompletion  *cmpl,
                                                         gboolean            automatic);
static void     moo_text_completion_complete_real       (MooTextCompletion  *cmpl,
                                                         GtkTreeModel       *model,
                                                         GtkTreeIter        *iter);

static void     moo_text_completion_populate            (MooTextCompletion  *cmpl,
                                                         GtkTreeModel       *model,
                                                         GtkTextIter        *cursor,
                                                         const char         *text);
static void     moo_text_completion_complete            (MooTextCompletion  *cmpl,
                                                         GtkTreeIter        *iter);
static char    *moo_text_completion_get_text            (MooTextCompletion  *cmpl,
                                                         GtkTreeModel       *model,
                                                         GtkTreeIter        *iter);
static void     moo_text_completion_update              (MooTextCompletion  *cmpl);
static void     moo_text_completion_update_real         (MooTextCompletion  *cmpl);

static gboolean moo_text_completion_empty               (MooTextCompletion  *cmpl);
static gboolean moo_text_completion_unique              (MooTextCompletion  *cmpl,
                                                         GtkTreeIter        *iter);
static void     moo_text_completion_finish              (MooTextCompletion  *cmpl);

static char    *find_common_prefix                      (MooTextCompletion  *cmpl,
                                                         const char         *text);
static char    *default_text_func                       (GtkTreeModel       *model,
                                                         GtkTreeIter        *iter,
                                                         gpointer            data);
static void     cell_data_func                          (GtkTreeViewColumn  *column,
                                                         GtkCellRenderer    *cell,
                                                         GtkTreeModel       *model,
                                                         GtkTreeIter        *iter,
                                                         MooTextCompletion  *cmpl);
static void     on_popup_activate                       (MooTextCompletion  *cmpl,
                                                         GtkTreeModel       *model,
                                                         GtkTreeIter        *iter);
static gboolean popup_key_press                         (MooTextCompletion  *cmpl,
                                                         GdkEventKey        *event);

enum {
    FINISH,
    N_SIGNALS
};

static guint signals[N_SIGNALS];

G_DEFINE_TYPE (MooTextCompletion, moo_text_completion, G_TYPE_OBJECT)


static void
moo_text_completion_class_init (MooTextCompletionClass *klass)
{
    G_OBJECT_CLASS(klass)->dispose = moo_text_completion_dispose;

    klass->try_complete = moo_text_completion_try_complete_real;
    klass->update = moo_text_completion_update_real;
    klass->complete = moo_text_completion_complete_real;

    signals[FINISH] = g_signal_new ("finish",
                                    G_TYPE_FROM_CLASS (klass),
                                    G_SIGNAL_RUN_LAST,
                                    G_STRUCT_OFFSET (MooTextCompletionClass, finish),
                                    NULL, NULL,
                                    _moo_marshal_VOID__VOID,
                                    G_TYPE_NONE, 0);
}


void
moo_text_completion_init (MooTextCompletion *cmpl)
{
    GtkCellRenderer *cell;

    cmpl->priv = g_new0 (MooTextCompletionPrivate, 1);

    cmpl->priv->doc = NULL;
    cmpl->priv->buffer = NULL;
    cmpl->priv->start = NULL;
    cmpl->priv->end = NULL;

    cmpl->priv->model = GTK_TREE_MODEL (gtk_list_store_new (1, G_TYPE_STRING));
    cmpl->priv->text_func = default_text_func;
    cmpl->priv->text_func_data = GINT_TO_POINTER (0);
    cmpl->priv->text_func_data_notify = NULL;

    cmpl->priv->popup = g_object_new (MOO_TYPE_TEXT_POPUP, NULL);

    cell = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (cmpl->priv->popup->column, cell, TRUE);
    gtk_tree_view_column_set_cell_data_func (cmpl->priv->popup->column, cell,
                                             (GtkTreeCellDataFunc) cell_data_func,
                                             cmpl, NULL);

    moo_text_popup_set_model (cmpl->priv->popup, cmpl->priv->model);
    g_signal_connect_swapped (cmpl->priv->popup, "activate",
                              G_CALLBACK (on_popup_activate), cmpl);
    g_signal_connect_swapped (cmpl->priv->popup, "text-changed",
                              G_CALLBACK (moo_text_completion_update), cmpl);
    g_signal_connect_swapped (cmpl->priv->popup, "hide",
                              G_CALLBACK (moo_text_completion_hide), cmpl);
    g_signal_connect_swapped (cmpl->priv->popup, "key-press-event",
                              G_CALLBACK (popup_key_press), cmpl);
}


static void
moo_text_completion_dispose (GObject *object)
{
    MooTextCompletion *cmpl = MOO_TEXT_COMPLETION (object);

    if (cmpl->priv)
    {
        moo_text_completion_set_doc (cmpl, NULL);
        moo_text_completion_set_text_func (cmpl, NULL, NULL, NULL);

        g_object_unref (cmpl->priv->model);

        moo_text_popup_set_model (cmpl->priv->popup, NULL);
        g_signal_handlers_disconnect_by_func (cmpl->priv->popup,
                                              (gpointer) on_popup_activate,
                                              cmpl);
        g_signal_handlers_disconnect_by_func (cmpl->priv->popup,
                                              (gpointer) moo_text_completion_update,
                                              cmpl);
        g_signal_handlers_disconnect_by_func (cmpl->priv->popup,
                                              (gpointer) moo_text_completion_hide,
                                              cmpl);
        g_object_unref (cmpl->priv->popup);

        g_free (cmpl->priv);
        cmpl->priv = NULL;
    }

    G_OBJECT_CLASS (moo_text_completion_parent_class)->dispose (object);
}


void
moo_text_completion_try_complete (MooTextCompletion  *cmpl,
                                  gboolean            automatic)
{
    g_return_if_fail (MOO_IS_TEXT_COMPLETION (cmpl));
    g_return_if_fail (MOO_TEXT_COMPLETION_GET_CLASS (cmpl)->try_complete != NULL);
    g_return_if_fail (moo_text_completion_get_doc (cmpl) != NULL);

    MOO_TEXT_COMPLETION_GET_CLASS (cmpl)->try_complete (cmpl, automatic);
}


static void
moo_text_completion_try_complete_real (MooTextCompletion *cmpl,
                                       gboolean           automatic)
{
    GtkTextIter start, end;
    GtkTreeIter iter;
    char *line = NULL;
    char *text = NULL;
    char *prefix = NULL;

    g_return_if_fail (cmpl->priv->buffer != NULL);
    g_return_if_fail (!cmpl->priv->shown);

    gtk_text_buffer_get_iter_at_mark (cmpl->priv->buffer, &end,
                                      gtk_text_buffer_get_insert (cmpl->priv->buffer));
    start = end;

    if (!gtk_text_iter_starts_line (&start))
        gtk_text_iter_set_line_offset (&start, 0);

    line = gtk_text_iter_get_slice (&start, &end);

    cmpl->priv->working = TRUE;

    moo_text_completion_set_region (cmpl, &end, &end);
    moo_text_completion_populate (cmpl, cmpl->priv->model, &end, line);

    if (moo_text_completion_empty (cmpl))
        goto finish;

    if (moo_text_completion_unique (cmpl, &iter))
    {
        if (!automatic)
        {
            moo_text_completion_complete (cmpl, &iter);
            goto finish;
        }

        prefix = moo_text_completion_get_text (cmpl, cmpl->priv->model, &iter);

        if (prefix)
        {
            moo_text_completion_get_region (cmpl, &start, NULL);
            end = start;
            gtk_text_iter_forward_chars (&end, g_utf8_strlen (prefix, -1));
            text = gtk_text_iter_get_slice (&start, &end);

            if (!strcmp (prefix, text))
                goto finish;
        }
    }
    else if (!automatic)
    {
        moo_text_completion_get_region (cmpl, &start, &end);
        text = gtk_text_iter_get_slice (&start, &end);
        prefix = find_common_prefix (cmpl, text);

        if (prefix && strcmp (text, prefix) != 0)
        {
            gtk_text_buffer_begin_user_action (cmpl->priv->buffer);
            gtk_text_buffer_delete (cmpl->priv->buffer, &start, &end);
            gtk_text_buffer_insert (cmpl->priv->buffer, &start, prefix, -1);
            gtk_text_buffer_end_user_action (cmpl->priv->buffer);
        }
    }

    moo_text_completion_show (cmpl);

    g_free (text);
    g_free (prefix);
    g_free (line);
    return;

finish:
    moo_text_completion_finish (cmpl);
    g_free (text);
    g_free (prefix);
    g_free (line);
    return;
}


static void
moo_text_completion_complete (MooTextCompletion *cmpl,
                              GtkTreeIter       *iter)
{
    g_return_if_fail (MOO_TEXT_COMPLETION_GET_CLASS(cmpl)->complete != NULL);
    MOO_TEXT_COMPLETION_GET_CLASS(cmpl)->complete (cmpl, cmpl->priv->model, iter);
    moo_text_completion_finish (cmpl);
}

static void
moo_text_completion_complete_real (MooTextCompletion *cmpl,
                                   GtkTreeModel      *model,
                                   GtkTreeIter       *iter)
{
    char *text, *old_text;
    GtkTextIter start, end;
    gboolean set_cursor = FALSE;

    text = moo_text_completion_get_text (cmpl, model, iter);
    g_return_if_fail (text != NULL);

    moo_text_completion_get_region (cmpl, &start, &end);
    old_text = gtk_text_buffer_get_slice (cmpl->priv->buffer,
                                          &start, &end, TRUE);

    gtk_text_buffer_begin_user_action (cmpl->priv->buffer);

    if (strcmp (text, old_text))
    {
        gtk_text_buffer_delete (cmpl->priv->buffer, &start, &end);
        gtk_text_buffer_insert (cmpl->priv->buffer, &start, text, -1);
        set_cursor = TRUE;
    }

    if (set_cursor)
        gtk_text_buffer_place_cursor (cmpl->priv->buffer, &start);

    gtk_text_buffer_end_user_action (cmpl->priv->buffer);

    moo_text_completion_hide (cmpl);

    g_free (old_text);
}


static char *
moo_text_completion_get_text (MooTextCompletion *cmpl,
                              GtkTreeModel      *model,
                              GtkTreeIter       *iter)
{
    g_return_val_if_fail (cmpl->priv->text_func != NULL, NULL);
    return cmpl->priv->text_func (model, iter, cmpl->priv->text_func_data);
}


static void
moo_text_completion_populate (MooTextCompletion *cmpl,
                              GtkTreeModel      *model,
                              GtkTextIter       *cursor,
                              const char        *text)
{
    g_return_if_fail (MOO_TEXT_COMPLETION_GET_CLASS (cmpl)->populate != NULL);

    if (GTK_IS_LIST_STORE (cmpl->priv->model))
        gtk_list_store_clear (GTK_LIST_STORE (cmpl->priv->model));

    MOO_TEXT_COMPLETION_GET_CLASS (cmpl)->populate (cmpl, model, cursor, text);
}


static void
moo_text_completion_update (MooTextCompletion *cmpl)
{
    g_return_if_fail (MOO_TEXT_COMPLETION_GET_CLASS(cmpl)->update != NULL);
    MOO_TEXT_COMPLETION_GET_CLASS(cmpl)->update (cmpl);
}

static void
moo_text_completion_update_real (MooTextCompletion *cmpl)
{
    GtkTextIter start, end;
    char *text;

    g_return_if_fail (cmpl->priv->shown);

    moo_text_completion_get_region (cmpl, &start, &end);
    text = gtk_text_iter_get_slice (&start, &end);

    moo_text_completion_populate (cmpl, cmpl->priv->model, &end, text);
    /* XXX preserve selected row */

    if (!moo_text_completion_empty (cmpl))
        moo_text_popup_update (cmpl->priv->popup);
    else
        moo_text_completion_hide (cmpl);

    g_free (text);
}


static gboolean
moo_text_completion_empty (MooTextCompletion *cmpl)
{
    return !gtk_tree_model_iter_n_children (cmpl->priv->model, NULL);
}


static gboolean
moo_text_completion_unique (MooTextCompletion *cmpl,
                            GtkTreeIter       *iter)
{
    return gtk_tree_model_iter_n_children (cmpl->priv->model, NULL) == 1 &&
            gtk_tree_model_get_iter_first (cmpl->priv->model, iter);
}


void
moo_text_completion_show (MooTextCompletion *cmpl)
{
    GtkTextIter iter;
    g_return_if_fail (MOO_IS_TEXT_COMPLETION (cmpl));
    cmpl->priv->shown = TRUE;
    moo_text_completion_get_region (cmpl, &iter, NULL);
    moo_text_popup_show (cmpl->priv->popup, &iter);
}


static void
moo_text_completion_finish (MooTextCompletion *cmpl)
{
    gboolean emit = FALSE;

    if (cmpl->priv->working)
    {
        cmpl->priv->working = FALSE;
        emit = TRUE;
    }

    if (cmpl->priv->shown)
        moo_text_completion_hide (cmpl);

    if (emit)
        g_signal_emit (cmpl, signals[FINISH], 0);
}


void
moo_text_completion_hide (MooTextCompletion *cmpl)
{
    g_return_if_fail (MOO_IS_TEXT_COMPLETION (cmpl));

    if (cmpl->priv->shown)
    {
        cmpl->priv->shown = FALSE;
        moo_text_popup_hide (cmpl->priv->popup);
        if (GTK_IS_LIST_STORE (cmpl->priv->model))
            gtk_list_store_clear (GTK_LIST_STORE (cmpl->priv->model));
    }

    if (cmpl->priv->working)
        moo_text_completion_finish (cmpl);
}


static void
on_popup_activate (MooTextCompletion *cmpl,
                   G_GNUC_UNUSED GtkTreeModel *model,
                   GtkTreeIter       *iter)
{
    moo_text_completion_complete (cmpl, iter);
}


static gboolean
popup_key_press (MooTextCompletion *cmpl,
                 GdkEventKey       *event)
{
    GtkTreeIter iter;

    switch (event->keyval)
    {
        case GDK_Tab:
            if (moo_text_completion_unique (cmpl, &iter))
            {
                moo_text_completion_complete (cmpl, &iter);
                return TRUE;
            }
        /* fall through */
        default:
            return FALSE;
    }
}


static char *
find_common_prefix (MooTextCompletion *cmpl,
                    const char        *text)
{
    GtkTreeIter iter;
    guint text_len;
    char *prefix = NULL;
    guint prefix_len;

    g_return_val_if_fail (text != NULL, NULL);

    if (!text[0])
        return NULL;

    text_len = strlen (text);

    if (!gtk_tree_model_get_iter_first (cmpl->priv->model, &iter))
        return NULL;

    do
    {
        char *entry;

        entry = moo_text_completion_get_text (cmpl, cmpl->priv->model, &iter);

        if (!entry)
            continue;

        if (!prefix)
        {
            prefix = g_strdup (entry);
            prefix_len = g_utf8_strlen (entry, -1);
        }
        else
        {
            guint i;
            char *p, *e;

            for (p = prefix, e = entry, i = 0; *e && i < prefix_len; ++i)
            {
                if (g_utf8_get_char (p) != g_utf8_get_char (e))
                {
                    prefix_len = i;
                    break;
                }

                p = g_utf8_next_char (p);
                e = g_utf8_next_char (e);
            }
        }

        g_free (entry);
    }
    while ((!prefix || prefix_len) &&
           gtk_tree_model_iter_next (cmpl->priv->model, &iter));

    if (prefix && !prefix_len)
    {
        g_free (prefix);
        prefix = NULL;
    }

    if (prefix)
        * g_utf8_offset_to_pointer (prefix, prefix_len) = 0;

    return prefix;
}


static char *
default_text_func (GtkTreeModel *model,
                   GtkTreeIter  *iter,
                   gpointer      data)
{
    char *text = NULL;
    int column = GPOINTER_TO_INT (data);

    g_return_val_if_fail (column >= 0, NULL);
    g_return_val_if_fail (gtk_tree_model_get_column_type (model, column) == G_TYPE_STRING, NULL);

    gtk_tree_model_get (model, iter, column, &text, -1);
    return text;
}


static void
cell_data_func (G_GNUC_UNUSED GtkTreeViewColumn *column,
                GtkCellRenderer   *cell,
                GtkTreeModel      *model,
                GtkTreeIter       *iter,
                MooTextCompletion *cmpl)
{
    char *text = moo_text_completion_get_text (cmpl, model, iter);
    g_object_set (cell, "text", text, NULL);
    g_free (text);
}


void
moo_text_completion_set_doc (MooTextCompletion *cmpl,
                             GtkTextView       *doc)
{
    g_return_if_fail (MOO_IS_TEXT_COMPLETION (cmpl));
    g_return_if_fail (!doc || GTK_IS_TEXT_VIEW (doc));

    if (cmpl->priv->doc == doc)
        return;

    moo_text_completion_hide (cmpl);

    if (cmpl->priv->doc)
    {
        if (cmpl->priv->start)
            gtk_text_buffer_delete_mark (cmpl->priv->buffer, cmpl->priv->start);
        if (cmpl->priv->end)
            gtk_text_buffer_delete_mark (cmpl->priv->buffer, cmpl->priv->end);
        g_object_unref (cmpl->priv->buffer);
        g_object_unref (cmpl->priv->doc);
        cmpl->priv->start = cmpl->priv->end = NULL;
        cmpl->priv->buffer = NULL;
        cmpl->priv->doc = NULL;
    }

    if (doc)
    {
        cmpl->priv->doc = g_object_ref (doc);
        cmpl->priv->buffer = g_object_ref (gtk_text_view_get_buffer (doc));
    }

    moo_text_popup_set_doc (cmpl->priv->popup, doc);
}


GtkTextView *
moo_text_completion_get_doc (MooTextCompletion *cmpl)
{
    g_return_val_if_fail (MOO_IS_TEXT_COMPLETION (cmpl), NULL);
    return cmpl->priv->doc;
}

GtkTextBuffer *
moo_text_completion_get_buffer (MooTextCompletion *cmpl)
{
    g_return_val_if_fail (MOO_IS_TEXT_COMPLETION (cmpl), NULL);
    return cmpl->priv->buffer;
}


void
moo_text_completion_set_region (MooTextCompletion *cmpl,
                                const GtkTextIter *start,
                                const GtkTextIter *end)
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
}


gboolean
moo_text_completion_get_region (MooTextCompletion *cmpl,
                                GtkTextIter       *start,
                                GtkTextIter       *end)
{
    g_return_val_if_fail (MOO_IS_TEXT_COMPLETION (cmpl), FALSE);

    if (!cmpl->priv->start)
        return FALSE;

    if (start)
        gtk_text_buffer_get_iter_at_mark (cmpl->priv->buffer, start,
                                          cmpl->priv->start);
    if (end)
        gtk_text_buffer_get_iter_at_mark (cmpl->priv->buffer, end,
                                          cmpl->priv->end);

    return TRUE;
}


void
moo_text_completion_set_model (MooTextCompletion *cmpl,
                               GtkTreeModel      *model)
{
    g_return_if_fail (MOO_IS_TEXT_COMPLETION (cmpl));
    g_return_if_fail (!model || GTK_IS_TREE_MODEL (model));

    if (model == cmpl->priv->model)
        return;

    if (cmpl->priv->model)
    {
        g_object_unref (cmpl->priv->model);
        cmpl->priv->model = NULL;
    }

    if (model)
        cmpl->priv->model = g_object_ref (model);

    moo_text_popup_set_model (cmpl->priv->popup, model);
}


GtkTreeModel *
moo_text_completion_get_model (MooTextCompletion *cmpl)
{
    g_return_val_if_fail (MOO_IS_TEXT_COMPLETION (cmpl), NULL);
    return cmpl->priv->model;
}


void
moo_text_completion_set_text_column (MooTextCompletion *cmpl,
                                     int                column)
{
    g_return_if_fail (MOO_IS_TEXT_COMPLETION (cmpl));
    g_return_if_fail (column >= 0);
    moo_text_completion_set_text_func (cmpl, default_text_func,
                                       GINT_TO_POINTER (column), NULL);
}


void
moo_text_completion_set_text_func (MooTextCompletion  *cmpl,
                                   MooTextCompletionTextFunc func,
                                   gpointer            data,
                                   GDestroyNotify      notify)
{
    GDestroyNotify old_notify;
    gpointer old_data;

    g_return_if_fail (MOO_IS_TEXT_COMPLETION (cmpl));

    old_notify = cmpl->priv->text_func_data_notify;
    old_data = cmpl->priv->text_func_data;

    cmpl->priv->text_func = NULL;
    cmpl->priv->text_func_data = NULL;
    cmpl->priv->text_func_data_notify = NULL;

    if (func)
    {
        cmpl->priv->text_func = func;
        cmpl->priv->text_func_data = data;
        cmpl->priv->text_func_data_notify = notify;
    }

    if (old_notify)
        old_notify (old_data);
}


MooTextPopup *
moo_text_completion_get_popup (MooTextCompletion *cmpl)
{
    g_return_val_if_fail (MOO_IS_TEXT_COMPLETION (cmpl), NULL);
    return cmpl->priv->popup;
}
