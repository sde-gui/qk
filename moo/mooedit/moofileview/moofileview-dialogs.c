/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
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

#define MOO_FILE_SYSTEM_COMPILATION
#include "moofileview-dialogs.h"
#include "moofilesystem.h"
#include <time.h>
#include <string.h>

#define MAGIC_STRING "moo-file-props-dialog-magic-string"
#define MAGIC_NUMBER (GINT_TO_POINTER (87654321))


static void dialog_response             (GtkWidget  *dialog,
                                         gint        response);
static void dialog_ok                   (GtkWidget  *dialog);


GtkWidget  *moo_file_props_dialog_new   (GtkWidget  *parent)
{
    GtkWidget *dialog, *notebook, *alignment, *vbox, *hbox, *label, *button;
    GtkWidget *table, *icon, *entry;

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

    notebook = gtk_notebook_new ();
    gtk_widget_show (notebook);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG(dialog)->vbox), notebook, TRUE, TRUE, 0);

    alignment = gtk_alignment_new (0.5, 0.5, 1.0, 1.0);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 6, 6, 6, 6);
    gtk_widget_show (alignment);
    label = gtk_label_new ("General");
    gtk_widget_show (alignment);
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), alignment, label);

    vbox = gtk_vbox_new (FALSE, 6);
    gtk_widget_show (vbox);
    gtk_container_add (GTK_CONTAINER (alignment), vbox);

    button = gtk_button_new ();
    icon = gtk_image_new ();
    gtk_container_add (GTK_CONTAINER (button), icon);
    entry = gtk_entry_new ();

    hbox = gtk_hbox_new (FALSE, 6);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 0);
    gtk_widget_show_all (hbox);

    table = gtk_table_new (1, 2, FALSE);
    gtk_table_set_row_spacings (GTK_TABLE (table), 6);
    gtk_table_set_col_spacings (GTK_TABLE (table), 6);
    gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);

    gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
    g_object_set_data (G_OBJECT (dialog), MAGIC_STRING, MAGIC_NUMBER);
    g_signal_connect (dialog, "response", G_CALLBACK (dialog_response), NULL);

    gtk_widget_grab_focus (entry);

    g_object_set_data (G_OBJECT (dialog), "moo-dialog-icon", icon);
    g_object_set_data (G_OBJECT (dialog), "moo-dialog-entry", entry);
    g_object_set_data (G_OBJECT (dialog), "moo-dialog-table", table);
    g_object_set_data (G_OBJECT (dialog), "moo-dialog-notebook", notebook);

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

    entry = g_object_get_data (G_OBJECT (dialog), "moo-dialog-entry");
    g_return_if_fail (GTK_IS_ENTRY (entry));

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
    GtkWidget *entry = g_object_get_data (G_OBJECT (dialog), "moo-dialog-entry");
    g_return_if_fail (GTK_IS_ENTRY (entry));

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


static void set_icon        (GtkWidget  *dialog,
                             GdkPixbuf  *icon)
{
    GtkImage *image = g_object_get_data (G_OBJECT (dialog), "moo-dialog-icon");
    g_return_if_fail (GTK_IS_IMAGE (image));
    gtk_image_set_from_pixbuf (image, icon);
}


static void erase_table (GtkWidget *table)
{
    gtk_container_foreach (GTK_CONTAINER (table),
                           (GtkCallback) gtk_widget_destroy, NULL);
}


void        moo_file_props_dialog_set_file      (GtkWidget      *dialog,
                                                 MooFile        *file,
                                                 MooFolder      *folder)
{
    char *text;
    GtkWidget *notebook, *table;
    char **info, **p;
    int i;

    g_return_if_fail (GTK_IS_DIALOG (dialog));
    g_return_if_fail ((!file && !folder) || (file && MOO_IS_FOLDER (folder)));
    g_return_if_fail (g_object_get_data (G_OBJECT (dialog), MAGIC_STRING) == MAGIC_NUMBER);

    notebook = g_object_get_data (G_OBJECT (dialog), "moo-dialog-notebook");
    table = g_object_get_data (G_OBJECT (dialog), "moo-dialog-table");
    g_return_if_fail (GTK_IS_NOTEBOOK (notebook));
    g_return_if_fail (GTK_IS_TABLE (table));

    if (!file)
    {
        gtk_widget_set_sensitive (notebook, FALSE);
        erase_table (table);
        set_file (dialog, NULL, NULL);
        return;
    }
    else
    {
        gtk_widget_set_sensitive (notebook, TRUE);
    }

    info = moo_folder_get_file_info (folder, file);
    g_return_if_fail (info != NULL);

    set_icon (dialog, moo_file_get_icon (file, dialog, GTK_ICON_SIZE_DIALOG));
    set_file (dialog, file, folder);

    for (p = info, i = 0; *p != NULL; ++i)
    {
        GtkWidget *label;

        if (!p[1])
        {
            g_critical ("%s: oops", G_STRLOC);
            break;
        }

        label = gtk_label_new (NULL);
        text = g_markup_printf_escaped ("<b>%s</b>", *(p++));
        gtk_label_set_markup (GTK_LABEL (label), text);
        g_free (text);
        gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i+1,
                          GTK_EXPAND | GTK_FILL, 0, 0, 0);

        label = gtk_label_new (*(p++));
        gtk_label_set_selectable (GTK_LABEL (label), TRUE);
        gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
        gtk_table_attach (GTK_TABLE (table), label, 1, 2, i, i+1,
                          GTK_EXPAND | GTK_FILL, 0, 0, 0);
    }

    gtk_widget_show_all (table);

    g_strfreev (info);
}



char       *moo_create_folder_dialog        (GtkWidget  *parent,
                                             MooFolder  *folder)
{
    GtkWidget *dialog, *entry, *label, *alignment, *vbox;
    char *text, *path, *new_folder_name = NULL;

    g_return_val_if_fail (MOO_IS_FOLDER (folder), NULL);

    if (parent)
        parent = gtk_widget_get_toplevel (parent);
    if (!GTK_IS_WINDOW (parent))
        parent = NULL;

    dialog = gtk_dialog_new_with_buttons ("New Folder",
                                          parent ? GTK_WINDOW (parent) : NULL,
                                          GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_OK, GTK_RESPONSE_OK,
                                          NULL);

    alignment = gtk_alignment_new (0.5, 0.5, 1.0, 1.0);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 6, 6, 6, 6);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG(dialog)->vbox), alignment, TRUE, TRUE, 0);
    vbox = gtk_vbox_new (FALSE, 6);
    gtk_container_add (GTK_CONTAINER (alignment), vbox);

    label = gtk_label_new (NULL);
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

    path = g_filename_display_name (moo_folder_get_path (folder));
    text = g_strdup_printf ("Create new folder in %s", path);
    gtk_label_set_text (GTK_LABEL (label), text);
    g_free (path);
    g_free (text);

    entry = gtk_entry_new ();
    gtk_box_pack_start (GTK_BOX (vbox), entry, FALSE, FALSE, 0);

    if (parent)
        gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (parent));

    gtk_entry_set_text (GTK_ENTRY (entry), "New Folder");
    gtk_widget_show_all (dialog);
    gtk_widget_grab_focus (entry);
    gtk_editable_select_region (GTK_EDITABLE (entry), 0, -1);

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK)
        new_folder_name = g_strdup (gtk_entry_get_text (GTK_ENTRY (entry)));

    gtk_widget_destroy (dialog);
    return new_folder_name;
}
