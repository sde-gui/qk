/*
 *   moo-test-utils.c
 *
 *   Copyright (C) 2004-2010 by Yevgen Muntyan <emuntyan@users.sourceforge.net>
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

#include "moo-test-macros.h"
#include "mooutils/mooutils-fs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *text;
    char *file;
    int line;
} TestAssertInfo;

typedef struct {
    char *name;
    char *description;
    MooTestFunc func;
    gpointer data;
    GSList *failed_asserts;
} MooTest;

struct MooTestSuite {
    char *name;
    char *description;
    GSList *tests;
    MooTestSuiteInit init_func;
    MooTestSuiteCleanup cleanup_func;
    gpointer data;
};

typedef struct {
    char *data_dir;
    GSList *test_suites;
    MooTestSuite *current_suite;
    MooTest *current_test;
    struct TestRun {
        guint suites;
        guint tests;
        guint asserts;
        guint suites_passed;
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
                    const char         *description,
                    MooTestSuiteInit    init_func,
                    MooTestSuiteCleanup cleanup_func,
                    gpointer            data)
{
    MooTestSuite *ts;

    g_return_val_if_fail (name != NULL, NULL);

    ts = g_new0 (MooTestSuite, 1);
    ts->name = g_strdup (name);
    ts->description = g_strdup (description);
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
                         const char   *description,
                         MooTestFunc   test_func,
                         gpointer      data)
{
    MooTest *test;

    g_return_if_fail (ts != NULL);
    g_return_if_fail (name != NULL);
    g_return_if_fail (test_func != NULL);

    test = g_new0 (MooTest, 1);
    test->name = g_strdup (name);
    test->description = g_strdup (description);
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
            g_free (test->description);
            g_free (test->name);
            g_free (test);
        }
        g_slist_free (ts->tests);
        g_free (ts->description);
        g_free (ts->name);
        g_free (ts);
    }
}

static gboolean
run_test (MooTest        *test,
          MooTestSuite   *ts,
          MooTestOptions  opts)
{
    MooTestEnv env;
    gboolean failed;

    if (opts & MOO_TEST_LIST_ONLY)
    {
        fprintf (stdout, "  Test: %s - %s\n", test->name, test->description);
        return TRUE;
    }

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

    return !failed;
}

static void
run_suite (MooTestSuite   *ts,
           MooTest        *single_test,
           MooTestOptions  opts)
{
    GSList *l;
    gboolean run = !(opts & MOO_TEST_LIST_ONLY);
    gboolean passed = TRUE;

    if (run && ts->init_func && !ts->init_func (ts->data))
        return;

    registry.current_suite = ts;

    g_print ("Suite: %s\n", ts->name);

    if (single_test)
        passed = run_test (single_test, ts, opts) && passed;
    else
        for (l = ts->tests; l != NULL; l = l->next)
            passed = run_test (l->data, ts, opts) && passed;

    if (run && ts->cleanup_func)
        ts->cleanup_func (ts->data);

    registry.current_suite = NULL;
    registry.tr.suites += 1;
    if (passed)
        registry.tr.suites_passed += 1;
}

static gboolean
find_test (const char    *name,
           MooTestSuite **ts_p,
           MooTest      **test_p)
{
    GSList *l;
    char **pieces;
    const char *suite_name = NULL;
    const char *test_name = NULL;
    gboolean retval = FALSE;

    g_return_val_if_fail (name != NULL, FALSE);

    pieces = g_strsplit (name, "/", 2);
    g_return_val_if_fail (pieces != NULL && pieces[0] != NULL, FALSE);

    suite_name = pieces[0];
    test_name = pieces[1];

    for (l = registry.test_suites; l != NULL; l = l->next)
    {
        MooTestSuite *ts = l->data;
        GSList *tl;

        if (strcmp (ts->name, suite_name) != 0)
            continue;

        if (!test_name)
        {
            *ts_p = ts;
            *test_p = NULL;
            retval = TRUE;
            goto out;
        }

        for (tl = ts->tests; tl != NULL; tl = tl->next)
        {
            MooTest *test = tl->data;
            if (strcmp (test->name, test_name) == 0)
            {
                *ts_p = ts;
                *test_p = test;
                retval = TRUE;
                goto out;
            }
        }

        break;
    }

out:
    g_strfreev (pieces);
    return retval;
}

gboolean
moo_test_run_tests (char          **tests,
                    const char     *coverage_file,
                    MooTestOptions  opts)
{
    if (coverage_file)
        moo_test_coverage_enable ();

    fprintf (stdout, "\n");

    if (tests && *tests)
    {
        char *name;
        while ((name = *tests++))
        {
            MooTestSuite *single_ts = NULL;
            MooTest *single_test = NULL;

            if (!find_test (name, &single_ts, &single_test))
            {
                g_printerr ("could not find test %s", name);
                exit (EXIT_FAILURE);
            }

            run_suite (single_ts, single_test, opts);
        }
    }
    else
    {
        GSList *l;
        for (l = registry.test_suites; l != NULL; l = l->next)
            run_suite (l->data, NULL, opts);
    }

    fprintf (stdout, "\n");

    if (!(opts & MOO_TEST_LIST_ONLY))
    {
        fprintf (stdout, "Run Summary: Type      Total     Ran  Passed  Failed\n");
        fprintf (stdout, "             suites    %5d     %3d  %6d  %6d\n",
                 registry.tr.suites, registry.tr.suites, registry.tr.suites_passed,
                 registry.tr.suites - registry.tr.suites_passed);
        fprintf (stdout, "             tests     %5d     %3d  %6d  %6d\n",
                 registry.tr.tests, registry.tr.tests, registry.tr.tests_passed,
                 registry.tr.tests - registry.tr.tests_passed);
        fprintf (stdout, "             asserts   %5d     %3d  %6d  %6d\n",
                 registry.tr.asserts, registry.tr.asserts, registry.tr.asserts_passed,
                 registry.tr.asserts - registry.tr.asserts_passed);
        fprintf (stdout, "\n");

        if (coverage_file)
            moo_test_coverage_write (coverage_file);
    }

    return moo_test_get_result ();
}

void
moo_test_cleanup (void)
{
    GSList *l;
    GError *error = NULL;

    for (l = registry.test_suites; l != NULL; l = l->next)
        moo_test_suite_free (l->data);

    g_free (registry.data_dir);
    registry.data_dir = NULL;

    if (g_file_test (moo_test_get_working_dir (), G_FILE_TEST_IS_DIR) &&
        !_moo_remove_dir (moo_test_get_working_dir (), TRUE, &error))
    {
        g_critical ("could not remove directory '%s': %s",
                    moo_test_get_working_dir (), error->message);
        g_error_free (error);
    }
}

gboolean
moo_test_get_result (void)
{
    return registry.tr.asserts_passed == registry.tr.asserts;
}


/**
 * moo_test_assert_impl: (moo.private 1)
 *
 * @passed:
 * @text: (type const-utf8)
 * @file: (type const-utf8) (allow-none) (default NULL)
 * @line: (default -1)
 **/
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


const char *
moo_test_get_data_dir (void)
{
    return registry.data_dir;
}

void
moo_test_set_data_dir (const char *dir)
{
    char *tmp;

    g_return_if_fail (dir != NULL);

    if (!g_file_test (dir, G_FILE_TEST_IS_DIR))
    {
        g_critical ("not a directory: %s", dir);
        return;
    }

    tmp = registry.data_dir;
    registry.data_dir = _moo_normalize_file_path (dir);
    g_free (tmp);
}

const char *
moo_test_get_working_dir (void)
{
    return "test-working-dir";
}

char *
moo_test_find_data_file (const char *basename)
{
    char *fullname;

    g_return_val_if_fail (registry.data_dir != NULL, NULL);

    if (!_moo_path_is_absolute (basename))
        fullname = g_build_filename (registry.data_dir, basename, NULL);
    else
        fullname = g_strdup (basename);

    return fullname;
}

char **
moo_test_list_data_files (const char *dir)
{
    GDir *gdir;
    char *freeme = NULL;
    GError *error = NULL;
    const char *name;
    GPtrArray *names = NULL;

    if (!_moo_path_is_absolute (dir))
    {
        g_return_val_if_fail (registry.data_dir != NULL, NULL);
        freeme = g_build_filename (registry.data_dir, dir, NULL);
        dir = freeme;
    }

    if (!(gdir = g_dir_open (dir, 0, &error)))
    {
        g_warning ("could not open directory '%s': %s",
                   dir, moo_error_message (error));
        g_error_free (error);
        error = NULL;
    }

    names = g_ptr_array_new ();
    while (gdir && (name = g_dir_read_name (gdir)))
        g_ptr_array_add (names, g_strdup (name));
    g_ptr_array_add (names, NULL);

    if (gdir)
        g_dir_close (gdir);

    g_free (freeme);
    return (char**) g_ptr_array_free (names, FALSE);
}

char *
moo_test_load_data_file (const char *basename)
{
    char *fullname;
    char *contents = NULL;
    GError *error = NULL;

    g_return_val_if_fail (registry.data_dir != NULL, NULL);

    if (!_moo_path_is_absolute (basename))
        fullname = g_build_filename (registry.data_dir, basename, NULL);
    else
        fullname = g_strdup (basename);

    if (!g_file_get_contents (fullname, &contents, NULL, &error))
    {
        TEST_FAILED_MSG ("could not open file `%s': %s",
                         fullname, error->message);
        g_error_free (error);
    }

    g_free (fullname);
    return contents;
}


/************************************************************************************
 * coverage
 */

static GHashTable *called_functions = NULL;

void
moo_test_coverage_enable (void)
{
    g_return_if_fail (called_functions == NULL);
    called_functions = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
}

static void
add_func (gpointer key,
          G_GNUC_UNUSED gpointer value,
          GString *string)
{
    g_string_append (string, key);
    g_string_append (string, "\n");
}

void
moo_test_coverage_write (const char *filename)
{
    GString *content;
    GError *error = NULL;

    g_return_if_fail (called_functions != NULL);
    g_return_if_fail (filename != NULL);

    content = g_string_new (NULL);
    g_hash_table_foreach (called_functions, (GHFunc) add_func, content);

    if (!g_file_set_contents (filename, content->str, -1, &error))
    {
        g_critical ("could not save file %s: %s", filename, moo_error_message (error));
        g_error_free (error);
    }

    g_string_free (content, TRUE);
    g_hash_table_destroy (called_functions);
    called_functions = NULL;
}

void
moo_test_coverage_record (const char *lang,
                          const char *function)
{
    if (G_UNLIKELY (called_functions))
        g_hash_table_insert (called_functions,
                             g_strdup_printf ("%s.%s", lang, function),
                             GINT_TO_POINTER (TRUE));
}
