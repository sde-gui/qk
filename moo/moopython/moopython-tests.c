#include "moopython-utils.h"
#include "moopython-tests.h"
#include "medit-python.h"

static void
moo_test_run_python_file (const char *basename)
{
    char *filename = moo_test_find_data_file (basename);

    if (!filename)
        TEST_FAILED_MSG ("could not find file `%s'", basename);
    else if (!medit_python_run_file (filename, TRUE))
        TEST_FAILED_MSG ("error running file `%s'", basename);

    g_free (filename);
}

static void
test_func (MooTestEnv *env)
{
    static gboolean been_here = FALSE;

    if (!been_here)
    {
        been_here = TRUE;
        moo_python_add_path (moo_test_get_data_dir ());
    }

    moo_test_run_python_file ((const char *) env->test_data);
}

static void
add_test (MooTestSuite *suite, const char *name, const char *description, const char *python_file)
{
    moo_test_suite_add_test (suite, name, description, test_func, (void*) python_file);
}

void
moo_test_python (void)
{
    MooTestSuite *suite;

    suite = moo_test_suite_new ("MooPython", "Python scripting tests", NULL, NULL, NULL);

    add_test (suite, "moo", "test of moo module", "test-python/testmoo.py");
}
