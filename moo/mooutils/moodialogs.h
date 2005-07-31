/*
 *   mooutils/moodialogs.h
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

#ifndef MOOUTILS_DIALOGS_H
#define MOOUTILS_DIALOGS_H

#include <gtk/gtkwidget.h>

G_BEGIN_DECLS


#define MOO_TYPE_FILE_DIALOG_TYPE (moo_file_dialog_type_get_type ())
GType moo_file_dialog_type_get_type (void) G_GNUC_CONST;

typedef enum {
    MOO_DIALOG_FILE_OPEN_EXISTING,
    MOO_DIALOG_FILE_OPEN_ANY,
    MOO_DIALOG_FILE_SAVE,
    MOO_DIALOG_DIR_OPEN
/*  MOO_DIALOG_FILE_CREATE,*/
/*  MOO_DIALOG_DIR_NEW,*/
} MooFileDialogType;


const char *moo_file_dialog (GtkWidget          *parent,
                             MooFileDialogType   type,
                             const char         *title,
                             const char         *start_dir);

GtkWidget  *moo_file_dialog_create          (GtkWidget          *parent,
                                             MooFileDialogType   type,
                                             gboolean            multiple,
                                             const char         *title,
                                             const char         *start_dir);
gboolean    moo_file_dialog_run             (GtkWidget          *dialog);
const char *moo_file_dialog_get_filename    (GtkWidget          *dialog);
GSList     *moo_file_dialog_get_filenames   (GtkWidget          *dialog);

const char *moo_file_dialogp(GtkWidget          *parent,
                             MooFileDialogType   type,
                             const char         *title,
                             const char         *prefs_key,
                             const char         *alternate_prefs_key);

const char *moo_font_dialog (GtkWidget          *parent,
                             const char         *title,
                             const char         *start_font,
                             gboolean            fixed_width);

void        moo_error_dialog    (GtkWidget  *parent,
                                 const char *text,
                                 const char *secondary_text);
void        moo_info_dialog     (GtkWidget  *parent,
                                 const char *text,
                                 const char *secondary_text);
void        moo_warning_dialog  (GtkWidget  *parent,
                                 const char *text,
                                 const char *secondary_text);


G_END_DECLS

#endif /* MOOUTILS_DIALOGS_H */
