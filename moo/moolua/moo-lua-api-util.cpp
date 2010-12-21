#include "moo-lua-api-util.h"
#include "moolua/lua/lauxlib.h"
#include "mooutils/mooutils.h"
#include <vector>
#include <string>

MOO_DEFINE_QUARK_STATIC ("moo-lua-methods", moo_lua_methods_quark)

#define GINSTANCE_META "moo-lua-ginstance-wrapper"

struct LGInstance
{
    gpointer instance;
    GType type;
};

static std::vector<std::string> function_stack;

MooLuaCurrentFunc::MooLuaCurrentFunc (const char *func)
{
    function_stack.push_back (func);
}

MooLuaCurrentFunc::~MooLuaCurrentFunc ()
{
    function_stack.pop_back ();
}

namespace {
class StringHolder
{
public:
    StringHolder (char *s)
        : m_s (s)
    {
    }

    ~StringHolder ()
    {
        g_free (m_s);
    }

private:
    char *m_s;
};
}

const char *
moo_lua_current_function (void)
{
    moo_return_val_if_fail (function_stack.size () > 0, "NONE");
    return function_stack.back ().c_str ();
}

static int
moo_lua_error_impl (lua_State  *L,
                    const char *message)
{
    return luaL_error (L, "function %s: %s", moo_lua_current_function (), message);
}

int
moo_lua_error (lua_State  *L,
               const char *fmt,
               ...)
{
    char *message;
    va_list args;

    va_start (args, fmt);
    message = g_strdup_vprintf (fmt, args);
    va_end (args);

    StringHolder sh(message);
    return moo_lua_error_impl (L, message);
}

int
moo_lua_errorv (lua_State          *L,
                const char         *fmt,
                va_list             args)
{
    char *message = g_strdup_vprintf (fmt, args);
    StringHolder sh(message);
    return moo_lua_error_impl (L, message);
}

static int
moo_lua_arg_error_impl (lua_State  *L,
                        int         narg,
                        const char *param_name,
                        const char *message)
{
    return luaL_error (L, "function %s, argument %d (%s): %s",
                       moo_lua_current_function (),
                       narg,
                       param_name ? param_name : "`none'",
                       message);
}

int
moo_lua_arg_error (lua_State  *L,
                   int         narg,
                   const char *param_name,
                   const char *fmt,
                   ...)
{
    char *message;
    va_list args;

    va_start (args, fmt);
    message = g_strdup_vprintf (fmt, args);
    va_end (args);

    StringHolder sh(message);
    return moo_lua_arg_error_impl (L, narg, param_name, message);
}

int
moo_lua_arg_errorv (lua_State  *L,
                    int         narg,
                    const char *param_name,
                    const char *fmt,
                    va_list     args)
{
    char *message = g_strdup_vprintf (fmt, args);
    StringHolder sh(message);
    return moo_lua_arg_error_impl (L, narg, param_name, message);
}

void
moo_lua_register_methods (GType              type,
                          MooLuaMethodEntry *entries)
{
    GHashTable *methods;

    methods = (GHashTable*) g_type_get_qdata (type, moo_lua_methods_quark ());

    if (!methods)
    {
        methods = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
        g_type_set_qdata (type, moo_lua_methods_quark (), methods);
    }

    while (entries->name)
    {
        g_hash_table_insert (methods, g_strdup (entries->name), (gpointer) entries->impl);
        entries++;
    }
}

MooLuaMethod
moo_lua_lookup_method (lua_State  *L,
                       GType       type,
                       const char *name)
{
    MooLuaMethod meth = NULL;

    moo_return_val_if_fail (name != NULL, NULL);

    while (TRUE)
    {
        GHashTable *methods = (GHashTable*) g_type_get_qdata (type, moo_lua_methods_quark ());

        if (methods != NULL)
            meth = (MooLuaMethod) g_hash_table_lookup (methods, name);

        if (meth != NULL)
            break;

        if (type == G_TYPE_OBJECT || G_TYPE_FUNDAMENTAL (type) != G_TYPE_OBJECT)
            break;

        type = g_type_parent (type);
    }

    if (!meth)
        luaL_error (L, "no method %s", name);

    return meth;
}


static LGInstance *
get_arg_instance (lua_State *L, int arg, const char *param_name)
{
    void *p = lua_touserdata (L, arg);

    if (!p || !lua_getmetatable (L, arg))
        moo_lua_arg_error (L, arg, param_name,
                           "instance expected, got %s",
                           luaL_typename (L, arg));

    lua_getfield (L, LUA_REGISTRYINDEX, GINSTANCE_META);
    if (!lua_rawequal (L, -1, -2))
        moo_lua_arg_error (L, arg, param_name,
                           "instance expected, got %s",
                           luaL_typename (L, arg));

    lua_pop (L, 2);  /* remove both metatables */
    return (LGInstance*) p;
}

static LGInstance *
get_arg_if_instance (lua_State *L, int arg)
{
    void *p = lua_touserdata (L, arg);

    if (!p || !lua_getmetatable (L, arg))
        return NULL;

    lua_getfield (L, LUA_REGISTRYINDEX, GINSTANCE_META);

    if (!lua_rawequal(L, -1, -2))
        p = NULL;

    lua_pop (L, 2);
    return (LGInstance*) p;
}

static const char *
get_arg_string (lua_State *L, int narg, const char *param_name)
{
    const char *arg = luaL_checkstring (L, narg);
    if (arg == 0)
        moo_lua_arg_error (L, narg, param_name, "nil string");
    return arg;
}


static gboolean
ginstance_eq (LGInstance *lg1, LGInstance *lg2)
{
    if (!lg1->instance || !lg2->instance)
        return lg1->instance == lg2->instance;

    if (lg1->type == lg2->type && lg1->type == GTK_TYPE_TEXT_ITER && lg2->type == GTK_TYPE_TEXT_ITER)
        return gtk_text_iter_equal ((GtkTextIter*) lg1->instance, (GtkTextIter*) lg2->instance);

    return lg1->instance == lg2->instance;
}

static int
cfunc_ginstance__eq (lua_State *L)
{
    LGInstance *lg1 = get_arg_instance (L, 1, "arg1");
    LGInstance *lg2 = get_arg_instance (L, 2, "arg2");
    lua_pushboolean (L, ginstance_eq (lg1, lg2));
    return 1;
}

static int
cfunc_ginstance__lt (lua_State *L)
{
    LGInstance *lg1 = get_arg_instance (L, 1, "arg1");
    LGInstance *lg2 = get_arg_instance (L, 2, "arg2");

    if (lg1->instance && lg2->instance && lg1->type == lg2->type &&
        lg1->type == GTK_TYPE_TEXT_ITER && lg2->type == GTK_TYPE_TEXT_ITER)
    {
        lua_pushboolean (L, gtk_text_iter_compare ((GtkTextIter*) lg1->instance, (GtkTextIter*) lg2->instance) < 0);
        return 1;
    }

    return luaL_error (L, "unsupported operation");
}

static int
cfunc_ginstance__le (lua_State *L)
{
    LGInstance *lg1 = get_arg_instance (L, 1, "arg1");
    LGInstance *lg2 = get_arg_instance (L, 2, "arg2");

    if (lg1->instance && lg2->instance && lg1->type == lg2->type &&
        lg1->type == GTK_TYPE_TEXT_ITER && lg2->type == GTK_TYPE_TEXT_ITER)
    {
        lua_pushboolean (L, gtk_text_iter_compare ((GtkTextIter*) lg1->instance, (GtkTextIter*) lg2->instance) <= 0);
        return 1;
    }

    return luaL_error (L, "unsupported operation");
}

int
cfunc_call_named_method (lua_State *L)
{
    // Allow both obj.method(arg) and obj:method(arg) syntaxes.
    // We store the object as upvalue so it's always available and
    // we only need to check whether the first function argument
    // is the same object or not. (This does mean a method can't
    // take a first argument equal to the target object)

    LGInstance *self = get_arg_instance (L, lua_upvalueindex (1), "self");
    LGInstance *arg = get_arg_if_instance (L, 1);

    int first_arg = 2;
    if (!arg || self->instance != arg->instance)
        first_arg = 1;

    const char *meth = get_arg_string (L, lua_upvalueindex (2), NULL);

    MooLuaMethod func = moo_lua_lookup_method (L, self->type, meth);
    moo_return_val_if_fail (func != NULL, 0);

    return func (self->instance, L, first_arg);
}

static int
cfunc_ginstance__index (lua_State *L)
{
    lua_pushvalue (L, 1);
    lua_pushvalue (L, 2);
    lua_pushcclosure (L, cfunc_call_named_method, 2);
    return 1;
}

int
cfunc_ginstance__gc (lua_State *L)
{
    LGInstance *self = get_arg_instance (L, 1, "self");

    if (self->instance)
    {
        switch (G_TYPE_FUNDAMENTAL (self->type))
        {
            case G_TYPE_OBJECT:
                g_object_unref (self->instance);
                break;
            case G_TYPE_BOXED:
                g_boxed_free (self->type, self->instance);
                break;
            case G_TYPE_POINTER:
                break;
            default:
                moo_assert_not_reached ();
                break;
        }
    }

    self->instance = NULL;
    return 0;
}

static int
moo_lua_push_ginstance (lua_State *L,
                        gpointer   instance,
                        GType      type,
                        GType      type_fundamental,
                        gboolean   make_copy)
{
    moo_return_val_if_fail (L != NULL, 0);

    if (!instance)
    {
        lua_pushnil (L);
        return 1;
    }

    LGInstance *lg = (LGInstance*) lua_newuserdata (L, sizeof (LGInstance));
    lg->instance = NULL;
    lg->type = type;

    // create metatable M
    if (luaL_newmetatable (L, GINSTANCE_META))
    {
        lua_pushcfunction (L, cfunc_ginstance__eq);
        lua_setfield (L, -2, "__eq");
        lua_pushcfunction (L, cfunc_ginstance__index);
        lua_setfield (L, -2, "__index");
        lua_pushcfunction (L, cfunc_ginstance__gc);
        lua_setfield (L, -2, "__gc");
        lua_pushcfunction (L, cfunc_ginstance__lt);
        lua_setfield (L, -2, "__lt");
        lua_pushcfunction (L, cfunc_ginstance__le);
        lua_setfield (L, -2, "__le");
    }

    lua_setmetatable (L, -2);

    if (!make_copy)
        lg->instance = instance;
    else if (type_fundamental == G_TYPE_POINTER)
        lg->instance = instance;
    else if (type_fundamental == G_TYPE_BOXED)
        lg->instance = g_boxed_copy (type, instance);
    else if (type_fundamental == G_TYPE_OBJECT)
        lg->instance = g_object_ref (instance);
    else
        moo_return_val_if_reached (0);

    return 1;
}

int
moo_lua_push_object (lua_State *L,
                     GObject   *obj)
{
    return moo_lua_push_ginstance (L, obj, G_OBJECT_TYPE (obj), G_TYPE_OBJECT, TRUE);
}

int
moo_lua_push_instance (lua_State *L,
                       gpointer   instance,
                       GType      type,
                       gboolean   make_copy)
{
    return moo_lua_push_ginstance (L, instance, type, G_TYPE_FUNDAMENTAL (type), make_copy);
}

int
moo_lua_push_bool (lua_State *L,
                   gboolean   value)
{
    lua_pushboolean (L, value);
    return 1;
}

int
moo_lua_push_int (lua_State *L,
                  int        value)
{
    lua_pushinteger (L, value);
    return 1;
}

int
moo_lua_push_index (lua_State *L,
                    int        value)
{
    lua_pushinteger (L, value + 1);
    return 1;
}

int
moo_lua_push_string (lua_State *L,
                     char      *value)
{
    moo_lua_push_string_copy (L, value);
    g_free (value);
    return 1;
}

int
moo_lua_push_string_copy (lua_State  *L,
                          const char *value)
{
    if (!value)
        lua_pushnil (L);
    else
        lua_pushstring (L, value);
    return 1;
}

int
moo_lua_push_gunichar (lua_State *L,
                       gunichar   value)
{
    char buf[12];

    if (!value)
    {
        lua_pushnil (L);
        return 1;
    }

    buf[g_unichar_to_utf8 (value, buf)] = 0;
    lua_pushstring (L, buf);
    return 1;
}

int
moo_lua_push_object_array (lua_State      *L,
                           MooObjectArray *value,
                           gboolean        make_copy)
{
    if (!value)
    {
        lua_pushnil (L);
        return 1;
    }

    lua_createtable (L, value->n_elms, 0);

    for (guint i = 0; i < value->n_elms; ++i)
    {
        // table[i+1] = ar[i]
        moo_lua_push_instance (L, value->elms[i], G_TYPE_OBJECT, TRUE);
        lua_rawseti(L, -2, i + 1);
    }

    if (!make_copy)
        moo_object_array_free (value);

    return 1;
}

int
moo_lua_push_strv (lua_State  *L,
                   char      **value)
{
    if (!value)
    {
        lua_pushnil (L);
        return 1;
    }

    guint n_elms = g_strv_length (value);
    lua_createtable (L, n_elms, 0);

    for (guint i = 0; i < n_elms; ++i)
    {
        // table[i+1] = ar[i]
        moo_lua_push_string_copy (L, value[i]);
        lua_rawseti(L, -2, i + 1);
    }

    g_strfreev (value);
    return 1;
}

int
moo_lua_push_error (lua_State *L,
                    GError    *error)
{
    if (error)
    {
        lua_pushstring (L, error->message);
        lua_pushinteger (L, error->domain);
        lua_pushinteger (L, error->code);
        g_error_free (error);
        return 3;
    }
    else
    {
        return 0;
    }
}


gpointer
moo_lua_get_arg_instance_opt (lua_State  *L,
                              int         narg,
                              const char *param_name,
                              GType       type)
{
    if (lua_isnoneornil (L, narg))
        return NULL;

    LGInstance *lg = get_arg_instance (L, narg, param_name);
    if (!lg->instance)
        return NULL;

    if (G_TYPE_FUNDAMENTAL (type) != G_TYPE_FUNDAMENTAL (lg->type) || !g_type_is_a (lg->type, type))
        moo_lua_arg_error (L, narg, param_name, "expected %s, got %s", g_type_name (type), g_type_name (lg->type));

    return lg->instance;
}

gpointer
moo_lua_get_arg_instance (lua_State  *L,
                          int         narg,
                          const char *param_name,
                          GType       type)
{
    luaL_checkany (L, narg);
    return moo_lua_get_arg_instance_opt (L, narg, param_name, type);
}

MooObjectArray *
moo_lua_get_arg_object_array (lua_State  *L,
                              int         narg,
                              const char *param_name,
                              GType       type)
{
    if (!lua_istable (L, narg))
        moo_lua_arg_error (L, narg, param_name,
                           "table expected, got %s",
                           luaL_typename (L, narg));

    std::vector<gpointer> vec;
    size_t len = lua_objlen(L, narg);

    lua_pushnil(L);
    while (lua_next(L, narg) != 0)
    {
        if (!lua_isnumber (L, -2))
            moo_lua_arg_error (L, narg, param_name,
                               "list expected, got dict");

        gpointer instance = moo_lua_get_arg_instance (L, -1, NULL, type);

        int idx = luaL_checkint (L, -2);
        if (idx <= 0 || idx > (int) len)
            moo_lua_arg_error (L, narg, param_name,
                               "list expected, got dict");

        if ((int) vec.size () < idx)
            vec.resize (idx);

        vec[idx - 1] = instance;

        lua_pop (L, 1);
    }

    MooObjectArray *array = moo_object_array_new ();
    for (int i = 0, c = vec.size(); i < c; ++i)
        moo_object_array_append (array, G_OBJECT (vec[i]));

    lua_pop (L, 1);

    return array;
}

char **
moo_lua_get_arg_strv_opt (lua_State  *L,
                          int         narg,
                          const char *param_name)
{
    if (lua_isnoneornil (L, narg))
        return NULL;
    else
        return moo_lua_get_arg_strv (L, narg, param_name);
}

char **
moo_lua_get_arg_strv (lua_State  *L,
                      int         narg,
                      G_GNUC_UNUSED const char *param_name)
{
    if (!lua_istable (L, narg))
        moo_lua_arg_error (L, narg, param_name,
                           "table expected, got %s",
                           luaL_typename (L, narg));

    std::vector<std::string> vec;
    size_t len = lua_objlen(L, narg);

    lua_pushnil(L);
    while (lua_next(L, narg) != 0)
    {
        if (!lua_isnumber (L, -2))
            moo_lua_arg_error (L, narg, param_name,
                               "list expected, got dict");

        const char *s = moo_lua_get_arg_string (L, -1, NULL);

        int idx = luaL_checkint (L, -2);
        if (idx <= 0 || idx > (int) len)
            moo_lua_arg_error (L, narg, param_name,
                               "list expected, got dict");

        if ((int) vec.size () < idx)
            vec.resize (idx);

        vec[idx - 1] = s;

        lua_pop (L, 1);
    }

    char **strv = g_new (char*, vec.size() + 1);
    for (int i = 0, c = vec.size(); i < c; ++i)
        strv[i] = g_strdup (vec[i].c_str());
    strv[vec.size()] = NULL;

    lua_pop (L, 1);

    return strv;
}

gboolean
moo_lua_get_arg_bool_opt (lua_State   *L,
                          int          narg,
                          G_GNUC_UNUSED const char  *param_name,
                          gboolean     default_value)
{
    if (lua_isnoneornil (L, narg))
        return default_value;
    else
        return lua_toboolean (L, narg);
}

gboolean
moo_lua_get_arg_bool (lua_State   *L,
                      int          narg,
                      const char  *param_name)
{
    luaL_checkany (L, narg);
    return moo_lua_get_arg_bool_opt (L, narg, param_name, FALSE);
}

int
moo_lua_get_arg_int_opt (lua_State  *L,
                         int         narg,
                         G_GNUC_UNUSED const char *param_name,
                         int         default_value)
{
    if (lua_isnoneornil (L, narg))
        return default_value;
    else
        return lua_tointeger (L, narg);
}

int
moo_lua_get_arg_int (lua_State  *L,
                     int         narg,
                     const char *param_name)
{
    luaL_checkany (L, narg);
    return moo_lua_get_arg_int_opt (L, narg, param_name, 0);
}

double
moo_lua_get_arg_double_opt (lua_State  *L,
                            int         narg,
                            G_GNUC_UNUSED const char *param_name,
                            double      default_value)
{
    if (lua_isnoneornil (L, narg))
        return default_value;
    else
        return lua_tonumber (L, narg);
}

double
moo_lua_get_arg_double (lua_State  *L,
                        int         narg,
                        const char *param_name)
{
    luaL_checkany (L, narg);
    return moo_lua_get_arg_double_opt (L, narg, param_name, 0);
}

int
moo_lua_get_arg_index_opt (lua_State  *L,
                           int         narg,
                           const char *param_name,
                           int         default_value)
{
    if (lua_isnoneornil (L, narg))
        return default_value;
    else
        return moo_lua_get_arg_index (L, narg, param_name);
}

int
moo_lua_get_arg_index (lua_State  *L,
                       int         narg,
                       G_GNUC_UNUSED const char *param_name)
{
    luaL_checkany (L, narg);
    return lua_tointeger (L, narg) - 1;
}

void
moo_lua_get_arg_iter (lua_State     *L,
                      int            narg,
                      const char    *param_name,
                      GtkTextBuffer *buffer,
                      GtkTextIter   *iter)
{
    luaL_checkany (L, narg);
    moo_lua_get_arg_iter_opt (L, narg, param_name, buffer, iter);
}

gboolean
moo_lua_get_arg_iter_opt (lua_State     *L,
                          int            narg,
                          const char    *param_name,
                          GtkTextBuffer *buffer,
                          GtkTextIter   *iter)
{
    GtkTextIter *iter_here;

    if (lua_isnoneornil (L, narg))
        return FALSE;

    if (lua_isnumber (L, narg))
    {
        if (!buffer)
            moo_lua_arg_error (L, narg, param_name,
                               "could not convert integer to iterator without a buffer instance");

        int pos = lua_tointeger (L, narg);
        if (pos <= 0 || pos > gtk_text_buffer_get_char_count (buffer) + 1)
            moo_lua_arg_error (L, narg, param_name,
                               "invalid position %d", pos);

        gtk_text_buffer_get_iter_at_offset (buffer, iter, pos - 1);
        return TRUE;
    }

    iter_here = (GtkTextIter*) moo_lua_get_arg_instance (L, narg, param_name, GTK_TYPE_TEXT_ITER);

    if (!iter_here)
        moo_lua_arg_error (L, narg, param_name,
                           "null iterator");

    *iter = *iter_here;
    return TRUE;
}

static int
parse_enum (const char *string,
            GType       type,
            lua_State  *L,
            int         narg,
            const char *param_name)
{
    GEnumValue *value;
    GEnumClass *enum_class = G_ENUM_CLASS (g_type_class_peek (type));

    value = g_enum_get_value_by_nick (enum_class, string);
    if (!value)
        value = g_enum_get_value_by_name (enum_class, string);

    if (!value)
        moo_lua_arg_error (L, narg, param_name,
                           "could not convert string '%s' to a value of type %s",
                           string, g_type_name (type));

    return value->value;
}

static int
parse_flags (const char *string,
             GType       type,
             lua_State  *L,
             int         narg,
             const char *param_name)
{
    char **pieces, **p;
    GEnumValue *value;
    GEnumClass *enum_class = G_ENUM_CLASS (g_type_class_peek (type));
    int ret = 0;

    pieces = g_strsplit_set (string, ",;|", 0);

    for (p = pieces; p && *p; ++p)
    {
        value = g_enum_get_value_by_nick (enum_class, string);
        if (!value)
            value = g_enum_get_value_by_name (enum_class, string);

        if (!value)
        {
            g_strfreev (pieces);
            moo_lua_arg_error (L, narg, param_name,
                               "could not convert string '%s' to a value of type %s",
                               string, g_type_name (type));
        }

        ret |= value->value;
    }

    g_strfreev (pieces);
    return ret;
}

static int
moo_lua_get_arg_enum_from_string (lua_State  *L,
                                  int         narg,
                                  const char *param_name,
                                  GType       type)
{
    const char *s = lua_tostring (L, narg);

    if (!s)
        moo_lua_arg_error (L, narg, param_name, "null string unexpected");

    if (g_type_is_a (type, G_TYPE_ENUM))
        return parse_enum (s, type, L, narg, param_name);
    else
        return parse_flags (s, type, L, narg, param_name);
}

int
moo_lua_get_arg_enum_opt (lua_State  *L,
                          int         narg,
                          const char *param_name,
                          GType       type,
                          int         default_value)
{
    if (lua_isnoneornil (L, narg))
        return default_value;

    if (lua_isnumber (L, narg))
        return lua_tointeger (L, narg);

    if (lua_isstring (L, narg))
        return moo_lua_get_arg_enum_from_string (L, narg, param_name, type);

    moo_lua_arg_error (L, narg, param_name,
                       "expected string or integer, got %s",
                       luaL_typename (L, narg));
    return 0;
}

int
moo_lua_get_arg_enum (lua_State  *L,
                      int         narg,
                      const char *param_name,
                      GType       type)
{
    luaL_checkany (L, narg);
    return moo_lua_get_arg_enum_opt (L, narg, param_name, type, 0);
}

const char *
moo_lua_get_arg_string_opt (lua_State   *L,
                            int          narg,
                            G_GNUC_UNUSED const char *param_name,
                            const char  *default_value)
{
    if (lua_isnoneornil (L, narg))
        return default_value;
    else
        return lua_tostring (L, narg);
}

const char *
moo_lua_get_arg_string (lua_State  *L,
                        int         narg,
                        const char *param_name)
{
    luaL_checkany (L, narg);
    return moo_lua_get_arg_string_opt (L, narg, param_name, NULL);
}
