/*
 *   moocommand.h
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

#ifndef __MOO_COMMAND_H__
#define __MOO_COMMAND_H__

#include <mooscript/mooscript-node.h>

G_BEGIN_DECLS


#define MOO_TYPE_COMMAND                    (moo_command_get_type ())
#define MOO_COMMAND(object)                 (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_COMMAND, MooCommand))
#define MOO_COMMAND_CLASS(klass)            (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_COMMAND, MooCommandClass))
#define MOO_IS_COMMAND(object)              (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_COMMAND))
#define MOO_IS_COMMAND_CLASS(klass)         (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_COMMAND))
#define MOO_COMMAND_GET_CLASS(obj)          (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_COMMAND, MooCommandClass))

typedef struct _MooCommand      MooCommand;
typedef struct _MooCommandClass MooCommandClass;

typedef enum {
    MOO_COMMAND_SCRIPT = 1,
    MOO_COMMAND_PYTHON,
    MOO_COMMAND_SHELL
} MooCommandType;

struct _MooCommand {
    GObject object;

    MooCommandType type;
    gpointer window;

    MSContext *context;
    gpointer py_dict; /* PyObject* */
    char **shell_env;
    GHashTable *shell_vars;

    char *string;
    MSNode *script;
};

struct _MooCommandClass {
    GObjectClass object_class;
    void        (*run)          (MooCommand *cmd);
    gboolean    (*run_shell)    (MooCommand *cmd,
                                 const char *cmd_line);
};


GType       moo_command_get_type        (void) G_GNUC_CONST;

MooCommand *moo_command_new             (MooCommandType type);

void        moo_command_set_context     (MooCommand *cmd,
                                         MSContext  *ctx);
void        moo_command_set_py_dict     (MooCommand *cmd,
                                         gpointer    dict);
void        moo_command_set_shell_env   (MooCommand *cmd,
                                         char      **env);
void        moo_command_set_window      (MooCommand *cmd,
                                         gpointer    window);

void        moo_command_clear_shell_vars(MooCommand *cmd);
void        moo_command_set_shell_var   (MooCommand *cmd,
                                         const char *variable,
                                         const char *value);
const char *moo_command_get_shell_var   (MooCommand *cmd,
                                         const char *variable);

void        moo_command_set_script      (MooCommand *cmd,
                                         const char *script);
void        moo_command_set_python      (MooCommand *cmd,
                                         const char *script);
void        moo_command_set_shell       (MooCommand *cmd,
                                         const char *cmd_line);

void        moo_command_run             (MooCommand *cmd);


G_END_DECLS

#endif /* __MOO_COMMAND_H__ */
