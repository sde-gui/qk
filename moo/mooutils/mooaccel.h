/*
 *   mooaccel.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_ACCEL_H
#define MOO_ACCEL_H

#include <glib.h>

G_BEGIN_DECLS


void         _moo_accel_register            (const char *accel_path,
                                             const char *default_accel);

const char  *_moo_get_accel                 (const char *accel_path);
const char  *_moo_get_default_accel         (const char *accel_path);

void         _moo_modify_accel              (const char *accel_path,
                                             const char *new_accel);

char        *_moo_get_accel_label           (const char *accel);


G_END_DECLS

#endif /* MOO_ACCEL_H */
