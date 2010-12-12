#include "moolua-tests.h"
#include "moo-tests-lua.h"
#include "moolua/lua/moolua.h"

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
