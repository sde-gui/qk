/*
 *   mooutils/moofiledialog.h
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

#ifndef __MOO_FILE_DIALOG_H__
#define __MOO_FILE_DIALOG_H__

#include <gtk/gtkwidget.h>
#include <mooutils/moofiltermgr.h>

G_BEGIN_DECLS


#define MOO_TYPE_FILE_DIALOG_TYPE         (moo_file_dialog_type_get_type ())

#define MOO_TYPE_FILE_DIALOG              (moo_file_dialog_get_type ())
#define MOO_FILE_DIALOG(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_FILE_DIALOG, MooFileDialog))
#define MOO_FILE_DIALOG_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_FILE_DIALOG, MooFileDialogClass))
#define MOO_IS_FILE_DIALOG(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_FILE_DIALOG))
#define MOO_IS_FILE_DIALOG_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_FILE_DIALOG))
#define MOO_FILE_DIALOG_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_FILE_DIALOG, MooFileDialogClass))

typedef struct _MooFileDialog        MooFileDialog;
typedef struct _MooFileDialogPrivate MooFileDialogPrivate;
typedef struct _MooFileDialogClass   MooFileDialogClass;

/* do not change, hardcoded in ggap package */
typedef enum {
    MOO_FILE_DIALOG_OPEN,
    MOO_FILE_DIALOG_OPEN_ANY,
    MOO_FILE_DIALOG_SAVE,
    MOO_FILE_DIALOG_OPEN_DIR
    /*  MOO_DIALOG_FILE_CREATE,*/
    /*  MOO_DIALOG_DIR_NEW,*/
} MooFileDialogType;

struct _MooFileDialog
{
    GObject parent;
    MooFileDialogPrivate *priv;
};

struct _MooFileDialogClass
{
    GObjectClass parent_class;

    void (*dialog_created) (MooFileDialog *fd,
                            GtkWidget     *widget);
};


GType           moo_file_dialog_get_type        (void) G_GNUC_CONST;
GType           moo_file_dialog_type_get_type   (void) G_GNUC_CONST;

MooFileDialog  *moo_file_dialog_new             (MooFileDialogType type,
                                                 GtkWidget      *parent,
                                                 gboolean        multiple,
                                                 const char     *title,
                                                 const char     *start_dir,
                                                 const char     *start_name);
void            moo_file_dialog_set_filter_mgr  (MooFileDialog  *dialog,
                                                 MooFilterMgr   *mgr,
                                                 const char     *id);

gboolean        moo_file_dialog_run             (MooFileDialog  *dialog);
const char     *moo_file_dialog_get_filename    (MooFileDialog  *dialog);
GSList         *moo_file_dialog_get_filenames   (MooFileDialog  *dialog);

void            moo_file_dialog_set_encoding    (MooFileDialog  *dialog,
                                                 const char     *encoding);
const char     *moo_file_dialog_get_encoding    (MooFileDialog  *dialog);

const char     *moo_file_dialog                 (GtkWidget      *parent,
                                                 MooFileDialogType type,
                                                 const char     *title,
                                                 const char     *start_dir);
const char     *moo_file_dialogp                (GtkWidget      *parent,
                                                 MooFileDialogType type,
                                                 const char     *title,
                                                 const char     *prefs_key,
                                                 const char     *alternate_prefs_key);


G_END_DECLS

#endif /* MOOUTILS_DIALOGS_H */
