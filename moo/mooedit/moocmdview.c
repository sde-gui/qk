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
#include "mooutils/moocmd.h"
#include <sys/wait.h>
#include <unistd.h>


struct _MooCmdViewPrivate {
    MooCmd *cmd;
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

    if (view->priv->cmd)
    {
        moo_cmd_abort (view->priv->cmd);
        g_object_unref (view->priv->cmd);
        view->priv->cmd = NULL;
    }

    if (GTK_OBJECT_CLASS (moo_cmd_view_parent_class)->destroy)
        GTK_OBJECT_CLASS (moo_cmd_view_parent_class)->destroy (object);
}


GtkWidget*
moo_cmd_view_new (void)
{
    return g_object_new (MOO_TYPE_CMD_VIEW, NULL);
}


static gboolean
cmd_exit_cb (MooCmd     *cmd,
             int         status,
             MooCmdView *view)
{
    gboolean result = FALSE;

    g_return_val_if_fail (cmd == view->priv->cmd, FALSE);
    g_signal_emit (view, signals[CMD_EXIT], 0, status, &result);

    g_signal_emit (view, signals[JOB_FINISHED], 0);
    g_object_unref (cmd);
    view->priv->cmd = NULL;

    return result;
}


static gboolean
stdout_text_cb (MooCmd     *cmd,
                const char *text,
                MooCmdView *view)
{
    gboolean result = FALSE;
    g_return_val_if_fail (cmd == view->priv->cmd, FALSE);
    g_signal_emit (view, signals[STDOUT_LINE], 0, text, &result);
    return result;
}


static gboolean
stderr_text_cb (MooCmd     *cmd,
                const char *text,
                MooCmdView *view)
{
    gboolean result = FALSE;
    g_return_val_if_fail (cmd == view->priv->cmd, FALSE);
    g_signal_emit (view, signals[STDERR_LINE], 0, text, &result);
    return result;
}


gboolean
moo_cmd_view_run_command (MooCmdView *view,
                          const char *cmd,
                          const char *job_name)
{
    GError *error = NULL;
    char **argv = NULL;
    gboolean result = FALSE;

    g_return_val_if_fail (MOO_IS_CMD_VIEW (view), FALSE);
    g_return_val_if_fail (cmd && cmd[0], FALSE);

    g_return_val_if_fail (!view->priv->cmd, FALSE);

    moo_line_view_write_line (MOO_LINE_VIEW (view), cmd, -1,
                              view->priv->message_tag);

    argv = g_new (char*, 4);
    argv[0] = g_strdup ("/bin/sh");
    argv[1] = g_strdup ("-c");
    argv[2] = g_strdup (cmd);
    argv[3] = NULL;

    view->priv->cmd = moo_cmd_new_full (NULL, argv, NULL,
                                        G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD,
                                        MOO_CMD_UTF8_OUTPUT,
                                        NULL, NULL,
                                        &error);

    if (error)
    {
        moo_line_view_write_line (MOO_LINE_VIEW (view),
                                  error->message, -1,
                                  view->priv->error_tag);
        g_error_free (error);
        goto out;
    }

    g_signal_connect (view->priv->cmd, "cmd-exit", G_CALLBACK (cmd_exit_cb), view);
    g_signal_connect (view->priv->cmd, "stdout-text", G_CALLBACK (stdout_text_cb), view);
    g_signal_connect (view->priv->cmd, "stderr-text", G_CALLBACK (stderr_text_cb), view);

    result = TRUE;
    g_signal_emit (view, signals[JOB_STARTED], 0, job_name);

out:
    g_strfreev (argv);
    return result;
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
            char *msg;

            if (exit_code > 128)
            {
                static const char *signals[] = {
                    NULL, "SIGHUP", "SIGINT", "SIGQUIT", "SIGILL",
                    "SIGTRAP", "SIGABRT", "SIGBUS", "SIGFPE",
                    "SIGKILL", "SIGUSR", "SIGSEGV", "SIGUSR",
                    "SIGPIPE", "SIGALRM", "SIGTERM", "SIGSTKFLT",
                    "SIGCHLD", "SIGCONT", "SIGSTOP", "SIGTSTP",
                    "SIGTTIN", "SIGTTOU", "SIGURG", "SIGXCPU",
                    "SIGXFSZ", "SIGVTALRM", "SIGPROF", "SIGWINCH",
                    "SIGIO", "SIGPWR", "SIGSYS"
                };

                int sig = exit_code - 128;

                if (sig > 0 && sig < (int) G_N_ELEMENTS (signals))
                    msg = g_strdup_printf ("*** Killed by signal %d (%s) ***",
                                           exit_code - 128, signals[sig]);
                else
                    msg = g_strdup_printf ("*** Killed by signal %d ***",
                                           exit_code - 128);
            }
            else
                msg = g_strdup_printf ("*** Exited with status %d ***",
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


static gboolean
moo_cmd_view_abort_real (MooCmdView *view)
{
    if (view->priv->cmd)
        moo_cmd_abort (view->priv->cmd);
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
