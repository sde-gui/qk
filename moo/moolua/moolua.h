#ifndef MOO_LUA_H
#define MOO_LUA_H

#include "lualib.h"
#include "lauxlib.h"

#define L_RETURN_BOOL(val)      \
G_STMT_START {                  \
    lua_pushboolean (L, val);   \
    return 1;                   \
} G_STMT_END

void         lua_addpath            (lua_State   *L,
                                     char      **dirs,
                                     unsigned    n_dirs);

int          luaopen_unicode        (lua_State  *L);

void         lua_take_utf8string    (lua_State  *L,
                                     char       *s);
void         lua_push_utf8string    (lua_State  *L,
                                     const char *s,
                                     int         len);
const char  *lua_check_utf8string   (lua_State  *L,
                                     int         numArg,
                                     size_t     *len);

#endif /* MOO_LUA_H */
