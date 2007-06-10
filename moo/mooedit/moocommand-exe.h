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

#ifndef MOO_COMMAND_EXE_H
#define MOO_COMMAND_EXE_H

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

struct _MooCommandExe {
    MooCommand base;
    MooCommandExePrivate *priv;
};

struct _MooCommandExeClass {
    MooCommandClass base_class;
};


GType       moo_command_exe_get_type        (void) G_GNUC_CONST;


G_END_DECLS

#endif /* MOO_COMMAND_EXE_H */
