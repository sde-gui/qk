/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mootextview-private.h
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

#ifndef MOOEDIT_COMPILATION
#error "Do not include this file"
#endif

#ifndef __MOO_TEXT_VIEW_PRIVATE_H__
#define __MOO_TEXT_VIEW_PRIVATE_H__

#include "mooedit/mootextview.h"
#include "mooutils/moohistorylist.h"

G_BEGIN_DECLS


/***********************************************************************/
/* Drag'n'drop
/*/
void     _moo_text_view_drag_data_received  (GtkWidget      *widget,
                                             GdkDragContext *context,
                                             int             x,
                                             int             y,
                                             GtkSelectionData *data,
                                             guint           info,
                                             guint           time);
gboolean _moo_text_view_drag_drop           (GtkWidget      *widget,
                                             GdkDragContext *context,
                                             int             x,
                                             int             y,
                                             guint           time);
void     _moo_text_view_drag_leave          (GtkWidget      *widget,
                                             GdkDragContext *context,
                                             guint           time);
gboolean _moo_text_view_drag_motion         (GtkWidget      *widget,
                                             GdkDragContext *context,
                                             int             x,
                                             int             y,
                                             guint           time);


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


typedef enum {
    MOO_TEXT_VIEW_DRAG_NONE = 0,
    MOO_TEXT_VIEW_DRAG_SELECT,
    MOO_TEXT_VIEW_DRAG_DRAG
} MooTextViewDragType;


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
};

enum {
    DND_TARGET_TEXT = 1
};


G_END_DECLS

#endif /* __MOO_TEXT_VIEW_PRIVATE_H__ */
