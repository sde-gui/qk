/*
 *   mooutils/moofilechooser.c
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

#include "mooutils/moofilechooser.h"
#include <string.h>


/* MOO_TYPE_FILE_CHOOSER */
G_DEFINE_TYPE (MooFileChooser, moo_file_chooser, GTK_TYPE_FILE_CHOOSER_DIALOG)

static void     moo_file_chooser_finalize       (GObject *object);
static GObject *moo_file_chooser_constructor    (GType type,
                                                 guint n_props,
                                                 GObjectConstructParam *props);

static void     screw_file_chooser  (MooFileChooser *dialog);


static void     moo_file_chooser_class_init (MooFileChooserClass    *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    gobject_class->constructor = moo_file_chooser_constructor;
    gobject_class->finalize = moo_file_chooser_finalize;
}


static void     moo_file_chooser_init       (MooFileChooser *dialog)
{
    dialog->shortcuts_vbox = NULL;
}


static void     moo_file_chooser_finalize       (GObject *object)
{
    MooFileChooser *dialog = MOO_FILE_CHOOSER (object);

    if (dialog->shortcuts_vbox)
        g_object_unref (dialog->shortcuts_vbox);
    dialog->shortcuts_vbox = NULL;

    G_OBJECT_CLASS(moo_file_chooser_parent_class)->finalize (object);
}


static GObject *moo_file_chooser_constructor    (GType type,
                                                 guint n_props,
                                                 GObjectConstructParam *props)
{
    GObject *object;
    MooFileChooser *dialog;

    object = G_OBJECT_CLASS(moo_file_chooser_parent_class)->constructor (type,
                                                                n_props, props);
    dialog = MOO_FILE_CHOOSER (object);

    screw_file_chooser (dialog);

    if (dialog->shortcuts_vbox)
    {
        GtkWidget *parent = dialog->shortcuts_vbox->parent;
        g_return_val_if_fail (parent != NULL, object);
        gtk_container_remove (GTK_CONTAINER (parent),
                              dialog->shortcuts_vbox);
    }

    return object;
}


static GtkWidget *moo_file_chooser_new_valist (const char           *title,
                                               GtkWindow            *parent,
                                               GtkFileChooserAction  action,
                                               const char           *backend,
                                               const char           *first_button_text,
                                               va_list               args)
{
    GtkWidget *dialog;
    const char *button_text = first_button_text;
    int response_id;

    dialog = g_object_new (MOO_TYPE_FILE_CHOOSER,
                           "title", title,
                           "action", action,
                           "file-system-backend", backend,
                           NULL);

    if (parent)
        gtk_window_set_transient_for (GTK_WINDOW (dialog), parent);

    while (button_text)
    {
        response_id = va_arg (args, int);
        gtk_dialog_add_button (GTK_DIALOG (dialog), button_text, response_id);
        button_text = va_arg (args, const char *);
    }

    return dialog;
}


GtkWidget   *moo_file_chooser_new       (const char             *title,
                                         GtkWindow              *parent,
                                         GtkFileChooserAction    action,
                                         const gchar            *first_button_text,
                                         ...)
{
    GtkWidget *dialog;
    va_list args;

    va_start (args, first_button_text);
    dialog = moo_file_chooser_new_valist (title, parent, action,
                                          NULL, first_button_text,
                                          args);
    va_end (args);

    return dialog;
}


#define LIST_FREE(l) g_list_free (l); l = NULL

static void     screw_file_chooser  (MooFileChooser *dialog)
{
    /*
        dialog -> vbox -> GtkFileChooserWidget -> GtkFileChooserDefault

        GtkFileChooserDefault is a vbox containing three children -
            save mode stuff; browse stuff; extra widget

        browse_stuff: vbox -> hpaned
        hpaned contains shortcuts widgets and files treeview
    */

    GtkWidget *vbox, *filechooser, *filechooser_default;
    GtkWidget *browse_stuff, *hpaned;
    GtkWidget *shortcuts_vbox;
    GList *children;

    vbox = GTK_DIALOG(dialog)->vbox;

    children = gtk_container_get_children (GTK_CONTAINER (vbox));
    g_return_if_fail (children != NULL);
    filechooser = children->data;
    LIST_FREE(children);
    g_return_if_fail (GTK_IS_FILE_CHOOSER_WIDGET (filechooser));

    children = gtk_container_get_children (GTK_CONTAINER (filechooser));
    g_return_if_fail (children != NULL);
    filechooser_default = children->data;
    LIST_FREE(children);
    g_return_if_fail (filechooser_default != NULL &&
            !strcmp (g_type_name (G_OBJECT_TYPE (filechooser_default)), "GtkFileChooserDefault"));

    children = gtk_container_get_children (GTK_CONTAINER (filechooser_default));
    g_return_if_fail (children != NULL);
    if (!children->next)
    {
        g_list_free (children);
        g_return_if_reached ();
    }
    browse_stuff = children->next->data;
    LIST_FREE(children);
    g_return_if_fail (GTK_IS_VBOX (browse_stuff));

    children = gtk_container_get_children (GTK_CONTAINER (browse_stuff));
    g_return_if_fail (children != NULL);
    hpaned = children->data;
    LIST_FREE(children);
    g_return_if_fail (GTK_IS_HPANED (hpaned));

    shortcuts_vbox = gtk_paned_get_child1 (GTK_PANED (hpaned));
    g_return_if_fail (GTK_IS_VBOX (shortcuts_vbox));

#if 0
    dialog->shortcuts_vbox = shortcuts_vbox;
    g_object_ref (shortcuts_vbox);
#endif
}
