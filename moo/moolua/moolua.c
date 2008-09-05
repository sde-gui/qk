/*
 *   moolua.c
 *
 *   Copyright (C) 2004-2008 by Yevgen Muntyan <muntyan@tamu.edu>
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

/* This is not a part of the Lua distribution */

#include "moolua.h"
#include <string.h>
#include <glib.h>

void
lua_push_utf8string (lua_State  *L,
                     const char *s,
                     int         len)
{
    if (!g_utf8_validate (s, len, NULL))
        luaL_error (L, "string is not valid UTF-8");

    if (len < 0)
        len = strlen (s);

    lua_pushlstring (L, s, len);
}

void
lua_take_utf8string (lua_State *L,
                     char      *s)
{
    if (!g_utf8_validate (s, -1, NULL))
    {
        g_free (s);
        luaL_error (L, "string is not valid UTF-8");
    }

    lua_pushstring (L, s);
    g_free (s);
}

const char *
lua_check_utf8string (lua_State  *L,
                      int         numArg,
                      size_t     *len)
{
    const char *s;

    s = luaL_checklstring (L, numArg, len);

    if (!g_utf8_validate (s, -1, NULL))
        luaL_argerror (L, numArg, "string is not valid UTF-8");

    return s;
}


void
lua_addpath (lua_State  *L,
             char      **dirs,
             unsigned    n_dirs)
{
    guint i;
    GString *new_path;
    const char *path;

    if (!n_dirs)
        return;

    lua_getglobal (L, "package"); /* push package */
    if (!lua_istable (L, -1))
    {
        lua_pop (L, 1);
        g_critical ("%s: package variable missing or not a table", G_STRFUNC);
        return;
    }

    lua_getfield (L, -1, "path"); /* push package.path */
    if (!lua_isstring (L, -1))
    {
        lua_pop (L, 2);
        g_critical ("%s: package.path is missing or not a string", G_STRFUNC);
        return;
    }

    path = lua_tostring (L, -1);
    new_path = g_string_new (NULL);

    for (i = 0; i < n_dirs; ++i)
        g_string_append_printf (new_path, "%s/?.lua;", dirs[i]);

    g_string_append (new_path, path);

    lua_pushstring (L, new_path->str);
    lua_setfield (L, -3, "path"); /* pops the string */
    lua_pop (L, 2); /* pop package.path and package */

    g_string_free (new_path, TRUE);
}


#ifdef MOO_ENABLE_UNIT_TESTS

#include "moo-tests-lua.h"
#include "moolua/moolua-tests.h"

static void
test_unicode (void)
{
    moo_test_run_lua_file ("testunicode.lua", NULL, NULL);
}

static void
test_ustring (void)
{
    moo_test_run_lua_file ("testustring.lua", NULL, NULL);
}

void
moo_test_lua (void)
{
    MooTestSuite *suite;

    suite = moo_test_suite_new ("moolua/ustring.c", NULL, NULL, NULL);

    moo_test_suite_add_test (suite, "test of unicode", (MooTestFunc) test_unicode, NULL);
    moo_test_suite_add_test (suite, "test of unicode (2)", (MooTestFunc) test_ustring, NULL);
}

#endif /* MOO_ENABLE_UNIT_TESTS */
