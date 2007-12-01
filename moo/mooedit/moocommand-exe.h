/*
 *   moocommand-exe.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_COMMAND_EXE_H
#define MOO_COMMAND_EXE_H

#include <mooedit/mooeditwindow.h>

G_BEGIN_DECLS


void    _moo_edit_run_in_pane   (const char     *cmd_line,
                                 const char     *working_dir,
                                 char          **envp,
                                 MooEditWindow  *window,
                                 MooEdit        *doc);
void    _moo_edit_run_async     (const char     *cmd_line,
                                 const char     *working_dir,
                                 char          **envp,
                                 MooEditWindow  *window,
                                 MooEdit        *doc);
void    _moo_edit_run_sync      (const char     *cmd_line,
                                 const char     *working_dir,
                                 char          **envp,
                                 MooEditWindow  *window,
                                 MooEdit        *doc,
                                 const char     *input,
                                 int            *exit_status,
                                 char          **output,
                                 char          **output_err);


G_END_DECLS

#endif /* MOO_COMMAND_EXE_H */
