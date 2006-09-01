/*
 *   moooutputfilter.c
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

#include "mooedit/moooutputfilter.h"
#include "mooutils/moomarshals.h"
#include <string.h>


G_DEFINE_TYPE (MooOutputFilter, moo_output_filter, G_TYPE_OBJECT)

enum {
    STDOUT_LINE,
    STDERR_LINE,
    CMD_START,
    CMD_EXIT,
    N_SIGNALS
};

static guint signals[N_SIGNALS];


static void
moo_output_filter_class_init (MooOutputFilterClass *klass)
{
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
                      g_signal_accumulator_true_handled, NULL,
                      _moo_marshal_BOOL__VOID,
                      G_TYPE_BOOLEAN, 0);

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
moo_output_filter_init (G_GNUC_UNUSED MooOutputFilter *cmd)
{
}


void
moo_output_filter_set_view (MooOutputFilter *filter,
                            MooLineView     *view)
{
    g_return_if_fail (MOO_IS_OUTPUT_FILTER (filter));
    g_return_if_fail (!view || MOO_IS_LINE_VIEW (view));

    if (filter->view == view)
        return;

    if (filter->view)
    {
        if (MOO_OUTPUT_FILTER_GET_CLASS (filter)->detach)
            MOO_OUTPUT_FILTER_GET_CLASS (filter)->detach (filter);
    }

    filter->view = view;

    if (view)
    {
        if (MOO_OUTPUT_FILTER_GET_CLASS (filter)->attach)
            MOO_OUTPUT_FILTER_GET_CLASS (filter)->attach (filter);
    }
}


MooLineView *
moo_output_filter_get_view (MooOutputFilter *filter)
{
    g_return_val_if_fail (MOO_IS_OUTPUT_FILTER (filter), NULL);
    return filter->view;
}


static gboolean
moo_output_filter_output_line (MooOutputFilter *filter,
                               const char      *line,
                               guint            sig)
{
    gboolean result = FALSE;

    if (line[strlen(line) - 1] == '\n')
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
moo_output_filter_cmd_start (MooOutputFilter *filter)
{
    g_return_if_fail (MOO_IS_OUTPUT_FILTER (filter));
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


MooOutputFilter *
moo_output_filter_new (void)
{
    return g_object_new (MOO_TYPE_OUTPUT_FILTER, NULL);
}
