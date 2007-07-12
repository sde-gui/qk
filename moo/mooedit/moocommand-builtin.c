/*
 *   moocommand-builtin.c
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mooeditor.h"
#include "moocommand-builtin.h"
#include "mooutils/mootype-macros.h"
#include <string.h>

#define KEY_FUNC "func"
#define INDEX_FUNC 0

typedef void (*RunCommandFunc) (MooCommandContext *ctx);

struct _MooCommandBuiltinPrivate {
    RunCommandFunc func;
};

G_DEFINE_TYPE (MooCommandBuiltin, _moo_command_builtin, MOO_TYPE_COMMAND)

typedef MooCommandFactory MooCommandFactoryBuiltin;
typedef MooCommandFactoryClass MooCommandFactoryBuiltinClass;
MOO_DEFINE_TYPE_STATIC (MooCommandFactoryBuiltin, _moo_command_factory_builtin, MOO_TYPE_COMMAND_FACTORY)


static void switch_header_and_impl (MooCommandContext *ctx);


static void
_moo_command_builtin_init (MooCommandBuiltin *cmd)
{
    cmd->priv = G_TYPE_INSTANCE_GET_PRIVATE (cmd,
                                             MOO_TYPE_COMMAND_BUILTIN,
                                             MooCommandBuiltinPrivate);
}


static void
moo_command_builtin_run (MooCommand        *cmd_base,
                         MooCommandContext *ctx)
{
    MooCommandBuiltin *cmd = MOO_COMMAND_BUILTIN (cmd_base);
    g_return_if_fail (cmd->priv->func != NULL);
    cmd->priv->func (ctx);
}


static void
_moo_command_builtin_class_init (MooCommandBuiltinClass *klass)
{
    MooCommandFactory *factory;
    static const char *const data_keys[] = {KEY_FUNC, NULL};

    MOO_COMMAND_CLASS(klass)->run = moo_command_builtin_run;

    g_type_class_add_private (klass, sizeof (MooCommandBuiltinPrivate));

    factory = g_object_new (_moo_command_factory_builtin_get_type (), NULL);
    moo_command_factory_register ("builtin", "builtin", factory, (char**) data_keys);
    g_object_unref (factory);
}


static void
_moo_command_factory_builtin_init (G_GNUC_UNUSED MooCommandFactoryBuiltin *factory)
{
}


static MooCommand *
factory_create_command (G_GNUC_UNUSED MooCommandFactory *factory,
                        MooCommandData *data,
                        const char     *options)
{
    MooCommandBuiltin *cmd;
    const char *funcname;
    RunCommandFunc func = NULL;

    funcname = moo_command_data_get (data, INDEX_FUNC);
    g_return_val_if_fail (funcname && funcname[0], NULL);

    if (!strcmp (funcname, "switch-header-and-impl"))
        func = switch_header_and_impl;

    if (!func)
    {
        g_critical ("unknown function `%s'", funcname);
        return NULL;
    }

    cmd = g_object_new (MOO_TYPE_COMMAND_BUILTIN, "options",
                        moo_command_options_parse (options), NULL);
    cmd->priv->func = func;

    return MOO_COMMAND (cmd);
}


static gboolean
factory_data_equal (G_GNUC_UNUSED MooCommandFactory *factory,
                    MooCommandData *data1,
                    MooCommandData *data2)
{
    const char *val1 = moo_command_data_get (data1, INDEX_FUNC);
    const char *val2 = moo_command_data_get (data2, INDEX_FUNC);
    return val1 ? (val2 && !strcmp (val1, val2)) : (!val2);
}


static void
_moo_command_factory_builtin_class_init (MooCommandFactoryBuiltinClass *klass)
{
    klass->create_command = factory_create_command;
    klass->data_equal = factory_data_equal;
}


static gboolean
find_ext (const char *extension,
          char      **list)
{
    for (; *list; ++list)
        if (!strcmp (extension, *list))
            return TRUE;
    return FALSE;
}

static void
switch_header_and_impl (MooCommandContext *ctx)
{
    const char *doc_ext, *doc_dir, *doc_base;
    char **new_ext;
    static const char *const c_h[] = {".h", ".hh", ".hpp", ".hxx", ".H", NULL};
    static const char *const c_impl[] = {".c", ".cc", ".cpp", ".cxx", ".C", ".m", NULL};
    static const char *const gap_h[] = {".gd", NULL};
    static const char *const gap_impl[] = {".gi", NULL};
    static char **pairs[] = {(char**) c_h, (char**) c_impl, (char**) gap_h, (char**) gap_impl};
    guint i;

    doc_ext = moo_command_context_get_string (ctx, "DOC_EXT");
    doc_dir = moo_command_context_get_string (ctx, "DOC_DIR");
    doc_base = moo_command_context_get_string (ctx, "DOC_BASE");
    g_return_if_fail (doc_ext && doc_dir && doc_base);

    new_ext = NULL;

    for (i = 0; !new_ext && i < G_N_ELEMENTS (pairs)/2; ++i)
    {
        if (find_ext (doc_ext, pairs[2*i]))
            new_ext = pairs[2*i+1];
        else if (find_ext (doc_ext, pairs[2*i+1]))
            new_ext = pairs[2*i];
    }

    if (!new_ext)
        return;

    for (; *new_ext; ++new_ext)
    {
        char *filename = g_strdup_printf ("%s" G_DIR_SEPARATOR_S "%s%s",
                                          doc_dir, doc_base, *new_ext);

        if (g_file_test (filename, G_FILE_TEST_EXISTS))
        {
            moo_editor_open_file (moo_editor_instance (),
                                  moo_command_context_get_window (ctx),
                                  NULL, filename, NULL);
            g_free (filename);
            return;
        }

        g_free (filename);
    }
}
