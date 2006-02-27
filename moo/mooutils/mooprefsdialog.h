/*
 *   mooutils/mooprefsdialog.h
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

#ifndef __MOO_PREFS_DIALOG_H__
#define __MOO_PREFS_DIALOG_H__

#include <mooutils/mooprefsdialogpage.h>

G_BEGIN_DECLS


#define MOO_TYPE_PREFS_DIALOG              (moo_prefs_dialog_get_type ())
#define MOO_PREFS_DIALOG(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_PREFS_DIALOG, MooPrefsDialog))
#define MOO_PREFS_DIALOG_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_PREFS_DIALOG, MooPrefsDialogClass))
#define MOO_IS_PREFS_DIALOG(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_PREFS_DIALOG))
#define MOO_IS_PREFS_DIALOG_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_PREFS_DIALOG))
#define MOO_PREFS_DIALOG_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_PREFS_DIALOG, MooPrefsDialogClass))


typedef struct _MooPrefsDialog        MooPrefsDialog;
typedef struct _MooPrefsDialogClass   MooPrefsDialogClass;

struct _MooPrefsDialog
{
    GtkDialog     dialog;
    GtkNotebook  *notebook;
    GtkListStore *store;
    GtkTreeView  *pages_list;
    gboolean      hide_on_delete;
};

struct _MooPrefsDialogClass
{
    GtkDialogClass parent_class;

    void (* init)   (MooPrefsDialog *dialog);
    void (* apply)  (MooPrefsDialog *dialog);
};


GType       moo_prefs_dialog_get_type       (void) G_GNUC_CONST;

GtkWidget*  moo_prefs_dialog_new            (const char         *title);

void        moo_prefs_dialog_run            (MooPrefsDialog     *dialog,
                                             GtkWidget          *parent);

void        moo_prefs_dialog_append_page    (MooPrefsDialog     *dialog,
                                             GtkWidget          *page);
void        moo_prefs_dialog_remove_page    (MooPrefsDialog     *dialog,
                                             GtkWidget          *page);
void        moo_prefs_dialog_insert_page    (MooPrefsDialog     *dialog,
                                             GtkWidget          *page,
                                             int                 position);


G_END_DECLS

#endif /* __MOO_PREFS_DIALOG_H__ */
