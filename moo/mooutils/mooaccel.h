/*
 *   mooaccel.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_ACCEL_H
#define MOO_ACCEL_H

#include <gtk/gtk.h>

G_BEGIN_DECLS


void         _moo_accel_register            (const char *accel_path,
                                             const char *default_accel);

const char  *_moo_get_accel                 (const char *accel_path);
const char  *_moo_get_default_accel         (const char *accel_path);

void         _moo_modify_accel              (const char *accel_path,
                                             const char *new_accel);

char        *_moo_get_accel_label           (const char *accel);


#ifndef GDK_WINDOWING_QUARTZ

#define MOO_ACCEL_CTRL "<Ctrl>"
#define MOO_ACCEL_CTRL_MASK GDK_CONTROL_MASK

#else /* GDK_WINDOWING_QUARTZ */

#define MOO_ACCEL_CTRL "<Meta>"
#define MOO_ACCEL_CTRL_MASK GDK_META_MASK

#endif /* GDK_WINDOWING_QUARTZ */

#define MOO_ACCEL_NEW MOO_ACCEL_CTRL "N"
#define MOO_ACCEL_OPEN MOO_ACCEL_CTRL "O"
#define MOO_ACCEL_SAVE MOO_ACCEL_CTRL "S"
#define MOO_ACCEL_SAVE_AS MOO_ACCEL_CTRL "<Shift>S"
#define MOO_ACCEL_CLOSE MOO_ACCEL_CTRL "W"

#define MOO_ACCEL_UNDO MOO_ACCEL_CTRL "Z"
#define MOO_ACCEL_REDO MOO_ACCEL_CTRL "<Shift>Z"
#define MOO_ACCEL_CUT MOO_ACCEL_CTRL "X"
#define MOO_ACCEL_COPY MOO_ACCEL_CTRL "C"
#define MOO_ACCEL_PASTE MOO_ACCEL_CTRL "V"
#define MOO_ACCEL_SELECT_ALL MOO_ACCEL_CTRL "A"

#define MOO_ACCEL_PAGE_SETUP MOO_ACCEL_CTRL "<Shift>P"
#define MOO_ACCEL_PRINT MOO_ACCEL_CTRL "P"


G_END_DECLS

#endif /* MOO_ACCEL_H */
