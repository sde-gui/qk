#include "moolua/medit-lua.h"
#include "moolua/moo-lua-api-util.h"
#include "mooutils/mooutils.h"
#include "moolua/lua-default-init.h"
#include "mooapp/mooapp.h"

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
    moo_return_val_if_fail (L != NULL, NULL);

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
    moo_return_val_if_fail (L != NULL, FALSE);
    moo_return_val_if_fail (string != NULL, FALSE);

    if (luaL_dostring (L, string) != 0)
    {
        const char *msg = lua_tostring (L, -1);
        g_critical ("%s: %s", G_STRLOC, msg ? msg : "ERROR");
        return false;
    }

    return true;
}

bool
medit_lua_do_file (lua_State *L, const char *filename)
{
    moo_return_val_if_fail (L != NULL, FALSE);
    moo_return_val_if_fail (filename != NULL, FALSE);

    char *content = NULL;
    GError *error = NULL;
    if (!g_file_get_contents (filename, &content, NULL, &error))
    {
        moo_warning ("could not read file '%s': %s", filename, error->message);
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
    moo_return_if_fail (string != NULL);
    lua_State *L = medit_lua_new (TRUE);
    if (L)
        medit_lua_do_string (L, string);
    medit_lua_free (L);
}

extern "C" void
medit_lua_run_file (const char *filename)
{
    moo_return_if_fail (filename != NULL);
    lua_State *L = medit_lua_new (TRUE);
    if (L)
        medit_lua_do_file (L, filename);
    medit_lua_free (L);
}
