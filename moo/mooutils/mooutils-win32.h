/*
 *   mooutils-win32.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifdef __WIN32__
#ifndef __MOO_UTILS_WIN32_H__
#define __MOO_UTILS_WIN32_H__

#include <gtk/gtk.h>
#include <string.h>
#include <sys/time.h>

G_BEGIN_DECLS


#define fnmatch _moo_win32_fnmatch
#define gettimeofday _moo_win32_gettimeofday
#define getc_unlocked getc

#undef MOO_LOCALE_DIR
#define MOO_LOCALE_DIR (_moo_win32_get_locale_dir())

char       *moo_win32_get_app_dir           (void);
char       *moo_win32_get_dll_dir           (const char     *dll);

GSList     *_moo_win32_add_data_dirs        (GSList         *list,
                                             const char     *prefix);

const char *_moo_win32_get_locale_dir       (void);

gboolean    _moo_win32_open_uri             (const char     *uri);
void        _moo_win32_show_fatal_error     (const char     *domain,
                                             const char     *logmsg);

int         _moo_win32_fnmatch              (const char     *pattern,
                                             const char     *string,
                                             int             flags);
int         _moo_win32_gettimeofday         (struct timeval *tp,
                                             gpointer        tzp);


G_END_DECLS

#endif /* __MOO_UTILS_WIN32_H__ */
#endif /* __WIN32__ */
