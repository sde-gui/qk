/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mootextview-private.h
 *
 *   Copyright (C) 2004-2006 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOOEDIT_COMPILATION
#error "Do not include this file"
#endif

#ifndef __MOO_TEXT_VIEW_PRIVATE_H__
#define __MOO_TEXT_VIEW_PRIVATE_H__

#include "mooedit/mootextview.h"
#include "mooedit/mootextsearch.h"
#include "mooutils/moohistorylist.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS


extern GtkTextViewClass *_moo_text_view_parent_class;


/***********************************************************************/
/* GtkTextView stuff
/*/
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
void        _moo_text_view_backspace            (GtkTextView        *text_view);
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

gboolean    _moo_text_view_has_placeholders     (MooTextView        *view);


typedef enum {
    MOO_TEXT_VIEW_DRAG_NONE = 0,
    MOO_TEXT_VIEW_DRAG_SELECT,
    MOO_TEXT_VIEW_DRAG_DRAG
} MooTextViewDragType;

typedef enum {
    MOO_TEXT_VIEW_POS_LEFT,
    MOO_TEXT_VIEW_POS_RIGHT,
    MOO_TEXT_VIEW_POS_TOP,
    MOO_TEXT_VIEW_POS_BOTTOM,
    MOO_TEXT_VIEW_POS_INVALID
} MooTextViewPos;


struct _MooTextViewPrivate {
    gboolean constructed;

    /* Clipboard */
    gboolean manage_clipboard;

    /* Overwrite mode cursor */
    gboolean overwrite_mode;
    gboolean saved_cursor_visible;
    gboolean cursor_visible;
    guint blink_timeout;

    /***********************************************************************/
    /* Drawing
    /*/
    gboolean highlight_current_line;
    GdkColor current_line_color;
    GdkGC *current_line_gc;
    gboolean draw_tabs;
    gboolean draw_trailing_spaces;
    gboolean highlight_matching_brackets;
    gboolean highlight_mismatching_brackets;

    gboolean show_line_numbers;
    int digit_width; /* max line number digit width */
    PangoFontDescription *line_numbers_font;
    gboolean bold_current_line_number;

    gboolean show_scrollbar_marks;
    int right_margin_width;
    int cursor_line;

    gboolean show_line_marks;
    int line_mark_width;
    GSList *line_marks;

    gboolean enable_folding;
    int fold_margin_width;
    int expander_size;

    /***********************************************************************/
    /* Search
    /*/
    int last_search_stamp;
    GtkTextMark *last_found_start, *last_found_end;

    /***********************************************************************/
    /* Indentation
    /*/
    MooIndenter *indenter;
    gboolean tab_indents;
    gboolean tab_inserts_indent_chars;
    gboolean shift_tab_unindents;
    gboolean backspace_indents;
    gboolean enter_indents;

    /***********************************************************************/
    /* Keyboard
    /*/
    gboolean smart_home_end;
    gboolean ctrl_up_down_scrolls;
    gboolean ctrl_page_up_down_scrolls;
    /* key press handler sets this flag in order to distinguish typed in
       characters in buffer's insert-text signal */
    gboolean in_key_press;
    gunichar char_inserted;
    GtkTextIter char_inserted_pos;

    /***********************************************************************/
    /* Selection and drag
    /*/
    guint           drag_scroll_timeout;
    GdkEventType    drag_button;
    MooTextViewDragType drag_type;
    int             drag_start_x;
    int             drag_start_y;
    guint           drag_moved                      : 1;
    guint           double_click_selects_brackets   : 1;
    guint           double_click_selects_inside     : 1;

    /***********************************************************************/
    /* Drag'n'drop from outside
    /*/
    gboolean drag_inside;
    gboolean drag_drop;
    GtkTargetList *targets;
    GtkTextMark *dnd_mark;

    /***********************************************************************/
    /* Children
    /*/
    GtkWidget *children[4];

    /***********************************************************************/
    /* Search
    /*/
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

#endif /* __MOO_TEXT_VIEW_PRIVATE_H__ */
