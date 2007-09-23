/*
 *   moogrep.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_EDIT_PLUGINS_H
#define MOO_EDIT_PLUGINS_H

#include <glib.h>

G_BEGIN_DECLS


#ifndef __WIN32__
gboolean _moo_find_plugin_init              (void);
gboolean _moo_ctags_plugin_init             (void);
#endif

gboolean _moo_active_strings_plugin_init    (void);
gboolean _moo_completion_plugin_init        (void);
gboolean _moo_file_selector_plugin_init     (void);


G_END_DECLS

#endif /* MOO_EDIT_PLUGINS_H */
