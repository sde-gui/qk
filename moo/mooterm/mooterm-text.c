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
#include "mooterm/mooterm-private.h"
#include "mooterm/mootermbuffer-private.h"
#include "mooterm/mooterm-selection.h"
#include "mooui/mootext.h"


typedef struct {
    GtkTextIter start;
    GtkTextIter end;
} Segment;

#define GET_SELECTION(term) ((Segment*)(term)->priv->selection)

#define ITER_ROW(iter)              ((iter)->dummy3)
#define ITER_COL(iter)              ((iter)->dummy4)
#define ITER_TERM(iter)             ((MooTerm*)(iter)->dummy1)
#define ITER_SET_TERM(iter, term)   (iter)->dummy1 = term

#define FILL_ITER(iter, term, row, col) \
    (iter)->dummy1 = term;              \
    ITER_ROW(iter) = row;               \
    ITER_COL(iter) = col;


static int  iter_cmp        (const GtkTextIter  *first,
                             const GtkTextIter  *second);
static void iter_order      (GtkTextIter        *first,
                             GtkTextIter        *second);
static void iter_set_start  (GtkTextIter        *iter);



gpointer    term_selection_new  (MooTerm    *term)
{
    Segment *sel = g_new0 (Segment, 1);

    FILL_ITER (&sel->start, term, 0, 0);
    FILL_ITER (&sel->end, term, 0, 0);

    return sel;
}


static gboolean segment_empty (Segment *s)
{
    return !iter_cmp (&s->start, &s->end);
}


static int segment_sym_diff (Segment *s1, Segment *s2,
                             Segment *result)
{
    Segment *left = result;
    Segment *right = &result[1];

    if (segment_empty (s1))
    {
        if (segment_empty (s2))
        {
            return 0;
        }
        else
        {
            *left = *s2;
            return 1;
        }
    }
    else if (segment_empty (s2))
    {
        *left = *s1;
        return 1;
    }
    else
    {
        Segment tmp;
        GtkTextIter itmp;

        *left = *s1;
        *right = *s2;
        iter_order (&left->start, &left->end);
        iter_order (&right->start, &right->end);

        switch (iter_cmp (&left->start, &right->start))
        {
            case 0:
                switch (iter_cmp (&left->end, &right->end))
                {
                    case 0:
                        return 0;

                    case 1:
                        left->start = right->end;
                        return 1;

                    case -1:
                        left->start = left->end;
                        left->end = right->end;
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
                return 1;

            case -1:
                return 2;

            case 1:
                itmp = left->end;
                left->end = right->start;
                right->start = itmp;
                return 2;
        }
    }

    g_assert_not_reached ();
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
        GtkTextIter start = segm[i].start;
        GtkTextIter end = segm[i].end;
        GdkRectangle rect;

        iter_order (&start, &end);

        if (ITER_ROW (&start) < ITER_ROW (&end))
        {
            if (ITER_COL (&start) < (int)term->priv->width)
            {
                rect.x = ITER_COL (&start);
                rect.width = term->priv->width - rect.x;
                rect.y = ITER_ROW (&start) - top_line;
                rect.height = 1;
                moo_term_invalidate_rect (term, &rect);
            }

            if (ITER_ROW (&start) + 1 < ITER_ROW (&end))
            {
                rect.x = 0;
                rect.width = term->priv->width;
                rect.y = ITER_ROW (&start) + 1 - top_line;
                rect.height = ITER_ROW (&end) - ITER_ROW (&start) - 1;
                moo_term_invalidate_rect (term, &rect);
            }

            if (ITER_COL (&end) > 0)
            {
                rect.x = 0;
                rect.width = ITER_COL (&end);
                rect.y = ITER_ROW (&end) - top_line;
                rect.height = 1;
                moo_term_invalidate_rect (term, &rect);
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
                moo_term_invalidate_rect (term, &rect);
            }
        }
    }
}


static void term_select_range (const GtkTextIter *start,
                               const GtkTextIter *end)
{
    Segment diff[2];
    Segment new_selection;
    Segment old_selection;

    old_selection = *GET_SELECTION (ITER_TERM (start));

    ITER_SET_TERM (&new_selection.start, ITER_TERM (start));
    ITER_SET_TERM (&new_selection.end, ITER_TERM (start));

    new_selection.start = *start;
    new_selection.end = *end;

    if (!iter_cmp (start, end))
    {
        iter_set_start (&new_selection.start);
        iter_set_start (&new_selection.end);
    }

    GET_SELECTION (ITER_TERM (start))->start = new_selection.start;
    GET_SELECTION (ITER_TERM (start))->end = new_selection.end;

    invalidate_segment (diff,
                        segment_sym_diff (&new_selection,
                                           &old_selection,
                                           diff));
}


gboolean    moo_term_button_press           (GtkWidget      *widget,
                                             GdkEventButton *event)
{
    moo_term_set_pointer_visible (MOO_TERM (widget), TRUE);
    return moo_text_button_press_event (widget, event);
}


gboolean    moo_term_button_release         (GtkWidget      *widget,
                                             GdkEventButton *event)
{
    moo_term_set_pointer_visible (MOO_TERM (widget), TRUE);
    return moo_text_button_release_event (widget, event);
}


gboolean    moo_term_motion_notify          (GtkWidget      *widget,
                                             GdkEventMotion *event)
{
    moo_term_set_pointer_visible (MOO_TERM (widget), TRUE);
    return moo_text_motion_event (widget, event);
}



gboolean    moo_term_selection_empty    (MooTerm    *term)
{
    return segment_empty (GET_SELECTION (term));
}


gboolean    moo_term_get_selection_bounds (MooTerm  *term,
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
        if (iter_cmp (&selection->start, &selection->end) > 0)
        {
            *left_row = ITER_ROW (&selection->end);
            *left_col = ITER_COL (&selection->end);
            *right_row = ITER_ROW (&selection->start);
            *right_col = ITER_COL (&selection->start);
        }
        else
        {
            *left_row = ITER_ROW (&selection->start);
            *left_col = ITER_COL (&selection->start);
            *right_row = ITER_ROW (&selection->end);
            *right_col = ITER_COL (&selection->end);
        }
        return TRUE;
    }
}


void        moo_term_selection_clear    (MooTerm    *term)
{
    GtkTextIter i;
    FILL_ITER (&i, term, 0, 0);
    term_select_range (&i, &i);
}


static void     middle_button_click     (MooText        *obj,
                                         G_GNUC_UNUSED GdkEventButton *event)
{
    moo_term_paste_clipboard (MOO_TERM (obj), GDK_SELECTION_PRIMARY);
}


static void     right_button_click      (MooText        *obj,
                                         GdkEventButton *event)
{
    moo_term_do_popup_menu (MOO_TERM (obj), event);
}


static gboolean extend_selection        (MooText        *obj,
                                         MooTextSelectionType type,
                                         GtkTextIter    *insert,
                                         GtkTextIter    *selection_bound)
{
    term_select_range (selection_bound, insert);
    return iter_cmp (selection_bound, insert);
}


static void     window_to_buffer_coords (MooText        *obj,
                                         int             window_x,
                                         int             window_y,
                                         int            *buffer_x,
                                         int            *buffer_y)
{
    MooTerm *term = MOO_TERM (obj);
    *buffer_x = window_x;
    *buffer_y = window_y + (term_top_line (term) * term_char_height (term));
}


static void     get_iter_at_location    (MooText        *obj,
                                         GtkTextIter    *iter,
                                         int             x,
                                         int             y)
{
    MooTerm *term = MOO_TERM (obj);
    int char_width = term_char_width (term);
    int char_height = term_char_height (term);
    int scrollback = buf_scrollback (term->priv->buffer);
    guint row, col;

    g_return_if_fail (iter != NULL);

    y /= char_height;
    y = CLAMP (y, 0, scrollback + (int)term->priv->height - 1);
    row = y;

    x = CLAMP (x, 0, (int)term->priv->width * char_width - 1);
    col = (x % char_width > char_width / 2) ?
                            x / char_width + 1 :
                            x / char_width;

    FILL_ITER (iter, term, row, col);
}


static gboolean get_selection_bounds    (MooText        *obj,
                                         GtkTextIter    *sel_start,
                                         GtkTextIter    *sel_end)
{
    MooTerm *term = MOO_TERM (obj);
    Segment *selection = term->priv->selection;

    if (sel_start)
        *sel_start = selection->start;

    if (sel_end)
        *sel_end = selection->end;

    return !moo_term_selection_empty (term);
}


static int      iter_cmp                (const GtkTextIter  *first,
                                         const GtkTextIter  *second)
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


static void iter_set_start  (GtkTextIter        *iter)
{
    ITER_ROW (iter) = ITER_COL (iter) = 0;
}


static void     iter_order              (GtkTextIter    *first,
                                         GtkTextIter    *second)
{
    if (iter_cmp (first, second) > 0)
    {
        GtkTextIter tmp = *first;
        *first = *second;
        *second = tmp;
    }
}


static gboolean iter_in_range           (const GtkTextIter  *iter,
                                         const GtkTextIter  *start,
                                         const GtkTextIter  *end)
{
    return iter_cmp (start, iter) <= 0 && iter_cmp (iter, end) <= 0;
}


static void     select_range            (G_GNUC_UNUSED MooText *obj,
                                         const GtkTextIter  *start,
                                         const GtkTextIter  *end)
{
    term_select_range (start, end);
}


static void     place_selection_end     (MooText            *obj,
                                         const GtkTextIter  *where)
{
    MooTerm *term = MOO_TERM (obj);
    term_select_range (&GET_SELECTION(term)->start, where);
}


static void     scroll_selection_end_onscreen   (MooText    *obj)
{
    MooTerm *term = MOO_TERM (obj);
    GtkTextIter iter = GET_SELECTION(term)->end;
    guint top_line = term_top_line (term);

    if (ITER_ROW(&iter) < (int)top_line)
        moo_term_scroll_lines (term, ITER_ROW(&iter) - (int)top_line);
    else if (ITER_ROW(&iter) >= (int)top_line + (int)term->priv->height)
        moo_term_scroll_lines (term, ITER_ROW(&iter) + 1 -
                               (int)top_line - (int)term->priv->height);
}


static GdkWindow* get_window            (MooText            *obj)
{
    return GTK_WIDGET(obj)->window;
}


static void     get_visible_rect        (MooText            *obj,
                                         GdkRectangle       *rect)
{
    MooTerm *term = MOO_TERM (obj);
    int char_height = term_char_height (term);
    rect->x = 0;
    rect->width = term->priv->width * term_char_width (term);
    rect->y = term_top_line (term) * char_height;
    rect->height = term->priv->height * char_height;
}


gboolean    moo_term_cell_selected      (MooTerm    *term,
                                         guint       row,
                                         guint       col)
{
    GtkTextIter start, end, iter;

    if (get_selection_bounds (MOO_TEXT (term), &start, &end))
    {
        iter_order (&start, &end);
        FILL_ITER (&iter, term, row, col);
        return iter_cmp (&start, &iter) <= 0 && iter_cmp (&iter, &end) < 0;
    }
    else
    {
        return FALSE;
    }
}


int         moo_term_row_selected           (MooTerm    *term,
                                             guint       row)
{
    GtkTextIter start, end;

    if (get_selection_bounds (MOO_TEXT (term), &start, &end))
    {
        iter_order (&start, &end);

        if (ITER_ROW (&end) < (int)row || (int)row < ITER_ROW (&start))
            return NOT_SELECTED;
        else if (ITER_ROW (&start) < (int)row && (int)row < ITER_ROW (&end))
            return FULL_SELECTED;
        else
            return PART_SELECTED;
    }
    else
    {
        return FALSE;
    }
}


void        moo_term_text_iface_init        (gpointer        g_iface)
{
    MooTextIface *iface = (MooTextIface*) g_iface;

    iface->start_selection_dnd = NULL;

    iface->middle_button_click = middle_button_click;
    iface->right_button_click = right_button_click;

    iface->extend_selection = extend_selection;
    iface->window_to_buffer_coords = window_to_buffer_coords;
    iface->get_iter_at_location = get_iter_at_location;
    iface->get_selection_bounds = get_selection_bounds;
    iface->iter_order = iter_order;
    iface->iter_in_range = iter_in_range;
    iface->place_selection_end = place_selection_end;
    iface->select_range = select_range;
    iface->scroll_selection_end_onscreen = scroll_selection_end_onscreen;

    iface->get_window = get_window;
    iface->get_visible_rect = get_visible_rect;
}


char       *moo_term_selection_get_text (MooTerm    *term)
{
    return NULL;
}
