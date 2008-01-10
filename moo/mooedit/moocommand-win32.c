/*
 *   moocommand-win32.c
 *
 *   Copyright (C) 2004-2008 by Yevgen Muntyan <muntyan@math.tamu.edu>
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
#include "mooedit/moocommand-exe.h"

void
_moo_edit_run_in_pane (G_GNUC_UNUSED const char     *cmd_line,
                       G_GNUC_UNUSED const char     *working_dir,
                       G_GNUC_UNUSED char          **envp,
                       G_GNUC_UNUSED MooEditWindow  *window,
                       G_GNUC_UNUSED MooEdit        *doc)
{
    g_return_if_reached ();
}

void
_moo_edit_run_async (G_GNUC_UNUSED const char     *cmd_line,
                     G_GNUC_UNUSED const char     *working_dir,
                     G_GNUC_UNUSED char          **envp,
                     G_GNUC_UNUSED MooEditWindow  *window,
                     G_GNUC_UNUSED MooEdit        *doc)
{
    g_return_if_reached ();
}

void
_moo_edit_run_sync (G_GNUC_UNUSED const char     *cmd_line,
                    G_GNUC_UNUSED const char     *working_dir,
                    G_GNUC_UNUSED char          **envp,
                    G_GNUC_UNUSED MooEditWindow  *window,
                    G_GNUC_UNUSED MooEdit        *doc,
                    G_GNUC_UNUSED const char     *input,
                    G_GNUC_UNUSED int            *exit_status,
                    G_GNUC_UNUSED char          **output,
                    G_GNUC_UNUSED char          **output_err)
{
    if (exit_status)
        *exit_status = 1;
    if (output)
        *output = NULL;
    if (output_err)
        *output_err = g_strdup ("not implemented");
    g_return_if_reached ();
}
