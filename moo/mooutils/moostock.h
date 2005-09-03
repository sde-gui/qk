/*
 *   mooutils/moostock.h
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

#ifndef MOOUTILS_STOCK_H
#define MOOUTILS_STOCK_H

#include <gtk/gtk.h>

G_BEGIN_DECLS


#define MOO_STOCK_APP                   "moo-app"
#define MOO_STOCK_GAP                   "moo-gap"
#define MOO_STOCK_TERMINAL              "moo-terminal"
#define MOO_STOCK_TERMINAL              "moo-terminal"
#define MOO_STOCK_KEYBOARD              GTK_STOCK_SELECT_FONT
#define MOO_STOCK_CLOSE                 "moo-close"
#define MOO_STOCK_STICKY                "moo-sticky"
#define MOO_STOCK_DETACH                "moo-detach"
#define MOO_STOCK_ATTACH                "moo-attach"
#define MOO_STOCK_KEEP_ON_TOP           "moo-keep-on-top"

#define MOO_STOCK_DOC_DELETED           GTK_STOCK_DIALOG_ERROR
#define MOO_STOCK_DOC_MODIFIED_ON_DISK  GTK_STOCK_DIALOG_WARNING
#define MOO_STOCK_DOC_MODIFIED          GTK_STOCK_SAVE

#define MOO_STOCK_SAVE_NONE             "moo-save-none"
#define MOO_STOCK_SAVE_SELECTED         "moo-save-selected"

#define MOO_ICON_SIZE_REAL_SMALL        (moo_get_icon_size_real_small ())

void        moo_create_stock_items          (void);
GtkIconSize moo_get_icon_size_real_small    (void) G_GNUC_CONST;


G_END_DECLS

#endif /* MOOUTILS_STOCK_H */
