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
#include "mooterm/mootermline.h"
#include "mooterm/mooterm-vt.h"

G_BEGIN_DECLS


struct _MooTermBufferPrivate {
    gboolean        constructed;

    GPtrArray      *lines; /* array of MooTermLine* */

    guint8          modes[MODE_MAX];
    MooTermTextAttr current_attr;

    int             single_shift;
    gunichar       *graph_sets[4];
    gunichar       *current_graph_set;
    gboolean        use_ascii_graphics;

    /* these are real position and
       dimensions of the screen */
    guint           _screen_offset;
    guint           screen_width;
    guint           screen_height;

    /* scrolling region - top and bottom rows */
    guint           top_margin;
    guint           bottom_margin;
    gboolean        scrolling_region_set;

    /* independent of scrolling region */
    guint           cursor_row;
    guint           cursor_col;

    /* TODO: is it per-terminal or per-buffer? */
    struct {
        guint           cursor_row, cursor_col;
        MooTermTextAttr attr;
        gunichar       *GL, *GR;
        gboolean        autowrap;
        gboolean        decom;
        guint           top_margin, bottom_margin;
        /* TODO: Selective erase attribute ??? */
        int             single_shift;
    }   saved;

    GList          *tab_stops;

    GdkRegion      *changed;
    gboolean        changed_all;

    gboolean        freeze_changed_notify;
    gboolean        freeze_cursor_notify;
};


void    moo_term_buffer_changed_clear           (MooTermBuffer  *buf);

void    moo_term_buffer_changed                 (MooTermBuffer  *buf);
void    moo_term_buffer_scrollback_changed      (MooTermBuffer  *buf);
void    moo_term_buffer_cursor_moved            (MooTermBuffer  *buf);
void    moo_term_buffer_feed_child              (MooTermBuffer  *buf,
                                                 const char     *string,
                                                 int             len);

void    moo_term_buffer_freeze_changed_notify   (MooTermBuffer  *buf);
void    moo_term_buffer_freeze_cursor_notify    (MooTermBuffer  *buf);
void    moo_term_buffer_thaw_changed_notify     (MooTermBuffer  *buf);
void    moo_term_buffer_thaw_cursor_notify      (MooTermBuffer  *buf);

void    moo_term_buffer_reset                   (MooTermBuffer  *buf);
void    moo_term_buffer_soft_reset              (MooTermBuffer  *buf);

void    moo_term_buffer_set_mode                (MooTermBuffer  *buf,
                                                 guint           mode,
                                                 gboolean        val);

void    moo_term_buffer_set_screen_width        (MooTermBuffer  *buf,
                                                 guint           columns);
void    moo_term_buffer_set_screen_height       (MooTermBuffer  *buf,
                                                 guint           rows);
void    moo_term_buffer_set_screen_size         (MooTermBuffer  *buf,
                                                 guint           columns,
                                                 guint           rows);

void    moo_term_buffer_cursor_move             (MooTermBuffer  *buf,
                                                 int             rows,
                                                 int             cols);
void    moo_term_buffer_cursor_move_to          (MooTermBuffer  *buf,
                                                 int             row,
                                                 int             col);

void    moo_term_buffer_reset_tab_stops         (MooTermBuffer  *buf);
guint   moo_term_buffer_next_tab_stop           (MooTermBuffer  *buf,
                                                 guint           current);
guint   moo_term_buffer_prev_tab_stop           (MooTermBuffer  *buf,
                                                 guint           current);
void    moo_term_buffer_clear_tab_stop          (MooTermBuffer  *buf,
                                                 int             what);
void    moo_term_buffer_set_tab_stop            (MooTermBuffer  *buf);

void    moo_term_buffer_select_charset          (MooTermBuffer  *buf,
                                                 guint           set_num,
                                                 guint           charset);
void    moo_term_buffer_shift                   (MooTermBuffer  *buf,
                                                 guint           set);
void    moo_term_buffer_single_shift            (MooTermBuffer  *buf,
                                                 guint           set);

void    moo_term_buffer_set_scrolling_region    (MooTermBuffer  *buf,
                                                 guint           top_margin,
                                                 guint           bottom_margin);
void    moo_term_buffer_set_ca_mode             (MooTermBuffer  *buf,
                                                 gboolean        set);


inline static guint buf_scrollback      (MooTermBuffer  *buf)
{
    if (buf_get_mode (MODE_CA))
        return 0;
    else
        return buf->priv->_screen_offset;
}

inline static guint buf_total_height    (MooTermBuffer  *buf)
{
    return buf_scrollback (buf) + buf->priv->screen_height;
}

inline static guint buf_screen_width    (MooTermBuffer  *buf)
{
    return buf->priv->screen_width;
}

inline static guint buf_screen_height    (MooTermBuffer  *buf)
{
    return buf->priv->screen_height;
}

inline static guint buf_cursor_row      (MooTermBuffer  *buf)
{
    return buf->priv->cursor_row;
}

inline static guint buf_cursor_row_abs  (MooTermBuffer  *buf)
{
    return buf->priv->cursor_row + buf_scrollback (buf);
}

inline static guint buf_cursor_col      (MooTermBuffer  *buf)
{
    return buf->priv->cursor_col;
}

inline static GdkRegion *buf_get_changed(MooTermBuffer  *buf)
{
    return buf->priv->changed;
}


#define buf_changed_add_rect(buf,rect)                              \
G_STMT_START {                                                      \
    if (!buf->priv->changed_all)                                    \
    {                                                               \
        if (buf->priv->changed)                                     \
            gdk_region_union_with_rect (buf->priv->changed, &rect); \
        else                                                        \
            buf->priv->changed = gdk_region_rectangle (&rect);      \
    }                                                               \
} G_STMT_END

#define buf_changed_add_range(buf, row, start, len)                 \
G_STMT_START {                                                      \
    if (!buf->priv->changed_all)                                    \
{                                                               \
        GdkRectangle rec = {start, row, len, 1};                    \
        buf_changed_add_rect (buf, rec);                            \
    }                                                               \
} G_STMT_END

#define buf_changed_set_all(buf)                                    \
G_STMT_START {                                                      \
    if (!buf->priv->changed_all)                                    \
    {                                                               \
        GdkRectangle rec = {                                        \
            0, 0, buf->priv->screen_width, buf->priv->screen_height \
        };                                                          \
        buf_changed_add_rect (buf, rec);                            \
        buf->priv->changed_all = TRUE;                              \
    }                                                               \
} G_STMT_END


#define buf_set_attrs_mask(mask_)       buf->priv->current_attr.mask = (mask_)
#define buf_add_attrs_mask(mask_)       buf->priv->current_attr.mask |= (mask_)
#define buf_remove_attrs_mask(mask_)    buf->priv->current_attr.mask &= ~(mask_)

#define buf_set_ansi_foreground(color)                              \
G_STMT_START {                                                      \
    if ((color) < MOO_TERM_COLOR_MAX)                               \
    {                                                               \
        buf->priv->current_attr.mask |= MOO_TERM_TEXT_FOREGROUND;   \
        buf->priv->current_attr.foreground = (color);               \
    }                                                               \
    else                                                            \
    {                                                               \
        buf->priv->current_attr.mask &= ~MOO_TERM_TEXT_FOREGROUND;  \
    }                                                               \
} G_STMT_END

#define buf_set_ansi_background(color)                              \
G_STMT_START {                                                      \
    if ((color) < MOO_TERM_COLOR_MAX)                               \
    {                                                               \
        buf->priv->current_attr.mask |= MOO_TERM_TEXT_BACKGROUND;   \
        buf->priv->current_attr.background = (color);               \
    }                                                               \
    else                                                            \
    {                                                               \
        buf->priv->current_attr.mask &= ~MOO_TERM_TEXT_BACKGROUND;  \
    }                                                               \
} G_STMT_END


inline static MooTermLine *buf_line         (MooTermBuffer  *buf,
                                             guint           n)
{
    if (buf_get_mode (MODE_CA))
    {
        g_assert (n < buf->priv->screen_height);
        return g_ptr_array_index (buf->priv->lines,
                                  n + buf->priv->_screen_offset);
    }
    else
    {
        g_assert (n < buf->priv->_screen_offset + buf->priv->screen_height);
        return g_ptr_array_index (buf->priv->lines, n);
    }
}

inline static MooTermLine *buf_screen_line  (MooTermBuffer  *buf,
                                             guint           n)
{
    g_assert (n < buf->priv->screen_height);

    return g_ptr_array_index (buf->priv->lines,
                              n + buf->priv->_screen_offset);
}


/*****************************************************************************/
/* Terminal stuff
 */

void    moo_term_buffer_new_line                (MooTermBuffer  *buf);
void    moo_term_buffer_index                   (MooTermBuffer  *buf);
void    moo_term_buffer_backspace               (MooTermBuffer  *buf);
void    moo_term_buffer_tab                     (MooTermBuffer  *buf,
                                                 guint           n);
void    moo_term_buffer_back_tab                (MooTermBuffer  *buf,
                                                 guint           n);
void    moo_term_buffer_cuu                     (MooTermBuffer  *buf,
                                                 guint           n);
void    moo_term_buffer_cud                     (MooTermBuffer  *buf,
                                                 guint           n);
void    moo_term_buffer_cursor_next_line        (MooTermBuffer  *buf,
                                                 guint           n);
void    moo_term_buffer_cursor_prev_line        (MooTermBuffer  *buf,
                                                 guint           n);
void    moo_term_buffer_linefeed                (MooTermBuffer  *buf);
void    moo_term_buffer_carriage_return         (MooTermBuffer  *buf);
void    moo_term_buffer_reverse_index           (MooTermBuffer  *buf);
void    moo_term_buffer_sgr                     (MooTermBuffer  *buf,
                                                 int            *args,
                                                 guint           args_len);
void    moo_term_buffer_delete_char             (MooTermBuffer  *buf,
                                                 guint           n);
void    moo_term_buffer_delete_line             (MooTermBuffer  *buf,
                                                 guint           n);
void    moo_term_buffer_erase_char              (MooTermBuffer  *buf,
                                                 guint           n);
void    moo_term_buffer_erase_in_display        (MooTermBuffer  *buf,
                                                 guint           what);
void    moo_term_buffer_erase_in_line           (MooTermBuffer  *buf,
                                                 guint           what);
void    moo_term_buffer_insert_char             (MooTermBuffer  *buf,
                                                 guint           n);
void    moo_term_buffer_insert_line             (MooTermBuffer  *buf,
                                                 guint           n);

void    moo_term_buffer_erase_range             (MooTermBuffer  *buf,
                                                 guint           row,
                                                 guint           col,
                                                 guint           len);

void    moo_term_buffer_cup                     (MooTermBuffer  *buf,
                                                 guint           row,
                                                 guint           col);

void    moo_term_buffer_clear_saved             (MooTermBuffer  *buf);
void    moo_term_buffer_decsc                   (MooTermBuffer  *buf);
void    moo_term_buffer_decrc                   (MooTermBuffer  *buf);

void    moo_term_buffer_decaln                  (MooTermBuffer  *buf);


G_END_DECLS

#endif /* MOOTERM_MOOTERMBUFFER_PRIVATE_H */
