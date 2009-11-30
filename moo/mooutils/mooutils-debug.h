/*
 *   mooutils-debug.h
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

#ifndef MOO_UTILS_DEBUG_H
#define MOO_UTILS_DEBUG_H

#include <glib.h>
#include <stdarg.h>
#include <mooutils/mooutils-macros.h>

G_BEGIN_DECLS

#define moo_assert _MOO_DEBUG_ASSERT
#define moo_release_assert _MOO_RELEASE_ASSERT

#ifdef DEBUG

#define MOO_DEBUG_INIT(domain, def_enabled)                 \
static const char *moo_debug_domain = "moo-debug-" #domain; \
static gboolean                                             \
_moo_debug_enabled (void)                                   \
{                                                           \
    static int enabled = -1;                                \
    if (enabled == -1)                                      \
        enabled = moo_debug_enabled (#domain, def_enabled); \
    return enabled;                                         \
}                                                           \
                                                            \
G_GNUC_UNUSED static void                                   \
moo_dmsg (const char *format, ...) G_GNUC_PRINTF (1, 2);    \
G_GNUC_UNUSED static void                                   \
moo_dprint (const char *format, ...) G_GNUC_PRINTF (1, 2);  \
                                                            \
static void                                                 \
moo_dmsg (const char *format, ...)                          \
{                                                           \
    va_list args;                                           \
    if (_moo_debug_enabled ())                              \
    {                                                       \
        va_start (args, format);                            \
        g_logv (moo_debug_domain,                           \
                G_LOG_LEVEL_MESSAGE,                        \
                format, args);                              \
        va_end (args);                                      \
    }                                                       \
}                                                           \
                                                            \
static void                                                 \
moo_dprint (const char *format, ...)                        \
{                                                           \
    va_list args;                                           \
    char *string;                                           \
                                                            \
    if (!_moo_debug_enabled ())                             \
        return;                                             \
                                                            \
    va_start (args, format);                                \
    string = g_strdup_vprintf (format, args);               \
    va_end (args);                                          \
                                                            \
    g_return_if_fail (string != NULL);                      \
    g_print ("%s", string);                                 \
    g_free (string);                                        \
}

#define MOO_DEBUG_CODE(code)                                \
G_STMT_START {                                              \
    if (_moo_debug_enabled ())                              \
    {                                                       \
        code ;                                              \
    }                                                       \
} G_STMT_END

void     _moo_message       (const char *format, ...) G_GNUC_PRINTF (1, 2);
gboolean moo_debug_enabled  (const char *var,
                             gboolean    def_enabled);
void     _moo_set_debug     (const char *domains);

#elif defined(MOO_CL_GCC)

#define MOO_DEBUG_INIT(domain, def_enabled)
#define moo_dmsg(format, args...) G_STMT_START {} G_STMT_END
#define moo_dprint(format, args...) G_STMT_START {} G_STMT_END
#define _moo_message(format, args...) G_STMT_START {} G_STMT_END
#define MOO_DEBUG_CODE(whatever) G_STMT_START {} G_STMT_END

#else /* not gcc, not DEBUG */

#define MOO_DEBUG_INIT(domain, def_enabled)
#define MOO_DEBUG_CODE(whatever) G_STMT_START {} G_STMT_END

static void moo_dmsg (const char *format, ...) G_GNUC_PRINTF(1,2)
{
}

static void moo_dprint (const char *format, ...) G_GNUC_PRINTF(1,2)
{
}

static void _moo_message_dummy (const char *format, ...) G_GNUC_PRINTF(1,2)
{
}

#define _moo_message _moo_message_dummy

#endif  /* gcc or DEBUG */


G_END_DECLS

#endif /* MOO_UTILS_DEBUG_H */
