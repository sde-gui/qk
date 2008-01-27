/* This is not a part of the Lua distribution */

#include "moolua.h"
#include <string.h>
#include <glib.h>

typedef struct {
    char *buf;
    int len;
    int char_len;
} UString;

static UString      *checkustring       (lua_State  *L);
static const char   *check_utf8string   (lua_State  *L,
                                         int         numArg,
                                         UString   **us,
                                         size_t     *len);

static void
push_ustring (lua_State *L,
              char      *s,
              int        len)
{
    UString *us;

    g_assert (g_utf8_validate (s, len, NULL));

    us = lua_newuserdata (L, sizeof (UString));
    luaL_getmetatable (L, "ustring.ustring");
    lua_setmetatable (L, -2);

    us->buf = s;
    us->len = len >= 0 ? len : (int) strlen (s);
    us->char_len = g_utf8_strlen (us->buf, -1);
}

void
lua_push_utf8string (lua_State  *L,
                     const char *s,
                     int         len)
{
    if (!g_utf8_validate (s, len, NULL))
        luaL_error (L, "string is not valid UTF-8");

    if (len < 0)
        len = strlen (s);

    push_ustring (L, g_strndup (s, len), len);
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

    push_ustring (L, g_strdup (s), -1);
}

static int
new_ustring (lua_State *L)
{
    size_t len;
    const char *s;

    s = luaL_checklstring (L, 1, &len);
    lua_push_utf8string (L, s, len);

    return 1;
}

static UString *
lua_checkustring (lua_State *L,
                  int        arg)
{
    UString *us = luaL_checkudata (L, arg, "ustring.ustring");
    luaL_argcheck (L, us != NULL, arg, "`ustring' expected");
    return us;
}

static UString *
checkustring (lua_State *L)
{
    return lua_checkustring (L, 1);
}

static UString *
get_ustring (lua_State *L,
             int        arg)
{
    void *p = lua_touserdata (L, arg);

    if (!p || !lua_getmetatable (L, arg))
        return NULL;

    lua_getfield (L, LUA_REGISTRYINDEX, "ustring.ustring");
    if (!lua_rawequal(L, -1, -2))
        p = NULL;

    lua_pop(L, 2);
    return p;
}

static const char *
check_utf8string (lua_State  *L,
                  int         numArg,
                  UString   **us_p,
                  size_t     *len)
{
    const char *s;
    UString *us = get_ustring (L, numArg);

    if (us_p)
        *us_p = us;

    if (us)
    {
        if (len)
            *len = us->len;
        return us->buf;
    }

    s = luaL_checklstring (L, numArg, len);

    if (!g_utf8_validate (s, -1, NULL))
        luaL_argerror (L, numArg, "string is not valid UTF-8");

    return s;
}

const char *
lua_check_utf8string (lua_State *L,
                      int        numArg,
                      size_t    *len)
{
    return check_utf8string (L, numArg, NULL, len);
}


static int
ustring__gc (lua_State *L)
{
    UString *us = checkustring (L);
    g_free (us->buf);
    us->buf = NULL;
    return 0;
}

static int
ustring__tostring (lua_State *L)
{
    UString *us = checkustring (L);
    lua_pushlstring (L, us->buf, us->len);
    return 1;
}

static int
ustring_bytelen (lua_State *L)
{
    UString *us = checkustring (L);
    lua_pushnumber (L, us->len);
    return 1;
}

static int
ustring_len (lua_State *L)
{
    UString *us;
    const char *s;

    s = check_utf8string (L, 1, &us, NULL);

    if (us)
        lua_pushnumber (L, us->char_len);
    else
        lua_pushnumber (L, g_utf8_strlen (s, -1));

    return 1;
}

static int
ustring__index (lua_State *L)
{
    UString *us = checkustring (L);
    int index = luaL_checkint (L, 2);
    char *sub;
    int len;

    luaL_argcheck (L, 1 <= index && index <= us->char_len,
                   2, "index out of range");

    sub = g_utf8_offset_to_pointer (us->buf, index - 1);
    len = g_utf8_offset_to_pointer (sub, 1) - sub;
    lua_push_utf8string (L, sub, len);

    return 1;
}

static int
ustring__concat (lua_State *L)
{
    UString *us;
    const char *s;
    char *result;

    us = lua_checkustring (L, 1);
    s = check_utf8string (L, 2, NULL, NULL);

    result = g_strconcat (us->buf, s, NULL);
    push_ustring (L, result, -1);

    return 1;
}

static int
ustring__eq (lua_State *L)
{
    UString *us;
    const char *s;

    us = lua_checkustring (L, 1);
    s = check_utf8string (L, 2, NULL, NULL);

    lua_pushboolean (L, strcmp (us->buf, s) == 0);
    return 1;
}

static int
ustring_lower (lua_State *L)
{
    const char *s;
    char *result;

    s = check_utf8string (L, 1, NULL, NULL);

    result = g_utf8_strdown (s, -1);
    push_ustring (L, result, -1);

    return 1;
}

static int
ustring_upper (lua_State *L)
{
    const char *s;
    char *result;

    s = check_utf8string (L, 1, NULL, NULL);

    result = g_utf8_strup (s, -1);
    push_ustring (L, result, -1);

    return 1;
}

static int
ustring_rep (lua_State *L)
{
    int n;
    size_t len;
    const char *s;
    GString *result;

    s = check_utf8string (L, 1, NULL, &len);
    n = luaL_checkint (L, 2);
    luaL_argcheck (L, n >= 0, 2, "negative number");

    if (!n)
    {
        push_ustring (L, g_strdup (""), -1);
        return 1;
    }

    result = g_string_new (s);
    for (n -= 1; n > 0; --n)
        g_string_append_len (result, s, len);

    push_ustring (L, g_string_free (result, FALSE), -1);
    return 1;
}

static int
ustring_reverse (lua_State *L)
{
    const char *s;

    s = check_utf8string (L, 1, NULL, NULL);

    push_ustring (L, g_utf8_strreverse (s, -1), -1);
    return 1;
}

static int
ustring_sub (lua_State *L)
{
    int len;
    int start, end;
    const char *start_p, *end_p;
    const char *s;
    UString *us;

    s = check_utf8string (L, 1, &us, NULL);
    if (us)
        len = us->char_len;
    else
        len = g_utf8_strlen (s, -1);

    start = luaL_checkint (L, 2);
    end = luaL_optint (L, 3, -1);

    if (start < 0)
        start = len + start + 1;
    if (end < 0)
        end = len + end + 1;

    luaL_argcheck (L, 1 <= start && start <= len, 2, "index out of range");
    luaL_argcheck (L, 1 <= end && end <= len, 3, "index out of range");

    if (start > end)
    {
        push_ustring (L, g_strdup (""), -1);
        return 1;
    }

    start_p = g_utf8_offset_to_pointer (s, start - 1);
    end_p = g_utf8_offset_to_pointer (start_p, end - start + 1);

    push_ustring (L, g_strndup (start_p, end_p - start_p), -1);

    return 1;
}


static const struct luaL_reg ustringlib_f[] = {
    { "new", new_ustring },
    { "len", ustring_len },
    { "lower", ustring_lower },
    { "upper", ustring_upper },
    { "rep", ustring_rep },
    { "reverse", ustring_reverse },
    { "sub", ustring_sub },
    { NULL, NULL }
};

static const struct luaL_reg ustringlib_m[] = {
    { "__tostring", ustring__tostring },
    { "__gc", ustring__gc },
    { "__index", ustring__index },
    { "__len", ustring_len },
    { "__concat", ustring__concat },
    { "__eq", ustring__eq },
    { "bytelen", ustring_bytelen },
    { "len", ustring_len },
    { "lower", ustring_lower },
    { "upper", ustring_upper },
    { "rep", ustring_rep },
    { "reverse", ustring_reverse },
    { "sub", ustring_sub },
    { NULL, NULL }
};

int
luaopen_ustring (lua_State *L)
{
    luaL_newmetatable (L, "ustring.ustring");
    luaL_register (L, NULL, ustringlib_m);

    lua_pushstring (L, "__index");
    lua_pushvalue (L, -2);  /* pushes the metatable */
    lua_settable (L, -3);  /* metatable.__index = metatable */

    luaL_register (L, "ustring", ustringlib_f);

    return 1;
}


#ifdef MOO_ENABLE_UNIT_TESTS

#include "moo-tests.h"

static void
run_script (const char *script,
            const char *filename)
{
    lua_State *L;
    int ret;
    int i;

    L = lua_open ();
    luaL_openlibs (L);

    if (luaL_loadstring (L, script) != 0)
    {
        const char *msg = lua_tostring (L, -1);
        TEST_FAILED_MSG ("error loading script `%s': %s",
                         filename, msg);
        lua_close (L);
        return;
    }

    if ((ret = lua_pcall (L, 0, LUA_MULTRET, 0)) != 0)
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

        lua_close (L);
        return;
    }

    for (i = 1; i+1 <= lua_gettop (L); i += 2)
    {
        if (!lua_isstring (L, i) || !lua_isboolean (L, i+1))
        {
            TEST_FAILED_MSG ("script `%s' returned wrong value",
                             filename);
        }
        else
        {
            const char *msg = lua_tostring (L, i);
            gboolean success = lua_toboolean (L, i+1);
            TEST_PASSED_OR_FAILED (success, 0, filename, "%s", msg);
        }
    }

    if (i != lua_gettop (L) + 1)
        TEST_FAILED_MSG ("script `%s' returned wrong number of values",
                         filename);

    lua_close (L);
}

static void
run_file (const char *file)
{
    char *fullname;
    char *contents;
    GError *error = NULL;

    fullname = g_build_filename (SRCDIR, "test", file, NULL);
    if (!g_file_get_contents (fullname, &contents, NULL, &error))
    {
        TEST_FAILED_MSG ("could not open file `%s': %s",
                         fullname, error->message);
        g_error_free (error);
        g_free (fullname);
        return;
    }

    run_script (contents, fullname);

    g_free (contents);
    g_free (fullname);
}

static void
test_ustring (void)
{
    run_file ("testustring.lua");
}

void
moo_test_lua (void)
{
    CU_pSuite suite;

    suite = CU_add_suite ("moolua/ustring/ustring.c", NULL, NULL);

    CU_add_test (suite, "test of ustring", test_ustring);
}

#endif /* MOO_ENABLE_UNIT_TESTS */
