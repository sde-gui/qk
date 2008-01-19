#ifndef MOO_TESTS_H
#define MOO_TESTS_H

#include <CUnit/CUnit.h>
#include <glib.h>
#include <string.h>
#include <stdarg.h>

G_BEGIN_DECLS


inline static void
moo_test_passed_or_failed (gboolean    passed,
                           int         line,
                           const char *file,
                           const char *format,
                           va_list     args)
{
    const char *msg = NULL;
    char *freeme = NULL;

    if (format)
        msg = freeme = g_strdup_vprintf (format, args);

    if (!msg)
        msg = passed ? "Passed" : "Failed";

    CU_assertImplementation (passed, line, msg, file, "", FALSE);

    g_free (freeme);
}

inline static void
moo_test_passed (int         line,
                 const char *file,
                 const char *format,
                 ...)
{
    va_list args;
    va_start (args, format);
    moo_test_passed_or_failed (TRUE, line, file, format, args);
    va_end (args);
}

inline static void
moo_test_failed (int         line,
                 const char *file,
                 const char *format,
                 ...)
{
    va_list args;
    va_start (args, format);
    moo_test_passed_or_failed (FALSE, line, file, format, args);
    va_end (args);
}

inline static gboolean
moo_test_str_equal (const char *s1, const char *s2)
{
    if (!s1 || !s2)
        return s1 == s2;
    else
        return strcmp (s1, s2) == 0;
}

static char *
moo_test_string_stack_add (char *string)
{
#define STACK_LEN 100
    static char *stack[STACK_LEN];
    static guint ptr = STACK_LEN;

    if (ptr == STACK_LEN)
        ptr = 0;

    g_free (stack[ptr]);
    stack[ptr++] = string;

    return string;
#undef STACK_LEN
}

inline static const char *
moo_test_str_format (const char *s)
{
    if (s)
        return moo_test_string_stack_add (g_strdup_printf ("\"%s\"", s));
    else
        return "<null>";
}

#define MOO_ASSERT_STRING_EQUAL(actual, expected)                                               \
G_STMT_START {                                                                                  \
    const char *a__ = actual;                                                                   \
    const char *e__ = expected;                                                                 \
    if (!moo_test_str_equal (a__, e__))                                                         \
    {                                                                                           \
        char *as__ = moo_test_str_format (a__);                                                 \
        char *es__ = moo_test_str_format (e__);                                                 \
        moo_test_failed (__LINE__, __FILE__,                                                    \
                         "MOO_ASSERT_STRING_EQUAL (actual = %s, expected = %s)",                \
                         moo_test_str_format (a__), moo_test_str_format (e__));                 \
        g_free (es__);                                                                          \
        g_free (as__);                                                                          \
    }                                                                                           \
    else                                                                                        \
    {                                                                                           \
        CU_assertImplementation (TRUE, __LINE__, "", __FILE__, "", FALSE);                      \
    }                                                                                           \
} G_STMT_END


void    moo_test_mooutils_fs    (void);


G_END_DECLS

#endif /* MOO_TESTS_H */
