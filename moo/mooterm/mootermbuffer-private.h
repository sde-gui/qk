/*
 *   mootermbuffer-private.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOOTERM_COMPILATION
#error "This file may not be included"
#endif

#ifndef MOO_TERM_BUFFER_PRIVATE_H
#define MOO_TERM_BUFFER_PRIVATE_H

#include <gdk/gdkregion.h>
#include "mooterm/mootermbuffer.h"
#include "mooterm/mootermline.h"
#include "mooterm/mooterm-vt.h"

G_BEGIN_DECLS


typedef enum {
    CHARSET_DRAWING = 0,
    CHARSET_ACRSSS  = 1,
    CHARSET_ACRSPS  = 2,
    CHARSET_UK      = 3,
    CHARSET_ASCII   = 4
} CharsetType;


struct _MooTermBufferPrivate {
    gboolean        constructed;

    GPtrArray      *lines; /* array of MooTermLine* */
    MooTermTagTable *tag_table;
    GHashTable     *data_sets; /* MooTermLine* -> MooTermLine* */

    guint8          modes[MODE_MAX];
    MooTermTextAttr current_attr;

    int             single_shift;
    CharsetType     GL[4];
    CharsetType     current_graph_set;

    /* these are real position and
       dimensions of the screen */
    guint           screen_offset;
    guint           screen_width;
    guint           screen_height;

    /* scrolling region - top and bottom rows */
    guint           top_margin;
    guint           bottom_margin;
    gboolean        scrolling_region_set;

    /* independent of scrolling region */
    guint           _cursor_row;
    guint           _cursor_col; /* 0..width - it equals width if a character
                                    was inserted at the last column in AWM mode */

    GList          *tab_stops;

    GdkRegion      *changed;

    gboolean        freeze_changed_notify;
    gboolean        freeze_cursor_notify;
};


typedef enum {
    CLEAR_TAB_AT_CURSOR = 0,
    CLEAR_ALL_TABS      = 3
} ClearTabType;


struct _MooTermTagTable {
    MooTermBuffer *buffer;
    GHashTable *named_tags;
    GSList *tags;
};


MooTermTagTable *_moo_term_tag_table_new        (MooTermBuffer   *buffer);
void    _moo_term_tag_table_free                (MooTermTagTable *table);

void    _moo_term_buffer_set_line_data          (MooTermBuffer  *buf,
                                                 MooTermLine    *line,
                                                 const char     *key,
                                                 gpointer        data,
                                                 GDestroyNotify  destroy);
gpointer _moo_term_buffer_get_line_data         (MooTermBuffer  *buf,
                                                 MooTermLine    *line,
                                                 const char     *key);

void    _moo_term_buffer_changed                (MooTermBuffer  *buf);
void    _moo_term_buffer_scrollback_changed     (MooTermBuffer  *buf);
void    _moo_term_buffer_cursor_moved           (MooTermBuffer  *buf);

void    _moo_term_buffer_freeze_changed_notify  (MooTermBuffer  *buf);
void    _moo_term_buffer_freeze_cursor_notify   (MooTermBuffer  *buf);
void    _moo_term_buffer_thaw_changed_notify    (MooTermBuffer  *buf);
void    _moo_term_buffer_thaw_cursor_notify     (MooTermBuffer  *buf);

void    _moo_term_buffer_reset                  (MooTermBuffer  *buf);
void    _moo_term_buffer_soft_reset             (MooTermBuffer  *buf);

void    moo_term_buffer_set_mode                (MooTermBuffer  *buf,
                                                 guint           mode,
                                                 gboolean        val);

void    moo_term_buffer_set_screen_size         (MooTermBuffer  *buf,
                                                 guint           columns,
                                                 guint           rows);

void    _moo_term_buffer_cursor_move            (MooTermBuffer  *buf,
                                                 int             rows,
                                                 int             cols);
void    _moo_term_buffer_cursor_move_to         (MooTermBuffer  *buf,
                                                 int             row,
                                                 int             col);

void    _moo_term_buffer_clear_tab_stop         (MooTermBuffer  *buf,
                                                 ClearTabType    what);
void    _moo_term_buffer_set_tab_stop           (MooTermBuffer  *buf);

void    _moo_term_buffer_select_charset         (MooTermBuffer  *buf,
                                                 guint           set_num,
                                                 CharsetType     charset);
void    _moo_term_buffer_shift                  (MooTermBuffer  *buf,
                                                 guint           set);
void    _moo_term_buffer_single_shift           (MooTermBuffer  *buf,
                                                 guint           set);

void    _moo_term_buffer_set_scrolling_region   (MooTermBuffer  *buf,
                                                 guint           top_margin,
                                                 guint           bottom_margin);
void    _moo_term_buffer_set_ca_mode            (MooTermBuffer  *buf,
                                                 gboolean        set);


inline static guint buf_scrollback      (MooTermBuffer  *buf)
{
    if (buf_get_mode (MODE_CA))
        return 0;
    else
        return buf->priv->screen_offset;
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
    return buf->priv->_cursor_row;
}

inline static guint buf_cursor_row_abs  (MooTermBuffer  *buf)
{
    return buf->priv->_cursor_row + buf_scrollback (buf);
}

inline static guint buf_cursor_col_real (MooTermBuffer  *buf)
{
    return buf->priv->_cursor_col;
}

inline static guint buf_cursor_col_display (MooTermBuffer  *buf)
{
    return buf->priv->_cursor_col >= buf->priv->screen_width ?
            buf->priv->screen_width - 1 : buf->priv->_cursor_col;
}

inline static GdkRegion *buf_get_changed(MooTermBuffer  *buf)
{
    return buf->priv->changed;
}


#define buf_changed_add_rect(buf,rect)                              \
G_STMT_START {                                                      \
    if ((buf)->priv->changed)                                       \
        gdk_region_union_with_rect ((buf)->priv->changed, rect);    \
    else                                                            \
        (buf)->priv->changed = gdk_region_rectangle (rect);         \
} G_STMT_END

#define buf_changed_add_rect_dim(buf,x_,y_,w_,h_)                   \
G_STMT_START {                                                      \
    GdkRectangle rect__;                                            \
    rect__.x = x_;                                                  \
    rect__.y = y_;                                                  \
    rect__.width = w_;                                              \
    rect__.height = h_;                                             \
    buf_changed_add_rect (buf, &rect__);                            \
} G_STMT_END

#define buf_changed_add_range(buf, row, start, len)                 \
G_STMT_START {                                                      \
    GdkRectangle rec__;                                             \
    rec__.x = start;                                                \
    rec__.y = row;                                                  \
    rec__.width = len;                                              \
    rec__.height = 1;                                               \
    buf_changed_add_rect (buf, &rec__);                             \
} G_STMT_END

#define buf_changed_set_all(buf)                                    \
G_STMT_START {                                                      \
    GdkRectangle rec__ = {0, 0, 0, 0};                              \
    rec__.width = (buf)->priv->screen_width;                        \
    rec__.height = (buf)->priv->screen_height;                      \
    buf_changed_add_rect (buf, &rec__);                             \
} G_STMT_END


#define buf_set_attrs_mask(mask_)       buf->priv->current_attr.mask = (mask_)
#define buf_add_attrs_mask(mask_)       buf->priv->current_attr.mask |= (mask_)
#define buf_remove_attrs_mask(mask_)    buf->priv->current_attr.mask &= ~(mask_)


MooTermLine *_moo_term_buffer_get_line  (MooTermBuffer  *buf,
                                         guint           n);
#define buf_line _moo_term_buffer_get_line


inline static MooTermLine *buf_screen_line  (MooTermBuffer  *buf,
                                             guint           n)
{
    g_assert (n < buf->priv->screen_height);

    return g_ptr_array_index (buf->priv->lines,
                              n + buf->priv->screen_offset);
}


/*****************************************************************************/
/* Terminal stuff
 */

/* fixed values */
typedef enum {
    ERASE_FROM_CURSOR   = 0,
    ERASE_TO_CURSOR     = 1,
    ERASE_ALL           = 2
} EraseType;

void    _moo_term_buffer_new_line               (MooTermBuffer  *buf);
void    _moo_term_buffer_index                  (MooTermBuffer  *buf);
void    _moo_term_buffer_backspace              (MooTermBuffer  *buf);
void    _moo_term_buffer_tab                    (MooTermBuffer  *buf,
                                                 guint           n);
void    _moo_term_buffer_back_tab               (MooTermBuffer  *buf,
                                                 guint           n);
void    _moo_term_buffer_cuu                    (MooTermBuffer  *buf,
                                                 guint           n);
void    _moo_term_buffer_cud                    (MooTermBuffer  *buf,
                                                 guint           n);
void    _moo_term_buffer_cursor_next_line       (MooTermBuffer  *buf,
                                                 guint           n);
void    _moo_term_buffer_cursor_prev_line       (MooTermBuffer  *buf,
                                                 guint           n);
void    _moo_term_buffer_linefeed               (MooTermBuffer  *buf);
void    _moo_term_buffer_carriage_return        (MooTermBuffer  *buf);
void    _moo_term_buffer_reverse_index          (MooTermBuffer  *buf);
void    _moo_term_buffer_sgr                    (MooTermBuffer  *buf,
                                                 int            *args,
                                                 guint           args_len);
void    _moo_term_buffer_delete_char            (MooTermBuffer  *buf,
                                                 guint           n);
void    _moo_term_buffer_delete_line            (MooTermBuffer  *buf,
                                                 guint           n);
void    _moo_term_buffer_erase_char             (MooTermBuffer  *buf,
                                                 guint           n);
void    _moo_term_buffer_erase_in_display       (MooTermBuffer  *buf,
                                                 EraseType       what);
void    _moo_term_buffer_erase_in_line          (MooTermBuffer  *buf,
                                                 EraseType       what);
void    _moo_term_buffer_insert_char            (MooTermBuffer  *buf,
                                                 guint           n);
void    _moo_term_buffer_insert_line            (MooTermBuffer  *buf,
                                                 guint           n);

void    _moo_term_buffer_cup                    (MooTermBuffer  *buf,
                                                 guint           row,
                                                 guint           col);

void    _moo_term_buffer_decaln                 (MooTermBuffer  *buf);


G_END_DECLS

#endif /* MOO_TERM_BUFFER_PRIVATE_H */
