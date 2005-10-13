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


typedef struct {
    char *in;
    int out;
} ListenerInfo;

static ListenerInfo *listener_info_new (const char *input, int output)
{
    ListenerInfo *info = g_new (ListenerInfo, 1);
    info->in = g_strdup (input);
    info->out = output;
    return info;
}

static void listener_info_free (ListenerInfo *info)
{
    if (!info) return;
    g_free (info->in);
    g_free (info);
}

static DWORD WINAPI listener_main (ListenerInfo *info);


gboolean     moo_app_output_start       (MooAppOutput *ch)
{
    int listener_pipe[2] = {-1, -1};
    DWORD id;
    ListenerInfo *info;

    g_return_val_if_fail (ch != NULL && !ch->ready, FALSE);

    g_free (ch->pipe_name);
    ch->pipe_name =
            g_strdup_printf ("\\\\.\\pipe\\%s_in_%ld",
                             ch->pipe_basename,
                             GetCurrentProcessId());

    if (_pipe (listener_pipe, 4096, _O_BINARY | _O_NOINHERIT))
    {
        g_critical ("%s: could not create listener pipe", G_STRLOC);
        g_free (ch->pipe_name);
        ch->pipe_name = NULL;
        return FALSE;
    }

    info = listener_info_new (ch->pipe_name, listener_pipe[1]);
    ch->listener = CreateThread (NULL, 0,
                                 (LPTHREAD_START_ROUTINE) listener_main,
                                 info, 0, &id);
    if (!ch->listener)
    {
        g_critical ("%s: could not start listener thread", G_STRLOC);
        listener_info_free (info);
        g_free (ch->pipe_name);
        ch->pipe_name = NULL;
        close (listener_pipe[0]);
        close (listener_pipe[1]);
        return FALSE;
    }

    ch->io = g_io_channel_win32_new_fd (listener_pipe[0]);
    g_io_channel_set_encoding (ch->io, NULL, NULL);
    g_io_channel_set_buffered (ch->io, FALSE);
    ch->io_watch = g_io_add_watch (ch->io, G_IO_IN | G_IO_PRI | G_IO_ERR | G_IO_HUP,
                                   (GIOFunc) read_input, ch);

    ch->ready = TRUE;
    return TRUE;
}


static gboolean read_input              (GIOChannel     *source,
                                         GIOCondition    condition,
                                         MooAppOutput      *self)
{
    gboolean error_occured = FALSE;
    GError *err = NULL;
    gboolean again = TRUE;
    gboolean got_zero = FALSE;
    char c;
    guint bytes_read;

    if (condition & (G_IO_ERR | G_IO_HUP))
        if (errno != EAGAIN)
            error_occured = TRUE;

    g_io_channel_read_chars (source, &c, 1, &bytes_read, &err);

    if (bytes_read == 1)
    {
        if (c != '\r')
            g_byte_array_append (self->buffer, &c, 1);

        if (!c)
        {
            got_zero = TRUE;
            again = FALSE;
        }
    }
    else
    {
        again = FALSE;
    }

    while (again && !error_occured && !err)
    {
        if (g_io_channel_get_buffer_condition (source) & G_IO_IN) {
            g_io_channel_read_chars (source, &c, 1, &bytes_read, &err);
            if (bytes_read == 1)
            {
                if (c != '\r')
                    g_byte_array_append (self->buffer, &c, 1);

                if (!c)
                {
                    got_zero = TRUE;
                    again = FALSE;
                }
            }
            else
                again = FALSE;
        }
        else
            again = FALSE;
    }

    if (error_occured || err)
    {
        g_critical ("%s: error", G_STRLOC);
        if (err) {
            g_critical ("%s: %s", G_STRLOC, err->message);
            g_error_free (err);
        }
        moo_app_output_shutdown (self);
        return FALSE;
    }

    if (got_zero)
        commit (self);

    return TRUE;
}


static DWORD WINAPI listener_main (ListenerInfo *info)
{
    char *pipe_name;
    int output;
    HANDLE input;

    pipe_name = g_strdup (info->in);
    output = info->out;
    listener_info_free (info);

    g_message ("%s: hi there", G_STRLOC);

    input = CreateNamedPipe (pipe_name,
                             PIPE_ACCESS_DUPLEX,
                             PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                             PIPE_UNLIMITED_INSTANCES,
                             0, 0, 200, NULL);

    if (input == INVALID_HANDLE_VALUE)
    {
        g_critical ("%s: could not create input pipe", G_STRLOC);
        g_free (pipe_name);
        return 1;
    }
    else
    {
        g_message ("%s: opened pipe %s", G_STRLOC, pipe_name);
    }

    while (TRUE)
    {
        DWORD bytes_read;
        char c;

        DisconnectNamedPipe (input);
        g_message ("%s: opening connection", G_STRLOC);

        if (!ConnectNamedPipe (input, NULL))
        {
            DWORD err = GetLastError();
            if (err != ERROR_PIPE_CONNECTED)
            {
                g_critical ("%s: error in ConnectNamedPipe()", G_STRLOC);
                CloseHandle (input);
                break;
            }
        }

        g_message ("%s: client connected", G_STRLOC);

        while (ReadFile (input, &c, 1, &bytes_read, NULL))
        {
            if (bytes_read == 1)
            {
                if (_write (output, &c, 1) != 1)
                {
                    g_message ("%s: parent disconnected", G_STRLOC);
                    break;
                }
            }
            else
            {
                g_message ("%s: client disconnected", G_STRLOC);
                break;
            }
        }
    }

    g_message ("%s: goodbye", G_STRLOC);

    CloseHandle (input);
    g_free (pipe_name);
    close (output);
    return 0;
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
