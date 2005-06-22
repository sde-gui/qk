/*
 *   mooui/mooshortcutsprefs.h
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

#ifndef MOOUI_MOOSHORTCUTSPREFS_H
#define MOOUI_MOOSHORTCUTSPREFS_H

#include "mooui/mooactiongroup.h"
#include "mooutils/mooprefsdialog.h"

G_BEGIN_DECLS


#define MOO_ACCEL_PREFS_KEY "shortcuts"

GtkWidget   *moo_shortcuts_prefs_page_new           (MooActionGroup *group);
GtkWidget   *moo_shortcuts_prefs_dialog_new         (MooActionGroup *group);
void         moo_shortcuts_prefs_dialog_run         (MooActionGroup *group,
                                                     GtkWidget      *parent);


G_END_DECLS

#endif /* MOOUI_MOOSHORTCUTSPREFS_H */
