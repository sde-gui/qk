#include "moo-test-macros.h"
#include <stdio.h>

typedef struct {
    char *text;
    char *file;
    int line;
} TestAssertInfo;

typedef struct {
    char *name;
    MooTestFunc func;
    gpointer data;
    GSList *failed_asserts;
} MooTest;

struct MooTestSuite {
    char *name;
    GSList *tests;
    MooTestSuiteInit init_func;
    MooTestSuiteCleanup cleanup_func;
    gpointer data;
};

typedef struct {
    GSList *test_suites;
    MooTestSuite *current_suite;
    MooTest *current_test;
    struct TestRun {
        guint suites;
        guint tests;
        guint asserts;
        guint tests_passed;
        guint asserts_passed;
    } tr;
} MooTestRegistry;

static MooTestRegistry registry;

static TestAssertInfo *
test_assert_info_new (const char *text,
                      const char *file,
                      int         line)
{
    TestAssertInfo *ai = g_new0 (TestAssertInfo, 1);
    ai->file = g_strdup (file);
    ai->text = g_strdup (text);
    ai->line = line;
    return ai;
}

static void
test_assert_info_free (TestAssertInfo *ai)
{
    if (ai)
    {
        g_free (ai->file);
        g_free (ai->text);
        g_free (ai);
    }
}

MooTestSuite *
moo_test_suite_new (const char         *name,
                    MooTestSuiteInit    init_func,
                    MooTestSuiteCleanup cleanup_func,
                    gpointer            data)
{
    MooTestSuite *ts;

    g_return_val_if_fail (name != NULL, NULL);

    ts = g_new0 (MooTestSuite, 1);
    ts->name = g_strdup (name);
    ts->init_func = init_func;
    ts->cleanup_func = cleanup_func;
    ts->data = data;
    ts->tests = NULL;

    registry.test_suites = g_slist_append (registry.test_suites, ts);

    return ts;
}

void
moo_test_suite_add_test (MooTestSuite *ts,
                         const char   *name,
                         MooTestFunc   test_func,
                         gpointer      data)
{
    MooTest *test;

    g_return_if_fail (ts != NULL);
    g_return_if_fail (name != NULL);
    g_return_if_fail (test_func != NULL);

    test = g_new0 (MooTest, 1);
    test->name = g_strdup (name);
    test->func = test_func;
    test->data = data;

    ts->tests = g_slist_append (ts->tests, test);
}

static void
moo_test_suite_free (MooTestSuite *ts)
{
    if (ts)
    {
        GSList *l;
        for (l = ts->tests; l != NULL; l = l->next)
        {
            MooTest *test = l->data;
            g_slist_foreach (test->failed_asserts,
                             (GFunc) test_assert_info_free,
                             NULL);
            g_slist_free (test->failed_asserts);
            g_free (test->name);
            g_free (test);
        }
        g_slist_free (ts->tests);
        g_free (ts->name);
        g_free (ts);
    }
}

static void
run_test (MooTest      *test,
          MooTestSuite *ts)
{
    MooTestEnv env;
    gboolean failed;

    env.suite_data = ts->data;
    env.test_data = test->data;

    fprintf (stdout, "  Test: %s ... ", test->name);
    fflush (stdout);

    registry.current_test = test;
    test->func (&env);
    registry.current_test = NULL;

    failed = test->failed_asserts != NULL;

    if (failed)
        fprintf (stdout, "FAILED\n");
    else
        fprintf (stdout, "passed\n");

    if (test->failed_asserts)
    {
        GSList *l;
        int count;

        test->failed_asserts = g_slist_reverse (test->failed_asserts);

        for (l = test->failed_asserts, count = 1; l != NULL; l = l->next, count++)
        {
            TestAssertInfo *ai = l->data;
            fprintf (stdout, "    %d. %s", count, ai->file ? ai->file : "<unknown>");
            if (ai->line > -1)
                fprintf (stdout, ":%d", ai->line);
            fprintf (stdout, " - %s\n", ai->text ? ai->text : "FAILED");
        }
    }

    registry.tr.tests += 1;
    if (!failed)
        registry.tr.tests_passed += 1;
}

static void
run_suite (MooTestSuite *ts)
{
    GSList *l;

    if (ts->init_func && !ts->init_func (ts->data))
        return;

    registry.current_suite = ts;

    g_print ("Suite: %s\n", ts->name);

    for (l = ts->tests; l != NULL; l = l->next)
        run_test (l->data, ts);

    if (ts->cleanup_func)
        ts->cleanup_func (ts->data);

    registry.current_suite = NULL;
    registry.tr.suites += 1;
}

void
moo_test_run_tests (void)
{
    GSList *l;

    fprintf (stdout, "\n");

    for (l = registry.test_suites; l != NULL; l = l->next)
        run_suite (l->data);

    fprintf (stdout, "\n");
    fprintf (stdout, "Run Summary: Type      Total     Ran  Passed  Failed\n");
    fprintf (stdout, "             suites    %5d     %3d     n/a     n/a\n",
             registry.tr.suites, registry.tr.suites);
    fprintf (stdout, "             tests     %5d     %3d  %6d  %6d\n",
             registry.tr.tests, registry.tr.tests, registry.tr.tests_passed,
             registry.tr.tests - registry.tr.tests_passed);
    fprintf (stdout, "             asserts   %5d     %3d  %6d  %6d\n",
             registry.tr.asserts, registry.tr.asserts, registry.tr.asserts_passed,
             registry.tr.asserts - registry.tr.asserts_passed);
    fprintf (stdout, "\n");
}

void
moo_test_cleanup (void)
{
    GSList *l;
    for (l = registry.test_suites; l != NULL; l = l->next)
        moo_test_suite_free (l->data);
}

gboolean
moo_test_get_result (void)
{
    return registry.tr.asserts_passed == registry.tr.asserts;
}


void
moo_test_assert_impl (gboolean    passed,
                      const char *text,
                      const char *file,
                      int         line)
{
    g_return_if_fail (registry.current_test != NULL);

    registry.tr.asserts += 1;
    if (passed)
        registry.tr.asserts_passed += 1;
    else
        registry.current_test->failed_asserts =
            g_slist_prepend (registry.current_test->failed_asserts,
                             test_assert_info_new (text, file, line));
}

void
moo_test_assert_msgv (gboolean    passed,
                      const char *file,
                      int         line,
                      const char *format,
                      va_list     args)
{
    char *text = NULL;

    if (format)
        text = g_strdup_vprintf (format, args);

    moo_test_assert_impl (passed, text, file, line);

    g_free (text);
}

void
moo_test_assert_msg (gboolean    passed,
                     const char *file,
                     int         line,
                     const char *format,
                     ...)
{
    va_list args;
    va_start (args, format);
    moo_test_assert_msgv (passed, file, line, format, args);
    va_end (args);
}
