/*
 *   mooterm/mootermpt-unix.c
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
#include "mooterm/mootermpt-private.h"
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
#include <termios.h>

#ifdef HAVE_POLL_H
#  include <poll.h>
#elif HAVE_SYS_POLL_H
#  include <sys/poll.h>
#endif

#define TERM_EMULATION          "xterm"
#define READ_CHUNK_SIZE         1024
#define WRITE_CHUNK_SIZE        4096
#define POLL_TIME               5
#define POLL_NUM                1


#define MOO_TERM_PT_UNIX(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOO_TYPE_TERM_PT_UNIX, MooTermPtUnix))
#define MOO_TERM_PT_UNIX_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  MOO_TYPE_TERM_PT_UNIX, MooTermPtUnixClass))
#define MOO_IS_TERM_PT_UNIX(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOO_TYPE_TERM_PT_UNIX))
#define MOO_IS_TERM_PT_UNIX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  MOO_TYPE_TERM_PT_UNIX))
#define MOO_TERM_PT_UNIX_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  MOO_TYPE_TERM_PT_UNIX, MooTermPtUnixClass))

typedef struct _MooTermPtUnix           MooTermPtUnix;
typedef struct _MooTermPtUnixClass      MooTermPtUnixClass;


struct _MooTermPtUnix {
    MooTermPt   parent;

    GPid        child_pid;

    int         master;
    int         width;
    int         height;

    gboolean    non_block;
    GIOChannel *io;
    guint       io_watch_id;
};


struct _MooTermPtUnixClass {
    MooTermPtClass  parent_class;
};


static void     moo_term_pt_unix_finalize       (GObject        *object);

static void     set_size            (MooTermPt      *pt,
                                     guint           width,
                                     guint           height);
static gboolean fork_command        (MooTermPt      *pt,
                                     const MooTermCommand *cmd,
                                     const char     *working_dir,
                                     char          **envp,
                                     GError        **error);
static gboolean fork_argv           (MooTermPt      *pt,
                                     char          **argv,
                                     const char     *working_dir,
                                     char          **envp,
                                     GError        **error);
static void     pt_write            (MooTermPt      *pt,
                                     const char     *string,
                                     gssize          len);
static void     kill_child          (MooTermPt      *pt);

static gboolean read_child_out      (GIOChannel     *source,
                                     GIOCondition    condition,
                                     MooTermPtUnix  *pt);
static void     feed_buffer         (MooTermPtUnix  *pt,
                                     const char     *string,
                                     int             len);

static void     start_writer        (MooTermPt      *pt);
static void     stop_writer         (MooTermPt      *pt);


/* MOO_TYPE_TERM_PT_UNIX */
G_DEFINE_TYPE (MooTermPtUnix, moo_term_pt_unix, MOO_TYPE_TERM_PT)


static void moo_term_pt_unix_class_init (MooTermPtUnixClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    MooTermPtClass *pt_class = MOO_TERM_PT_CLASS (klass);

    gobject_class->finalize = moo_term_pt_unix_finalize;

    pt_class->set_size = set_size;
    pt_class->fork_command = fork_command;
    pt_class->write = pt_write;
    pt_class->kill_child = kill_child;
}


static void     moo_term_pt_unix_init            (MooTermPtUnix      *pt)
{
    MOO_TERM_PT(pt)->priv->child_alive = FALSE;

    pt->child_pid = (GPid)-1;

    pt->master = -1;
    pt->width = 80;
    pt->height = 24;

    pt->non_block = FALSE;
    pt->io = NULL;
    pt->io_watch_id = 0;
}


static void     moo_term_pt_unix_finalize        (GObject            *object)
{
    MooTermPtUnix *pt = MOO_TERM_PT_UNIX (object);
    kill_child (MOO_TERM_PT (pt));
    G_OBJECT_CLASS (moo_term_pt_unix_parent_class)->finalize (object);
}


static void     set_size        (MooTermPt      *pt,
                                 guint           width,
                                 guint           height)
{
    MooTermPtUnix *ptu;

    g_return_if_fail (MOO_IS_TERM_PT_UNIX (pt));

    ptu = MOO_TERM_PT_UNIX (pt);

    if (pt->priv->child_alive)
        _vte_pty_set_size (ptu->master, width, height);

    ptu->width = width;
    ptu->height = height;
}


static gboolean fork_argv       (MooTermPt      *pt_gen,
                                 char          **argv,
                                 const char     *working_dir,
                                 char          **envp,
                                 GError        **error)
{
    MooTermPtUnix *pt;
    int env_len = 0;
    char **new_env;
    int status, flags;
    int i;
    GSource *src;

    g_return_val_if_fail (argv != NULL && argv[0] != NULL, FALSE);
    g_return_val_if_fail (MOO_IS_TERM_PT_UNIX (pt_gen), FALSE);

    pt = MOO_TERM_PT_UNIX (pt_gen);

    g_return_val_if_fail (!pt_gen->priv->child_alive, FALSE);

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

    pt->master = _vte_pty_open (&pt->child_pid, new_env,
                                 argv[0],
                                 argv,
                                 working_dir,
                                 pt->width, pt->height,
                                 FALSE, FALSE, FALSE);
    g_strfreev (new_env);

    if (pt->master == -1)
    {
        g_critical ("%s: could not fork child", G_STRLOC);

        if (errno)
            g_set_error (error, MOO_TERM_ERROR, errno,
                         "could not fork command: %s",
                         g_strerror (errno));
        else
            g_set_error (error, MOO_TERM_ERROR, errno,
                         "could not fork command");

        return FALSE;
    }
    else
    {
        g_message ("%s: forked child pid %d", G_STRLOC, pt->child_pid);
    }

    if (waitpid (-1, &status, WNOHANG) == -1)
        g_critical ("%s: error in waitpid", G_STRLOC);

    if ((flags = fcntl (pt->master, F_GETFL)) < 0)
        g_critical ("%s: F_GETFL on master", G_STRLOC);
    else if (-1 == fcntl (pt->master, F_SETFL, O_NONBLOCK | flags))
        g_critical ("%s: F_SETFL on master", G_STRLOC);
    else
        pt->non_block = TRUE;

    pt->io =  g_io_channel_unix_new (pt->master);
    g_return_val_if_fail (pt->io != NULL, FALSE);

    g_io_channel_set_encoding (pt->io, NULL, NULL);
    g_io_channel_set_buffered (pt->io, FALSE);

    pt->io_watch_id = g_io_add_watch (pt->io,
                                      G_IO_IN | G_IO_PRI | G_IO_HUP,
                                      (GIOFunc) read_child_out,
                                      pt);

    src = g_main_context_find_source_by_id (NULL, pt->io_watch_id);

    if (src)
        g_source_set_priority (src, PT_READER_PRIORITY);
    else
        g_warning ("%s: could not find io_watch_id source", G_STRLOC);

    pt_gen->priv->child_alive = TRUE;

    return TRUE;
}


static gboolean fork_command    (MooTermPt      *pt_gen,
                                 const MooTermCommand *cmd,
                                 const char     *working_dir,
                                 char          **envp,
                                 GError        **error)
{
    g_return_val_if_fail (cmd != NULL, FALSE);
    g_return_val_if_fail (cmd->argv != NULL, FALSE);
    g_return_val_if_fail (MOO_IS_TERM_PT_UNIX (pt_gen), FALSE);

    return fork_argv (pt_gen, cmd->argv, working_dir, envp, error);
}


static void     kill_child      (MooTermPt      *pt_gen)
{
    MooTermPtUnix *pt = MOO_TERM_PT_UNIX (pt_gen);

    if (pt->io_watch_id)
    {
        g_source_remove (pt->io_watch_id);
        pt->io_watch_id = 0;
    }

    if (pt->io)
    {
        pt_write (MOO_TERM_PT (pt), "\4", 1);
        g_io_channel_shutdown (pt->io, TRUE, NULL);
        g_io_channel_unref (pt->io);
        pt->io = NULL;
    }

    stop_writer (pt_gen);
    pt_flush_pending_write (pt_gen);

    if (pt->master != -1)
    {
        _vte_pty_close (pt->master);
        pt->master = -1;
    }

    pt->non_block = FALSE;

    pt->child_pid = (GPid)-1;

    if (pt_gen->priv->child_alive)
    {
        pt_gen->priv->child_alive = FALSE;
        g_signal_emit_by_name (pt, "child-died");
    }
}


static gboolean read_child_out  (G_GNUC_UNUSED GIOChannel     *source,
                                 GIOCondition    condition,
                                 MooTermPtUnix  *pt)
{
    gboolean error_occured = FALSE;
    int error_no = 0;

    char buf[READ_CHUNK_SIZE];
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

    if (pt->non_block)
    {
        int r = read (pt->master, buf, READ_CHUNK_SIZE);

        switch (r)
        {
            case -1:
                if (errno != EAGAIN && errno != EINTR)
                {
                    error_occured = TRUE;
                    error_no = errno;
                    goto error;
                }
                break;

            case 0:
                break;

            default:
#if 0
            {
                char *s = _moo_term_nice_bytes (buf, r);
                g_print ("got '%s'\n", s);
                g_free (s);
            }
#endif
                feed_buffer (pt, buf, r);
                break;
        }

        return TRUE;
    }

    while (again && !error_occured && current < READ_CHUNK_SIZE)
    {
        struct pollfd fd = {pt->master, POLLIN | POLLPRI, 0};

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
                    int r = read (pt->master, buf + current, 1);

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
        feed_buffer (pt, buf, current);

    if (error_occured)
        goto error;

    return TRUE;

error:
    if (error_occured)
        g_message ("error in %s", G_STRLOC);

    if (error_no)
        g_message ("%s: %s", G_STRLOC, g_strerror (error_no));

    if (pt->io)
    {
        _vte_pty_close (pt->master);
        pt->master = -1;

        g_io_channel_shutdown (pt->io, TRUE, NULL);
        g_io_channel_unref (pt->io);
        pt->io = NULL;

        pt->io_watch_id = 0;
    }

    kill_child (MOO_TERM_PT (pt));

    return FALSE;
}


static void     feed_buffer     (MooTermPtUnix  *pt,
                                 const char     *string,
                                 int             len)
{
    moo_term_feed (MOO_TERM_PT(pt)->priv->term,
                   string, len);
}


/* writes given data to file, returns TRUE on successful write,
   FALSE when could not write al teh data, puts start of leftover
   to string, length of it to len, and fills err in case of error */
static gboolean do_write        (MooTermPt      *pt_gen,
                                 const char    **string,
                                 guint          *plen,
                                 int            *err)
{
    int written;

    MooTermPtUnix *pt = MOO_TERM_PT_UNIX (pt_gen);

    g_return_val_if_fail (pt->io != NULL, FALSE);

    written = write (pt->master, *string,
                     *plen > WRITE_CHUNK_SIZE ? WRITE_CHUNK_SIZE : *plen);

    if (written == -1)
    {
        *err = errno;

        switch (errno)
        {
            case EAGAIN:
            case EINTR:
                g_warning ("%s", G_STRLOC);
                return TRUE;

            default:
                return FALSE;
        }
    }
    else
    {
        *string += written;
        *plen -= written;

        if (!*plen)
            *string = NULL;

        return TRUE;
    }
}


static void     append                  (MooTermPt      *pt,
                                         const char     *data,
                                         guint           len)
{
    GByteArray *ar = g_byte_array_sized_new (len);
    g_byte_array_append (ar, (const guint8*)data, len);
    g_queue_push_tail (pt->priv->pending_write, ar);
}


static gboolean write_cb        (MooTermPt      *pt)
{
    pt_write (pt, NULL, 0);
    return TRUE;
}

static void     start_writer    (MooTermPt      *pt)
{
    if (!pt->priv->pending_write_id)
        pt->priv->pending_write_id =
                g_idle_add_full (PT_WRITER_PRIORITY,
                                 (GSourceFunc) write_cb,
                                 pt, NULL);
}

static void     stop_writer     (MooTermPt      *pt)
{
    if (pt->priv->pending_write_id)
    {
        g_source_remove (pt->priv->pending_write_id);
        pt->priv->pending_write_id = 0;
    }
}


static void     pt_write        (MooTermPt      *pt,
                                 const char     *data,
                                 int             data_len)
{
    g_return_if_fail (data == NULL || data_len != 0);
    g_return_if_fail (pt->priv->child_alive);

    while (data || !g_queue_is_empty (pt->priv->pending_write))
    {
        int err = 0;
        const char *string;
        guint len;
        GByteArray *freeme = NULL;

        if (!g_queue_is_empty (pt->priv->pending_write))
        {
            if (data)
            {
                append (pt, data, data_len > 0 ? (guint)data_len : strlen (data));
                data = NULL;
            }

            freeme = g_queue_peek_head (pt->priv->pending_write);
            string = (const char *) freeme->data;
            len = freeme->len;
        }
        else
        {
            string = data;
            len = data_len > 0 ? (guint)data_len : strlen (data);
            data = NULL;
        }

        if (do_write (pt, &string, &len, &err))
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
                    append (pt, string, len);
                }

                break;
            }
            else if (freeme)
            {
                g_byte_array_free (freeme, TRUE);
                g_queue_pop_head (pt->priv->pending_write);
            }
        }
        else
        {
            g_message ("%s: stopping writing to child", G_STRLOC);
            if (err)
                g_message ("%s: %s", G_STRLOC, g_strerror (err));
            kill_child (pt);
        }
    }

    if (!g_queue_is_empty (pt->priv->pending_write))
        start_writer (pt);
    else
        stop_writer (pt);
}


char            moo_term_pt_get_erase_char  (MooTermPt      *pt_gen)
{
    MooTermPtUnix *pt = MOO_TERM_PT_UNIX (pt_gen);
    struct termios tio;

    g_return_val_if_fail (pt->master != -1, 0);

    if (!tcgetattr (pt->master, &tio))
    {
        return tio.c_cc[VERASE];
    }
    else
    {
        g_warning ("%s: %s", G_STRLOC,
                   g_strerror (errno));
        return 127;
    }
}


void            moo_term_pt_send_intr       (MooTermPt      *pt)
{
    g_return_if_fail (pt->priv->child_alive);
    pt_flush_pending_write (pt);
    pt_write (pt, "\003", 1);
}


/* TODO: it should be in glib */
MooTermCommand *moo_term_get_default_shell (void)
{
    static char *argv[2] = {NULL, NULL};

    if (!argv[0])
    {
        argv[0] = g_strdup (g_getenv ("SHELL"));
        if (!argv[0]) argv[0] = g_strdup ("/bin/sh");
    }

    return moo_term_command_new (NULL, argv);
}


/* it's a win32 function */
void        moo_term_set_helper_directory   (G_GNUC_UNUSED const char *dir)
{
    g_return_if_reached ();
}


static char *argv_to_cmd_line (char **argv)
{
    GString *cmd = NULL;
    char **p;

    g_return_val_if_fail (argv != NULL, NULL);
    g_return_val_if_fail (argv[0] != NULL, NULL);

    for (p = argv; *p != NULL; ++p)
    {
        char *quoted = g_shell_quote (*p);
        /* TODO: may not happen? */
        g_return_val_if_fail (quoted != NULL,
                              g_string_free (cmd, FALSE));
        if (cmd)
            g_string_append_printf (cmd, " %s", quoted);
        else
            cmd = g_string_new (quoted);
        g_free (quoted);
    }

    return g_string_free (cmd, FALSE);
}


static char **cmd_line_to_argv (const char  *cmd_line,
                                GError     **error)
{
    int argc;
    char **argv;
    g_shell_parse_argv (cmd_line, &argc, &argv, error);
    return argv;
}


gboolean        moo_term_check_cmd          (MooTermCommand *cmd,
                                             GError     **error)
{
    g_return_val_if_fail (cmd != NULL, FALSE);
    g_return_val_if_fail (cmd->cmd_line != NULL || cmd->argv != NULL, FALSE);

    if (cmd->argv)
    {
        g_free (cmd->cmd_line);
        cmd->cmd_line = argv_to_cmd_line (cmd->argv);
        g_return_val_if_fail (cmd->cmd_line != NULL, FALSE);
        return TRUE;
    }
    else
    {
        cmd->argv = cmd_line_to_argv (cmd->cmd_line, error);
        return cmd->argv != NULL;
    }
}
