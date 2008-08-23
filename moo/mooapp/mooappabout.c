/*
 *   mooapp/mooappabout.c
 *
 *   Copyright (C) 2004-2008 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#include "mooappabout-glade.h"
#include "mooappabout.h"
#include "mooapp.h"
#include "moohtml.h"
#include "moolinklabel.h"
#include "mooutils/moostock.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/mooglade.h"
#include "mooutils/mooi18n.h"
#include "mooutils/moodialogs.h"
#include "mooutils/moohelp.h"
#include "help-sections.h"
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>

#ifdef MOO_USE_XML
#include <libxml/xmlversion.h>
#endif

static gpointer about_dialog;
static gpointer credits_dialog;
static gpointer system_info_dialog;


#ifndef MOO_USE_XML
#define MooHtml GtkTextView
#undef MOO_TYPE_HTML
#define MOO_TYPE_HTML GTK_TYPE_TEXT_VIEW
#endif

static void
set_translator_credits (MooGladeXML *xml)
{
    GtkWidget *notebook, *page;
    const char *credits, *credits_markup;
    GtkTextView *view;

    /* Translators: this goes into About box, under Translated by tab,
       do not ignore it, markup isn't always used. It should be something
       like "Some Guy <someguy@domain.net>", with lines separated by \n */
    credits = _("translator-credits");
    /* Translators: this goes into About box, under Translated by tab,
       this must be valid html markup, e.g.
       "Some Guy <a href=\"mailto:someguy@domain.net\">&lt;someguy@domain.net&gt;</a>"
       Lines must be separated by <br>, like "First guy<br>Second Guy" */
    credits_markup = _("translator-credits-markup");

    if (!strcmp (credits, "translator-credits"))
    {
        notebook = moo_glade_xml_get_widget (xml, "notebook");
        page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), 2);
        gtk_widget_hide (page);
        return;
    }

    view = moo_glade_xml_get_widget (xml, "translated_by");

#if defined(MOO_USE_XML) && !defined(__WIN32__)
    if (strcmp (credits_markup, "translator-credits-markup") != 0)
        _moo_html_load_memory (view, credits_markup, -1, NULL, NULL);
    else
#endif
    {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer (view);
        gtk_text_buffer_insert_at_cursor (buffer, credits, -1);
    }
}

static void
show_credits (void)
{
    MooGladeXML *xml;
    GtkTextView *written_by;
    GtkTextView *thanks;
    GtkTextBuffer *buffer;
    const MooAppInfo *info;

    info = moo_app_get_info (moo_app_get_instance());
    g_return_if_fail (info && info->credits);

    if (credits_dialog)
    {
        if (about_dialog)
            moo_window_set_parent (credits_dialog, about_dialog);
        gtk_window_present (GTK_WINDOW (credits_dialog));
        return;
    }

    xml = moo_glade_xml_new_empty (GETTEXT_PACKAGE);
    moo_glade_xml_map_id (xml, "written_by", MOO_TYPE_HTML);
    moo_glade_xml_map_id (xml, "translated_by", MOO_TYPE_HTML);
    moo_glade_xml_parse_memory (xml, mooappabout_glade_xml, -1, "credits", NULL);

    credits_dialog = moo_glade_xml_get_widget (xml, "credits");
    g_return_if_fail (credits_dialog != NULL);
    g_object_add_weak_pointer (G_OBJECT (credits_dialog), &credits_dialog);
    g_signal_connect (credits_dialog, "response", G_CALLBACK (gtk_widget_destroy), NULL);

    written_by = moo_glade_xml_get_widget (xml, "written_by");
#if defined(MOO_USE_XML) && !defined(__WIN32__)
    _moo_html_load_memory (written_by,
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

    set_translator_credits (xml);

    thanks = moo_glade_xml_get_widget (xml, "thanks");
    buffer = gtk_text_view_get_buffer (thanks);
    gtk_text_buffer_insert_at_cursor (buffer, info->credits, -1);

    if (about_dialog)
        moo_window_set_parent (credits_dialog, about_dialog);
    gtk_window_present (GTK_WINDOW (credits_dialog));
}


static void
license_clicked (void)
{
    moo_help_open_id (HELP_SECTION_APP_LICENSE, NULL);
}


#define COPYRIGHT_SYMBOL "\302\251"
static const char copyright[] = COPYRIGHT_SYMBOL " 2004-2008 Yevgen Muntyan";


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
    char *markup;
    GtkLabel *label;
    MooLinkLabel *url;

    info = moo_app_get_info (moo_app_get_instance());
    xml = moo_glade_xml_new_empty (GETTEXT_PACKAGE);
    moo_glade_xml_map_id (xml, "url", MOO_TYPE_LINK_LABEL);
    moo_glade_xml_parse_memory (xml, mooappabout_glade_xml, -1, "dialog", NULL);
    g_return_val_if_fail (xml != NULL, NULL);

    dialog = moo_glade_xml_get_widget (xml, "dialog");
    g_signal_connect (dialog, "key-press-event", G_CALLBACK (about_dialog_key_press), NULL);

    g_object_add_weak_pointer (G_OBJECT (dialog), &about_dialog);
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
    _moo_link_label_set_url (url, info->website);
    _moo_link_label_set_text (url, info->website_label);

    logo = moo_glade_xml_get_widget (xml, "logo");

    if (info->logo)
    {
        GtkStockItem dummy;

        if (gtk_stock_lookup (info->logo, &dummy))
            gtk_image_set_from_stock (GTK_IMAGE (logo), info->logo,
                                      GTK_ICON_SIZE_DIALOG);
        else
            gtk_image_set_from_icon_name (GTK_IMAGE (logo), info->logo,
                                          GTK_ICON_SIZE_DIALOG);
    }
    else
    {
        gtk_widget_hide (logo);
    }

    button = moo_glade_xml_get_widget (xml, "credits_button");
    g_signal_connect (button, "clicked", G_CALLBACK (show_credits), NULL);
    button = moo_glade_xml_get_widget (xml, "license_button");
    g_signal_connect (button, "clicked", G_CALLBACK (license_clicked), NULL);
    button = moo_glade_xml_get_widget (xml, "close_button");
    g_signal_connect_swapped (button, "clicked", G_CALLBACK (gtk_widget_hide), dialog);

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
        moo_window_set_parent (about_dialog, parent);

    gtk_window_present (GTK_WINDOW (about_dialog));
}


static void
create_system_info_dialog (void)
{
    MooGladeXML *xml;
    GtkTextView *textview;
    GtkTextBuffer *buffer;
    char *text;

    if (system_info_dialog != NULL)
        return;

    xml = moo_glade_xml_new_from_buf (mooappabout_glade_xml, -1,
                                      "system", GETTEXT_PACKAGE, NULL);

    system_info_dialog = moo_glade_xml_get_widget (xml, "system");
    g_return_if_fail (system_info_dialog != NULL);
    g_object_add_weak_pointer (G_OBJECT (system_info_dialog), &system_info_dialog);
    g_signal_connect (system_info_dialog, "response", G_CALLBACK (gtk_widget_destroy), NULL);

    textview = moo_glade_xml_get_widget (xml, "textview");
    buffer = gtk_text_view_get_buffer (textview);
    text = moo_app_get_system_info (moo_app_get_instance ());
    gtk_text_buffer_set_text (buffer, text, -1);
    g_free (text);
}


char *
moo_app_get_system_info (MooApp *app)
{
    GString *text;
    char *string;
    char **dirs, **p;
    const MooAppInfo *app_info;

    g_return_val_if_fail (MOO_IS_APP (app), NULL);

    text = g_string_new (NULL);

    app_info = moo_app_get_info (app);
    g_return_val_if_fail (app_info != NULL, NULL);
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

    g_string_append_printf (text, "GTK version: %u.%u.%u\n",
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
#ifdef MOO_USE_GAMIN
    g_string_append_printf (text, "FAM support: gamin\n");
#else
    g_string_append_printf (text, "FAM support: yes\n");
#endif
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

#ifdef HAVE_CONFIGARGS_H
    g_string_append_printf (text, "Configured with: %s\n", configure_args);
#endif

    return g_string_free (text, FALSE);
}


void
moo_app_system_info_dialog (GtkWidget *parent)
{
    create_system_info_dialog ();

    if (parent)
        parent = gtk_widget_get_toplevel (parent);

    if (parent && GTK_IS_WINDOW (parent))
        moo_window_set_parent (system_info_dialog, parent);

    gtk_window_present (GTK_WINDOW (system_info_dialog));
}
