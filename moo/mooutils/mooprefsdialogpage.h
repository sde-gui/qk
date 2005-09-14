/*
 *   mooprefsdialogpage.h
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

#ifndef __MOO_PREFS_DIALOG_PAGE_H__
#define __MOO_PREFS_DIALOG_PAGE_H__

#include <gtk/gtk.h>
#include "mooutils/bind.h"
#include "mooutils/mooglade.h"

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

    MooGladeXML *xml;
};

struct _MooPrefsDialogPageClass
{
    GtkVBoxClass    parent_class;

    void (* init)           (MooPrefsDialogPage *page);
    void (* apply)          (MooPrefsDialogPage *page);
    void (* set_defaults)   (MooPrefsDialogPage *page);
};

GType       moo_prefs_dialog_page_get_type      (void) G_GNUC_CONST;

GtkWidget  *moo_prefs_dialog_page_new           (const char         *label,
                                                 const char         *icon_stock_id);
GtkWidget  *moo_prefs_dialog_page_new_from_xml  (const char         *label,
                                                 const char         *icon_stock_id,
                                                 const char         *buffer,
                                                 int                 buffer_size,
                                                 const char         *page_id,
                                                 const char         *prefs_root);

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


G_END_DECLS

#endif /* __MOO_PREFS_DIALOG_PAGE_H__ */

