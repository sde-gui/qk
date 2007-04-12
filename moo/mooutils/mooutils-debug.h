/*
 *   mooutils-debug.h
 *
 *   Copyright (C) 2004-2006 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_UTILS_DEBUG_H
#define MOO_UTILS_DEBUG_H

#include "config.h"
#include <glib.h>
#include <stdlib.h>
#ifdef HAVE_EXECINFO_H
#include <execinfo.h>
#endif

G_BEGIN_DECLS


#if defined(MOO_DEBUG) && defined(HAVE_EXECINFO_H) && defined(HAVE_BACKTRACE)
#ifndef MOO_BACKTRACE_DEPTH
#define MOO_BACKTRACE_DEPTH 10
#endif
#define MOO_BACKTRACE()                                         \
G_STMT_START {                                                  \
    gpointer buf__[MOO_BACKTRACE_DEPTH];                        \
    int n_entries__, i__;                                       \
    char **symbols__;                                           \
                                                                \
    n_entries__ = backtrace (buf__, G_N_ELEMENTS (buf__));      \
    symbols__ = backtrace_symbols (buf__, n_entries__);         \
                                                                \
    if (symbols__)                                              \
        for (i__ = 0; i__ < n_entries__; ++i__)                 \
            g_message ("%s", symbols__[i__]);                   \
                                                                \
    free (symbols__);                                           \
} G_STMT_END
#else
#define MOO_BACKTRACE()
#endif



G_END_DECLS

#endif /* MOO_UTILS_DEBUG_H */
