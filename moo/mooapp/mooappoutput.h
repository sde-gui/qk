/*
 *   mooappoutput.h
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

#ifndef __MOO_APP_OUTPUT__
#define __MOO_APP_OUTPUT__

#ifdef __WIN32__
#include <windows.h>
#endif /* __WIN32__ */

#include <glib.h>

G_BEGIN_DECLS


typedef struct _MooAppOutput MooAppOutput;

struct _MooAppOutput
{
    guint        ref_count;

    char        *pipe_basename;
    char        *pipe_name;
    gboolean     ready;

#ifdef __WIN32__
    HANDLE       pipe;
#else
    int          in;
    int          out;
#endif
};


MooAppOutput   *moo_app_output_new      (const char     *pipe_basename);

MooAppOutput   *moo_app_output_ref      (MooAppOutput   *ch);
void            moo_app_output_unref    (MooAppOutput   *ch);

gboolean        moo_app_output_start    (MooAppOutput   *ch);
void            moo_app_output_shutdown (MooAppOutput   *ch);
void            moo_app_output_write    (MooAppOutput   *ch,
                                         const char     *data,
                                         gssize          len);
void            moo_app_output_flush    (MooAppOutput   *ch);


G_END_DECLS

#endif /* __MOO_APP_OUTPUT__ */
