/*
 *   mooapp/mooappinput.c
 *
 *   Copyright (C) 2004-2010 by Yevgen Muntyan <emuntyan@users.sourceforge.net>
 *
 *   This file is part of medit.  medit is free software; you can
 *   redistribute it and/or modify it under the terms of the
 *   GNU Lesser General Public License as published by the
 *   Free Software Foundation; either version 2.1 of the License,
 *   or (at your option) any later version.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with medit.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#ifdef __WIN32__
#define MOO_APP_INPUT_WIN32
#else
#define MOO_APP_INPUT_SOCKET
#endif

#if defined(MOO_APP_INPUT_WIN32)
# include <windows.h>
# include <io.h>
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
#include "mooappinput.h"
#include "mooapp-ipc.h"
#include "mooutils-misc.h"
#include "mooutils-thread.h"
#include "mooutils-debug.h"

MOO_DEBUG_INIT(input, FALSE)

#define MAX_BUFFER_SIZE 4096
#define IPC_MAGIC_CHAR 'I'
#define MOO_APP_INPUT_NAME_DEFAULT "main"

#ifdef MOO_APP_INPUT_SOCKET
#define INPUT_PREFIX "in-"
#else
#define INPUT_PREFIX "input-"
#endif

typedef struct MooAppInput MooAppInput;
typedef struct InputChannel InputChannel;

struct MooAppInput
{
    GSList *pipes;
    char *appname;
    char *main_path;
    MooAppInputCallback callback;
    gpointer callback_data;
};

static MooAppInput *inp_instance;

static InputChannel *input_channel_new      (const char     *appname,
                                             const char     *name,
                                             gboolean        may_fail);
static void          input_channel_free     (InputChannel   *ch);
static char         *input_channel_get_path (InputChannel   *ch);
G_GNUC_UNUSED
static const char   *input_channel_get_name (InputChannel   *ch);


static void
exec_callback (char        cmd,
               const char *data,
               gsize       len)
{
    g_return_if_fail (inp_instance && inp_instance->callback);
    if (cmd == IPC_MAGIC_CHAR)
        _moo_ipc_dispatch (data, len);
    else
        inp_instance->callback (cmd, data, len, inp_instance->callback_data);
}


static MooAppInput *
moo_app_input_new (const char *name,
                   gboolean    bind_default,
                   MooAppInputCallback callback,
                   gpointer    callback_data)
{
    MooAppInput *ch;
    InputChannel *ich;

    g_return_val_if_fail (callback != NULL, NULL);

    ch = g_slice_new0 (MooAppInput);

    ch->callback = callback;
    ch->callback_data = callback_data;
    ch->pipes = NULL;
    ch->appname = g_strdup (MOO_PACKAGE_NAME);

    if ((ich = input_channel_new (ch->appname, _moo_get_pid_string (), FALSE)))
    {
        ch->pipes = g_slist_prepend (ch->pipes, ich);
        ch->main_path = input_channel_get_path (ich);
    }

    if (name && (ich = input_channel_new (ch->appname, name, FALSE)))
        ch->pipes = g_slist_prepend (ch->pipes, ich);

    if (bind_default && (ich = input_channel_new (ch->appname, MOO_APP_INPUT_NAME_DEFAULT, TRUE)))
        ch->pipes = g_slist_prepend (ch->pipes, ich);

    return ch;
}

void
_moo_app_input_start (const char     *name,
                      gboolean        bind_default,
                      MooAppInputCallback callback,
                      gpointer        callback_data)
{
    g_return_if_fail (inp_instance == NULL);
    inp_instance = moo_app_input_new (name, bind_default,
                                      callback, callback_data);
}

static void
moo_app_input_free (MooAppInput *ch)
{
    g_return_if_fail (ch != NULL);

    g_slist_foreach (ch->pipes, (GFunc) input_channel_free, NULL);
    g_slist_free (ch->pipes);

    g_free (ch->main_path);
    g_free (ch->appname);
    g_slice_free (MooAppInput, ch);
}

void
_moo_app_input_shutdown (void)
{
    if (inp_instance)
    {
        MooAppInput *tmp = inp_instance;
        inp_instance = NULL;
        moo_app_input_free (tmp);
    }
}


const char *
_moo_app_input_get_path (void)
{
    return inp_instance ? inp_instance->main_path : NULL;
}

gboolean
_moo_app_input_running (void)
{
    return inp_instance != NULL;
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
        moo_dmsg ("got empty command");
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

    exec_callback (ptr[0], ptr + 1, len - 1);

    if (freeme)
        g_string_free (freeme, TRUE);
}


#ifndef MOO_APP_INPUT_WIN32

static const char *
get_display_name (void)
{
    static char *name;

#ifdef GDK_WINDOWING_X11
    static gboolean been_here;
    if (!been_here)
    {
        GdkDisplay *display;
        const char *display_name;

        been_here = TRUE;

        if ((display = gdk_display_get_default ()))
        {
            display_name = gdk_display_get_name (display);
        }
        else
        {
            display_name = gdk_get_display_arg_name ();
            if (!display_name || !display_name[0])
                display_name = g_getenv ("DISPLAY");
        }

        if (display_name && display_name[0])
        {
            char *colon, *dot;

            if ((colon = strchr (display_name, ':')) &&
                (dot = strrchr (display_name, '.')) &&
                dot > colon)
                    name = g_strndup (display_name, dot - display_name);
            else
                name = g_strdup (display_name);

            if (name[0] == ':')
            {
                if (name[1])
                {
                    char *tmp = g_strdup (name + 1);
                    g_free (name);
                    name = tmp;
                }
                else
                {
                    g_free (name);
                    name = NULL;
                }
            }

            if (name)
                g_strcanon (name,
                            G_CSET_A_2_Z G_CSET_a_2_z G_CSET_DIGITS,
                            '-');
        }
    }
#endif

    return name;
}

static const char *
get_user_name (void)
{
    static char *user_name;

    if (!user_name)
        user_name = g_strcanon (g_strdup (g_get_user_name ()),
                                G_CSET_A_2_Z G_CSET_a_2_z G_CSET_DIGITS,
                                '-');

    return user_name;
}

static char *
get_pipe_dir (const char *appname)
{
    const char *display_name;
    const char *user_name;
    char *name;

    g_return_val_if_fail (appname != NULL, NULL);

    display_name = get_display_name ();
    user_name = get_user_name ();

    if (display_name)
        name = g_strdup_printf ("%s/%s-%s-%s", g_get_tmp_dir (), appname, user_name, display_name);
    else
        name = g_strdup_printf ("%s/%s-%s", g_get_tmp_dir (), appname, user_name);

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
                         const char *iheader,
                         const char *data,
                         gssize      data_len);

static gboolean
try_send (const char *pipe_dir_name,
          const char *name,
          const char *iheader,
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

    result = do_send (filename, iheader, data, data_len);

out:
    g_free (filename);
    return result;
}

gboolean
_moo_app_input_send_msg (const char *name,
                         const char *data,
                         gssize      len)
{
    const char *entry;
    GDir *pipe_dir = NULL;
    char *pipe_dir_name;
    gboolean success = FALSE;

    g_return_val_if_fail (data != NULL, FALSE);

    moo_dmsg ("_moo_app_input_send_msg: sending data to %s", name ? name : "NONE");

    pipe_dir_name = get_pipe_dir (MOO_PACKAGE_NAME);
    g_return_val_if_fail (pipe_dir_name != NULL, FALSE);

    if (name)
    {
        success = try_send (pipe_dir_name, name, NULL, data, len);
        goto out;
    }

    success = try_send (pipe_dir_name, MOO_APP_INPUT_NAME_DEFAULT, NULL, data, len);
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

            if (try_send (pipe_dir_name, name, NULL, data, len))
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

void
_moo_app_input_broadcast (const char *header,
                          const char *data,
                          gssize      len)
{
    const char *entry;
    GDir *pipe_dir = NULL;
    char *pipe_dir_name;

    g_return_if_fail (data != NULL);

    moo_dmsg ("_moo_app_input_broadcast");

    if (!inp_instance)
        return;

    pipe_dir_name = get_pipe_dir (inp_instance->appname);
    g_return_if_fail (pipe_dir_name != NULL);

    pipe_dir = g_dir_open (pipe_dir_name, 0, NULL);

    while (pipe_dir && (entry = g_dir_read_name (pipe_dir)))
    {
        if (!strncmp (entry, INPUT_PREFIX, strlen (INPUT_PREFIX)))
        {
            GSList *l;
            gboolean my_name = FALSE;
            const char *name = entry + strlen (INPUT_PREFIX);

            for (l = inp_instance->pipes; !my_name && l != NULL; l = l->next)
            {
                InputChannel *ch = l->data;
                const char *ch_name = input_channel_get_name (ch);
                if (ch_name && strcmp (ch_name, name) == 0)
                    my_name = TRUE;
            }

            if (!my_name)
                try_send (pipe_dir_name, name, header, data, len);
        }
    }

    if (pipe_dir)
        g_dir_close (pipe_dir);
    g_free (pipe_dir_name);
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
                g_warning ("in write: %s", g_strerror (errno));
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

static const char *
input_channel_get_name (InputChannel *ch)
{
    g_return_val_if_fail (ch != NULL, NULL);
    return ch->name;
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
    g_slice_free (Connection, conn);
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
            moo_dmsg ("%s", g_strerror (errno));
        else
            moo_dmsg ("EOF");
        goto remove;
    }
    else
    {
        moo_dmsg ("got bytes: '%s'", conn->buffer->str);
    }

    if (do_commit)
        commit (&conn->buffer);

    if (!do_commit && (condition & (G_IO_ERR | G_IO_HUP)))
    {
        moo_dmsg ("%s", (condition & G_IO_ERR) ? "G_IO_ERR" : "G_IO_HUP");
        goto remove;
    }

    return TRUE;

remove:
    if (conn->ch)
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

    conn = g_slice_new0 (Connection);
    conn->ch = ch;
    conn->buffer = g_string_new_len (NULL, MAX_BUFFER_SIZE);

    conn->fd = accept (ch->fd, NULL, &dummy);

    if (conn->fd == -1)
    {
        g_warning ("in accept: %s", g_strerror (errno));
        g_slice_free (Connection, conn);
        return TRUE;
    }

    if (!input_channel_start_io (conn->fd, (GIOFunc) read_input, conn,
                                 &conn->io, &conn->io_watch))
    {
        close (conn->fd);
        g_slice_free (Connection, conn);
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
        g_critical ("oops");
        return FALSE;
    }

    addr.sun_family = AF_UNIX;
    strcpy (addr.sun_path, filename);
    fd = socket (PF_UNIX, SOCK_STREAM, 0);

    if (fd == -1)
    {
        g_warning ("in socket for %s: %s", filename, g_strerror (errno));
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
            g_warning ("'%s' is already in use", ch->path);
        return FALSE;
    }

    if (strlen (ch->path) + 1 > sizeof addr.sun_path)
    {
        g_critical ("oops");
        return FALSE;
    }

    addr.sun_family = AF_UNIX;
    strcpy (addr.sun_path, ch->path);

    errno = 0;

    if ((ch->fd = socket (PF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        g_warning ("in socket for %s: %s", ch->path, g_strerror (errno));
        return FALSE;
    }

    if (bind (ch->fd, (struct sockaddr*) &addr, sizeof addr) == -1)
    {
        g_warning ("in bind for %s: %s", ch->path, g_strerror (errno));
        close (ch->fd);
        ch->fd = -1;
        return FALSE;
    }

    if (listen (ch->fd, 5) == -1)
    {
        g_warning ("in listen for %s: %s", ch->path, g_strerror (errno));
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

    ch = g_slice_new0 (InputChannel);

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

    g_slice_free (InputChannel, ch);
}


static gboolean
do_send (const char *filename,
         const char *iheader,
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
        data_len = strlen (data);

    if (iheader)
    {
        char c = IPC_MAGIC_CHAR;
        result = do_write (fd, &c, 1) &&
                 do_write (fd, iheader, strlen (iheader));
    }

    if (result && data_len)
        result = do_write (fd, data, data_len);

    if (result)
    {
        char c = 0;
        result = do_write (fd, &c, 1);
    }

    close (fd);
    return result;
}

#endif /* MOO_APP_INPUT_SOCKET */


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

G_GNUC_UNUSED static const char *
input_channel_get_name (InputChannel *ch)
{
    g_return_val_if_fail (ch != NULL, NULL);
    return ch->name;
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
                _moo_message_async ("%s: error in ConnectNamedPipe(): %s", G_STRLOC, msg);
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
            commit (&ch->buffer);

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

    ch = g_slice_new0 (InputChannel);
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
    g_slice_free (InputChannel, ch);
}

static gboolean
write_data (HANDLE      file,
            const char *data,
            gsize       len,
            const char *pipe_name)
{
    DWORD bytes_written;

    if (!WriteFile (file, data, (DWORD) len, &bytes_written, NULL))
    {
        char *err_msg = g_win32_error_message (GetLastError ());
        g_warning ("could not write data to '%s': %s", pipe_name, err_msg);
        g_free (err_msg);
        return FALSE;
    }

    if (bytes_written < (DWORD) len)
    {
        g_warning ("written less data than requested to '%s'", pipe_name);
        return FALSE;
    }

    return TRUE;
}

gboolean
_moo_app_input_send_msg (const char *name,
                         const char *data,
                         gssize      len)
{
    char *err_msg = NULL;
    char *pipe_name;
    HANDLE pipe_handle;
    gboolean result = FALSE;

    g_return_val_if_fail (data != NULL, FALSE);

    if (len < 0)
        len = strlen (data);

    if (!len)
        return TRUE;

    if (!name)
        name = "main";

    pipe_name = get_pipe_name (MOO_PACKAGE_NAME, name);
	/* XXX unicode */
    pipe_handle = CreateFileA (pipe_name, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

    if (!pipe_handle)
    {
        err_msg = g_win32_error_message (GetLastError ());
        g_warning ("could not open pipe '%s': %s", pipe_name, err_msg);
        goto out;
    }

    result = write_data (pipe_handle, data, len, pipe_name);

    if (result)
    {
        char c = 0;
        result = write_data (pipe_handle, &c, 1, pipe_name);
    }

out:
    if (pipe_handle != INVALID_HANDLE_VALUE)
        CloseHandle (pipe_handle);

    g_free (pipe_name);
    g_free (err_msg);
    return result;
}

void
_moo_app_input_broadcast (const char *header,
                          const char *data,
                          gssize      len)
{
    MOO_IMPLEMENT_ME

    g_return_if_fail (header != NULL);
    g_return_if_fail (data != NULL);

    if (len < 0)
        len = strlen (data);

    g_return_if_fail (len != 0);
}

#endif /* MOO_APP_INPUT_WIN32 */
