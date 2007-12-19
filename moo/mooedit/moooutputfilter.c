/*
 *   moooutputfilter.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#include "mooedit/moooutputfilter.h"
#include "mooedit/mooeditor.h"
#include "mooutils/moomarshals.h"
#include "mooutils/mooutils-debug.h"
#include "mooutils/mooutils-misc.h"
#include <string.h>


struct _MooOutputFilterPrivate {
    MooLineView *view;
    char *active_dir;
    char *active_file;
    MooEditWindow *window;
};


G_DEFINE_TYPE (MooOutputFilter, moo_output_filter, G_TYPE_OBJECT)


static void moo_output_filter_open_file_line    (MooOutputFilter    *filter,
                                                 MooFileLineData    *data);


enum {
    STDOUT_LINE,
    STDERR_LINE,
    CMD_START,
    CMD_EXIT,
    N_SIGNALS
};

static guint signals[N_SIGNALS];


static void
moo_output_filter_finalize (GObject *object)
{
    MooOutputFilter *filter = MOO_OUTPUT_FILTER (object);

    g_free (filter->priv->active_dir);
    g_free (filter->priv->active_file);

    G_OBJECT_CLASS (moo_output_filter_parent_class)->finalize (object);
}


static void
moo_output_filter_activate (MooOutputFilter *filter,
                            int              line)
{
    MooFileLineData *data;

    data = moo_line_view_get_boxed (filter->priv->view, line, MOO_TYPE_FILE_LINE_DATA);

    if (data)
    {
        moo_output_filter_open_file_line (filter, data);
        moo_file_line_data_free (data);
    }
}


static void
moo_output_filter_class_init (MooOutputFilterClass *klass)
{
    G_OBJECT_CLASS (klass)->finalize = moo_output_filter_finalize;
    klass->activate = moo_output_filter_activate;

    g_type_class_add_private (klass, sizeof (MooOutputFilterPrivate));

    signals[STDOUT_LINE] =
        g_signal_new ("stdout-line",
                      G_OBJECT_CLASS_TYPE (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (MooOutputFilterClass, stdout_line),
                      g_signal_accumulator_true_handled, NULL,
                      _moo_marshal_BOOL__STRING,
                      G_TYPE_BOOLEAN, 1,
                      G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE);

    signals[STDERR_LINE] =
        g_signal_new ("stderr-line",
                      G_OBJECT_CLASS_TYPE (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (MooOutputFilterClass, stderr_line),
                      g_signal_accumulator_true_handled, NULL,
                      _moo_marshal_BOOL__STRING,
                      G_TYPE_BOOLEAN, 1,
                      G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE);

    signals[CMD_START] =
        g_signal_new ("cmd-start",
                      G_OBJECT_CLASS_TYPE (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (MooOutputFilterClass, cmd_start),
                      NULL, NULL,
                      _moo_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    signals[CMD_EXIT] =
        g_signal_new ("cmd-exit",
                      G_OBJECT_CLASS_TYPE (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (MooOutputFilterClass, cmd_exit),
                      g_signal_accumulator_true_handled, NULL,
                      _moo_marshal_BOOL__INT,
                      G_TYPE_BOOLEAN, 1,
                      G_TYPE_INT);
}


static void
moo_output_filter_init (MooOutputFilter *filter)
{
    filter->priv = G_TYPE_INSTANCE_GET_PRIVATE (filter, MOO_TYPE_OUTPUT_FILTER, MooOutputFilterPrivate);
}


static void
view_activate (MooLineView     *view,
               int              line,
               MooOutputFilter *filter)
{
    g_return_if_fail (MOO_IS_LINE_VIEW (view));
    g_return_if_fail (MOO_IS_OUTPUT_FILTER (filter));

    if (MOO_OUTPUT_FILTER_GET_CLASS (filter)->activate)
        MOO_OUTPUT_FILTER_GET_CLASS (filter)->activate (filter, line);
}


void
moo_output_filter_set_view (MooOutputFilter *filter,
                            MooLineView     *view)
{
    g_return_if_fail (MOO_IS_OUTPUT_FILTER (filter));
    g_return_if_fail (!view || MOO_IS_LINE_VIEW (view));

    if (filter->priv->view == view)
        return;

    if (filter->priv->view)
    {
        g_signal_handlers_disconnect_by_func (filter->priv->view,
                                              (gpointer) view_activate,
                                              filter);

        if (MOO_OUTPUT_FILTER_GET_CLASS (filter)->detach)
            MOO_OUTPUT_FILTER_GET_CLASS (filter)->detach (filter);
    }

    filter->priv->view = view;

    if (view)
    {
        g_signal_connect (view, "activate", G_CALLBACK (view_activate), filter);

        if (MOO_OUTPUT_FILTER_GET_CLASS (filter)->attach)
            MOO_OUTPUT_FILTER_GET_CLASS (filter)->attach (filter);
    }
}


MooLineView *
moo_output_filter_get_view (MooOutputFilter *filter)
{
    g_return_val_if_fail (MOO_IS_OUTPUT_FILTER (filter), NULL);
    return filter->priv->view;
}


static gboolean
moo_output_filter_output_line (MooOutputFilter *filter,
                               const char      *line,
                               guint            sig)
{
    gboolean result = FALSE;

    if (line[0] && line[1] && line[strlen(line) - 1] == '\n')
        g_warning ("%s: oops", G_STRLOC);

    g_signal_emit (filter, signals[sig], 0, line, &result);

    return result;
}


gboolean
moo_output_filter_stdout_line (MooOutputFilter *filter,
                               const char      *line)
{
    g_return_val_if_fail (MOO_IS_OUTPUT_FILTER (filter), FALSE);
    g_return_val_if_fail (line != NULL, FALSE);
    return moo_output_filter_output_line (filter, line, STDOUT_LINE);
}


gboolean
moo_output_filter_stderr_line (MooOutputFilter *filter,
                               const char      *line)
{
    g_return_val_if_fail (MOO_IS_OUTPUT_FILTER (filter), FALSE);
    g_return_val_if_fail (line != NULL, FALSE);
    return moo_output_filter_output_line (filter, line, STDERR_LINE);
}


void
moo_output_filter_cmd_start (MooOutputFilter *filter,
                             const char      *active_dir)
{
    g_return_if_fail (MOO_IS_OUTPUT_FILTER (filter));
    MOO_ASSIGN_STRING (filter->priv->active_dir, active_dir);
    g_signal_emit (filter, signals[CMD_START], 0);
}


gboolean
moo_output_filter_cmd_exit (MooOutputFilter *filter,
                            int              status)
{
    gboolean result = FALSE;

    g_return_val_if_fail (MOO_IS_OUTPUT_FILTER (filter), FALSE);

    g_signal_emit (filter, signals[CMD_EXIT], 0, status, &result);

    return result;
}


MooFileLineData *
moo_file_line_data_new (const char *file,
                        int         line,
                        int         character)
{
    MooFileLineData *data;

    data = g_new0 (MooFileLineData, 1);
    data->file = file && file[0] ? g_strdup (file) : NULL;
    data->line = line;
    data->character = character;

    return data;
}


static MooFileLineData *
moo_file_line_data_copy (MooFileLineData *data)
{
    MooFileLineData *copy = NULL;

    if (data)
    {
        copy = g_memdup (data, sizeof (MooFileLineData));
        copy->file = g_strdup (data->file);
    }

    return copy;
}


void
moo_file_line_data_free (MooFileLineData *data)
{
    if (data)
    {
        g_free (data->file);
        g_free (data);
    }
}


GType
moo_file_line_data_get_type (void)
{
    static GType type = 0;

    if (G_UNLIKELY (!type))
        type = g_boxed_type_register_static ("MooFileLineData",
                                             (GBoxedCopyFunc) moo_file_line_data_copy,
                                             (GBoxedFreeFunc) moo_file_line_data_free);

    return type;
}


const char *
moo_output_filter_get_active_file (MooOutputFilter *filter)
{
    g_return_val_if_fail (MOO_IS_OUTPUT_FILTER (filter), NULL);
    return filter->priv->active_file;
}


void
moo_output_filter_set_active_file (MooOutputFilter *filter,
                                   const char      *filename)
{
    g_return_if_fail (MOO_IS_OUTPUT_FILTER (filter));
    MOO_ASSIGN_STRING (filter->priv->active_file, filename);
}


const char *
moo_output_filter_get_active_dir (MooOutputFilter *filter)
{
    g_return_val_if_fail (MOO_IS_OUTPUT_FILTER (filter), NULL);
    return filter->priv->active_dir;
}


void
moo_output_filter_set_window (MooOutputFilter *filter,
                              gpointer         window)
{
    g_return_if_fail (MOO_IS_OUTPUT_FILTER (filter));
    g_return_if_fail (!window || MOO_IS_EDIT_WINDOW (window));
    filter->priv->window = window;
}

#if 0
gpointer
moo_output_filter_get_window (MooOutputFilter *filter)
{
    g_return_val_if_fail (MOO_IS_OUTPUT_FILTER (filter), NULL);
    return filter->priv->window;
}
#endif


static void
moo_output_filter_open_file_line (MooOutputFilter *filter,
                                  MooFileLineData *data)
{
    const char *filename;
    const char *path = NULL;
    char *freeme = NULL;

    g_return_if_fail (MOO_IS_OUTPUT_FILTER (filter));
    g_return_if_fail (data != NULL);

    filename = data->file ? data->file : filter->priv->active_file;
    g_return_if_fail (filename != NULL);

    if (g_path_is_absolute (filename))
    {
        path = filename;
    }
    else if (filter->priv->active_dir)
    {
        freeme = g_build_filename (filter->priv->active_dir, filename, NULL);
        path = freeme;
    }

    if (path)
    {
        if (g_file_test (path, G_FILE_TEST_EXISTS))
        {
            MooEditor *editor = moo_editor_instance ();
            moo_editor_open_file_line (editor, path, data->line, filter->priv->window);
        }
        else
        {
            _moo_message ("file '%s' does not exist", path);
        }
    }
    else
    {
        _moo_message ("could not find file '%s'", filename);
    }

    g_free (freeme);
}
