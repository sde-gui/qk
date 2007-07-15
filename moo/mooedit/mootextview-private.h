/*
 *   mootextview-private.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOOEDIT_COMPILATION
#error "Do not include this file"
#endif

#ifndef MOO_TEXT_VIEW_PRIVATE_H
#define MOO_TEXT_VIEW_PRIVATE_H

#include "mooedit/mootextview.h"
#include "mooedit/mootextsearch.h"
#include "mooutils/moohistorylist.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS


/***********************************************************************/
/* GtkTextView stuff
 */
void        _moo_text_view_move_cursor          (GtkTextView        *text_view,
                                                 GtkMovementStep     step,
                                                 gint                count,
                                                 gboolean            extend_selection);
void        _moo_text_view_page_horizontally    (GtkTextView        *text_view,
                                                 int                 count,
                                                 gboolean            extend_selection);
void        _moo_text_view_delete_from_cursor   (GtkTextView        *text_view,
                                                 GtkDeleteType       type,
                                                 gint                count);
int         _moo_text_view_key_press_event      (GtkWidget          *widget,
                                                 GdkEventKey        *event);
int         _moo_text_view_key_release_event    (GtkWidget          *widget,
                                                 GdkEventKey        *event);
int         _moo_text_view_button_press_event   (GtkWidget          *widget,
                                                 GdkEventButton     *event);
int         _moo_text_view_button_release_event (GtkWidget          *widget,
                                                 GdkEventButton     *event);
int         _moo_text_view_motion_event         (GtkWidget          *widget,
                                                 GdkEventMotion     *event);
int         _moo_text_view_extend_selection     (MooTextView        *view,
                                                 MooTextSelectionType type,
                                                 GtkTextIter        *insert,
                                                 GtkTextIter        *selection_bound);

void        _moo_text_view_check_char_inserted  (MooTextView        *view);
void        _moo_text_view_pend_cursor_blink    (MooTextView        *view);
int         _moo_text_view_get_line_height      (MooTextView        *view);
void        _moo_text_view_set_line_numbers_font (MooTextView       *view,
                                                 const char         *name);


typedef enum {
    MOO_TEXT_VIEW_DRAG_NONE = 0,
    MOO_TEXT_VIEW_DRAG_SELECT,
    MOO_TEXT_VIEW_DRAG_DRAG,
    MOO_TEXT_VIEW_DRAG_SELECT_LINES
} MooTextViewDragType;

typedef enum {
    MOO_TEXT_VIEW_POS_LEFT,
    MOO_TEXT_VIEW_POS_RIGHT,
    MOO_TEXT_VIEW_POS_TOP,
    MOO_TEXT_VIEW_POS_BOTTOM,
    MOO_TEXT_VIEW_POS_INVALID
} MooTextViewPos;

typedef enum {
    MOO_TEXT_VIEW_COLOR_CURRENT_LINE,
    MOO_TEXT_VIEW_COLOR_RIGHT_MARGIN,
    MOO_TEXT_VIEW_N_COLORS
} MooTextViewColor;

struct _MooTextViewPrivate {
    gboolean constructed;

    GdkRectangle *update_rectangle;
    gboolean in_expose;
    guint update_idle;

    /* Clipboard */
    gboolean manage_clipboard;

    /* Overwrite mode cursor */
    gboolean overwrite_mode;
    gboolean saved_cursor_visible;
    gboolean cursor_visible;
    guint blink_timeout;

    /***********************************************************************/
    /* Drawing
     */
    MooTextStyleScheme *style_scheme;
    guint tab_width;
    gboolean color_settings[MOO_TEXT_VIEW_N_COLORS];
    char *colors[MOO_TEXT_VIEW_N_COLORS];
    GdkGC *gcs[MOO_TEXT_VIEW_N_COLORS];
    guint right_margin_offset;
    int right_margin_pixel_offset;
    gboolean draw_tabs;
    gboolean draw_trailing_spaces;
    gboolean highlight_matching_brackets;
    gboolean highlight_mismatching_brackets;

    struct {
        gboolean show_icons;
        int icon_width;
        gboolean show_numbers;
        int digit_width;
        int numbers_width;
        PangoFontDescription *numbers_font;
        gboolean show_folds;
        int fold_width;
    } lm;

    int n_lines;
    guint update_n_lines_idle;

    gboolean enable_folding;
    GSList *line_marks;

    /***********************************************************************/
    /* Search
     */
    int last_search_stamp;
    GtkTextMark *last_found_start, *last_found_end;

    /***********************************************************************/
    /* Indentation
     */
    MooIndenter *indenter;
    gboolean backspace_indents;
    gboolean enter_indents;
    MooTextTabKeyAction tab_key_action;

    /***********************************************************************/
    /* Keyboard
     */
    gboolean smart_home_end;
    gboolean ctrl_up_down_scrolls;
    gboolean ctrl_page_up_down_scrolls;
    /* key press handler sets this flag in order to distinguish typed in
       characters in buffer's insert-text signal */
    gboolean in_key_press;
    gunichar char_inserted;
    int char_inserted_offset;

    /***********************************************************************/
    /* Selection and drag
     */
    struct {
        guint           scroll_timeout;
        GdkEventType    button;
        MooTextViewDragType type;
        int             start_x; /* buffer coordinates */
        int             start_y; /* buffer coordinates */
        GtkTextMark    *start_mark;
        guint           moved                           : 1;
        guint           double_click_selects_brackets   : 1;
        guint           double_click_selects_inside     : 1;
    } dnd;

    /***********************************************************************/
    /* Drag'n'drop from outside
     */
    gboolean drag_inside;
    gboolean drag_drop;
    GtkTargetList *targets;
    GtkTextMark *dnd_mark;

    /***********************************************************************/
    /* Children
     */
    GtkWidget *children[4];
    GSList *boxes;

    /***********************************************************************/
    /* Search
     */
    struct {
        gboolean enable;
        gboolean in_search;
        GtkWidget *evbox;
        GtkWidget *entry;
        GtkToggleButton *case_sensitive;
        GtkToggleButton *regex;
        MooTextSearchFlags flags;
    } qs;
};

enum {
    DND_TARGET_TEXT = 1
};


G_END_DECLS

#endif /* MOO_TEXT_VIEW_PRIVATE_H */
