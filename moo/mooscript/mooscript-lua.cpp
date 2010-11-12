#include "mooscript-lua.h"
#include "lua-default-init.h"

namespace mom {

static void push_object(lua_State *L, const HObject &h);

static gpointer data_cookie;
static gpointer object_cookie;

struct MomLuaData {
};

static MomLuaData *set_data(lua_State *L)
{
    moo_return_val_if_fail(L != 0, 0);

    MomLuaData *data = new MomLuaData;

    lua_pushlightuserdata (L, &data_cookie);
    lua_pushlightuserdata (L, data);
    lua_settable (L, LUA_REGISTRYINDEX);

    return data;
}

static void unset_data(lua_State *L)
{
    MomLuaData *data = 0;

    lua_pushlightuserdata(L, &data_cookie);
    lua_gettable(L, LUA_REGISTRYINDEX);

    if (lua_isuserdata(L, -1))
    {
        data = (MomLuaData*) lua_touserdata(L, -1);
        lua_pop(L, 1);
        lua_pushlightuserdata(L, &data_cookie);
        lua_pushnil(L);
        lua_settable(L, LUA_REGISTRYINDEX);
    }
    else
    {
        lua_pop(L, 1);
    }

    delete data;
}

G_GNUC_UNUSED static MomLuaData *get_data(lua_State *L)
{
    MomLuaData *data = 0;

    lua_pushlightuserdata(L, &data_cookie);
    lua_gettable(L, LUA_REGISTRYINDEX);

    if (lua_isuserdata(L, -1))
        data = (MomLuaData*) lua_touserdata(L, -1);

    lua_pop(L, 1);

    mooThrowIfFalse(data != 0);
    return data;
}

static HObject get_arg_if_object(lua_State *L, int narg)
{
    if (!lua_istable(L, narg))
        return HObject();

    int id = 0;

    lua_pushlightuserdata(L, &object_cookie);
    lua_rawget(L, narg);
    if (!lua_isnil(L, -1))
        id = luaL_checkint(L, -1);
    lua_pop(L, 1);

    return HObject(id);
}

static HObject get_arg_object(lua_State *L, int narg)
{
    if (!lua_istable(L, narg))
        luaL_argerror(L, narg, "table expected");

    lua_pushlightuserdata(L, &object_cookie);
    lua_rawget(L, narg);
    int id = luaL_checkint(L, -1);
    lua_pop(L, 1);

    return HObject(id);
}

static const char *get_arg_string(lua_State *L, int narg)
{
    const char *arg = luaL_checkstring(L, narg);
    if (arg == 0)
        luaL_argerror(L, narg, "nil string");
    return arg;
}

static Variant get_arg_variant(lua_State *L, int narg);

static Variant convert_table_to_variant(lua_State *L, int narg)
{
    VariantArray list;
    VariantDict dic;

    size_t len = lua_objlen(L, narg);

    lua_pushnil(L);
    while (lua_next(L, narg) != 0)
    {
        Variant v = get_arg_variant(L, -1);

        bool int_idx = lua_isnumber(L, -2);

        if ((int_idx && !dic.empty()) || (!int_idx && !list.empty()))
            luaL_argerror(L, narg, "either table with string keys or an array expected");

        if (int_idx)
        {
            int idx = luaL_checkint(L, -2);
            if (idx <= 0 || idx > (int) len)
                luaL_argerror(L, narg, "either table with string keys or an array expected");

            if (list.size() < idx)
                list.resize(idx);

            list[idx - 1] = v;
        }
        else
        {
            if (!lua_isstring(L, -2))
                luaL_argerror(L, narg, "either table with string keys or an array expected");
            const char *key = luaL_checkstring(L, -2);
            dic[key] = v;
        }

        lua_pop(L, 1);
    }

    if (!dic.empty())
        return dic;
    else
        return list;
}

static Variant get_arg_variant(lua_State *L, int narg)
{
    luaL_checkany(L, narg);

    if (lua_istable(L, narg))
    {
        lua_pushlightuserdata(L, &object_cookie);
        lua_rawget(L, narg);
        if (lua_isnumber(L, -1))
        {
            int id = luaL_checkint(L, -1);
            lua_pop(L, 1);
            return HObject(id);
        }

        lua_pop(L, 1);
        return convert_table_to_variant(L, narg);
    }

    switch (lua_type(L, narg))
    {
        case LUA_TNIL:
            return Variant();
        case LUA_TBOOLEAN:
            return bool(lua_toboolean(L, narg));
        case LUA_TNUMBER:
            return Base1Int(luaL_checkint(L, narg));
        case LUA_TSTRING:
            return String(luaL_checkstring(L, narg));
        default:
            luaL_argerror(L, narg, "nil/boolean/number/string/table expected");
            break;
    }

    mooThrowIfReached();
}

static int object__eq(lua_State *L)
{
    HObject h1 = get_arg_object(L, 1);
    HObject h2 = get_arg_object(L, 2);
    return h1.id() == h2.id();
}

static void check_result(lua_State *L, Result r)
{
    if (!r.succeeded())
        luaL_error(L, "%s", (const char*) r.message());
}

static void push_variant(lua_State *L, const Variant &v);

static void push_args_array(lua_State *L, const ArgList &args)
{
    for (int i = 0, c = args.size(); i < c; ++i)
        push_variant(L, args[i]);
}

static int cfunc_call_named_method(lua_State *L)
{
    const char *meth = get_arg_string(L, lua_upvalueindex(1));

    // Allow both obj.method(arg) and obj:method(arg) syntaxes.
    // We store the object as upvalue so it's always available and
    // we only need to check whether the first function argument
    // is the same object or not. (This does mean a method can't
    // take a first argument equal to the target object)

    HObject self = get_arg_object(L, lua_upvalueindex(2));
    HObject harg = get_arg_if_object(L, 1);

    int first_arg = 2;
    if (harg.id() != self.id())
        first_arg = 1;

    ArgList args;
    for (int i = first_arg; i <= lua_gettop(L); ++i)
        args.append(get_arg_variant(L, i));

    Variant v;
    Result r = Script::call_method(self, meth, ArgSet(args), v);
    check_result(L, r);

    if (v.vt() != VtVoid)
    {
        push_variant(L, v);
        return 1;
    }
    else
    {
        return 0;
    }
}

static int object__index(lua_State *L)
{
    HObject self = get_arg_object(L, 1);
    const char *field = get_arg_string(L, 2);
    lua_pushstring(L, field);
    push_object(L, self);
    lua_pushcclosure(L, cfunc_call_named_method, 2);
    return 1;
}

static void push_object(lua_State *L, const HObject &h)
{
    // create table O
    lua_createtable(L, 0, 2);

    // O[object_cookie] = id
    lua_pushlightuserdata(L, &object_cookie);
    lua_pushinteger(L, h.id());
    lua_settable(L, -3);

    // create metatable M
    if (luaL_newmetatable(L, "mom-script-object"))
    {
        lua_pushcfunction(L, object__eq);
        lua_setfield(L, -2, "__eq");
        lua_pushcfunction(L, object__index);
        lua_setfield(L, -2, "__index");
    }

    // setmetatable(O, M)
    lua_setmetatable(L, -2);
}

static void push_variant_array(lua_State *L, const VariantArray &ar)
{
    lua_createtable(L, ar.size(), 0);
    for (int i = 0; i < ar.size(); ++i)
    {
        // table[i+1] = ar[i]
        push_variant(L, ar[i]);
        lua_rawseti(L, -2, i + 1);
    }
}

static void push_variant_dict(lua_State *L, const VariantDict &dic)
{
    lua_createtable(L, 0, dic.size());
    for (VariantDict::const_iterator iter = dic.begin(); iter != dic.end(); ++iter)
    {
        // table[key] = value
        lua_pushstring(L, iter.key());
        push_variant(L, iter.value());
        lua_rawset(L, -3);
    }
}

static void push_variant(lua_State *L, const Variant &v)
{
    switch (v.vt())
    {
        case VtVoid:
            lua_pushnil(L);
            return;
        case VtBool:
            lua_pushboolean(L, v.value<VtBool>());
            return;
        case VtIndex:
            lua_pushinteger(L, v.value<VtIndex>().get_base1());
            return;
        case VtBase1:
            lua_pushinteger(L, v.value<VtBase1>().get());
            return;
        case VtInt:
            lua_pushinteger(L, v.value<VtInt>());
            return;
        case VtDouble:
            lua_pushnumber(L, v.value<VtDouble>());
            return;
        case VtString:
            lua_pushstring(L, v.value<VtString>());
            return;
        case VtArray:
            push_variant_array(L, v.value<VtArray>());
            return;
        case VtDict:
            push_variant_dict(L, v.value<VtDict>());
            return;
        case VtObject:
            push_object(L, v.value<VtObject>());
            return;
        case VtArgList:
        case VtArgDict:
            break;
    }

    luaL_error(L, "bad value");
    mooThrowIfReached();
}

static int cfunc_get_app_obj(lua_State *L)
{
    HObject h = Script::get_app_obj();
    push_object(L, h);
    return 1;
}

class CallbackLua : public Callback
{
public:
    CallbackLua(lua_State *L)
        : L(L)
        , id(0)
    {
    }

    static void do_run(lua_State *L, void *ud)
    {
        CallbackLua *self = (CallbackLua *) ud;
        lua_getfield(L, LUA_GLOBALSINDEX, "__medit_invoke_callback");
        lua_pushnumber(L, self->id);
        push_args_array(L, self->args);
        if (lua_pcall(L, self->args.size() + 1, 1, 0) != 0)
        {
            const char *msg = lua_tostring(L, -1);
            moo_critical ("%s: %s", G_STRLOC, msg ? msg : "ERROR");
            lua_pop(L, 1);
        }
        else
        {
            self->retval = get_arg_variant(L, -1);
        }
    }

    Variant run(const ArgList &args)
    {
        this->retval.reset();
        this->args = args;
        do_run(L, this);
        this->args = ArgList();
        Variant retval = this->retval;
        this->retval.reset();
        return retval;
    }

    void on_connect()
    {
    }

    void on_disconnect()
    {
    }

public:
    lua_State *L;
    gulong id;
    Variant retval;
    ArgList args;
};

static int cfunc_connect(lua_State *L)
{
    HObject h = get_arg_object(L, 1);
    const char *name = get_arg_string(L, 2);

    gulong id;
    moo::SharedPtr<CallbackLua> cb = new CallbackLua(L);
    Result r = Script::connect_callback(h, name, cb, id);
    check_result(L, r);

    cb->id = id;
    lua_pushinteger(L, id);
    return 1;
}

static bool add_raw_api(lua_State *L, bool enable_callbacks)
{
#define CALLBACKS \
    { "connect", cfunc_connect }, \

    static const struct luaL_reg meditlib_no_callbacks[] = {
        { "get_app_obj", cfunc_get_app_obj },
        { 0, 0 }
    };

    static const struct luaL_reg meditlib[] = {
        { "get_app_obj", cfunc_get_app_obj },
        CALLBACKS
        { 0, 0 }
    };

    g_assert (lua_gettop (L) == 0);

    if (enable_callbacks)
        luaL_register(L, "medit", meditlib);
    else
        luaL_register(L, "medit", meditlib_no_callbacks);

    lua_pop(L, 1);

    g_assert (lua_gettop (L) == 0);

    return true;
}

bool lua_setup(lua_State *L, bool default_init, bool enable_callbacks) throw()
{
    try
    {
        g_assert (lua_gettop (L) == 0);

        if (!set_data(L))
            return false;

        g_assert (lua_gettop (L) == 0);

        if (!add_raw_api(L, enable_callbacks))
        {
            unset_data(L);
            return false;
        }

        g_assert (lua_gettop (L) == 0);

        if (default_init)
            medit_lua_do_string (L, LUA_DEFAULT_INIT);

        return true;
    }
    catch (...)
    {
        moo_assert_not_reached();
        return false;
    }
}

void lua_cleanup(lua_State *L) throw()
{
    try
    {
        unset_data(L);
    }
    catch (...)
    {
        moo_assert_not_reached();
    }
}

} // namespace mom
