#ifndef MEDIT_LUA_H
#define MEDIT_LUA_H

#include "moolua/lua/moolua.h"

#ifdef __cplusplus

bool         medit_lua_setup        (lua_State *L,
                                     bool       default_init);

lua_State   *medit_lua_new          (bool        default_init);
void         medit_lua_free         (lua_State  *L);

bool         medit_lua_do_string    (lua_State  *L,
                                     const char *string);
bool         medit_lua_do_file      (lua_State  *L,
                                     const char *filename);

#endif // __cplusplus

#ifdef __cplusplus
extern "C" {
#endif

void         medit_lua_run_string   (const char *string);
void         medit_lua_run_file     (const char *filename);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // MEDIT_LUA_H
