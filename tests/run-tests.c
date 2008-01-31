#include <moo.h>
#include <moo-tests.h>

int
main (int argc, char *argv[])
{
    gtk_init (&argc, &argv);

    moo_test_mooaccel ();
    moo_test_mooutils_fs ();

#ifdef __WIN32__
    moo_test_mooutils_win32 ();
#endif

    moo_test_lua ();
    moo_test_mooedit_lua_api ();

    moo_test_run_tests ();
    moo_test_cleanup ();
    return moo_test_get_result () ? 0 : 1;
}
