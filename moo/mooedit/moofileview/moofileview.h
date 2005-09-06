/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *   moofileview.h
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

#ifndef __MOO_FILE_VIEW_H__
#define __MOO_FILE_VIEW_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS


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
typedef struct _MooFileViewPrivate  MooFileViewPrivate;
typedef struct _MooFileViewClass    MooFileViewClass;

struct _MooFileView
{
    GtkVBox             vbox;
    GtkWidget          *toolbar;
    MooFileViewPrivate *priv;
};

struct _MooFileViewClass
{
    GtkVBoxClass        vbox_class;

    gboolean    (*chdir)            (MooFileView        *fileview,
                                     const char         *dir,
                                     GError            **error);
    void        (*populate_popup)   (MooFileView        *fileview,
                                     GList              *selected,
                                     GtkMenu            *menu);
};


GType       moo_file_view_get_type                      (void) G_GNUC_CONST;
GType       moo_file_view_type_get_type                 (void) G_GNUC_CONST;

GtkWidget  *moo_file_view_new                           (void);

gboolean    moo_file_view_chdir                         (MooFileView    *fileview,
                                                         const char     *dir,
                                                         GError        **error);

void        moo_file_view_select_name                   (MooFileView    *fileview,
                                                         const char     *name);
void        moo_file_view_select_display_name           (MooFileView    *fileview,
                                                         const char     *name);

void        moo_file_view_set_view_type                 (MooFileView    *fileview,
                                                         MooFileViewType type);

void        moo_file_view_set_show_hidden               (MooFileView    *fileview,
                                                         gboolean        show);
void        moo_file_view_set_show_parent               (MooFileView    *fileview,
                                                         gboolean        show);
void        moo_file_view_set_sort_case_sensitive       (MooFileView    *fileview,
                                                         gboolean        case_sensitive);
void        moo_file_view_set_typeahead_case_sensitive  (MooFileView    *fileview,
                                                         gboolean        case_sensitive);


G_END_DECLS

#endif /* __MOO_FILE_VIEW_H__ */
