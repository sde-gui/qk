/*
 *   mooutils-debug.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_UTILS_DEBUG_H
#define MOO_UTILS_DEBUG_H

#include <glib.h>
#include <stdarg.h>

G_BEGIN_DECLS


#ifdef MOO_DEBUG_ENABLED

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
inline static void                                          \
moo_dmsg (const char *format, ...) G_GNUC_PRINTF (1, 2);    \
                                                            \
inline static void                                          \
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
}

void     _moo_message       (const char *format, ...) G_GNUC_PRINTF (1, 2);
gboolean moo_debug_enabled  (const char *var,
                             gboolean    def_enabled);

#define MOO_DEBUG(whatever) G_STMT_START {whatever;} G_STMT_END

#else

#define MOO_DEBUG_INIT(domain, def_enabled)
#define moo_dmsg(format, args...) G_STMT_START {} G_STMT_END
#define _moo_message(format, args...) G_STMT_START {} G_STMT_END
#define MOO_DEBUG(whatever) G_STMT_START {} G_STMT_END

#endif


G_END_DECLS

#endif /* MOO_UTILS_DEBUG_H */
