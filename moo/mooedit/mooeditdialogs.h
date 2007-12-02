/*
 *   mooeditdialogs.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_EDIT_DIALOGS_H
#define MOO_EDIT_DIALOGS_H

#include "mooedit/mooedit.h"
#include "mooutils/moofiltermgr.h"
#include "mooutils/moodialogs.h"

G_BEGIN_DECLS


MooEditFileInfo *_moo_edit_save_as_dialog           (MooEdit        *edit,
                                                     MooFilterMgr   *mgr,
                                                     const char     *display_basename);
GSList          *_moo_edit_open_dialog              (GtkWidget      *widget,
                                                     MooFilterMgr   *mgr,
                                                     MooEdit        *current_doc);

MooSaveChangesDialogResponse
                 _moo_edit_save_changes_dialog      (MooEdit        *edit);
MooSaveChangesDialogResponse
                 _moo_edit_save_multiple_changes_dialog (GSList     *docs,
                                                     GSList        **to_save);

gboolean         _moo_edit_reload_modified_dialog   (MooEdit        *edit);
gboolean         _moo_edit_overwrite_modified_dialog (MooEdit        *edit);

void             _moo_edit_save_error_dialog        (GtkWidget      *widget,
                                                     const char     *filename,
                                                     GError         *error);
void             _moo_edit_save_error_enc_dialog    (GtkWidget      *widget,
                                                     const char     *filename,
                                                     const char     *encoding);
void             _moo_edit_open_error_dialog        (GtkWidget      *widget,
                                                     const char     *filename,
                                                     const char     *encoding,
                                                     GError         *error);
void             _moo_edit_reload_error_dialog      (MooEdit        *doc,
                                                     GError         *error);


gboolean         _moo_text_search_from_start_dialog (GtkWidget      *parent,
                                                     gboolean        backwards);
void             _moo_text_regex_error_dialog       (GtkWidget      *parent,
                                                     GError         *error);

gboolean         _moo_text_replace_from_start_dialog(GtkWidget      *parent,
                                                     int             replaced);
GtkWidget       *_moo_text_prompt_on_replace_dialog (GtkWidget      *parent);


G_END_DECLS

#endif /* MOO_EDIT_DIALOGS_H */
