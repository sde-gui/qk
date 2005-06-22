/*
 *   mooterm/mooterm-priavte.h
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

#ifndef MOOTERM_MOOTERM_PRIVATE_H
#define MOOTERM_MOOTERM_PRIVATE_H

#include "mooterm/mooterm.h"
#include "mooterm/mootermbuffer-private.h"
#include "mooterm/mootermvt.h"

G_BEGIN_DECLS


#define ADJUSTMENT_PRIORITY         G_PRIORITY_DEFAULT_IDLE
#define ADJUSTMENT_VALUE_PRIORITY   G_PRIORITY_DEFAULT_IDLE
#define EXPOSE_PRIORITY             G_PRIORITY_DEFAULT_IDLE

#define MAX_TERMINAL_WIDTH          4096
#define DEFAULT_MONOSPACE_FONT      "Courier New 11"
#define DEFAULT_MONOSPACE_FONT2     "Monospace"


typedef enum {
    CURSOR_NONE     = 0,
    CURSOR_TEXT     = 1,
    CURSOR_POINTER  = 2,
    CURSORS_NUM     = 3
} TermCursorType;

enum {
    FOREGROUND = 0,
    BACKGROUND = 1
};

enum {
    NORMAL      = 0,
    SELECTED    = 1,
    CURSOR      = 2
};

typedef enum {
    CARET_BLOCK,
    CARET_UNDERLINE
} TermCaretShape;


typedef struct _TermPangoLines  TermPangoLines;
typedef struct _TermFontInfo    TermFontInfo;
typedef struct _TermSelection    TermSelection;


struct _MooTermPrivate {
    MooTermBuffer   *buffer;
    MooTermVt       *vt;

    gboolean         scrolled;
    gulong           _top_line;
//     gulong           old_scrollback;
//     gulong           old_width;
//     gulong           old_height;
    guint            char_width;
    guint            char_height;

    TermSelection   *selection;

    TermPangoLines  *pango_lines;
    TermFontInfo    *font_info;

    GdkGC           *fg[3];
    GdkGC           *bg[3];
    /* thing[MOO_TERM_COLOR_MAX] == NULL */
    GdkColor        *color[MOO_TERM_COLOR_MAX + 1];
    GdkGC           *pair[MOO_TERM_COLOR_MAX + 1][MOO_TERM_COLOR_MAX + 1];

    TermCaretShape   caret_shape;
    guint            caret_height;

    guint            pending_expose;
    GdkRegion       *dirty; /* pixel coordinates */

    GdkCursor       *cursor[CURSORS_NUM];
    GtkIMContext    *im;

    GtkAdjustment   *adjustment;
    guint            pending_adjustment_changed;
    guint            pending_adjustment_value_changed;
    guint            scrollback_changed_id;
    guint            width_changed_id;
    guint            height_changed_id;
    guint            buf_content_changed_id;
    guint            cursor_moved_id;
};


inline static gulong term_top_line (MooTerm *term)
{
    if (term->priv->scrolled)
        return term->priv->_top_line;
    else
        return buf_screen_offset (term->priv->buffer);
}

inline static gulong term_width (MooTerm *term)
{
    return buf_screen_width (term->priv->buffer);
}

inline static gulong term_height (MooTerm *term)
{
    return buf_screen_height (term->priv->buffer);
}


void        moo_term_buf_content_changed(MooTerm        *term);
void        moo_term_cursor_moved       (MooTerm        *term,
                                         gulong          old_row,
                                         gulong          old_col);

void        moo_term_size_changed       (MooTerm        *term);
void        moo_term_init_font_stuff    (MooTerm        *term);
void        moo_term_setup_palette      (MooTerm        *term);
void        moo_term_im_commit          (GtkIMContext   *imcontext,
                                         gchar          *arg,
                                         MooTerm        *term);

void        moo_term_force_update       (MooTerm        *term);
gboolean    moo_term_expose_event       (GtkWidget      *widget,
                                         GdkEventExpose *event);
void        moo_term_invalidate_rect    (MooTerm        *term,
                                         BufRectangle   *rect);

inline static void moo_term_invalidate_all  (MooTerm        *term)
{
    BufRectangle rec = {0, 0, term_width (term), term_height (term)};
    moo_term_invalidate_rect (term, &rec);
}


struct _TermPangoLines
{
    gulong           size;
    GPtrArray       *screen;
    GByteArray      *valid;
    PangoLayout     *line;
    PangoContext    *ctx;
};

inline static PangoLayout *term_pango_line  (MooTerm        *term,
                                             gulong          i)
{
    g_assert (i < term->priv->pango_lines->size);
    return (PangoLayout*) term->priv->pango_lines->screen->pdata[i];
}

TermPangoLines *term_pango_lines_new        (PangoContext   *ctx);

void        term_pango_lines_free           (TermPangoLines *lines);
void        term_pango_lines_resize         (MooTerm        *term,
                                             gulong          size);
void        term_pango_lines_set_font       (TermPangoLines *lines,
                                             PangoFontDescription *font);
gboolean    term_pango_lines_valid          (TermPangoLines *lines,
                                             gulong          row);
void        term_pango_lines_set_text       (TermPangoLines *lines,
                                             gulong          row,
                                             const char     *text,
                                             int             len);
void        term_pango_lines_invalidate     (MooTerm        *term,
                                             gulong          row);
void        term_pango_lines_invalidate_all (MooTerm        *term);


struct _TermFontInfo {
    PangoContext   *ctx;
    guint           width;
    guint           height;
    guint           ascent;
};

void             term_font_info_calculate   (TermFontInfo           *info);
void             term_font_info_set_font    (TermFontInfo           *info,
                                             PangoFontDescription   *font_desc);
TermFontInfo    *term_font_info_new         (PangoContext           *ctx);
void             term_font_info_free        (TermFontInfo           *info);


enum {
    NOT_SELECTED    = 0,
    FULL_SELECTED   = 1,
    PART_SELECTED   = 2
};

#define SELECT_SCROLL_NONE  (0)
#define SELECT_SCROLL_UP    (-1)
#define SELECT_SCROLL_DOWN  (1)

struct _TermSelection {
    gulong      screen_width;

    // absolute coordinates in the buffer
    // selected range is [(l_row, l_col), (r_row, r_col))
    // l_row, l_col and r_row are valid
    // r_col may be equal to _width
    gulong      l_row;
    gulong      l_col;
    gulong      r_row;
    gulong      r_col;
    gboolean    empty;

    gboolean    button_pressed;
    int         click;
    gboolean    drag;
    // buffer coordinates
    gulong      drag_begin_row;
    gulong      drag_begin_col;
    gulong      drag_end_row;
    gulong      drag_end_col;

    int scroll;
    guint st_id;
};


TermSelection   *term_selection_new         (void);
inline static void term_selection_free      (TermSelection *sel)
{
    g_free (sel);
}

void             term_set_selection         (MooTerm       *term,
                                             gulong         row1,
                                             gulong         col1,
                                             gulong         row2,
                                             gulong         col2);
void             term_selection_clear       (MooTerm       *term);

inline static void term_selection_set_width (MooTerm       *term,
                                             gulong         width)
{
    term->priv->selection->screen_width = width;
    term_selection_clear (term);
}

inline static int term_selection_row_selected (TermSelection *sel,
                                               gulong         row)
{
    if (sel->empty || sel->r_row < row || row < sel->l_row)
        return NOT_SELECTED;
    else if (sel->l_row < row && row < sel->r_row)
        return FULL_SELECTED;
    else
        return PART_SELECTED;
}

inline static gboolean term_selected           (TermSelection   *sel,
                                                gulong           row,
                                                gulong           col)
{
    if (sel->empty || sel->r_row < row || row < sel->l_row)
        return FALSE;
    else if (sel->l_row < row && row < sel->r_row)
        return TRUE;
    else if (sel->l_row == sel->r_row)
        return sel->l_col <= col && col < sel->r_col;
    else if (sel->l_row == row)
        return sel->l_col <= col;
    else
        return col < sel->r_row;
}


G_END_DECLS

#endif /* MOOTERM_MOOTERM_PRIVATE_H */
