#include <moo.h>
#include <moo-tests.h>
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

int
main (int argc, char *argv[])
{
//     CU_pSuite pSuite;

    gtk_init (&argc, &argv);

    if (CU_initialize_registry () != CUE_SUCCESS)
        g_error ("CU_initialize_registry() failed");

//     /* add a suite to the registry */
//     pSuite = CU_add_suite("Suite_1", init_suite1, clean_suite1);
//     if (NULL == pSuite) {
//         CU_cleanup_registry();
//         return CU_get_error();
//     }
//
//     /* add the tests to the suite */
//     /* NOTE - ORDER IS IMPORTANT - MUST TEST fread() AFTER fprintf() */
//     if ((NULL == CU_add_test(pSuite, "test of fprintf()", testFPRINTF)) ||
//        (NULL == CU_add_test(pSuite, "test of fread()", testFREAD)))
//     {
//         CU_cleanup_registry();
//         return CU_get_error();
//     }

    moo_test_mooaccel ();
    moo_test_mooutils_fs ();

#ifdef __WIN32__
    moo_test_mooutils_win32 ();
#endif

    CU_basic_set_mode (CU_BRM_VERBOSE);
    CU_basic_run_tests ();
    CU_cleanup_registry ();

    return CU_get_error ();
}
