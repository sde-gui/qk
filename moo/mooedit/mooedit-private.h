/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mooedit-private.h
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

#ifndef __MOO_EDIT_PRIVATE_H__
#define __MOO_EDIT_PRIVATE_H__

#include "mooedit/mooeditor.h"
#include "mooedit/mootextview.h"

G_BEGIN_DECLS


extern GSList *_moo_edit_instances;
void        _moo_edit_add_class_actions     (MooEdit        *edit);
void        _moo_edit_check_actions         (MooEdit        *edit);
void        _moo_edit_class_init_actions    (MooEditClass   *klass);

void        _moo_edit_do_popup              (MooEdit        *edit,
                                             GdkEventButton *event);

gboolean    _moo_edit_has_comments          (MooEdit        *edit,
                                             gboolean       *single_line,
                                             gboolean       *multi_line);


/***********************************************************************/
/* Preferences
/*/
void        _moo_edit_init_settings         (void);
void        _moo_edit_apply_settings        (MooEdit        *edit);
void        _moo_edit_freeze_config_notify  (MooEdit        *edit);
void        _moo_edit_thaw_config_notify    (MooEdit        *edit);
void        _moo_edit_update_config         (void);


/***********************************************************************/
/* File operations
/*/

void        _moo_edit_set_filename          (MooEdit        *edit,
                                             const char     *file,
                                             const char     *encoding);
char        *_moo_edit_filename_to_utf8     (const char     *filename);

void         _moo_edit_start_file_watch     (MooEdit        *edit);
void         _moo_edit_stop_file_watch      (MooEdit        *edit);

MooEdit     *_moo_edit_new                  (MooEditor      *editor);
void         _moo_edit_set_status           (MooEdit        *edit,
                                             MooEditStatus   status);


typedef enum {
    MOO_EDIT_LINE_END_NONE,
    MOO_EDIT_LINE_END_UNIX,
    MOO_EDIT_LINE_END_WIN32,
    MOO_EDIT_LINE_END_MAC,
    MOO_EDIT_LINE_END_MIX
} MooEditLineEndType;


struct _MooEditPrivate {
    MooEditor *editor;

    gulong modified_changed_handler_id;
    guint apply_config_idle;

    /***********************************************************************/
    /* Document
    /*/
    char *filename;
    char *basename;
    char *display_filename;
    char *display_basename;

    char *encoding;
    MooEditLineEndType line_end_type;
    MooEditStatus status;

    MooEditOnExternalChanges file_watch_policy;
    int file_monitor_id;
    gulong file_watch_event_handler_id;
    gulong focus_in_handler_id;
    gboolean modified_on_disk;
    gboolean deleted_from_disk;

    /***********************************************************************/
    /* Bookmarks
    /*/
    gboolean enable_bookmarks;
    GSList *bookmarks; /* sorted by line number */
    guint update_bookmarks_idle;

    /***********************************************************************/
    /* Actions
    /*/
    GtkMenu *menu;
    GtkActionGroup *actions;
};


G_END_DECLS

#endif /* __MOO_EDIT_PRIVATE_H__ */
