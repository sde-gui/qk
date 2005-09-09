/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   moopaneview.c
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

#include "mooedit/moopaneview.h"
#include "mooutils/moomarshals.h"


struct _MooPaneViewPrivate {
    GHashTable *line_data;
    gboolean busy;
    gboolean scrolled;
    GtkTextMark *end_mark;
};

typedef struct {
    gpointer data;
    GDestroyNotify free_func;
} LineData;

static LineData *line_data_new              (gpointer        data,
                                             GDestroyNotify  free_func);
static void      line_data_free             (LineData       *line_data);

static void      moo_pane_view_finalize     (GObject        *object);

// static void      moo_pane_view_set_property (GObject        *object,
//                                              guint           prop_id,
//                                              const GValue   *value,
//                                              GParamSpec     *pspec);
// static void      moo_pane_view_get_property (GObject        *object,
//                                              guint           prop_id,
//                                              GValue         *value,
//                                              GParamSpec     *pspec);

static void      moo_pane_view_realize      (GtkWidget      *widget);
static gboolean  moo_pane_view_button_press (GtkWidget      *widget,
                                             GdkEventButton *event);

static GtkTextBuffer *get_buffer            (MooPaneView    *view);
static GHashTable *get_hash_table           (MooPaneView    *view);


enum {
    CLICK,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];


enum {
    PROP_0,
};


/* MOO_TYPE_PANE_VIEW */
G_DEFINE_TYPE (MooPaneView, moo_pane_view, MOO_TYPE_TEXT_VIEW)


static void moo_pane_view_class_init (MooPaneViewClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

//     gobject_class->set_property = moo_pane_view_set_property;
//     gobject_class->get_property = moo_pane_view_get_property;
    gobject_class->finalize = moo_pane_view_finalize;

    widget_class->realize = moo_pane_view_realize;
    widget_class->button_press_event = moo_pane_view_button_press;

    signals[CLICK] =
            g_signal_new ("click",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooPaneViewClass, click),
                          g_signal_accumulator_true_handled, NULL,
                          _moo_marshal_BOOL__POINTER_INT,
                          G_TYPE_BOOLEAN, 2,
                          G_TYPE_POINTER, G_TYPE_INT);
}


static void moo_pane_view_init (MooPaneView *view)
{
    view->priv = g_new0 (MooPaneViewPrivate, 1);

    g_object_set (view,
                  "editable", FALSE,
                  "cursor-visible", FALSE,
                  "current-line-color", "grey",
                  NULL);
}


static void moo_pane_view_finalize       (GObject      *object)
{
    MooPaneView *view = MOO_PANE_VIEW (object);

    if (view->priv->line_data)
        g_hash_table_destroy (view->priv->line_data);

    g_free (view->priv);

    G_OBJECT_CLASS (moo_pane_view_parent_class)->finalize (object);
}


GtkWidget*
moo_pane_view_new (void)
{
    return g_object_new (MOO_TYPE_PANE_VIEW, NULL);
}


void
moo_pane_view_clear (MooPaneView *view)
{
    GtkTextBuffer *buffer;
    GtkTextIter start, end;

    g_return_if_fail (MOO_IS_PANE_VIEW (view));

    buffer = get_buffer (view);
    gtk_text_buffer_get_bounds (buffer, &start, &end);
    gtk_text_buffer_delete (buffer, &start, &end);

    if (view->priv->line_data)
    {
        g_hash_table_destroy (view->priv->line_data);
        view->priv->line_data = NULL;
    }
}


static GtkTextBuffer*
get_buffer (MooPaneView *view)
{
    return gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
}


static LineData*
line_data_new (gpointer        data,
               GDestroyNotify  free_func)
{
    LineData *line_data;

    g_return_val_if_fail (data != NULL, NULL);

    line_data = g_new (LineData, 1);
    line_data->data = data;
    line_data->free_func = free_func;

    return line_data;
}


static void
line_data_free (LineData *line_data)
{
    if (line_data)
    {
        if (line_data->data && line_data->free_func)
            line_data->free_func (line_data->data);
        g_free (line_data);
    }
}


static GHashTable*
get_hash_table (MooPaneView *view)
{
    if (!view->priv->line_data)
        view->priv->line_data =
                g_hash_table_new_full (g_direct_hash, g_direct_equal,
                                       NULL, (GDestroyNotify) line_data_free);

    return view->priv->line_data;
}


static gboolean
moo_pane_view_button_press (GtkWidget      *widget,
                            GdkEventButton *event)
{
    GtkTextView *textview = GTK_TEXT_VIEW (widget);
    MooPaneView *view = MOO_PANE_VIEW (widget);
    int buffer_x, buffer_y, line;
    GtkTextIter iter;
    gpointer data;
    gboolean handled = FALSE;

    if (gtk_text_view_get_window_type (textview, event->window) == GTK_TEXT_WINDOW_TEXT)
    {
        gtk_text_view_window_to_buffer_coords (textview,
                                               GTK_TEXT_WINDOW_TEXT,
                                               event->x, event->y,
                                               &buffer_x, &buffer_y);
        /* XXX */
        gtk_text_view_get_line_at_y (textview, &iter, buffer_y, NULL);

        line = gtk_text_iter_get_line (&iter);
        data = moo_pane_view_get_line_data (view, line);

        g_signal_emit (view, signals[CLICK], 0, data, line, &handled);

        if (handled)
            gtk_text_buffer_place_cursor (get_buffer (view), &iter);
    }

    if (!handled)
        return GTK_WIDGET_CLASS(moo_pane_view_parent_class)->button_press_event (widget, event);
    else
        return TRUE;
}


static void
moo_pane_view_realize (GtkWidget *widget)
{
    GdkWindow *window;

    GTK_WIDGET_CLASS(moo_pane_view_parent_class)->realize (widget);

    window = gtk_text_view_get_window (GTK_TEXT_VIEW (widget),
                                       GTK_TEXT_WINDOW_TEXT);
    gdk_window_set_cursor (window, NULL);
}


/* XXX check line */
void
moo_pane_view_set_line_data (MooPaneView    *view,
                             int             line,
                             gpointer        data,
                             GDestroyNotify  free_func)
{
    g_return_if_fail (MOO_IS_PANE_VIEW (view));
    g_return_if_fail (line >= 0);

    if (data)
    {
        GHashTable *hash = get_hash_table (view);
        g_hash_table_insert (hash, GINT_TO_POINTER (line),
                             line_data_new (data, free_func));
    }
    else if (view->priv->line_data)
    {
        g_hash_table_remove (view->priv->line_data, GINT_TO_POINTER (line));
    }
}


gpointer
moo_pane_view_get_line_data (MooPaneView    *view,
                             int             line)
{
    g_return_val_if_fail (MOO_IS_PANE_VIEW (view), NULL);
    g_return_val_if_fail (line >= 0, NULL);

    if (!view->priv->line_data)
    {
        return NULL;
    }
    else
    {
        LineData *line_data =
                g_hash_table_lookup (view->priv->line_data,
                                     GINT_TO_POINTER (line));
        return line_data ? line_data->data : NULL;
    }
}


GtkTextTag*
moo_pane_view_create_tag (MooPaneView    *view,
                          const char     *tag_name,
                          const char     *first_property_name,
                          ...)
{
    GtkTextTag *tag;
    GtkTextBuffer *buffer;
    va_list list;

    g_return_val_if_fail (MOO_IS_PANE_VIEW (view), NULL);

    buffer = get_buffer (view);
    tag = gtk_text_buffer_create_tag (buffer, tag_name, NULL);
    g_return_val_if_fail (tag != NULL, NULL);

    if (first_property_name)
    {
        va_start (list, first_property_name);
        g_object_set_valist (G_OBJECT (tag), first_property_name, list);
        va_end (list);
    }

    return tag;
}


GtkTextTag*
moo_pane_view_lookup_tag (MooPaneView    *view,
                          const char     *tag_name)
{
    GtkTextTagTable *table;
    GtkTextBuffer *buffer;

    g_return_val_if_fail (MOO_IS_PANE_VIEW (view), NULL);
    g_return_val_if_fail (tag_name != NULL, NULL);

    buffer = get_buffer (view);
    table = gtk_text_buffer_get_tag_table (buffer);

    return gtk_text_tag_table_lookup (table, tag_name);
}


int
moo_pane_view_write_line (MooPaneView    *view,
                          const char     *text,
                          gssize          len,
                          GtkTextTag     *tag)
{
    int line;

    g_return_val_if_fail (MOO_IS_PANE_VIEW (view), -1);
    g_return_val_if_fail (text != NULL, -1);

    line = moo_pane_view_start_line (view);
    g_return_val_if_fail (line >= 0, -1);

    moo_pane_view_write (view, text, len, tag);
    moo_pane_view_end_line (view);

    return line;
}


static void
check_if_scrolled (MooPaneView *view)
{
    GdkWindow *text_window;
    GdkRectangle rect;
    GtkTextIter iter;
    int line;

    if (!GTK_WIDGET_REALIZED (view))
    {
        view->priv->scrolled = FALSE;
        return;
    }

    text_window = gtk_text_view_get_window (GTK_TEXT_VIEW (view),
                                            GTK_TEXT_WINDOW_TEXT);

    gtk_text_view_get_visible_rect (GTK_TEXT_VIEW (view), &rect);
    gtk_text_view_get_line_at_y (GTK_TEXT_VIEW (view), &iter,
                                 rect.y + rect.height - 1, NULL);
    line = gtk_text_iter_get_line (&iter);

    if (line + 1 < gtk_text_buffer_get_line_count (get_buffer (view)))
        view->priv->scrolled = TRUE;
    else
        view->priv->scrolled = FALSE;
}


int
moo_pane_view_start_line (MooPaneView *view)
{
    GtkTextBuffer *buffer;
    GtkTextIter iter;

    g_return_val_if_fail (MOO_IS_PANE_VIEW (view), -1);
    g_return_val_if_fail (!view->priv->busy, -1);

    view->priv->busy = TRUE;

    buffer = get_buffer (view);
    gtk_text_buffer_get_end_iter (buffer, &iter);

    check_if_scrolled (view);

    if (!gtk_text_iter_starts_line (&iter))
        gtk_text_buffer_insert (buffer, &iter, "\n", 1);

    return gtk_text_iter_get_line (&iter);
}


void
moo_pane_view_write (MooPaneView    *view,
                     const char     *text,
                     gssize          len,
                     GtkTextTag     *tag)
{
    GtkTextBuffer *buffer;
    GtkTextIter iter;

    g_return_if_fail (MOO_IS_PANE_VIEW (view));
    g_return_if_fail (text != NULL);
    g_return_if_fail (view->priv->busy);

    buffer = get_buffer (view);
    gtk_text_buffer_get_end_iter (buffer, &iter);

    if (g_utf8_validate (text, len, NULL))
    {
        gtk_text_buffer_insert_with_tags (buffer, &iter, text, len, tag, NULL);
    }
    else
    {
        char *text_utf8 = g_locale_to_utf8 (text, len, NULL, NULL, NULL);

        if (text_utf8)
            gtk_text_buffer_insert_with_tags (buffer, &iter, text_utf8, -1, tag, NULL);
        else
            g_warning ("%s: could not convert '%s' to utf8",
                       G_STRLOC, text);

        g_free (text_utf8);
    }
}


static GtkTextMark*
get_end_mark (MooPaneView *view)
{
    if (!view->priv->end_mark)
    {
        GtkTextIter end;
        GtkTextBuffer *buffer = get_buffer (view);
        gtk_text_buffer_get_end_iter (buffer, &end);
        view->priv->end_mark =
                gtk_text_buffer_create_mark (buffer, NULL, &end, FALSE);
    }

    return view->priv->end_mark;
}


void
moo_pane_view_end_line (MooPaneView    *view)
{
    g_return_if_fail (MOO_IS_PANE_VIEW (view));
    g_return_if_fail (view->priv->busy);

    view->priv->busy = FALSE;

    if (!view->priv->scrolled)
        gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW (view),
                                            get_end_mark (view));
}
