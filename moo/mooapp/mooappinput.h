/*
 *   mooapp/mooappinput.h
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

#ifndef MOOAPP_MOOAPPINPUT_H
#define MOOAPP_MOOAPPINPUT_H

#ifdef __WIN32__
#include <windows.h>
#endif /* __WIN32__ */
#include "mooapp/moopython.h"

G_BEGIN_DECLS


typedef struct _MooAppInput        MooAppInput;

struct _MooAppInput
{
    guint        ref_count;

    int          pipe;
    char        *pipe_basename;
    char        *pipe_name;
    GIOChannel  *io;
    guint        io_watch;
    MooPython   *python;
    GByteArray  *buffer;
    gboolean     ready;

#ifdef __WIN32__
    HANDLE       listener;
#endif /* __WIN32__ */
};


MooAppInput *moo_app_input_new      (MooPython      *python,
                                     const char     *pipe_basename);

MooAppInput *moo_app_input_ref      (MooAppInput    *ch);
void         moo_app_input_unref    (MooAppInput    *ch);

gboolean     moo_app_input_start    (MooAppInput    *ch);
void         moo_app_input_shutdown (MooAppInput    *ch);
gboolean     moo_app_input_ready    (MooAppInput    *ch);

const char  *moo_app_input_get_name (MooAppInput    *ch);


G_END_DECLS

#endif /* MOOAPP_MOOAPPINPUT_H */
