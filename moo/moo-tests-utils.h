#ifndef MOO_TESTS_UTILS_H
#define MOO_TESTS_UTILS_H

#include <CUnit/CUnit.h>
#include <glib.h>
#include <string.h>
#include <stdarg.h>

G_BEGIN_DECLS


G_GNUC_UNUSED static void
TEST_PASSED_OR_FAILEDV (gboolean    passed,
                        int         line,
                        const char *file,
                        const char *format,
                        va_list     args)
{
    const char *msg;
    char *freeme = NULL;

    if (format)
        msg = freeme = g_strdup_vprintf (format, args);
    else
        msg = passed ? "Passed" : "Failed";

    CU_assertImplementation (passed, line, (char*) msg,
                             (char*) file, (char*) "", FALSE);

    g_free (freeme);
}

G_GNUC_UNUSED static void
TEST_PASSED_OR_FAILED (gboolean    passed,
                       int         line,
                       const char *file,
                       const char *format,
                       ...)
{
    va_list args;
    va_start (args, format);
    TEST_PASSED_OR_FAILEDV (passed, line, file, format, args);
    va_end (args);
}

#define TEST_ASSERT_MSG(cond,format,...)                            \
    TEST_PASSED_OR_FAILED (!!(cond), __LINE__, __FILE__,            \
                           format, __VA_ARGS__)

#define TEST_ASSERT(cond)                                           \
    CU_assertImplementation (!!(cond), __LINE__, (char*) #cond,     \
                             (char*) __FILE__, (char*) "", FALSE)

#define TEST_ASSERT_CMP__(Type,actual,expected,cmp,fmt_arg,msg)     \
G_STMT_START {                                                      \
    Type actual__ = actual;                                         \
    Type expected__ = expected;                                     \
    gboolean passed__ = cmp (actual__, expected__);                 \
    TEST_ASSERT_MSG (passed__,                                      \
                     "%s: expected %s, got %s", msg,                \
                     fmt_arg (expected__),                          \
                     fmt_arg (actual__));                           \
} G_STMT_END

#define TEST_ASSERT_CMP(Type,actual,expected,cmp,cmp_sym,fmt_arg)   \
    TEST_ASSERT_CMP__ (Type, actual, expected, cmp, fmt_arg,        \
                       #actual " " #cmp_sym " " #expected)          \

#define TEST_ASSERT_CMP_MSG(Type,actual,expected,cmp,               \
                            fmt_arg,format,...)                     \
G_STMT_START {                                                      \
    char *msg__ = g_strdup_printf (format, __VA_ARGS__);            \
    TEST_ASSERT_CMP__ (Type, actual, expected, cmp, fmt_arg,        \
                       msg__);                                      \
    g_free (msg__);                                                 \
} G_STMT_END

#define TEST_ASSERT_STR_EQ(actual,expected)                         \
    TEST_ASSERT_CMP (const char *, actual, expected,                \
                     TEST_STR_EQ, =, TEST_FMT_STR)

#define TEST_ASSERT_STR_NEQ(actual,expected)                        \
    TEST_ASSERT_CMP (const char *, actual, expected,                \
                     TEST_STR_NEQ, !=, TEST_FMT_STR)

#define TEST_ASSERT_STR_EQ_MSG(actual,expected,format,...)          \
    TEST_ASSERT_CMP_MSG (const char *, actual, expected,            \
                         TEST_STR_EQ, TEST_FMT_STR,                 \
                         format, __VA_ARGS__)

#define TEST_ASSERT_STRV_EQ_MSG(actual,expected,format,...)         \
    TEST_ASSERT_CMP_MSG (char**, actual, expected,                  \
                         TEST_STRV_EQ, TEST_FMT_STRV,               \
                         format, __VA_ARGS__)

#define TEST_CMP_EQ(a,b)    ((a) == (b))
#define TEST_CMP_NEQ(a,b)   ((a) != (b))
#define TEST_CMP_LT(a,b)    ((a) < (b))
#define TEST_CMP_LE(a,b)    ((a) <= (b))
#define TEST_CMP_GT(a,b)    ((a) > (b))
#define TEST_CMP_GE(a,b)    ((a) >= (b))

#define TEST_STR_NEQ(s1,s2)  (!TEST_STR_EQ ((s1), (s2)))
#define TEST_STRV_NEQ(s1,s2) (!TEST_STRV_EQ ((s1), (s2)))

G_GNUC_UNUSED static gboolean
TEST_STR_EQ (const char *s1, const char *s2)
{
    if (!s1 || !s2)
        return s1 == s2;
    else
        return strcmp (s1, s2) == 0;
}

G_GNUC_UNUSED static gboolean
TEST_STRV_EQ (char **ar1, char **ar2)
{
    if (!ar1 || !ar2)
        return ar1 == ar2;

    while (*ar1 && *ar2)
        if (strcmp (*ar1++, *ar2++) != 0)
            return FALSE;

    return *ar1 == *ar2;
}

static char *
test_string_stack_add__ (char *string)
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

G_GNUC_UNUSED static const char *
TEST_FMT_STR (const char *s)
{
    if (s)
        return test_string_stack_add__ (g_strdup_printf ("\"%s\"", s));
    else
        return "<null>";
}

G_GNUC_UNUSED static const char *
TEST_FMT_STRV (char **array)
{
    GString *s;

    if (!array)
        return "<null>";

    s = g_string_new (NULL);

    while (*array)
    {
        if (s->len)
            g_string_append (s, " ");
        g_string_append_printf (s, "\"%s\"", *array++);
    }

    return test_string_stack_add__ (g_string_free (s, FALSE));
}

#define TEST_FMT_INT(a)     test_string_stack_add__ (g_strdup_printf ("%d", (int) a))
#define TEST_FMT_UINT(a)    test_string_stack_add__ (g_strdup_printf ("%u", (guint) a))

#define TEST_G_ASSERT(expr)                     \
G_STMT_START {                                  \
     if (G_UNLIKELY (!(expr)))                  \
        g_assert_warning (G_LOG_DOMAIN,		\
	                  __FILE__,    		\
	                  __LINE__,	      	\
	                  G_STRFUNC,            \
	                  #expr);               \
} G_STMT_END


G_GNUC_UNUSED struct TestWarningsInfo {
    int count;
    int line;
    char *file;
    char *msg;
} *test_warnings_info;

static void
test_log_handler (const gchar    *log_domain,
                  GLogLevelFlags  log_level,
                  const gchar    *message,
                  gpointer        data)
{
    TEST_G_ASSERT (data == test_warnings_info);

    if (log_level & (G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_WARNING))
    {
        test_warnings_info->count -= 1;

        if (test_warnings_info->count < 0)
            g_log_default_handler (log_domain, log_level, message, NULL);
    }
    else
    {
        g_log_default_handler (log_domain, log_level, message, NULL);
    }
}

static void
TEST_EXPECT_WARNINGV_ (int         howmany,
                       int         line,
                       const char *file,
                       const char *fmt,
                       va_list     args)
{
    TEST_G_ASSERT (test_warnings_info == NULL);
    TEST_G_ASSERT (howmany >= 0);

    test_warnings_info = g_new0 (struct TestWarningsInfo, 1);
    test_warnings_info->count = howmany;
    test_warnings_info->line = line;
    test_warnings_info->file = g_strdup (file);
    test_warnings_info->msg = g_strdup_vprintf (fmt, args);

    g_log_set_default_handler (test_log_handler, test_warnings_info);
    g_log_set_handler (NULL, G_LOG_LEVEL_WARNING | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION,
                       test_log_handler, test_warnings_info);
    g_log_set_handler (G_LOG_DOMAIN, G_LOG_LEVEL_WARNING | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION,
                       test_log_handler, test_warnings_info);
}

G_GNUC_UNUSED static void
TEST_EXPECT_WARNING_ (int         howmany,
                      int         line,
                      const char *file,
                      const char *fmt,
                      ...)
{
    va_list args;
    va_start (args, fmt);
    TEST_EXPECT_WARNINGV_ (howmany, line, file, fmt, args);
    va_end (args);
}

#define TEST_EXPECT_WARNING(howmany,fmt,...)            \
    TEST_EXPECT_WARNING_ (howmany, __LINE__, __FILE__,  \
                          fmt, __VA_ARGS__)

G_GNUC_UNUSED static void
TEST_CHECK_WARNING (void)
{
    TEST_G_ASSERT (test_warnings_info != NULL);

    TEST_PASSED_OR_FAILED (test_warnings_info->count == 0,
                           test_warnings_info->line,
                           test_warnings_info->file,
                           "%s: %d %s warning(s)",
                           test_warnings_info->msg ? test_warnings_info->msg : "",
                           ABS (test_warnings_info->count),
                           test_warnings_info->count < 0 ?
                            "unexpected" : "missing");

    g_free (test_warnings_info->msg);
    g_free (test_warnings_info->file);
    g_free (test_warnings_info);
    test_warnings_info = NULL;

    g_log_set_default_handler (g_log_default_handler, NULL);
    g_log_set_handler (NULL, G_LOG_LEVEL_WARNING | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION,
                       g_log_default_handler, test_warnings_info);
    g_log_set_handler (G_LOG_DOMAIN, G_LOG_LEVEL_WARNING | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION,
                       g_log_default_handler, test_warnings_info);
}


G_END_DECLS

#endif /* MOO_TESTS_UTILS_H */
