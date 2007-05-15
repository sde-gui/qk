/*
 *   mooapp/mooappinput.c
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
#include "mooutils/mooutils-misc.h"
#include "mooutils/mooutils-thread.h"


struct _MooAppInput
{
    guint        ref_count;

    int          pipe;
    char        *pipe_basename;
    char        *pipe_name;
    char        *pipe_dir;
    GIOChannel  *io;
    guint        io_watch;
    GString     *buffer; /* messages are zero-terminated */
    gboolean     ready;

#ifdef __WIN32__
    guint        event_id;
#endif /* __WIN32__ */
};


#define MAX_BUFFER_SIZE 4096


MooAppInput *
_moo_app_input_new (const char *pipe_basename)
{
    MooAppInput *ch;

    g_return_val_if_fail (pipe_basename != NULL, NULL);

    ch = g_new0 (MooAppInput, 1);
    ch->ref_count = 1;

    ch->pipe_basename = g_strdup (pipe_basename);

#ifndef __WIN32__
    ch->pipe = -1;
#endif /* ! __WIN32__ */
    ch->pipe_name = NULL;
    ch->io = NULL;
    ch->io_watch = 0;
    ch->ready = FALSE;
    ch->buffer = g_string_new_len (NULL, MAX_BUFFER_SIZE);

    return ch;
}


static MooAppInput *
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
    if (ch->event_id)
    {
        _moo_event_queue_disconnect (ch->event_id);
        ch->event_id = 0;
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

    if (ch->pipe_dir)
    {
        remove (ch->pipe_dir);
        g_free (ch->pipe_dir);
        ch->pipe_dir = NULL;
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
    char buf[MAX_BUFFER_SIZE];
    GString *freeme = NULL;
    char *ptr;
    gsize len;

    if (!self->buffer->len)
    {
        g_warning ("%s: got empty command", G_STRLOC);
        return;
    }

    if (self->buffer->len + 1 > MAX_BUFFER_SIZE)
    {
        freeme = self->buffer;
        self->buffer = g_string_new_len (NULL, MAX_BUFFER_SIZE);
        ptr = freeme->str;
        len = freeme->len;
    }
    else
    {
        memcpy (buf, self->buffer->str, self->buffer->len + 1);
        ptr = buf;
        len = self->buffer->len;
        g_string_truncate (self->buffer, 0);
    }

    if (0)
        g_print ("%s: commit %c\n%s\n-----\n", G_STRLOC, ptr[0], ptr + 1);

    _moo_app_exec_cmd (moo_app_get_instance (), ptr[0], ptr + 1, len - 1);

    if (freeme)
        g_string_free (freeme, TRUE);
}


/****************************************************************************/
/* WIN32
 */
#ifdef __WIN32__


typedef struct {
    char *pipe_name;
    guint event_id;
} ListenerInfo;

static ListenerInfo *
listener_info_new (const char *pipe_name,
                   guint       event_id)
{
    ListenerInfo *info = g_new (ListenerInfo, 1);
    info->pipe_name = g_strdup (pipe_name);
    info->event_id = event_id;
    return info;
}

static void
listener_info_free (ListenerInfo *info)
{
    if (info)
    {
        g_free (info->pipe_name);
        g_free (info);
    }
}


static gpointer
listener_main (ListenerInfo *info)
{
    HANDLE input;

    _moo_message_async ("%s: hi there", G_STRLOC);

    input = CreateNamedPipe (info->pipe_name,
                             PIPE_ACCESS_DUPLEX,
                             PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                             PIPE_UNLIMITED_INSTANCES,
                             0, 0, 200, NULL);

    if (input == INVALID_HANDLE_VALUE)
    {
        _moo_message_async ("%s: could not create input pipe", G_STRLOC);
        listener_info_free (info);
        return NULL;
    }

    _moo_message_async ("%s: opened pipe %s", G_STRLOC, info->pipe_name);

    while (TRUE)
    {
        DWORD bytes_read;
        char c;

        DisconnectNamedPipe (input);
        _moo_message_async ("%s: opening connection", G_STRLOC);

        if (!ConnectNamedPipe (input, NULL))
        {
            DWORD err = GetLastError();

            if (err != ERROR_PIPE_CONNECTED)
            {
                char *msg = g_win32_error_message (err);
                _moo_message_async ("%s: error in ConnectNamedPipe()", G_STRLOC);
                _moo_message_async ("%s: %s", G_STRLOC, msg);
                CloseHandle (input);
                g_free (msg);
                break;
            }
        }

        _moo_message_async ("%s: client connected", G_STRLOC);

        while (ReadFile (input, &c, 1, &bytes_read, NULL))
        {
            if (bytes_read == 1)
            {
                _moo_event_queue_push (info->event_id, GINT_TO_POINTER ((int) c), NULL);
            }
            else
            {
                _moo_message_async ("%s: client disconnected", G_STRLOC);
                break;
            }
        }
    }

    _moo_message_async ("%s: goodbye", G_STRLOC);

    CloseHandle (input);
    listener_info_free (info);
    return NULL;
}


static char *
get_pipe_name (const char *pipe_basename,
               const char *pid_string)
{
    if (!pid_string)
        pid_string = _moo_get_pid_string ();

    return g_strdup_printf ("\\\\.\\pipe\\%s_in_%s",
                            pipe_basename, pid_string);
}


static void
event_callback (GList       *events,
                MooAppInput *chan)
{
    while (events)
    {
        char c = GPOINTER_TO_INT (events->data);

        if (c != 0)
            g_string_append_c (chan->buffer, c);
        else
        {
            gdk_threads_enter ();
            commit (chan);
            gdk_threads_leave ();
        }

        events = events->next;
    }
}


gboolean
_moo_app_input_start (MooAppInput *ch)
{
    ListenerInfo *info;

    g_return_val_if_fail (ch != NULL && !ch->ready, FALSE);

    g_free (ch->pipe_name);
    ch->pipe_name = get_pipe_name (ch->pipe_basename, NULL);
    ch->event_id = _moo_event_queue_connect ((MooEventQueueCallback) event_callback,
                                             ch, NULL);

    info = listener_info_new (ch->pipe_name, ch->event_id);

    if (!g_thread_create ((GThreadFunc) listener_main, info, FALSE, NULL))
    {
        g_critical ("could not start listener thread");
        listener_info_free (info);
        g_free (ch->pipe_name);
        ch->pipe_name = NULL;
        _moo_event_queue_disconnect (ch->event_id);
        ch->event_id = 0;
        return FALSE;
    }

    ch->ready = TRUE;
    return TRUE;
}


gboolean
_moo_app_input_send_msg (const char *pipe_basename,
                         const char *pid,
                         const char *data,
                         gssize      len)
{
    char *err_msg = NULL;
    char *pipe_name;
    HANDLE pipe_handle;
    gboolean result = FALSE;
    DWORD bytes_written;

    g_return_val_if_fail (pipe_basename != NULL, FALSE);
    g_return_val_if_fail (data != NULL, FALSE);

    if (len < 0)
        len = strlen (data);

    if (!len)
        return TRUE;

    if (!pid)
        return FALSE;

    pipe_name = get_pipe_name (pipe_basename, pid);
    pipe_handle = CreateFile (pipe_name, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

    if (!pipe_handle)
    {
        err_msg = g_win32_error_message (GetLastError ());
        g_warning ("could not open pipe '%s': %s", pipe_name, err_msg);
        goto out;
    }

    if (!WriteFile(pipe_handle, data, len, &bytes_written, NULL))
    {
        err_msg = g_win32_error_message (GetLastError ());
        g_warning ("could not write data to '%s': %s", pipe_name, err_msg);
        goto out;
    }

    if (bytes_written < (DWORD) len)
    {
        g_warning ("written less data than requested to '%s'", pipe_name);
        goto out;
    }

    result = TRUE;

out:
    if (pipe_handle != INVALID_HANDLE_VALUE)
        CloseHandle (pipe_handle);

    g_free (pipe_name);
    g_free (err_msg);
    return result;
}

#endif /* __WIN32__ */


/****************************************************************************/
/* UNIX
 */
#ifndef __WIN32__

#define INPUT_PREFIX "input-"

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

    /* XXX */
    if (condition & (G_IO_ERR | G_IO_HUP))
        error_occured = TRUE;

    while (again && !error_occured && !err)
    {
        char c;
        int bytes_read;

        struct pollfd fd;

        fd.fd = self->pipe;
        fd.events = POLLIN | POLLPRI;
        fd.revents = 0;

        switch (poll (&fd, 1, 0))
        {
            case -1:
                if (errno != EINTR && errno != EAGAIN)
                    error_occured = TRUE;
                perror ("poll");
                again = FALSE;
                break;

            case 0:
                if (0)
                    g_print ("%s: got nothing\n", G_STRLOC);
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
        g_critical ("%s: %s", G_STRLOC, err ? err->message : "error");

        if (err)
            g_error_free (err);

        _moo_app_input_shutdown (self);
        return FALSE;
    }

    if (got_zero)
        commit (self);

    return TRUE;
}


static char *
get_pipe_dir (const char *pipe_basename)
{
    GdkDisplay *display;
    char *display_name;
    char *user_name;
    char *name;

    g_return_val_if_fail (pipe_basename != NULL, NULL);

    display = gdk_display_get_default ();
    g_return_val_if_fail (display != NULL, NULL);

    display_name = g_strcanon (g_strdup (gdk_display_get_name (display)),
                               G_CSET_A_2_Z G_CSET_a_2_z G_CSET_DIGITS,
                               '-');
    user_name = g_strcanon (g_strdup (g_get_user_name ()),
                            G_CSET_A_2_Z G_CSET_a_2_z G_CSET_DIGITS,
                            '-');

    name = g_strdup_printf ("%s/%s-%s-%s", g_get_tmp_dir (), pipe_basename, user_name,
                            display_name[0] == '-' ? &display_name[1] : display_name);

    g_free (display_name);
    g_free (user_name);
    return name;
}

/* TODO: could you finally learn non-blocking io? */
gboolean
_moo_app_input_start (MooAppInput *ch)
{
    GSource *source;

    g_return_val_if_fail (!ch->ready, FALSE);

    if (!ch->pipe_dir)
        ch->pipe_dir = get_pipe_dir (ch->pipe_basename);
    g_return_val_if_fail (ch->pipe_dir != NULL, FALSE);

    mkdir (ch->pipe_dir, S_IRWXU);

    ch->pipe_name = g_strdup_printf ("%s/"INPUT_PREFIX"%d",
                                     ch->pipe_dir,
                                     getpid ());

    unlink (ch->pipe_name);

    if (mkfifo (ch->pipe_name, S_IRUSR | S_IWUSR))
    {
        int err = errno;
        g_critical ("%s: error in mkfifo()", G_STRLOC);
        g_critical ("%s: %s", G_STRLOC, g_strerror (err));
        return FALSE;
    }

    /* XXX O_RDWR is not good (man 3p open), but it must be opened for
     * writing by us, otherwise we get POLLHUP when a writer dies on the other end.
     * So, open for writing separately. */
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

    _moo_message ("%s: opened input pipe %s with fd %d",
                  G_STRLOC, ch->pipe_name, ch->pipe);

    ch->io = g_io_channel_unix_new (ch->pipe);
    g_io_channel_set_encoding (ch->io, NULL, NULL);
    ch->io_watch = _moo_io_add_watch (ch->io,
                                      G_IO_IN | G_IO_PRI | G_IO_HUP | G_IO_ERR,
                                      (GIOFunc) read_input,
                                      ch);

    source = g_main_context_find_source_by_id (NULL, ch->io_watch);
    g_source_set_can_recurse (source, TRUE);

    ch->ready = TRUE;
    return TRUE;
}


static gboolean
try_send (const char *pipe_dir_name,
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
    GError *error = NULL;

    if (!pid_string[0])
        goto out;

    errno = 0;
    pid = strtol (pid_string, &endptr, 10);

    if (errno != 0 || endptr == pid_string || *endptr != 0)
    {
        g_warning ("invalid pid string '%s'", pid_string);
        goto out;
    }

    filename = g_strdup_printf ("%s/"INPUT_PREFIX"%s", pipe_dir_name, pid_string);
    _moo_message ("try_send: sending data to pid %s, filename %s", pid_string, filename);

    if (!g_file_test (filename, G_FILE_TEST_EXISTS))
    {
        _moo_message ("try_send: file %s doesn't exist", filename);
        goto out;
    }

    if (kill (pid, 0) != 0)
    {
        _moo_message ("try_send: no process with pid %d", pid);
        unlink (filename);
        goto out;
    }

    chan = g_io_channel_new_file (filename, "w", &error);

    if (!chan)
    {
        _moo_message ("try_send: could not open %s for writing: %s",
                      filename, error ? error->message : "<?>");
        g_error_free (error);
        goto out;
    }

    g_io_channel_set_encoding (chan, NULL, NULL);
    status = g_io_channel_set_flags (chan, G_IO_FLAG_NONBLOCK, &error);

    if (status != G_IO_STATUS_NORMAL)
    {
        _moo_message ("try_send: could not set NONBLOCK flag: %s",
                      error ? error->message : "<?>");
        goto out;
    }

    status = g_io_channel_write_chars (chan, data, data_len, NULL, &error);

    if (status != G_IO_STATUS_NORMAL)
    {
        _moo_message ("try_send: error writing to pipe: %s",
                      error ? error->message : "<?>");
        goto out;
    }

    result = TRUE;
    _moo_message ("try_send: successfully sent stuff to pid %s", pid_string);

out:
    if (chan)
        g_io_channel_unref (chan);
    if (error)
        g_error_free (error);
    g_free (filename);
    return result;
}

gboolean
_moo_app_input_send_msg (const char *pipe_basename,
                         const char *pid,
                         const char *data,
                         gssize      data_len)
{
    const char *entry;
    GDir *pipe_dir = NULL;
    char *pipe_dir_name;
    gboolean success = FALSE;

    g_return_val_if_fail (pipe_basename != NULL, FALSE);
    g_return_val_if_fail (data != NULL, FALSE);

    _moo_message ("_moo_app_input_send_msg: sending data to pid %s", pid ? pid : "NONE");

    pipe_dir_name = get_pipe_dir (pipe_basename);
    g_return_val_if_fail (pipe_dir_name != NULL, FALSE);

    if (pid)
    {
        success = try_send (pipe_dir_name, pid, data, data_len);
        goto out;
    }

    pipe_dir = g_dir_open (pipe_dir_name, 0, NULL);

    if (!pipe_dir)
        goto out;

    while ((entry = g_dir_read_name (pipe_dir)))
    {
        if (!strncmp (entry, INPUT_PREFIX, strlen (INPUT_PREFIX)))
        {
            const char *pid_string = entry + strlen (INPUT_PREFIX);

            if (try_send (pipe_dir_name, pid_string, data, data_len))
            {
                success = TRUE;
                goto out;
            }
        }
    }

out:
    if (pipe_dir)
        g_dir_close (pipe_dir);
    g_free (pipe_dir_name);
    return success;
}

#endif /* ! __WIN32__ */
