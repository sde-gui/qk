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

#if defined(__GNUC__)
#  undef MOO_COMPILER_MSVC
#  define MOO_COMPILER_GCC 1
#  define MOO_GCC_VERSION(maj,min) ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
#elif defined(_MSC_VER)
#  define MOO_COMPILER_MSVC 1
#  undef MOO_COMPILER_GCC
#  define MOO_GCC_VERSION(maj,min) (0)
#else /* not gcc, not visual studio */
#  undef MOO_COMPILER_MSVC
#  undef MOO_COMPILER_GCC
#  define MOO_GCC_VERSION(maj,min) (0)
#endif /* gcc or visual studio */

#if defined(MOO_COMPILER_GCC)
#  define MOO_STRFUNC ((const char*) (__PRETTY_FUNCTION__))
#elif defined(MOO_COMPILER_MSVC)
#  define MOO_STRFUNC ((const char*) (__FUNCTION__))
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#  define MOO_STRFUNC ((const char*) (__func__))
#else
#  define MOO_STRFUNC ((const char*) (""))
#endif

#if defined(MOO_COMPILER_MSVC)
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

#if defined(MOO_COMPILER_GCC)
#define _MOO_GCC_PRAGMA(x) _Pragma (#x)
#define MOO_COMPILER_MESSAGE(x)     _MOO_GCC_PRAGMA(message (#x))
#define MOO_TODO(x)                 _MOO_GCC_PRAGMA(message ("TODO: " #x))
#define MOO_IMPLEMENT_ME            _MOO_GCC_PRAGMA(message ("IMPLEMENT ME"))
#elif defined(MOO_COMPILER_MSVC)
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

#if defined(MOO_COMPILER_MSVC)
#define MOO_NORETURN __declspec(noreturn)
#elif defined(__GNUC__)
#define MOO_NORETURN __attribute__((noreturn))
#else
#define MOO_NORETURN
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

#ifndef DEBUG
MOO_NORETURN
#endif
void moo_assert_message(const char *message, MooCodeLoc loc);

#ifdef __cplusplus
} // extern "C"
#endif

#ifdef __COUNTER__
#define _MOO_COUNTER __COUNTER__
#else
#define _MOO_COUNTER 0
#endif

#define MOO_CODE_LOC (moo_make_code_loc (__FILE__, MOO_STRFUNC, __LINE__, _MOO_COUNTER))

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

#define _MOO_CONCAT_(a, b) a##b
#define _MOO_CONCAT(a, b) _MOO_CONCAT_(a, b)
#define __MOO_STATIC_ASSERT_MACRO(cond, counter) enum { _MOO_CONCAT(_MooStaticAssert_##counter##_, __LINE__) = 1 / ((cond) ? 1 : 0) }
#define _MOO_STATIC_ASSERT_MACRO(cond) __MOO_STATIC_ASSERT_MACRO(cond, 0)
#define MOO_STATIC_ASSERT(cond, message) _MOO_STATIC_ASSERT_MACRO(cond)

#ifdef DEBUG
inline static void _moo_test_asserts_dummy(void *p)
{
    MOO_STATIC_ASSERT (sizeof(char) == 1, "test");
    _MOO_ASSERT_CHECK (p != (void*)0);
    _MOO_DEBUG_ASSERT (p != (void*)0);
    _MOO_RELEASE_ASSERT (p != (void*)0);
    _MOO_RELEASE_ASSERT_NOT_REACHED ();
    _MOO_DEBUG_ASSERT_NOT_REACHED ();
}
#endif

#endif /* MOO_UTILS_MACROS_H */
/* -%- strip:true -%- */
