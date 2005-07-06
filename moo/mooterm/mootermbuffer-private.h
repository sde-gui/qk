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


enum {
    KAM         = 1 << 0,   /* Keyboard Action Mode - ignored */
    IRM         = 1 << 1,   /* Insertion-Replacement Mode.
                               Set selects insert mode and turns INSERT on.
                               New display characters move old display
                               characters to the right. Characters moved
                               past the right margin are lost.
                               Reset selects replace mode and turns INSERT off.
                               New display characters replace old display
                               characters at cursor position. The old character
                               is erased.
                               aka insert mode */
    LNM         = 1 << 2,   /* Linefeed/New Line Mode.
                               Set causes a received linefeed, form feed, or
                               vertical tab to move cursor to first column of
                               next line. RETURN transmits both a carriage
                               return and linefeed.
                               Reset causes a received linefeed, form feed, or
                               vertical tab to move cursor to next line in
                               current column. RETURN transmits a carriage
                               return. */
    DECCKM      = 1 << 3,   /* Cursor Key Mode.
                               Set selects cursor keys to generate control
                               (application) functions.
                               Reset selects cursor keys to generate ANSI
                               cursor control sequences.*/
    DECANM      = 1 << 4,   /* ANSI/VT52 mode - ignored */
    DECCOLM     = 1 << 5,   /* Column Mode - ignored */
    DECSCLM     = 1 << 6,   /* Scroll Mode - ignored */
    DECSCNM     = 1 << 7,   /* Screen Mode.
                               Set selects reverse screen.
                               Reset selects normal screen*/
    DECOM       = 1 << 8,   /* Origin Mode.
                               Set selects home position in scrolling region.
                               Line numbers start at top margin of scrolling
                               region. The cursor cannot move out of scrolling
                               region.
                               Reset selects home position in upper-left corner
                               of screen. Line numbers are independent of the
                               scrolling region (absolute). */
    DECAWM      = 1 << 9,   /* Auto Wrap Mode.
                               Set selects auto wrap. Any display characters
                               received when cursor is at right margin appear
                               on next line. The display scrolls up if cursor
                               is at end of scrolling region.
                               Reset turns auto wrap off. Display characters
                               received when cursor is at right margin replace
                               previously displayed character.
                               aka am mode */
    DECARM      = 1 << 10,  /* Auto Repeat Mode - ignored */
    DECPFF      = 1 << 11,  /* Printer Form Feed Mode - ignored */
    DECPEX      = 1 << 12,  /* Printer Extent Mode - ignored */

    MOUSE_TRACKING          = 1 << 13,
    HILITE_MOUSE_TRACKING   = 1 << 14,

    KEYPAD_NUMERIC          = 1 << 15
};


/* LNM mode affects also data sent by RETURN */
struct _MooTermBufferPrivate {
    gboolean        constructed;

    GPtrArray      *lines; /* array of MooTermLine* */

    int             modes;
    int             saved_modes;
    MooTermTextAttr current_attr;
    gboolean        cursor_visible;

    guint           screen_offset;
    guint           screen_width;
    guint           screen_height;

    /* scrolling region - top and bottom rows */
    guint           top_margin;
    guint           bottom_margin;
    gboolean        scrolling_region_set;

    /* always absolute */
    guint           cursor_row;
    guint           cursor_col;

    /* used for save_cursor/restore_cursor */
    guint           saved_cursor_row;
    guint           saved_cursor_col;

    GList          *tab_stops;

    int             max_height;
    MooTermParser  *parser;

    BufRegion      *changed;
    gboolean        changed_all;

    gboolean        freeze_changed_notify;
    gboolean        freeze_cursor_notify;
};


inline static guint buf_screen_width (MooTermBuffer *buf)
{
    return buf->priv->screen_width;
}

inline static guint buf_screen_height (MooTermBuffer *buf)
{
    return buf->priv->screen_height;
}

inline static guint buf_screen_offset (MooTermBuffer *buf)
{
    return buf->priv->screen_offset;
}

inline static guint buf_total_height (MooTermBuffer *buf)
{
    return buf->priv->screen_height + buf->priv->screen_offset;
}

inline static MooTermLine *buf_line (MooTermBuffer *buf, guint i)
{
    g_assert (buf->priv->lines->len ==
            buf->priv->screen_height + buf->priv->screen_offset);
    g_assert (i < buf->priv->screen_height +
            buf->priv->screen_offset);

    return g_ptr_array_index (buf->priv->lines, i);
}

inline static MooTermLine *buf_screen_line (MooTermBuffer *buf, guint i)
{
    g_assert (i < buf->priv->screen_height);

    return g_ptr_array_index (buf->priv->lines,
                              i + buf->priv->screen_offset);
}

inline static guint buf_cursor_row_abs (MooTermBuffer *buf)
{
    return buf->priv->screen_offset + buf->priv->cursor_row;
}

inline static guint buf_cursor_row (MooTermBuffer *buf)
{
    return buf->priv->cursor_row;
}

inline static guint buf_cursor_col (MooTermBuffer *buf)
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
                                          guint          row,
                                          guint          start,
                                          guint          len)
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


inline static void _buf_add_lines               (MooTermBuffer *buf,
                                                 guint          num)
{
    guint i;

    for (i = 0; i < num; ++i)
    {
        g_ptr_array_add (buf->priv->lines,
                         term_line_new (buf->priv->screen_width));
        buf->priv->screen_offset++;
    }
}

inline static void _buf_delete_lines            (MooTermBuffer *buf,
                                                 guint          num)
{
    guint i;

    g_return_if_fail (num != 0 && num < buf->priv->screen_offset);

    for (i = 0; i < num; ++i)
        term_line_free (buf_line (buf, buf->priv->lines->len - i - 1));

    buf->priv->screen_offset -= num;

    g_ptr_array_set_size (buf->priv->lines, buf->priv->lines->len - num);
}


void    moo_term_buffer_bell            (MooTermBuffer  *buf);
void    moo_term_buffer_feed_child      (MooTermBuffer  *buf,
                                         const char     *string,
                                         int             len);
void    moo_term_buffer_full_reset      (MooTermBuffer  *buf);
void    moo_term_buffer_set_window_title(MooTermBuffer  *buf,
                                         const char     *title);
void    moo_term_buffer_set_icon_name   (MooTermBuffer  *buf,
                                         const char     *icon);
void    moo_term_buffer_set_font        (MooTermBuffer  *buf,
                                         const char     *font);

void    moo_term_buffer_cursor_move     (MooTermBuffer  *buf,
                                         int             rows,
                                         int             cols);
void    moo_term_buffer_cursor_move_to  (MooTermBuffer  *buf,
                                         int             row,
                                         int             col);

void    moo_term_buffer_set_keypad_numeric  (MooTermBuffer  *buf,
                                             gboolean        setting);

void    moo_term_buffer_index           (MooTermBuffer  *buf);
void    moo_term_buffer_new_line        (MooTermBuffer  *buf);
void    moo_term_buffer_erase_display   (MooTermBuffer  *buf);

void    moo_term_buffer_reset_tab_stops     (MooTermBuffer  *buf);
void    moo_term_buffer_clear_tab_stop      (MooTermBuffer  *buf);
void    moo_term_buffer_set_tab_stop        (MooTermBuffer  *buf);
guint   moo_term_buffer_next_tab_stop       (MooTermBuffer  *buf,
                                             guint           current);
guint   moo_term_buffer_prev_tab_stop       (MooTermBuffer  *buf,
                                             guint           current);


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


G_END_DECLS

#endif /* MOOTERM_MOOTERMBUFFER_PRIVATE_H */
