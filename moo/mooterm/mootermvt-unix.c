/*
 *   mooterm/mootermvt-unix.c
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

#define MOOTERM_COMPILATION
#include "mooterm/mooterm-private.h"
#include "mooterm/pty.h"
#include "mooutils/moomarshals.h"
#include "mooutils/moocompat.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#ifdef HAVE_POLL_H
#include <poll.h>
#elif HAVE_SYS_POLL_H
#include <sys/poll.h>
#endif


#define TERM_EMULATION          "xterm"
#define READ_BUFSIZE            4096
#define POLL_TIME               5
#define POLL_NUM                1


#define MOO_TERM_VT_UNIX(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOO_TYPE_TERM_VT_UNIX, MooTermVtUnix))
#define MOO_TERM_VT_UNIX_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  MOO_TYPE_TERM_VT_UNIX, MooTermVtUnixClass))
#define MOO_IS_TERM_VT_UNIX(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOO_TYPE_TERM_VT_UNIX))
#define MOO_IS_TERM_VT_UNIX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  MOO_TYPE_TERM_VT_UNIX))
#define MOO_TERM_VT_UNIX_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  MOO_TYPE_TERM_VT_UNIX, MooTermVtUnixClass))

typedef struct _MooTermVtUnix           MooTermVtUnix;
typedef struct _MooTermVtUnixClass      MooTermVtUnixClass;


struct _MooTermVtUnix {
    MooTermVt   parent;

    gboolean    child_alive;
    GPid        child_pid;

    int         master;
    int         width;
    int         height;

    gboolean    non_block;
    GIOChannel *io;
    guint       io_watch_id;
};


struct _MooTermVtUnixClass {
    MooTermVtClass  parent_class;
};


static void     moo_term_vt_unix_finalize       (GObject        *object);

static void     set_size        (MooTermVt      *vt,
                                 gulong          width,
                                 gulong          height);
static gboolean fork_command    (MooTermVt      *vt,
                                 const char     *cmd,
                                 const char     *working_dir,
                                 char          **envp);
static void     vt_write        (MooTermVt      *vt,
                                 const char     *string,
                                 gssize          len);
static void     kill_child      (MooTermVt      *vt);

static gboolean read_child_out  (GIOChannel     *source,
                                 GIOCondition    condition,
                                 MooTermVtUnix  *vt);
static void     feed_buffer     (MooTermVtUnix  *vt,
                                 const char     *string,
                                 gssize          len);

static void     start_writer    (MooTermVt      *vt);
static void     stop_writer     (MooTermVt      *vt);


/* MOO_TYPE_TERM_VT_UNIX */
G_DEFINE_TYPE (MooTermVtUnix, moo_term_vt_unix, MOO_TYPE_TERM_VT)


static void moo_term_vt_unix_class_init (MooTermVtUnixClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    MooTermVtClass *vt_class = MOO_TERM_VT_CLASS (klass);

    gobject_class->finalize = moo_term_vt_unix_finalize;

    vt_class->set_size = set_size;
    vt_class->fork_command = fork_command;
    vt_class->write = vt_write;
    vt_class->kill_child = kill_child;
}


static void     moo_term_vt_unix_init            (MooTermVtUnix      *vt)
{
    vt->child_alive = FALSE;
    vt->child_pid = (GPid)-1;

    vt->master = -1;
    vt->width = 80;
    vt->height = 24;

    vt->non_block = FALSE;
    vt->io = NULL;
    vt->io_watch_id = 0;
}


static void     moo_term_vt_unix_finalize        (GObject            *object)
{
    MooTermVtUnix *vt = MOO_TERM_VT_UNIX (object);
    kill_child (MOO_TERM_VT (vt));
    G_OBJECT_CLASS (moo_term_vt_unix_parent_class)->finalize (object);
}


static void     set_size        (MooTermVt      *vt,
                                 gulong          width,
                                 gulong          height)
{
    MooTermVtUnix *vtu;

    g_return_if_fail (MOO_IS_TERM_VT_UNIX (vt));

    vtu = MOO_TERM_VT_UNIX (vt);

    if (vtu->child_alive)
        _vte_pty_set_size (vtu->master, width, height);

    vtu->width = width;
    vtu->height = height;
}


static gboolean fork_command    (MooTermVt      *vt_gen,
                                 const char     *cmd,
                                 const char     *working_dir,
                                 char          **envp)
{
    MooTermVtUnix *vt;
    int argv_len;
    char **argv = NULL;
    int env_len = 0;
    char **new_env;
    GError *err = NULL;
    int status, flags;
    int i;
    GSource *src;

    g_return_val_if_fail (cmd != NULL, FALSE);
    g_return_val_if_fail (MOO_IS_TERM_VT_UNIX (vt_gen), FALSE);

    vt = MOO_TERM_VT_UNIX (vt_gen);

    g_return_val_if_fail (!vt->child_alive, FALSE);

    if (!g_shell_parse_argv (cmd, &argv_len, &argv, &err))
    {
        g_critical ("%s: could not parse command line", G_STRLOC);

        if (err != NULL)
        {
            g_critical ("%s: %s", G_STRLOC, err->message);
            g_error_free (err);
        }

        if (argv != NULL)
            g_strfreev (argv);

        return FALSE;
    }

    if (envp)
    {
        char **e;
        for (e = envp; *e != NULL; ++e)
            ++env_len;
    }

    new_env = g_new (char*, env_len + 2);

    for (i = 0; i < env_len; ++i)
        new_env[i] = g_strdup (envp[i]);

    new_env[env_len] = g_strdup ("TERM=" TERM_EMULATION);
    new_env[env_len + 1] = NULL;

    vt->master = _vte_pty_open (&vt->child_pid, new_env,
                                 argv[0],
                                 argv,
                                 working_dir,
                                 vt->width, vt->height,
                                 FALSE, FALSE, FALSE);
    g_strfreev (new_env);

    if (vt->master == -1)
    {
        g_critical ("%s: could not fork child", G_STRLOC);
        return FALSE;
    }
    else
    {
        g_message ("%s: forked child pid %d", G_STRLOC, vt->child_pid);
    }

    if (waitpid (-1, &status, WNOHANG) == -1)
        g_critical ("%s: error in waitpid", G_STRLOC);

    if ((flags = fcntl (vt->master, F_GETFL)) < 0)
        g_critical ("%s: F_GETFL on master", G_STRLOC);
    else if (-1 == fcntl (vt->master, F_SETFL, O_NONBLOCK | flags))
        g_critical ("%s: F_SETFL on master", G_STRLOC);
    else
        vt->non_block = TRUE;

    vt->io =  g_io_channel_unix_new (vt->master);
    g_return_val_if_fail (vt->io != NULL, FALSE);

    g_io_channel_set_encoding (vt->io, NULL, NULL);
    g_io_channel_set_buffered (vt->io, FALSE);

    vt->io_watch_id = g_io_add_watch (vt->io,
                                      G_IO_IN | G_IO_PRI | G_IO_HUP,
                                      (GIOFunc) read_child_out,
                                      vt);

    src = g_main_context_find_source_by_id (NULL, vt->io_watch_id);

    if (src)
        g_source_set_priority (src, VT_READER_PRIORITY);
    else
        g_warning ("%s: could not find io_watch_id source", G_STRLOC);

    vt->child_alive = TRUE;

    return TRUE;
}


static void     kill_child      (MooTermVt      *vt_gen)
{
    MooTermVtUnix *vt = MOO_TERM_VT_UNIX (vt_gen);

    stop_writer (vt_gen);
    vt_flush_pending_write (vt_gen);

    if (vt->io_watch_id)
    {
        g_source_remove (vt->io_watch_id);
        vt->io_watch_id = 0;
    }

    if (vt->io)
    {
        vt_write (MOO_TERM_VT (vt), "\4", 1);
        g_io_channel_shutdown (vt->io, TRUE, NULL);
        g_io_channel_unref (vt->io);
        vt->io = NULL;
    }

    if (vt->master != -1)
    {
        _vte_pty_close (vt->master);
        vt->master = -1;
    }

    vt->non_block = FALSE;

    vt->child_pid = (GPid)-1;

    if (vt->child_alive)
    {
        vt->child_alive = FALSE;
        g_signal_emit_by_name (vt, "child-died");
    }
}


static gboolean read_child_out  (G_GNUC_UNUSED GIOChannel     *source,
                                 GIOCondition    condition,
                                 MooTermVtUnix  *vt)
{
    gboolean error_occured = FALSE;
    int error_no = 0;

    char buf[READ_BUFSIZE];
    int current = 0;
    guint again = POLL_NUM;

    if (condition & G_IO_HUP)
    {
        g_message ("%s: G_IO_HUP", G_STRLOC);
        error_no = errno;
        goto error;
    }
    else if (condition & G_IO_ERR)
    {
        g_message ("%s: G_IO_ERR", G_STRLOC);
        error_no = errno;
        goto error;
    }

    g_assert (condition & (G_IO_IN | G_IO_PRI));

    if (vt->non_block)
    {
        int r = read (vt->master, buf, READ_BUFSIZE);

        switch (r)
        {
            case -1:
                error_occured = TRUE;
                error_no = errno;
                goto error;

            case 0:
                break;

            default:
                {
                    int i;
                    g_print ("got >>>");
                    for (i = 0; i < r; ++i)
                    {
                        if (32 <= buf[i] && buf[i] <= 126)
                            g_print ("%c", buf[i]);
                        else
                            g_print ("<%d>", buf[i]);
                    }
                    g_print ("<<<\n");
                }
                feed_buffer (vt, buf, r);
                break;
        }
    
        return TRUE;
    }

    while (again && !error_occured && current < READ_BUFSIZE)
    {
        struct pollfd fd = {vt->master, POLLIN | POLLPRI, 0};

        int res = poll (&fd, 1, POLL_TIME);

        switch (res)
        {
            case 0:
                --again;
                break;

            case 1:
                if (fd.revents & (POLLNVAL | POLLERR | POLLHUP))
                {
                    again = 0;
                    if (errno != EAGAIN && errno != EINTR)
                    {
                        error_occured = TRUE;
                        error_no = errno;
                    }
                }
                else if (fd.revents & (POLLIN | POLLPRI))
                {
                    int r = read (vt->master, buf + current, 1);

                    switch (r)
                    {
                        case -1:
                            error_occured = TRUE;
                            error_no = errno;
                            break;

                        case 0:
                            --again;
                            break;

                        case 1:
                            ++current;
                            break;

                        default:
                            g_assert_not_reached();
                    }
                }
    
                break;

            case -1:
                again = 0;

                if (errno != EAGAIN && errno != EINTR)
                {
                    error_occured = TRUE;
                    error_no = errno;
                }

                break;

            default:
                g_assert_not_reached();
        }
    }

    if (current > 0)
        feed_buffer (vt, buf, current);

    if (error_occured)
        goto error;

    return TRUE;

error:
    if (error_occured)
        g_message ("error in %s", G_STRLOC);

    if (error_no)
        g_message ("%s: %s", G_STRLOC, g_strerror (error_no));

    if (vt->io)
    {
        _vte_pty_close (vt->master);
        vt->master = -1;

        g_io_channel_shutdown (vt->io, TRUE, NULL);
        g_io_channel_unref (vt->io);
        vt->io = NULL;

        vt->io_watch_id = 0;
    }

    kill_child (MOO_TERM_VT (vt));

    return FALSE;
}


static void     feed_buffer     (MooTermVtUnix  *vt,
                                 const char     *string,
                                 gssize          len)
{
    moo_term_buffer_write (moo_term_vt_get_buffer (MOO_TERM_VT (vt)),
                           string, len);
}


/* writes given data to file, returns TRUE on successful write,
   FALSE when could not write al teh data, puts start of leftover
   to string, length of it to len, and fills err in case of error */
static gboolean do_write        (MooTermVt      *vt_gen,
                                 const char    **string,
                                 gsize          *plen,
                                 GError        **err)
{
    GIOStatus status;
    gsize written = 0;

    MooTermVtUnix *vt = MOO_TERM_VT_UNIX (vt_gen);

    g_return_val_if_fail (vt->io != NULL, FALSE);

    *err = NULL;

    status = g_io_channel_write_chars (vt->io, *string, *plen,
                                       &written, err);

    if (written == *plen)
    {
        *string = NULL;
        *plen = 0;
    }
    else
    {
        *string += written;
        *plen -= written;
    }

    switch (status)
    {
        case G_IO_STATUS_NORMAL:
        case G_IO_STATUS_AGAIN:
            return TRUE;

        default:
            return FALSE;
    }
}


static void     append                  (MooTermVt      *vt,
                                         const char     *data,
                                         guint           len)
{
    GByteArray *ar = g_byte_array_sized_new (len);
    g_byte_array_append (ar, (const guint8*)data, len);
    g_queue_push_tail (vt->priv->pending_write, ar);
}


static gboolean write_cb        (MooTermVt      *vt)
{
    vt_write (vt, NULL, 0);
    return TRUE;
}

static void     start_writer    (MooTermVt      *vt)
{
    if (!vt->priv->pending_write_id)
        vt->priv->pending_write_id =
                g_idle_add_full (VT_WRITER_PRIORITY,
                                 (GSourceFunc) write_cb,
                                 vt, NULL);
}

static void     stop_writer     (MooTermVt      *vt)
{
    if (vt->priv->pending_write_id)
    {
        g_source_remove (vt->priv->pending_write_id);
        vt->priv->pending_write_id = 0;
    }
}


static void     vt_write        (MooTermVt      *vt,
                                 const char     *data,
                                 gssize          data_len)
{
    g_return_if_fail (data == NULL || data_len != 0);

    while (data || !g_queue_is_empty (vt->priv->pending_write))
    {
        GError *err = NULL;
        const char *string;
        gsize len;
        GByteArray *freeme = NULL;

        if (!g_queue_is_empty (vt->priv->pending_write))
        {
            if (data)
            {
                append (vt, data, data_len > 0 ? (guint)data_len : strlen (data));
                data = NULL;
            }

            freeme = g_queue_peek_head (vt->priv->pending_write);
            string = freeme->data;
            len = freeme->len;
        }
        else
        {
            string = data;
            len = data_len > 0 ? (gsize)data_len : strlen (data);
            data = NULL;
        }

        if (do_write (vt, &string, &len, &err))
        {
            if (len)
            {
                if (freeme)
                {
                    memmove (freeme->data, freeme->data + (freeme->len - len), len);
                    g_byte_array_set_size (freeme, len);
                }
                else
                {
                    append (vt, string, len);
                }

                break;
            }
            else if (freeme)
            {
                g_byte_array_free (freeme, TRUE);
                g_queue_pop_head (vt->priv->pending_write);
            }
        }
        else
        {
            g_message ("%s: stopping writing to child", G_STRLOC);
            if (err)
            {
                g_message ("%s: %s", G_STRLOC, err->message);
                g_error_free (err);
            }
            stop_writer (vt);
            kill_child (vt);
        }
    }

    if (!g_queue_is_empty (vt->priv->pending_write))
        start_writer (vt);
    else
        stop_writer (vt);
}
