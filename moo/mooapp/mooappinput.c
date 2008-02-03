/*
 *   mooapp/mooappinput.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef __WIN32__
#define MOO_APP_INPUT_WIN32
#elif defined(MOO_USE_PIPE_INPUT)
#define MOO_APP_INPUT_PIPE
#else
#define MOO_APP_INPUT_SOCKET
#endif

#if defined(MOO_APP_INPUT_WIN32)
# include <windows.h>
# include <io.h>
#elif defined(MOO_APP_INPUT_PIPE)
# include <sys/poll.h>
# include <signal.h>
#else
# include <sys/socket.h>
# include <sys/un.h>
#endif

#ifndef __WIN32__
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "mooapp/mooappinput.h"
#define MOO_APP_COMPILATION
#include "mooapp/mooapp-private.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/mooutils-thread.h"
#include "mooutils/mooutils-debug.h"

MOO_DEBUG_INIT(input, TRUE)

#define MAX_BUFFER_SIZE 4096

#ifdef MOO_APP_INPUT_SOCKET
#define INPUT_PREFIX "in-"
#else
#define INPUT_PREFIX "input-"
#endif

typedef struct InputChannel InputChannel;

struct _MooAppInput
{
    GSList *pipes;
    char *appname;
    char *main_path;
};

static InputChannel *input_channel_new      (const char     *appname,
                                             const char     *name,
                                             gboolean        may_fail);
static void          input_channel_free     (InputChannel   *ch);
static char         *input_channel_get_path (InputChannel   *ch);


MooAppInput *
_moo_app_input_new (const char *appname,
                    const char *name,
                    gboolean    bind_default)
{
    MooAppInput *ch;
    InputChannel *ich;

    g_return_val_if_fail (appname != NULL, NULL);

    ch = moo_new0 (MooAppInput);

    ch->pipes = NULL;
    ch->appname = g_strdup (appname);

    if ((ich = input_channel_new (appname, _moo_get_pid_string (), FALSE)))
    {
        ch->pipes = g_slist_prepend (ch->pipes, ich);
        ch->main_path = input_channel_get_path (ich);
    }

    if (name && (ich = input_channel_new (appname, name, FALSE)))
        ch->pipes = g_slist_prepend (ch->pipes, ich);

    if (bind_default && (ich = input_channel_new (appname, MOO_APP_INPUT_NAME_DEFAULT, TRUE)))
        ch->pipes = g_slist_prepend (ch->pipes, ich);

    return ch;
}

void
_moo_app_input_free (MooAppInput *ch)
{
    g_return_if_fail (ch != NULL);

    g_slist_foreach (ch->pipes, (GFunc) input_channel_free, NULL);
    g_slist_free (ch->pipes);

    g_free (ch->main_path);
    g_free (ch->appname);
    moo_free (MooAppInput, ch);
}


const char *
_moo_app_input_get_path (MooAppInput *ch)
{
    g_return_val_if_fail (ch != NULL, NULL);
    return ch->main_path;
}


static void
commit (GString **buffer)
{
    char buf[MAX_BUFFER_SIZE];
    GString *freeme = NULL;
    char *ptr;
    gsize len;

    if (!(*buffer)->len)
    {
        moo_dmsg ("%s: got empty command", G_STRLOC);
        return;
    }

    if ((*buffer)->len + 1 > MAX_BUFFER_SIZE)
    {
        freeme = *buffer;
        *buffer = g_string_new_len (NULL, MAX_BUFFER_SIZE);
        ptr = freeme->str;
        len = freeme->len;
    }
    else
    {
        memcpy (buf, (*buffer)->str, (*buffer)->len + 1);
        ptr = buf;
        len = (*buffer)->len;
        g_string_truncate (*buffer, 0);
    }

    if (0)
        g_print ("%s: commit %c\n%s\n-----\n", G_STRLOC, ptr[0], ptr + 1);

    _moo_app_exec_cmd (moo_app_get_instance (), ptr[0], ptr + 1, len - 1);

    if (freeme)
        g_string_free (freeme, TRUE);
}


#ifndef MOO_APP_INPUT_WIN32

static char *
get_pipe_dir (const char *appname)
{
    GdkDisplay *display;
    char *display_name;
    char *user_name;
    char *name;

    g_return_val_if_fail (appname != NULL, NULL);

    display = gdk_display_get_default ();
    g_return_val_if_fail (display != NULL, NULL);

    display_name = g_strcanon (g_strdup (gdk_display_get_name (display)),
                               G_CSET_A_2_Z G_CSET_a_2_z G_CSET_DIGITS,
                               '-');
    user_name = g_strcanon (g_strdup (g_get_user_name ()),
                            G_CSET_A_2_Z G_CSET_a_2_z G_CSET_DIGITS,
                            '-');

    name = g_strdup_printf ("%s/%s-%s-%s", g_get_tmp_dir (), appname, user_name,
                            display_name[0] == '-' ? &display_name[1] : display_name);

    g_free (display_name);
    g_free (user_name);
    return name;
}

static char *
get_pipe_path (const char *pipe_dir,
               const char *name)
{
    return g_strdup_printf ("%s/" INPUT_PREFIX "%s",
                            pipe_dir, name);
}

static gboolean
input_channel_start_io (int           fd,
                        GIOFunc       io_func,
                        gpointer      data,
                        GIOChannel  **io_channel,
                        guint        *io_watch)
{
    GSource *source;

    *io_channel = g_io_channel_unix_new (fd);
    g_io_channel_set_encoding (*io_channel, NULL, NULL);

    *io_watch = _moo_io_add_watch (*io_channel,
                                   G_IO_IN | G_IO_PRI | G_IO_HUP | G_IO_ERR,
                                   io_func, data);

    source = g_main_context_find_source_by_id (NULL, *io_watch);
    g_source_set_can_recurse (source, TRUE);

    return TRUE;
}


static gboolean do_send (const char *filename,
                         const char *data,
                         gssize      data_len);

static gboolean
try_send (const char *pipe_dir_name,
          const char *name,
          const char *data,
          gssize      data_len)
{
    char *filename = NULL;
    gboolean result = FALSE;

    g_return_val_if_fail (name && name[0], FALSE);

    filename = get_pipe_path (pipe_dir_name, name);
    moo_dmsg ("try_send: sending data to `%s'", filename);

    if (!g_file_test (filename, G_FILE_TEST_EXISTS))
    {
        moo_dmsg ("try_send: file %s doesn't exist", filename);
        goto out;
    }

    result = do_send (filename, data, data_len);

out:
    g_free (filename);
    return result;
}

gboolean
_moo_app_input_send_msg (const char *appname,
                         const char *name,
                         const char *data,
                         gssize      len)
{
    const char *entry;
    GDir *pipe_dir = NULL;
    char *pipe_dir_name;
    gboolean success = FALSE;

    g_return_val_if_fail (appname != NULL, FALSE);
    g_return_val_if_fail (data != NULL, FALSE);

    moo_dmsg ("_moo_app_input_send_msg: sending data to %s", name ? name : "NONE");

    pipe_dir_name = get_pipe_dir (appname);
    g_return_val_if_fail (pipe_dir_name != NULL, FALSE);

    if (name)
    {
        success = try_send (pipe_dir_name, name, data, len);
        goto out;
    }

    success = try_send (pipe_dir_name, MOO_APP_INPUT_NAME_DEFAULT, data, len);
    if (success)
        goto out;

    pipe_dir = g_dir_open (pipe_dir_name, 0, NULL);

    if (!pipe_dir)
        goto out;

    while ((entry = g_dir_read_name (pipe_dir)))
    {
        if (!strncmp (entry, INPUT_PREFIX, strlen (INPUT_PREFIX)))
        {
            name = entry + strlen (INPUT_PREFIX);

            if (try_send (pipe_dir_name, name, data, len))
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

static gboolean
do_write (int         fd,
          const char *data,
          gsize       data_len)
{
    while (data_len > 0)
    {
        ssize_t n;

        errno = 0;
        n = write (fd, data, data_len);

        if (n < 0)
        {
            if (errno != EAGAIN && errno != EINTR)
            {
                g_warning ("%s in write: %s", G_STRLOC, g_strerror (errno));
                return FALSE;
            }
        }
        else
        {
            data += n;
            data_len -= n;
        }
    }

    return TRUE;
}

#endif


#ifdef MOO_APP_INPUT_SOCKET

#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX 108
#endif

typedef struct {
    int fd;
    GIOChannel *io;
    guint io_watch;
    GString *buffer; /* messages are zero-terminated */
    InputChannel *ch;
} Connection;

struct InputChannel
{
    char *name;
    char *path;
    char *pipe_dir;
    gboolean owns_file;
    int fd;
    GIOChannel *io;
    guint io_watch;
    GSList *connections;
};

static char *
input_channel_get_path (InputChannel *ch)
{
    g_return_val_if_fail (ch != NULL, NULL);
    return g_strdup (ch->path);
}

static void
connection_free (Connection *conn)
{
    if (conn->io_watch)
        g_source_remove (conn->io_watch);

    if (conn->io)
    {
        g_io_channel_shutdown (conn->io, FALSE, NULL);
        g_io_channel_unref (conn->io);
    }

    if (conn->fd != -1)
        close (conn->fd);

    g_string_free (conn->buffer, TRUE);
    moo_free (Connection, conn);
}

static void
input_channel_shutdown (InputChannel *ch)
{
    g_slist_foreach (ch->connections, (GFunc) connection_free, NULL);
    g_slist_free (ch->connections);
    ch->connections = NULL;

    if (ch->io_watch)
    {
        g_source_remove (ch->io_watch);
        ch->io_watch = 0;
    }

    if (ch->io)
    {
        g_io_channel_shutdown (ch->io, FALSE, NULL);
        g_io_channel_unref (ch->io);
        ch->io = NULL;
    }

    if (ch->fd != -1)
    {
        close (ch->fd);
        ch->fd = -1;
    }

    if (ch->path)
    {
        if (ch->owns_file)
            unlink (ch->path);
        g_free (ch->path);
        ch->path = NULL;
    }
}

static gboolean
read_input (G_GNUC_UNUSED GIOChannel *source,
            G_GNUC_UNUSED GIOCondition condition,
            Connection *conn)
{
    char c;
    int n;
    gboolean do_commit = FALSE;

    errno = 0;

    while ((n = read (conn->fd, &c, 1)) > 0)
    {
        if (c == 0)
        {
            do_commit = TRUE;
            break;
        }

        g_string_append_c (conn->buffer, c);
    }

    if (n <= 0)
    {
        if (n < 0)
            moo_dmsg ("%s: %s", G_STRLOC, g_strerror (errno));
        else
            moo_dmsg ("%s: EOF", G_STRLOC);
        goto remove;
    }
    else
    {
        moo_dmsg ("%s: got bytes: '%s'", G_STRLOC, conn->buffer->str);
    }

    if (do_commit)
        commit (&conn->buffer);

    if (condition & (G_IO_ERR | G_IO_HUP))
    {
        moo_dmsg ("%s: %s", G_STRLOC,
                  (condition & G_IO_ERR) ? "G_IO_ERR" : "G_IO_HUP");
        goto remove;
    }

    return TRUE;

remove:
    conn->ch->connections = g_slist_remove (conn->ch->connections, conn);
    connection_free (conn);
    return FALSE;
}

static gboolean
accept_connection (G_GNUC_UNUSED GIOChannel *source,
                   GIOCondition  condition,
                   InputChannel *ch)
{
    Connection *conn;
    socklen_t dummy;

    if (condition & G_IO_ERR)
    {
        input_channel_shutdown (ch);
        return FALSE;
    }

    conn = moo_new0 (Connection);
    conn->ch = ch;
    conn->buffer = g_string_new_len (NULL, MAX_BUFFER_SIZE);

    conn->fd = accept (ch->fd, NULL, &dummy);

    if (conn->fd == -1)
    {
        g_warning ("%s in accept: %s", G_STRLOC, g_strerror (errno));
        moo_free (Connection, conn);
        return TRUE;
    }

    if (!input_channel_start_io (conn->fd, (GIOFunc) read_input, conn,
                                 &conn->io, &conn->io_watch))
    {
        close (conn->fd);
        moo_free (Connection, conn);
        return TRUE;
    }

    ch->connections = g_slist_prepend (ch->connections, conn);
    return TRUE;
}

static gboolean
try_connect (const char *filename,
             int        *fdp)
{
    int fd;
    struct sockaddr_un addr;

    g_return_val_if_fail (filename != NULL, FALSE);

    if (strlen (filename) + 1 > sizeof addr.sun_path)
    {
        g_critical ("%s: oops", G_STRLOC);
        return FALSE;
    }

    addr.sun_family = AF_UNIX;
    strcpy (addr.sun_path, filename);
    fd = socket (PF_UNIX, SOCK_STREAM, 0);

    if (fd == -1)
    {
        g_warning ("%s in socket for %s: %s", G_STRLOC, filename, g_strerror (errno));
        return FALSE;
    }

    errno = 0;

    if (connect (fd, (struct sockaddr *) &addr, sizeof addr) == -1)
    {
        unlink (filename);
        close (fd);
    	return FALSE;
    }

    if (fdp)
        *fdp = fd;
    else
        close (fd);

    return TRUE;
}

static gboolean
input_channel_start (InputChannel *ch,
                     gboolean      may_fail)
{
    struct sockaddr_un addr;

    mkdir (ch->pipe_dir, S_IRWXU);

    if (try_connect (ch->path, NULL))
    {
        if (!may_fail)
            g_warning ("%s: '%s' is already in use",
                       G_STRLOC, ch->path);
        return FALSE;
    }

    if (strlen (ch->path) + 1 > sizeof addr.sun_path)
    {
        g_critical ("%s: oops", G_STRLOC);
        return FALSE;
    }

    addr.sun_family = AF_UNIX;
    strcpy (addr.sun_path, ch->path);

    errno = 0;

    if ((ch->fd = socket (PF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        g_warning ("%s in socket for %s: %s", G_STRLOC, ch->path, g_strerror (errno));
        return FALSE;
    }

    if (bind (ch->fd, (struct sockaddr*) &addr, sizeof addr) == -1)
    {
        g_warning ("%s in bind for %s: %s", G_STRLOC, ch->path, g_strerror (errno));
        close (ch->fd);
        ch->fd = -1;
        return FALSE;
    }

    if (listen (ch->fd, 5) == -1)
    {
        g_warning ("%s in listen for %s: %s", G_STRLOC, ch->path, g_strerror (errno));
        close (ch->fd);
    	ch->fd = -1;
    	return FALSE;
    }

    ch->owns_file = TRUE;

    if (!input_channel_start_io (ch->fd, (GIOFunc) accept_connection, ch,
                                 &ch->io, &ch->io_watch))
    	return FALSE;

    return TRUE;
}

static InputChannel *
input_channel_new (const char *appname,
                   const char *name,
                   gboolean    may_fail)
{
    InputChannel *ch;

    g_return_val_if_fail (appname != NULL, NULL);
    g_return_val_if_fail (name != NULL, NULL);

    ch = moo_new0 (InputChannel);

    ch->name = g_strdup (name);
    ch->pipe_dir = get_pipe_dir (appname);
    ch->path = get_pipe_path (ch->pipe_dir, name);
    ch->fd = -1;
    ch->io = NULL;
    ch->io_watch = 0;

    if (!input_channel_start (ch, may_fail))
    {
        input_channel_free (ch);
        return NULL;
    }

    return ch;
}

static void
input_channel_free (InputChannel *ch)
{
    input_channel_shutdown (ch);
    g_free (ch->name);
    g_free (ch->path);

    if (ch->pipe_dir)
    {
        remove (ch->pipe_dir);
        g_free (ch->pipe_dir);
    }

    moo_free (InputChannel, ch);
}


static gboolean
do_send (const char *filename,
         const char *data,
         gssize      data_len)
{
    int fd;
    gboolean result = TRUE;

    g_return_val_if_fail (filename != NULL, FALSE);
    g_return_val_if_fail (data != NULL || data_len == 0, FALSE);

    if (!try_connect (filename, &fd))
        return FALSE;

    if (data_len < 0)
        data_len = strlen (data) + 1;

    if (data_len)
        result = do_write (fd, data, data_len);

    close (fd);
    return result;
}

#endif /* MOO_APP_INPUT_SOCKET */


#ifdef MOO_APP_INPUT_PIPE

struct InputChannel
{
    char *name;
    char *path;
    char *pipe_dir;
    gboolean owns_file;
    int fd;
    GString *buffer;
    GIOChannel *io;
    guint io_watch;
    GSList *connections;
};

static char *
input_channel_get_path (InputChannel *ch)
{
    g_return_val_if_fail (ch != NULL, NULL);
    return g_strdup (ch->path);
}

static void
input_channel_shutdown (InputChannel *ch)
{
    if (ch->io_watch)
    {
        g_source_remove (ch->io_watch);
        ch->io_watch = 0;
    }

    if (ch->io)
    {
        g_io_channel_shutdown (ch->io, FALSE, NULL);
        g_io_channel_unref (ch->io);
        ch->io = NULL;
    }

    if (ch->fd != -1)
    {
        close (ch->fd);
        ch->fd = -1;
    }

    if (ch->path)
    {
        unlink (ch->path);
        g_free (ch->path);
        ch->path = NULL;
    }

    if (ch->buffer)
    {
        g_string_free (ch->buffer, TRUE);
        ch->buffer = NULL;
    }
}

static gboolean
read_input (GIOChannel   *source,
            GIOCondition  condition,
            InputChannel *ch)
{
    gboolean error_occured = FALSE;
    GError *err = NULL;
    gboolean again = TRUE;
    gboolean got_zero = FALSE;

    g_return_val_if_fail (source == ch->io, FALSE);

    /* XXX */
    if (condition & (G_IO_ERR | G_IO_HUP))
        error_occured = TRUE;

    while (again && !error_occured && !err)
    {
        char c;
        int bytes_read;

        struct pollfd fd;

        fd.fd = ch->fd;
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
                    bytes_read = read (ch->fd, &c, 1);

                    if (bytes_read == 1)
                    {
                        g_string_append_c (ch->buffer, c);

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

        input_channel_shutdown (ch);
        return FALSE;
    }

    if (got_zero)
        commit (&ch->buffer);

    return TRUE;
}

static gboolean
input_channel_start (InputChannel *ch,
                     G_GNUC_UNUSED gboolean may_fail)
{
    mkdir (ch->pipe_dir, S_IRWXU);
    unlink (ch->path);

    if (mkfifo (ch->path, S_IRUSR | S_IWUSR) != 0)
    {
        int err = errno;
        g_critical ("%s: error in mkfifo()", G_STRLOC);
        g_critical ("%s: %s", G_STRLOC, g_strerror (err));
        return FALSE;
    }

    /* XXX O_RDWR is not good (man 3p open), but it must be opened for
     * writing by us, otherwise we get POLLHUP when a writer dies on the other end.
     * So, open for writing separately. */
    ch->fd = open (ch->path, O_RDWR | O_NONBLOCK);
    if (ch->fd == -1)
    {
        int err = errno;
        g_critical ("%s: error in open()", G_STRLOC);
        g_critical ("%s: %s", G_STRLOC, g_strerror (err));
        return FALSE;
    }

    moo_dmsg ("%s: opened input pipe %s with fd %d",
              G_STRLOC, ch->path, ch->fd);

    if (!input_channel_start_io (ch->fd, (GIOFunc) read_input, ch,
                                 &ch->io, &ch->io_watch))
        return FALSE;

    return TRUE;
}

static InputChannel *
input_channel_new  (const char *appname,
                    const char *name,
                    gboolean    may_fail)
{
    InputChannel *ch;

    g_return_val_if_fail (appname != NULL, NULL);
    g_return_val_if_fail (name != NULL, NULL);

    ch = moo_new0 (InputChannel);

    ch->name = g_strdup (name);
    ch->pipe_dir = get_pipe_dir (appname);
    ch->path = get_pipe_path (ch->pipe_dir, name);
    ch->fd = -1;
    ch->io = NULL;
    ch->io_watch = 0;
    ch->buffer = g_string_new_len (NULL, MAX_BUFFER_SIZE);

    if (!input_channel_start (ch, may_fail))
    {
        input_channel_free (ch);
        return NULL;
    }

    return ch;
}

static void
input_channel_free (InputChannel *ch)
{
    input_channel_shutdown (ch);

    g_free (ch->name);
    g_free (ch->path);

    if (ch->pipe_dir)
    {
        remove (ch->pipe_dir);
        g_free (ch->pipe_dir);
    }

    moo_free (InputChannel, ch);
}

static gboolean
do_send (const char *filename,
         const char *data,
         gssize      data_len)
{
    gboolean result = FALSE;
    int fd;

    moo_dmsg ("do_send: sending data to `%s'", filename);

    if (!g_file_test (filename, G_FILE_TEST_EXISTS))
    {
        moo_dmsg ("do_send: file `%s' doesn't exist", filename);
        return FALSE;
    }

    fd = open (filename, O_WRONLY | O_NONBLOCK);

    if (fd == -1)
    {
        moo_dmsg ("do_send: could not open `%s': %s", filename, g_strerror (errno));
        return FALSE;
    }

    result = do_write (fd, data, data_len);
    close (fd);

    if (result)
        moo_dmsg ("do_send: successfully sent stuff to `%s'", filename);

    return result;
}

#endif /* MOO_APP_INPUT_PIPE */


/****************************************************************************/
/* WIN32
 */
#ifdef MOO_APP_INPUT_WIN32

typedef struct {
    char *pipe_name;
    guint event_id;
} ListenerInfo;

struct InputChannel
{
    char *appname;
    char *name;
    char *pipe_name;
    GString *buffer;
    guint event_id;
};

static char *
input_channel_get_path (InputChannel *ch)
{
    g_return_val_if_fail (ch != NULL, NULL);
    return g_strdup (ch->pipe_name);
}

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

	/* XXX unicode */
    input = CreateNamedPipeA (info->pipe_name,
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
get_pipe_name (const char *appname,
               const char *name)
{
    if (!name)
        name = _moo_get_pid_string ();
    return g_strdup_printf ("\\\\.\\pipe\\%s_in_%s",
                            appname, name);
}


static void
event_callback (GList        *events,
                InputChannel *ch)
{
    while (events)
    {
        char c = GPOINTER_TO_INT (events->data);

        if (c != 0)
            g_string_append_c (ch->buffer, c);
        else
        {
            gdk_threads_enter ();
            commit (&ch->buffer);
            gdk_threads_leave ();
        }

        events = events->next;
    }
}


static gboolean
input_channel_start (InputChannel *ch)
{
    ListenerInfo *info;

    g_free (ch->pipe_name);
    ch->pipe_name = get_pipe_name (ch->appname, ch->name);
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

    return TRUE;
}


static InputChannel *
input_channel_new (const char *appname,
                   const char *name,
                   G_GNUC_UNUSED gboolean may_fail)
{
    InputChannel *ch;

    ch = moo_new0 (InputChannel);
    ch->appname = g_strdup (appname);
    ch->name = g_strdup (name);
    ch->buffer = g_string_new_len (NULL, MAX_BUFFER_SIZE);

    if (!input_channel_start (ch))
    {
        input_channel_free (ch);
        return NULL;
    }

    return ch;
}

static void
input_channel_free (InputChannel *ch)
{
    if (ch->event_id)
        _moo_event_queue_disconnect (ch->event_id);
    if (ch->buffer)
        g_string_free (ch->buffer, TRUE);
    g_free (ch->pipe_name);
    g_free (ch->appname);
    g_free (ch->name);
    moo_free (InputChannel, ch);
}

gboolean
_moo_app_input_send_msg (const char *appname,
                         const char *name,
                         const char *data,
                         gssize      len)
{
    char *err_msg = NULL;
    char *pipe_name;
    HANDLE pipe_handle;
    gboolean result = FALSE;
    DWORD bytes_written;

    g_return_val_if_fail (appname != NULL, FALSE);
    g_return_val_if_fail (data != NULL, FALSE);

    if (len < 0)
        len = strlen (data);

    if (!len)
        return TRUE;

    if (!name)
        name = "main";

    pipe_name = get_pipe_name (appname, name);
	/* XXX unicode */
    pipe_handle = CreateFileA (pipe_name, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

    if (!pipe_handle)
    {
        err_msg = g_win32_error_message (GetLastError ());
        g_warning ("could not open pipe '%s': %s", pipe_name, err_msg);
        goto out;
    }

    if (!WriteFile (pipe_handle, data, len, &bytes_written, NULL))
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

#endif /* MOO_APP_INPUT_WIN32 */
