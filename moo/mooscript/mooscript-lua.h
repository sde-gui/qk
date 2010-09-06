#ifndef MOM_SCRIPT_LUA_H
#define MOM_SCRIPT_LUA_H

#include <moolua/moolua.h>
#include "mooscript-api.h"

namespace mom {

bool lua_setup(lua_State *L) throw();
void lua_cleanup(lua_State *L) throw();

} // namespace mom

#endif // MOM_SCRIPT_LUA_H
