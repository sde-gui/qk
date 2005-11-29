/*
 *   mooterm/mootermselection.c
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

#define MOOTERM_COMPILATION
#include "mooterm/mooterm-text-private.h"
#include "mooterm/mooterm-private.h"
#include "mooterm/mootermbuffer-private.h"
#include "mooterm/mooterm-selection.h"
#include "mooterm/mootermline-private.h"


typedef struct {
    MooTermIter start;
    MooTermIter end;
} Segment;

#define GET_SELECTION(term__) ((Segment*)(term__)->priv->selection)

#define ITER_TOTAL_HEIGHT(iter__) ((int)((iter__)->term->priv->height + buf_scrollback ((iter__)->buffer)))
#define ITER_TERM(iter__)   ((iter__)->term)
#define ITER_ROW(iter__)    ((iter__)->row)
#define ITER_COL(iter__)    ((iter__)->col)
#define ITER_WIDTH(iter__)  ((iter__)->width)

#define FILL_ITER(iter__,term__,row__,col__)                \
    (iter__)->term = term__;                                \
    (iter__)->buffer = term__->priv->buffer;                \
    (iter__)->line = buf_line ((iter__)->buffer, row__);    \
    (iter__)->row = row__;                                  \
    (iter__)->col = col__;                                  \
    (iter__)->width = term__->priv->width;                  \
    CHECK_ITER(iter__)


#ifdef DEBUG
static void CHECK_ITER (const MooTermIter *iter)
{
    g_assert (iter->term != NULL);
    g_assert ((int)iter->term->priv->width == iter->width);
    g_assert (iter->row >= 0);
    g_assert (iter->row < ITER_TOTAL_HEIGHT(iter));
    g_assert (iter->col >= 0);
    g_assert (iter->col <= iter->width);
}

static void CHECK_SEGMENT (const Segment *segment)
{
    CHECK_ITER (&segment->start);
    CHECK_ITER (&segment->end);
}
#else
#define CHECK_ITER(iter__)
#define CHECK_SEGMENT(segment__)
#endif


static void     get_start_iter      (MooTerm            *term,
                                     MooTermIter        *iter);
static void     get_end_iter        (MooTerm            *term,
                                     MooTermIter        *iter);
static int      iter_cmp            (const MooTermIter  *first,
                                     const MooTermIter  *second);
static void     iter_set_start      (MooTermIter        *iter);

static char    *segment_get_text    (Segment            *segment);
static void     invalidate_segment  (Segment            *segments,
                                     guint               num);

static void     middle_button_click (MooTerm            *term);
static void     right_button_click  (MooTerm            *term,
                                     GdkEventButton     *event);
#define MOUSE_STUFF(term__) (&(term__)->priv->mouse_stuff)


gpointer
_moo_term_selection_new  (MooTerm    *term)
{
    Segment *sel = g_new0 (Segment, 1);

    FILL_ITER (&sel->start, term, 0, 0);
    FILL_ITER (&sel->end, term, 0, 0);

    return sel;
}


void
_moo_term_selection_invalidate (MooTerm    *term)
{
    Segment *sel = term->priv->selection;

    invalidate_segment (sel, 1);
    FILL_ITER (&sel->start, term, 0, 0);
    FILL_ITER (&sel->end, term, 0, 0);

    _moo_term_release_selection (term);
}


static gboolean segment_empty (Segment *s)
{
    CHECK_SEGMENT (s);
    return !iter_cmp (&s->start, &s->end);
}


static int segment_sym_diff (Segment *s1, Segment *s2,
                             Segment *result)
{
    Segment *left = result;
    Segment *right = &result[1];

    CHECK_SEGMENT (s1);
    CHECK_SEGMENT (s2);

    if (segment_empty (s1))
    {
        if (segment_empty (s2))
        {
            return 0;
        }
        else
        {
            *left = *s2;
            CHECK_SEGMENT (left);
            return 1;
        }
    }
    else if (segment_empty (s2))
    {
        *left = *s1;
        CHECK_SEGMENT (left);
        return 1;
    }
    else
    {
        Segment tmp;
        MooTermIter itmp;

        *left = *s1;
        *right = *s2;
        _moo_term_iter_order (&left->start, &left->end);
        _moo_term_iter_order  (&right->start, &right->end);

        switch (iter_cmp (&left->start, &right->start))
        {
            case 0:
                switch (iter_cmp (&left->end, &right->end))
                {
                    case 0:
                        return 0;

                    case 1:
                        left->start = right->end;
                        CHECK_SEGMENT (left);
                        return 1;

                    case -1:
                        left->start = left->end;
                        left->end = right->end;
                        CHECK_SEGMENT (left);
                        return 1;
                }

            case 1:
                tmp = *left;
                *left = *right;
                *right = tmp;
                break;
        }

        switch (iter_cmp (&left->end, &right->start))
        {
            case 0:
                left->end = right->end;
                CHECK_SEGMENT (left);
                return 1;

            case -1:
                CHECK_SEGMENT (left);
                CHECK_SEGMENT (right);
                return 2;

            case 1:
                itmp = left->end;
                left->end = right->start;
                right->start = itmp;
                CHECK_SEGMENT (left);
                CHECK_SEGMENT (right);
                return 2;
        }
    }

    g_return_val_if_reached (0);
}


static void invalidate_segment (Segment *segm, guint num)
{
    MooTerm *term;
    int top_line;
    guint i;

    if (!num)
        return;

    term = ITER_TERM (&segm->start);
    top_line = term_top_line (term);

    for (i = 0; i < num; ++i)
    {
        MooTermIter start = segm[i].start;
        MooTermIter end = segm[i].end;
        GdkRectangle rect;

        _moo_term_iter_order  (&start, &end);

        if (ITER_ROW (&start) < ITER_ROW (&end))
        {
            if (ITER_COL (&start) < (int)term->priv->width)
            {
                rect.x = ITER_COL (&start);
                rect.width = term->priv->width - rect.x;
                rect.y = ITER_ROW (&start) - top_line;
                rect.height = 1;
                _moo_term_invalidate_rect (term, &rect);
            }

            if (ITER_ROW (&start) + 1 < ITER_ROW (&end))
            {
                rect.x = 0;
                rect.width = term->priv->width;
                rect.y = ITER_ROW (&start) + 1 - top_line;
                rect.height = ITER_ROW (&end) - ITER_ROW (&start) - 1;
                _moo_term_invalidate_rect (term, &rect);
            }

            if (ITER_COL (&end) > 0)
            {
                rect.x = 0;
                rect.width = ITER_COL (&end);
                rect.y = ITER_ROW (&end) - top_line;
                rect.height = 1;
                _moo_term_invalidate_rect (term, &rect);
            }
        }
        else
        {
            if (ITER_COL (&start) < ITER_COL (&end))
            {
                rect.x = ITER_COL (&start);
                rect.width = ITER_COL (&end) - ITER_COL (&start);
                rect.y = ITER_ROW (&start) - top_line;
                rect.height = 1;
                _moo_term_invalidate_rect (term, &rect);
            }
        }
    }
}


void
_moo_term_select_range (MooTerm            *term,
                        const MooTermIter  *start,
                        const MooTermIter  *end)
{
    Segment diff[2];
    Segment new_sel;
    Segment old_selection;
    gboolean inv = FALSE;

    CHECK_ITER (start);
    CHECK_ITER (end);

    old_selection = *GET_SELECTION (ITER_TERM (start));
    CHECK_SEGMENT (&old_selection);

    new_sel.start = *start;
    new_sel.end = *end;

    switch (iter_cmp (start, end))
    {
        case 0:
            iter_set_start (&new_sel.start);
            iter_set_start (&new_sel.end);
            break;

        case 1:
            inv = TRUE;
            break;
    }

    _moo_term_iter_order  (&new_sel.start, &new_sel.end);

    if (ITER_COL(&new_sel.start) == ITER_WIDTH(&new_sel.start))
    {
        g_assert (ITER_ROW(&new_sel.start) < ITER_TOTAL_HEIGHT(&new_sel.start));
        if (ITER_ROW(&new_sel.start) + 1 < ITER_TOTAL_HEIGHT(&new_sel.start))
        {
            ITER_ROW(&new_sel.start)++;
            ITER_COL(&new_sel.start) = 0;
        }
    }

    if (ITER_COL(&new_sel.end) == 0 && ITER_ROW(&new_sel.end) != 0)
    {
        ITER_ROW(&new_sel.end)--;
        ITER_COL(&new_sel.end) = ITER_WIDTH(&new_sel.start);
    }

    if (inv)
        _moo_term_iter_order  (&new_sel.end, &new_sel.start);

    CHECK_SEGMENT (&new_sel);

    *GET_SELECTION (ITER_TERM (start)) = new_sel;

    invalidate_segment (diff,
                        segment_sym_diff (&new_sel,
                                           &old_selection,
                                           diff));

    if (_moo_term_selection_empty (term))
        _moo_term_release_selection (term);
    else
        _moo_term_grab_selection (term);
}


static void     get_start_iter      (MooTerm            *term,
                                     MooTermIter        *iter)
{
    FILL_ITER (iter, term, 0, 0);
}


static void     get_end_iter        (MooTerm            *term,
                                     MooTermIter        *iter)
{
    int width = term->priv->width;
    int total_height = term->priv->height + buf_scrollback (term->priv->buffer);
    FILL_ITER (iter, term, total_height - 1, width);
}


/**************************************************************************/
/* mouse functionality
 */

static void     start_drag_scroll               (MooTerm    *term);
static void     stop_drag_scroll                (MooTerm    *term);
static gboolean drag_scroll_timeout_func        (MooTerm    *term);
static void     scroll_selection_end_onscreen   (MooTerm    *term);
static void     start_selection_dnd             (MooTerm    *term,
                                                 const MooTermIter *iter,
                                                 GdkEventMotion *event);

#define SCROLL_TIMEOUT 100


gboolean
_moo_term_button_press (GtkWidget      *widget,
                        GdkEventButton *event)
{
    MooTermIter iter;
    int x, y;
    MooTerm *term = MOO_TERM (widget);

    moo_term_set_pointer_visible (term, TRUE);

    if (!GTK_WIDGET_HAS_FOCUS (widget))
        gtk_widget_grab_focus (widget);

    moo_term_window_to_buffer_coords (term, event->x, event->y, &x, &y);
    moo_term_get_iter_at_location (term, &iter, x, y);

    if (event->type == GDK_BUTTON_PRESS)
    {
        if (event->button == 1)
        {
#if 0
            MooTermIter sel_start, sel_end;
#endif

            MOUSE_STUFF(term)->drag_button = GDK_BUTTON_PRESS;
            MOUSE_STUFF(term)->drag_start_x = x;
            MOUSE_STUFF(term)->drag_start_y = y;

#if 0
            /* TODO: implement drag'n'drop */
            /* if clicked in selected, start drag */
            if (_moo_term_get_selection_bounds (term, &sel_start, &sel_end))
            {
                moo_term_iter_order (&sel_start, &sel_end);
                if (_moo_term_iter_in_range (&iter, &sel_start, &sel_end)
                    && FALSE)
                {
                    /* clicked inside of selection,
                    * set up drag and return */
                    MOUSE_STUFF(term)->drag_type = DRAG_DRAG;
                    return TRUE;
                }
            }
#endif

            /* otherwise, clear selection, and position cursor at clicked point */
            if (event->state & GDK_SHIFT_MASK)
                _moo_term_place_selection_end (term, &iter);
            else
                _moo_term_select_range (term, &iter, &iter);

            MOUSE_STUFF(term)->drag_type = DRAG_SELECT;
        }
        else if (event->button == 2)
        {
            middle_button_click (term);
        }
        else if (event->button == 3)
        {
            right_button_click (term, event);
        }
        else
        {
            g_warning ("got button %d in button_press callback", event->button);
        }
    }
    else if (event->type == GDK_2BUTTON_PRESS && event->button == 1)
    {
        MooTermIter bound;

        if (_moo_term_get_selection_bounds (term, NULL, NULL))
        {
            /* it may happen sometimes, if you click fast enough */
            _moo_term_select_range (term, &iter, &iter);
        }

        MOUSE_STUFF(term)->drag_button = GDK_2BUTTON_PRESS;
        MOUSE_STUFF(term)->drag_start_x = x;
        MOUSE_STUFF(term)->drag_start_y = y;
        MOUSE_STUFF(term)->drag_type = DRAG_SELECT;

        bound = iter;

        if (_moo_term_extend_selection (term, MOO_TEXT_SELECT_WORDS, &iter, &bound))
            _moo_term_select_range (term, &bound, &iter);
    }
    else if (event->type == GDK_3BUTTON_PRESS && event->button == 1)
    {
        MooTermIter bound;

        MOUSE_STUFF(term)->drag_button = GDK_3BUTTON_PRESS;
        MOUSE_STUFF(term)->drag_start_x = x;
        MOUSE_STUFF(term)->drag_start_y = y;
        MOUSE_STUFF(term)->drag_type = DRAG_SELECT;

        bound = iter;

        if (_moo_term_extend_selection (term, MOO_TEXT_SELECT_LINES, &iter, &bound))
            _moo_term_select_range (term, &bound, &iter);
    }

    return TRUE;
}


gboolean
_moo_term_button_release (GtkWidget      *widget,
                          G_GNUC_UNUSED GdkEventButton *event)
{
    MooTerm *term = MOO_TERM (widget);
    MooTermIter iter;

    moo_term_set_pointer_visible (term, TRUE);

    switch (MOUSE_STUFF(term)->drag_type)
    {
        case DRAG_NONE:
            /* it may happen after right-click, or clicking outside
            * of widget or something like that
            * everything has been taken care of, so do nothing */
            break;

        case DRAG_SELECT:
            /* everything should be done already in button_press and
            * motion_notify handlers */
            stop_drag_scroll (term);
            break;

        case DRAG_DRAG:
            /* if we were really dragging, drop it
            * otherwise, it was just a single click in selected text */
            g_assert (!MOUSE_STUFF(term)->drag_moved); /* parent should handle drag */ /* TODO ??? */

            moo_term_get_iter_at_location (term, &iter,
                                           MOUSE_STUFF(term)->drag_start_x,
                                           MOUSE_STUFF(term)->drag_start_y);
            _moo_term_select_range (term, &iter, &iter);
            break;

        default:
            g_assert_not_reached ();
    }

    MOUSE_STUFF(term)->drag_moved = FALSE;
    MOUSE_STUFF(term)->drag_type = DRAG_NONE;
    MOUSE_STUFF(term)->drag_start_x = -1;
    MOUSE_STUFF(term)->drag_start_y = -1;
    MOUSE_STUFF(term)->drag_button = GDK_BUTTON_RELEASE;

    return TRUE;
}


#define OUTSIDE(x,y,rect)               \
    ((x) < (rect).x ||                  \
     (y) < (rect).y ||                  \
     (x) >= (rect).x + (rect).width ||  \
     (y) >= (rect).y + (rect).height)


gboolean
_moo_term_motion_notify (GtkWidget      *widget,
                         GdkEventMotion *event)
{
    MooTerm *term = MOO_TERM (widget);
    int x, y, event_x, event_y;
    MooTermIter iter;

    moo_term_set_pointer_visible (MOO_TERM (widget), TRUE);

    if (!MOUSE_STUFF(term)->drag_type)
        return FALSE;

    if (event->is_hint)
    {
        gdk_window_get_pointer (event->window, &event_x, &event_y, NULL);
    }
    else {
        event_x = (int)event->x;
        event_y = (int)event->y;
    }

    moo_term_window_to_buffer_coords (term, event_x, event_y, &x, &y);
    moo_term_get_iter_at_location (term, &iter, x, y);

    if (MOUSE_STUFF(term)->drag_type == DRAG_SELECT)
    {
        GdkRectangle rect;
        MooTermIter start;
        MooTextSelectionType t;

        MOUSE_STUFF(term)->drag_moved = TRUE;
        _moo_term_get_visible_rect (term, &rect);

        if (OUTSIDE (x, y, rect))
        {
            start_drag_scroll (term);
            return TRUE;
        }
        else
        {
            stop_drag_scroll (term);
        }

        moo_term_get_iter_at_location (term, &start,
                                       MOUSE_STUFF(term)->drag_start_x,
                                       MOUSE_STUFF(term)->drag_start_y);

        switch (MOUSE_STUFF(term)->drag_button)
        {
            case GDK_BUTTON_PRESS:
                t = MOO_TEXT_SELECT_CHARS;
                break;
            case GDK_2BUTTON_PRESS:
                t = MOO_TEXT_SELECT_WORDS;
                break;
            default:
                t = MOO_TEXT_SELECT_LINES;
        }

        if (_moo_term_extend_selection (term, t, &iter, &start))
            _moo_term_select_range (term, &start, &iter);
        else
            _moo_term_select_range (term, &iter, &iter);
    }
    else
    {
        /* this piece is from gtktextview.c */
        int x, y;

        gdk_window_get_pointer (widget->window, &x, &y, NULL);

        if (gtk_drag_check_threshold (widget, MOUSE_STUFF(term)->drag_start_x,
                                      MOUSE_STUFF(term)->drag_start_y, x, y))
        {
            MooTermIter iter;
            int buffer_x, buffer_y;

            moo_term_window_to_buffer_coords (term,
                                              MOUSE_STUFF(term)->drag_start_x,
                                              MOUSE_STUFF(term)->drag_start_y,
                                              &buffer_x, &buffer_y);

            moo_term_get_iter_at_location (term, &iter, buffer_x, buffer_y);

            MOUSE_STUFF(term)->drag_type = DRAG_NONE;
            start_selection_dnd (term, &iter, event);
        }
    }

    return TRUE;
}


static void
start_drag_scroll (MooTerm *term)
{
    if (!MOUSE_STUFF(term)->drag_scroll_timeout)
        MOUSE_STUFF(term)->drag_scroll_timeout =
                g_timeout_add (SCROLL_TIMEOUT,
                               (GSourceFunc) drag_scroll_timeout_func,
                               term);

    drag_scroll_timeout_func (term);
}


static void
stop_drag_scroll (MooTerm *term)
{
    if (MOUSE_STUFF(term)->drag_scroll_timeout)
        g_source_remove (MOUSE_STUFF(term)->drag_scroll_timeout);
    MOUSE_STUFF(term)->drag_scroll_timeout = 0;
}


static gboolean
drag_scroll_timeout_func (MooTerm *term)
{
    int x, y, px, py;
    MooTermIter iter;
    MooTermIter start;
    MooTextSelectionType t;

    g_assert (MOUSE_STUFF(term)->drag_type == DRAG_SELECT);

    gdk_window_get_pointer (GTK_WIDGET(term)->window, &px, &py, NULL);
    moo_term_window_to_buffer_coords (term, px, py, &x, &y);
    moo_term_get_iter_at_location (term, &iter, x, y);

    moo_term_get_iter_at_location (term, &start,
                                   MOUSE_STUFF(term)->drag_start_x,
                                   MOUSE_STUFF(term)->drag_start_y);

    switch (MOUSE_STUFF(term)->drag_button)
    {
        case GDK_BUTTON_PRESS:
            t = MOO_TEXT_SELECT_CHARS;
            break;
        case GDK_2BUTTON_PRESS:
            t = MOO_TEXT_SELECT_WORDS;
            break;
        default:
            t = MOO_TEXT_SELECT_LINES;
    }

    if (_moo_term_extend_selection (term, t, &iter, &start))
        _moo_term_select_range (term, &start, &iter);
    else
        _moo_term_select_range (term, &iter, &iter);

    scroll_selection_end_onscreen (term);

    return TRUE;
}


static void
scroll_selection_end_onscreen (MooTerm *term)
{
    MooTermIter iter = GET_SELECTION(term)->end;
    guint top_line = term_top_line (term);

    if (ITER_ROW(&iter) < (int)top_line)
        moo_term_scroll_lines (term, ITER_ROW(&iter) - (int)top_line);
    else if (ITER_ROW(&iter) >= (int)top_line + (int)term->priv->height)
        moo_term_scroll_lines (term, ITER_ROW(&iter) + 1 - (int)top_line -
                (int)term->priv->height);
}


static void
start_selection_dnd (G_GNUC_UNUSED MooTerm    *term,
                     G_GNUC_UNUSED const MooTermIter *iter,
                     G_GNUC_UNUSED GdkEventMotion *event)
{
    g_return_if_reached ();
}













gboolean
_moo_term_selection_empty (MooTerm    *term)
{
    return segment_empty (GET_SELECTION (term));
}


gboolean
_moo_term_get_selected_cells (MooTerm    *term,
                              guint      *left_row,
                              guint      *left_col,
                              guint      *right_row,
                              guint      *right_col)
{
    Segment *selection = GET_SELECTION (term);

    if (segment_empty (selection))
    {
        return FALSE;
    }
    else
    {
        MooTermIter start = selection->start;
        MooTermIter end = selection->end;

        _moo_term_iter_order  (&start, &end);

        if (left_row)
            *left_row = ITER_ROW (&start);
        if (left_col)
            *left_col = ITER_COL (&start);
        if (right_row)
            *right_row = ITER_ROW (&end);
        if (right_col)
            *right_col = ITER_COL (&end);

        return TRUE;
    }
}


void
_moo_term_selection_clear (MooTerm    *term)
{
    MooTermIter i;
    FILL_ITER (&i, term, 0, 0);
    _moo_term_select_range (term, &i, &i);
}


static void
middle_button_click (MooTerm *term)
{
    moo_term_paste_clipboard (term, GDK_SELECTION_PRIMARY);
}


static void
right_button_click (MooTerm        *term,
                    GdkEventButton *event)
{
    _moo_term_do_popup_menu (term, event);
}


void
moo_term_window_to_buffer_coords (MooTerm            *term,
                                  int                 window_x,
                                  int                 window_y,
                                  int                *buffer_x,
                                  int                *buffer_y)
{
    g_return_if_fail (MOO_IS_TERM (term));
    g_return_if_fail (buffer_x != NULL && buffer_y != NULL);
    *buffer_x = window_x;
    *buffer_y = window_y + (term_top_line (term) * term_char_height (term));
}


void
moo_term_get_iter_at_location (MooTerm            *term,
                               MooTermIter        *iter,
                               int                 x,
                               int                 y)
{
    int char_width = term_char_width (term);
    int char_height = term_char_height (term);
    int scrollback = buf_scrollback (term->priv->buffer);

    g_return_if_fail (iter != NULL);

    y /= char_height;

    x = CLAMP (x, 0, (int)term->priv->width * char_width - 1);
    x = (x % char_width > char_width / 2) ?
                            x / char_width + 1 :
                            x / char_width;

    if (y < 0)
    {
        y = 0;
        x = 0;
    }
    else if (y > scrollback + (int)term->priv->height - 1)
    {
        y = scrollback + (int)term->priv->height - 1;
        x = term->priv->width;
    }

    FILL_ITER (iter, term, y, x);
}


void
moo_term_get_iter_at_pos (MooTerm            *term,
                          MooTermIter        *iter,
                          int                 x,
                          int                 y)
{
    int char_width = term_char_width (term);
    int char_height = term_char_height (term);
    int scrollback = buf_scrollback (term->priv->buffer);

    g_return_if_fail (iter != NULL);

    y /= char_height;
    x /= char_width;
    x = CLAMP (x, 0, (int)term->priv->width);

    if (y < 0)
    {
        y = 0;
        x = 0;
    }
    else if (y > scrollback + (int)term->priv->height - 1)
    {
        y = scrollback + (int)term->priv->height - 1;
        x = term->priv->width;
    }

    FILL_ITER (iter, term, y, x);
}


gboolean
_moo_term_get_selection_bounds (MooTerm            *term,
                                MooTermIter        *sel_start,
                                MooTermIter        *sel_end)
{
    Segment *selection = term->priv->selection;

    if (sel_start)
        *sel_start = selection->start;

    if (sel_end)
        *sel_end = selection->end;

    return !_moo_term_selection_empty (term);
}


static int      iter_cmp                (const MooTermIter  *first,
                                         const MooTermIter  *second)
{
    g_assert (ITER_TERM (first) == ITER_TERM (second));

    if (ITER_ROW (first) < ITER_ROW (second))
        return -1;
    else if (ITER_ROW (first) > ITER_ROW (second))
        return 1;
    else if (ITER_COL (first) < ITER_COL (second))
        return -1;
    else if (ITER_COL (first) > ITER_COL (second))
        return 1;
    else
        return 0;
}


static void iter_set_start  (MooTermIter        *iter)
{
    ITER_ROW (iter) = ITER_COL (iter) = 0;
    CHECK_ITER (iter);
}


void
_moo_term_iter_order (MooTermIter    *first,
                      MooTermIter    *second)
{
    if (iter_cmp (first, second) > 0)
    {
        MooTermIter tmp = *first;
        *first = *second;
        *second = tmp;
    }
}


gboolean
_moo_term_iter_in_range (const MooTermIter  *iter,
                         const MooTermIter  *start,
                         const MooTermIter  *end)
{
    CHECK_ITER (iter);
    CHECK_ITER (start);
    CHECK_ITER (end);
    return iter_cmp (start, iter) <= 0 && iter_cmp (iter, end) <= 0;
}


void
_moo_term_place_selection_end (MooTerm            *term,
                               const MooTermIter  *where)
{
    CHECK_ITER (where);
    _moo_term_select_range (term, &GET_SELECTION(term)->start, where);
}


void
_moo_term_get_visible_rect (MooTerm            *term,
                           GdkRectangle       *rect)
{
    int char_height = term_char_height (term);
    rect->x = 0;
    rect->width = term->priv->width * term_char_width (term);
    rect->y = term_top_line (term) * char_height;
    rect->height = term->priv->height * char_height;
}


gboolean
_moo_term_cell_selected (MooTerm    *term,
                         guint       row,
                         guint       col)
{
    MooTermIter start, end, iter;

    if (_moo_term_get_selection_bounds (term, &start, &end))
    {
        _moo_term_iter_order  (&start, &end);
        FILL_ITER (&iter, term, row, col);
        return iter_cmp (&start, &iter) <= 0 && iter_cmp (&iter, &end) < 0;
    }
    else
    {
        return FALSE;
    }
}


int
_moo_term_row_selected (MooTerm    *term,
                        guint       row)
{
    guint l_row, l_col, r_row, r_col;

    if (_moo_term_get_selected_cells (term, &l_row, &l_col, &r_row, &r_col))
    {
        if (row < l_row || row > r_row)
            return NOT_SELECTED;
        else if (l_row < row && row < r_row)
            return FULL_SELECTED;
        else
            return PART_SELECTED;
    }
    else
    {
        return NOT_SELECTED;
    }
}


char       *moo_term_get_selection (MooTerm    *term)
{
    return segment_get_text (term->priv->selection);
}


static int      char_class              (MooTerm            *term,
                                         const MooTermIter  *iter);
static gboolean iter_ends_line          (const MooTermIter  *iter);
static gboolean iter_starts_line        (const MooTermIter  *iter);
static void     iter_forward_char       (MooTermIter        *iter);
static void     iter_backward_char      (MooTermIter        *iter);
static void     iter_set_line_offset    (MooTermIter        *iter,
                                         guint               offset);
static gboolean iter_forward_line       (MooTermIter        *iter);
static gboolean is_space                (const MooTermIter  *iter);
static gboolean is_word_char            (const MooTermIter  *iter);
static gunichar iter_get_char           (const MooTermIter  *iter);


gboolean
_moo_term_extend_selection (MooTerm            *term,
                            MooTextSelectionType type,
                            MooTermIter        *end,
                            MooTermIter        *start)
{
    int order = iter_cmp (start, end);

    CHECK_ITER (start);
    CHECK_ITER (end);

    if (type == MOO_TEXT_SELECT_CHARS)
    {
        return order;
    }

    if (order > 0)
    {
        MooTermIter *tmp = start;
        start = end; end = tmp;
    }

    if (type == MOO_TEXT_SELECT_WORDS)
    {
        int ch_class;

        ch_class = char_class (term, end);

        while (!iter_ends_line (end) &&
                char_class (term, end) == ch_class)
        {
            iter_forward_char (end);
        }

        ch_class = char_class (term, start);

        while (!iter_starts_line (start) &&
                char_class (term, start) == ch_class)
        {
            iter_backward_char (start);
        }

        if (char_class (term, start) != ch_class)
            iter_forward_char (start);

        return iter_cmp (start, end);
    }

    if (type == MOO_TEXT_SELECT_LINES)
    {
        iter_set_line_offset (start, 0);
        iter_forward_line (end);
        return iter_cmp (start, end);
    }

    g_return_val_if_reached (FALSE);
}


/* TODO */
static int
char_class (MooTerm            *term,
            const MooTermIter  *iter)
{
    if (iter_ends_line (iter))
        return -1;
    else if (is_space (iter))
        return 0;
    else if (is_word_char (iter))
        return 1;
    else
        return 2;
}


static gboolean
iter_ends_line (const MooTermIter *iter)
{
    CHECK_ITER (iter);
    return iter->col == (int)iter->term->priv->width;
}


static gboolean
iter_starts_line (const MooTermIter *iter)
{
    CHECK_ITER (iter);
    return iter->col == 0;
}


static void     iter_forward_char       (MooTermIter        *iter)
{
    MooTerm *term = iter->term;
    int width = term->priv->width;
    int total_height = term->priv->height + buf_scrollback (term->priv->buffer);

    g_return_if_fail (iter->row < total_height || iter->col < width);

    if (iter->col < width)
    {
        iter->col++;
    }
    else
    {
        iter->col = 0;
        iter->row++;
    }

    CHECK_ITER (iter);
}


static void     iter_backward_char      (MooTermIter        *iter)
{
    MooTerm *term = iter->term;
    int width = term->priv->width;

    g_return_if_fail (iter->row > 0 || iter->col > 0);

    if (iter->col > 0)
    {
        iter->col--;
    }
    else
    {
        iter->row--;
        iter->col = width;
    }

    CHECK_ITER (iter);
}


static void     iter_set_line_offset    (MooTermIter        *iter,
                                         guint               offset)
{
    g_return_if_fail (offset <= iter->term->priv->width);
    iter->col = offset;
    CHECK_ITER (iter);
}


static gboolean iter_forward_line       (MooTermIter        *iter)
{
    MooTerm *term = iter->term;
    int width = term->priv->width;
    int total_height = term->priv->height + buf_scrollback (term->priv->buffer);

    if (iter->row == total_height - 1)
    {
        iter->col = width;
        CHECK_ITER (iter);
        return FALSE;
    }
    else
    {
        iter->col = 0;
        iter->row++;
        CHECK_ITER (iter);
        return TRUE;
    }
}


static gboolean is_space                (const MooTermIter  *iter)
{
    gunichar c = iter_get_char (iter);
    g_return_val_if_fail (c != 0, FALSE);
    return g_unichar_isspace (c);
}


static gboolean is_word_char            (const MooTermIter  *iter)
{
    gunichar c = iter_get_char (iter);
    g_return_val_if_fail (c != 0, FALSE);
    return g_unichar_isalnum (c) || c == '_';
}


static gunichar
iter_get_char (const MooTermIter *iter)
{
    MooTermLine *line = buf_line (iter->term->priv->buffer, iter->row);
    return _moo_term_line_get_char (line, iter->col);
}


static char*
segment_get_text (Segment *segment)
{
    MooTerm *term = ITER_TERM (&segment->start);
    MooTermBuffer *buf = term->priv->buffer;
    MooTermIter start = segment->start;
    MooTermIter end = segment->end;
    GString *text;
    MooTermLine *line;
    int width = term->priv->width;
    int i, j;

    _moo_term_iter_order (&start, &end);

    if (!iter_cmp (&start, &end))
        return NULL;

    text = g_string_new ("");

    if (ITER_ROW(&start) == ITER_ROW(&end))
    {
        line = buf_line (buf, ITER_ROW(&start));

        for (j = ITER_COL(&start);
             j < ITER_COL(&end) && j < (int) _moo_term_line_len (line); ++j)
                g_string_append_unichar (text, _moo_term_line_get_char (line, j));
    }
    else
    {
        if (ITER_COL(&start) < width)
        {
            line = buf_line (buf, ITER_ROW(&start));

            for (j = ITER_COL(&start); j < (int) _moo_term_line_len (line); ++j)
                g_string_append_unichar (text, _moo_term_line_get_char (line, j));

            if (!_moo_term_line_wrapped (line))
                g_string_append_c (text, '\n');
        }

        for (i = ITER_ROW(&start) + 1; i < ITER_ROW(&end); ++i)
        {
            line = buf_line (buf, i);

            for (j = 0; j < (int) _moo_term_line_len (line); ++j)
                g_string_append_unichar (text, _moo_term_line_get_char (line, j));

            if (!_moo_term_line_wrapped (line))
                g_string_append_c (text, '\n');
        }

        if (ITER_COL(&end) > 0)
        {
            line = buf_line (buf, ITER_ROW(&end));

            for (j = 0; j < ITER_COL(&end) && j < (int) _moo_term_line_len (line); ++j)
                g_string_append_unichar (text, _moo_term_line_get_char (line, j));

            if (iter_ends_line (&end) && !_moo_term_line_wrapped (line))
                g_string_append_c (text, '\n');
        }
    }

    return g_string_free (text, FALSE);
}


void
moo_term_select_all (MooTerm        *term)
{
    MooTermIter start, end;

    get_start_iter (term, &start);
    get_end_iter (term, &end);

    _moo_term_select_range (term, &start, &end);
}


char       *moo_term_get_content            (MooTerm        *term)
{
    Segment segm;

    get_start_iter (term, &segm.start);
    get_end_iter (term, &segm.end);

    return segment_get_text (&segm);
}


char*
moo_term_get_text (MooTermIter    *start,
                   MooTermIter    *end)
{
    /* XXX check iters */
    Segment segm = {*start, *end};
    return segment_get_text (&segm);
}


guint
moo_term_get_line_count (MooTerm *term)
{
    g_return_val_if_fail (MOO_IS_TERM (term), 0);
    return term->priv->height + buf_scrollback (term->priv->primary_buffer);
}


MooTermLine*
moo_term_get_line (MooTerm            *term,
                   guint               line_no)
{
    g_return_val_if_fail (MOO_IS_TERM (term), NULL);
    g_return_val_if_fail (line_no < moo_term_get_line_count (term), NULL);
    return buf_line (term->priv->primary_buffer, line_no);
}


gboolean
moo_term_get_iter_at_line (MooTerm            *term,
                           MooTermIter        *iter,
                           guint               line)
{
    g_return_val_if_fail (iter != NULL, FALSE);
    g_return_val_if_fail (MOO_IS_TERM (term), FALSE);

    if (line >= moo_term_get_line_count (term))
        return FALSE;

    FILL_ITER (iter, term, line, 0);
    return TRUE;
}


gboolean
moo_term_get_iter_at_line_offset (MooTerm            *term,
                                  MooTermIter        *iter,
                                  guint               line_no,
                                  guint               offset)
{
    MooTermLine *line;

    g_return_val_if_fail (iter != NULL, FALSE);
    g_return_val_if_fail (MOO_IS_TERM (term), FALSE);

    if (line_no >= moo_term_get_line_count (term))
        return FALSE;

    line = buf_line (term->priv->buffer, line_no);

    if (offset > _moo_term_line_width (line))
        return FALSE;

    FILL_ITER (iter, term, line_no, offset);
    return TRUE;
}


void
moo_term_get_iter_at_cursor (MooTerm            *term,
                             MooTermIter        *iter)
{
    g_return_if_fail (MOO_IS_TERM (term));
    g_return_if_fail (iter != NULL);

    FILL_ITER (iter, term, buf_cursor_row_abs (term->priv->buffer),
               buf_cursor_col (term->priv->buffer));
}


void
moo_term_iter_forward_to_line_end (MooTermIter *iter)
{
    MooTerm *term;

    g_return_if_fail (iter != NULL);

    term = ITER_TERM (iter);
    g_return_if_fail (MOO_IS_TERM (term));

    ITER_COL (iter) = term->priv->width;
}


void
moo_term_apply_tag (MooTerm            *term,
                    MooTermTag         *tag,
                    MooTermIter        *start,
                    MooTermIter        *end)
{
    MooTermBuffer *buf;
    MooTermLine *line;
    int i;
    Segment inv;

    g_return_if_fail (MOO_IS_TERM (term));
    g_return_if_fail (MOO_IS_TERM_TAG (tag));
    g_return_if_fail (start != NULL && end != NULL);
    g_return_if_fail (ITER_TERM(start) == term && ITER_TERM(end) == term);

    _moo_term_iter_order (start, end);
    buf = term->priv->primary_buffer;

    if (ITER_ROW (start) == ITER_ROW (end))
    {
        line = buf_line (buf, ITER_ROW (start));
        _moo_term_line_apply_tag (line, tag, ITER_COL (start),
                                  ITER_COL (end) - ITER_COL (start));
    }
    else
    {
        line = buf_line (buf, ITER_ROW (start));
        _moo_term_line_apply_tag (line, tag, ITER_COL (start),
                                  _moo_term_line_width (line) - ITER_COL (start));
        line = buf_line (buf, ITER_ROW (end));
        _moo_term_line_apply_tag (line, tag, 0, ITER_COL (end));
        if (ITER_ROW (start) + 1 < ITER_ROW (end))
        {
            for (i = ITER_ROW (start) + 1; i < ITER_ROW (end); ++i)
            {
                line = buf_line (buf, i);
                _moo_term_line_apply_tag (line, tag, 0,
                                          _moo_term_line_width (line));
            }
        }
    }

    inv.start = *start;
    inv.end = *end;
    invalidate_segment (&inv, 1);
}


void
moo_term_remove_tag (MooTerm            *term,
                     MooTermTag         *tag,
                     MooTermIter        *start,
                     MooTermIter        *end)
{
    MooTermBuffer *buf;
    MooTermLine *line;
    Segment inv;
    int i;

    g_return_if_fail (MOO_IS_TERM (term));
    g_return_if_fail (MOO_IS_TERM_TAG (tag));
    g_return_if_fail (start != NULL && end != NULL);
    g_return_if_fail (ITER_TERM(start) == term && ITER_TERM(end) == term);

    _moo_term_iter_order (start, end);
    buf = term->priv->primary_buffer;

    if (ITER_ROW (start) == ITER_ROW (end))
    {
        line = buf_line (buf, ITER_ROW (start));
        _moo_term_line_remove_tag (line, tag, ITER_COL (start),
                                   ITER_COL (end) - ITER_COL (start));
    }
    else
    {
        line = buf_line (buf, ITER_ROW (start));
        _moo_term_line_remove_tag (line, tag, ITER_COL (start),
                                   _moo_term_line_width (line) - ITER_COL (start));
        line = buf_line (buf, ITER_ROW (end));
        _moo_term_line_remove_tag (line, tag, 0, ITER_COL (end));
        if (ITER_ROW (start) + 1 < ITER_ROW (end))
        {
            for (i = ITER_ROW (start) + 1; i < ITER_ROW (end); ++i)
            {
                line = buf_line (buf, i);
                _moo_term_line_remove_tag (line, tag, 0,
                                           _moo_term_line_width (line));
            }
        }
    }

    inv.start = *start;
    inv.end = *end;
    invalidate_segment (&inv, 1);
}


gboolean
moo_term_iter_has_tag (MooTermIter        *iter,
                       MooTermTag         *tag)
{
    MooTermBuffer *buf;
    MooTerm *term;

    g_return_val_if_fail (iter != NULL, FALSE);
    g_return_val_if_fail (MOO_IS_TERM (ITER_TERM (iter)), FALSE);
    g_return_val_if_fail (MOO_IS_TERM_TAG (tag), FALSE);

    term = ITER_TERM (iter);
    buf = term->priv->primary_buffer;

    return _moo_term_line_has_tag (buf_line (buf, ITER_ROW (iter)),
                                   tag, ITER_COL (iter));
}


gboolean
moo_term_iter_get_tag_start (MooTermIter        *iter,
                             MooTermTag         *tag)
{
    MooTermBuffer *buf;
    MooTermLine *line;
    MooTerm *term;
    guint col;

    g_return_val_if_fail (iter != NULL, FALSE);
    g_return_val_if_fail (MOO_IS_TERM (ITER_TERM (iter)), FALSE);
    g_return_val_if_fail (MOO_IS_TERM_TAG (tag), FALSE);

    term = ITER_TERM (iter);
    buf = term->priv->primary_buffer;
    line = buf_line (buf, ITER_ROW (iter));

    col = ITER_COL (iter);

    if (_moo_term_line_get_tag_start (line, tag, &col))
    {
        ITER_COL (iter) = col;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


gboolean
moo_term_iter_get_tag_end (MooTermIter        *iter,
                           MooTermTag         *tag)
{
    MooTermBuffer *buf;
    MooTermLine *line;
    MooTerm *term;
    guint col;

    g_return_val_if_fail (iter != NULL, FALSE);
    g_return_val_if_fail (MOO_IS_TERM (ITER_TERM (iter)), FALSE);
    g_return_val_if_fail (MOO_IS_TERM_TAG (tag), FALSE);

    term = ITER_TERM (iter);
    buf = term->priv->primary_buffer;
    line = buf_line (buf, ITER_ROW (iter));

    col = ITER_COL (iter);

    if (_moo_term_line_get_tag_end (line, tag, &col))
    {
        ITER_COL (iter) = col;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


void
moo_term_set_line_data (MooTerm            *term,
                        MooTermLine        *line,
                        const char         *key,
                        gpointer            data)
{
    moo_term_set_line_data_full (term, line, key, data, NULL);
}


void
moo_term_set_line_data_full (MooTerm            *term,
                             MooTermLine        *line,
                             const char         *key,
                             gpointer            data,
                             GDestroyNotify      destroy)
{
    g_return_if_fail (MOO_IS_TERM (term));
    g_return_if_fail (key != NULL);
    g_return_if_fail (line != NULL);

    _moo_term_buffer_set_line_data (term->priv->primary_buffer,
                                    line, key, data, destroy);
}


gpointer
moo_term_get_line_data (MooTerm            *term,
                        MooTermLine        *line,
                        const char         *key)
{
    g_return_val_if_fail (MOO_IS_TERM (term), NULL);
    g_return_val_if_fail (key != NULL, NULL);
    g_return_val_if_fail (line != NULL, NULL);

    return _moo_term_buffer_get_line_data (term->priv->primary_buffer, line, key);
}


MooTermIter*
moo_term_iter_copy (const MooTermIter *iter)
{
    g_return_val_if_fail (iter != NULL, NULL);
    return g_memdup (iter, sizeof (MooTermIter));
}

void
moo_term_iter_free (MooTermIter *iter)
{
    g_free (iter);
}


GType
moo_term_iter_get_type (void)
{
    static GType type = 0;

    if (!type)
        type = g_boxed_type_register_static ("MooTermIter",
                                             (GBoxedCopyFunc) moo_term_iter_copy,
                                             (GBoxedFreeFunc) moo_term_iter_free);
    return type;
}
