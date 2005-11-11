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

#include "mooedit/mooeditsearch.h"
#include "mooedit/mootextview.h"
#include "mooutils/gtksourceundomanager.h"
#include "mooutils/moohistorylist.h"

G_BEGIN_DECLS


/***********************************************************************/
/* GtkTextView stuff
/*/
void        _moo_text_view_move_cursor          (GtkTextView        *text_view,
                                                 GtkMovementStep     step,
                                                 gint                count,
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


typedef enum {
    MOO_TEXT_VIEW_DRAG_NONE = 0,
    MOO_TEXT_VIEW_DRAG_SELECT,
    MOO_TEXT_VIEW_DRAG_DRAG
} MooTextViewDragType;


typedef struct {
    int          last_search_stamp;
    char        *text;
    char        *replace_with;
    gboolean     regex;
    gboolean     case_sensitive;
    gboolean     backwards;
    gboolean     whole_words;
    gboolean     from_cursor;
    gboolean     dont_prompt_on_replace;
    MooHistoryList *text_to_find_history;
    MooHistoryList *replacement_history;
} MooTextSearchParams;

extern MooTextSearchParams *_moo_text_search_params;


struct _MooTextViewPrivate {
    gboolean constructed;

    GtkSourceUndoManager *undo_mgr;

    gboolean highlight_current_line;
    GdkColor current_line_color;
    GdkGC *current_line_gc;
    gboolean show_tabs;
    gboolean show_trailing_space;
    gboolean check_brackets;

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
};


G_END_DECLS

#endif /* __MOO_TEXT_VIEW_PRIVATE_H__ */
