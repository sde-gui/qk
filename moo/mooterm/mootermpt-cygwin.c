/*
 *   mooterm/mootermpt-cygwin.c
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

#define MOOTERM_COMPILATION
#include "mooterm/mootermpt-private.h"
#include "mooterm/mooterm-private.h"
#include "mooterm/mootermhelper.h"
#include "mooterm/mooterm-private.h"
#include "mooutils/moomarshals.h"
#include "mooutils/moocompat.h"
#include "mooutils/mooutils-misc.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <errno.h>

#define TRY_NUM                 10
#define SLEEP_TIME              10
#define TERM_EMULATION          "xterm"
#define WRITE_CHUNK_SIZE        4096
#define POLL_TIME               5
#define POLL_NUM                1

static char *HELPER_DIR = NULL;
#define HELPER_BINARY "termhelper.exe"


void
moo_term_set_helper_directory (const char *dir)
{
    g_free (HELPER_DIR);
    HELPER_DIR = g_strdup (dir);
}


#define TERM_WIDTH(pt__)  (MOO_TERM_PT(pt__)->priv->term->priv->width)
#define TERM_HEIGHT(pt__) (MOO_TERM_PT(pt__)->priv->term->priv->height)


#define MOO_TERM_PT_CYG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOO_TYPE_TERM_PT_CYG, MooTermPtCyg))
#define MOO_TERM_PT_CYG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  MOO_TYPE_TERM_PT_CYG, MooTermPtCygClass))
#define MOO_IS_TERM_PT_CYG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOO_TYPE_TERM_PT_CYG))
#define MOO_IS_TERM_PT_CYG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  MOO_TYPE_TERM_PT_CYG))
#define MOO_TERM_PT_CYG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  MOO_TYPE_TERM_PT_CYG, MooTermPtCygClass))

typedef struct _MooTermPtCyg           MooTermPtCyg;
typedef struct _MooTermPtCygClass      MooTermPtCygClass;


struct _MooTermPtCyg {
    MooTermPt   parent;

    GPid        pid;
    gulong      process_id;
    int         in;
    int         out;
    GIOChannel *in_io;
    GIOChannel *out_io;
    guint       watch_id;
    guint       out_watch_id;
};


struct _MooTermPtCygClass {
    MooTermPtClass  parent_class;
};

static void moo_term_pt_cyg_finalize    (GObject    *object);

static gboolean read_helper_out (GIOChannel *source,
                                 GIOCondition condition,
                                 MooTermPtCyg   *self);

static void     set_size            (MooTermPt      *pt,
                                     guint           width,
                                     guint           height);
static gboolean fork_command        (MooTermPt      *pt,
                                     const MooTermCommand *cmd,
                                     GError        **error);
static void     pt_write            (MooTermPt      *pt,
                                     const char     *string,
                                     gssize          len);
static void     kill_child          (MooTermPt      *pt);
static void     send_intr           (MooTermPt      *pt);
static char     get_erase_char      (MooTermPt      *pt);

static gboolean run_in_helper       (const char *cmd,
                                     const char *working_dir,
                                     char **env,
                                     guint width, guint height,
                                     int *hin, int *hout,
                                     GPid *hpid, gulong *hproc_id,
                                     GError        **error);


/* MOO_TYPE_TERM_PT_CYG */
G_DEFINE_TYPE (MooTermPtCyg, _moo_term_pt_cyg, MOO_TYPE_TERM_PT)


static void
_moo_term_pt_cyg_class_init (MooTermPtCygClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    MooTermPtClass *pt_class = MOO_TERM_PT_CLASS (klass);

    gobject_class->finalize = moo_term_pt_cyg_finalize;

    pt_class->set_size = set_size;
    pt_class->fork_command = fork_command;
    pt_class->write = pt_write;
    pt_class->kill_child = kill_child;
    pt_class->send_intr = send_intr;
    pt_class->get_erase_char = get_erase_char;
}


static void
_moo_term_pt_cyg_init (MooTermPtCyg *pt)
{
    pt->pid = (GPid) -1;
    pt->process_id = 0;
    pt->watch_id = 0;
    pt->in = -1;
    pt->in_io = NULL;
    pt->out = -1;
    pt->out_io = NULL;
    pt->out_watch_id = 0;
}


static void
moo_term_pt_cyg_finalize (GObject *object)
{
    MooTermPtCyg *pt = MOO_TERM_PT_CYG (object);
    kill_child (MOO_TERM_PT (pt));
    G_OBJECT_CLASS (_moo_term_pt_cyg_parent_class)->finalize (object);
}


MooTermCommand *
_moo_term_get_default_shell (void)
{
    g_return_val_if_reached (NULL);
}


static gboolean
fork_command (MooTermPt      *pt_gen,
              const MooTermCommand *cmd,
              GError        **error)
{
    MooTermPtCyg *pt;
    gboolean result;

    g_return_val_if_fail (!pt_gen->priv->child_alive, FALSE);
    g_return_val_if_fail (cmd->cmd_line != NULL, FALSE);

    pt = MOO_TERM_PT_CYG (pt_gen);

    result = run_in_helper (cmd->cmd_line,
                            cmd->working_dir, cmd->envp,
                            TERM_WIDTH (pt), TERM_HEIGHT (pt),
                            &pt->in, &pt->out, &pt->pid,
                            &pt->process_id, error);

    if (!result)
        return FALSE;

    g_message ("%s: started helper pid %d", G_STRLOC, (int) pt->pid);

    pt->in_io = g_io_channel_win32_new_fd (pt->in);
    g_io_channel_set_encoding (pt->in_io, NULL, NULL);
//     pt->priv->in_watch_id = _moo_io_add_watch (pt->priv->in_io,
//                                              (GIOCondition)(G_IO_ERR | G_IO_HUP),
//                                              (GIOFunc)helper_in_hup,
//                                              pt);

    pt->out_io = g_io_channel_win32_new_fd (pt->out);
    g_io_channel_set_encoding (pt->out_io, NULL, NULL);
    g_io_channel_set_buffered (pt->in_io, FALSE);
    pt->out_watch_id = _moo_io_add_watch_full (pt->out_io,
                                               G_PRIORITY_DEFAULT_IDLE,
                                               G_IO_IN | G_IO_PRI | G_IO_HUP,
                                               (GIOFunc) read_helper_out,
                                               pt, NULL);

//     GSource *src = g_main_context_find_source_by_id (NULL, helper.out_watch_id);
//     if (src) g_source_set_priority (src, READ_HELPER_OUT_PRIORITY);
//     else g_warning ("%s: could not find helper_io_watch source", G_STRLOC);

    pt_gen->priv->child_alive = TRUE;
    return TRUE;
}


static gboolean
read_helper_out (GIOChannel     *source,
                 GIOCondition    condition,
                 MooTermPtCyg   *pt)
{
    GError *err = NULL;
    gboolean error_occured = FALSE;

    g_assert (pt->out_io == source);

    if (condition & G_IO_HUP)
    {
        g_message ("%s: G_IO_HUP", G_STRLOC);
        error_occured = TRUE;
    }
    else if (condition & G_IO_ERR)
    {
        g_message ("%s: G_IO_ERR", G_STRLOC);
        error_occured = TRUE;
    }
    else if (condition & (G_IO_IN | G_IO_PRI))
    {
        char buf[INPUT_CHUNK_SIZE];
        gsize count = 0, read, to_read;
        int again = TRY_NUM;

        to_read = _moo_term_get_input_chunk_len (MOO_TERM_PT(pt)->priv->term);
        g_assert (to_read <= sizeof (buf));

        if (!to_read)
            return TRUE;

        g_io_channel_read_chars (source, buf, 1, &read, &err);

        if (read == 1)
            ++count;

        while (again && !err && !error_occured && count < to_read)
        {
            if (g_io_channel_get_buffer_condition (source) & G_IO_IN)
            {
                g_io_channel_read_chars (source, buf + count, 1, &read, &err);

                if (read == 1)
                    ++count;
                else if (--again)
                    g_usleep (SLEEP_TIME);
            }
            else
            {
                if (--again)
                    g_usleep (SLEEP_TIME);
            }
        }

        if (count)
            moo_term_feed (MOO_TERM_PT(pt)->priv->term, buf, count);

        error_occured = (err != NULL);
    }
    else
    {
        g_critical ("%s: unknown source condition", G_STRLOC);
    }

    if (error_occured)
    {
        if (err)
        {
            g_message ("%s: %s", G_STRLOC, err->message);
            g_error_free (err);
        }

        g_io_channel_shutdown (pt->in_io, TRUE, NULL);
        g_io_channel_unref (pt->in_io);
        pt->in_io = NULL;
        pt->in = -1;
        kill_child (MOO_TERM_PT (pt));

        return FALSE;
    }

    return TRUE;
}


static void
kill_child (MooTermPt *pt_gen)
{
    MooTermPtCyg *pt = MOO_TERM_PT_CYG (pt_gen);

    if (pt->in_io)
    {
        char cmd[2] = {HELPER_CMD_CHAR, HELPER_GOODBYE};
        pt_write (MOO_TERM_PT (pt), cmd, 2);
        g_io_channel_shutdown (pt->in_io, TRUE, NULL);
        g_io_channel_unref (pt->in_io);
        pt->in_io = NULL;
    }

    if (pt->out_io)
    {
        g_io_channel_shutdown (pt->out_io, TRUE, NULL);
        g_io_channel_unref (pt->out_io);
        pt->out_io = NULL;
    }

    if (pt->watch_id)
    {
        g_source_remove (pt->watch_id);
        pt->watch_id = 0;
    }

    if (pt->out_watch_id)
    {
        g_source_remove (pt->out_watch_id);
        pt->out_watch_id = 0;
    }

    if (pt->pid != (GPid) -1)
    {
        g_spawn_close_pid (pt->pid);
        pt->pid = (GPid) -1;
    }

    pt->in = -1;
    pt->out = -1;
    pt->process_id = 0;

    if (pt_gen->priv->child_alive)
    {
        pt_gen->priv->child_alive = FALSE;
        g_signal_emit_by_name (pt, "child-died");
    }
}


static void
set_size (MooTermPt      *pt,
          guint           width,
          guint           height)
{
    if (pt->priv->child_alive)
        pt_write (MOO_TERM_PT (pt), set_size_cmd (width, height), SIZE_CMD_LEN);
}


static void
append (MooTermPt      *pt,
        const char     *data,
        guint           len)
{
    GByteArray *ar = g_byte_array_sized_new (len);
    g_byte_array_append (ar, (const guint8*)data, len);
    g_queue_push_tail (pt->priv->pending_write, ar);
}


/* writes given data to file, returns TRUE on successful write,
   FALSE when could not write al teh data, puts start of leftover
   to string, length of it to len, and fills err in case of error */
static gboolean
do_write (MooTermPt      *pt_gen,
          const char    **string,
          guint          *plen,
          GError        **err)
{
    GIOStatus status;
    guint written;

    MooTermPtCyg *pt = MOO_TERM_PT_CYG (pt_gen);

    g_return_val_if_fail (pt->in_io != NULL, FALSE);

    status = g_io_channel_write_chars (pt->in_io, *string,
                                       *plen > WRITE_CHUNK_SIZE ? WRITE_CHUNK_SIZE : *plen,
                                       &written, err);

    if (status == G_IO_STATUS_ERROR)
        return FALSE;

    *string += written;
    *plen -= written;

    if (!*plen)
        *string = NULL;

    return TRUE;
}


static gboolean
write_cb (MooTermPt *pt)
{
    pt_write (pt, NULL, 0);
    return TRUE;
}

static void
start_writer (MooTermPt *pt)
{
    if (!pt->priv->pending_write_id)
        pt->priv->pending_write_id =
                _moo_idle_add_full (PT_WRITER_PRIORITY,
                                    (GSourceFunc) write_cb,
                                    pt, NULL);
}

static void
stop_writer (MooTermPt *pt)
{
    if (pt->priv->pending_write_id)
    {
        g_source_remove (pt->priv->pending_write_id);
        pt->priv->pending_write_id = 0;
    }
}


static void
pt_write (MooTermPt      *pt,
          const char     *data,
          gssize          data_len)
{
    g_return_if_fail (data == NULL || data_len != 0);
    g_return_if_fail (pt->priv->child_alive);

    while (data || !g_queue_is_empty (pt->priv->pending_write))
    {
        GError *err = NULL;
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
            kill_child (pt);
        }

        if (err)
        {
            g_message ("%s: %s", G_STRLOC, err->message);
            g_error_free (err);
        }
    }

    if (!g_queue_is_empty (pt->priv->pending_write))
        start_writer (pt);
    else
        stop_writer (pt);
}



enum {
    READ_END = 0,
    WRITE_END = 1
};

static gboolean
run_in_helper (const char *cmd,
               const char *working_dir,
               char **env,
               guint width, guint height,
               int *hin, int *hout,
               GPid *hpid, gulong *hproc_id,
               GError        **error)
{
    /* char *cwidth = NULL, *cheight = NULL; */
    int my_stdin = _dup(0);
    int my_stdout = _dup(1);

    GPid helper = (GPid)INVALID_HANDLE_VALUE;
    int helper_in[2] = {-1, -1};
    int helper_out[2] = {-1, -1};

    char *cmd_line = NULL;
    STARTUPINFO sinfo;
    PROCESS_INFORMATION pinfo;

    GString *helper_binary = NULL;
    GPtrArray *saved_env = NULL;

    const char **s;

    g_return_val_if_fail (cmd != NULL && hin != NULL && hout != NULL &&
            hpid != NULL, FALSE);

    if(_pipe (helper_in, 512, O_NOINHERIT | O_BINARY) == -1)
    {
        g_set_error (error, MOO_TERM_ERROR, MOO_TERM_ERROR_FAILED,
                     "_pipe: %s", g_strerror (errno));
        goto error;
    }

    if(_pipe (helper_out, 512, O_NOINHERIT | O_BINARY) == -1)
    {
        g_set_error (error, MOO_TERM_ERROR, MOO_TERM_ERROR_FAILED,
                     "_pipe: %s", g_strerror (errno));
        goto error;
    }

    if (_dup2 (helper_in[READ_END], 0))
    {
        g_set_error (error, MOO_TERM_ERROR, MOO_TERM_ERROR_FAILED,
                     "_dup2: %s", g_strerror (errno));
        goto error;
    }

    if (_dup2 (helper_out[WRITE_END], 1))
    {
        g_set_error (error, MOO_TERM_ERROR, MOO_TERM_ERROR_FAILED,
                     "_dup2: %s", g_strerror (errno));
        goto error;
    }

    close (helper_in[READ_END]);
    close (helper_out[WRITE_END]);

#if 0
//     cwidth = g_strdup_printf ("%d", width);
//     cheight = g_strdup_printf ("%d", height);
//     helper = (GPid) _spawnl(P_NOWAIT,
//                             HELPER_BINARY,
//                             HELPER_BINARY,
//                             cwidth,
//                             cheight,
//                             cmd,
//                             NULL);
#endif

    /*
     * We need to use CreateProcess here in order to be able to create new,
     * hidden console
     */

    if (!HELPER_DIR || !HELPER_DIR[0])
    {
        g_warning ("%s: helper directory is not set", G_STRLOC);
        g_free (HELPER_DIR);
        HELPER_DIR = NULL;
    }

    if (HELPER_DIR)
    {
        helper_binary = g_string_new (HELPER_DIR);
        g_string_append (helper_binary, "\\" HELPER_BINARY);
    }
    else
    {
        helper_binary = g_string_new (HELPER_BINARY);
    }

    g_message ("%s: helper '%s'", G_STRLOC, helper_binary->str);

    cmd_line = g_strdup_printf ("%s %d %d %s", HELPER_BINARY,
                                width, height, cmd);

    saved_env = g_ptr_array_new ();

    if (env)
    {
        for (s = (const char**)env; *s != NULL; ++s)
        {
            char **pair = g_strsplit (*s, "=", 2);

            if (pair && pair[0])
            {
                const char *val = g_getenv (pair[0]);

                g_ptr_array_add (saved_env, g_strdup (pair[0]));
                g_ptr_array_add (saved_env, g_strdup (val));

                if (pair[1])
                    g_setenv (pair[0], pair[1], TRUE);
                else
                    g_unsetenv (pair[0]);
            }

            g_strfreev (pair);
        }
    }

    if (HELPER_DIR)
    {
        const char *var = "PATH";
        const char *val = g_getenv (var);
        char *newval = 0;

        g_message ("%s: pushing '%s' to %s", G_STRLOC, HELPER_DIR, var);

        g_ptr_array_add (saved_env, g_strdup (var));
        g_ptr_array_add (saved_env, g_strdup (val));

        if (val)
            newval = g_strdup_printf ("%s;%s", HELPER_DIR, val);
        else
            newval = g_strdup (HELPER_DIR);

        g_setenv (var, newval, TRUE);
        g_free (newval);
    }

    if (working_dir)
    {
        const char *val = g_getenv (MOO_TERM_HELPER_ENV);

        g_ptr_array_add (saved_env, g_strdup (MOO_TERM_HELPER_ENV));
        g_ptr_array_add (saved_env, g_strdup (val));

        g_setenv (MOO_TERM_HELPER_ENV, working_dir, TRUE);
    }

    g_ptr_array_add (saved_env, NULL);

    g_message ("%s: command line '%s'", G_STRLOC, cmd_line);

    memset (&sinfo, 0, sizeof (sinfo));
    sinfo.cb = sizeof (STARTUPINFO);
    sinfo.dwFlags = STARTF_USESTDHANDLES;
    sinfo.hStdInput = (HANDLE)_get_osfhandle (0);
    sinfo.hStdOutput = (HANDLE)_get_osfhandle (1);
    sinfo.hStdError = (HANDLE)_get_osfhandle (2);

    sinfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    sinfo.wShowWindow = SW_HIDE;

    moo_disable_win32_error_message ();

    if (! CreateProcess (helper_binary->str, cmd_line, NULL, NULL, TRUE,
                         CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP,
                         NULL, HELPER_DIR, &sinfo, &pinfo))
    {
        char *msg = g_win32_error_message (GetLastError ());
        g_set_error (error, MOO_TERM_ERROR, MOO_TERM_ERROR_FAILED,
                     "CreateProcess: %s", msg);
        g_free (msg);

        moo_enable_win32_error_message ();

        goto error;
    }

    moo_enable_win32_error_message ();

    g_free (cmd_line);
    cmd_line = NULL;

    for (s = (const char**)saved_env->pdata; *s != NULL; )
    {
        const char *var = *s++;
        const char *val = *s++;

        if (val)
            g_setenv (var, val, TRUE);
        else
            g_unsetenv (var);
    }

    CloseHandle (pinfo.hThread);

    /***********************************************************************/

#if 0
//     g_free (cwidth); cwidth = NULL;
//     g_free (cheight); cheight = NULL;
//     if (helper == (GPid)-1) {
//         g_critical ("%s: error in spawn()", G_STRLOC);
//         goto error;
//     }
#endif

    _dup2 (my_stdin, 0);
    _dup2 (my_stdout, 1);
    close (my_stdin); my_stdin = -1;
    close (my_stdout); my_stdout = -1;

    /* g_message ("%s: created process pid %d", G_STRLOC, (int)pinfo.dwProcessId); */

    *hin = helper_in[WRITE_END];
    *hout = helper_out[READ_END];
    *hpid = pinfo.hProcess;
    *hproc_id = pinfo.dwProcessId;

    if (helper_binary)
        g_string_free (helper_binary, TRUE);

    if (saved_env)
    {
        g_strfreev ((char**)saved_env->pdata);
        g_ptr_array_free (saved_env, FALSE);
    }

    return TRUE;

    /* cleanup on error */
error:
    _dup2(my_stdin, 0);
    _dup2(my_stdout, 1);
    close (helper_in[0]);
    close (helper_in[1]);
    close (helper_out[0]);
    close (helper_out[1]);
#if 0
//     if (cwidth) g_free (cwidth);
//     if (cheight) g_free (cheight);
#endif
    if (cmd_line)
        g_free (cmd_line);

    CloseHandle ((HANDLE)helper);

    if (helper_binary)
        g_string_free (helper_binary, TRUE);

    if (saved_env)
    {
        for (s = (const char**)saved_env->pdata; *s != NULL; )
        {
            const char *var = *s++;
            const char *val = *s++;

            if (val)
                g_setenv (var, val, TRUE);
            else
                g_unsetenv (var);
        }

        g_strfreev ((char**)saved_env->pdata);
        g_ptr_array_free (saved_env, FALSE);
    }

    return FALSE;
}


static char
get_erase_char (G_GNUC_UNUSED MooTermPt *pt)
{
    return 127;
}


static void
send_intr (MooTermPt *pt)
{
    g_return_if_fail (pt->priv->child_alive);
    pt_discard_pending_write (pt);
    pt_write (pt, "\3", 1);
}


gboolean
_moo_term_check_cmd (MooTermCommand *cmd,
                     GError        **error)
{
    GString *cmd_line = NULL;
    char **p;

    g_return_val_if_fail (cmd != NULL, FALSE);

    if (cmd->cmd_line)
    {
        g_strfreev (cmd->argv);
        cmd->argv = NULL;
        return TRUE;
    }

    if (!cmd->argv)
    {
        g_set_error (error, MOO_TERM_ERROR, MOO_TERM_ERROR_INVAL,
                     "NULL command line and arguments array");
        return FALSE;
    }

    if (!cmd->argv[0])
    {
        g_set_error (error, MOO_TERM_ERROR, MOO_TERM_ERROR_INVAL,
                     "empty arguments array");
        return FALSE;
    }

    cmd_line = g_string_new ("");

    for (p = cmd->argv; *p != NULL; ++p)
    {
        if (strchr (*p, ' '))
            g_string_append_printf (cmd_line, "\"%s\" ", *p);
        else
            g_string_append_printf (cmd_line, "%s ", *p);
    }

    cmd->cmd_line = g_string_free (cmd_line, FALSE);
    return TRUE;
}
