/*
 *   moofileview-dialogs.h
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

#ifndef __MOO_FILE_VIEW_DIALOGS_H__
#define __MOO_FILE_VIEW_DIALOGS_H__

#include "moofile.h"

G_BEGIN_DECLS


GtkWidget  *moo_file_props_dialog_new       (GtkWidget  *parent);
void        moo_file_props_dialog_set_file  (GtkWidget  *dialog,
                                             MooFile    *file,
                                             MooFolder  *folder);

char       *moo_create_folder_dialog        (GtkWidget  *parent,
                                             MooFolder  *folder);


G_END_DECLS

#endif /* __MOO_FILE_VIEW_DIALOGS_H__ */
