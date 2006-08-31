/*
 *   moocommand-exe.h
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

#ifndef __MOO_COMMAND_EXE_H__
#define __MOO_COMMAND_EXE_H__

#include <mooedit/moocommand.h>

G_BEGIN_DECLS


#define MOO_TYPE_COMMAND_EXE                    (moo_command_exe_get_type ())
#define MOO_COMMAND_EXE(object)                 (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_COMMAND_EXE, MooCommandExe))
#define MOO_COMMAND_EXE_CLASS(klass)            (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_COMMAND_EXE, MooCommandExeClass))
#define MOO_IS_COMMAND_EXE(object)              (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_COMMAND_EXE))
#define MOO_IS_COMMAND_EXE_CLASS(klass)         (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_COMMAND_EXE))
#define MOO_COMMAND_EXE_GET_CLASS(obj)          (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_COMMAND_EXE, MooCommandExeClass))

typedef struct _MooCommandExe        MooCommandExe;
typedef struct _MooCommandExePrivate MooCommandExePrivate;
typedef struct _MooCommandExeClass   MooCommandExeClass;

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

struct _MooCommandExe {
    MooCommand base;
    MooCommandExePrivate *priv;
};

struct _MooCommandExeClass {
    MooCommandClass base_class;
};


GType       moo_command_exe_get_type    (void) G_GNUC_CONST;

MooCommand *moo_command_exe_new         (const char         *exe,
                                         MooCommandOptions   options,
                                         MooCommandExeInput  input,
                                         MooCommandExeInput  output);


G_END_DECLS

#endif /* __MOO_COMMAND_EXE_H__ */
