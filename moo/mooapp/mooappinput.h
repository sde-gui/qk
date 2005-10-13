/*
 *   mooappinput.h
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

#ifndef __MOO_APP_INPUT__
#define __MOO_APP_INPUT__

#ifdef __WIN32__
#include <windows.h>
#endif /* __WIN32__ */

#include <glib.h>

G_BEGIN_DECLS


#define MOO_APP_PYTHON_STRING   's'
#define MOO_APP_PYTHON_FILE     'p'
#define MOO_APP_OPEN_FILE       'f'
#define MOO_APP_QUIT            'q'
#define MOO_APP_DIE             'd'


typedef struct _MooAppInput MooAppInput;

struct _MooAppInput
{
    guint        ref_count;

    int          pipe;
    char        *pipe_basename;
    char        *pipe_name;
    GIOChannel  *io;
    guint        io_watch;
    GByteArray  *buffer;
    gboolean     ready;

#ifdef __WIN32__
    HANDLE       listener;
#endif /* __WIN32__ */
};


MooAppInput *moo_app_input_new      (const char     *pipe_basename);

MooAppInput *moo_app_input_ref      (MooAppInput    *ch);
void         moo_app_input_unref    (MooAppInput    *ch);

gboolean     moo_app_input_start    (MooAppInput    *ch);
void         moo_app_input_shutdown (MooAppInput    *ch);
gboolean     moo_app_input_ready    (MooAppInput    *ch);

const char  *moo_app_input_get_name (MooAppInput    *ch);


G_END_DECLS

#endif /* __MOO_APP_INPUT__ */
