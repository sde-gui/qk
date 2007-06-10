/*
 *   mooeditfiltersettings.h
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

#ifndef MOOEDIT_COMPILATION
#error "This file may not be used"
#endif

#ifndef MOO_EDIT_FILTER_SETTINGS_H
#define MOO_EDIT_FILTER_SETTINGS_H

#include <glib.h>

G_BEGIN_DECLS


void        _moo_edit_filter_settings_load              (void);

GSList     *_moo_edit_filter_settings_get_strings       (void);
void        _moo_edit_filter_settings_set_strings       (GSList     *strings);

const char *_moo_edit_filter_settings_get_for_file      (const char *filename);


G_END_DECLS

#endif /* MOO_EDIT_FILTER_SETTINGS_H */
