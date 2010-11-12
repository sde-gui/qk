/*
 *   moolua.cpp
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

/* This is not a part of the Lua distribution */

#include "moolua.h"
#include "moo-tests-lua.h"
#include "moolua-tests.h"
#include "mooscript/mooscript-lua.h"
#include "mooutils/moospawn.h"
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef __WIN32__
#include <io.h>
#endif
#include <glib.h>
#include <glib/gstdio.h>

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


static int
cfunc_sleep (lua_State *L)
{
    double sec = luaL_checknumber (L, 1);
    g_usleep (G_USEC_PER_SEC * sec);
    return 0;
}

static gboolean
quit_main_loop (GMainLoop *main_loop)
{
    g_main_loop_quit (main_loop);
    return FALSE;
}

static int
cfunc_spin_main_loop (lua_State *L)
{
    double sec = luaL_checknumber (L, 1);
    g_return_val_if_fail (sec >= 0, 0);

    GMainLoop *main_loop = g_main_loop_new (NULL, FALSE);
    _moo_timeout_add (sec * 1000, (GSourceFunc) quit_main_loop, main_loop);

    gdk_threads_leave ();
    g_main_loop_run (main_loop);
    gdk_threads_enter ();

    g_main_loop_unref (main_loop);

    return 0;
}

enum {
    my_F_OK = 0,
    my_R_OK = 1 << 0,
    my_W_OK = 1 << 1,
    my_X_OK = 1 << 2
};

static int
cfunc__access (lua_State *L)
{
    const char *filename = luaL_checkstring (L, 1);
    int mode = luaL_checkint (L, 2);

    int g_mode = F_OK;
    if (mode & my_R_OK)
        g_mode |= R_OK;
    if (mode & my_W_OK)
        g_mode |= W_OK;
    if (mode & my_X_OK)
        g_mode |= X_OK;

    lua_pushboolean (L, g_access (filename, g_mode) == 0);
    return 1;
}

static int
cfunc__execute (lua_State *L)
{
    char **argv = 0;
    const char *command = luaL_checkstring (L, 1);
    GError *error = NULL;
    int exit_status;

    argv = _moo_win32_lame_parse_cmd_line (command, &error);

    if (!argv)
    {
        lua_pushinteger (L, 2);
        moo_message ("could not parse command line '%s': %s", command, error->message);
        g_error_free (error);
        return 1;
    }

    if (!g_spawn_sync (NULL, argv, NULL,
                       (GSpawnFlags) (G_SPAWN_SEARCH_PATH | MOO_SPAWN_WIN32_HIDDEN_CONSOLE),
                       NULL, NULL, NULL, NULL,
                       &exit_status, &error))
    {
        lua_pushinteger (L, 2);
        moo_message ("could not run command '%s': %s", command, error->message);
        g_error_free (error);
        g_strfreev (argv);
        return 1;
    }

    g_strfreev (argv);
    lua_pushinteger (L, exit_status);
    return 1;
}

static const luaL_Reg moo_utils_funcs[] = {
  { "sleep", cfunc_sleep },
  { "spin_main_loop", cfunc_spin_main_loop },
  { "_access", cfunc__access },
  { "_execute", cfunc__execute },
  { NULL, NULL }
};

int
luaopen_moo_utils (lua_State *L)
{
    luaL_register (L, "_moo_utils", moo_utils_funcs);

    lua_pushinteger (L, my_F_OK);
    lua_setfield (L, -2, "F_OK");
    lua_pushinteger (L, my_R_OK);
    lua_setfield (L, -2, "R_OK");
    lua_pushinteger (L, my_W_OK);
    lua_setfield (L, -2, "W_OK");
    lua_pushinteger (L, my_X_OK);
    lua_setfield (L, -2, "X_OK");

    return 1;
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

void
moo_lua_add_user_path (lua_State *L)
{
    char **dirs;
    dirs = moo_get_data_subdirs ("lua");
    lua_addpath (L, dirs, g_strv_length (dirs));
    g_strfreev (dirs);
}


lua_State *
medit_lua_new (bool default_init, bool enable_callbacks)
{
    lua_State *L = lua_open ();
    moo_return_val_if_fail (L != NULL, NULL);

    luaL_openlibs (L);
    moo_lua_add_user_path (L);

    moo_assert (lua_gettop (L) == 0);

    if (!mom::lua_setup (L, default_init, enable_callbacks))
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
        mom::lua_cleanup (L);
        lua_close (L);
    }
}


extern "C" void
medit_lua_run_string (const char *string)
{
    moo_return_if_fail (string != NULL);
    lua_State *L = medit_lua_new (TRUE, FALSE);
    if (L)
        medit_lua_do_string (L, string);
    medit_lua_free (L);
}

extern "C" void
medit_lua_run_file (const char *filename)
{
    moo_return_if_fail (filename != NULL);
    lua_State *L = medit_lua_new (TRUE, FALSE);
    if (L)
        medit_lua_do_file (L, filename);
    medit_lua_free (L);
}


static void
test_func (MooTestEnv *env)
{
    moo_test_run_lua_file ((const char *) env->test_data);
}

static void
add_test (MooTestSuite *suite, const char *name, const char *description, const char *lua_file)
{
    moo_test_suite_add_test (suite, name, description, test_func, (void*) lua_file);
}

void
moo_test_lua (void)
{
    MooTestSuite *suite;

    suite = moo_test_suite_new ("MooLua", "Lua scripting tests", NULL, NULL, NULL);

    add_test (suite, "unicode", "test of unicode", "testunicode.lua");
    add_test (suite, "unicode", "test of unicode (2)", "testustring.lua");
    add_test (suite, "moo", "test of moo package", "testmoo.lua");
    add_test (suite, "medit", "test of medit package", "testmedit.lua");
}
