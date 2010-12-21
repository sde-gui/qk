#ifndef MOO_LUA_API_UTIL_H
#define MOO_LUA_API_UTIL_H

#include "moolua/lua/lua.h"
#include "moolua/lua/lauxlib.h"
#include "mooutils/mooarray.h"
#include <gtk/gtk.h>

typedef int (*MooLuaMethod) (gpointer instance, lua_State *L, int first_arg);
typedef void (*MooLuaMetatableFunc) (lua_State *L);

typedef struct {
    const char *name;
    MooLuaMethod impl;
} MooLuaMethodEntry;

MooLuaMethod    moo_lua_lookup_method           (lua_State          *L,
                                                 GType               type,
                                                 const char         *meth);
void            moo_lua_register_methods        (GType               type,
                                                 MooLuaMethodEntry  *entries);

gpointer        moo_lua_get_arg_instance_opt    (lua_State          *L,
                                                 int                 narg,
                                                 const char         *param_name,
                                                 GType               type);
gpointer        moo_lua_get_arg_instance        (lua_State          *L,
                                                 int                 narg,
                                                 const char         *param_name,
                                                 GType               type);
MooObjectArray *moo_lua_get_arg_object_array    (lua_State          *L,
                                                 int                 narg,
                                                 const char         *param_name,
                                                 GType               type);
gboolean        moo_lua_get_arg_bool_opt        (lua_State          *L,
                                                 int                 narg,
                                                 const char         *param_name,
                                                 gboolean            default_value);
gboolean        moo_lua_get_arg_bool            (lua_State          *L,
                                                 int                 narg,
                                                 const char         *param_name);
int             moo_lua_get_arg_int_opt         (lua_State          *L,
                                                 int                 narg,
                                                 const char         *param_name,
                                                 int                 default_value);
int             moo_lua_get_arg_int             (lua_State          *L,
                                                 int                 narg,
                                                 const char         *param_name);
void            moo_lua_get_arg_iter            (lua_State          *L,
                                                 int                 narg,
                                                 const char         *param_name,
                                                 GtkTextBuffer      *buffer,
                                                 GtkTextIter        *iter);
gboolean        moo_lua_get_arg_iter_opt        (lua_State          *L,
                                                 int                 narg,
                                                 const char         *param_name,
                                                 GtkTextBuffer      *buffer,
                                                 GtkTextIter        *iter);
int             moo_lua_get_arg_enum_opt        (lua_State          *L,
                                                 int                 narg,
                                                 const char         *param_name,
                                                 GType               type,
                                                 int                 default_value);
int             moo_lua_get_arg_enum            (lua_State          *L,
                                                 int                 narg,
                                                 const char         *param_name,
                                                 GType               type);
const char     *moo_lua_get_arg_string_opt      (lua_State          *L,
                                                 int                 narg,
                                                 const char         *param_name,
                                                 const char         *default_value);
const char     *moo_lua_get_arg_string          (lua_State          *L,
                                                 int                 narg,
                                                 const char         *param_name);
char          **moo_lua_get_arg_strv_opt        (lua_State          *L,
                                                 int                 narg,
                                                 const char         *param_name);
char          **moo_lua_get_arg_strv            (lua_State          *L,
                                                 int                 narg,
                                                 const char         *param_name);

int             moo_lua_push_instance           (lua_State          *L,
                                                 gpointer            instance,
                                                 GType               type,
                                                 gboolean            make_copy);
int             moo_lua_push_object             (lua_State          *L,
                                                 GObject            *obj);
int             moo_lua_push_bool               (lua_State          *L,
                                                 gboolean            value);
int             moo_lua_push_int                (lua_State          *L,
                                                 int                 value);
int             moo_lua_push_strv               (lua_State          *L,
                                                 char              **value);
int             moo_lua_push_string             (lua_State          *L,
                                                 char               *value);
int             moo_lua_push_string_copy        (lua_State          *L,
                                                 const char         *value);
int             moo_lua_push_object_array       (lua_State          *L,
                                                 MooObjectArray     *value,
                                                 gboolean            make_copy);

#endif /* MOO_LUA_API_UTIL_H */
