/*
 *   moofileview-dialogs.c
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define MOO_FILE_SYSTEM_COMPILATION
#include "moofileview-dialogs.h"
#include "moofilesystem.h"
#include "xdgmime/xdgmime.h"
#include <glade/glade.h>
#include <time.h>
#include <string.h>

#define MAGIC_STRING "moo-file-props-dialog-magic-string"
#define MAGIC_NUMBER (GINT_TO_POINTER (87654321))


static void dialog_response             (GtkWidget  *dialog,
                                         gint        response);
static void dialog_ok                   (GtkWidget  *dialog);


#ifndef MOO_FILE_PROPS_GLADE_FILE
#define MOO_FILE_PROPS_GLADE_FILE "fileprops.glade"
#endif


GtkWidget  *moo_file_props_dialog_new   (GtkWidget  *parent)
{
    GtkWidget *dialog, *notebook, *entry;
    GladeXML *xml;

    if (parent)
        parent = gtk_widget_get_toplevel (parent);
    if (!GTK_IS_WINDOW (parent))
        parent = NULL;

    dialog = gtk_dialog_new_with_buttons ("Properties",
                                          parent ? GTK_WINDOW (parent) : NULL,
                                          GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_OK, GTK_RESPONSE_OK,
                                          NULL);

    xml = glade_xml_new (MOO_FILE_PROPS_GLADE_FILE, "notebook", NULL);

    if (!xml)
        g_error ("Yes, glade is great usually, but not always");

    g_object_set_data_full (G_OBJECT (dialog), "dialog-glade-xml",
                            xml, g_object_unref);

    notebook = glade_xml_get_widget (xml, "notebook");
    g_assert (notebook != NULL);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG(dialog)->vbox), notebook, TRUE, TRUE, 0);

    entry = glade_xml_get_widget (xml, "entry_name");
    gtk_widget_grab_focus (entry);

    gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
    g_object_set_data (G_OBJECT (dialog), MAGIC_STRING, MAGIC_NUMBER);
    g_signal_connect (dialog, "response", G_CALLBACK (dialog_response), NULL);

    return dialog;
}


static void dialog_response             (GtkWidget  *dialog,
                                         gint        response)
{
    switch (response)
    {
        case GTK_RESPONSE_OK:
            dialog_ok (dialog);
            break;

        case GTK_RESPONSE_DELETE_EVENT:
        case GTK_RESPONSE_CANCEL:
            break;

        default:
            g_warning ("%s: unknown response code", G_STRLOC);
    }

    moo_file_props_dialog_set_file (dialog, NULL, NULL);
    gtk_widget_hide (dialog);
}


#define get_widget(dialog,name) \
    (glade_xml_get_widget (g_object_get_data (G_OBJECT (dialog), "dialog-glade-xml"), name))


static void dialog_ok                   (GtkWidget  *dialog)
{
    GtkWidget *entry;
    MooFile *file;
    MooFolder *folder;
    const char *old_name, *new_name;
    char *old_path, *new_path;
    GError *error = NULL;

    file = g_object_get_data (G_OBJECT (dialog), "moo-file");
    folder = g_object_get_data (G_OBJECT (dialog), "moo-folder");

    if (!file)
        return;

    entry = get_widget (dialog, "entry_name");
    old_name = moo_file_display_name (file);
    new_name = gtk_entry_get_text (GTK_ENTRY (entry));

    if (!strcmp (old_name, new_name))
        return;

    old_path = moo_file_system_make_path (moo_folder_get_file_system (folder),
                                          moo_folder_get_path (folder),
                                          old_name, NULL);
    new_path = moo_file_system_make_path (moo_folder_get_file_system (folder),
                                          moo_folder_get_path (folder),
                                          new_name, NULL);

    if (!old_path || !new_path)
    {
        g_warning ("%s: something wrong happened", G_STRLOC);
        goto out;
    }

    if (!moo_file_system_move_file (moo_folder_get_file_system (folder),
                                    old_path, new_path, &error))
    {
        g_warning ("%s: could not rename '%s' to '%s'",
                   G_STRLOC, old_path, new_path);
        if (error)
        {
            g_warning ("%s: %s", G_STRLOC, error->message);
            g_error_free (error);
        }
        goto out;
    }

out:
    g_free (old_path);
    g_free (new_path);
    g_object_set_data (G_OBJECT (dialog), "moo-file", NULL);
    g_object_set_data (G_OBJECT (dialog), "moo-folder", NULL);
}


static void set_file        (GtkWidget  *dialog,
                             MooFile    *file,
                             MooFolder  *folder)
{
    GtkWidget *entry = get_widget (dialog, "entry_name");

    if (file)
    {
        char *title;
        const char *name;

        name = moo_file_display_name (file);
        gtk_entry_set_text (GTK_ENTRY (entry), name);
        title = g_strdup_printf ("%s Properties", name);
        gtk_window_set_title (GTK_WINDOW (dialog), title);
        g_free (title);

        g_object_set_data_full (G_OBJECT (dialog), "moo-file",
                                moo_file_ref (file),
                                (GDestroyNotify) moo_file_unref);
        g_object_set_data_full (G_OBJECT (dialog), "moo-folder",
                                g_object_ref (folder),
                                g_object_unref);
    }
    else
    {
        gtk_entry_set_text (GTK_ENTRY (entry), "");
        gtk_window_set_title (GTK_WINDOW (dialog), "Properties");

        g_object_set_data (G_OBJECT (dialog), "moo-file", NULL);
        g_object_set_data (G_OBJECT (dialog), "moo-folder", NULL);
    }
}


static void set_label       (GtkWidget  *dialog,
                             const char *label_name,
                             const char *text)
{
    GtkWidget *label = get_widget (dialog, label_name);
    gtk_label_set_text (GTK_LABEL (label), text ? text : "");
}


static void set_points_to   (GtkWidget  *dialog,
                             const char *text)
{
    GtkWidget *label = get_widget (dialog, "points_to_label");
    GtkWidget *caption = get_widget (dialog, "points_to_caption");

    if (text && text[0])
    {
        gtk_widget_show (label);
        gtk_widget_show (caption);
        gtk_label_set_text (GTK_LABEL (label), text);
    }
    else
    {
        gtk_widget_hide (label);
        gtk_widget_hide (caption);
    }
}


static void set_icon        (GtkWidget  *dialog,
                             GdkPixbuf  *icon)
{
    GtkWidget *image = get_widget (dialog, "image_icon");
    gtk_image_set_from_pixbuf (GTK_IMAGE (image), icon);
}


#define MAX_DATE_LEN 1024

void        moo_file_props_dialog_set_file      (GtkWidget      *dialog,
                                                 MooFile        *file,
                                                 MooFolder      *folder)
{
    GdkPixbuf *icon;
    char *text;
    GtkWidget *notebook;
    MooFileTime mtime;
    MooFileSize size;
    char buf[MAX_DATE_LEN];

    g_return_if_fail (GTK_IS_DIALOG (dialog));
    g_return_if_fail ((!file && !folder) || (file && MOO_IS_FOLDER (folder)));
    g_return_if_fail (g_object_get_data (G_OBJECT (dialog), MAGIC_STRING) == MAGIC_NUMBER);

    notebook = get_widget (dialog, "notebook");

    if (!file)
    {
        gtk_widget_set_sensitive (notebook, FALSE);

        set_file (dialog, NULL, NULL);
        set_label (dialog, "location_label", NULL);
        set_label (dialog, "mime_label", NULL);
        set_points_to (dialog, NULL);

        return;
    }
    else
    {
        gtk_widget_set_sensitive (notebook, TRUE);
    }

    moo_folder_get_file_info (folder, file);

    icon = moo_file_get_icon (file, dialog, GTK_ICON_SIZE_DIALOG);
    set_icon (dialog, icon);

    set_file (dialog, file, folder);

    text = g_filename_display_name (moo_folder_get_path (folder));
    set_label (dialog, "location_label", text);
    g_free (text);

    set_label (dialog, "mime_label", moo_file_get_mime_type (file));

    if (MOO_FILE_IS_LINK (file))
    {
#ifndef __WIN32__
        char *display_target;
        const char *target = moo_file_link_get_target (file);

        if (target)
            display_target = g_filename_display_name (target);
        else
            display_target = g_strdup ("<Broken>");

        set_points_to (dialog, display_target);
        g_free (display_target);
#endif /* !__WIN32__ */
    }
    else
    {
        set_points_to (dialog, NULL);
    }

    size = moo_file_get_size (file);
    text = g_strdup_printf ("%" G_GINT64_FORMAT, size);
    set_label (dialog, "size_label", text);
    g_free (text);

    mtime = moo_file_get_mtime (file);
    if (strftime (buf, MAX_DATE_LEN, "%x %X", localtime ((time_t*)&mtime)))
        set_label (dialog, "time_label", buf);
    else
        set_label (dialog, "time_label", NULL);
}



#ifndef MOO_CREATE_FOLDER_GLADE_FILE
#define MOO_CREATE_FOLDER_GLADE_FILE "create_folder.glade"
#endif


char       *moo_create_folder_dialog        (GtkWidget  *parent,
                                             MooFolder  *folder)
{
    GtkWidget *dialog, *entry, *label;
    GladeXML *xml;
    char *path, *new_folder_name = NULL;

    g_return_val_if_fail (MOO_IS_FOLDER (folder), NULL);

    if (parent)
        parent = gtk_widget_get_toplevel (parent);
    if (!GTK_IS_WINDOW (parent))
        parent = NULL;

    xml = glade_xml_new (MOO_CREATE_FOLDER_GLADE_FILE, NULL, NULL);

    if (!xml)
        g_error ("Yes, glade is great usually, but not always");

    dialog = glade_xml_get_widget (xml, "dialog");
    entry = glade_xml_get_widget (xml, "entry");
    label = glade_xml_get_widget (xml, "location_label");
    g_assert (dialog != NULL && entry != NULL && label != NULL);

    path = g_filename_display_name (moo_folder_get_path (folder));
    gtk_label_set_text (GTK_LABEL (label), path);
    g_free (path);

    gtk_entry_set_text (GTK_ENTRY (entry), "New Folder");
    gtk_editable_select_region (GTK_EDITABLE (entry), 0, -1);
    gtk_widget_grab_focus (entry);

    if (parent)
        gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (parent));

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK)
        new_folder_name = g_strdup (gtk_entry_get_text (GTK_ENTRY (entry)));

    gtk_widget_destroy (dialog);
    g_object_unref (xml);
    return new_folder_name;
}
