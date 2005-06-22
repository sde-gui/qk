/*
 *   mooutils/moolog.h
 *
 *   Copyright (C) 2004-2005 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef DEBUG_H
#define DEBUG_H

#include <gtk/gtkwidget.h>

G_BEGIN_DECLS


void    moo_log_window_show     (void);
void    moo_log_window_hide     (void);

void    moo_log_window_write    (const gchar    *log_domain,
                                 GLogLevelFlags  flags,
                                 const gchar    *message);


void    moo_set_log_func_window (int             show);
void    moo_set_log_func_file   (const char     *log_file);
void    moo_set_log_func        (int             show_log);
#ifdef __WIN32__
void    moo_show_fatal_error    (const char     *logdomain,
                                 const char     *logmsg);
#endif /* __WIN32__ */


G_END_DECLS

#endif /* DEBUG_H */
