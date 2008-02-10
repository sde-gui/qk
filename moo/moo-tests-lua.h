#ifndef MOO_TESTS_LUA_H
#define MOO_TESTS_LUA_H

#include "moo-tests.h"

G_BEGIN_DECLS


static void
moo_test_run_lua_script (lua_State  *L,
                         const char *script,
                         const char *filename)
{
    int ret;
    int i;

    if (lua_gettop (L) != 0)
    {
        TEST_FAILED_MSG ("before running script `%s': Lua state is corrupted",
                         filename);
        return;
    }

    if (luaL_loadstring (L, script) != 0)
    {
        const char *msg = lua_tostring (L, -1);
        TEST_FAILED_MSG ("error loading script `%s': %s",
                         filename, msg);
        return;
    }

    if ((ret = lua_pcall (L, 0, 0, 0)) != 0)
    {
        const char *msg = lua_tostring (L, -1);

        switch (ret)
        {
            case LUA_ERRRUN:
                TEST_FAILED_MSG ("error running script `%s': %s",
                                 filename, msg);
                break;
            case LUA_ERRMEM:
                TEST_FAILED_MSG ("error running script `%s', memory exhausted",
                                 filename);
                break;
            case LUA_ERRERR:
                TEST_FAILED_MSG ("error running script `%s', "
                                 "this should not have happened!",
                                 filename);
                break;
        }

        return;
    }

    luaL_loadstring (L, "return munit_report()");
    if ((ret = lua_pcall (L, 0, LUA_MULTRET, 0)) != 0)
        g_error ("%s: fix me!", G_STRFUNC);

    for (i = 1; i+2 <= lua_gettop (L); i += 3)
    {
        if (!lua_isstring (L, i) || !lua_isboolean (L, i+1) || !lua_isnumber (L, i+2))
        {
            TEST_FAILED_MSG ("script `%s' returned wrong value!", filename);
        }
        else
        {
            const char *msg = lua_tostring (L, i);
            gboolean success = lua_toboolean (L, i+1);
            int line = lua_tointeger (L, i+2);
            moo_test_assert_msg (success, filename, line, "%s", msg);
        }
    }

    if (i != lua_gettop (L) + 1)
        TEST_FAILED_MSG ("script `%s' returned wrong number of values (%d)",
                         filename, lua_gettop (L));
}

static void
moo_test_run_lua_file (const char *basename,
                       void (*setup_lua) (lua_State*),
                       void (*cleanup_lua) (lua_State*))
{
    static char *contents;

    if ((contents = moo_test_load_data_file (basename)))
    {
        lua_State *L;

        L = lua_open ();
        luaL_openlibs (L);

        {
            char *testdir = g_build_filename (moo_test_get_data_dir (), "lua", NULL);
            lua_addpath (L, (char**) &testdir, 1);
            g_free (testdir);
        }

        if (setup_lua)
            setup_lua (L);

        moo_test_run_lua_script (L, contents, basename);

        if (cleanup_lua)
            cleanup_lua (L);

        lua_close (L);
    }

    g_free (contents);
}


G_END_DECLS

#endif /* MOO_TESTS_LUA_H */
