/*
 *   mooprefsdialogpage.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_PREFS_DIALOG_PAGE_H
#define MOO_PREFS_DIALOG_PAGE_H

#include <gtk/gtk.h>
#include <mooutils/mooglade.h>

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
    char        *label;
    GdkPixbuf   *icon;
    char        *icon_stock_id;
    MooGladeXML *xml;
    GSList      *widgets;
    gboolean     auto_apply;
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
MooPrefsDialogPage *
            moo_prefs_dialog_page_new_from_xml  (const char         *label,
                                                 const char         *icon_stock_id,
                                                 MooGladeXML        *xml,
                                                 const char         *buffer,
                                                 const char         *page_id,
                                                 const char         *prefs_root);
gboolean    moo_prefs_dialog_page_fill_from_xml (MooPrefsDialogPage *page,
                                                 MooGladeXML        *xml,
                                                 const char         *buffer,
                                                 const char         *page_id,
                                                 const char         *prefs_root);

void        moo_prefs_dialog_page_bind_setting  (MooPrefsDialogPage *page,
                                                 GtkWidget          *widget,
                                                 const char         *setting,
                                                 GtkToggleButton    *set_or_not);


G_END_DECLS

#endif /* MOO_PREFS_DIALOG_PAGE_H */
