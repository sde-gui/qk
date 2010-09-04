/*
 *   mooeditplugins.c
 *
 *   Copyright (C) 2004-2009 by Yevgen Muntyan <muntyan@tamu.edu>
 *
 *   This file is part of medit.  medit is free software; you can
 *   redistribute it and/or modify it under the terms of the
 *   GNU Lesser General Public License as published by the
 *   Free Software Foundation; either version 2.1 of the License,
 *   or (at your option) any later version.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with medit.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>
#include "mooeditplugins.h"

void initmoo (void);

void
moo_plugin_init_builtin (void)
{
    _moo_file_selector_plugin_init ();
    _moo_file_list_plugin_init ();
    _moo_find_plugin_init ();
#ifdef MOO_BUILD_CTAGS
    _moo_ctags_plugin_init ();
#endif
    _moo_user_tools_plugin_init ();
}
