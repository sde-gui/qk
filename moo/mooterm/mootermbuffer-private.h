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

#ifndef MOOTERM_MOOTERMBUFFER_PRIVATE_H
#define MOOTERM_MOOTERMBUFFER_PRIVATE_H

#ifndef MOOTERM_COMPILATION
#error "This file may not be included directly"
#endif

#include <gdk/gdkregion.h>
#include "mooterm/mootermbuffer.h"
#include "mooterm/mootermparser.h"
#include "mooterm/mootermline.h"

G_BEGIN_DECLS

#define BufRectangle                GdkRectangle
#define BufRegion                   GdkRegion
#define BUF_OVERLAP_RECTANGLE_IN    GDK_OVERLAP_RECTANGLE_IN
#define BUF_OVERLAP_RECTANGLE_OUT   GDK_OVERLAP_RECTANGLE_OUT
#define BUF_OVERLAP_RECTANGLE_PART  GDK_OVERLAP_RECTANGLE_PART
#define buf_region_new              gdk_region_new
#define buf_region_destroy          gdk_region_destroy
#define buf_region_copy             gdk_region_copy
#define buf_region_rectangle        gdk_region_rectangle
#define buf_region_destroy          gdk_region_destroy
#define buf_region_get_clipbox      gdk_region_get_clipbox
#define buf_region_get_rectangles   gdk_region_get_rectangles
#define buf_region_empty            gdk_region_empty
#define buf_region_equal            gdk_region_equal
#define buf_region_point_in         gdk_region_point_in
#define buf_region_rect_in          gdk_region_rect_in
#define buf_region_offset           gdk_region_offset
#define buf_region_union_with_rect  gdk_region_union_with_rect
#define buf_region_intersect        gdk_region_intersect
#define buf_region_union            gdk_region_union
#define buf_region_subtract         gdk_region_subtract
#define buf_region_xor              gdk_region_xor


struct _MooTermBufferPrivate {
    gboolean        constructed;

    GArray         *lines; /* array of MooTermLine */

    MooTermTextAttr current_attr;
    gboolean        cursor_visible;
    gboolean        secure;
    gboolean        insert_mode;
    gboolean        am_mode;

    gulong          screen_offset;
    gulong          screen_width;
    gulong          screen_height;

    gulong          cursor_row;
    gulong          cursor_col;

    /* used for save_cursor/restore_cursor */
    gulong          saved_cursor_row;
    gulong          saved_cursor_col;

    glong           max_height;
    MooTermParser  *parser;

    BufRegion      *changed;
    gboolean        changed_all;

    gboolean        freeze_changed_notify;
    gboolean        freeze_cursor_notify;
};


inline static gulong buf_screen_width (MooTermBuffer *buf)
{
    return buf->priv->screen_width;
}

inline static gulong buf_screen_height (MooTermBuffer *buf)
{
    return buf->priv->screen_height;
}

inline static gulong buf_screen_offset (MooTermBuffer *buf)
{
    return buf->priv->screen_offset;
}

inline static gulong buf_total_height (MooTermBuffer *buf)
{
    return buf->priv->screen_height + buf->priv->screen_offset;
}

inline static MooTermLine *buf_line (MooTermBuffer *buf, gulong i)
{
    return &g_array_index (buf->priv->lines, MooTermLine, i);
}

inline static MooTermLine *buf_screen_line (MooTermBuffer *buf, gulong line)
{
    g_assert (line < buf->priv->screen_height);
    return &g_array_index (buf->priv->lines, MooTermLine,
                           line + buf->priv->screen_offset);
}

inline static gulong buf_cursor_row_abs (MooTermBuffer *buf)
{
    return buf->priv->screen_offset + buf->priv->cursor_row;
}

inline static gulong buf_cursor_row (MooTermBuffer *buf)
{
    return buf->priv->cursor_row;
}

inline static gulong buf_cursor_col (MooTermBuffer *buf)
{
    return buf->priv->cursor_col;
}

inline static BufRegion *buf_get_changed (MooTermBuffer *buf)
{
    return buf->priv->changed;
}

inline static void buf_changed_add_rectangle (MooTermBuffer *buf,
                                              BufRectangle  *rect)
{
    if (!buf->priv->changed_all)
    {
        if (!buf->priv->changed)
            buf->priv->changed = buf_region_rectangle (rect);
        else
            buf_region_union_with_rect (buf->priv->changed, rect);
    }
}

inline static void buf_changed_set_all (MooTermBuffer *buf)
{
    if (!buf->priv->changed_all)
    {
        BufRectangle changed = {
            0, 0, buf->priv->screen_width,
            buf->priv->screen_height
        };

        buf->priv->changed_all = TRUE;

        if (buf->priv->changed)
            buf_region_destroy (buf->priv->changed);

        buf->priv->changed = buf_region_rectangle (&changed);
    }
}

inline static void buf_changed_add_range (MooTermBuffer *buf,
                                          gulong         row,
                                          gulong         start,
                                          gulong         len)
{
    BufRectangle rec = {start, row, len, 1};
    buf_changed_add_rectangle (buf, &rec);
}

inline static void buf_changed_clear (MooTermBuffer *buf)
{
    if (buf->priv->changed)
    {
        buf_region_destroy (buf->priv->changed);
        buf->priv->changed = NULL;
    }

    buf->priv->changed_all = FALSE;
}

inline static char *buf_screen_get_text (MooTermBuffer *buf)
{
    guint i;
    GString *s = g_string_new ("");

    for (i = 0; i < buf_screen_height (buf); ++i)
    {
        g_string_append_len (s, term_line_chars (buf_screen_line (buf, i)),
                             term_line_len (buf_screen_line (buf, i)));
        g_string_append_c (s, '\n');
    }

    return g_string_free (s, FALSE);
}

inline static gboolean buf_cursor_visible (MooTermBuffer *buf)
{
    return buf->priv->cursor_visible;
}

inline static void buf_freeze_changed_notify    (MooTermBuffer *buf)
{
    buf->priv->freeze_changed_notify = TRUE;
}

inline static void buf_freeze_cursor_notify     (MooTermBuffer *buf)
{
    buf->priv->freeze_cursor_notify = TRUE;
}

inline static void buf_thaw_changed_notify      (MooTermBuffer *buf)
{
    buf->priv->freeze_changed_notify = FALSE;
}

inline static void buf_thaw_cursor_notify       (MooTermBuffer *buf)
{
    buf->priv->freeze_cursor_notify = FALSE;
}


void moo_term_buffer_set_window_title   (MooTermBuffer  *buf,
                                         const char     *title);
void moo_term_buffer_set_icon_name      (MooTermBuffer  *buf,
                                         const char     *icon);


/***************************************************************************/

typedef enum {
    ANSI_ALL_ATTRS_OFF = 0,
    ANSI_BOLD          = 1,
    ANSI_UNDERSCORE    = 4,
    ANSI_BLINK         = 5,
    ANSI_REVERSE       = 7,
    ANSI_CONCEALED     = 8,

    ANSI_FORE_BLACK    = 30,
    ANSI_FORE_RED      = 31,
    ANSI_FORE_GREEN    = 32,
    ANSI_FORE_YELLOW   = 33,
    ANSI_FORE_BLUE     = 34,
    ANSI_FORE_MAGENTA  = 35,
    ANSI_FORE_CYAN     = 36,
    ANSI_FORE_WHITE    = 37,

    ANSI_BACK_BLACK    = 40,
    ANSI_BACK_RED      = 41,
    ANSI_BACK_GREEN    = 42,
    ANSI_BACK_YELLOW   = 43,
    ANSI_BACK_BLUE     = 44,
    ANSI_BACK_MAGENTA  = 45,
    ANSI_BACK_CYAN     = 46,
    ANSI_BACK_WHITE    = 47
} AnsiTextAttr;


#define buf_bell            moo_term_buffer_bell
#define buf_flash_screen    moo_term_buffer_flash_screen
#define buf_plain_chars     moo_term_buffer_print_chars
#define buf_cursor_move_to  moo_term_buffer_cursor_move_to


inline static void buf_set_attrs_mask   (MooTermBuffer      *buf,
                                         MooTermTextAttrMask mask)
{
    buf->priv->current_attr.mask = mask;
}

inline static void buf_add_attrs_mask   (MooTermBuffer      *buf,
                                         MooTermTextAttrMask mask)
{
    buf->priv->current_attr.mask =
            (MooTermTextAttrMask)(buf->priv->current_attr.mask | mask);
}

inline static void buf_remove_attrs_mask(MooTermBuffer      *buf,
                                         MooTermTextAttrMask mask)
{
    buf->priv->current_attr.mask =
            (MooTermTextAttrMask)(buf->priv->current_attr.mask & ~mask);
}

inline static void buf_set_secure_mode  (MooTermBuffer      *buf,
                                         gboolean            secure)
{
    buf->priv->secure = secure;
}

inline static void buf_set_ansi_foreground  (MooTermBuffer      *buf,
                                             MooTermBufferColor  color)
{
    if (color < 8)
    {
        buf_add_attrs_mask (buf, MOO_TERM_TEXT_FOREGROUND);
        buf->priv->current_attr.foreground = color;
    }
    else
    {
        buf_remove_attrs_mask(buf, MOO_TERM_TEXT_FOREGROUND);
    }
}

inline static void buf_set_ansi_background  (MooTermBuffer      *buf,
                                             MooTermBufferColor  color)
{
    if (color < 8)
    {
        buf_add_attrs_mask (buf, MOO_TERM_TEXT_BACKGROUND);
        buf->priv->current_attr.background = color;
    }
    else
    {
        buf_remove_attrs_mask(buf, MOO_TERM_TEXT_BACKGROUND);
    }
}

inline static void buf_enter_reverse_mode       (MooTermBuffer  *buf)
{
    buf_add_attrs_mask (buf, MOO_TERM_TEXT_REVERSE);
}

inline static void buf_exit_reverse_mode        (MooTermBuffer  *buf)
{
    buf_remove_attrs_mask (buf, MOO_TERM_TEXT_REVERSE);
}

inline static void buf_cursor_invisible         (MooTermBuffer  *buf)
{
    moo_term_buffer_set_cursor_visible (buf, FALSE);
}

inline static void buf_cursor_normal            (MooTermBuffer  *buf)
{
    moo_term_buffer_set_cursor_visible (buf, TRUE);
}

inline static void buf_ena_acs                  (G_GNUC_UNUSED MooTermBuffer  *buf)
{
}

inline static void buf_enter_alt_charset_mode   (MooTermBuffer  *buf)
{
    buf_add_attrs_mask (buf, MOO_TERM_TEXT_ALTERNATE);
}

inline static void buf_exit_alt_charset_mode    (MooTermBuffer  *buf)
{
    buf_remove_attrs_mask (buf, MOO_TERM_TEXT_ALTERNATE);
}

inline static void buf_enter_secure_mode        (MooTermBuffer  *buf)
{
    buf_set_secure_mode (buf, TRUE);
}

inline static void buf_orig_pair                (MooTermBuffer  *buf)
{
    buf_set_ansi_foreground (buf, MOO_TERM_COLOR_NONE);
    buf_set_ansi_background (buf, MOO_TERM_COLOR_NONE);
}

inline static void buf_enter_underline_mode     (MooTermBuffer  *buf)
{
    buf_add_attrs_mask (buf, MOO_TERM_TEXT_UNDERLINE);
}

inline static void buf_exit_underline_mode      (MooTermBuffer  *buf)
{
    buf_remove_attrs_mask (buf, MOO_TERM_TEXT_UNDERLINE);
}

inline static void buf_set_background           (MooTermBuffer  *buf,
                                                 guint           d)
{
    g_assert (40 <= d && d <= 47);
    buf_set_ansi_background (buf, (MooTermBufferColor)(d - 40));
}

inline static void buf_set_foreground           (MooTermBuffer  *buf,
                                                 guint           d)
{
    g_assert (30 <= d && d <= 37);
    buf_set_ansi_foreground (buf, (MooTermBufferColor)(d - 30));
}

inline static void buf_set_tab                  (G_GNUC_UNUSED MooTermBuffer  *buf)
{
    g_warning ("%s: implement me", G_STRLOC);
}

inline static gulong buf_compute_next_tab_stop  (G_GNUC_UNUSED MooTermBuffer  *buf,
                                                 gulong          col)
{
    return ((col >> 3) << 3) + 8;
}

inline static gulong buf_compute_prev_tab_stop  (G_GNUC_UNUSED MooTermBuffer  *buf,
                                                 gulong          col)
{
    if (!col)
        return col;
    else if (col & 7)
        return (col >> 3) << 3;
    else
        return col - 8;
}

inline static void buf_tab                      (MooTermBuffer  *buf)
{
    buf_cursor_move_to (buf, -1,
                        buf_compute_next_tab_stop (buf, buf_cursor_col (buf)));
}

inline static void buf_back_tab                 (MooTermBuffer  *buf)
{
    buf_cursor_move_to (buf, -1,
                        buf_compute_prev_tab_stop (buf, buf_cursor_col (buf)));
}

inline static void buf_carriage_return          (MooTermBuffer  *buf)
{
    buf_cursor_move_to (buf, -1, 0);
}

inline static void buf_cursor_home              (MooTermBuffer  *buf)
{
    buf_cursor_move_to (buf, 0, 0);
}

inline static void buf_cursor_move              (MooTermBuffer  *buf,
                                                 long            rows,
                                                 long            cols)
{
    long width = buf_screen_width (buf);
    long height = buf_screen_height (buf);
    long cursor_row = buf_cursor_row (buf);
    long cursor_col = buf_cursor_col (buf);

    if (rows && cursor_row + rows >= 0 && cursor_row + rows < height)
        cursor_row += rows;
    if (cols && cursor_col + cols >= 0 && cursor_col + cols < width)
        cursor_col += cols;

    buf_cursor_move_to (buf, cursor_row, cursor_col);
}


inline static void buf_line_feed                (MooTermBuffer  *buf)
{
    gulong cursor_row = buf_cursor_row (buf);
    gulong height = buf_screen_height (buf);
    gulong width = buf_screen_width (buf);

    if (cursor_row < height - 1)
    {
        buf_cursor_move_to (buf, cursor_row + 1, 0);
    }
    else
    {
        MooTermLine new_line;

        term_line_init (&new_line, width);
        g_array_append_val (buf->priv->lines, new_line);
        buf->priv->screen_offset++;

        buf_changed_set_all (buf);
        buf_cursor_move_to (buf, -1, 0);

        moo_term_buffer_changed (buf);
        moo_term_buffer_scrollback_changed (buf);
    }
}


inline static void buf_clear_screen             (MooTermBuffer  *buf)
{
    gulong i;
    gulong height = buf_screen_height (buf);

    for (i = 0; i < height; ++i)
        term_line_erase (buf_screen_line (buf, i));
    buf_cursor_home (buf);
    buf_changed_set_all (buf);
    moo_term_buffer_changed (buf);
}

inline static void buf_clr_eos                  (MooTermBuffer  *buf)
{
    gulong i;
    BufRectangle changed = {
        0, buf_cursor_row (buf), buf_screen_width (buf),
        buf_screen_height (buf) - buf_cursor_row (buf)
    };

    for (i = (gulong)changed.y; i < (gulong)changed.height; ++i)
        term_line_erase (buf_screen_line (buf, i));
    buf_changed_add_rectangle (buf, &changed);
    moo_term_buffer_changed (buf);
}

inline static void buf_clr_bol                  (MooTermBuffer  *buf)
{
    BufRectangle changed = {0, buf_cursor_row (buf), buf_cursor_col (buf) + 1, 1};
    term_line_erase_range (buf_screen_line (buf, changed.y),
                           0, changed.width,
                           &buf->priv->current_attr);
    buf_changed_add_rectangle (buf, &changed);
    moo_term_buffer_changed (buf);
}

inline static void buf_clr_eol                  (MooTermBuffer  *buf)
{
    BufRectangle changed = {
        buf_cursor_col (buf), buf_cursor_row (buf),
        buf_screen_width (buf) - buf_cursor_col (buf), 1
    };

    term_line_erase_range (buf_screen_line (buf, changed.y),
                           changed.x, changed.width,
                           &buf->priv->current_attr);
    buf_changed_add_rectangle (buf, &changed);
    moo_term_buffer_changed (buf);
}

inline static void buf_parm_dch                 (MooTermBuffer  *buf,
                                                 guint           d)
{
    BufRectangle changed = {
        buf_cursor_col (buf), buf_cursor_row (buf),
        buf_screen_width (buf) - buf_cursor_col (buf), 1
    };

    term_line_delete_range (buf_screen_line (buf, changed.y),
                           changed.x, d);
    buf_changed_add_rectangle (buf, &changed);
    moo_term_buffer_changed (buf);
}

inline static void buf_delete_character         (MooTermBuffer  *buf)
{
    buf_parm_dch (buf, 1);
}

inline static void buf_parm_delete_line         (MooTermBuffer  *buf,
                                                 guint           d)
{
    gulong row = buf_cursor_row (buf);
    gulong height = buf_screen_height (buf);

    if (row < height - 1)
    {
        if (d > height - 1 - row)
            d = height - 1 - row;
    }
    else
    {
        d = 0;
    }

    if (d)
    {
        BufRectangle changed = {
            0, row + 1,
            buf_screen_width (buf), height - row - 1
        };
        guint i;

        for (i = row + 1; i < row + d; ++i)
            term_line_destroy (buf_screen_line (buf, i));

        g_array_remove_range (buf->priv->lines,
                              row + 1 + buf->priv->screen_offset,
                              d);

        for (i = 0; i < d; ++i)
        {
            MooTermLine new_line;
            term_line_init (&new_line, buf->priv->screen_width);
            g_array_append_val (buf->priv->lines, new_line);
        }

        buf_changed_add_rectangle (buf, &changed);
        moo_term_buffer_changed (buf);
    }
}

inline static void buf_delete_line              (MooTermBuffer  *buf)
{
    buf_parm_delete_line (buf, 1);
}

inline static void buf_erase_chars              (MooTermBuffer  *buf,
                                                 guint           d)
{
    BufRectangle changed = {
        buf_cursor_col (buf),
        buf_cursor_row (buf), d, 1
    };

    if (d > buf_screen_width (buf) - changed.x)
    {
        d = buf_screen_width (buf) - changed.x;
        changed.width = d;
    }

    term_line_erase_range (buf_screen_line (buf, changed.y),
                           changed.x, changed.width,
                           &buf->priv->current_attr);
    buf_changed_add_rectangle (buf, &changed);
    moo_term_buffer_changed (buf);
}

inline static void buf_parm_ich                 (MooTermBuffer  *buf,
                                                 guint           d)
{
    BufRectangle changed = {
        buf_cursor_col (buf), buf_cursor_row (buf),
        buf_screen_width (buf) - buf_cursor_col (buf), 1
    };

    term_line_insert_chars (buf_screen_line (buf, changed.y),
                            changed.x,
                            EMPTY_CHAR,
                            d,
                            buf_screen_width (buf),
                            &buf->priv->current_attr);
    buf_changed_add_rectangle (buf, &changed);
    moo_term_buffer_changed (buf);
}

inline static void buf_insert_lines             (MooTermBuffer  *buf,
                                                 guint           pos,
                                                 guint           num)
{
    gulong height = buf_screen_height (buf);
    gulong total_height = buf_total_height (buf);
    guint i;

    BufRectangle changed = {
        0, pos,
        buf_screen_width (buf), height - pos
    };

    if (pos < height)
    {
        if (num > height - pos)
            num = height - pos;
    }
    else
    {
        return;
    }

    for (i = 0; i < num; ++i)
    {
        MooTermLine new_line;
        term_line_init (&new_line, buf->priv->screen_width);
        g_array_insert_val (buf->priv->lines,
                            pos, new_line);
    }

    for (i = total_height; i < total_height + num; ++i)
        term_line_destroy (buf_line (buf, i));

    g_array_remove_range (buf->priv->lines,
                          total_height, num);

    buf_changed_add_rectangle (buf, &changed);
    moo_term_buffer_changed (buf);
}

inline static void buf_parm_insert_line         (MooTermBuffer  *buf,
                                                 guint           d)
{
    buf_insert_lines (buf, buf_cursor_row (buf) + 1, d);
}

inline static void buf_insert_line              (MooTermBuffer  *buf)
{
    buf_insert_lines (buf, buf_cursor_row (buf) + 1, 1);
}

inline static void buf_scroll_forward           (MooTermBuffer  *buf)
{
    buf_line_feed (buf);
}

/* apparently it's not just 'cursor down' */
inline static void buf_parm_down_cursor         (MooTermBuffer  *buf,
                                                 guint           num)
{
    guint i;
    for (i = 0; i < num; ++i)
        buf_line_feed (buf);
}

inline static void buf_cursor_down              (MooTermBuffer  *buf)
{
    buf_line_feed (buf);
}

inline static void buf_scroll_reverse           (MooTermBuffer  *buf)
{
    buf_insert_lines (buf, 0, 1);
}

inline static void buf_print_screen             (G_GNUC_UNUSED MooTermBuffer  *buf)
{
    g_warning ("%s: implement me", G_STRLOC);
}

inline static void buf_prtr_off                 (G_GNUC_UNUSED MooTermBuffer  *buf)
{
    g_warning ("%s: implement me", G_STRLOC);
}

inline static void buf_prtr_on                  (G_GNUC_UNUSED MooTermBuffer  *buf)
{
    g_warning ("%s: implement me", G_STRLOC);
}


inline static void buf_restore_cursor           (MooTermBuffer  *buf)
{
    buf_cursor_move_to (buf,
                        buf->priv->saved_cursor_row,
                        buf->priv->saved_cursor_col);
}

inline static void buf_save_cursor              (MooTermBuffer  *buf)
{
    buf->priv->saved_cursor_row = buf_cursor_row (buf);
    buf->priv->saved_cursor_col = buf_cursor_col (buf);
}

inline static void buf_enter_am_mode            (MooTermBuffer  *buf)
{
    moo_term_buffer_set_am_mode (buf, TRUE);
}

inline static void buf_exit_am_mode             (MooTermBuffer  *buf)
{
    moo_term_buffer_set_am_mode (buf, FALSE);
}

inline static void buf_enter_ca_mode            (G_GNUC_UNUSED MooTermBuffer  *buf)
{
}
inline static void buf_exit_ca_mode             (G_GNUC_UNUSED MooTermBuffer  *buf)
{
}

inline static void buf_keypad_local             (G_GNUC_UNUSED MooTermBuffer  *buf)
{
    g_warning ("%s: implement me", G_STRLOC);
}
inline static void buf_keypad_xmit              (G_GNUC_UNUSED MooTermBuffer  *buf)
{
    g_warning ("%s: implement me", G_STRLOC);
}

inline static void buf_enter_insert_mode        (MooTermBuffer  *buf)
{
    moo_term_buffer_set_insert_mode (buf, TRUE);
}

inline static void buf_exit_insert_mode         (MooTermBuffer  *buf)
{
    moo_term_buffer_set_insert_mode (buf, FALSE);
}

inline static void buf_clear_all_tabs           (G_GNUC_UNUSED MooTermBuffer  *buf)
{
    g_warning ("%s: implement me", G_STRLOC);
}

inline static void buf_change_scroll_region     (G_GNUC_UNUSED MooTermBuffer  *buf,
                                                 G_GNUC_UNUSED gulong          row1,
                                                 G_GNUC_UNUSED gulong          row2)
{
    g_warning ("%s: implement me", G_STRLOC);
}

inline static void buf_user6                    (G_GNUC_UNUSED MooTermBuffer  *buf,
                                                 G_GNUC_UNUSED gulong          row,
                                                 G_GNUC_UNUSED gulong          col)
{
    g_warning ("%s: implement me", G_STRLOC);
}
inline static void buf_user7                    (G_GNUC_UNUSED MooTermBuffer  *buf)
{
    g_warning ("%s: implement me", G_STRLOC);
}
inline static void buf_user8                    (G_GNUC_UNUSED MooTermBuffer  *buf)
{
    g_warning ("%s: implement me", G_STRLOC);
}
inline static void buf_user9                    (G_GNUC_UNUSED MooTermBuffer  *buf)
{
    g_warning ("%s: implement me", G_STRLOC);
}

inline static void buf_init_2string             (G_GNUC_UNUSED MooTermBuffer  *buf)
{
    g_warning ("%s: implement me", G_STRLOC);
}

inline static void buf_reset_2string            (G_GNUC_UNUSED MooTermBuffer  *buf)
{
    g_warning ("%s: implement me", G_STRLOC);
}

inline static void buf_reset_1string            (G_GNUC_UNUSED MooTermBuffer  *buf)
{
    g_warning ("%s: implement me", G_STRLOC);
}

inline static void buf_esc_l                    (G_GNUC_UNUSED MooTermBuffer  *buf)
{
    g_warning ("%s: implement me", G_STRLOC);
}

inline static void buf_esc_m                    (G_GNUC_UNUSED MooTermBuffer  *buf)
{
    g_warning ("%s: implement me", G_STRLOC);
}


G_END_DECLS

#endif /* MOOTERM_MOOTERMBUFFER_PRIVATE_H */
