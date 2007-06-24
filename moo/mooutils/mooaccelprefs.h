/*
 *   mooaccelprefs.c
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

#ifndef MOO_ACCEL_PREFS_H
#define MOO_ACCEL_PREFS_H

#include <mooutils/mooactioncollection.h>

G_BEGIN_DECLS


void    _moo_accel_prefs_dialog_run (MooActionCollection    *coll,
                                     GtkWidget              *parent);


G_END_DECLS

#endif /* MOO_ACCEL_PREFS_H */
