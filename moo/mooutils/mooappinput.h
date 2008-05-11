/*
 *   mooapp/mooappinput.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_APP_INPUT_H
#define MOO_APP_INPUT_H

#include <glib.h>

G_BEGIN_DECLS


typedef void (*MooAppInputCallback) (char        cmd,
                                     const char *data,
                                     guint       len,
                                     gpointer    cb_data);

void         _moo_app_input_start       (const char     *name,
                                         gboolean        bind_default,
                                         MooAppInputCallback callback,
                                         gpointer        callback_data);
void         _moo_app_input_shutdown    (void);
gboolean     _moo_app_input_running     (void);

gboolean     _moo_app_input_send_msg    (const char     *name,
                                         const char     *data,
                                         gssize          len);
void         _moo_app_input_broadcast   (const char     *header,
                                         const char     *data,
                                         gssize          len);
const char  *_moo_app_input_get_path    (void);


G_END_DECLS

#endif /* MOO_APP_INPUT_H */
