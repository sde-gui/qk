/*
 *   mooterm/mootermbuffer-private.h
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

#ifndef MOOTERM_MOOTERMBUFFER_VT_H
#define MOOTERM_MOOTERMBUFFER_VT_H

#include "mooterm/mootermbuffer-private.h"

G_BEGIN_DECLS


#define BUF_VT_DA_STRING            "\033[?1;2c"
#define BUF_VT_DECREQTPARM_STRING   "\033[3;1;7;120;120;1;0x"
#define BUF_VT_DSR_STRING           "\033[0n"
#define BUF_VT_CPR_STRING           "\033[%d;%dR"


inline static void buf_vt_bell              (MooTermBuffer  *buf)
{
    moo_term_buffer_bell (buf);
}


inline static void buf_vt_save_cursor       (MooTermBuffer  *buf)
{
    buf->priv->saved_cursor_row = buf_cursor_row (buf);
    buf->priv->saved_cursor_col = buf_cursor_col (buf);
}

inline static void buf_vt_restore_cursor    (MooTermBuffer  *buf)
{
    moo_term_buffer_cursor_move_to (buf,
                                    buf->priv->saved_cursor_row,
                                    buf->priv->saved_cursor_col);
}


inline static void buf_vt_set_window_title  (MooTermBuffer  *buf,
                                             const char     *string)
{
    moo_term_buffer_set_window_title (buf, string);
}


inline static void buf_vt_set_icon_name     (MooTermBuffer  *buf,
                                             const char     *string)
{
    moo_term_buffer_set_icon_name (buf, string);
}


inline static void buf_vt_full_reset        (MooTermBuffer  *buf)
{
    moo_term_buffer_full_reset (buf);
}


inline static void buf_vt_index             (MooTermBuffer  *buf)
{
    moo_term_buffer_index (buf);
}


inline static void buf_vt_reverse_index     (MooTermBuffer  *buf)
{
    guint cursor_row = buf_cursor_row (buf);

    if (cursor_row == 0)
    {
        if (buf_screen_offset (buf))
        {
            _buf_delete_lines (buf, 1);

            buf_changed_set_all (buf);

            moo_term_buffer_changed (buf);
            moo_term_buffer_scrollback_changed (buf);
        }
        else
        {
            buf_vt_bell (buf);
        }
    }
    else
    {
        moo_term_buffer_cursor_move (buf, -1, 0);
    }
}


inline static void buf_vt_next_line         (MooTermBuffer  *buf)
{
    moo_term_buffer_new_line (buf);
}


inline static void buf_vt_da                (MooTermBuffer  *buf)
{
    moo_term_buffer_feed_child (buf, BUF_VT_DA_STRING, -1);
}


inline static void buf_vt_ich               (MooTermBuffer  *buf,
                                             guint           num)
{
    guint cursor_col = buf_cursor_col (buf);
    guint cursor_row = buf_cursor_row (buf);
    guint screen_width = buf_screen_width (buf);

    BufRectangle changed = {cursor_col, cursor_row, screen_width - cursor_col, 1};

    if (!num)
        num = 1;
    if (num > screen_width - cursor_col)
        num = screen_width - cursor_col;

    term_line_insert_unichar (buf_screen_line (buf, cursor_row),
                              cursor_col,
                              EMPTY_CHAR,
                              num,
                              &buf->priv->current_attr,
                              screen_width);

    buf_changed_add_rectangle (buf, &changed);
    moo_term_buffer_changed (buf);
}


inline static void buf_vt_dch               (MooTermBuffer  *buf,
                                             guint           num)
{
    guint cursor_col = buf_cursor_col (buf);
    guint cursor_row = buf_cursor_row (buf);
    guint screen_width = buf_screen_width (buf);

    BufRectangle changed = {cursor_col, cursor_row, screen_width - cursor_col, 1};

    if (!num)
        num = 1;
    if (num > screen_width - cursor_col)
        num = screen_width - cursor_col;

    term_line_delete_range (buf_screen_line (buf, cursor_row),
                            cursor_col,
                            num);

    buf_changed_add_rectangle (buf, &changed);
    moo_term_buffer_changed (buf);
}


inline static void buf_vt_cuu               (MooTermBuffer  *buf,
                                             guint           num)
{
    moo_term_buffer_cursor_move (buf, -num, 0);
}

inline static void buf_vt_cud               (MooTermBuffer  *buf,
                                             guint           num)
{
    moo_term_buffer_cursor_move (buf, num, 0);
}

inline static void buf_vt_cuf               (MooTermBuffer  *buf,
                                             guint           num)
{
    moo_term_buffer_cursor_move (buf, 0, num);
}

inline static void buf_vt_cub               (MooTermBuffer  *buf,
                                             guint           num)
{
    moo_term_buffer_cursor_move (buf, 0, -num);
}


inline static void buf_vt_cursor_home       (MooTermBuffer  *buf)
{
    guint row = 0;

    if (buf->priv->scrolling_region_set && buf->priv->modes & DECOM)
            row = buf->priv->top_margin;

    moo_term_buffer_cursor_move_to (buf, row, 0);
}


inline static void buf_vt_cup               (MooTermBuffer  *buf,
                                             guint           row,
                                             guint           col)
{
    if (!row)
        row = 1;

    if (!col)
        col = 1;

    moo_term_buffer_cursor_move_to (buf, row - 1, col - 1);
}

inline static void buf_vt_column_address    (MooTermBuffer  *buf,
                                             guint           col)
{
    if (!col)
        col = 1;

    moo_term_buffer_cursor_move_to (buf, -1, col - 1);
}

inline static void buf_vt_row_address       (MooTermBuffer  *buf,
                                             guint           row)
{
    if (!row)
        row = 1;

    moo_term_buffer_cursor_move_to (buf, row - 1, -1);
}


inline static void buf_vt_erase_from_cursor (MooTermBuffer  *buf)
{
    guint cursor_col = buf_cursor_col (buf);
    guint cursor_row = buf_cursor_row (buf);
    guint screen_height = buf_screen_height (buf);
    guint screen_width = buf_screen_width (buf);
    guint i;

    BufRectangle changed = {cursor_col, cursor_row, screen_width - cursor_col, 1};

    term_line_erase_range (buf_screen_line (buf, cursor_row),
                           cursor_col,
                           screen_width);

    for (i = cursor_row + 1; i < screen_height; ++i)
        term_line_erase (buf_screen_line (buf, i));

    buf_changed_add_rectangle (buf, &changed);

    if (cursor_row + 1 < screen_height)
    {
        changed.x = 0;
        changed.width = screen_width;
        changed.y = cursor_row + 1;
        changed.height = screen_height - cursor_row - 1;
        buf_changed_add_rectangle (buf, &changed);
    }

    moo_term_buffer_changed (buf);
}


inline static void buf_vt_erase_to_cursor   (MooTermBuffer  *buf)
{
    guint cursor_col = buf_cursor_col (buf);
    guint cursor_row = buf_cursor_row (buf);
    guint screen_width = buf_screen_width (buf);
    guint i;

    BufRectangle changed = {0, cursor_row, cursor_col + 1, 1};

    for (i = 0; i < cursor_row; ++i)
        term_line_erase (buf_screen_line (buf, i));

    term_line_erase_range (buf_screen_line (buf, cursor_row),
                           0,
                           cursor_col + 1);

    buf_changed_add_rectangle (buf, &changed);

    if (cursor_row > 0)
    {
        changed.x = 0;
        changed.width = screen_width;
        changed.y = 0;
        changed.height = cursor_row;
        buf_changed_add_rectangle (buf, &changed);
    }

    moo_term_buffer_changed (buf);
}


inline static void buf_vt_erase_display     (MooTermBuffer  *buf)
{
    moo_term_buffer_erase_display (buf);
}


inline static void buf_vt_erase_line_from_cursor    (MooTermBuffer  *buf)
{
    guint cursor_col = buf_cursor_col (buf);
    guint cursor_row = buf_cursor_row (buf);
    guint screen_width = buf_screen_width (buf);

    BufRectangle changed = {cursor_col, cursor_row, screen_width - cursor_col, 1};

    term_line_erase_range (buf_screen_line (buf, cursor_row),
                           cursor_col,
                           screen_width);

    buf_changed_add_rectangle (buf, &changed);
    moo_term_buffer_changed (buf);
}


inline static void buf_vt_erase_line_to_cursor      (MooTermBuffer  *buf)
{
    guint cursor_col = buf_cursor_col (buf);
    guint cursor_row = buf_cursor_row (buf);

    BufRectangle changed = {0, cursor_row, cursor_col + 1, 1};

    term_line_erase_range (buf_screen_line (buf, cursor_row),
                           0,
                           cursor_col + 1);

    buf_changed_add_rectangle (buf, &changed);
    moo_term_buffer_changed (buf);
}


inline static void buf_vt_erase_line        (MooTermBuffer  *buf)
{
    guint cursor_row = buf_cursor_row (buf);
    guint screen_width = buf_screen_width (buf);

    BufRectangle changed = {0, cursor_row, screen_width, 1};

    term_line_erase (buf_screen_line (buf, cursor_row));

    buf_changed_add_rectangle (buf, &changed);
    moo_term_buffer_changed (buf);
}


inline static void buf_vt_request_terminal_parameters   (MooTermBuffer  *buf,
                                                         G_GNUC_UNUSED guint ignored)
{
    moo_term_buffer_feed_child (buf, BUF_VT_DECREQTPARM_STRING, -1);
}


inline static void buf_vt_backspace         (MooTermBuffer  *buf)
{
    buf_vt_cub (buf, 1);
}


inline static void buf_vt_linefeed          (MooTermBuffer  *buf)
{
    if (buf->priv->modes & LNM)
        buf_vt_next_line (buf);
    else
        buf_vt_index (buf);
}

inline static void buf_vt_vert_tab          (MooTermBuffer  *buf)
{
    buf_vt_linefeed (buf);
}

inline static void buf_vt_form_feed         (MooTermBuffer  *buf)
{
    buf_vt_linefeed (buf);
}


inline static void buf_vt_carriage_return   (MooTermBuffer  *buf)
{
    moo_term_buffer_cursor_move_to (buf, -1, 0);
}


inline static void buf_vt_report_status             (MooTermBuffer  *buf)
{
    moo_term_buffer_feed_child (buf, BUF_VT_DSR_STRING, -1);
}

inline static void buf_vt_report_active_position    (MooTermBuffer  *buf)
{
    char *cpr;

    if (buf->priv->modes & DECOM)
        cpr = g_strdup_printf (BUF_VT_CPR_STRING,
                               (guint) (buf_cursor_row (buf) - buf->priv->top_margin),
                               (guint) buf_cursor_col (buf));
    else
        cpr = g_strdup_printf (BUF_VT_CPR_STRING,
                               (guint) buf_cursor_row (buf),
                               (guint) buf_cursor_col (buf));

    moo_term_buffer_feed_child (buf, cpr, -1);

    g_free (cpr);
}


inline static void buf_vt_set_scrolling_region  (MooTermBuffer  *buf,
                                                 guint           first,
                                                 guint           last)
{
    guint height = buf_screen_height (buf);

    if (!first)
        first = 1;
    if (!last)
        last = height;
    else if (last > height)
        last = height;

    g_return_if_fail (first < last && first < height);

    first--;
    last--;

    buf->priv->top_margin = first;
    buf->priv->bottom_margin = last;

    if (first > 0 || last < height - 1)
        buf->priv->scrolling_region_set = TRUE;
    else
        buf->priv->scrolling_region_set = FALSE;

    g_object_notify (G_OBJECT (buf), "scrolling-region-set");
    buf_vt_cursor_home (buf);
}


inline static void buf_vt_init_hilite_mouse_tracking    (G_GNUC_UNUSED MooTermBuffer  *buf,
                                                         G_GNUC_UNUSED guint           func,
                                                         G_GNUC_UNUSED guint           startx,
                                                         G_GNUC_UNUSED guint           starty,
                                                         G_GNUC_UNUSED guint           firstrow,
                                                         G_GNUC_UNUSED guint           lastrow)
{
    g_message ("%s: implement me", G_STRLOC);
}


inline static void buf_vt_clear_tab_stop    (G_GNUC_UNUSED MooTermBuffer  *buf)
{
    moo_term_buffer_clear_tab_stop (buf);
}

inline static void buf_vt_clear_all_tab_stops   (G_GNUC_UNUSED MooTermBuffer  *buf)
{
    moo_term_buffer_reset_tab_stops (buf);
}

inline static void buf_vt_tab               (MooTermBuffer  *buf)
{
    moo_term_buffer_cursor_move_to (buf, -1,
                                    moo_term_buffer_next_tab_stop (buf,
                                            buf_cursor_col (buf)));
}

inline static void buf_vt_set_tab_stop      (G_GNUC_UNUSED MooTermBuffer  *buf)
{
    moo_term_buffer_set_tab_stop (buf);
}


inline static void buf_vt_select_charset    (G_GNUC_UNUSED MooTermBuffer  *buf,
                                             G_GNUC_UNUSED guint           set_num,
                                             G_GNUC_UNUSED guint           charset)
{
//     g_message ("%s: implement me", G_STRLOC);
}

inline static void buf_vt_invoke_charset    (G_GNUC_UNUSED MooTermBuffer  *buf,
                                             G_GNUC_UNUSED guint           charset)
{
//     g_message ("%s: implement me", G_STRLOC);
}

inline static void buf_vt_single_shift      (G_GNUC_UNUSED MooTermBuffer  *buf,
                                             G_GNUC_UNUSED guint           charset)
{
    g_message ("%s: implement me", G_STRLOC);
}


inline static void buf_vt_reset_2           (G_GNUC_UNUSED MooTermBuffer  *buf)
{
    g_message ("%s: implement me", G_STRLOC);
}


/* should respect scrolling region */
inline static void buf_vt_il                (MooTermBuffer  *buf,
                                             guint           num)
{
    guint cursor_row = buf_cursor_row (buf);
    guint bottom = buf->priv->bottom_margin;
    guint width = buf_screen_width (buf);
    guint screen_offset = buf->priv->screen_offset;
    BufRectangle changed = {0, cursor_row + 1, width, 0};

    if (!num)
        return;

    if (cursor_row < buf->priv->top_margin)
        return;

    if (cursor_row >= bottom)
        return;

    changed.height = bottom - cursor_row;

    if (num >= bottom - cursor_row)
    {
        guint i;
        for (i = cursor_row + 1; i <= bottom; ++i)
            term_line_erase (buf_screen_line (buf, i));
    }
    else
    {
        guint i;

        gpointer dest = &g_ptr_array_index (buf->priv->lines,
                                            screen_offset + cursor_row + num + 1);
        gpointer src = &g_ptr_array_index (buf->priv->lines,
                                           screen_offset + cursor_row + 1);

        for (i = 0; i < num; ++i)
            term_line_free (buf_screen_line (buf, bottom - i));

        memmove (dest, src, sizeof (MooTermLine*) * (bottom - cursor_row - 1));

        for (i = 0; i < num; ++i)
            g_ptr_array_index (buf->priv->lines,
                               screen_offset + cursor_row + 1 + i) =
                    term_line_new (width);
    }

    buf_changed_add_rectangle (buf, &changed);
    moo_term_buffer_changed (buf);
}


inline static void buf_vt_dl                (MooTermBuffer  *buf,
                                             guint           num)
{
    guint cursor_row = buf_cursor_row (buf);
    guint bottom = buf->priv->bottom_margin;
    guint width = buf_screen_width (buf);
    guint screen_offset = buf->priv->screen_offset;
    BufRectangle changed = {0, cursor_row, width, 0};

    if (!num)
        return;

    if (cursor_row < buf->priv->top_margin)
        return;

    if (cursor_row > bottom)
        return;

    changed.height = bottom - cursor_row + 1;

    /* TODO: it should preserve attributes in bottom lines */

    if (num >= bottom - cursor_row + 1)
    {
        guint i;
        for (i = 0; i < bottom - cursor_row + 1; ++i)
            term_line_free (buf_screen_line (buf, cursor_row + i));
    }
    else
    {
        guint i;

        gpointer dest = &g_ptr_array_index (buf->priv->lines,
                                            screen_offset + cursor_row);
        gpointer src = &g_ptr_array_index (buf->priv->lines,
                                           screen_offset + cursor_row + num);

        for (i = 0; i < num; ++i)
            term_line_free (buf_screen_line (buf, cursor_row + i));

        memmove (dest, src, sizeof (MooTermLine*) * (bottom - cursor_row + 1 - num));

        for (i = 0; i < num; ++i)
            g_ptr_array_index (buf->priv->lines, screen_offset + bottom - i) =
                    term_line_new (width);
    }

    buf_changed_add_rectangle (buf, &changed);
    moo_term_buffer_changed (buf);
}


inline static void buf_vt_decaln            (MooTermBuffer  *buf)
{
    guint i;
    guint height = buf_screen_height (buf);
    guint width = buf_screen_width (buf);

    for (i = 0; i < height; ++i)
        term_line_set_unichar (buf_screen_line (buf, i), 0,
                               'S', width,
                               &buf->priv->current_attr,
                               width);
}


G_END_DECLS

#endif /* MOOTERM_MOOTERMBUFFER_VT_H */
