/*
 * "ex" API implementation
 * http://lua-users.org/wiki/ExtensionProposal
 * Copyright 2007 Mark Edgar < medgar at student gc maricopa edu >
 */
#ifndef SPAWN_H
#define SPAWN_H

#include <stdio.h>
#include "lua.h"

#define spawn_param_init _moo_lua_ex_spawn_param_init
#define spawn_param_filename _moo_lua_ex_spawn_param_filename
#define spawn_param_args _moo_lua_ex_spawn_param_args
#define spawn_param_env _moo_lua_ex_spawn_param_env
#define spawn_param_redirect _moo_lua_ex_spawn_param_redirect
#define spawn_param_execute _moo_lua_ex_spawn_param_execute
#define process_wait _moo_lua_ex_process_wait
#define process_tostring _moo_lua_ex_process_tostring

#define PROCESS_HANDLE "process"
struct process;
struct spawn_params;

struct spawn_params *spawn_param_init(lua_State *L);
void spawn_param_filename(struct spawn_params *p, const char *filename);
void spawn_param_args(struct spawn_params *p);
void spawn_param_env(struct spawn_params *p);
void spawn_param_redirect(struct spawn_params *p, const char *stdname, int fd);
int spawn_param_execute(struct spawn_params *p);

int process_wait(lua_State *L);
int process_tostring(lua_State *L);

#endif/*SPAWN_H*/
