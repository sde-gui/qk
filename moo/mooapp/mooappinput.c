/*
 *   mooapp/mooappinput.c
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
#include "mooapp/mooappinput.h"
#include "mooapp/mooapp.h"


#define MAX_BUFFER_SIZE 4096


/*********************************************************************/
/*********************************************************************/

static gboolean read_input              (GIOChannel     *source,
                                         GIOCondition    condition,
                                         MooAppInput      *ch);
static void     commit                  (MooAppInput      *ch);


MooAppInput *moo_app_input_new      (const char     *pipe_basename)
{
    MooAppInput *ch;

    g_return_val_if_fail (pipe_basename != NULL, NULL);

    ch = g_new0 (MooAppInput, 1);
    ch->ref_count = 1;

    ch->pipe_basename = g_strdup (pipe_basename);

#ifdef __WIN32__
    ch->listener = NULL;
#else /* ! __WIN32__ */
    ch->pipe = -1;
#endif /* ! __WIN32__ */
    ch->pipe_name = NULL;
    ch->io = NULL;
    ch->io_watch = 0;
    ch->ready = FALSE;
    ch->buffer = g_byte_array_new ();

    return ch;
}


MooAppInput *moo_app_input_ref          (MooAppInput    *ch)
{
    g_return_val_if_fail (ch != NULL, NULL);
    ++ch->ref_count;
    return ch;
}


void         moo_app_input_unref        (MooAppInput    *ch)
{
    g_return_if_fail (ch != NULL);

    if (--ch->ref_count)
        return;

    moo_app_input_shutdown (ch);

    g_byte_array_free (ch->buffer, TRUE);
    g_free (ch->pipe_basename);
    g_free (ch);
}


void
moo_app_input_shutdown (MooAppInput *ch)
{
    g_return_if_fail (ch != NULL);

#ifdef __WIN32__
    if (ch->listener)
    {
        CloseHandle (ch->listener);
        ch->listener = NULL;
    }
#endif /* __WIN32__ */

    if (ch->io)
    {
        g_io_channel_shutdown (ch->io, TRUE, NULL);
        g_io_channel_unref (ch->io);
        ch->io = NULL;
    }

    if (ch->pipe_name)
    {
#ifndef __WIN32__
        ch->pipe  = -1;
        unlink (ch->pipe_name);
#endif /* ! __WIN32__ */
        g_free (ch->pipe_name);
        ch->pipe_name = NULL;
    }

    if (ch->io_watch)
    {
        g_source_remove (ch->io_watch);
        ch->io_watch = 0;
    }

    ch->ready = FALSE;
}


static void
commit (MooAppInput *self)
{
    g_assert (self->buffer->len > 0 && self->buffer->data[self->buffer->len-1] == 0);

    moo_app_input_ref (self);

    if (self->buffer->len <= 1)
        g_warning ("%s: got empty command", G_STRLOC);
    else
        _moo_app_exec_cmd (moo_app_get_instance (),
                           self->buffer->data[0],
                           (char*) self->buffer->data + 1,
                           self->buffer->len - 2);

    if (self->buffer->len > MAX_BUFFER_SIZE)
    {
        g_byte_array_free (self->buffer, TRUE);
        self->buffer = g_byte_array_new ();
    }
    else
    {
        g_byte_array_set_size (self->buffer, 0);
    }

    moo_app_input_unref (self);
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


gboolean     moo_app_input_start       (MooAppInput *ch)
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
                                         MooAppInput      *self)
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
        moo_app_input_shutdown (self);
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

/* TODO: could you finally learn non-blocking io? */
gboolean     moo_app_input_start       (MooAppInput *ch)
{
    g_return_val_if_fail (!ch->ready, FALSE);

    ch->pipe_name =
            g_strdup_printf ("%s/%s_in.%d",
                             g_get_tmp_dir(),
                             ch->pipe_basename,
                             getpid ());
    unlink (ch->pipe_name);

    if (mkfifo (ch->pipe_name, S_IRUSR | S_IWUSR))
    {
        int err = errno;
        g_critical ("%s: error in mkfifo()", G_STRLOC);
        g_critical ("%s: %s", G_STRLOC, g_strerror (err));
        return FALSE;
    }

    /* XXX posix man page says results of this are undefined */
    ch->pipe = open (ch->pipe_name, O_RDWR | O_NONBLOCK);
    if (ch->pipe == -1)
    {
        int err = errno;
        g_critical ("%s: error in open()", G_STRLOC);
        g_critical ("%s: %s", G_STRLOC, g_strerror (err));
        g_free (ch->pipe_name);
        ch->pipe_name = NULL;
        return FALSE;
    }

    ch->io = g_io_channel_unix_new (ch->pipe);
    g_io_channel_set_encoding (ch->io, NULL, NULL);
    ch->io_watch = g_io_add_watch (ch->io,
                                   G_IO_IN | G_IO_PRI | G_IO_ERR | G_IO_HUP,
                                   (GIOFunc) read_input,
                                   ch);

    ch->ready = TRUE;
    return TRUE;
}


static gboolean
read_input (G_GNUC_UNUSED GIOChannel     *source,
            GIOCondition    condition,
            MooAppInput      *self)
{
    gboolean error_occured = FALSE;
    GError *err = NULL;
    gboolean again = TRUE;
    gboolean got_zero = FALSE;

    if (condition & (G_IO_ERR | G_IO_HUP))
        if (errno != EINTR && errno != EAGAIN)
            error_occured = TRUE;

    while (again && !error_occured && !err)
    {
        char c;
        int bytes_read;

        struct pollfd fd = {self->pipe, POLLIN | POLLPRI, 0};

        int res = poll (&fd, 1, 0);

        switch (res)
        {
            case -1:
                if (errno != EINTR && errno != EAGAIN)
                    error_occured = TRUE;
                perror ("poll");
                break;

            case 0:
                again = FALSE;
                break;

            case 1:
                if (fd.revents & (POLLERR))
                {
                    if (errno != EINTR && errno != EAGAIN)
                        error_occured = TRUE;
                    perror ("poll");
                }
                else
                {
                    bytes_read = read (self->pipe, &c, 1);

                    if (bytes_read == 1)
                    {
                        g_byte_array_append (self->buffer, (guint8*) &c, 1);

                        if (!c)
                        {
                            got_zero = TRUE;
                            again = FALSE;
                        }
                    }
                    else if (bytes_read == -1)
                    {
                        perror ("read");
                        if (errno != EINTR && errno != EAGAIN)
                            error_occured = TRUE;
                    }
                    else
                    {
                        again = FALSE;
                    }
                }
                break;

            default:
                g_assert_not_reached ();
        }
    }

    if (error_occured || err)
    {
        g_critical ("%s: error", G_STRLOC);

        if (err)
        {
            g_critical ("%s: %s", G_STRLOC, err->message);
            g_error_free (err);
        }

        moo_app_input_shutdown (self);
        return FALSE;
    }

    if (got_zero)
        commit (self);

    return TRUE;
}

#endif /* ! __WIN32__ */
