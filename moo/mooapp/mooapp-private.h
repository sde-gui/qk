/*
 *   mooapp/mooapp-private.h
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

#ifndef MOOAPP_MOOAPP_PRIVATE_H
#define MOOAPP_MOOAPP_PRIVATE_H

#include "mooapp/mooapp.h"
#include "mooutils/mooprefsdialog.h"

G_BEGIN_DECLS


GtkWidget *_moo_app_create_prefs_dialog (MooApp *app);


G_END_DECLS

#endif /* MOOAPP_MOOAPP_PRIVATE_H */
