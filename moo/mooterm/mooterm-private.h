/*
 *   mooterm/mooterm-private.h
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
#include "mooterm/mootermpt.h"

G_BEGIN_DECLS


#define ADJUSTMENT_PRIORITY         G_PRIORITY_DEFAULT_IDLE
#define ADJUSTMENT_VALUE_PRIORITY   G_PRIORITY_DEFAULT_IDLE
#define EXPOSE_PRIORITY             G_PRIORITY_DEFAULT_IDLE

#define PT_WRITER_PRIORITY          G_PRIORITY_DEFAULT_IDLE
#define PT_READER_PRIORITY          G_PRIORITY_HIGH_IDLE

#define MAX_TERMINAL_WIDTH          4096
#define DEFAULT_MONOSPACE_FONT      "Courier New 9"
#define DEFAULT_MONOSPACE_FONT2     "Monospace"


typedef enum {
    CURSOR_NONE     = 0,
    CURSOR_TEXT     = 1,
    CURSOR_POINTER  = 2,
    CURSORS_NUM     = 3
} TermCursorType;

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
    MooTermPt       *pt;

    gboolean         scrolled;
    guint            _top_line;
    guint            char_width;
    guint            char_height;

    TermSelection   *selection;

    TermFontInfo    *font_info;

    GdkPixmap       *back_pixmap;
    GdkRegion       *changed_content; /* buffer coordinates, relative to top_line */
    GdkGC           *clip;
    gboolean         font_changed;
    PangoLayout     *layout;

    GdkGC           *fg[MOO_TERM_COLOR_MAX + 1][3];
    GdkGC           *bg[MOO_TERM_COLOR_MAX + 1][3];

    TermCaretShape   caret_shape;
    guint            caret_height;

    guint            pending_expose;
    GdkRegion       *dirty; /* pixel coordinates */

    GdkCursor       *cursor[CURSORS_NUM];
    GtkIMContext    *im;

    gboolean         scroll_on_keystroke;

    GtkAdjustment   *adjustment;
    guint            pending_adjustment_changed;
    guint            pending_adjustment_value_changed;
    guint            buf_scrollback_changed_id;
    guint            buf_width_changed_id;
    guint            buf_height_changed_id;
    guint            buf_content_changed_id;
    guint            buf_cursor_moved_id;
    guint            buf_feed_child_id;
};


inline static guint term_top_line (MooTerm *term)
{
    if (term->priv->scrolled)
        return term->priv->_top_line;
    else
        return buf_screen_offset (term->priv->buffer);
}

inline static guint term_width (MooTerm *term)
{
    return buf_screen_width (term->priv->buffer);
}

inline static guint term_height (MooTerm *term)
{
    return buf_screen_height (term->priv->buffer);
}


void        moo_term_buf_content_changed(MooTerm        *term);
void        moo_term_cursor_moved       (MooTerm        *term,
                                         guint           old_row,
                                         guint           old_col);

void        moo_term_size_changed       (MooTerm        *term);
void        moo_term_init_font_stuff    (MooTerm        *term);
void        moo_term_setup_palette      (MooTerm        *term);
void        moo_term_im_commit          (GtkIMContext   *imcontext,
                                         gchar          *arg,
                                         MooTerm        *term);

gboolean    moo_term_button_press       (GtkWidget      *widget,
                                         GdkEventButton *event);
gboolean    moo_term_button_release     (GtkWidget      *widget,
                                         GdkEventButton *event);
gboolean    moo_term_key_press          (GtkWidget      *widget,
                                         GdkEventKey    *event);
gboolean    moo_term_key_release        (GtkWidget      *widget,
                                         GdkEventKey    *event);

void        moo_term_init_back_pixmap       (MooTerm        *term);
void        moo_term_resize_back_pixmap     (MooTerm        *term);
void        moo_term_update_back_pixmap     (MooTerm        *term);
void        moo_term_invalidate_content_all (MooTerm        *term);
void        moo_term_invalidate_content_rect(MooTerm        *term,
                                             GdkRectangle   *rect);

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


/*************************************************************************/
/* TermSelection
 */

enum {
    NOT_SELECTED    = 0,
    FULL_SELECTED   = 1,
    PART_SELECTED   = 2
};

#define SELECT_SCROLL_NONE  (0)
#define SELECT_SCROLL_UP    (-1)
#define SELECT_SCROLL_DOWN  (1)

struct _TermSelection {
    guint       screen_width;

    // absolute coordinates in the buffer
    // selected range is [(l_row, l_col), (r_row, r_col))
    // l_row, l_col and r_row are valid
    // r_col may be equal to _width
    guint       l_row;
    guint       l_col;
    guint       r_row;
    guint       r_col;
    gboolean    empty;

    gboolean    button_pressed;
    int         click;
    gboolean    drag;
    // buffer coordinates
    guint       drag_begin_row;
    guint       drag_begin_col;
    guint       drag_end_row;
    guint       drag_end_col;

    int scroll;
    guint st_id;
};


TermSelection   *term_selection_new         (void);
inline static void term_selection_free      (TermSelection *sel)
{
    g_free (sel);
}

void             term_set_selection         (MooTerm       *term,
                                             guint          row1,
                                             guint          col1,
                                             guint          row2,
                                             guint          col2);
void             term_selection_clear       (MooTerm       *term);

inline static void term_selection_set_width (MooTerm       *term,
                                             guint          width)
{
    term->priv->selection->screen_width = width;
    term_selection_clear (term);
}

inline static int term_selection_row_selected (TermSelection *sel,
                                               guint          row)
{
    if (sel->empty || sel->r_row < row || row < sel->l_row)
        return NOT_SELECTED;
    else if (sel->l_row < row && row < sel->r_row)
        return FULL_SELECTED;
    else
        return PART_SELECTED;
}

inline static gboolean term_selected           (TermSelection   *sel,
                                                guint            row,
                                                guint            col)
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


/*************************************************************************/
/* MooTermPtPrivate
 */

enum {
    PT_NONE = 0,
    PT_READ,
    PT_WRITE
};


struct _MooTermPtPrivate {
    MooTermBuffer   *buffer;

    GQueue          *pending_write;  /* list->data is GByteArray* */
    guint            pending_write_id;
};


inline static void pt_discard (GSList **list)
{
    GSList *l;

    for (l = *list; l != NULL; ++l)
        g_byte_array_free (l->data, TRUE);

    g_slist_free (*list);
    *list = NULL;
}

inline static void pt_flush_pending_write (MooTermPt *pt)
{
    GList *l;

    for (l = pt->priv->pending_write->head; l != NULL; l = l->next)
        g_byte_array_free (l->data, TRUE);

    while (!g_queue_is_empty (pt->priv->pending_write))
        g_queue_pop_head (pt->priv->pending_write);
}

inline static void pt_add_data (GSList **list, const char *data, gssize len)
{
    if (data && len && (len > 0 || data[0]))
    {
        GByteArray *ar;

        if (len < 0)
            len = strlen (data);

        ar = g_byte_array_sized_new ((guint) len);
        *list = g_slist_append (*list,
                                 g_byte_array_append (ar, data, len));
    }
}


G_END_DECLS

#endif /* MOOTERM_MOOTERM_PRIVATE_H */
