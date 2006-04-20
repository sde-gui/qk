/*
 *   moousertools.h
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

#ifndef __MOO_USER_TOOLS_H__
#define __MOO_USER_TOOLS_H__

#include <mooutils/moouixml.h>

G_BEGIN_DECLS


#define MOO_USER_TOOLS_FILE     "tools.cfg"
#define MOO_USER_TOOLS_ADD_FILE "tools-add.cfg"
#define MOO_USER_MENU_FILE      "menu.cfg"
#define MOO_USER_MENU_ADD_FILE  "menu-add.cfg"


char  **moo_edit_get_user_tools_files   (guint      *n_files);
void    moo_edit_load_user_tools        (char      **files,
                                         guint       n_files,
                                         MooUIXML   *xml,
                                         const char *ui_path);

char  **moo_edit_get_user_menu_files    (guint      *n_files);
void    moo_edit_load_user_menu         (char      **files,
                                         guint       n_files,
                                         MooUIXML   *xml,
                                         const char *start_path,
                                         const char *end_path);


G_END_DECLS

#endif /* __MOO_USER_TOOLS_H__ */
