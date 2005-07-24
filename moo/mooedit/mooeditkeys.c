/*
 *   mooedit/mooeditkeys.c
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

#define MOOEDIT_COMPILATION

#include "mooedit/mooedit-private.h"
#include "mooedit/mootextiter.h"
#include "mooutils/moocompat.h"
#include <gdk/gdkkeysyms.h>
#include <string.h>


inline G_GNUC_CONST static GtkWidgetClass *parent_class (void)
{
    static gpointer sourceview_class = NULL;
    if (!sourceview_class)
        sourceview_class = GTK_WIDGET_CLASS (gtk_type_class (GTK_TYPE_SOURCE_VIEW));
    return GTK_WIDGET_CLASS (sourceview_class);
}


/* these two functions are taken from gtk/gtktextview.c */
static void text_view_obscure_mouse_cursor (GtkTextView *text_view);
static void set_invisible_cursor (GdkWindow *window);

static gboolean handle_tab          (MooEdit        *edit,
                                     GdkEventKey    *event);
static gboolean handle_backspace    (MooEdit        *edit,
                                     GdkEventKey    *event);
static gboolean handle_enter        (MooEdit        *edit,
                                     GdkEventKey    *event);
static gboolean handle_shift_tab    (MooEdit        *edit,
                                     GdkEventKey    *event);
static gboolean handle_ctrl_up      (MooEdit        *edit,
                                     GdkEventKey    *event,
                                     gboolean        up);
static gboolean handle_ctrl_pgup    (MooEdit        *edit,
                                     GdkEventKey    *event,
                                     gboolean        up);

static void     indent_line         (MooEdit        *edit,
                                     GtkTextIter    *iter);
static void     unindent_line       (MooEdit        *edit,
                                     GtkTextIter    *iter);

inline static void get_iter_at_selection_bound  (GtkTextBuffer  *buffer,
                                                 GtkTextIter    *iter)
{
    gtk_text_buffer_get_iter_at_mark (buffer, iter,
                                      gtk_text_buffer_get_selection_bound (buffer));
}

inline static void get_iter_at_insert           (GtkTextBuffer  *buffer,
                                                 GtkTextIter    *iter)
{
    gtk_text_buffer_get_iter_at_mark (buffer, iter,
                                      gtk_text_buffer_get_insert (buffer));
}

inline static void move_selection_bound         (GtkTextBuffer  *buffer,
                                                 GtkTextIter    *iter)
{
    gtk_text_buffer_move_mark (buffer,
                               gtk_text_buffer_get_selection_bound (buffer),
                               iter);
}

inline static void move_insert                  (GtkTextBuffer  *buffer,
                                                 GtkTextIter    *iter)
{
    gtk_text_buffer_move_mark (buffer,
                               gtk_text_buffer_get_insert (buffer),
                               iter);
}


int         _moo_edit_key_press_event       (GtkWidget          *widget,
                                             GdkEventKey        *event)
{
    MooEdit *edit;
    GtkTextView *text_view;
    GtkTextBuffer *buffer;
    gboolean obscure = TRUE;
    gboolean handled = FALSE;
    int keyval = event->keyval;
    GdkModifierType mods =
            event->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_MOD1_MASK);

    edit = MOO_EDIT (widget);
    text_view = GTK_TEXT_VIEW (widget);
    buffer = edit->priv->text_buffer;

    if (!mods)
    {
        switch (keyval)
        {
            case GDK_Tab:
            case GDK_KP_Tab:
                handled = handle_tab (edit, event);
                break;
            case GDK_BackSpace:
                handled = handle_backspace (edit, event);
                break;
            case GDK_KP_Enter:
            case GDK_Return:
                handled = handle_enter (edit, event);
                break;
        }
    }
    else if (mods == GDK_SHIFT_MASK)
    {
        switch (keyval)
        {
            /* TODO TODO stupid X and gtk !!! */
            case GDK_ISO_Left_Tab:
            case GDK_KP_Tab:
                handled = handle_shift_tab (edit, event);
                break;
        }
    }
    else if (mods == GDK_CONTROL_MASK)
    {
        switch (keyval)
        {
            case GDK_Up:
            case GDK_KP_Up:
                handled = handle_ctrl_up (edit, event, TRUE);
                /* if we scroll, let mouse cursor stay */
                obscure = FALSE;
                break;
            case GDK_Down:
            case GDK_KP_Down:
                handled = handle_ctrl_up (edit, event, FALSE);
                obscure = FALSE;
                break;
            case GDK_Page_Up:
            case GDK_KP_Page_Up:
                handled = handle_ctrl_pgup (edit, event, TRUE);
                obscure = FALSE;
                break;
            case GDK_Page_Down:
            case GDK_KP_Page_Down:
                handled = handle_ctrl_pgup (edit, event, FALSE);
                obscure = FALSE;
                break;
        }
    }
    else
    {
        obscure = FALSE;
    }

    if (obscure && handled)
        text_view_obscure_mouse_cursor (text_view);

    return handled ||
            parent_class()->key_press_event (widget, event);
}


static void text_view_obscure_mouse_cursor (GtkTextView *text_view)
{
    if (!text_view->mouse_cursor_obscured)
    {
        GdkWindow *window =
                gtk_text_view_get_window (text_view,
                                          GTK_TEXT_WINDOW_TEXT);
        set_invisible_cursor (window);
        text_view->mouse_cursor_obscured = TRUE;
    }
}


static void set_invisible_cursor (GdkWindow *window)
{
    GdkBitmap *empty_bitmap;
    GdkCursor *cursor;
    GdkColor useless;
    char invisible_cursor_bits[] = { 0x0 };

    useless.red = useless.green = useless.blue = 0;
    useless.pixel = 0;

    empty_bitmap =
            gdk_bitmap_create_from_data (window,
                                         invisible_cursor_bits,
                                         1, 1);

    cursor = gdk_cursor_new_from_pixmap (empty_bitmap,
                                         empty_bitmap,
                                         &useless,
                                         &useless, 0, 0);

    gdk_window_set_cursor (window, cursor);

    gdk_cursor_unref (cursor);
    g_object_unref (empty_bitmap);
}


static gboolean handle_tab          (MooEdit        *edit,
                                     G_GNUC_UNUSED GdkEventKey    *event)
{
    GtkTextBuffer *buffer = edit->priv->text_buffer;
    GtkTextIter insert, bound, start, end;
    gboolean starts_line, insert_last;
    int first_line, last_line, i;

    if (!edit->priv->tab_indents ||
         !gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
        return FALSE;

    get_iter_at_insert (buffer, &insert);
    get_iter_at_selection_bound (buffer, &bound);

    insert_last = (gtk_text_iter_compare (&insert, &bound) > 0);
    starts_line = gtk_text_iter_starts_line (&start);

    first_line = gtk_text_iter_get_line (&start);
    last_line = gtk_text_iter_get_line (&end);
    if (gtk_text_iter_starts_line (&end))
        last_line--;

    gtk_text_buffer_begin_user_action (buffer);

    for (i = first_line; i <= last_line; ++i)
    {
        indent_line (edit, &start);
        gtk_text_iter_forward_line (&start);
    }

    if (starts_line)
    {
        if (insert_last)
        {
            get_iter_at_selection_bound (buffer, &insert);
            gtk_text_iter_set_line_offset (&insert, 0);
            move_selection_bound (buffer, &insert);
        }
        else
        {
            get_iter_at_insert (buffer, &insert);
            gtk_text_iter_set_line_offset (&insert, 0);
            move_insert (buffer, &insert);
        }
    }

    gtk_text_buffer_end_user_action (buffer);

    return TRUE;
}


static gboolean handle_shift_tab    (MooEdit        *edit,
                                     G_GNUC_UNUSED GdkEventKey    *event)
{
    GtkTextBuffer *buffer = edit->priv->text_buffer;
    GtkTextIter start, end;
    int first_line, last_line, i;

    if (!edit->priv->shift_tab_unindents)
        return FALSE;

    gtk_text_buffer_get_selection_bounds (buffer, &start, &end);

    first_line = gtk_text_iter_get_line (&start);
    last_line = gtk_text_iter_get_line (&end);
    if (gtk_text_iter_starts_line (&end) && first_line != last_line)
        last_line--;

    gtk_text_buffer_begin_user_action (buffer);

    for (i = first_line; i <= last_line; ++i)
    {
        unindent_line (edit, &start);
        gtk_text_iter_forward_line (&start);
    }

    gtk_text_buffer_end_user_action (buffer);

    return TRUE;
}


static void     indent_line         (MooEdit        *edit,
                                     GtkTextIter    *iter)
{
    gboolean use_spaces;
    char *text;
    guint len;

    use_spaces = gtk_source_view_get_insert_spaces_instead_of_tabs (GTK_SOURCE_VIEW (edit));

    if (use_spaces)
    {
        len = gtk_source_view_get_tabs_width (GTK_SOURCE_VIEW (edit));
        text = g_strnfill (len, ' ');
    }
    else
    {
        len = 1;
        text = g_strdup ("\t");
    }

    gtk_text_iter_set_line_offset (iter, 0);
    gtk_text_buffer_insert (edit->priv->text_buffer, iter,
                            text, len);
    g_free (text);
}


static void     unindent_line       (MooEdit        *edit,
                                     GtkTextIter    *iter)
{
    GtkTextBuffer *buffer;
    GtkTextIter end;
    int deleted;
    guint tab_width;
    gunichar c;

    buffer = edit->priv->text_buffer;
    tab_width = gtk_source_view_get_tabs_width (GTK_SOURCE_VIEW (edit));

    gtk_text_iter_set_line_offset (iter, 0);
    end = *iter;
    deleted = 0;

    while (TRUE)
    {
        if (gtk_text_iter_ends_line (&end))
            break;

        c = gtk_text_iter_get_char (&end);
        if (c == ' ')
        {
            gtk_text_iter_forward_char (&end);
            deleted += 1;
        }
        else if (c == '\t')
        {
            gtk_text_iter_forward_char (&end);
            deleted += tab_width;
        }
        else
        {
            break;
        }

        if (deleted >= (int) tab_width)
            break;
    }

    gtk_text_buffer_delete (buffer, iter, &end);

    deleted -= tab_width;
    if (deleted > 0)
    {
        char *text = g_strnfill (deleted, ' ');
        gtk_text_buffer_insert (buffer, iter, text, deleted);
        g_free (text);
    }
}


static guint get_text_width_offset (const GtkTextIter *iter,
                                    guint              tab_width)
{
    GtkTextIter i;
    guint offset;

    if (gtk_text_iter_starts_line (iter))
        return 0;

    i = *iter;
    gtk_text_iter_set_line_offset (&i, 0);
    offset = 0;

    while (gtk_text_iter_compare (&i, iter))
    {
        gunichar c = gtk_text_iter_get_char (&i);

        if (c == '\t')
        {
            guint add = offset - (offset / tab_width) * tab_width;
            if (!add)
                add = tab_width;
            offset += add;
        }
        else if (g_unichar_iswide (c))
        {
            offset += 2;
        }
        else
        {
            offset += (g_unichar_iswide (c) ? 2 : 1);
        }

        gtk_text_iter_forward_char (&i);
    }

    return offset;
}


/* computes offset of start and returns offset or -1 if there are
   non-whitespace characters before start */
static int compute_offset (const GtkTextIter *start,
                           guint              tab_width)
{
    GtkTextIter iter;
    guint offset;

    if (gtk_text_iter_starts_line (start))
        return 0;

    iter = *start;
    gtk_text_iter_set_line_offset (&iter, 0);
    offset = 0;

    while (gtk_text_iter_compare (&iter, start))
    {
        gunichar c = gtk_text_iter_get_char (&iter);

        if (c == ' ')
        {
            offset += 1;
        }
        else if (c == '\t')
        {
            guint add = offset - (offset / tab_width) * tab_width;
            if (!add)
                add = tab_width;
            offset += add;
        }
        else
        {
            return -1;
        }

        gtk_text_iter_forward_char (&iter);
    }

    return offset;
}


/* computes amount of leading white space on the line containing
   start; returns TRUE if line contains some non-whitespace chars */
static gboolean compute_line_indent (const GtkTextIter *line,
                                     guint              tab_width,
                                     guint             *indent)
{
    GtkTextIter iter;

    *indent = 0;

    iter = *line;
    gtk_text_iter_set_line_offset (&iter, 0);
    if (gtk_text_iter_ends_line (&iter))
        return FALSE;

    while (TRUE)
    {
        gunichar c = gtk_text_iter_get_char (&iter);

        if (c == ' ')
        {
            *indent += 1;
        }
        else if (c == '\t')
        {
            guint add = *indent - (*indent / tab_width) * tab_width;
            if (!add)
                add = tab_width;
            *indent += add;
        }
        else
        {
            return TRUE;
        }

        gtk_text_iter_forward_char (&iter);

        if (gtk_text_iter_ends_line (&iter))
            return FALSE;
    }
}


/* computes where cursor should jump when backspace is pressed

<-- result -->
              blah blah blah
                      blah
                     | offset
*/
static guint compute_next_stop (const GtkTextIter *start,
                                guint              tab_width,
                                guint              offset,
                                gboolean           same_line)
{
    GtkTextIter iter;
    guint indent;

    iter = *start;

    gtk_text_iter_set_line_offset (&iter, 0);
    if (!same_line)
    {
        if (gtk_text_iter_is_start (&iter))
            return 0;
        gtk_text_iter_backward_line (&iter);
    }

    while (TRUE)
    {
        if (compute_line_indent (&iter, tab_width, &indent) &&
            indent && indent <= offset)
                return indent;
        if (!gtk_text_iter_backward_line (&iter))
            return 0;
    }
}


static char *compute_indent_string (guint       offset,
                                    guint       tab_width,
                                    gboolean    use_spaces)
{
    if (!offset)
        return NULL;
    else if (use_spaces)
        return g_strnfill (offset, ' ');
    else
    {
        guint num_tabs, num_spaces;
        char *string;

        num_tabs = offset / tab_width;
        num_spaces = offset - num_tabs * tab_width;
        string = g_new (char, num_tabs + num_spaces + 1);
        if (num_tabs)
            memset (string, '\t', num_tabs);
        if (num_spaces)
            memset (string + num_tabs, ' ', num_spaces);
        string[num_tabs + num_spaces] = 0;

        return string;
    }
}


static gboolean handle_backspace    (G_GNUC_UNUSED MooEdit        *edit,
                                     G_GNUC_UNUSED GdkEventKey    *event)
{
    GtkTextBuffer *buffer;
    GtkTextIter start, end;
    int offset;
    guint new_offset;
    guint tab_width;
    char *insert = NULL;

    buffer = edit->priv->text_buffer;
    get_iter_at_insert (buffer, &end);

    if (!edit->priv->backspace_indents)
        return FALSE;
    if (gtk_text_buffer_get_selection_bounds (buffer, NULL, NULL))
        return FALSE;
    if (gtk_text_iter_starts_line (&end))
        return FALSE;

    tab_width = gtk_source_view_get_tabs_width (GTK_SOURCE_VIEW (edit));

    offset = compute_offset (&end, tab_width);

    if (offset < 0)
        return FALSE;

    if (!offset)
        new_offset = 0;
    else
        new_offset = compute_next_stop (&end, tab_width, offset - 1, FALSE);

    gtk_text_buffer_begin_user_action (buffer);

    start = end;
    gtk_text_iter_set_line_offset (&start, 0);
    gtk_text_buffer_delete (buffer, &start, &end);
    insert = compute_indent_string (new_offset, tab_width,
        gtk_source_view_get_insert_spaces_instead_of_tabs (GTK_SOURCE_VIEW (edit)));
    if (insert)
    {
        gtk_text_buffer_insert (buffer, &start, insert, -1);
        g_free (insert);
    }

    gtk_text_buffer_end_user_action (buffer);

    return TRUE;
}


static gboolean handle_enter        (MooEdit        *edit,
                                     G_GNUC_UNUSED GdkEventKey    *event)
{
    GtkTextBuffer *buffer;
    GtkTextIter start, end;
    gboolean has_selection;
    guint indent, tab_width;
    char *indent_string;

    if (!edit->priv->auto_indent)
        return FALSE;

    buffer = edit->priv->text_buffer;

    has_selection =
            gtk_text_buffer_get_selection_bounds (buffer, &start, &end);

    tab_width = gtk_source_view_get_tabs_width (GTK_SOURCE_VIEW (edit));
    indent = compute_next_stop (&start, tab_width,
                                get_text_width_offset (&start, tab_width),
                                TRUE);
    indent_string = compute_indent_string (indent, tab_width,
        gtk_source_view_get_insert_spaces_instead_of_tabs (GTK_SOURCE_VIEW (edit)));

    gtk_text_buffer_begin_user_action (buffer);

    if (has_selection)
        gtk_text_buffer_delete (buffer, &start, &end);

    gtk_text_buffer_insert (buffer, &start, "\n", 1);

    if (indent_string)
    {
        gtk_text_buffer_insert (buffer, &start, indent_string, -1);
        g_free (indent_string);
    }

    gtk_text_buffer_end_user_action (buffer);

    gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW (edit),
                                        gtk_text_buffer_get_insert (buffer));
    return TRUE;
}


static gboolean handle_ctrl_up      (MooEdit        *edit,
                                     G_GNUC_UNUSED GdkEventKey    *event,
                                     gboolean        up)
{
    GtkTextView *text_view;
    GtkAdjustment *adjustment;
    double value;

    if (!edit->priv->ctrl_up_down_scrolls)
        return FALSE;

    text_view = GTK_TEXT_VIEW (edit);
    adjustment = text_view->vadjustment;

    if (!adjustment)
        return FALSE;

    if (up)
    {
        value = adjustment->value - adjustment->step_increment;
        if (value < adjustment->lower)
            value = adjustment->lower;
        gtk_adjustment_set_value (adjustment, value);
    }
    else
    {
        value = adjustment->value + adjustment->step_increment;
        if (value > adjustment->upper - adjustment->page_size)
            value = adjustment->upper - adjustment->page_size;
        gtk_adjustment_set_value (adjustment, value);
    }

    return TRUE;
}


static gboolean handle_ctrl_pgup    (MooEdit        *edit,
                                     G_GNUC_UNUSED GdkEventKey    *event,
                                     gboolean        up)
{
    GtkTextView *text_view;
    GtkAdjustment *adjustment;
    double value;

    if (!edit->priv->ctrl_page_up_down_scrolls)
        return FALSE;

    text_view = GTK_TEXT_VIEW (edit);
    adjustment = text_view->vadjustment;

    if (!adjustment)
        return FALSE;

    if (up)
    {
        value = adjustment->value - adjustment->page_increment;
        if (value < adjustment->lower)
            value = adjustment->lower;
        gtk_adjustment_set_value (adjustment, value);
    }
    else
    {
        value = adjustment->value + adjustment->page_increment;
        if (value > adjustment->upper - adjustment->page_size)
            value = adjustment->upper - adjustment->page_size;
        gtk_adjustment_set_value (adjustment, value);
    }

    return TRUE;
}
