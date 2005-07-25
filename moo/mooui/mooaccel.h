/*
 *   mooui/mooaccel.h
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

#ifndef MOOUI_MOOACCEL_H
#define MOOUI_MOOACCEL_H

#include <glib.h>

G_BEGIN_DECLS


void         moo_prefs_set_accel            (const char     *accel_path,
                                             const char     *accel);
const char  *moo_prefs_get_accel            (const char     *accel_path);

void         moo_set_accel                  (const char     *accel_path,
                                             const char     *accel);
const char  *moo_get_accel                  (const char     *accel_path);
void         moo_set_default_accel          (const char     *accel_path,
                                             const char     *accel);
const char  *moo_get_default_accel          (const char     *accel_path);

char        *moo_get_accel_label            (const char     *accel);


G_END_DECLS

#endif /* MOOUI_MOOACCEL_H */
