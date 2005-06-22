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

#ifdef __cplusplus
extern "C" {
#endif


#define MOO_STOCK_APP                   "moo-app"
#define MOO_STOCK_GAP                   "moo-gap"
#define MOO_STOCK_TERMINAL              "moo-terminal"
#define MOO_STOCK_TERMINAL              "moo-terminal"
#define MOO_STOCK_KEYBOARD              GTK_STOCK_SELECT_FONT
#define MOO_STOCK_DOC_DELETED           GTK_STOCK_DIALOG_ERROR
#define MOO_STOCK_DOC_MODIFIED_ON_DISK  GTK_STOCK_DIALOG_WARNING


void moo_create_stock_items (void);


#ifdef __cplusplus
} // extern "C"
#endif

#endif /* MOOUTILS_STOCK_H */
