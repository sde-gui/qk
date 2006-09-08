/*
 *   mooapp/mooappinput.c
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
#include <signal.h>
#endif /* !__WIN32__ */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "mooapp/mooappinput.h"
#define MOO_APP_COMPILATION
#include "mooapp/mooapp-private.h"


struct _MooAppInput
{
    guint        ref_count;

    int          pipe;
    char        *pipe_basename;
    char        *pipe_name;
    GIOChannel  *io;
    guint        io_watch;
    GString     *buffer; /* messages are zero-terminated */
    gboolean     ready;

#ifdef __WIN32__
    HANDLE       listener;
#endif /* __WIN32__ */
};


#define MAX_BUFFER_SIZE 4096


static gboolean read_input              (GIOChannel     *source,
                                         GIOCondition    condition,
                                         MooAppInput      *ch);
static void     commit                  (MooAppInput      *ch);


MooAppInput *
_moo_app_input_new (const char *pipe_basename)
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
    ch->buffer = g_string_new_len (NULL, MAX_BUFFER_SIZE);

    return ch;
}


MooAppInput *
_moo_app_input_ref (MooAppInput *ch)
{
    g_return_val_if_fail (ch != NULL, NULL);
    ++ch->ref_count;
    return ch;
}


void
_moo_app_input_unref (MooAppInput *ch)
{
    g_return_if_fail (ch != NULL);

    if (--ch->ref_count)
        return;

    _moo_app_input_shutdown (ch);

    g_string_free (ch->buffer, TRUE);
    g_free (ch->pipe_basename);
    g_free (ch);
}


void
_moo_app_input_shutdown (MooAppInput *ch)
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


const char *
_moo_app_input_get_name (MooAppInput *ch)
{
    g_return_val_if_fail (ch != NULL, NULL);
    return ch->pipe_name;
}


static void
commit (MooAppInput *self)
{
    _moo_app_input_ref (self);
//     g_print ("%s: commit %c\n%s\n-----\n", G_STRLOC,
//              self->buffer->str[0], self->buffer->str + 1);

    if (!self->buffer->len)
        g_warning ("%s: got empty command", G_STRLOC);
    else
        _moo_app_exec_cmd (moo_app_get_instance (),
                           self->buffer->str[0],
                           self->buffer->str + 1,
                           self->buffer->len - 1);

    if (self->buffer->len > MAX_BUFFER_SIZE)
    {
        g_string_free (self->buffer, TRUE);
        self->buffer = g_string_new_len (NULL, MAX_BUFFER_SIZE);
    }
    else
    {
        g_string_truncate (self->buffer, 0);
    }

    _moo_app_input_unref (self);
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


gboolean
_moo_app_input_start (MooAppInput *ch)
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


gboolean
_moo_app_input_send_msg (G_GNUC_UNUSED const char *pipe_basename,
                         G_GNUC_UNUSED const char *pid,
                         G_GNUC_UNUSED const char *data,
                         G_GNUC_UNUSED gssize len)
{
    g_return_val_if_fail (pipe_basename != NULL, FALSE);
    g_return_val_if_fail (data != NULL, FALSE);

#ifdef __GNUC__
#warning "Implement _moo_app_input_send_msg()"
#endif

    return FALSE;
}


static gboolean
read_input (GIOChannel     *source,
            GIOCondition    condition,
            MooAppInput    *self)
{
    gboolean error_occured = FALSE;
    GError *err = NULL;
    gboolean again = TRUE;
    gboolean got_zero = FALSE;
    char c;
    guint bytes_read;

    if (condition & (G_IO_ERR | G_IO_HUP))
    {
        if (errno != EAGAIN)
            error_occured = TRUE;
        else
            again = FALSE;
    }

    g_io_channel_read_chars (source, &c, 1, &bytes_read, &err);

    if (bytes_read == 1)
    {
        /* XXX Why do I ignore \r ??? */
        if (c != '\r')
            g_string_append_c (self->buffer, c);

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
        if (g_io_channel_get_buffer_condition (source) & G_IO_IN)
        {
            g_io_channel_read_chars (source, &c, 1, &bytes_read, &err);

            if (bytes_read == 1)
            {
                /* XXX Why do I ignore \r ??? */
                if (c != '\r')
                    g_string_append_c (self->buffer, c);

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
        }
        else
        {
            again = FALSE;
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

        _moo_app_input_shutdown (self);
        return FALSE;
    }

    if (got_zero)
        commit (self);

    return TRUE;
}


static DWORD WINAPI
listener_main (ListenerInfo *info)
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
                char *msg = g_win32_error_message (err);
                g_critical ("%s: error in ConnectNamedPipe()", G_STRLOC);
                g_critical ("%s: %s", G_STRLOC, msg);
                CloseHandle (input);
                g_free (msg);
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

static char *
get_prefix (const char *pipe_basename)
{
    GdkDisplay *display;
    char *display_name;
    char *user_name;
    char *prefix;

    g_return_val_if_fail (pipe_basename != NULL, NULL);

    display = gdk_display_get_default ();
    g_return_val_if_fail (display != NULL, NULL);

    display_name = g_strcanon (g_strdup (gdk_display_get_name (display)),
                               G_CSET_A_2_Z G_CSET_a_2_z G_CSET_DIGITS,
                               '_');
    user_name = g_strcanon (g_strdup (g_get_user_name ()),
                            G_CSET_A_2_Z G_CSET_a_2_z G_CSET_DIGITS,
                            '_');

    prefix = g_strdup_printf ("%s_%s_%s_in.", pipe_basename, user_name, display_name);

    g_free (display_name);
    g_free (user_name);
    return prefix;
}

/* TODO: could you finally learn non-blocking io? */
gboolean
_moo_app_input_start (MooAppInput *ch)
{
    char *prefix;

    g_return_val_if_fail (!ch->ready, FALSE);

    prefix = get_prefix (ch->pipe_basename);
    g_return_val_if_fail (prefix != NULL, FALSE);

    ch->pipe_name =
            g_strdup_printf ("%s/%s%d",
                             g_get_tmp_dir(),
                             prefix,
                             getpid ());
    g_free (prefix);

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
try_send (const char *tmpdir_name,
          const char *prefix,
          const char *pid_string,
          const char *data,
          gssize      data_len)
{
    GPid pid;
    char *filename = NULL;
    GIOChannel *chan = NULL;
    GIOStatus status;
    gboolean result = FALSE;
    char *endptr;

    if (!pid_string[0])
        goto out;

    errno = 0;
    pid = strtol (pid_string, &endptr, 10);

    if (errno != 0 || endptr == pid_string || *endptr != 0)
    {
        g_warning ("invalid pid string '%s'", pid_string);
        goto out;
    }

    filename = g_strdup_printf ("%s/%s%s", tmpdir_name, prefix, pid_string);

    if (!g_file_test (filename, G_FILE_TEST_EXISTS))
        goto out;

    if (kill (pid, 0) != 0)
    {
        unlink (filename);
        goto out;
    }

    chan = g_io_channel_new_file (filename, "w", NULL);

    if (!chan)
        goto out;

    g_io_channel_set_encoding (chan, NULL, NULL);
    status = g_io_channel_set_flags (chan, G_IO_FLAG_NONBLOCK, NULL);

    if (status != G_IO_STATUS_NORMAL)
        goto out;

    status = g_io_channel_write_chars (chan, data, data_len, NULL, NULL);

    if (status != G_IO_STATUS_NORMAL)
        goto out;

    result = TRUE;

out:
    if (chan)
        g_io_channel_unref (chan);
    g_free (filename);
    return result;
}

gboolean
_moo_app_input_send_msg (const char     *pipe_basename,
                         const char     *pid,
                         const char     *data,
                         gssize          data_len)
{
    const char *tmpdir_name, *entry;
    GDir *tmpdir = NULL;
    char *prefix = NULL;
    guint prefix_len;
    gboolean success = FALSE;

    g_return_val_if_fail (pipe_basename != NULL, FALSE);
    g_return_val_if_fail (data != NULL, FALSE);

    prefix = get_prefix (pipe_basename);
    g_return_val_if_fail (prefix != NULL, FALSE);

    prefix_len = strlen (prefix);
    tmpdir_name = g_get_tmp_dir ();

    if (pid)
    {
        success = try_send (tmpdir_name, prefix, pid, data, data_len);
        goto out;
    }

    tmpdir = g_dir_open (tmpdir_name, 0, NULL);

    if (!tmpdir)
        goto out;

    while ((entry = g_dir_read_name (tmpdir)))
    {
        if (!strncmp (entry, prefix, prefix_len))
        {
            const char *pid_string = entry + prefix_len;

            if (try_send (tmpdir_name, prefix, pid_string, data, data_len))
            {
                success = TRUE;
                goto out;
            }
        }
    }

out:
    if (tmpdir)
        g_dir_close (tmpdir);
    g_free (prefix);
    return success;
}


static gboolean
read_input (GIOChannel   *source,
            GIOCondition  condition,
            MooAppInput  *self)
{
    gboolean error_occured = FALSE;
    GError *err = NULL;
    gboolean again = TRUE;
    gboolean got_zero = FALSE;

    g_return_val_if_fail (source == self->io, FALSE);

    if (condition & (G_IO_ERR | G_IO_HUP))
    {
        if (errno != EINTR && errno != EAGAIN)
            error_occured = TRUE;
        else if (!(condition & (G_IO_IN | G_IO_PRI)))
            again = FALSE;
    }

    while (again && !error_occured && !err)
    {
        char c;
        int bytes_read;

        struct pollfd fd = {self->pipe, POLLIN | POLLPRI, 0};

        switch (poll (&fd, 1, 0))
        {
            case -1:
                if (errno != EINTR && errno != EAGAIN)
                    error_occured = TRUE;
                perror ("poll");
                again = FALSE;
                break;

            case 0:
//                 g_print ("%s: got nothing\n", G_STRLOC);
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
                        g_string_append_c (self->buffer, c);

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

                        again = FALSE;
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

        _moo_app_input_shutdown (self);
        return FALSE;
    }

    if (got_zero)
        commit (self);

    return TRUE;
}

#endif /* ! __WIN32__ */
