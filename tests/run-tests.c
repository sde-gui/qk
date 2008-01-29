#include <moo.h>
#include <moo-tests.h>
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

int
main (int argc, char *argv[])
{
    gtk_init (&argc, &argv);

    if (CU_initialize_registry () != CUE_SUCCESS)
        g_error ("CU_initialize_registry() failed");

    moo_test_mooaccel ();
    moo_test_mooutils_fs ();

#ifdef __WIN32__
    moo_test_mooutils_win32 ();
#endif

    moo_test_lua ();
    moo_test_mooedit_lua_api ();

    CU_basic_set_mode (CU_BRM_VERBOSE);
    CU_basic_run_tests ();
    CU_cleanup_registry ();

    return CU_get_error ();
}
