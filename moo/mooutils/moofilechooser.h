/*
 *   mooutils/moofilechooser.h
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

#ifndef MOOUTILS_MOOFILECHOOSER_H
#define MOOUTILS_MOOFILECHOOSER_H

#include <gtk/gtk.h>

G_BEGIN_DECLS


#define MOO_TYPE_FILE_CHOOSER             (moo_file_chooser_get_type ())
#define MOO_FILE_CHOOSER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOO_TYPE_FILE_CHOOSER, MooFileChooser))
#define MOO_FILE_CHOOSER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_FILE_CHOOSER, MooFileChooserClass))
#define MOO_IS_FILE_CHOOSER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOO_TYPE_FILE_CHOOSER))
#define MOO_IS_FILE_CHOOSER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_FILE_CHOOSER))
#define MOO_FILE_CHOOSER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_FILE_CHOOSER, MooFileChooserClass))

typedef struct _MooFileChooser        MooFileChooser;
typedef struct _MooFileChooserClass   MooFileChooserClass;

struct _MooFileChooser {
    GtkFileChooserDialog parent;
    GtkWidget   *shortcuts_vbox;
};

struct _MooFileChooserClass {
    GtkFileChooserDialogClass parent_class;
};


GType        moo_file_chooser_get_type  (void) G_GNUC_CONST;

GtkWidget   *moo_file_chooser_new       (const char *title,
                                         GtkWindow *parent,
                                         GtkFileChooserAction action,
                                         const gchar *first_button_text,
                                         ...);


G_END_DECLS

#endif /* MOOUTILS_MOOFILECHOOSER_H */
