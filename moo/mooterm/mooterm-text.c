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
#define ITER_WIDTH(iter)            ((iter)->dummy5)
#define ITER_TERM(iter)             ((MooTerm*)(iter)->dummy1)
#define ITER_SET_TERM(iter, term)   (iter)->dummy1 = term
#define ITER_BUF(iter)              (ITER_TERM(iter)->priv->buffer)
#define ITER_TOTAL_HEIGHT(iter)     ((int)(ITER_TERM(iter)->priv->height + \
                                     buf_scrollback (ITER_BUF(iter))))

#define FILL_ITER(iter, term, row, col)     \
    (iter)->dummy1 = term;                  \
    ITER_ROW(iter) = row;                   \
    ITER_COL(iter) = col;                   \
    ITER_WIDTH(iter) = term->priv->width;   \
    CHECK_ITER(iter);


#ifdef DEBUG
static void CHECK_ITER (const GtkTextIter *iter)
{
    g_assert (ITER_TERM(iter) != NULL);
    g_assert ((int)ITER_TERM(iter)->priv->width == ITER_WIDTH(iter));
    g_assert (ITER_ROW(iter) >= 0);
    g_assert (ITER_ROW(iter) < ITER_TOTAL_HEIGHT(iter));
    g_assert (ITER_COL(iter) >= 0);
    g_assert (ITER_COL(iter) <= ITER_WIDTH(iter));
}

static void CHECK_SEGMENT (const Segment *segment)
{
    CHECK_ITER (&segment->start);
    CHECK_ITER (&segment->end);
}
#else
#define CHECK_ITER(iter)
#define CHECK_SEGMENT(segment)
#endif


static int      iter_cmp            (const GtkTextIter  *first,
                                     const GtkTextIter  *second);
static void     iter_order          (GtkTextIter        *first,
                                     GtkTextIter        *second);
static void     iter_set_start      (GtkTextIter        *iter);

static char    *segment_get_text    (Segment            *segment);
static void     invalidate_segment  (Segment            *segments,
                                     guint               num);


gpointer    moo_term_selection_new  (MooTerm    *term)
{
    Segment *sel = g_new0 (Segment, 1);

    FILL_ITER (&sel->start, term, 0, 0);
    FILL_ITER (&sel->end, term, 0, 0);

    return sel;
}


void        moo_term_selection_invalidate   (MooTerm    *term)
{
    Segment *sel = term->priv->selection;

    invalidate_segment (sel, 1);
    FILL_ITER (&sel->start, term, 0, 0);
    FILL_ITER (&sel->end, term, 0, 0);

    moo_term_release_selection (term);
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
    Segment new_sel;
    Segment old_selection;
    gboolean inv = FALSE;
    MooTerm *term = ITER_TERM (start);

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

    iter_order (&new_sel.start, &new_sel.end);

    if (ITER_COL(&new_sel.start) == ITER_WIDTH(&new_sel.start))
    {
        g_assert (ITER_ROW(&new_sel.start) < ITER_TOTAL_HEIGHT(&new_sel.start));
        ITER_ROW(&new_sel.start)++;
        ITER_COL(&new_sel.start) = 0;
    }

    if (ITER_COL(&new_sel.end) == 0 && ITER_ROW(&new_sel.end) != 0)
    {
        ITER_ROW(&new_sel.end)--;
        ITER_COL(&new_sel.end) = ITER_WIDTH(&new_sel.start);
    }

    if (inv)
        iter_order (&new_sel.end, &new_sel.start);

    CHECK_SEGMENT (&new_sel);

    *GET_SELECTION (ITER_TERM (start)) = new_sel;

    invalidate_segment (diff,
                        segment_sym_diff (&new_sel,
                                           &old_selection,
                                           diff));

    if (moo_term_selection_empty (term))
        moo_term_release_selection (term);
    else
        moo_term_grab_selection (term);
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


gboolean    moo_term_get_selection_bounds   (MooTerm    *term,
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
        GtkTextIter start = selection->start;
        GtkTextIter end = selection->end;

        iter_order (&start, &end);

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
    CHECK_ITER (iter);
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
    CHECK_ITER (iter);
    CHECK_ITER (start);
    CHECK_ITER (end);
    return iter_cmp (start, iter) <= 0 && iter_cmp (iter, end) <= 0;
}


static void     select_range            (G_GNUC_UNUSED MooText *obj,
                                         const GtkTextIter  *start,
                                         const GtkTextIter  *end)
{
    CHECK_ITER (start);
    CHECK_ITER (end);
    term_select_range (start, end);
}


static void     place_selection_end     (MooText            *obj,
                                         const GtkTextIter  *where)
{
    MooTerm *term = MOO_TERM (obj);
    CHECK_ITER (where);
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
    guint l_row, l_col, r_row, r_col;

    if (moo_term_get_selection_bounds (term, &l_row, &l_col,
                                        &r_row, &r_col))
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


static gboolean extend_selection        (MooText        *obj,
                                         MooTextSelectionType type,
                                         GtkTextIter    *end,
                                         GtkTextIter    *start);

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
    return segment_get_text (term->priv->selection);
}


static int      char_class              (MooTerm            *term,
                                         const GtkTextIter  *iter);
static gboolean iter_ends_line          (const GtkTextIter  *iter);
static gboolean iter_starts_line        (const GtkTextIter  *iter);
static void     iter_forward_char       (GtkTextIter        *iter);
static void     iter_backward_char      (GtkTextIter        *iter);
static void     iter_set_line_offset    (GtkTextIter        *iter,
                                         guint               offset);
static gboolean iter_forward_line       (GtkTextIter        *iter);
static gboolean is_space                (const GtkTextIter  *iter);
static gboolean is_word_char            (const GtkTextIter  *iter);
static gunichar iter_get_char           (const GtkTextIter  *iter);


static gboolean extend_selection        (MooText        *obj,
                                         MooTextSelectionType type,
                                         GtkTextIter    *end,
                                         GtkTextIter    *start)
{
    MooTerm *term = MOO_TERM (obj);
    int order = iter_cmp (start, end);

    CHECK_ITER (start);
    CHECK_ITER (end);

    if (type == MOO_TEXT_SELECT_CHARS)
    {
        return order;
    }

    if (order > 0)
    {
        GtkTextIter *tmp = start;
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

    g_assert_not_reached ();
}


static int      char_class              (MooTerm            *term,
                                         const GtkTextIter  *iter)
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


static gboolean iter_ends_line          (const GtkTextIter  *iter)
{
    return ITER_COL(iter) == (int)ITER_TERM(iter)->priv->width;
}


static gboolean iter_starts_line        (const GtkTextIter  *iter)
{
    return ITER_COL(iter) == 0;
}


static void     iter_forward_char       (GtkTextIter        *iter)
{
    MooTerm *term = ITER_TERM(iter);
    int width = term->priv->width;
    int total_height = term->priv->height + buf_scrollback (term->priv->buffer);

    g_return_if_fail (ITER_ROW(iter) < total_height || ITER_COL(iter) < width);

    if (ITER_COL(iter) < width)
    {
        ITER_COL(iter)++;
    }
    else
    {
        ITER_COL(iter) = 0;
        ITER_ROW(iter)++;
    }
}


static void     iter_backward_char      (GtkTextIter        *iter)
{
    MooTerm *term = ITER_TERM(iter);
    int width = term->priv->width;

    g_return_if_fail (ITER_ROW(iter) > 0 || ITER_COL(iter) > 0);

    if (ITER_COL(iter) > 0)
    {
        ITER_COL(iter)--;
    }
    else
    {
        ITER_ROW(iter)--;
        ITER_COL(iter) = width;
    }
}


static void     iter_set_line_offset    (GtkTextIter        *iter,
                                         guint               offset)
{
    g_return_if_fail (offset <= ITER_TERM(iter)->priv->width);
    ITER_COL(iter) = offset;
}


static gboolean iter_forward_line       (GtkTextIter        *iter)
{
    MooTerm *term = ITER_TERM(iter);
    int width = term->priv->width;
    int total_height = term->priv->height + buf_scrollback (term->priv->buffer);

    if (ITER_ROW(iter) == total_height)
    {
        ITER_COL(iter) = width;
        return FALSE;
    }
    else
    {
        ITER_COL(iter) = 0;
        ITER_ROW(iter)++;
        return TRUE;
    }
}


static gboolean is_space                (const GtkTextIter  *iter)
{
    gunichar c = iter_get_char (iter);
    g_return_val_if_fail (c != 0, FALSE);
    return g_unichar_isspace (c);
}


static gboolean is_word_char            (const GtkTextIter  *iter)
{
    gunichar c = iter_get_char (iter);
    g_return_val_if_fail (c != 0, FALSE);
    return g_unichar_isalnum (c) || c == '_';
}


static gunichar iter_get_char           (const GtkTextIter  *iter)
{
    MooTermLine *line = buf_line (ITER_TERM(iter)->priv->buffer,
                                  ITER_ROW(iter));
    return moo_term_line_get_unichar (line, ITER_COL(iter));
}


static char    *segment_get_text    (Segment            *segment)
{
    MooTerm *term = ITER_TERM (&segment->start);
    MooTermBuffer *buf = term->priv->buffer;
    GtkTextIter start = segment->start;
    GtkTextIter end = segment->end;
    GString *text;
    MooTermLine *line;
    int width = term->priv->width;
    int i;

    iter_order (&start, &end);

    if (!iter_cmp (&start, &end))
        return NULL;

    text = g_string_new ("");

    if (ITER_ROW(&start) == ITER_ROW(&end))
    {
        line = buf_line (buf, ITER_ROW(&start));
        for (i = ITER_COL(&start); i < ITER_COL(&end); ++i)
            g_string_append_unichar (text, moo_term_line_get_unichar (line, i));
    }
    else
    {
        if (ITER_COL(&start) < width)
        {
            line = buf_line (buf, ITER_ROW(&start));
            for (i = ITER_COL(&start); i < width; ++i)
                g_string_append_unichar (text, moo_term_line_get_unichar (line, i));
            g_string_append_c (text, '\n');
        }

        if (ITER_ROW(&start) + 1 < ITER_ROW(&end))
        {
            for (i = ITER_ROW(&start) + 1; i < ITER_ROW(&end); ++i)
            {
                int j;
                line = buf_line (buf, i);
                for (j = 0; j < width; ++j)
                    g_string_append_unichar (text, moo_term_line_get_unichar (line, j));
                g_string_append_c (text, '\n');
            }
        }

        if (ITER_COL(&end) > 0)
        {
            line = buf_line (buf, ITER_ROW(&end));
            for (i = 0; i < ITER_COL(&start); ++i)
                g_string_append_unichar (text, moo_term_line_get_unichar (line, i));
        }
    }

    return g_string_free (text, FALSE);
}
