/*
 *   mooedit/mooedit-private.h
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

#ifndef MOOEDIT_MOOEDIT_PRIVATE_H
#define MOOEDIT_MOOEDIT_PRIVATE_H

#include "mooedit/mooeditor.h"
#include "mooedit/mooeditsearch.h"

G_BEGIN_DECLS


/***********************************************************************/
/* GtkTextView stuff
/*/
void        _moo_edit_move_cursor           (GtkTextView        *text_view,
                                             GtkMovementStep     step,
                                             gint                count,
                                             gboolean            extend_selection);
void        _moo_edit_delete_from_cursor    (GtkTextView        *text_view,
                                             GtkDeleteType       type,
                                             gint                count);
int         _moo_edit_key_press_event       (GtkWidget          *widget,
                                             GdkEventKey        *event);
int         _moo_edit_key_release_event     (GtkWidget          *widget,
                                             GdkEventKey        *event);
int         _moo_edit_button_press_event    (GtkWidget          *widget,
                                             GdkEventButton     *event);
int         _moo_edit_button_release_event  (GtkWidget          *widget,
                                             GdkEventButton     *event);
int         _moo_edit_motion_event          (GtkWidget          *widget,
                                             GdkEventMotion     *event);
int         _moo_edit_extend_selection      (MooEdit            *edit,
                                             MooEditSelectionType type,
                                             GtkTextIter        *insert,
                                             GtkTextIter        *selection_bound);

/***********************************************************************/
/* Preferences
/*/
void        _moo_edit_set_default_settings  (void);
void        _moo_edit_apply_settings        (MooEdit    *edit);
void        _moo_edit_apply_style_settings  (MooEdit    *edit);
void        _moo_edit_settings_changed      (const char *key,
                                             const char *newval,
                                             MooEdit    *edit);

/***********************************************************************/
/* File operations
/*/
gboolean    _moo_edit_load              (MooEdit    *edit,
                                         const char *file,
                                         const char *encoding,
                                         GError    **error);
gboolean    _moo_edit_write             (MooEdit    *edit,
                                         const char *file,
                                         const char *encoding,
                                         GError    **error);

gboolean    _moo_edit_open              (MooEdit    *edit,
                                         const char *file,
                                         const char *encoding);
gboolean    _moo_edit_save              (MooEdit    *edit);
gboolean    _moo_edit_save_as           (MooEdit    *edit,
                                         const char *file,
                                         const char *encoding);
gboolean    _moo_edit_close             (MooEdit    *edit);
gboolean    _moo_edit_reload            (MooEdit    *edit,
                                         GError    **error);

void        _moo_edit_set_filename      (MooEdit    *edit,
                                         const char *file,
                                         const char *encoding);


MooEdit        *_moo_edit_new               (MooEditor  *editor);
MooEditLangMgr *_moo_edit_get_lang_mgr      (MooEdit    *edit);


typedef enum {
    MOO_EDIT_DRAG_NONE = 0,
    MOO_EDIT_DRAG_SELECT,
    MOO_EDIT_DRAG_DRAG
} MooEditDragType;


typedef enum {
    MOO_EDIT_LINE_END_UNIX  = 0,
    MOO_EDIT_LINE_END_WIN32 = 1,
    MOO_EDIT_LINE_END_MAC   = 2
} MooEditLineEndType;


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
} MooEditSearchParams;

extern MooEditSearchParams *_moo_edit_search_params;


struct _MooEditPrivate {
    guint constructed : 1;

    GtkTextBuffer *text_buffer;
    GtkSourceBuffer *source_buffer;

    MooEditor *editor;

    gulong can_undo_handler_id;
    gulong can_redo_handler_id;
    gulong modified_changed_handler_id;

    /***********************************************************************/
    /* Document
    /*/
    char *filename;
    char *basename;
    char *display_filename;
    char *display_basename;

    gboolean readonly;

    char *encoding;
    MooEditLineEndType line_end_type;
    MooEditDocStatus status;

    MooEditOnExternalChanges file_watch_policy;
    GTime timestamp;
    guint file_watch_id;
    gulong focus_in_handler_id;
    guint file_watch_timeout;

    /***********************************************************************/
    /* Search
    /*/
    int last_search_stamp;
    GtkTextMark *last_found_start, *last_found_end;

    /***********************************************************************/
    /* Language stuff
    /*/
    MooEditLang *lang;
    gboolean lang_custom;

    /***********************************************************************/
    /* Preferences
    /*/
    guint prefs_notify;

    /***********************************************************************/
    /* Keyboard
    /*/
    gboolean tab_indents;
    gboolean shift_tab_unindents;
    gboolean backspace_indents;
    gboolean auto_indent;
    gboolean ctrl_up_down_scrolls;
    gboolean ctrl_page_up_down_scrolls;

    /***********************************************************************/
    /* Selection and drag
    /*/
    guint           drag_scroll_timeout;
    GdkEventType    drag_button;
    MooEditDragType drag_type;
    int             drag_start_x;
    int             drag_start_y;
    guint           drag_moved                      : 1;
    guint           double_click_selects_brackets   : 1;
    guint           double_click_selects_inside     : 1;
};


MooEditPrivate *moo_edit_private_new (void);


G_END_DECLS

#endif /* MOOEDIT_MOOEDIT_PRIVATE_H */
