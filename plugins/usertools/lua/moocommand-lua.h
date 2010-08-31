/*
 *   moocommand-lua.h
 *
 *   Copyright (C) 2004-2008 by Yevgen Muntyan <muntyan@tamu.edu>
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

#ifndef MOO_COMMAND_LUA_H
#define MOO_COMMAND_LUA_H

#include "moocommand.h"

G_BEGIN_DECLS


#define MOO_TYPE_COMMAND_LUA             (_moo_command_lua_get_type ())
#define MOO_COMMAND_LUA(object)          (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_COMMAND_LUA, MooCommandLua))
#define MOO_COMMAND_LUA_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_COMMAND_LUA, MooCommandLuaClass))
#define MOO_IS_COMMAND_LUA(object)       (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_COMMAND_LUA))
#define MOO_IS_COMMAND_LUA_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_COMMAND_LUA))
#define MOO_COMMAND_LUA_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_COMMAND_LUA, MooCommandLuaClass))

typedef struct _MooCommandLua        MooCommandLua;
typedef struct _MooCommandLuaPrivate MooCommandLuaPrivate;
typedef struct _MooCommandLuaClass   MooCommandLuaClass;

struct _MooCommandLua {
    MooCommand base;
    MooCommandLuaPrivate *priv;
};

struct _MooCommandLuaClass {
    MooCommandClass base_class;
};


GType _moo_command_lua_get_type (void) G_GNUC_CONST;


G_END_DECLS

#endif /* MOO_COMMAND_LUA_H */
