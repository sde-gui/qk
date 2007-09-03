#ifndef MOO_LUA_H
#define MOO_LUA_H

#include "lualib.h"
#include "lauxlib.h"

#define L_RETURN_BOOL(val)      \
G_STMT_START {                  \
    lua_pushboolean (L, val);   \
    return 1;                   \
} G_STMT_END

#endif /* MOO_LUA_H */
