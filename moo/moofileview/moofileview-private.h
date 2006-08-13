/*
 *   moofileview-private.h
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

#ifndef MOO_FILE_VIEW_COMPILATION
#error "This file may not be included"
#endif

#include <moofileview/moofileview-impl.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkmenu.h>

#ifndef __MOO_FILE_VIEW_PRIVATE_H__
#define __MOO_FILE_VIEW_PRIVATE_H__


#define MOO_TYPE_FILE_VIEW_TYPE         (_moo_file_view_type_get_type ())

typedef enum {
    MOO_FILE_VIEW_LIST,
    MOO_FILE_VIEW_ICON,
    MOO_FILE_VIEW_BOOKMARK
} MooFileViewType;


GType       _moo_file_view_type_get_type                (void) G_GNUC_CONST;

GtkWidget  *_moo_file_view_new                          (void);

void        _moo_file_view_select_display_name          (MooFileView    *fileview,
                                                         const char     *name);


void        _moo_file_view_set_view_type                (MooFileView    *fileview,
                                                         MooFileViewType type);

void        _moo_file_view_set_show_hidden              (MooFileView    *fileview,
                                                         gboolean        show);
void        _moo_file_view_set_show_parent              (MooFileView    *fileview,
                                                         gboolean        show);
void        _moo_file_view_set_sort_case_sensitive      (MooFileView    *fileview,
                                                         gboolean        case_sensitive);
void        _moo_file_view_set_typeahead_case_sensitive (MooFileView    *fileview,
                                                         gboolean        case_sensitive);

/* returns list of MooFile* pointers, must be freed, and elements must be unref'ed */
GList      *_moo_file_view_get_files                    (MooFileView    *fileview);


#endif /* __MOO_FILE_VIEW_PRIVATE_H__ */
