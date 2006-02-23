/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *   moogrep.h
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

#ifndef __MOO_EDIT_PLUGINS_H__
#define __MOO_EDIT_PLUGINS_H__

#include <gtk/gtkversion.h>

G_BEGIN_DECLS


#ifndef __WIN32__
gboolean _moo_find_plugin_init           (void);

#if GTK_CHECK_VERSION(2,6,0)
gboolean _moo_file_selector_plugin_init  (void);
#endif

#endif

gboolean _moo_active_strings_plugin_init (void);


G_END_DECLS

#endif /* __MOO_EDIT_PLUGINS_H__ */
