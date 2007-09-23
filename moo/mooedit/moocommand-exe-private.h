/*
 *   moocommand-exe-private.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_COMMAND_EXE_PRIVATE_H
#define MOO_COMMAND_EXE_PRIVATE_H

#include <mooedit/moocommand-exe.h>

G_BEGIN_DECLS


typedef enum
{
    MOO_COMMAND_EXE_INPUT_NONE,
    MOO_COMMAND_EXE_INPUT_LINES,
    MOO_COMMAND_EXE_INPUT_SELECTION,
    MOO_COMMAND_EXE_INPUT_DOC
} MooCommandExeInput;

typedef enum
{
    MOO_COMMAND_EXE_OUTPUT_NONE,
    MOO_COMMAND_EXE_OUTPUT_NONE_ASYNC,
    MOO_COMMAND_EXE_OUTPUT_PANE,
    MOO_COMMAND_EXE_OUTPUT_INSERT,
    MOO_COMMAND_EXE_OUTPUT_NEW_DOC
} MooCommandExeOutput;


G_END_DECLS

#endif /* MOO_COMMAND_EXE_PRIVATE_H */
