/*
 *   moofilechooser.c
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

#include "mooedit/plugins/moofilechooser.h"
#include "mooedit/plugins/moofilechooser-glade.h"
#include "mooutils/mooglade.h"
#include <gtk/gtk.h>


G_DEFINE_TYPE(MooFileChooser, moo_file_chooser, GTK_TYPE_DIALOG)


static void
fileview_activate (GtkDialog *dialog)
{
    gtk_dialog_response (dialog, GTK_RESPONSE_OK);
}


static void
moo_file_chooser_destroy (GtkObject *object)
{
    MooFileChooser *chooser = MOO_FILE_CHOOSER (object);

    if (chooser->fileview)
    {
        g_signal_handlers_disconnect_by_func (chooser->fileview,
                                              (gpointer) fileview_activate,
                                              chooser);
        chooser->fileview = NULL;
    }

    GTK_OBJECT_CLASS(moo_file_chooser_parent_class)->destroy (object);
}


static void
moo_file_chooser_class_init (MooFileChooserClass *klass)
{
    GtkObjectClass *gtkobject_class = GTK_OBJECT_CLASS (klass);
    gtkobject_class->destroy = moo_file_chooser_destroy;
}


static void
moo_file_chooser_init (MooFileChooser *chooser)
{
    MooGladeXML *xml;
    GtkDialog *dialog = GTK_DIALOG (chooser);

    xml = moo_glade_xml_new_empty ();
    moo_glade_xml_map_id (xml, "fileview", MOO_TYPE_FILE_VIEW);
    moo_glade_xml_fill_widget (xml, GTK_WIDGET (dialog),
                               MOO_FILE_CHOOSER_GLADE_XML, -1, "dialog");

    gtk_dialog_set_alternative_button_order (dialog, GTK_RESPONSE_OK,
                                             GTK_RESPONSE_CANCEL);
    chooser->fileview = moo_glade_xml_get_widget (xml, "fileview");

    g_signal_connect_swapped (chooser->fileview, "activate",
                              G_CALLBACK (fileview_activate),
                              chooser);
    g_signal_emit_by_name (chooser->fileview, "go-home");
}


GtkWidget *
moo_file_chooser_new (void)
{
    return g_object_new (MOO_TYPE_FILE_CHOOSER, NULL);
}
