/*
 *   mooedit-lua.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_EDIT_LUA_H
#define MOO_EDIT_LUA_H

#include <mooedit/mooedit.h>
#include <moolua/moolua.h>

G_BEGIN_DECLS


void    _moo_edit_lua_add_api   (lua_State      *L);

void    _moo_edit_lua_set_doc   (lua_State      *L,
                                 GtkTextView    *doc);
void    _moo_edit_lua_cleanup   (lua_State      *L);


G_END_DECLS

#endif /* MOO_EDIT_LUA_H */
