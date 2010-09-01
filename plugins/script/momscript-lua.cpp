#include "momscript-lua.h"

namespace mom {

static gpointer data_cookie;
static gpointer object_cookie;

struct MomLuaData {
    moo::SharedPtr<IScript> script;
};

static MomLuaData *set_data(lua_State *L)
{
    moo_return_val_if_fail(L != 0, 0);

    MomLuaData *data = new MomLuaData;
    data->script = get_mom_script_instance();

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

static MomLuaData *get_data(lua_State *L)
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
    VariantArray ar;
    VariantDict dic;

    size_t len = lua_objlen(L, narg);

    lua_pushnil(L);
    while (lua_next(L, narg) != 0)
    {
        Variant v = get_arg_variant(L, -1);

        bool int_idx = lua_isnumber(L, -2);

        if ((int_idx && dic.empty()) || (!int_idx && !ar.empty()))
            luaL_argerror(L, narg, "either table with string keys or an array expected");

        if (int_idx)
        {
            int idx = luaL_checkint(L, -2);
            if (idx <= 0 || idx > (int) len)
                luaL_argerror(L, narg, "either table with string keys or an array expected");

            if (ar.size() < idx)
                ar.resize(idx);

            ar[idx - 1] = v;
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
        return ar;
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

static int cfunc_call_named_method(lua_State *L)
{
    MomLuaData *data = get_data(L);
    HObject h = get_arg_object(L, 1);
    const char *meth = get_arg_string(L, lua_upvalueindex(1));
    VariantArray args;
    for (int i = 3; i <= lua_gettop(L); ++i)
        args.append(get_arg_variant(L, i));

    Variant v;
    Result r = data->script->call_method(h, meth, args, v);
    check_result(L, r);

    push_variant(L, v);
    return 1;
}

static int object__index(lua_State *L)
{
    MomLuaData *data = get_data(L);

    HObject h = get_arg_object(L, 1);
    const char *field = get_arg_string(L, 2);

    switch (data->script->lookup_field(h, field))
    {
        case FieldMethod:
            lua_pushstring(L, field);
            lua_pushcclosure(L, cfunc_call_named_method, 1);
            return 1;

        case FieldProperty:
            {
            Variant v;
            Result r = data->script->get_property(h, field, v);
            check_result(L, r);
            push_variant(L, v);
            return 1;
            }

        default:
            return luaL_error(L, "no property or method '%s'", field);
    }
}

static int object__newindex(lua_State *L)
{
    MomLuaData *data = get_data(L);

    HObject h = get_arg_object(L, 1);
    const char *prop = get_arg_string(L, 2);

    if (data->script->lookup_field(h, prop) != FieldProperty)
        return luaL_error(L, "no property '%s'", prop);

    Variant v = get_arg_variant(L, 3);
    Result r = data->script->set_property(h, prop, v);

    check_result(L, r);
    return 0;
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
        lua_pushcfunction(L, object__newindex);
        lua_setfield(L, -2, "__newindex");
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
    }

    luaL_error(L, "bad value");
    mooThrowIfReached();
}

// static int cfunc_set_property(lua_State *L)
// {
//     MomLuaData *data = get_data(L);
//
//     HObject h = get_arg_object(L, 1);
//     const char *prop = get_arg_string(L, 2);
//     Variant v = get_arg_variant(L, 3);
//
//     Result r = data->script->set_property(h, prop, v);
//     check_result(L, r);
//
//     return 0;
// }
//
// static int cfunc_get_property(lua_State *L)
// {
//     MomLuaData *data = get_data(L);
//
//     HObject h = get_arg_object(L, 1);
//     const char *prop = get_arg_string(L, 2);
//
//     Variant v;
//     Result r = data->script->get_property(h, prop, v);
//     check_result(L, r);
//
//     push_variant(L, v);
//     return 1;
// }
//
// static int cfunc_call_method(lua_State *L)
// {
//     MomLuaData *data = get_data(L);
//
//     HObject h = get_arg_object(L, 1);
//     const char *meth = get_arg_string(L, 2);
//     VariantArray args;
//
//     for (int i = 3; i <= lua_gettop(L); ++i)
//         args.append(get_arg_variant(L, i));
//
//     Variant v;
//     Result r = data->script->call_method(h, meth, args, v);
//     check_result(L, r);
//
//     push_variant(L, v);
//     return 1;
// }

static int cfunc_get_global_obj(lua_State *L)
{
    MomLuaData *data = get_data(L);
    HObject h = data->script->get_global_obj();
    push_object(L, h);
    return 1;
}

static bool add_raw_api(lua_State *L)
{
    static const struct luaL_reg meditlib[] = {
        { "get_global_obj", cfunc_get_global_obj },
        { 0, 0 }
    };

    luaL_openlib(L, "medit", meditlib, 0);

    return true;
}

bool lua_setup(lua_State *L) throw()
{
    try
    {
        if (!set_data(L))
            return false;

        if (!add_raw_api(L))
        {
            unset_data(L);
            return false;
        }

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
