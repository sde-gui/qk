/*
 *   mooapp/mooappabout.c
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
#include <config.h>
#endif

#include "mooapp/mooappabout-glade.h"
#include "mooapp/mooappabout.h"
#include "mooapp/mooapp.h"
#include "mooapp/moohtml.h"
#include "mooutils/moostock.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/moolinklabel.h"
#include "mooutils/mooglade.h"
#include "config.h"
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>

#ifdef MOO_USE_XML
#include <libxml/xmlversion.h>
#endif

static GtkWidget *about_dialog;
static GtkWidget *license_dialog;
static GtkWidget *credits_dialog;
static GtkWidget *system_info_dialog;


#ifndef MOO_USE_XML
#define MooHtml GtkTextView
#undef MOO_TYPE_HTML
#define MOO_TYPE_HTML GTK_TYPE_TEXT_VIEW
#endif

static void
show_credits (void)
{
    MooGladeXML *xml;
    MooHtml *written_by;
    GtkTextView *thanks;
    GtkTextBuffer *buffer;
    const MooAppInfo *info;

    info = moo_app_get_info (moo_app_get_instance());
    g_return_if_fail (info && info->credits);

    if (credits_dialog)
    {
        if (about_dialog)
            gtk_window_set_transient_for (GTK_WINDOW (credits_dialog),
                                          GTK_WINDOW (about_dialog));
        gtk_window_present (GTK_WINDOW (credits_dialog));
        return;
    }

    xml = moo_glade_xml_new_empty ();
    moo_glade_xml_map_id (xml, "written_by", MOO_TYPE_HTML);
    moo_glade_xml_parse_memory (xml, MOO_APP_ABOUT_GLADE_UI, -1, "credits");

    credits_dialog = moo_glade_xml_get_widget (xml, "credits");
    g_return_if_fail (credits_dialog != NULL);
    g_object_add_weak_pointer (G_OBJECT (credits_dialog), (gpointer*) &credits_dialog);
    g_signal_connect (credits_dialog, "response", G_CALLBACK (gtk_widget_destroy), NULL);

    written_by = moo_glade_xml_get_widget (xml, "written_by");
#ifdef MOO_USE_XML
    moo_html_load_memory (written_by,
                          "Yevgen Muntyan <a href=\"mailto://muntyan@tamu.edu\">"
                                  "&lt;muntyan@tamu.edu&gt;</a>",
                          -1, NULL, NULL);
#else
    /* XXX */
    {
        buffer = gtk_text_view_get_buffer (written_by);
        gtk_text_buffer_insert_at_cursor (buffer,
                                          "Yevgen Muntyan <muntyan@tamu.edu>", -1);
    }
#endif

    thanks = moo_glade_xml_get_widget (xml, "thanks");
    buffer = gtk_text_view_get_buffer (thanks);
    gtk_text_buffer_insert_at_cursor (buffer, info->credits, -1);

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

    xml = moo_glade_xml_new_from_buf (MOO_APP_ABOUT_GLADE_UI, -1, "license");

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


static void
show_system_info (void)
{
    MooGladeXML *xml;
    GtkTextView *textview;
    GtkTextBuffer *buffer;
    GString *text;
    char *string;
    char **dirs, **p;
    const MooAppInfo *app_info;

    if (system_info_dialog)
    {
        if (about_dialog)
            gtk_window_set_transient_for (GTK_WINDOW (system_info_dialog),
                                          GTK_WINDOW (about_dialog));
        gtk_window_present (GTK_WINDOW (system_info_dialog));
        return;
    }

    xml = moo_glade_xml_new_from_buf (MOO_APP_ABOUT_GLADE_UI, -1, "system");

    system_info_dialog = moo_glade_xml_get_widget (xml, "system");
    g_return_if_fail (system_info_dialog != NULL);
    g_object_add_weak_pointer (G_OBJECT (system_info_dialog), (gpointer*) &system_info_dialog);
    g_signal_connect (system_info_dialog, "response", G_CALLBACK (gtk_widget_destroy), NULL);

    textview = moo_glade_xml_get_widget (xml, "textview");
    buffer = gtk_text_view_get_buffer (textview);
    text = g_string_new (NULL);

    app_info = moo_app_get_info (moo_app_get_instance ());
    g_return_if_fail (app_info != NULL);
    g_string_append_printf (text, "%s-%s\n", app_info->full_name, app_info->version);

#ifdef __WIN32__
    string = get_windows_name ();
    g_string_append_printf (text, "OS: %s\n", string ? string : "Win32");
    g_free (string);
#else
    g_string_append (text, "OS: " MOO_OS_NAME "\n");

    if ((string = get_uname ()))
    {
        g_string_append_printf (text, "OS details: %s", string);

        if (!*string || string[strlen(string) - 1] != '\n')
            g_string_append (text, "\n");

        g_free (string);
    }
#endif

    g_string_append_printf (text, "GTK version: %d.%d.%d\n",
                            gtk_major_version,
                            gtk_minor_version,
                            gtk_micro_version);
    g_string_append_printf (text, "Built with GTK %d.%d.%d\n",
                            GTK_MAJOR_VERSION,
                            GTK_MINOR_VERSION,
                            GTK_MICRO_VERSION);

    if (moo_python_running ())
    {
        g_string_append (text, "Python support: yes\n");
        string = get_python_info ();
        g_string_append_printf (text, "Python: %s\n", string ? string : "None");
        g_free (string);
    }
    else
    {
        g_string_append (text, "Python support: no\n");
    }

#ifdef MOO_USE_XML
    g_string_append_printf (text, "libxml2: %s\n", LIBXML_DOTTED_VERSION);
#endif

#ifdef MOO_USE_FAM
    g_string_append_printf (text, "FAM support: yes\n");
#endif

#ifdef MOO_ENABLE_RELOCATION
    g_string_append_printf (text, "relocation enabled\n");
#else
    g_string_append_printf (text, "relocation disabled\n");
#endif

    g_string_append (text, "Data dirs: ");
    dirs = moo_get_data_dirs (MOO_DATA_SHARE, NULL);
    for (p = dirs; p && *p; ++p)
        g_string_append_printf (text, "%s'%s'", p == dirs ? "" : ", ", *p);
    g_string_append (text, "\n");
    g_strfreev (dirs);

    g_string_append (text, "Lib dirs: ");
    dirs = moo_get_data_dirs (MOO_DATA_LIB, NULL);
    for (p = dirs; p && *p; ++p)
        g_string_append_printf (text, "%s'%s'", p == dirs ? "" : ", ", *p);
    g_string_append (text, "\n");
    g_strfreev (dirs);

#ifdef MOO_BROKEN_GTK_THEME
    g_string_append (text, "Broken gtk theme: yes\n");
#endif

    gtk_text_buffer_set_text (buffer, text->str, -1);
    g_string_free (text, TRUE);

    if (about_dialog)
        gtk_window_set_transient_for (GTK_WINDOW (system_info_dialog),
                                      GTK_WINDOW (about_dialog));
    gtk_window_present (GTK_WINDOW (system_info_dialog));
}


#define COPYRIGHT_SYMBOL "\302\251"
static const char *copyright = COPYRIGHT_SYMBOL " 2004-2006 Yevgen Muntyan";


static gboolean
about_dialog_key_press (GtkWidget   *dialog,
                        GdkEventKey *event)
{
    if (event->keyval == GDK_Escape)
    {
        gtk_widget_hide (dialog);
        return TRUE;
    }

    return TRUE;
}


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
    g_signal_connect (dialog, "key-press-event", G_CALLBACK (about_dialog_key_press), NULL);

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

    if (info->logo)
        gtk_image_set_from_stock (GTK_IMAGE (logo), info->logo,
                                  GTK_ICON_SIZE_DIALOG);
    else
        gtk_widget_hide (logo);

    button = moo_glade_xml_get_widget (xml, "credits_button");
    g_signal_connect (button, "clicked", G_CALLBACK (show_credits), NULL);
    button = moo_glade_xml_get_widget (xml, "license_button");
    g_signal_connect (button, "clicked", G_CALLBACK (show_license), NULL);
    button = moo_glade_xml_get_widget (xml, "system_button");
    g_signal_connect (button, "clicked", G_CALLBACK (show_system_info), NULL);

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

    if (parent && GTK_IS_WINDOW (parent))
        gtk_window_set_transient_for (GTK_WINDOW (about_dialog), GTK_WINDOW (parent));

    gtk_window_present (GTK_WINDOW (about_dialog));
}
