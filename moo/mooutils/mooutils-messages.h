/*
 *   mooutils-messages.h
 *
 *   Copyright (C) 2004-2016 by Yevgen Muntyan <emuntyan@users.sourceforge.net>
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

#pragma once

#include <mooutils/mooutils-macros.h>
#include <stdarg.h>
#include <mooglib/moo-glib.h>

G_BEGIN_DECLS

#define MOO_STMT_START do

#define MOO_STMT_END                    \
    MOO_MSVC_WARNING_PUSH_DISABLE(4127) \
    while (0)                           \
    MOO_MSVC_WARNING_POP

#undef G_STMT_START
#undef G_STMT_END
#define G_STMT_START MOO_STMT_START
#define G_STMT_END MOO_STMT_END

#ifdef __COUNTER__
#define _MOO_CODE_LOC_COUNTER (__COUNTER__ + 1)
#else
#define _MOO_CODE_LOC_COUNTER 0
#endif

#define MOO_CODE_LOC (moo_make_code_loc (__FILE__, MOO_STRFUNC, __LINE__, _MOO_CODE_LOC_COUNTER))
#define MOO_CODE_LOC_UNKNOWN (moo_default_code_loc ())

#define _MOO_ASSERT_MESSAGE(msg) _moo_assert_message (MOO_CODE_LOC, msg)

#define _MOO_ASSERT_CHECK_MSG(cond, msg)    \
MOO_STMT_START {                            \
    if (G_LIKELY (cond))                    \
        ;                                   \
    else                                    \
        _MOO_ASSERT_MESSAGE (msg);          \
} MOO_STMT_END

#define _MOO_ASSERT_CHECK(cond)             \
    _MOO_ASSERT_CHECK_MSG(cond,             \
        "condition failed: " #cond)

#define MOO_VOID_STMT MOO_STMT_START {} MOO_STMT_END

#define _MOO_RELEASE_ASSERT _MOO_ASSERT_CHECK
#define _MOO_RELEASE_ASSERT_NOT_REACHED() _MOO_ASSERT_MESSAGE ("should not be reached")

#ifdef DEBUG
#define _MOO_DEBUG_ASSERT _MOO_ASSERT_CHECK
#define _MOO_DEBUG_ASSERT_NOT_REACHED() _MOO_ASSERT_MESSAGE ("should not be reached")
#define _MOO_DEBUG_SIDE_ASSERT(what) MOO_STMT_START { gboolean res__ = (what); _MOO_DEBUG_ASSERT(res__); } MOO_STMT_END
#else
#define _MOO_DEBUG_ASSERT(cond) MOO_VOID_STMT
#define _MOO_DEBUG_ASSERT_NOT_REACHED() MOO_VOID_STMT
#define _MOO_DEBUG_SIDE_ASSERT(what) MOO_VOID_STMT
#endif

#define moo_assert _MOO_DEBUG_ASSERT
#define moo_side_assert _MOO_DEBUG_SIDE_ASSERT
#define moo_release_assert _MOO_RELEASE_ASSERT
#define moo_assert_not_reached _MOO_DEBUG_ASSERT_NOT_REACHED
#define moo_release_assert_not_reached _MOO_RELEASE_ASSERT_NOT_REACHED

typedef struct MooCodeLoc
{
    const char *file;
    const char *func;
    int line;
    int counter;

#ifdef __cplusplus

    bool empty() const
    {
        return !file;
    }

#endif // __cplusplus
} MooCodeLoc;

G_INLINE_FUNC MooCodeLoc
moo_default_code_loc (void)
{
    MooCodeLoc loc = { 0, 0, 0, 0 };
    return loc;
}

G_INLINE_FUNC MooCodeLoc
moo_make_code_loc (const char *file, const char *func, int line, int counter)
{
    MooCodeLoc loc;
    loc.file = file;
    loc.func = func;
    loc.line = line;
    loc.counter = counter;
    return loc;
}

void _moo_log (MooCodeLoc loc, GLogLevelFlags flags, const char *format, ...) G_GNUC_PRINTF (3, 4);
void _moo_logv (MooCodeLoc loc, GLogLevelFlags flags, const char *format, va_list args) G_GNUC_PRINTF(3, 0);
void MOO_NORETURN _moo_error (MooCodeLoc loc, const char *format, ...) G_GNUC_PRINTF (2, 3);
void MOO_NORETURN _moo_errorv (MooCodeLoc loc, const char *format, va_list args) G_GNUC_PRINTF(2, 0);

void MOO_NORETURN _moo_assert_message (MooCodeLoc loc, const char *message);

#define moo_return_val_if_fail(cond, val)               \
MOO_STMT_START {                                        \
    if (cond)                                           \
    {                                                   \
    }                                                   \
    else                                                \
    {                                                   \
        moo_critical("Condition '%s' failed", #cond);   \
        return val;                                     \
    }                                                   \
} MOO_STMT_END

#define moo_return_if_fail(cond) moo_return_val_if_fail(cond,;)

#define moo_return_val_if_reached(val)                  \
MOO_STMT_START {                                        \
    moo_critical("should not be reached");              \
    return val;                                         \
} MOO_STMT_END

#define moo_return_if_reached() moo_return_val_if_reached(;)

#define _moo_on_error moo_debug_break
#define _moo_on_critical moo_break_if_in_debugger

#ifdef __WIN32__

#define moo_break_if_in_debugger()  \
    MOO_STMT_START {                \
        if (IsDebuggerPresent())    \
            __debugbreak();         \
    } MOO_STMT_END

#define moo_debug_break __debugbreak

#else // !__WIN32__

#define moo_break_if_in_debugger() MOO_VOID_STMT
#define moo_debug_break() MOO_VOID_STMT

#endif // !__WIN32__

#define moo_check_break_in_debugger(log_level_flags)                                            \
    MOO_STMT_START {                                                                            \
        if ((log_level_flags) & (G_LOG_LEVEL_ERROR | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION))  \
            _moo_on_error();                                                                    \
        else if ((log_level_flags) & (G_LOG_LEVEL_CRITICAL))                                    \
            _moo_on_critical();                                                                 \
    } MOO_STMT_END

/*
 * Suppress warnings when GCC is in -pedantic mode and not -std=c99
 */
#ifdef __GNUC__
#pragma GCC system_header
#endif

#if defined(G_HAVE_ISO_VARARGS)

#  define moo_error(...)                MOO_STMT_START { _moo_on_error(); _moo_error (MOO_CODE_LOC, __VA_ARGS__); } MOO_STMT_END
#  define moo_message(...)              _moo_log (MOO_CODE_LOC, G_LOG_LEVEL_MESSAGE, __VA_ARGS__)
#  define moo_critical(...)             MOO_STMT_START { _moo_on_critical(); _moo_log (MOO_CODE_LOC, G_LOG_LEVEL_CRITICAL, __VA_ARGS__); } MOO_STMT_END
#  define moo_warning(...)              _moo_log (MOO_CODE_LOC, G_LOG_LEVEL_WARNING, __VA_ARGS__)
#  define moo_debug(...)                _moo_log (MOO_CODE_LOC, G_LOG_LEVEL_DEBUG, __VA_ARGS__)
#  define moo_error_noloc(...)          MOO_STMT_START { _moo_on_error(); _moo_error (MOO_CODE_LOC_UNKNOWN, __VA_ARGS__); } MOO_STMT_END
#  define moo_message_noloc(...)        _moo_log (MOO_CODE_LOC_UNKNOWN, G_LOG_LEVEL_MESSAGE, __VA_ARGS__)
#  define moo_critical_noloc(...)       MOO_STMT_START { _moo_on_critical(); _moo_log (MOO_CODE_LOC_UNKNOWN, G_LOG_LEVEL_CRITICAL, __VA_ARGS__); } MOO_STMT_END
#  define moo_warning_noloc(...)        _moo_log (MOO_CODE_LOC_UNKNOWN, G_LOG_LEVEL_WARNING, __VA_ARGS__)
#  define moo_debug_noloc(...)          _moo_log (MOO_CODE_LOC_UNKNOWN, G_LOG_LEVEL_DEBUG, __VA_ARGS__)

#elif defined(G_HAVE_GNUC_VARARGS)

#  define moo_error(format...)          MOO_STMT_START { _moo_on_error(); _moo_error (MOO_CODE_LOC, format); } MOO_STMT_END
#  define moo_message(format...)        _moo_log (MOO_CODE_LOC, G_LOG_LEVEL_MESSAGE, format)
#  define moo_critical(format...)       MOO_STMT_START { _moo_on_critical(); _moo_log (MOO_CODE_LOC, G_LOG_LEVEL_CRITICAL, format); } MOO_STMT_END
#  define moo_warning(format...)        _moo_log (MOO_CODE_LOC, G_LOG_LEVEL_WARNING, format)
#  define moo_debug(format...)          _moo_log (MOO_CODE_LOC, G_LOG_LEVEL_DEBUG, format)
#  define moo_error_noloc(format...)    MOO_STMT_START { _moo_on_error(); _moo_error (MOO_CODE_LOC_UNKNOWN, format); } MOO_STMT_END
#  define moo_message_noloc(format...)  _moo_log (MOO_CODE_LOC_UNKNOWN, G_LOG_LEVEL_MESSAGE, format)
#  define moo_critical_noloc(format...) MOO_STMT_START { _moo_on_critical(); _moo_log (MOO_CODE_LOC_UNKNOWN, G_LOG_LEVEL_CRITICAL, format); } MOO_STMT_END
#  define moo_warning_noloc(format...)  _moo_log (MOO_CODE_LOC_UNKNOWN, G_LOG_LEVEL_WARNING, format)
#  define moo_debug_noloc(format...)    _moo_log (MOO_CODE_LOC_UNKNOWN, G_LOG_LEVEL_DEBUG, format)

#else /* no varargs macros */

G_INLINE_FUNC void MOO_NORETURN
moo_error (const char *format, ...) G_GNUC_PRINTF (1, 2)
{
    _moo_on_error();
    va_list args;
    va_start (args, format);
    _moo_errorv (MOO_CODE_LOC_UNKNOWN, format, args);
    // _moo_errorv does not return
}

#define _MOO_DEFINE_LOG_FUNC(func, FUNC)                \
G_INLINE_FUNC void                                      \
moo_##func (const gchar *format, ...)                   \
G_GNUC_PRINTF (1, 2)                                    \
{                                                       \
    moo_check_break_in_debugger(G_LOG_LEVEL_##FUNC);    \
    va_list args;                                       \
    va_start (args, format);                            \
    _moo_logv (MOO_CODE_LOC_UNKNOWN,                    \
              G_LOG_LEVEL_##FUNC,                       \
              format, args);                            \
    va_end (args);                                      \
}

_MOO_DEFINE_LOG_FUNC (message, MESSAGE)
_MOO_DEFINE_LOG_FUNC (warning, WARNING)
_MOO_DEFINE_LOG_FUNC (critical, CRITICAL)
_MOO_DEFINE_LOG_FUNC (debug, DEBUG)

#undef _MOO_DEFINE_LOG_FUNC

#define moo_error_noloc moo_error
#define moo_message_noloc moo_message
#define moo_critical_noloc moo_critical
#define moo_warning_noloc moo_warning
#define moo_debug_noloc moo_debug

#endif /* varargs macros */

#undef g_critical
#undef g_error
#undef g_warning
#undef g_message
#undef g_debug
#undef g_assert
#define g_critical moo_critical
#define g_error moo_error
#define g_warning moo_warning
#define g_message moo_message
#define g_debug moo_debug
#define g_assert moo_assert

#undef g_return_if_fail
#undef g_return_if_reached
#undef g_return_val_if_fail
#undef g_return_val_if_reached
#define g_return_if_fail moo_return_if_fail
#define g_return_val_if_fail moo_return_val_if_fail
#define g_return_if_reached moo_return_if_reached
#define g_return_val_if_reached moo_return_val_if_reached

G_END_DECLS
