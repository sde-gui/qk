#include <moo.h>
#include <moo-tests.h>

int
main (int argc, char *argv[])
{
    const char *data_dir;

    g_thread_init (NULL);
    gtk_init (&argc, &argv);

    moo_test_gobject ();
    moo_test_mooaccel ();
    moo_test_mooutils_fs ();
    moo_test_moo_file_writer ();
    moo_test_mooutils_misc ();

#ifdef __WIN32__
    moo_test_mooutils_win32 ();
#endif

    moo_test_lua ();
    moo_test_mooedit_lua_api ();

    moo_test_key_file ();
    moo_test_editor ();

#ifdef __WIN32__
    data_dir = "test-data";
#else
    data_dir = SRCDIR "/data";
#endif

    moo_test_run_tests (data_dir);

    moo_test_cleanup ();

#ifdef __WIN32__
    if (!g_getenv ("WINESERVER"))
        getchar ();
#endif

    return moo_test_get_result () ? 0 : 1;
}
