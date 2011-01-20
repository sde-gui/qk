#ifndef MOO_LUA_API_UTIL_H
#define MOO_LUA_API_UTIL_H

#include "moolua/lua/lua.h"
#include "moolua/lua/lauxlib.h"
#include "mooutils/mooarray.h"
#include <gtk/gtk.h>
#include <stdarg.h>

typedef int (*MooLuaMethod) (gpointer instance, lua_State *L, int first_arg);
typedef void (*MooLuaMetatableFunc) (lua_State *L);

typedef struct {
    const char *name;
    MooLuaMethod impl;
} MooLuaMethodEntry;

class MooLuaCurrentFunc
{
public:
    MooLuaCurrentFunc(const char *func);
    ~MooLuaCurrentFunc();
};

MooLuaMethod    moo_lua_lookup_method           (lua_State          *L,
                                                 GType               type,
                                                 const char         *meth);
void            moo_lua_register_methods        (GType               type,
                                                 MooLuaMethodEntry  *entries);
void            moo_lua_register_static_methods (lua_State          *L,
                                                 const char         *package_name,
                                                 const char         *class_name,
                                                 const luaL_Reg     *methods);

void            moo_lua_register_gobject        (void);

int             moo_lua_error                   (lua_State          *L,
                                                 const char         *fmt,
                                                 ...) G_GNUC_PRINTF (2, 3);
int             moo_lua_errorv                  (lua_State          *L,
                                                 const char         *fmt,
                                                 va_list             args);
int             moo_lua_arg_error               (lua_State          *L,
                                                 int                 narg,
                                                 const char         *param_name,
                                                 const char         *fmt,
                                                 ...) G_GNUC_PRINTF (4, 5);
int             moo_lua_arg_errorv              (lua_State          *L,
                                                 int                 narg,
                                                 const char         *param_name,
                                                 const char         *fmt,
                                                 va_list             args);

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
double          moo_lua_get_arg_double_opt      (lua_State          *L,
                                                 int                 narg,
                                                 const char         *param_name,
                                                 double              default_value);
double          moo_lua_get_arg_double          (lua_State          *L,
                                                 int                 narg,
                                                 const char         *param_name);
int             moo_lua_get_arg_index_opt       (lua_State          *L,
                                                 int                 narg,
                                                 const char         *param_name,
                                                 int                 default_value);
int             moo_lua_get_arg_index           (lua_State          *L,
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
void            moo_lua_get_arg_rect            (lua_State          *L,
                                                 int                 narg,
                                                 const char         *param_name,
                                                 GdkRectangle       *rect);
gboolean        moo_lua_get_arg_rect_opt        (lua_State          *L,
                                                 int                 narg,
                                                 const char         *param_name,
                                                 GdkRectangle       *rect);
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
const char     *moo_lua_get_arg_utf8_opt        (lua_State          *L,
                                                 int                 narg,
                                                 const char         *param_name,
                                                 const char         *default_value);
const char     *moo_lua_get_arg_utf8            (lua_State          *L,
                                                 int                 narg,
                                                 const char         *param_name);
const char     *moo_lua_get_arg_filename_opt    (lua_State          *L,
                                                 int                 narg,
                                                 const char         *param_name,
                                                 const char         *default_value);
const char     *moo_lua_get_arg_filename        (lua_State          *L,
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
                                                 GObject            *obj,
                                                 gboolean            make_copy);
int             moo_lua_push_bool               (lua_State          *L,
                                                 gboolean            value);
int             moo_lua_push_int                (lua_State          *L,
                                                 int                 value);
int             moo_lua_push_index              (lua_State          *L,
                                                 int                 value);
int             moo_lua_push_strv               (lua_State          *L,
                                                 char              **value);
int             moo_lua_push_gunichar           (lua_State          *L,
                                                 gunichar            value);
int             moo_lua_push_string             (lua_State          *L,
                                                 char               *value);
int             moo_lua_push_string_copy        (lua_State          *L,
                                                 const char         *value);
int             moo_lua_push_utf8               (lua_State          *L,
                                                 char               *value);
int             moo_lua_push_utf8_copy          (lua_State          *L,
                                                 const char         *value);
int             moo_lua_push_filename           (lua_State          *L,
                                                 char               *value);
int             moo_lua_push_filename_copy      (lua_State          *L,
                                                 const char         *value);
int             moo_lua_push_object_array       (lua_State          *L,
                                                 MooObjectArray     *value,
                                                 gboolean            make_copy);
int             moo_lua_push_error              (lua_State          *L,
                                                 GError             *error);

#endif /* MOO_LUA_API_UTIL_H */
