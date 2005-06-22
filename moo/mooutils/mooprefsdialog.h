/*
 *   mooutils/mooprefsdialog.h
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

#ifndef MOOEDIT_MOOPREFSDIALOG_H
#define MOOEDIT_MOOPREFSDIALOG_H

#include <gtk/gtkdialog.h>
#include <gtk/gtknotebook.h>
#include <gtk/gtktreeview.h>
#include <gtk/gtkvbox.h>
#include "mooutils/bind.h"

G_BEGIN_DECLS


#define MOO_TYPE_PREFS_DIALOG_PAGE              (moo_prefs_dialog_page_get_type ())
#define MOO_PREFS_DIALOG_PAGE(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_PREFS_DIALOG_PAGE, MooPrefsDialogPage))
#define MOO_PREFS_DIALOG_PAGE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_PREFS_DIALOG_PAGE, MooPrefsDialogPageClass))
#define MOO_IS_PREFS_DIALOG_PAGE(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_PREFS_DIALOG_PAGE))
#define MOO_IS_PREFS_DIALOG_PAGE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_PREFS_DIALOG_PAGE))
#define MOO_PREFS_DIALOG_PAGE_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_PREFS_DIALOG_PAGE, MooPrefsDialogPageClass))


typedef struct _MooPrefsDialogPage        MooPrefsDialogPage;
typedef struct _MooPrefsDialogPageClass   MooPrefsDialogPageClass;

struct _MooPrefsDialogPage
{
    GtkVBox     vbox;

    char       *label;
    GdkPixbuf  *icon;
    char       *icon_stock_id;
};

struct _MooPrefsDialogPageClass
{
    GtkVBoxClass    parent_class;

    void (* init)           (MooPrefsDialogPage *page);
    void (* apply)          (MooPrefsDialogPage *page);
    void (* set_defaults)   (MooPrefsDialogPage *page);
};

GType       moo_prefs_dialog_page_get_type      (void) G_GNUC_CONST;
GtkWidget*  moo_prefs_dialog_page_new           (const char         *label,
                                                 const char         *stock_icon_id);

void        moo_prefs_dialog_page_bind_setting  (MooPrefsDialogPage *page,
                                                 GtkWidget          *widget,
                                                 const char         *setting,
                                                 GtkToggleButton    *set_or_not);

void        moo_prefs_dialog_page_bind_radio_setting
                                                (MooPrefsDialogPage *page,
                                                 const char         *setting,
                                                 GtkToggleButton   **btns,
                                                 const char        **cvals);
void        moo_prefs_dialog_page_bind_radio    (MooPrefsDialogPage *page,
                                                 const char         *setting,
                                                 GtkToggleButton    *btn,
                                                 const char         *cval);


#define MOO_TYPE_PREFS_DIALOG              (moo_prefs_dialog_get_type ())
#define MOO_PREFS_DIALOG(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_PREFS_DIALOG, MooPrefsDialog))
#define MOO_PREFS_DIALOG_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_PREFS_DIALOG, MooPrefsDialogClass))
#define MOO_IS_PREFS_DIALOG(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_PREFS_DIALOG))
#define MOO_IS_PREFS_DIALOG_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_PREFS_DIALOG))
#define MOO_PREFS_DIALOG_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_PREFS_DIALOG, MooPrefsDialogClass))


typedef struct _MooPrefsDialog        MooPrefsDialog;
// typedef struct _MooPrefsDialogPrivate MooPrefsDialogPrivate;
typedef struct _MooPrefsDialogClass   MooPrefsDialogClass;

struct _MooPrefsDialog
{
    GtkDialog               dialog;
    GtkNotebook            *notebook;
    GtkTreeView            *pages_list;
    gboolean                hide_on_delete;
//     MooPrefsDialogPrivate  *priv;
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

int         moo_prefs_dialog_append_page    (MooPrefsDialog     *dialog,
                                             GtkWidget          *page);


G_END_DECLS

#endif // MOOEDIT_MOOPREFSDIALOG_H

