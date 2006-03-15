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
#include "mooapp/mooappabout-glade.h"
#include "mooapp/moohtml.h"
#include "mooutils/moostock.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/moolinklabel.h"
#include "mooutils/mooglade.h"
#include <gtk/gtk.h>


static GtkWidget *about_dialog;
static GtkWidget *license_dialog;
static GtkWidget *credits_dialog;
static GtkWidget *system_info_dialog;


static void
show_credits (void)
{
    MooGladeXML *xml;
    MooHtml *written_by;

    if (credits_dialog)
    {
        if (about_dialog)
            gtk_window_set_transient_for (GTK_WINDOW (credits_dialog),
                                          GTK_WINDOW (about_dialog));
        gtk_window_present (GTK_WINDOW (credits_dialog));
    }

    xml = moo_glade_xml_new_empty ();
    moo_glade_xml_map_id (xml, "written_by", MOO_TYPE_HTML);
    moo_glade_xml_parse_memory (xml, MOO_APP_ABOUT_GLADE_UI, -1, "credits");

    credits_dialog = moo_glade_xml_get_widget (xml, "credits");
    g_return_if_fail (credits_dialog != NULL);
    g_object_add_weak_pointer (G_OBJECT (credits_dialog), (gpointer*) &credits_dialog);
    g_signal_connect (credits_dialog, "response", G_CALLBACK (gtk_widget_destroy), NULL);

    written_by = moo_glade_xml_get_widget (xml, "written_by");
    moo_html_load_memory (written_by,
                          "Yevgen Muntyan <a href=\"mailto://muntyan@math.tamu.edu\">"
                          "&lt;muntyan@math.tamu.edu&gt;</a>",
                          -1, NULL, NULL);

    if (about_dialog)
        gtk_window_set_transient_for (GTK_WINDOW (credits_dialog),
                                      GTK_WINDOW (about_dialog));
    gtk_window_present (GTK_WINDOW (credits_dialog));
}


static void
show_license (void)
{
    MooGladeXML *xml;
    GtkTextView *textview;

    const char *gpl =
#include "mooapp/gpl"
    ;

    if (license_dialog)
    {
        if (about_dialog)
            gtk_window_set_transient_for (GTK_WINDOW (license_dialog),
                                          GTK_WINDOW (about_dialog));
        gtk_window_present (GTK_WINDOW (license_dialog));
        return;
    }

    xml = moo_glade_xml_new_from_buf (MOO_APP_ABOUT_GLADE_UI, -1, "license", NULL);

    license_dialog = moo_glade_xml_get_widget (xml, "license");
    g_return_if_fail (license_dialog != NULL);
    g_object_add_weak_pointer (G_OBJECT (license_dialog), (gpointer*) &license_dialog);
    g_signal_connect (license_dialog, "response", G_CALLBACK (gtk_widget_destroy), NULL);

    textview = moo_glade_xml_get_widget (xml, "textview");
    gtk_text_buffer_set_text (gtk_text_view_get_buffer (textview), gpl, -1);

    if (about_dialog)
        gtk_window_set_transient_for (GTK_WINDOW (license_dialog),
                                      GTK_WINDOW (about_dialog));
    gtk_window_present (GTK_WINDOW (license_dialog));
}


#define COPYRIGHT_SYMBOL "\302\251"
static const char *copyright = COPYRIGHT_SYMBOL " 2004-2006 Yevgen Muntyan";


static GtkWidget *
create_about_dialog (void)
{
    MooGladeXML *xml;
    GtkWidget *dialog, *logo, *button;
    const MooAppInfo *info;
    char *markup, *title;
    GtkLabel *label;
    MooLinkLabel *url;

    info = moo_app_get_info (moo_app_get_instance());
    xml = moo_glade_xml_new_empty ();
    moo_glade_xml_map_id (xml, "url", MOO_TYPE_LINK_LABEL);
    moo_glade_xml_parse_memory (xml, MOO_APP_ABOUT_GLADE_UI, -1, "dialog");
    g_return_val_if_fail (xml != NULL, NULL);

    dialog = moo_glade_xml_get_widget (xml, "dialog");

    title = g_strdup_printf ("About %s", info->full_name);
    gtk_window_set_title (GTK_WINDOW (dialog), title);

    g_object_add_weak_pointer (G_OBJECT (dialog), (gpointer*) &about_dialog);
    g_signal_connect (dialog, "delete-event", G_CALLBACK (gtk_widget_hide_on_delete), NULL);

    label = moo_glade_xml_get_widget (xml, "name");
    markup = g_strdup_printf ("<span size=\"xx-large\"><b>%s-%s</b></span>",
                              info->full_name, info->version);
    gtk_label_set_markup (label, markup);
    g_free (markup);

    label = moo_glade_xml_get_widget (xml, "description");
    gtk_label_set_text (label, info->description);

    label = moo_glade_xml_get_widget (xml, "copyright");
    markup = g_strdup_printf ("<small>%s</small>", copyright);
    gtk_label_set_markup (label, markup);
    g_free (markup);

    url = moo_glade_xml_get_widget (xml, "url");
    moo_link_label_set_url (url, info->website);
    moo_link_label_set_text (url, info->website_label);

    logo = moo_glade_xml_get_widget (xml, "logo");
    gtk_image_set_from_stock (GTK_IMAGE (logo), MOO_STOCK_APP,
                              GTK_ICON_SIZE_DIALOG);

    button = moo_glade_xml_get_widget (xml, "credits_button");
    g_signal_connect (button, "clicked", G_CALLBACK (show_credits), NULL);
    button = moo_glade_xml_get_widget (xml, "license_button");
    g_signal_connect (button, "clicked", G_CALLBACK (show_license), NULL);
//     button = moo_glade_xml_get_widget (xml, "system_button");
//     g_signal_connect (button, "clicked", G_CALLBACK (show_system_info), NULL);

    g_free (title);
    g_object_unref (xml);

    return dialog;
}


void
moo_app_about_dialog (GtkWidget *parent)
{
    if (!about_dialog)
        about_dialog = create_about_dialog ();

    if (parent)
        parent = gtk_widget_get_toplevel (parent);

    if (parent && GTK_WIDGET_TOPLEVEL (parent))
        gtk_window_set_transient_for (GTK_WINDOW (about_dialog), GTK_WINDOW (parent));

    gtk_window_present (GTK_WINDOW (about_dialog));
}
