/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   moocmdview.c
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

#include "mooedit/moocmdview.h"
#include "mooutils/moomarshals.h"
#include <sys/wait.h>
#include <unistd.h>


struct _MooCmdViewPrivate {
    gboolean running;
    char *cmd;
    int exit_status;
    GPid pid;
    int stdout;
    int stderr;
    GIOChannel *stdout_io;
    GIOChannel *stderr_io;
    guint child_watch;
    guint stdout_watch;
    guint stderr_watch;

    GtkTextTag *error_tag;
    GtkTextTag *message_tag;
    GtkTextTag *stdout_tag;
    GtkTextTag *stderr_tag;
};

static void      moo_cmd_view_finalize      (GObject    *object);
static void      moo_cmd_view_destroy       (GtkObject  *object);
static GObject  *moo_cmd_view_constructor   (GType                  type,
                                             guint                  n_construct_properties,
                                             GObjectConstructParam *construct_param);

static void     moo_cmd_view_check_stop     (MooCmdView *view);
static void     moo_cmd_view_cleanup        (MooCmdView *view);

static gboolean moo_cmd_view_abort_real     (MooCmdView *view);
static gboolean moo_cmd_view_cmd_exit       (MooCmdView *view,
                                             int         status);
static gboolean moo_cmd_view_stdout_line    (MooCmdView *view,
                                             const char *line);
static gboolean moo_cmd_view_stderr_line    (MooCmdView *view,
                                             const char *line);



enum {
    ABORT,
    CMD_EXIT,
    OUTPUT_LINE,
    STDOUT_LINE,
    STDERR_LINE,
    JOB_STARTED,
    JOB_FINISHED,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];


enum {
    PROP_0,
};


/* MOO_TYPE_CMD_VIEW */
G_DEFINE_TYPE (MooCmdView, moo_cmd_view, MOO_TYPE_LINE_VIEW)


static void
moo_cmd_view_class_init (MooCmdViewClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GtkObjectClass *gtkobject_class = GTK_OBJECT_CLASS (klass);

    gobject_class->finalize = moo_cmd_view_finalize;
    gobject_class->constructor = moo_cmd_view_constructor;

    gtkobject_class->destroy = moo_cmd_view_destroy;

    klass->abort = moo_cmd_view_abort_real;
    klass->cmd_exit = moo_cmd_view_cmd_exit;
    klass->output_line = NULL;
    klass->stdout_line = moo_cmd_view_stdout_line;
    klass->stderr_line = moo_cmd_view_stderr_line;

    signals[ABORT] =
            g_signal_new ("abort",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                          G_STRUCT_OFFSET (MooCmdViewClass, abort),
                          g_signal_accumulator_true_handled, NULL,
                          _moo_marshal_BOOL__VOID,
                          G_TYPE_BOOLEAN, 0);

    signals[CMD_EXIT] =
            g_signal_new ("cmd-exit",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooCmdViewClass, cmd_exit),
                          g_signal_accumulator_true_handled, NULL,
                          _moo_marshal_BOOL__INT,
                          G_TYPE_BOOLEAN, 1,
                          G_TYPE_INT);

    signals[OUTPUT_LINE] =
            g_signal_new ("output-line",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooCmdViewClass, output_line),
                          g_signal_accumulator_true_handled, NULL,
                          _moo_marshal_BOOL__STRING_BOOL,
                          G_TYPE_BOOLEAN, 2,
                          G_TYPE_STRING, G_TYPE_BOOLEAN);

    signals[STDOUT_LINE] =
            g_signal_new ("stdout-line",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooCmdViewClass, stdout_line),
                          g_signal_accumulator_true_handled, NULL,
                          _moo_marshal_BOOL__STRING,
                          G_TYPE_BOOLEAN, 1,
                          G_TYPE_STRING);

    signals[STDERR_LINE] =
            g_signal_new ("stderr-line",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooCmdViewClass, stderr_line),
                          g_signal_accumulator_true_handled, NULL,
                          _moo_marshal_BOOL__STRING,
                          G_TYPE_BOOLEAN, 1,
                          G_TYPE_STRING);

    signals[JOB_STARTED] =
            g_signal_new ("job-started",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooCmdViewClass, job_started),
                          NULL, NULL,
                          _moo_marshal_VOID__STRING,
                          G_TYPE_NONE, 1,
                          G_TYPE_STRING);

    signals[JOB_FINISHED] =
            g_signal_new ("job-finished",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooCmdViewClass, job_finished),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);
}


static void
moo_cmd_view_init (MooCmdView *view)
{
    view->priv = g_new0 (MooCmdViewPrivate, 1);
}


static void
moo_cmd_view_finalize (GObject *object)
{
    MooCmdView *view = MOO_CMD_VIEW (object);

    g_free (view->priv);

    G_OBJECT_CLASS (moo_cmd_view_parent_class)->finalize (object);
}


static GObject*
moo_cmd_view_constructor (GType                  type,
                          guint                  n_props,
                          GObjectConstructParam *props)
{
    GObject *object;
    MooCmdView *view;
    GtkTextBuffer *buffer;

    object = G_OBJECT_CLASS(moo_cmd_view_parent_class)->constructor (type, n_props, props);
    view = MOO_CMD_VIEW (object);

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

    view->priv->message_tag = gtk_text_buffer_create_tag (buffer, "message", NULL);
    view->priv->error_tag = gtk_text_buffer_create_tag (buffer, "error", NULL);
    view->priv->stdout_tag = gtk_text_buffer_create_tag (buffer, "stdout", NULL);
    view->priv->stderr_tag = gtk_text_buffer_create_tag (buffer, "stderr", NULL);

    g_object_set (view->priv->error_tag, "foreground", "red", NULL);
    g_object_set (view->priv->stderr_tag, "foreground", "red", NULL);

    return object;
}


static void
moo_cmd_view_destroy (GtkObject *object)
{
    MooCmdView *view = MOO_CMD_VIEW (object);

    moo_cmd_view_abort (view);
    moo_cmd_view_cleanup (view);

    if (GTK_OBJECT_CLASS (moo_cmd_view_parent_class)->destroy)
        GTK_OBJECT_CLASS (moo_cmd_view_parent_class)->destroy (object);
}


GtkWidget*
moo_cmd_view_new (void)
{
    return g_object_new (MOO_TYPE_CMD_VIEW, NULL);
}


static void
command_exit (GPid        pid,
              gint        status,
              MooCmdView *view)
{
    g_return_if_fail (pid == view->priv->pid);

    view->priv->child_watch = 0;
    view->priv->exit_status = status;

    g_spawn_close_pid (view->priv->pid);
    view->priv->pid = 0;

    moo_cmd_view_check_stop (view);
}


static void
process_line (MooCmdView  *view,
              const char  *line,
              gboolean     stderr)
{
    gboolean handled = FALSE;

    g_signal_emit (view, signals[OUTPUT_LINE], 0, line, stderr, &handled);

    if (!handled)
    {
        if (stderr)
            g_signal_emit (view, signals[STDERR_LINE], 0, line, &handled);
        else
            g_signal_emit (view, signals[STDOUT_LINE], 0, line, &handled);
    }
}


static gboolean
command_out_or_err (MooCmdView     *view,
                    GIOChannel     *channel,
                    GIOCondition    condition,
                    gboolean        stdout)
{
    char *line;
    gsize line_end;
    GError *error = NULL;
    GIOStatus status;

    status = g_io_channel_read_line (channel, &line, NULL, &line_end, &error);

    if (line)
    {
        line[line_end] = 0;
        process_line (view, line, !stdout);
        g_free (line);
    }

    if (error)
    {
        g_warning ("%s: %s", G_STRLOC, error->message);
        g_error_free (error);
        return FALSE;
    }

    if (condition & (G_IO_ERR | G_IO_HUP))
        return FALSE;

    if (status == G_IO_STATUS_EOF)
        return FALSE;

    return TRUE;
}


static gboolean
command_out (GIOChannel     *channel,
             GIOCondition    condition,
             MooCmdView     *view)
{
    return command_out_or_err (view, channel, condition, TRUE);
}


static gboolean
command_err (GIOChannel     *channel,
             GIOCondition    condition,
             MooCmdView     *view)
{
    return command_out_or_err (view, channel, condition, FALSE);
}


static void
try_channel_leftover (MooCmdView  *view,
                      GIOChannel  *channel,
                      gboolean     stdout)
{
    char *text;

    g_io_channel_read_to_end (channel, &text, NULL, NULL);

    if (text)
    {
        char **lines, **p;
        g_strdelimit (text, "\r", '\n');
        lines = g_strsplit (text, "\n", 0);

        if (lines)
        {
            for (p = lines; *p != NULL; p++)
                if (**p)
                    process_line (view, *p, !stdout);
        }

        g_strfreev (lines);
        g_free (text);
    }
}


static void
stdout_watch_removed (MooCmdView *view)
{
    if (view->priv->stdout_io)
    {
        try_channel_leftover (view, view->priv->stdout_io, TRUE);
        g_io_channel_unref (view->priv->stdout_io);
    }

    view->priv->stdout_io = NULL;
    view->priv->stdout_watch = 0;
    moo_cmd_view_check_stop (view);
}


static void
stderr_watch_removed (MooCmdView *view)
{
    if (view->priv->stderr_io)
    {
        try_channel_leftover (view, view->priv->stderr_io, TRUE);
        g_io_channel_unref (view->priv->stderr_io);
    }

    view->priv->stderr_io = NULL;
    view->priv->stderr_watch = 0;
    moo_cmd_view_check_stop (view);
}


static void
child_setup (G_GNUC_UNUSED gpointer dummy)
{
    setpgrp ();
}


gboolean
moo_cmd_view_run_command (MooCmdView *view,
                          const char *cmd,
                          const char *job_name)
{
    GError *error = NULL;
    char **argv = NULL;
    gboolean result = TRUE;

    g_return_val_if_fail (MOO_IS_CMD_VIEW (view), FALSE);
    g_return_val_if_fail (cmd && cmd[0], FALSE);

    g_return_val_if_fail (!view->priv->running, FALSE);

    moo_line_view_write_line (MOO_LINE_VIEW (view), cmd, -1,
                              view->priv->message_tag);

    argv = g_new (char*, 4);
    argv[0] = g_strdup ("/bin/sh");
    argv[1] = g_strdup ("-c");
    argv[2] = g_strdup (cmd);
    argv[3] = NULL;

    g_spawn_async_with_pipes (NULL,
                              argv, NULL,
                              G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD,
                              child_setup, NULL,
                              &view->priv->pid,
                              NULL,
                              &view->priv->stdout,
                              &view->priv->stderr,
                              &error);

    if (error)
    {
        moo_line_view_write_line (MOO_LINE_VIEW (view),
                                  error->message, -1,
                                  view->priv->error_tag);
        g_error_free (error);
        goto out;
    }

    view->priv->running = TRUE;
    view->priv->cmd = g_strdup (cmd);

    view->priv->child_watch =
            g_child_watch_add (view->priv->pid,
                               (GChildWatchFunc) command_exit,
                               view);

    view->priv->stdout_io = g_io_channel_unix_new (view->priv->stdout);
    g_io_channel_set_encoding (view->priv->stdout_io, NULL, NULL);
    g_io_channel_set_buffered (view->priv->stdout_io, TRUE);
    g_io_channel_set_flags (view->priv->stdout_io, G_IO_FLAG_NONBLOCK, NULL);
    g_io_channel_set_close_on_unref (view->priv->stdout_io, TRUE);
    view->priv->stdout_watch =
            g_io_add_watch_full (view->priv->stdout_io,
                                 G_PRIORITY_DEFAULT_IDLE,
                                 G_IO_IN | G_IO_PRI | G_IO_ERR | G_IO_HUP,
                                 (GIOFunc) command_out, view,
                                 (GDestroyNotify) stdout_watch_removed);

    view->priv->stderr_io = g_io_channel_unix_new (view->priv->stderr);
    g_io_channel_set_encoding (view->priv->stderr_io, NULL, NULL);
    g_io_channel_set_buffered (view->priv->stderr_io, TRUE);
    g_io_channel_set_flags (view->priv->stderr_io, G_IO_FLAG_NONBLOCK, NULL);
    g_io_channel_set_close_on_unref (view->priv->stderr_io, TRUE);
    view->priv->stderr_watch =
            g_io_add_watch_full (view->priv->stderr_io,
                                 G_PRIORITY_DEFAULT_IDLE,
                                 G_IO_IN | G_IO_PRI | G_IO_ERR | G_IO_HUP,
                                 (GIOFunc) command_err, view,
                                 (GDestroyNotify) stderr_watch_removed);

    g_signal_emit (view, signals[JOB_STARTED], 0, job_name);

out:
    g_strfreev (argv);
    return result;
}


static void
moo_cmd_view_check_stop (MooCmdView *view)
{
    gboolean result;

    if (!view->priv->running)
        return;

    if (!view->priv->child_watch && !view->priv->stdout_watch && !view->priv->stderr_watch)
    {
        g_signal_emit (view, signals[CMD_EXIT], 0, view->priv->exit_status, &result);
        moo_cmd_view_cleanup (view);
    }
}


static gboolean
moo_cmd_view_cmd_exit (MooCmdView *view,
                       int         status)
{
    if (WIFEXITED (status))
    {
        guint8 exit_code = WEXITSTATUS (status);

        if (!exit_code)
        {
            moo_line_view_write_line (MOO_LINE_VIEW (view),
                                      "*** Done ***", -1,
                                      view->priv->message_tag);
        }
        else
        {
            char *msg = g_strdup_printf ("*** Failed with code %d ***",
                                         exit_code);
            moo_line_view_write_line (MOO_LINE_VIEW (view),
                                      msg, -1,
                                      view->priv->error_tag);
            g_free (msg);
        }
    }
#ifdef WCOREDUMP
    else if (WCOREDUMP (status))
    {
        moo_line_view_write_line (MOO_LINE_VIEW (view),
                                  "*** Dumped core ***", -1,
                                  view->priv->error_tag);
    }
#endif
    else if (WIFSIGNALED (status))
    {
        moo_line_view_write_line (MOO_LINE_VIEW (view),
                                  "*** Killed ***", -1,
                                  view->priv->error_tag);
    }
    else
    {
        moo_line_view_write_line (MOO_LINE_VIEW (view),
                                  "*** ??? ***", -1,
                                  view->priv->error_tag);
    }

    return FALSE;
}


static void
moo_cmd_view_cleanup (MooCmdView *view)
{
    if (!view->priv->running)
        return;

    view->priv->running = FALSE;

    if (view->priv->child_watch)
        g_source_remove (view->priv->child_watch);
    if (view->priv->stdout_watch)
        g_source_remove (view->priv->stdout_watch);
    if (view->priv->stderr_watch)
        g_source_remove (view->priv->stderr_watch);

    if (view->priv->stdout_io)
        g_io_channel_unref (view->priv->stdout_io);
    if (view->priv->stderr_io)
        g_io_channel_unref (view->priv->stderr_io);

    if (view->priv->pid)
    {
        kill (-view->priv->pid, SIGHUP);
        g_spawn_close_pid (view->priv->pid);
        view->priv->pid = 0;
    }

    g_free (view->priv->cmd);
    view->priv->cmd = NULL;
    view->priv->pid = 0;
    view->priv->stdout = -1;
    view->priv->stderr = -1;
    view->priv->child_watch = 0;
    view->priv->stdout_watch = 0;
    view->priv->stderr_watch = 0;
    view->priv->stdout_io = NULL;
    view->priv->stderr_io = NULL;

    g_signal_emit (view, signals[JOB_FINISHED], 0);
}


static gboolean
moo_cmd_view_abort_real (MooCmdView *view)
{
    if (!view->priv->running)
        return TRUE;

    g_return_val_if_fail (view->priv->pid != 0, TRUE);

    kill (-view->priv->pid, SIGHUP);

    if (view->priv->stdout_watch)
    {
        g_source_remove (view->priv->stdout_watch);
        view->priv->stdout_watch = 0;
    }

    if (view->priv->stderr_watch > 0)
    {
        g_source_remove (view->priv->stderr_watch);
        view->priv->stderr_watch = 0;
    }

    return TRUE;
}


void
moo_cmd_view_abort (MooCmdView *view)
{
    gboolean handled;
    g_return_if_fail (MOO_IS_CMD_VIEW (view));
    g_signal_emit (view, signals[ABORT], 0, &handled);
}


static gboolean
moo_cmd_view_stdout_line (MooCmdView *view,
                          const char *line)
{
    moo_line_view_write_line (MOO_LINE_VIEW (view), line, -1,
                              view->priv->stdout_tag);
    return FALSE;
}


static gboolean
moo_cmd_view_stderr_line (MooCmdView *view,
                          const char *line)
{
    moo_line_view_write_line (MOO_LINE_VIEW (view), line, -1,
                              view->priv->stderr_tag);
    return FALSE;
}
