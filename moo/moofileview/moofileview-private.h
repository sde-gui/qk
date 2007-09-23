/*
 *   moofileview-private.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_FILE_VIEW_COMPILATION
#error "This file may not be included"
#endif

#include <moofileview/moofileview-impl.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkmenu.h>

#ifndef MOO_FILE_VIEW_PRIVATE_H
#define MOO_FILE_VIEW_PRIVATE_H


#define MOO_TYPE_FILE_VIEW_TYPE         (_moo_file_view_type_get_type ())

typedef enum {
    MOO_FILE_VIEW_LIST,
    MOO_FILE_VIEW_ICON,
    MOO_FILE_VIEW_BOOKMARK
} MooFileViewType;


GType       _moo_file_view_type_get_type                (void) G_GNUC_CONST;

/* returns list of MooFile* pointers, must be freed, and elements must be unref'ed */
GList      *_moo_file_view_get_files                    (MooFileView    *fileview);


#endif /* MOO_FILE_VIEW_PRIVATE_H */
