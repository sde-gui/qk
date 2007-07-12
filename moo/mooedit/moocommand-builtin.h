/*
 *   moocommand-builtin.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_COMMAND_BUILTIN_H
#define MOO_COMMAND_BUILTIN_H

#include <mooedit/moocommand.h>

G_BEGIN_DECLS


#define MOO_TYPE_COMMAND_BUILTIN            (_moo_command_builtin_get_type ())
#define MOO_COMMAND_BUILTIN(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_COMMAND_BUILTIN, MooCommandBuiltin))
#define MOO_COMMAND_BUILTIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_COMMAND_BUILTIN, MooCommandBuiltinClass))
#define MOO_IS_COMMAND_BUILTIN(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_COMMAND_BUILTIN))
#define MOO_IS_COMMAND_BUILTIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_COMMAND_BUILTIN))
#define MOO_COMMAND_BUILTIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_COMMAND_BUILTIN, MooCommandBuiltinClass))

typedef struct _MooCommandBuiltin        MooCommandBuiltin;
typedef struct _MooCommandBuiltinPrivate MooCommandBuiltinPrivate;
typedef struct _MooCommandBuiltinClass   MooCommandBuiltinClass;

struct _MooCommandBuiltin {
    MooCommand base;
    MooCommandBuiltinPrivate *priv;
};

struct _MooCommandBuiltinClass {
    MooCommandClass base_class;
};


GType _moo_command_builtin_get_type (void) G_GNUC_CONST;


G_END_DECLS

#endif /* MOO_COMMAND_BUILTIN_H */
