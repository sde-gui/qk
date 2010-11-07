/*
 *   moo-tests-lua.h
 *
 *   Copyright (C) 2004-2010 by Yevgen Muntyan <emuntyan@sourceforge.net>
 *
 *   This file is part of medit.  medit is free software; you can
 *   redistribute it and/or modify it under the terms of the
 *   GNU Lesser General Public License as published by the
 *   Free Software Foundation; either version 2.1 of the License,
 *   or (at your option) any later version.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with medit.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MOO_TESTS_LUA_H
#define MOO_TESTS_LUA_H

#include "mooutils/moo-test-macros.h"

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
moo_test_run_lua_file (const char *basename)
{
    static char *contents;

    if ((contents = moo_test_load_data_file (basename)))
    {
        lua_State *L = medit_lua_new (NULL, false);
        moo_return_if_fail (L != NULL);

        g_assert (lua_gettop (L) == 0);

        {
            char *testdir = g_build_filename (moo_test_get_data_dir (), "lua", (char*) NULL);
            lua_addpath (L, (char**) &testdir, 1);
            g_free (testdir);
        }

        g_assert (lua_gettop (L) == 0);

        moo_test_run_lua_script (L, contents, basename);
        lua_pop (L, lua_gettop (L));

        medit_lua_free (L);
    }

    g_free (contents);
}


G_END_DECLS

#endif /* MOO_TESTS_LUA_H */
