/*
 *   mooappoutput.c
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef __WIN32__
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#else /* !__WIN32__ */
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/poll.h>
#endif /* !__WIN32__ */
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "mooapp/mooappoutput.h"


#define MAX_BUFFER_SIZE 4096


MooAppOutput*
moo_app_output_new (const char *pipe_basename)
{
    MooAppOutput *ch;

    g_return_val_if_fail (pipe_basename != NULL, NULL);

    ch = g_new0 (MooAppOutput, 1);
    ch->ref_count = 1;

    ch->pipe_basename = g_strdup (pipe_basename);

#ifdef __WIN32__
    ch->pipe = NULL;
#else /* ! __WIN32__ */
    ch->in = -1;
    ch->out = -1;
#endif /* ! __WIN32__ */
    ch->pipe_name = NULL;
    ch->ready = FALSE;

    return ch;
}


MooAppOutput*
moo_app_output_ref (MooAppOutput    *ch)
{
    g_return_val_if_fail (ch != NULL, NULL);
    ++ch->ref_count;
    return ch;
}


void
moo_app_output_unref (MooAppOutput    *ch)
{
    g_return_if_fail (ch != NULL);

    if (--ch->ref_count)
        return;

    moo_app_output_shutdown (ch);

    g_free (ch->pipe_basename);
    g_free (ch);
}


void
moo_app_output_shutdown (MooAppOutput *ch)
{
    g_return_if_fail (ch != NULL);

#ifdef __WIN32__
    if (ch->pipe)
    {
        CloseHandle (ch->pipe);
        ch->pipe = NULL;
    }
#endif /* __WIN32__ */

#ifndef __WIN32__
    if (ch->out >= 0)
        close (ch->out);
    if (ch->in >= 0)
        close (ch->in);
    ch->out = -1;
    ch->in = -1;

    if (ch->pipe_name)
        unlink (ch->pipe_name);
#endif /* ! __WIN32__ */

    if (ch->pipe_name)
    {
        g_free (ch->pipe_name);
        ch->pipe_name = NULL;
    }

    ch->ready = FALSE;
}


/****************************************************************************/
/* WIN32
 */
#ifdef __WIN32__

gboolean
moo_app_output_start (MooAppOutput *ch)
{
#warning "Implement me"
    return FALSE;
}

#endif /* __WIN32__ */


/****************************************************************************/
/* UNIX
 */
#ifndef __WIN32__

gboolean
moo_app_output_start (MooAppOutput *ch)
{
    g_return_val_if_fail (!ch->ready, FALSE);

    ch->pipe_name = g_strdup_printf ("%s/%s_out.%d", g_get_tmp_dir(),
                                     ch->pipe_basename, getpid ());
    unlink (ch->pipe_name);

    if (mkfifo (ch->pipe_name, S_IRUSR | S_IWUSR))
    {
        perror ("mkfifo");
        goto error;
    }

    ch->in = open (ch->pipe_name, O_RDONLY | O_NONBLOCK);

    if (ch->in == -1)
    {
        perror ("open");
        goto error;
    }

    ch->out = open (ch->pipe_name, O_WRONLY | O_NONBLOCK);

    if (ch->out == -1)
    {
        perror ("open");
        goto error;
    }

    ch->ready = TRUE;
    return TRUE;

error:
    g_free (ch->pipe_name);
    ch->pipe_name = NULL;
    if (ch->in >= 0)
        close (ch->in);
    if (ch->out >= 0)
        close (ch->out);
    ch->in = -1;
    ch->out = -1;
    return FALSE;
}


void
moo_app_output_write (MooAppOutput   *ch,
                      const char     *data,
                      gssize          len)
{
    g_return_if_fail (ch != NULL && data != NULL);
    g_return_if_fail (ch->ready);

    if (!len)
        return;

    if (len < 0)
        len = strlen (data);

    /* XXX make a buffer, and so on */

    while (TRUE)
    {
        gssize result = write (ch->out, data, len);

        if (result < 0)
        {
            int err = errno;

            switch (err)
            {
                case EAGAIN:
                    perror ("write");
                    return;

                case EINTR:
                    continue;

                default:
                    perror ("write");
                    return;
            }
        }
        else if (result < len)
        {
            data += result;
            len -= result;
        }
    }
}


void
moo_app_output_flush (MooAppOutput   *ch)
{
#define BUF_LEN 1024
    char buf[BUF_LEN];

    g_return_if_fail (ch != NULL);

    if (!ch->ready)
        return;

    /* XXX make a buffer, and so on */

    while (TRUE)
    {
        gssize result = read (ch->in, buf, BUF_LEN);

        if (result < 0)
        {
            int err = errno;

            switch (err)
            {
                case EAGAIN:
                    return;

                case EINTR:
                    continue;

                default:
                    perror ("read");
                    return;
            }
        }
    }
}

#endif /* ! __WIN32__ */
