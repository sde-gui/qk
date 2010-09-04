/*
 *   mooedit-lua1.h
 *
 *   Copyright (C) 2004-2010 by Yevgen Muntyan <emuntyan@sourceforge.net>
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

#ifndef MOO_EDIT_LUA1_H
#define MOO_EDIT_LUA1_H

#include <mooedit/mooedit.h>
#include <moolua/moolua.h>

G_BEGIN_DECLS


void    _moo_edit_lua1_add_api  (lua_State      *L);

void    _moo_edit_lua1_set_doc  (lua_State      *L,
                                 GtkTextView    *doc);
void    _moo_edit_lua1_cleanup  (lua_State      *L);

void    _moo_lua1_push_object   (lua_State      *L,
                                 GObject        *object);


G_END_DECLS

#endif /* MOO_EDIT_LUA1_H */
