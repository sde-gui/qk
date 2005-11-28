/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   moocmd.h
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

#ifndef __MOO_CMD_H__
#define __MOO_CMD_H__

#include <glib-object.h>

G_BEGIN_DECLS


#define MOO_TYPE_CMD              (moo_cmd_get_type ())
#define MOO_CMD(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_CMD, MooCmd))
#define MOO_CMD_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_CMD, MooCmdClass))
#define MOO_IS_CMD(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_CMD))
#define MOO_IS_CMD_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_CMD))
#define MOO_CMD_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_CMD, MooCmdClass))


typedef struct _MooCmd         MooCmd;
typedef struct _MooCmdPrivate  MooCmdPrivate;
typedef struct _MooCmdClass    MooCmdClass;

typedef enum {
    MOO_CMD_COLLECT_STDOUT      = 1 << 0,
    MOO_CMD_COLLECT_STDERR      = 1 << 1,
    MOO_CMD_STDOUT_TO_PARENT    = 1 << 2,
    MOO_CMD_STDERR_TO_PARENT    = 1 << 3,
    MOO_CMD_UTF8_OUTPUT         = 1 << 4
} MooCmdFlags;

struct _MooCmd
{
    GObject object;

    GString *out_buffer;
    GString *err_buffer;
    MooCmdPrivate *priv;
};

struct _MooCmdClass
{
    GObjectClass object_class;

    /* action signal */
    gboolean (*abort)       (MooCmd     *cmd);

    gboolean (*cmd_exit)    (MooCmd     *cmd,
                             int         status);
    gboolean (*stdout_text) (MooCmd     *cmd,
                             const char *text);
    gboolean (*stderr_text) (MooCmd     *cmd,
                             const char *text);
};


GType       moo_cmd_get_type        (void) G_GNUC_CONST;

MooCmd     *moo_cmd_new_full        (const char *working_dir,
                                     char      **argv,
                                     char      **envp,
                                     GSpawnFlags flags,
                                     MooCmdFlags cmd_flags,
                                     GSpawnChildSetupFunc child_setup,
                                     gpointer    user_data,
                                     GError    **error);

void        moo_cmd_abort           (MooCmd     *cmd);


G_END_DECLS

#endif /* __MOO_CMD_H__ */
