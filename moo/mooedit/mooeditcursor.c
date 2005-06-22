/*
 *   mooedit/mooeditcursor.c
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


inline static GtkWidgetClass *parent_class (void)
{
    static gpointer sourceview_class = NULL;
    if (!sourceview_class)
        sourceview_class = GTK_WIDGET_CLASS (gtk_type_class (GTK_TYPE_SOURCE_VIEW));
    return GTK_WIDGET_CLASS (sourceview_class);
}


inline static gboolean is_word_char (const GtkTextIter *iter)
{
    gunichar c = gtk_text_iter_get_char (iter);
    g_return_val_if_fail (c != 0, FALSE);
    return g_unichar_isalnum (c) || c == '_';
}

/* Glib docs say: "(Note: don't use this to do word breaking; you have
 * to use Pango or equivalent to get word breaking right, the algorithm
 * is fairly complex.)"
 */
inline static gboolean is_space (const GtkTextIter *iter)
{
    gunichar c = gtk_text_iter_get_char (iter);
    g_return_val_if_fail (c != 0, FALSE);
    return g_unichar_isspace (c);
}

inline static gboolean is_word_start (const GtkTextIter *iter)
{
    GtkTextIter i;
    if (!is_word_char (iter)) return FALSE;
    i = *iter;
    if (!gtk_text_iter_backward_char (&i)) return TRUE;
    return !is_word_char (&i);
}


static gboolean text_iter_forward_word_start (GtkTextIter *iter)
{
    gboolean moved = FALSE;
    if (gtk_text_iter_is_end (iter)) return FALSE;

    /* if iter points to word char, then go to the first non-space char after the word
     * otherwise, go to the next word char
     * stop at end of line
     */

    if (is_word_char (iter)) {
        while (!gtk_text_iter_is_end (iter) && is_word_char (iter))
        {
            gtk_text_iter_forward_char (iter);
            moved = TRUE;
        }
        if (gtk_text_iter_is_end (iter)) return FALSE;
        while (!gtk_text_iter_is_end (iter) &&
               is_space (iter) &&
               !gtk_text_iter_ends_line (iter))
        {
            gtk_text_iter_forward_char (iter);
            moved = TRUE;
        }
    }
    else {
        if (gtk_text_iter_ends_line (iter)) {
            gtk_text_iter_forward_char (iter);
            moved = TRUE;
        }
        else {
            while (!gtk_text_iter_is_end (iter) &&
                   !is_word_char (iter) &&
                   !gtk_text_iter_ends_line (iter))
            {
                gtk_text_iter_forward_char (iter);
                moved = TRUE;
            }
        }
    }
    return moved && !gtk_text_iter_is_end (iter);
}

inline static gboolean text_iter_forward_word_start_n (GtkTextIter *iter, guint count)
{
    if (!count) return FALSE;
    while (count) {
        if (!text_iter_forward_word_start (iter)) {
            gtk_text_iter_forward_to_end (iter);
            return FALSE;
        }
        else
            --count;
    }
    return TRUE;
}


inline static gboolean text_iter_backward_word_start (GtkTextIter *iter)
{
    gboolean moved = FALSE;
    if (gtk_text_iter_starts_line (iter)) {
        moved = gtk_text_iter_backward_char (iter);
        /* it may point now to \n in \r\n combination */
        if (moved && !gtk_text_iter_ends_line (iter))
            gtk_text_iter_backward_char (iter);
    }
    else {
        while (gtk_text_iter_backward_char (iter) &&
               !is_word_start (iter) &&
               !gtk_text_iter_starts_line (iter))
            moved = TRUE;
    }
    return moved;
}

inline static gboolean text_iter_backward_word_start_n (GtkTextIter *iter, guint count)
{
    gboolean moved = FALSE;
    while (count && text_iter_backward_word_start (iter)) {
        moved = TRUE;
        --count;
    }
    return moved;
}


/* TODO: may I do this? */
inline static void text_view_reset_im_context (GtkTextView *text_view)
{
  if (text_view->need_im_reset)
    {
      text_view->need_im_reset = FALSE;
      gtk_im_context_reset (text_view->im_context);
    }
}


void    _moo_edit_move_cursor           (GtkTextView        *text_view,
                                         GtkMovementStep     step,
                                         gint                count,
                                         gboolean            extend_selection)
{
    GtkTextBuffer *buffer;
    GtkTextIter insert;
    GtkTextIter newplace;

    g_return_if_fail (text_view != NULL);
    if (!text_view->cursor_visible || step != GTK_MOVEMENT_WORDS)
        return GTK_TEXT_VIEW_CLASS (parent_class())->move_cursor (
            text_view, step, count, extend_selection);

    buffer = gtk_text_view_get_buffer (text_view);

    text_view_reset_im_context (text_view);

    gtk_text_buffer_get_iter_at_mark (buffer, &insert,
                                      gtk_text_buffer_get_mark (buffer,
                                                                "insert"));
    newplace = insert;

    if (count < 0)
        text_iter_backward_word_start_n (&newplace, -count);
    else if (count > 0)
        text_iter_forward_word_start_n (&newplace, count);

    /* call move_cursor() even if the cursor hasn't moved, since it
        cancels the selection
    */
    /* move_cursor (buf, &newplace, extend_selection); */
    if (extend_selection)
        gtk_text_buffer_move_mark (buffer, gtk_text_buffer_get_insert (buffer),
                                   &newplace);
    else
        gtk_text_buffer_place_cursor (buffer, &newplace);

    if (!gtk_text_iter_equal (&insert, &newplace))
    {
        gtk_text_view_scroll_mark_onscreen (text_view,
                                            gtk_text_buffer_get_insert (buffer));
    }
}


void    _moo_edit_delete_from_cursor    (GtkTextView        *text_view,
                                         GtkDeleteType       type,
                                         gint                count)
{
    GtkTextBuffer *buf;
    GtkTextMark *insert_mark;
    GtkTextIter insert, start, end;

    if (type != GTK_DELETE_WORD_ENDS)
        return GTK_TEXT_VIEW_CLASS (parent_class())->delete_from_cursor (text_view, type, count);

    text_view_reset_im_context (text_view);

    buf = gtk_text_view_get_buffer (text_view);
    insert_mark = gtk_text_buffer_get_insert (buf);
    gtk_text_buffer_get_iter_at_mark (buf, &insert, insert_mark);

    start = insert;
    end = insert;

    if (count > 0)
        text_iter_forward_word_start_n (&end, count);
    else if (count < 0)
        text_iter_backward_word_start_n (&start, -count);

    if (!gtk_text_iter_equal (&start, &end))
    {
        gtk_text_buffer_begin_user_action (buf);
        gtk_text_buffer_delete_interactive (buf, &start, &end, text_view->editable);
        gtk_text_buffer_end_user_action (buf);
        gtk_text_view_scroll_mark_onscreen (text_view, insert_mark);
    }
}


/****************************************************************************/
/* Mouse
 */

inline static gboolean extend_selection (MooEdit                *edit,
                                         MooEditSelectionType    type,
                                         GtkTextIter            *insert,
                                         GtkTextIter            *selection_bound)
{
    return MOO_EDIT_GET_CLASS(edit)->extend_selection (edit, type, insert, selection_bound);
}

inline static void clear_drag_stuff (MooEdit *edit)
{
    edit->priv->drag_moved = FALSE;
    edit->priv->drag_type = MOO_EDIT_DRAG_NONE;
    edit->priv->drag_start_x = -1;
    edit->priv->drag_start_y = -1;
    edit->priv->drag_button = GDK_BUTTON_RELEASE;
}

#define get_buf(edit) \
    GTK_SOURCE_BUFFER (gtk_text_view_get_buffer (GTK_TEXT_VIEW (edit)))

/* from gtktextview.c */
inline static void text_view_unobscure_mouse_cursor (GtkTextView *text_view)
{
    if (text_view->mouse_cursor_obscured)
    {
        GdkCursor *cursor =
            gdk_cursor_new_for_display (gtk_widget_get_display (GTK_WIDGET (text_view)),
                                        GDK_XTERM);
        gdk_window_set_cursor (gtk_text_view_get_window (text_view, GTK_TEXT_WINDOW_TEXT), cursor);
        gdk_cursor_unref (cursor);
        text_view->mouse_cursor_obscured = FALSE;
    }
}

static void start_drag_scroll (MooEdit *edit);
static void stop_drag_scroll (MooEdit *edit);
static gboolean drag_scroll_timeout_func (MooEdit *edit);
/* from gtktextview.c */
static void text_view_start_selection_dnd (GtkTextView       *text_view,
                                           const GtkTextIter *iter,
                                           GdkEventMotion    *event);
static const int SCROLL_TIMEOUT = 100;


int     _moo_edit_button_press_event    (GtkWidget          *widget,
                                         GdkEventButton     *event)
{
    GtkTextView *text_view;
    MooEdit *edit;
    GtkTextBuffer *buffer;
    int x, y;
    GtkTextIter iter;

    text_view = GTK_TEXT_VIEW (widget);
    text_view_unobscure_mouse_cursor (text_view);

    if (gtk_text_view_get_window_type (text_view, event->window) != GTK_TEXT_WINDOW_TEXT)
    {
        return FALSE;
    }

    edit = MOO_EDIT (widget);
    buffer = gtk_text_view_get_buffer (text_view);
    gtk_widget_grab_focus (widget);

    text_view_reset_im_context (text_view);

    gtk_text_view_window_to_buffer_coords (text_view, GTK_TEXT_WINDOW_TEXT,
                                           (int)event->x, (int)event->y, &x, &y);

    gtk_text_view_get_iter_at_location (text_view, &iter, x, y);

    if (event->type == GDK_BUTTON_PRESS)
    {
        if (event->button == 1) {
            edit->priv->drag_button = GDK_BUTTON_PRESS;
            edit->priv->drag_start_x = x;
            edit->priv->drag_start_y = y;

            /* if clicked in selected, start drag */
            GtkTextIter sel_start, sel_end;
            if (gtk_text_buffer_get_selection_bounds (buffer, &sel_start, &sel_end))
            {
                gtk_text_iter_order (&sel_start, &sel_end);
                if (gtk_text_iter_in_range (&iter, &sel_start, &sel_end)) {
                    /* clicked inside of selection,
                     * set up drag and return */
                    edit->priv->drag_type = MOO_EDIT_DRAG_DRAG;
                    return TRUE;
                }
            }

            /* otherwise, clear selection, and position cursor at clicked point */
            if (event->state & GDK_SHIFT_MASK)
                gtk_text_buffer_move_mark (buffer,
                                           gtk_text_buffer_get_insert (buffer),
                                           &iter);
            else
                gtk_text_buffer_place_cursor (buffer, &iter);
            edit->priv->drag_type = MOO_EDIT_DRAG_SELECT;
        }
        else if (event->button == 2 || event->button == 3) {
            /* let GtkSourceView worry about this */
            return parent_class()->button_press_event (widget, event);
        }
        else {
            g_warning ("got button %d in button_press callback", event->button);
        }
    }
    else if (event->type == GDK_2BUTTON_PRESS && event->button == 1)
    {
        if (gtk_text_buffer_get_selection_bounds (buffer, NULL, NULL)) {
            /* it may happen sometimes, if you click fast enough */
            gtk_text_buffer_place_cursor (buffer, &iter);
        }
        edit->priv->drag_button = GDK_2BUTTON_PRESS;
        edit->priv->drag_start_x = x;
        edit->priv->drag_start_y = y;
        edit->priv->drag_type = MOO_EDIT_DRAG_SELECT;
        GtkTextIter bound = iter;
        if (extend_selection (edit, MOO_EDIT_SELECT_WORDS, &iter, &bound))
            gtk_text_buffer_select_range (buffer, &iter, &bound);
    }
    else if (event->type == GDK_3BUTTON_PRESS && event->button == 1)
    {
        edit->priv->drag_button = GDK_3BUTTON_PRESS;
        edit->priv->drag_start_x = x;
        edit->priv->drag_start_y = y;
        edit->priv->drag_type = MOO_EDIT_DRAG_SELECT;
        GtkTextIter bound = iter;
        if (extend_selection (edit, MOO_EDIT_SELECT_LINES, &iter, &bound))
            gtk_text_buffer_select_range (buffer, &iter, &bound);
    }
    return TRUE;
}


int     _moo_edit_button_release_event  (GtkWidget          *widget,
                                         G_GNUC_UNUSED GdkEventButton *event)
{
    GtkTextView *text_view = GTK_TEXT_VIEW (widget);
    MooEdit *edit = MOO_EDIT (widget);

    switch (edit->priv->drag_type) {
        case MOO_EDIT_DRAG_NONE:
            /* it may happen after right-click, or clicking outside
             * of widget or something like that
             * everything has been taken care of, so do nothing */
            break;

        case MOO_EDIT_DRAG_SELECT:
            /* everything should be done already in button_press and
             * motion_notify handlers */
            stop_drag_scroll (edit);
            break;

        case MOO_EDIT_DRAG_DRAG:
            /* if we were really dragging, drop it
             * otherwise, it was just a single click in selected text */
            g_assert (!edit->priv->drag_moved); /* parent should handle drag */
            GtkTextIter iter;
            gtk_text_view_get_iter_at_location (text_view, &iter,
                                                edit->priv->drag_start_x,
                                                edit->priv->drag_start_y);
            gtk_text_buffer_place_cursor (gtk_text_view_get_buffer (text_view),
                                          &iter);
            break;

        default:
            g_assert_not_reached ();
    }

    clear_drag_stuff (edit);
    return TRUE;
}


#define outside(x,y,rect)               \
    ((x) < (rect).x ||                  \
     (y) < (rect).y ||                  \
     (x) >= (rect).x + (rect).width ||  \
     (y) >= (rect).y + (rect).height)

int     _moo_edit_motion_event          (GtkWidget          *widget,
                                         GdkEventMotion     *event)
{
    GtkTextView *text_view = GTK_TEXT_VIEW (widget);
    MooEdit *edit = MOO_EDIT (widget);
    int x, y, event_x, event_y;
    GtkTextIter iter;

    text_view_unobscure_mouse_cursor (text_view);

    if (!edit->priv->drag_type) return FALSE;

    if (event->is_hint)
        gdk_window_get_pointer (event->window, &event_x, &event_y, NULL);
    else {
        event_x = (int)event->x;
        event_y = (int)event->y;
    }
    gtk_text_view_window_to_buffer_coords (text_view,
                                           gtk_text_view_get_window_type (text_view, event->window),
                                           event_x, event_y, &x, &y);
    gtk_text_view_get_iter_at_location (text_view, &iter, x, y);

    if (edit->priv->drag_type == MOO_EDIT_DRAG_SELECT) {
        GdkRectangle rect;
        GtkTextIter start;
        MooEditSelectionType t;
        GtkTextBuffer *buffer;

        edit->priv->drag_moved = TRUE;
        gtk_text_view_get_visible_rect (text_view, &rect);

        if (outside (x, y, rect)) {
            start_drag_scroll (edit);
            return TRUE;
        }
        else
            stop_drag_scroll (edit);

        buffer = gtk_text_view_get_buffer (text_view);

        gtk_text_view_get_iter_at_location (text_view, &start,
                                            edit->priv->drag_start_x,
                                            edit->priv->drag_start_y);

        if (edit->priv->drag_button == GDK_BUTTON_PRESS)
            t = MOO_EDIT_SELECT_CHARS;
        else if (edit->priv->drag_button == GDK_2BUTTON_PRESS)
            t = MOO_EDIT_SELECT_WORDS;
        else
            t = MOO_EDIT_SELECT_LINES;

        if (extend_selection (edit, t, &start, &iter))
            gtk_text_buffer_select_range (buffer, &iter, &start);
        else
            gtk_text_buffer_place_cursor (buffer, &iter);
    }
    else {
        /* this piece is from gtktextview.c */
        int x, y;
        gdk_window_get_pointer (gtk_text_view_get_window (text_view, GTK_TEXT_WINDOW_TEXT),
                                &x, &y, NULL);

        if (gtk_drag_check_threshold (widget,
                                      edit->priv->drag_start_x,
                                      edit->priv->drag_start_y,
                                      x, y))
        {
            GtkTextIter iter;
            int buffer_x, buffer_y;

            gtk_text_view_window_to_buffer_coords (text_view,
                                                   GTK_TEXT_WINDOW_TEXT,
                                                   edit->priv->drag_start_x,
                                                   edit->priv->drag_start_y,
                                                   &buffer_x,
                                                   &buffer_y);

            gtk_text_view_get_iter_at_location (text_view, &iter, buffer_x, buffer_y);

            edit->priv->drag_type = MOO_EDIT_DRAG_NONE;
            text_view_start_selection_dnd (text_view, &iter, event);
        }
    }

    return TRUE;
}


static const GtkTargetEntry gtk_text_view_target_table[] = {
    { (char*)"GTK_TEXT_BUFFER_CONTENTS", GTK_TARGET_SAME_APP, 0 }
};

static void text_view_start_selection_dnd  (GtkTextView       *text_view,
                                            G_GNUC_UNUSED const GtkTextIter *iter,
                                            GdkEventMotion    *event)
{
    GdkDragContext *context;
    GtkTargetList *target_list;

    text_view->drag_start_x = -1;
    text_view->drag_start_y = -1;
    text_view->pending_place_cursor_button = 0;

    target_list = gtk_target_list_new (gtk_text_view_target_table,
                                       G_N_ELEMENTS (gtk_text_view_target_table));
    gtk_target_list_add_text_targets (target_list, 0);

    context = gtk_drag_begin (GTK_WIDGET (text_view), target_list,
                              (GdkDragAction) (GDK_ACTION_COPY | GDK_ACTION_MOVE),
                              1, (GdkEvent*)event);

    gtk_target_list_unref (target_list);

    gtk_drag_set_icon_default (context);
}


inline static int char_class (const GtkTextIter *iter)
{
    if (gtk_text_iter_ends_line (iter)) return -1;
    else if (is_space (iter)) return 0;
    else if (is_word_char (iter)) return 1;
    else return 2;
}

int     _moo_edit_extend_selection      (MooEdit            *edit,
                                         MooEditSelectionType type,
                                         GtkTextIter        *start,
                                         GtkTextIter        *end)
{
    int order = gtk_text_iter_compare (start, end);
    if (type == MOO_EDIT_SELECT_CHARS) {
        return order;
    }
    else if (type == MOO_EDIT_SELECT_WORDS) {
        int ch_class;

        if (!order && edit->priv->double_click_selects_brackets) {
            GtkTextIter rstart = *start;
            if (moo_edit_at_bracket (get_buf (edit), &rstart) &&
                !(order == 1 && gtk_text_iter_compare (&rstart, start) == -1))  /* this means (...)| */
            {
                GtkTextIter rend = rstart;
                if (moo_edit_find_matching_bracket (get_buf (edit), &rend) ==
                        MOO_EDIT_BRACKET_MATCH_CORRECT)
                {
                    if (gtk_text_iter_compare (&rstart, &rend) > 0) {    /*  <rend>(     <rstart>) */
                        if (edit->priv->double_click_selects_inside)
                            gtk_text_iter_forward_char (&rend);
                        else
                            gtk_text_iter_forward_char (&rstart);
                    }
                    else {              /*  <rstart>(     <rend>) */
                        if (edit->priv->double_click_selects_inside)
                            gtk_text_iter_forward_char (&rstart);
                        else
                            gtk_text_iter_forward_char (&rend);
                    }
                    *start = rstart;
                    *end = rend;
                    return TRUE;
                }
            }
        }

        if (order > 0) {
            GtkTextIter *tmp = start;
            start = end; end = tmp;
        }
        ch_class = char_class (end);
        while (!gtk_text_iter_ends_line (end) &&
                char_class (end) == ch_class)
        {
            gtk_text_iter_forward_char (end);
        }
        ch_class = char_class (start);
        while (!gtk_text_iter_starts_line (start) &&
                char_class (start) == ch_class)
        {
            gtk_text_iter_backward_char (start);
        }
        if (char_class (start) != ch_class)
            gtk_text_iter_forward_char (start);
        return gtk_text_iter_compare (start, end);
    }
    else if (type == MOO_EDIT_SELECT_LINES) {
        if (order > 0) {
            GtkTextIter *tmp = start;
            start = end; end = tmp;
        }
        gtk_text_iter_set_line_offset (start, 0);
        gtk_text_iter_forward_line (end);
        return gtk_text_iter_compare (start, end);
    }
    g_assert_not_reached ();
}


static void start_drag_scroll (MooEdit *edit)
{
    if (!edit->priv->drag_scroll_timeout)
        edit->priv->drag_scroll_timeout =
            g_timeout_add (SCROLL_TIMEOUT,
                           (GSourceFunc)drag_scroll_timeout_func,
                           edit);
    drag_scroll_timeout_func (edit);
}


static void stop_drag_scroll (MooEdit *edit)
{
    if (edit->priv->drag_scroll_timeout)
        g_source_remove (edit->priv->drag_scroll_timeout);
    edit->priv->drag_scroll_timeout = 0;
}


static gboolean drag_scroll_timeout_func (MooEdit *edit)
{
    GtkTextView *text_view;
    int x, y, px, py;
    GtkTextIter iter;
    GtkTextIter start;
    MooEditSelectionType t;
    GtkTextBuffer *buffer;

    g_assert (edit->priv->drag_type == MOO_EDIT_DRAG_SELECT);

    text_view = GTK_TEXT_VIEW (edit);

    gdk_window_get_pointer (gtk_text_view_get_window (text_view, GTK_TEXT_WINDOW_TEXT),
                            &px, &py, NULL);
    gtk_text_view_window_to_buffer_coords (text_view, GTK_TEXT_WINDOW_TEXT,
                                           px, py, &x, &y);
    gtk_text_view_get_iter_at_location (text_view, &iter, x, y);

    buffer = gtk_text_view_get_buffer (text_view);

    gtk_text_view_get_iter_at_location (text_view, &start,
                                        edit->priv->drag_start_x,
                                        edit->priv->drag_start_y);

    if (edit->priv->drag_button == GDK_BUTTON_PRESS) t = MOO_EDIT_SELECT_CHARS;
    else if (edit->priv->drag_button == GDK_2BUTTON_PRESS) t = MOO_EDIT_SELECT_WORDS;
    else t = MOO_EDIT_SELECT_LINES;

    if (extend_selection (edit, t, &start, &iter))
        gtk_text_buffer_select_range (buffer, &iter, &start);
    else
        gtk_text_buffer_place_cursor (buffer, &iter);
    gtk_text_view_scroll_mark_onscreen (text_view,
                                        gtk_text_buffer_get_insert (buffer));
    return TRUE;
}
