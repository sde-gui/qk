/*
 *   mooapp/mooappdialogs.c
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mooapp/mooapp-private.h"
#include "mooterm/mooterm-prefs.h"
#include "mooedit/mooeditprefs.h"
#include "mooedit/mooplugin.h"
#include "mooutils/mooprefsdialog.h"
#include "mooutils/moostock.h"
#include "mooutils/mooutils-misc.h"
#include <gtk/gtk.h>


void             moo_app_prefs_dialog           (GtkWidget  *parent)
{
    MooApp *app;
    GtkWidget *dialog;

    app = moo_app_get_instance ();
    dialog = MOO_APP_GET_CLASS(app)->prefs_dialog (app);
    g_return_if_fail (MOO_IS_PREFS_DIALOG (dialog));

    moo_prefs_dialog_run (MOO_PREFS_DIALOG (dialog), parent);
}


GtkWidget *_moo_app_create_prefs_dialog (MooApp *app)
{
    char *title;
    const MooAppInfo *info;
    MooPrefsDialog *dialog;

    info = moo_app_get_info (app);
    title = g_strdup_printf ("%s Preferences", info->full_name);
    dialog = MOO_PREFS_DIALOG (moo_prefs_dialog_new (title));
    g_free (title);

#ifdef MOO_BUILD_TERM
//     moo_prefs_dialog_append_page (dialog, moo_term_prefs_page_new ());
#endif

#ifdef MOO_BUILD_EDIT
    moo_prefs_dialog_append_page (dialog, moo_edit_prefs_page_new (moo_app_get_editor (app)));
//     moo_prefs_dialog_append_page (dialog, moo_edit_colors_prefs_page_new (moo_app_get_editor (app)));
    _moo_plugin_attach_prefs (GTK_WIDGET (dialog));
#endif

    return GTK_WIDGET (dialog);
}


#define COPYRIGHT_SYMBOL "\302\251"
static const char *copyright = COPYRIGHT_SYMBOL " 2004-2005 Yevgen Muntyan";


static void show_about_dialog (GtkWidget *dialog,
                               GtkWindow *parent)
{
    g_return_if_fail (dialog != NULL);

    if (parent)
        gtk_window_set_transient_for (GTK_WINDOW (dialog), parent);

    gtk_window_present (GTK_WINDOW (dialog));
}

static void dialog_destroyed (void)
{
    g_object_set_data (G_OBJECT (moo_app_get_instance()),
                       "moo-app-about-dialog", NULL);
}


#if GTK_CHECK_VERSION(2,6,0)

static void open_link (G_GNUC_UNUSED GtkAboutDialog *about,
                       const gchar *link,
                       G_GNUC_UNUSED gpointer data)
{
    moo_open_url (link);
}

static void send_mail (G_GNUC_UNUSED GtkAboutDialog *about,
                       const gchar *address,
                       G_GNUC_UNUSED gpointer data)
{
    moo_open_email (address, NULL, NULL);
}


void             moo_app_about_dialog           (GtkWidget  *parent)
{
    GtkWindow *parent_window = NULL;
    GtkWidget *dialog;
    static gboolean url_hook_set = FALSE;
    const MooAppInfo *info;
    const char *authors[] = {"Yevgen Muntyan <muntyan@math.tamu.edu>", NULL};
    const char *gpl =
#include "mooapp/gpl"
    ;

    if (!url_hook_set)
    {
        url_hook_set = TRUE;

        gtk_about_dialog_set_email_hook (send_mail, NULL, NULL);
        gtk_about_dialog_set_url_hook (open_link, NULL, NULL);
    }

    if (parent)
        parent_window = GTK_WINDOW (gtk_widget_get_toplevel (parent));

    info = moo_app_get_info (moo_app_get_instance());

    dialog = g_object_get_data (G_OBJECT (moo_app_get_instance()),
                                "moo-app-about-dialog");

    if (!dialog)
    {
        dialog = g_object_new (GTK_TYPE_ABOUT_DIALOG,
                               "authors", authors,
                               "comments", info->description,
                               "copyright", copyright,
                               "license", gpl,
                               "logo-icon-name", MOO_STOCK_APP,
                               "name", info->full_name,
                               "version", info->version,
                               "website", info->website,
                               "website-label", info->website_label,
                               NULL);
        g_object_set_data_full (G_OBJECT (moo_app_get_instance()),
                                "moo-app-about-dialog", dialog,
                                g_object_unref);
        g_signal_connect (dialog, "delete-event",
                          G_CALLBACK (gtk_widget_hide_on_delete),
                          NULL);
        g_signal_connect (dialog, "destroy",
                          G_CALLBACK (dialog_destroyed),
                          NULL);
    }

    show_about_dialog (dialog, parent_window);
}

#else /* !GTK_CHECK_VERSION(2,6,0) */

#include "mooapp/mooappabout-glade.h"
#include "mooutils/mooglade.h"

void moo_app_about_dialog (GtkWidget *parent)
{
    GtkWindow *parent_window = NULL;
    GtkWidget *dialog;
    MooGladeXML *xml;

    if (parent)
        parent_window = GTK_WINDOW (gtk_widget_get_toplevel (parent));

    dialog = g_object_get_data (G_OBJECT (moo_app_get_instance()),
                                "moo-app-about-dialog");

    if (!dialog)
    {
        const MooAppInfo *info;
        char *name_markup, *copyright_markup, *title;
        GtkLabel *name_label, *copyright_label;
        GtkWidget *dialog, *logo;

        info = moo_app_get_info (moo_app_get_instance());

        xml = moo_glade_xml_new_from_buf (MOO_APP_ABOUT_GLADE_UI,
                                          -1, "dialog", NULL);
        g_return_if_fail (xml != NULL);

        dialog = moo_glade_xml_get_widget (xml, "dialog");

        title = g_strdup_printf ("About %s", info->full_name);
        gtk_window_set_title (GTK_WINDOW (dialog), title);

        g_object_set_data (G_OBJECT (moo_app_get_instance()),
                           "moo-app-about-dialog",
                           dialog);
        g_signal_connect (dialog, "delete-event",
                          G_CALLBACK (gtk_widget_hide_on_delete),
                          NULL);
        g_signal_connect (dialog, "destroy",
                          G_CALLBACK (dialog_destroyed),
                          NULL);

        name_label = moo_glade_xml_get_widget (xml, "name_label");
        name_markup = g_strdup_printf ("<span size=\"xx-large\"><b>%s</b></span>",
                                       info->full_name);
        gtk_label_set_markup (name_label, name_markup);

        copyright_label = moo_glade_xml_get_widget (xml, "copyright_label");
        copyright_markup = g_strdup_printf ("<small>%s</small>", copyright);
        gtk_label_set_markup (copyright_label, copyright_markup);

        logo = moo_glade_xml_get_widget (xml, "logo");
        gtk_image_set_from_stock (GTK_IMAGE (logo), MOO_STOCK_APP, GTK_ICON_SIZE_DIALOG);

        g_free (name_markup);
        g_free (copyright_markup);
        g_free (title);
        moo_glade_xml_unref (xml);
    }

    show_about_dialog (dialog, parent_window);
}

#endif /* !GTK_CHECK_VERSION(2,6,0) */
