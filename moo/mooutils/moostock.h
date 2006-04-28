/*
 *   mooutils/moostock.h
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

#ifndef MOOUTILS_STOCK_H
#define MOOUTILS_STOCK_H

#include <gtk/gtkstock.h>
#include <gtk/gtkiconfactory.h>

G_BEGIN_DECLS


#define MOO_STOCK_MEDIT                 "moo-medit"
#define MOO_STOCK_GGAP                  "moo-ggap"
#define MOO_STOCK_GAP                   "moo-gap"
#define MOO_STOCK_TERMINAL              "moo-terminal"
#define MOO_STOCK_KEYBOARD              GTK_STOCK_SELECT_FONT
#define MOO_STOCK_MENU                  GTK_STOCK_INDEX
#define MOO_STOCK_RESTART               GTK_STOCK_REFRESH
#define MOO_STOCK_CLOSE                 "moo-close"
#define MOO_STOCK_STICKY                "moo-sticky"
#define MOO_STOCK_DETACH                "moo-detach"
#define MOO_STOCK_ATTACH                "moo-attach"
#define MOO_STOCK_KEEP_ON_TOP           "moo-keep-on-top"

#define MOO_STOCK_DOC_DELETED           GTK_STOCK_DIALOG_ERROR
#define MOO_STOCK_DOC_MODIFIED_ON_DISK  GTK_STOCK_DIALOG_WARNING
#define MOO_STOCK_DOC_MODIFIED          GTK_STOCK_SAVE

#define MOO_STOCK_FILE_SELECTOR         "gtk-directory"

#define MOO_STOCK_SAVE_NONE             "moo-save-none"
#define MOO_STOCK_SAVE_SELECTED         "moo-save-selected"

#define MOO_ICON_SIZE_REAL_SMALL        (moo_get_icon_size_real_small ())

#define MOO_STOCK_NEW_PROJECT           "moo-new-project"
#define MOO_STOCK_OPEN_PROJECT          "moo-open-project"
#define MOO_STOCK_CLOSE_PROJECT         "moo-close-project"
#define MOO_STOCK_PROJECT_OPTIONS       "moo-project-options"
#define MOO_STOCK_BUILD                 "moo-build"
#define MOO_STOCK_COMPILE               "moo-compile"
#define MOO_STOCK_EXECUTE               "moo-execute"

#define MOO_STOCK_FIND_IN_FILES         "moo-find-in-files"
#define MOO_STOCK_FIND_FILE             "moo-find-file"

#define MOO_STOCK_FILE_COPY             "moo-file-copy"
#define MOO_STOCK_FILE_MOVE             "moo-file-move"
#define MOO_STOCK_FILE_LINK             "moo-file-link"
#define MOO_STOCK_FILE_SAVE_AS          "moo-file-save-as"
#define MOO_STOCK_FILE_SAVE_COPY        "moo-file-save-copy"

#define MOO_STOCK_EDIT_BOOKMARK         "moo-edit-bookmark"

#define MOO_STOCK_PLUGINS               GTK_STOCK_PREFERENCES


void        moo_create_stock_items          (void);
GtkIconSize moo_get_icon_size_real_small    (void) G_GNUC_CONST;


G_END_DECLS

#endif /* MOOUTILS_STOCK_H */
