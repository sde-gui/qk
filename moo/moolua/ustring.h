/* This is not a part of the Lua distribution */

#ifndef USTRING_H
#define USTRING_H

#include "lua.h"


int          luaopen_ustring        (lua_State  *L);

void         lua_take_utf8string    (lua_State  *L,
                                     char       *s);
void         lua_push_utf8string    (lua_State  *L,
                                     const char *s,
                                     int         len);
const char  *lua_check_utf8string   (lua_State  *L,
                                     int         numArg,
                                     size_t     *len);


#endif /* USTRING_H */
