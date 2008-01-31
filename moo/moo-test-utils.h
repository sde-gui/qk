#ifndef MOO_TEST_UTILS_H
#define MOO_TEST_UTILS_H

#include <glib.h>
#include <string.h>
#include <stdarg.h>

G_BEGIN_DECLS


typedef struct {
    gpointer suite_data;
    gpointer test_data;
} MooTestEnv;

typedef struct MooTestSuite MooTestSuite;
typedef gboolean (*MooTestSuiteInit)    (gpointer    data);
typedef void     (*MooTestSuiteCleanup) (gpointer    data);
typedef void     (*MooTestFunc)         (MooTestEnv *env);

MooTestSuite    *moo_test_suite_new         (const char         *name,
                                             MooTestSuiteInit    init_func,
                                             MooTestSuiteCleanup cleanup_func,
                                             gpointer            data);
void             moo_test_suite_add_test    (MooTestSuite       *ts,
                                             const char         *name,
                                             MooTestFunc         test_func,
                                             gpointer            data);

void             moo_test_run_tests         (void);
void             moo_test_cleanup           (void);
gboolean         moo_test_get_result        (void);

void             moo_test_assert_impl       (gboolean            passed,
                                             const char         *text,
                                             const char         *filename,
                                             int                 line);
void             moo_test_assert_msgv       (gboolean            passed,
                                             const char         *file,
                                             int                 line,
                                             const char         *format,
                                             va_list             args);
void             moo_test_assert_msg        (gboolean            passed,
                                             const char         *file,
                                             int                 line,
                                             const char         *format,
                                             ...);


G_END_DECLS

#endif /* MOO_TEST_UTILS_H */
