/*
 *   mooutils/moofileview.h
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

#ifndef MOOUTILS_MOOFILEVIEW_H
#define MOOUTILS_MOOFILEVIEW_H

#include <gtk/gtk.h>

G_BEGIN_DECLS


#define MOO_TYPE_FILE_VIEW_FILE         (moo_file_view_file_get_type ())
#define MOO_TYPE_FILE_VIEW_TYPE         (moo_file_view_type_get_type ())
#define MOO_TYPE_FILE_VIEW              (moo_file_view_get_type ())
#define MOO_FILE_VIEW(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_FILE_VIEW, MooFileView))
#define MOO_FILE_VIEW_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_FILE_VIEW, MooFileViewClass))
#define MOO_IS_FILE_VIEW(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_FILE_VIEW))
#define MOO_IS_FILE_VIEW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_FILE_VIEW))
#define MOO_FILE_VIEW_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_FILE_VIEW, MooFileViewClass))


typedef enum {
    MOO_FILE_VIEW_LIST,
    MOO_FILE_VIEW_ICON
} MooFileViewType;

typedef struct _MooFileView         MooFileView;
typedef struct _MooFileViewFile     MooFileViewFile;
typedef struct _MooFileViewPrivate  MooFileViewPrivate;
typedef struct _MooFileViewClass    MooFileViewClass;

struct _MooFileView
{
    GtkVBox             vbox;
    MooFileViewPrivate *priv;
};

struct _MooFileViewClass
{
    GtkVBoxClass        vbox_class;

    gboolean    (*chdir)        (MooFileView        *fileview,
                                 const char         *dir,
                                 GError            **error);
    void        (*activate)     (MooFileView        *fileview,
                                 MooFileViewFile    *file);
    void        (*go_back)      (MooFileView        *fileview);
    void        (*go_forward)   (MooFileView        *fileview);
    void        (*go_home)      (MooFileView        *fileview);
    void        (*go_up)        (MooFileView        *fileview);
};


GType       moo_file_view_get_type          (void) G_GNUC_CONST;
GType       moo_file_view_file_get_type     (void) G_GNUC_CONST;
GType       moo_file_view_type_get_type     (void) G_GNUC_CONST;

GtkWidget  *moo_file_view_new               (void);

gboolean    moo_file_view_chdir             (MooFileView    *fileview,
                                             const char     *dir,
                                             GError        **error);

void        moo_file_view_set_view_type     (MooFileView    *fileview,
                                             MooFileViewType type);

gconstpointer moo_file_view_file_get_stat   (MooFileViewFile *file);
const char *moo_file_view_file_path         (MooFileViewFile *file);
const char *moo_file_view_file_mime_type    (MooFileViewFile *file);

GdkPixbuf  *moo_get_icon_for_file           (GtkWidget         *widget,
                                             MooFileViewFile   *file,
                                             GtkIconSize        size);


G_END_DECLS

#endif /* MOOUTILS_MOOFILEVIEW_H */
