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
    CU_pSuite suite;

    suite = CU_add_suite ("moolua/ustring.c", NULL, NULL);

    CU_add_test (suite, "test of unicode", test_unicode);
    CU_add_test (suite, "test of unicode (2)", test_ustring);
}

#endif /* MOO_ENABLE_UNIT_TESTS */
