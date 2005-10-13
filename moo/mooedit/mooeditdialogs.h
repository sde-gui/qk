/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mooeditdialogs.h
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

#ifndef __MOO_EDIT_DIALOGS_H__
#define __MOO_EDIT_DIALOGS_H__

#include "mooedit/mooedit.h"
#include "mooutils/moofiltermgr.h"

G_BEGIN_DECLS


typedef enum {
    MOO_EDIT_RESPONSE_CANCEL,
    MOO_EDIT_RESPONSE_SAVE,
    MOO_EDIT_RESPONSE_DONT_SAVE
} MooEditDialogResponse;


MooEditFileInfo *moo_edit_save_as_dialog            (MooEdit        *edit,
                                                     MooFilterMgr   *mgr);
GSList          *moo_edit_open_dialog               (GtkWidget      *widget,
                                                     MooFilterMgr   *mgr);

MooEditDialogResponse moo_edit_save_changes_dialog  (MooEdit        *edit);
MooEditDialogResponse moo_edit_save_multiple_changes_dialog (GSList *docs,
                                                     GSList        **to_save);

gboolean         moo_edit_reload_modified_dialog    (MooEdit        *edit);
gboolean         moo_edit_overwrite_modified_dialog (MooEdit        *edit);
gboolean         moo_edit_overwrite_deleted_dialog  (MooEdit        *edit);

void             moo_edit_file_deleted_dialog       (MooEdit        *edit);
int              moo_edit_file_modified_on_disk_dialog (MooEdit     *edit);

void             moo_edit_save_error_dialog         (GtkWidget      *widget,
                                                     const char     *err_msg);
void             moo_edit_open_error_dialog         (GtkWidget      *widget,
                                                     const char     *err_msg);
void             moo_edit_load_error_dialog         (GtkWidget      *widget,
                                                     const char     *err_msg);
void             moo_edit_reload_error_dialog       (GtkWidget      *widget,
                                                     const char     *err_msg);


void             moo_text_nothing_found_dialog      (MooTextView    *view,
                                                     const char     *text,
                                                     gboolean        regex);
gboolean         moo_text_search_from_beginning_dialog
                                                    (MooTextView    *view,
                                                     gboolean        backwards);
void             moo_text_regex_error_dialog        (MooTextView    *view,
                                                     GError         *error);
void             moo_text_replaced_n_dialog         (MooTextView    *view,
                                                     guint           n);
GtkWidget       *moo_text_prompt_on_replace_dialog  (MooTextView    *view);


G_END_DECLS

#endif /* __MOO_EDIT_DIALOGS_H__ */
