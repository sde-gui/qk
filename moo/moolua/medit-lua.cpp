#include "moolua/medit-lua.h"
#include "moolua/moo-lua-api-util.h"
#include "mooutils/mooutils.h"
#include "moolua/lua-default-init.h"
#include "mooapp/mooapp.h"

extern "C" {

/**
 * class:MooLuaState: (parent GObject) (moo.private 1) (moo.lua 0) (constructable)
 **/

typedef struct MooLuaStateClass MooLuaStateClass;

struct MooLuaState
{
    GObject base;
    lua_State *L;
};

struct MooLuaStateClass
{
    GObjectClass base_class;
};

G_DEFINE_TYPE (MooLuaState, moo_lua_state, G_TYPE_OBJECT)

static void
moo_lua_state_init (MooLuaState *lua)
{
    try
    {
        lua->L = medit_lua_new (true);
    }
    catch (...)
    {
        g_critical ("oops");
    }
}

static void
moo_lua_state_finalize (GObject *object)
{
    MooLuaState *lua = (MooLuaState*) object;

    try
    {
        medit_lua_free (lua->L);
    }
    catch (...)
    {
        g_critical ("oops");
    }

    G_OBJECT_CLASS (moo_lua_state_parent_class)->finalize (object);
}

static void
moo_lua_state_class_init (MooLuaStateClass *klass)
{
    G_OBJECT_CLASS (klass)->finalize = moo_lua_state_finalize;
}

/**
 * moo_lua_state_run_string:
 *
 * @lua:
 * @string: (type const-utf8)
 *
 * Returns: (type utf8)
 **/
char *
moo_lua_state_run_string (MooLuaState *lua,
                          const char  *string)
{
    g_return_val_if_fail (lua && lua->L, g_strdup ("error:unexpected error"));
    g_return_val_if_fail (string != NULL, g_strdup ("error:unexpected error"));

    try
    {
        int old_top = lua_gettop (lua->L);

        if (luaL_dostring (lua->L, string) == 0)
        {
            GString *sret = g_string_new ("ok:");
            int nret = lua_gettop (lua->L) - old_top;
            if (nret > 1 || (nret == 1 && !lua_isnil (lua->L, -1)))
                for (int i = 1; i <= nret; ++i)
                {
                    if (i > 1)
                        g_string_append (sret, ", ");
                    if (lua_isnil (lua->L, -i))
                        g_string_append (sret, "nil");
                    else
                        g_string_append (sret, lua_tostring (lua->L, -i));
                }
            lua_pop (lua->L, nret);
            return g_string_free (sret, FALSE);
        }

        char *ret;
        const char *msg = lua_tostring (lua->L, -1);
        if (strstr (msg, "<eof>"))
            ret = g_strdup ("error:incomplete");
        else
            ret = g_strdup_printf ("error:%s", msg);
        lua_pop (lua->L, lua_gettop (lua->L) - old_top);
        return ret;
    }
    catch (...)
    {
        g_critical ("oops");
        return g_strdup ("error:unexpected error");
    }
}

} // extern "C"

void gtk_lua_api_add_to_lua (lua_State *L, const char *package_name);
void moo_lua_api_add_to_lua (lua_State *L, const char *package_name);

static bool
add_raw_api (lua_State *L)
{
    g_assert (lua_gettop (L) == 0);

    gtk_lua_api_add_to_lua (L, "gtk");
    lua_pop(L, 1);

    moo_lua_api_add_to_lua (L, "medit");
    lua_pop(L, 1);

    g_assert (lua_gettop (L) == 0);

    return true;
}

bool
medit_lua_setup (lua_State *L,
                 bool       default_init)
{
    try
    {
        g_assert (lua_gettop (L) == 0);

        if (!add_raw_api (L))
            return false;

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

lua_State *
medit_lua_new (bool default_init)
{
    lua_State *L = lua_open ();
    g_return_val_if_fail (L != NULL, NULL);

    luaL_openlibs (L);
    moo_lua_add_user_path (L);

    moo_assert (lua_gettop (L) == 0);

    if (!medit_lua_setup (L, default_init))
    {
        lua_close (L);
        return NULL;
    }

    moo_assert (lua_gettop (L) == 0);

    return L;
}

bool
medit_lua_do_string (lua_State *L, const char *string)
{
    g_return_val_if_fail (L != NULL, FALSE);
    g_return_val_if_fail (string != NULL, FALSE);

    if (luaL_dostring (L, string) != 0)
    {
        const char *msg = lua_tostring (L, -1);
        g_critical ("%s", msg ? msg : "ERROR");
        return false;
    }

    return true;
}

bool
medit_lua_do_file (lua_State *L, const char *filename)
{
    g_return_val_if_fail (L != NULL, FALSE);
    g_return_val_if_fail (filename != NULL, FALSE);

    char *content = NULL;
    GError *error = NULL;
    if (!g_file_get_contents (filename, &content, NULL, &error))
    {
        g_warning ("could not read file '%s': %s", filename, error->message);
        g_error_free (error);
        return false;
    }

    gboolean ret = medit_lua_do_string (L, content);
    g_free (content);
    return ret;
}

void
medit_lua_free (lua_State *L)
{
    if (L)
    {
//         mom::lua_cleanup (L);
        lua_close (L);
    }
}


extern "C" void
medit_lua_run_string (const char *string)
{
    g_return_if_fail (string != NULL);
    lua_State *L = medit_lua_new (TRUE);
    if (L)
        medit_lua_do_string (L, string);
    medit_lua_free (L);
}

extern "C" void
medit_lua_run_file (const char *filename)
{
    g_return_if_fail (filename != NULL);
    lua_State *L = medit_lua_new (TRUE);
    if (L)
        medit_lua_do_file (L, filename);
    medit_lua_free (L);
}
