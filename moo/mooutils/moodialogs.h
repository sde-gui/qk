/*
 *   moodialogs.h
 *
 *   Copyright (C) 2004-2008 by Yevgen Muntyan <muntyan@tamu.edu>
 *
 *   This file is part of medit.  medit is free software; you can
 *   redistribute it and/or modify it under the terms of the
 *   GNU Lesser General Public License as published by the
 *   Free Software Foundation; either version 2.1 of the License,
 *   or (at your option) any later version.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with medit.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MOOUTILS_DIALOGS_H
#define MOOUTILS_DIALOGS_H

#include <gtk/gtkmessagedialog.h>
#include "mooutils/moofiledialog.h"

G_BEGIN_DECLS


typedef enum {
    MOO_SAVE_CHANGES_RESPONSE_CANCEL,
    MOO_SAVE_CHANGES_RESPONSE_SAVE,
    MOO_SAVE_CHANGES_RESPONSE_DONT_SAVE
} MooSaveChangesDialogResponse;


const char *moo_font_dialog                 (GtkWidget  *parent,
                                             const char *title,
                                             const char *start_font,
                                             gboolean    fixed_width);

gboolean    moo_overwrite_file_dialog       (GtkWidget  *parent,
                                             const char *display_name,
                                             const char *display_dirname);
MooSaveChangesDialogResponse
            moo_save_changes_dialog         (const char *display_name,
                                             GtkWidget  *parent);

void        moo_position_window_at_pointer  (GtkWidget  *window,
                                             GtkWidget  *parent);
void        moo_window_set_parent           (GtkWidget  *window,
                                             GtkWidget  *parent);

void        moo_error_dialog                (GtkWidget  *parent,
                                             const char *text,
                                             const char *secondary_text);
void        moo_info_dialog                 (GtkWidget  *parent,
                                             const char *text,
                                             const char *secondary_text);
void        moo_warning_dialog              (GtkWidget  *parent,
                                             const char *text,
                                             const char *secondary_text);

void        _moo_window_set_remember_size   (GtkWindow  *window,
                                             const char *prefs_key,
                                             gboolean    remember_position);


G_END_DECLS

#endif /* MOOUTILS_DIALOGS_H */
