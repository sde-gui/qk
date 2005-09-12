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
#include "mooutils/moosignal.h"
#include "mooutils/moocompat.h"
#include <gdk/gdkkeysyms.h>


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

static void      moo_pane_view_realize          (GtkWidget      *widget);
static gboolean  moo_pane_view_button_release   (GtkWidget      *widget,
                                                 GdkEventButton *event);

static void      moo_pane_view_move_cursor      (GtkTextView    *text_view,
                                                 GtkMovementStep step,
                                                 gint            count,
                                                 gboolean        extend_selection);
static void      moo_pane_view_populate_popup   (GtkTextView    *text_view,
                                                 GtkMenu        *menu);

static void      activate                       (MooPaneView    *view,
                                                 int             line);
static void      activate_current_line          (MooPaneView    *view);


static GtkTextBuffer *get_buffer                (MooPaneView    *view);
static GHashTable *get_hash_table               (MooPaneView    *view);


enum {
    ACTIVATE,
    ACTIVATE_CURRENT_LINE,
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
    GtkTextViewClass *textview_class = GTK_TEXT_VIEW_CLASS (klass);
    GtkBindingSet *binding_set;

//     gobject_class->set_property = moo_pane_view_set_property;
//     gobject_class->get_property = moo_pane_view_get_property;
    gobject_class->finalize = moo_pane_view_finalize;

    widget_class->realize = moo_pane_view_realize;
    widget_class->button_release_event = moo_pane_view_button_release;

    textview_class->move_cursor = moo_pane_view_move_cursor;
    textview_class->populate_popup = moo_pane_view_populate_popup;

    signals[ACTIVATE] =
            g_signal_new ("activate",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooPaneViewClass, activate),
                          NULL, NULL,
                          _moo_marshal_VOID__POINTER_INT,
                          G_TYPE_NONE, 2,
                          G_TYPE_POINTER, G_TYPE_INT);

    signals[ACTIVATE_CURRENT_LINE] =
            moo_signal_new_cb ("activate-current-line",
                               G_OBJECT_CLASS_TYPE (klass),
                               G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                               G_CALLBACK (activate_current_line),
                               NULL, NULL,
                               _moo_marshal_VOID__VOID,
                               G_TYPE_NONE, 0);

    binding_set = gtk_binding_set_by_class (klass);

    gtk_binding_entry_add_signal (binding_set, GDK_Return, 0,
                                  "activate-current-line", 0);
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
moo_pane_view_button_release (GtkWidget      *widget,
                              GdkEventButton *event)
{
    GtkTextView *textview = GTK_TEXT_VIEW (widget);
    MooPaneView *view = MOO_PANE_VIEW (widget);
    int buffer_x, buffer_y;
    GtkTextIter iter;
    gboolean result;

    result = GTK_WIDGET_CLASS(moo_pane_view_parent_class)->button_release_event (widget, event);

    if (gtk_text_view_get_window_type (textview, event->window) == GTK_TEXT_WINDOW_TEXT &&
        !moo_text_view_has_selection (MOO_TEXT_VIEW (widget)))
    {
        gtk_text_view_window_to_buffer_coords (textview,
                                               GTK_TEXT_WINDOW_TEXT,
                                               event->x, event->y,
                                               &buffer_x, &buffer_y);
        /* XXX */
        gtk_text_view_get_line_at_y (textview, &iter, buffer_y, NULL);
        activate (view, gtk_text_iter_get_line (&iter));
    }

    return result;
}


static void
activate (MooPaneView *view,
          int          line)
{
    g_signal_emit (view, signals[ACTIVATE], 0,
                   moo_pane_view_get_line_data (view, line),
                   line);
}


static int
get_current_line (MooPaneView *view)
{
    GtkTextIter iter;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, gtk_text_buffer_get_insert (buffer));
    return gtk_text_iter_get_line (&iter);
}


static void
activate_current_line (MooPaneView *view)
{
    activate (view, get_current_line (view));
}


static int
get_visible_height (GtkTextView *text_view)
{
    GdkRectangle rect;
    GtkTextIter iter;
    int start, end;

    gtk_text_view_get_visible_rect (text_view, &rect);
    gtk_text_view_get_line_at_y (text_view, &iter, rect.y, NULL);
    start = gtk_text_iter_get_line (&iter);
    gtk_text_view_get_line_at_y (text_view, &iter, rect.y + rect.height - 1, NULL);
    end = gtk_text_iter_get_line (&iter);

    return end - start + 1;
}


static void
moo_pane_view_move_cursor (GtkTextView    *text_view,
                           GtkMovementStep step,
                           gint            count,
                           gboolean        extend_selection)
{
    gboolean handle;
    MooPaneView *view;
    GtkTextBuffer *buffer;
    int current_line, new_line, height, total;
    GtkTextIter iter;

    switch (step)
    {
        case GTK_MOVEMENT_LOGICAL_POSITIONS:
        case GTK_MOVEMENT_VISUAL_POSITIONS:
        case GTK_MOVEMENT_WORDS:
        case GTK_MOVEMENT_PARAGRAPH_ENDS:
        case GTK_MOVEMENT_HORIZONTAL_PAGES:
            handle = FALSE;
            break;

        default:
            handle = TRUE;
    }

    if (extend_selection)
        handle = FALSE;

    if (!handle)
        return GTK_TEXT_VIEW_CLASS(moo_pane_view_parent_class)->move_cursor (text_view, step, count, extend_selection);

    view = MOO_PANE_VIEW (text_view);
    buffer = gtk_text_view_get_buffer (text_view);

    gtk_text_buffer_get_iter_at_mark (buffer, &iter, gtk_text_buffer_get_insert (buffer));
    current_line = get_current_line (view);

    height = get_visible_height (text_view);
    total = gtk_text_buffer_get_line_count (buffer);

    switch (step)
    {
        case GTK_MOVEMENT_DISPLAY_LINES:
        case GTK_MOVEMENT_PARAGRAPHS:
            new_line = current_line + count;
            break;

        case GTK_MOVEMENT_PAGES:
            new_line = current_line + count * (height - 1);
            break;

        case GTK_MOVEMENT_DISPLAY_LINE_ENDS:
        case GTK_MOVEMENT_BUFFER_ENDS:
            if (count < 0)
                new_line = 0;
            else
                new_line = total - 1;
            break;

        case GTK_MOVEMENT_LOGICAL_POSITIONS:
        case GTK_MOVEMENT_VISUAL_POSITIONS:
        case GTK_MOVEMENT_WORDS:
        case GTK_MOVEMENT_PARAGRAPH_ENDS:
        case GTK_MOVEMENT_HORIZONTAL_PAGES:
            g_return_if_reached ();
    }

    new_line = CLAMP (new_line, 0, total - 1);
    gtk_text_buffer_get_iter_at_line (buffer, &iter, new_line);
    gtk_text_buffer_place_cursor (buffer, &iter);
    gtk_text_view_scroll_to_mark (text_view,
                                  gtk_text_buffer_get_insert (buffer),
                                  0, FALSE, 0, 0);
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
    GtkTextIter iter;
    GtkTextBuffer *buffer = get_buffer (view);

    gtk_text_buffer_get_end_iter (buffer, &iter);
    gtk_text_iter_set_line_offset (&iter, 0);

    if (!view->priv->end_mark)
        view->priv->end_mark =
                gtk_text_buffer_create_mark (buffer, NULL, &iter, FALSE);
    else
        gtk_text_buffer_move_mark (buffer, view->priv->end_mark, &iter);

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


static void
copy_clipboard (GtkTextView *text_view)
{
    g_signal_emit_by_name (text_view, "copy-clipboard");
}


static void
moo_pane_view_populate_popup (GtkTextView *text_view,
                              GtkMenu     *menu)
{
    GtkWidget *item;
    gboolean has_selection, has_text;

    gtk_container_foreach (GTK_CONTAINER (menu),
                           (GtkCallback) gtk_widget_destroy,
                           NULL);

    item = gtk_image_menu_item_new_from_stock (GTK_STOCK_COPY, NULL);
    g_signal_connect_swapped (item, "activate",
                              G_CALLBACK (copy_clipboard), text_view);
    has_selection = moo_text_view_has_selection (MOO_TEXT_VIEW (text_view));
    gtk_widget_set_sensitive (item, has_selection);
    gtk_widget_show (item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

    item = gtk_image_menu_item_new_from_stock (GTK_STOCK_SELECT_ALL, NULL);
    g_signal_connect_swapped (item, "activate",
                              G_CALLBACK (moo_text_view_select_all),
                              text_view);
    has_text = moo_text_view_has_text (MOO_TEXT_VIEW (text_view));
    gtk_widget_set_sensitive (item, has_text);
    gtk_widget_show (item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
}
