/*
 *   mooutils-macros.h
 *
 *   Copyright (C) 2004-2009 by Yevgen Muntyan <muntyan@tamu.edu>
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

#ifndef MOO_UTILS_MACROS_H
#define MOO_UTILS_MACROS_H

#include <moo-config.h>

#if defined(MOO_CL_GCC)
#  define MOO_STRFUNC ((const char*) (__PRETTY_FUNCTION__))
#elif defined(MOO_CL_MSVC)
#  define MOO_STRFUNC ((const char*) (__FUNCTION__))
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#  define MOO_STRFUNC ((const char*) (__func__))
#else
#  define MOO_STRFUNC ((const char*) (""))
#endif

#if defined(MOO_CL_MSVC)
#define MOO_MSVC_WARNING_PUSH       __pragma(warning(push))
#define MOO_MSVC_WARNING_POP        __pragma(warning(push))
#define MOO_MSVC_WARNING_DISABLE(N) __pragma(warning(disable:N))
#define MOO_MSVC_WARNING_PUSH_DISABLE(N) MOO_MSVC_WARNING_PUSH MOO_MSVC_WARNING_DISABLE(N)
#else
#define MOO_MSVC_WARNING_PUSH
#define MOO_MSVC_WARNING_POP
#define MOO_MSVC_WARNING_DISABLE(N)
#define MOO_MSVC_WARNING_PUSH_DISABLE(N)
#endif

#if defined(MOO_CL_GCC)
#define _MOO_GCC_PRAGMA(x) _Pragma (#x)
#define MOO_COMPILER_MESSAGE(x)     _MOO_GCC_PRAGMA(message (#x))
#define MOO_TODO(x)                 _MOO_GCC_PRAGMA(message ("TODO: " #x))
#define MOO_IMPLEMENT_ME            _MOO_GCC_PRAGMA(message ("IMPLEMENT ME"))
#elif defined(MOO_CL_MSVC)
#define _MOO_MESSAGE_LINE(line) #line
#define _MOO_MESSAGE_LOC __FILE__ "(" _MOO_MESSAGE_LINE(__LINE__) ") : "
#define MOO_COMPILER_MESSAGE(x)     __pragma(message(_MOO_MESSAGE_LOC #x))
#define MOO_TODO(x)                 __pragma(message(_MOO_MESSAGE_LOC "TODO: " #x))
#define MOO_IMPLEMENT_ME            __pragma(message(_MOO_MESSAGE_LOC "IMPLEMENT ME: " __FUNCTION__))
#else
#define MOO_COMPILER_MESSAGE(x)
#define MOO_TODO(x)
#define MOO_IMPLEMENT_ME
#endif

#define MOO_CONCAT__(a, b) a##b
#define MOO_CONCAT(a, b) MOO_CONCAT__(a, b)

#define MOO_UNUSED(x) (void)(x)
#ifdef MOO_CL_GCC
#  define MOO_UNUSED_ARG(x) x __attribute__ ((__unused__))
#else
#  define MOO_UNUSED_ARG(x) x
#endif

#if defined(MOO_CL_GCC)
#  define MOO_FA_ERROR(msg) __attribute__((error(msg)))
#  define MOO_FA_WARNING(msg) __attribute__((warning(msg)))
#  define MOO_FA_NORETURN __attribute__((noreturn))
#  define MOO_FA_CONST __attribute__((const))
#  define MOO_FA_UNUSED __attribute__((unused))
#  define MOO_FA_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#  define MOO_FA_MALLOC __attribute__((malloc))
#  if  MOO_GCC_CHECK_VERSION(3, 3)
#    define MOO_FA_DEPRECATED __attribute__((deprecated))
#  else
#    define MOO_FA_DEPRECATED
#  endif
#  if  MOO_GCC_CHECK_VERSION(3, 3)
#    define MOO_FA_NONNULL(indices) __attribute__((nonnull indices))
#    define MOO_FA_NOTHROW __attribute__((nothrow))
#  else
#    define MOO_FA_NONNULL(indices)
#    define MOO_FA_NOTHROW
#  endif
#else /* !MOO_CL_GCC */
#  define MOO_FA_ERROR(msg)
#  define MOO_FA_WARNING(msg)
#  define MOO_FA_MALLOC
#  define MOO_FA_UNUSED
#  define MOO_FA_WARN_UNUSED_RESULT
#  define MOO_FA_CONST
#  define MOO_FA_DEPRECATED
#  define MOO_FA_NONNULL(indices)
#  if defined(MOO_CL_MSVC)
#    define MOO_FA_NORETURN __declspec(noreturn)
#    define MOO_FA_NOTHROW __declspec(nothrow)
#  else
#    define MOO_FA_NORETURN
#    define MOO_FA_NOTHROW
#  endif
#endif /* !MOO_CL_GCC */

#define MOO_NORETURN MOO_FA_NORETURN
#define MOO_NOTHROW MOO_FA_NOTHROW
#define NORETURN MOO_NORETURN
#define NOTHROW MOO_NOTHROW

#define MOO_FA_MISSING MOO_FA_ERROR("This function must not be used")

#if !defined(MOO_DEV_MODE)
#  define MOO_FUNC_DEV_MODE MOO_FA_ERROR("This function must be used only in dev mode")
#else
#  define MOO_FUNC_DEV_MODE
#endif

#if defined(MOO_CL_GCC)
#  define MOO_VA_CLEANUP(func) __attribute__((cleanup(func)))
#  define _MOO_VA_CLEANUP_DEFINED 1
#  define MOO_VA_DEPRECATED __attribute__((deprecated))
#  define MOO_VA_UNUSED __attribute__((unused))
#else /* !MOO_CL_GCC */
#  define MOO_VA_CLEANUP(func)
#  undef _MOO_VA_CLEANUP_DEFINED
#  define MOO_VA_DEPRECATED
#  define MOO_VA_UNUSED
#endif /* !MOO_CL_GCC */

#define MOO_VAR_CLEANUP_CHECK(func)
#define MOO_VAR_CLEANUP_CHECKD(func)
#undef MOO_VAR_CLEANUP_CHECK_ENABLED
#undef MOO_VAR_CLEANUP_CHECKD_ENABLED

#if defined(_MOO_VA_CLEANUP_DEFINED)
#  undef MOO_VAR_CLEANUP_CHECK
#  define MOO_VAR_CLEANUP_CHECK(func) MOO_VA_CLEANUP(func)
#  define MOO_VAR_CLEANUP_CHECK_ENABLED 1
#  ifdef DEBUG
#    undef MOO_VAR_CLEANUP_CHECKD
#    define MOO_VAR_CLEANUP_CHECKD(func) MOO_VA_CLEANUP(func)
#    define MOO_VAR_CLEANUP_CHECKD_ENABLED 1
#  endif
#endif

#ifdef __COUNTER__
#define _MOO_COUNTER __COUNTER__
#else
#define _MOO_COUNTER 0
#endif

#define MOO_CODE_LOC (moo_make_code_loc (__FILE__, MOO_STRFUNC, __LINE__, _MOO_COUNTER))

#define _MOO_STATIC_ASSERT_MACRO(cond) enum { MOO_CONCAT(_MooStaticAssert_, __LINE__) = 1 / ((cond) ? 1 : 0) }
#define MOO_STATIC_ASSERT(cond, message) _MOO_STATIC_ASSERT_MACRO(cond)

#define _MOO_ASSERT_MESSAGE(msg) moo_assert_message (msg, MOO_CODE_LOC)

#define _MOO_ASSERT_CHECK_MSG(cond, msg)    \
do {                                        \
    if (cond)                               \
        ;                                   \
    else                                    \
        _MOO_ASSERT_MESSAGE (msg);          \
} while(0)

#define _MOO_ASSERT_CHECK(cond)             \
    _MOO_ASSERT_CHECK_MSG(cond,             \
        "condition failed: " #cond)

#define MOO_VOID_STMT do {} while (0)

#define _MOO_RELEASE_ASSERT _MOO_ASSERT_CHECK
#define _MOO_RELEASE_ASSERT_NOT_REACHED() _MOO_ASSERT_MESSAGE ("should not be reached")

#ifdef DEBUG
#define _MOO_DEBUG_ASSERT _MOO_ASSERT_CHECK
#define _MOO_DEBUG_ASSERT_NOT_REACHED() _MOO_ASSERT_MESSAGE ("should not be reached")
#else
#define _MOO_DEBUG_ASSERT(cond) MOO_VOID_STMT
#define _MOO_DEBUG_ASSERT_NOT_REACHED() MOO_VOID_STMT
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MooCodeLoc
{
    const char *file;
    const char *func;
    int line;
    int counter;
} MooCodeLoc;

static inline MooCodeLoc
moo_default_code_loc (void)
{
    MooCodeLoc loc = { "<unknown>", "<unknown>", 0, 0 };
    return loc;
}

static inline MooCodeLoc
moo_make_code_loc (const char *file, const char *func, int line, int counter)
{
    MooCodeLoc loc;
    loc.file = file;
    loc.func = func;
    loc.line = line;
    loc.counter = counter;
    return loc;
}

#ifndef MOO_DEV_MODE
NORETURN
#endif
void moo_assert_message(const char *message, MooCodeLoc loc);

#ifdef __cplusplus
} // extern "C"
#endif

#define __moo_test_func_name MOO_CONCAT(__moo_test_func_, __LINE__)
#define __moo_test_func __moo_test_func_name
#define __moo_test_func_a(args) __moo_test_func_name args

#ifdef MOO_DEV_MODE

MOO_STATIC_ASSERT(sizeof(char) == 1, "test");

inline static void __moo_test_func(void)
{
    _MOO_RELEASE_ASSERT(0);
    _MOO_RELEASE_ASSERT_NOT_REACHED();
    _MOO_DEBUG_ASSERT(0);
    _MOO_DEBUG_ASSERT_NOT_REACHED();
}

int __moo_test_func_name(void) MOO_FA_MISSING;
void __moo_test_func_name(void) MOO_FA_ERROR("test");
void MOO_FA_ERROR("test") __moo_test_func_name(void);
void __moo_test_func_name(void) MOO_FUNC_DEV_MODE;
void MOO_FUNC_DEV_MODE __moo_test_func_name(void);
void __moo_test_func(void);
void __moo_test_func(void) MOO_FA_WARNING("warning");
void MOO_FA_WARNING("warning") __moo_test_func(void);
void __moo_test_func(void) MOO_FA_DEPRECATED;
void MOO_FA_DEPRECATED __moo_test_func(void);
void *__moo_test_func(void) MOO_FA_MALLOC;
void * MOO_FA_MALLOC __moo_test_func(void);
void * MOO_FA_NONNULL(()) __moo_test_func_a((void *p));
void * __moo_test_func_a((void *p)) MOO_FA_NONNULL(());
void * MOO_FA_NONNULL((1)) __moo_test_func_a((void *p));
void * __moo_test_func_a((void *p)) MOO_FA_NONNULL((1));
void * MOO_FA_NONNULL((1,2)) __moo_test_func_a((void *p1, void *p2));
void * __moo_test_func_a((void *p1, void *p2)) MOO_FA_NONNULL((1,2));

void MOO_NORETURN __moo_test_func_name(void);
void MOO_NOTHROW __moo_test_func_name(void);

inline static void MOO_NOTHROW __moo_test_func_name(void)
{
}

void __moo_test_func(void) MOO_FA_NORETURN;
void MOO_FA_NORETURN __moo_test_func(void);

void __moo_test_func(void) MOO_FA_NOTHROW;
void MOO_FA_NOTHROW __moo_test_func(void);

void __moo_test_func(void) MOO_FA_UNUSED;
void MOO_FA_UNUSED __moo_test_func(void);

int __moo_test_func(void) MOO_FA_WARN_UNUSED_RESULT;
int MOO_FA_WARN_UNUSED_RESULT __moo_test_func(void);

inline static void __moo_test_dummy1(void *p)
{
    MOO_UNUSED(p);
}

inline static void __moo_test_func(void)
{
    char *p MOO_VA_CLEANUP(__moo_test_dummy1) = 0;
    MOO_UNUSED(p);
}

inline static void __moo_test_func(void)
{
    char *p MOO_VAR_CLEANUP_CHECK(__moo_test_dummy1) = 0;
    MOO_UNUSED(p);
}

inline static void __moo_test_func(void)
{
    char *p MOO_VAR_CLEANUP_CHECKD(__moo_test_dummy1) = 0;
    MOO_UNUSED(p);
}

inline static void __moo_test_func_a((MOO_VA_UNUSED MOO_VA_DEPRECATED int var))
{
    MOO_VA_UNUSED MOO_VA_DEPRECATED int var2;
    MOO_VA_UNUSED int MOO_VA_DEPRECATED var3;
    MOO_VA_UNUSED int var4 MOO_VA_DEPRECATED;
}

inline static void __moo_test_func(void *p)
{
    MOO_STATIC_ASSERT (sizeof(char) == 1, "test");
    _MOO_ASSERT_CHECK (p != (void*)0);
    _MOO_DEBUG_ASSERT (p != (void*)0);
    _MOO_RELEASE_ASSERT (p != (void*)0);
    _MOO_RELEASE_ASSERT_NOT_REACHED ();
    _MOO_DEBUG_ASSERT_NOT_REACHED ();
}

inline static void __moo_test_dummy2(void) NOTHROW;
inline static void NOTHROW __moo_test_dummy2(void)
{
}

#endif /* MOO_DEV_MODE */

#endif /* MOO_UTILS_MACROS_H */
