#include <mooedit/mooedit-tests.h>
#include <mooscript/lua/moolua-tests.h>
#include <mooutils/mooutils-tests.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include "mem-debug.h"

static void
add_tests (void)
{
    moo_test_gobject ();
    moo_test_mooaccel ();
    moo_test_mooutils_fs ();
    moo_test_moo_file_writer ();
    moo_test_mooutils_misc ();
    moo_test_i18n ();

#ifdef __WIN32__
    moo_test_mooutils_win32 ();
#endif

    moo_test_lua ();

//     moo_test_key_file ();
    moo_test_editor ();
}

static int
unit_tests_main (MooTestOptions opts, char **tests, const char *data_dir_arg)
{
    const char *data_dir = NULL;

#ifdef MOO_UNIT_TEST_DATA_DIR
    data_dir = MOO_UNIT_TEST_DATA_DIR;
#endif

    if (data_dir_arg)
        data_dir = data_dir_arg;

    add_tests ();
    moo_test_run_tests (tests, data_dir, opts);

    moo_test_cleanup ();

    return moo_test_get_result () ? 0 : 1;
}

static void
list_unit_tests (void)
{
    unit_tests_main (MOO_TEST_LIST_ONLY, NULL, NULL);
}
