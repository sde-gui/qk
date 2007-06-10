/*
 *   moofileview.h
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

#ifndef MOO_FILE_VIEW_H
#define MOO_FILE_VIEW_H

#include <mooutils/moouixml.h>

G_BEGIN_DECLS


#define MOO_TYPE_FILE_VIEW              (moo_file_view_get_type ())
#define MOO_FILE_VIEW(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_FILE_VIEW, MooFileView))
#define MOO_FILE_VIEW_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_FILE_VIEW, MooFileViewClass))
#define MOO_IS_FILE_VIEW(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_FILE_VIEW))
#define MOO_IS_FILE_VIEW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_FILE_VIEW))
#define MOO_FILE_VIEW_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_FILE_VIEW, MooFileViewClass))

typedef struct _MooFileView      MooFileView;
typedef struct _MooFileViewClass MooFileViewClass;

GType                   moo_file_view_get_type      (void) G_GNUC_CONST;

gboolean                moo_file_view_chdir         (MooFileView    *fileview,
                                                     const char     *dir,
                                                     GError        **error);

MooUIXML               *moo_file_view_get_ui_xml    (MooFileView    *fileview);
MooActionCollection    *moo_file_view_get_actions   (MooFileView    *fileview);


G_END_DECLS

#endif /* MOO_FILE_VIEW_H */
