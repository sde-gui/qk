/*
 *   moocommand.c
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

#include "mooedit/moocommand-script.h"
#include "mooedit/moocommand-exe.h"
#include "mooedit/mooeditwindow.h"
#include <gtk/gtkwindow.h>
#include <gtk/gtktextview.h>
#include <string.h>


G_DEFINE_TYPE (MooCommand, moo_command, G_TYPE_OBJECT)
G_DEFINE_TYPE (MooCommandContext, moo_command_context, G_TYPE_OBJECT)

enum {
    CTX_PROP_0,
    CTX_PROP_DOC,
    CTX_PROP_WINDOW
};

enum {
    CMD_PROP_0,
    CMD_PROP_FLAGS
};

typedef struct {
    GValue value;
} Variable;

struct _MooCommandContextPrivate {
    GHashTable *vars;
    gpointer window;
    gpointer doc;
};

typedef struct {
    MooCommandFactoryFunc func;
    gpointer data;
    GDestroyNotify notify;
} CommandType;

struct _MooCommandData {
    guint ref_count;
    GHashTable *hash;
};


static GHashTable *registered_types;


static Variable *variable_new   (const GValue   *value);
static void      variable_free  (Variable       *var);


MooCommand *
moo_command_create (const char     *name,
                    MooCommandData *data)
{
    CommandType *type;
    MooCommand *cmd;

    g_return_val_if_fail (registered_types != NULL, NULL);

    if (data)
        moo_command_data_ref (data);
    else
        data = moo_command_data_new ();

    type = g_hash_table_lookup (registered_types, name);
    g_return_val_if_fail (type != NULL, NULL);

    cmd = type->func (data, type->data);

    moo_command_data_unref (data);
    return cmd;
}


void
moo_command_register (const char         *name,
                      MooCommandFactoryFunc func,
                      gpointer            user_data,
                      GDestroyNotify      notify)
{
    CommandType *type;

    g_return_if_fail (name != NULL);
    g_return_if_fail (func != NULL);

    if (registered_types != NULL)
    {
        CommandType *old = g_hash_table_lookup (registered_types, name);

        if (old != NULL)
        {
            g_warning ("reregistering command type '%s'", name);
            if (old->notify)
                old->notify (old->data);
            g_free (old);
        }
    }

    if (!registered_types)
        registered_types = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

    type = g_new0 (CommandType, 1);
    type->func = func;
    type->data = user_data;
    type->notify = notify;

    g_hash_table_insert (registered_types, g_strdup (name), type);
}


gboolean
moo_command_registered (const char *name)
{
    g_return_val_if_fail (name != NULL, FALSE);
    return registered_types != NULL &&
            g_hash_table_lookup (registered_types, name) != NULL;
}


static void
moo_command_set_property (GObject *object,
                          guint property_id,
                          const GValue *value,
                          GParamSpec *pspec)
{
    MooCommand *cmd = MOO_COMMAND (object);

    switch (property_id)
    {
        case CMD_PROP_FLAGS:
            moo_command_set_flags (cmd, g_value_get_flags (value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}


static void
moo_command_get_property (GObject *object,
                          guint property_id,
                          GValue *value,
                          GParamSpec *pspec)
{
    MooCommand *cmd = MOO_COMMAND (object);

    switch (property_id)
    {
        case CMD_PROP_FLAGS:
            g_value_set_flags (value, cmd->flags);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}


static gboolean
moo_command_check_sensitive_real (MooCommand *cmd,
                                  gpointer    doc,
                                  gpointer    window)
{
    if ((cmd->flags & MOO_COMMAND_NEED_WINDOW) && !MOO_IS_EDIT_WINDOW (window))
        return FALSE;

    if ((cmd->flags & MOO_COMMAND_NEED_DOC) && !doc)
        return FALSE;

    if ((cmd->flags & MOO_COMMAND_NEED_FILE) && (!MOO_IS_EDIT (doc) || !moo_edit_get_filename (doc)))
        return FALSE;

    return TRUE;
}


static gboolean
moo_command_check_context_real (MooCommand        *cmd,
                                MooCommandContext *ctx)
{
    gpointer doc, window;
    doc = moo_command_context_get_doc (ctx);
    window = moo_command_context_get_window (ctx);
    return moo_command_check_sensitive (cmd, doc, window);
}


static void
moo_command_class_init (MooCommandClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->set_property = moo_command_set_property;
    object_class->get_property = moo_command_get_property;

    klass->check_context = moo_command_check_context_real;
    klass->check_sensitive = moo_command_check_sensitive_real;

    g_object_class_install_property (object_class, CMD_PROP_FLAGS,
                                     g_param_spec_flags ("flags", "flags", "flags",
                                                         MOO_TYPE_COMMAND_FLAGS,
                                                         0,
                                                         G_PARAM_READWRITE));
}


static void
moo_command_init (G_GNUC_UNUSED MooCommand *cmd)
{
}


static MooCommandFlags
check_flags (MooCommandFlags flags)
{
    MooCommandFlags checked = flags;

    if (flags & MOO_COMMAND_NEED_FILE)
        checked |= MOO_COMMAND_NEED_DOC;
    if (flags & MOO_COMMAND_NEED_SAVE)
        checked |= MOO_COMMAND_NEED_DOC;

    return checked;
}


void
moo_command_run (MooCommand         *cmd,
                 MooCommandContext  *ctx)
{
    gpointer doc, window;

    g_return_if_fail (MOO_IS_COMMAND (cmd));
    g_return_if_fail (MOO_IS_COMMAND_CONTEXT (ctx));
    g_return_if_fail (MOO_COMMAND_GET_CLASS(cmd)->run != NULL);

    doc = moo_command_context_get_doc (ctx);
    window = moo_command_context_get_window (ctx);

    if (cmd->flags & MOO_COMMAND_NEED_WINDOW)
        g_return_if_fail (MOO_IS_EDIT_WINDOW (window));

    if (cmd->flags & MOO_COMMAND_NEED_DOC)
        g_return_if_fail (doc != NULL);

    if (cmd->flags & MOO_COMMAND_NEED_FILE)
        g_return_if_fail (MOO_IS_EDIT (doc) && moo_edit_get_filename (doc) != NULL);

    if (cmd->flags & MOO_COMMAND_NEED_SAVE)
    {
        if (MOO_EDIT_IS_MODIFIED (doc) && !moo_edit_save (doc, NULL))
            return;
    }

    MOO_COMMAND_GET_CLASS(cmd)->run (cmd, ctx);
}


gboolean
moo_command_check_context (MooCommand        *cmd,
                           MooCommandContext *ctx)
{
    g_return_val_if_fail (MOO_IS_COMMAND (cmd), FALSE);
    g_return_val_if_fail (MOO_IS_COMMAND_CONTEXT (ctx), FALSE);
    g_return_val_if_fail (MOO_COMMAND_GET_CLASS(cmd)->check_context != NULL, FALSE);

    return MOO_COMMAND_GET_CLASS(cmd)->check_context (cmd, ctx);
}


gboolean
moo_command_check_sensitive (MooCommand *cmd,
                             gpointer    doc,
                             gpointer    window)
{
    g_return_val_if_fail (MOO_IS_COMMAND (cmd), FALSE);
    g_return_val_if_fail (!doc || GTK_IS_TEXT_VIEW (doc), FALSE);
    g_return_val_if_fail (!window || GTK_IS_WINDOW (window), FALSE);
    g_return_val_if_fail (MOO_COMMAND_GET_CLASS(cmd)->check_sensitive != NULL, FALSE);
    return MOO_COMMAND_GET_CLASS(cmd)->check_sensitive (cmd, doc, window);
}


void
moo_command_set_flags (MooCommand      *cmd,
                       MooCommandFlags  flags)
{
    g_return_if_fail (MOO_IS_COMMAND (cmd));

    flags = check_flags (flags);

    if (flags != cmd->flags)
    {
        cmd->flags = flags;
        g_object_notify (G_OBJECT (cmd), "flags");
    }
}


MooCommandFlags
moo_command_get_flags (MooCommand *cmd)
{
    g_return_val_if_fail (MOO_IS_COMMAND (cmd), 0);
    return cmd->flags;
}


GType
moo_command_flags_get_type (void)
{
    static GType type;

    if (!type)
    {
        static GFlagsValue values[] = {
            { MOO_COMMAND_NEED_DOC, (char*) "MOO_COMMAND_NEED_DOC", (char*) "need-doc" },
            { MOO_COMMAND_NEED_FILE, (char*) "MOO_COMMAND_NEED_FILE", (char*) "need-file" },
            { MOO_COMMAND_NEED_SAVE, (char*) "MOO_COMMAND_NEED_SAVE", (char*) "need-save" },
            { MOO_COMMAND_NEED_WINDOW, (char*) "MOO_COMMAND_NEED_WINDOW", (char*) "need-window" },
            { 0, NULL, NULL }
        };

        type = g_flags_register_static ("MooCommandFlags", values);
    }

    return type;
}


MooCommandFlags
moo_command_flags_parse (const char *string)
{
    MooCommandFlags flags = 0;
    char **pieces, **p;

    if (!string)
        return 0;

    pieces = g_strsplit (string, ";", 0);

    if (!pieces || !pieces[0])
        goto out;

    for (p = pieces; *p != NULL; ++p)
    {
        char *s = *p;

        g_strdelimit (g_strstrip (s), "_", '-');

        if (!*s)
            continue;

        if (!strcmp (s, "need-doc"))
            flags |= MOO_COMMAND_NEED_DOC;
        else if (!strcmp (s, "need-file"))
            flags |= MOO_COMMAND_NEED_FILE;
        else if (!strcmp (s, "need-save"))
            flags |= MOO_COMMAND_NEED_SAVE;
        else if (!strcmp (s, "need-window"))
            flags |= MOO_COMMAND_NEED_WINDOW;
        else
            g_warning ("unknown flag '%s'", s);
    }

out:
    g_strfreev (pieces);
    return flags;
}


void
moo_command_load_data (MooCommand     *cmd,
                       MooCommandData *data)
{
    const char *flags;

    g_return_if_fail (MOO_IS_COMMAND (cmd));
    g_return_if_fail (data != NULL);

    flags = moo_command_data_get (data, "flags");
    moo_command_set_flags (cmd, moo_command_flags_parse (flags));
}


static Variable *
variable_new (const GValue *value)
{
    Variable *var;
    g_return_val_if_fail (G_IS_VALUE (value), NULL);
    var = g_new0 (Variable, 1);
    g_value_init (&var->value, G_VALUE_TYPE (value));
    g_value_copy (value, &var->value);
    return var;
}


static void
variable_free (Variable *var)
{
    if (var)
    {
        g_value_unset (&var->value);
        g_free (var);
    }
}


void
moo_command_context_set (MooCommandContext  *ctx,
                         const char         *name,
                         const GValue       *value)
{
    Variable *var;

    g_return_if_fail (MOO_IS_COMMAND_CONTEXT (ctx));
    g_return_if_fail (name != NULL);
    g_return_if_fail (G_IS_VALUE (value));

    var = variable_new (value);
    g_hash_table_insert (ctx->priv->vars, g_strdup (name), var);
}


gboolean
moo_command_context_get (MooCommandContext  *ctx,
                         const char         *name,
                         GValue             *value)
{
    Variable *var;

    g_return_val_if_fail (MOO_IS_COMMAND_CONTEXT (ctx), FALSE);
    g_return_val_if_fail (name != NULL, FALSE);
    g_return_val_if_fail (value != NULL, FALSE);

    var = g_hash_table_lookup (ctx->priv->vars, name);

    if (!var)
        return FALSE;

    g_value_init (value, G_VALUE_TYPE (&var->value));
    g_value_copy (&var->value, value);
    return TRUE;
}


void
moo_command_context_unset (MooCommandContext  *ctx,
                           const char         *name)
{
    g_return_if_fail (MOO_IS_COMMAND_CONTEXT (ctx));
    g_return_if_fail (name != NULL);
    g_hash_table_remove (ctx->priv->vars, name);
}


static void
moo_command_context_dispose (GObject *object)
{
    MooCommandContext *ctx = MOO_COMMAND_CONTEXT (object);

    if (ctx->priv)
    {
        g_hash_table_destroy (ctx->priv->vars);
        if (ctx->priv->window)
            g_object_unref (ctx->priv->window);
        if (ctx->priv->doc)
            g_object_unref (ctx->priv->doc);
        g_free (ctx->priv);
        ctx->priv = NULL;
    }

    G_OBJECT_CLASS (moo_command_context_parent_class)->dispose (object);
}


static void
moo_command_context_set_property (GObject *object,
                                  guint property_id,
                                  const GValue *value,
                                  GParamSpec *pspec)
{
    MooCommandContext *ctx = MOO_COMMAND_CONTEXT (object);

    switch (property_id)
    {
        case CTX_PROP_DOC:
            moo_command_context_set_doc (ctx, g_value_get_object (value));
            break;
        case CTX_PROP_WINDOW:
            moo_command_context_set_window (ctx, g_value_get_object (value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}


static void
moo_command_context_get_property (GObject *object,
                                  guint property_id,
                                  GValue *value,
                                  GParamSpec *pspec)
{
    MooCommandContext *ctx = MOO_COMMAND_CONTEXT (object);

    switch (property_id)
    {
        case CTX_PROP_DOC:
            g_value_set_object (value, ctx->priv->doc);
            break;
        case CTX_PROP_WINDOW:
            g_value_set_object (value, ctx->priv->window);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}


static void
moo_command_context_class_init (MooCommandContextClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = moo_command_context_dispose;
    object_class->set_property = moo_command_context_set_property;
    object_class->get_property = moo_command_context_get_property;

    g_object_class_install_property (object_class, CTX_PROP_DOC,
                                     g_param_spec_object ("doc", "doc", "doc",
                                                          GTK_TYPE_TEXT_VIEW,
                                                          G_PARAM_READWRITE));

    g_object_class_install_property (object_class, CTX_PROP_WINDOW,
                                     g_param_spec_object ("window", "window", "window",
                                                          GTK_TYPE_WINDOW,
                                                          G_PARAM_READWRITE));
}


static void
moo_command_context_init (MooCommandContext *ctx)
{
    ctx->priv = g_new0 (MooCommandContextPrivate, 1);
    ctx->priv->vars = g_hash_table_new_full (g_str_hash,
                                             g_str_equal,
                                             g_free,
                                             (GDestroyNotify) variable_free);
}


static void
ctx_foreach_func (const char *name,
                  Variable   *var,
                  gpointer    user_data)
{
    struct {
        MooCommandContextForeachFunc func;
        gpointer data;
    } *data = user_data;

    data->func (name, &var->value, data->data);
}

void
moo_command_context_foreach (MooCommandContext  *ctx,
                             MooCommandContextForeachFunc func,
                             gpointer            func_data)
{
    struct {
        MooCommandContextForeachFunc func;
        gpointer data;
    } data;

    g_return_if_fail (MOO_IS_COMMAND_CONTEXT (ctx));
    g_return_if_fail (func != NULL);

    data.func = func;
    data.data = func_data;
    g_hash_table_foreach (ctx->priv->vars, (GHFunc) ctx_foreach_func, &data);
}


MooCommandContext *
moo_command_context_new (gpointer doc,
                         gpointer window)
{
    g_return_val_if_fail (!doc || GTK_IS_TEXT_VIEW (doc), NULL);
    g_return_val_if_fail (!window || GTK_IS_WINDOW (window), NULL);

    return g_object_new (MOO_TYPE_COMMAND_CONTEXT,
                         "doc", doc,
                         "window", window,
                         NULL);
}


void
moo_command_context_set_doc (MooCommandContext *ctx,
                             gpointer           doc)
{
    g_return_if_fail (MOO_IS_COMMAND_CONTEXT (ctx));
    g_return_if_fail (!doc || GTK_IS_TEXT_VIEW (doc));

    if (ctx->priv->doc != doc)
    {
        if (ctx->priv->doc)
            g_object_unref (ctx->priv->doc);
        if (doc)
            g_object_ref (doc);
        ctx->priv->doc = doc;
        g_object_notify (G_OBJECT (ctx), "doc");
    }
}


void
moo_command_context_set_window (MooCommandContext *ctx,
                                gpointer           window)
{
    g_return_if_fail (MOO_IS_COMMAND_CONTEXT (ctx));
    g_return_if_fail (!window || GTK_IS_WINDOW (window));

    if (ctx->priv->window != window)
    {
        if (ctx->priv->window)
            g_object_unref (ctx->priv->window);
        if (window)
            g_object_ref (window);
        ctx->priv->window = window;
        g_object_notify (G_OBJECT (ctx), "window");
    }
}


gpointer
moo_command_context_get_doc (MooCommandContext *ctx)
{
    g_return_val_if_fail (MOO_IS_COMMAND_CONTEXT (ctx), NULL);
    return ctx->priv->doc;
}


gpointer
moo_command_context_get_window (MooCommandContext *ctx)
{
    g_return_val_if_fail (MOO_IS_COMMAND_CONTEXT (ctx), NULL);
    return ctx->priv->window;
}


MooCommandData *
moo_command_data_new (void)
{
    MooCommandData *data = g_new0 (MooCommandData, 1);
    data->ref_count = 1;
    data->hash = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
    return data;
}


MooCommand *
_moo_command_parse_markup (MooMarkupNode *node)
{
    MooCommand *cmd;
    MooCommandData *data;
    MooMarkupNode *child;
    const char *type;

    g_return_val_if_fail (MOO_MARKUP_IS_ELEMENT (node), NULL);
    g_return_val_if_fail (!strcmp (node->name, "command"), NULL);

    type = moo_markup_get_prop (node, "type");

    if (!type)
    {
        g_warning ("no type attribute in %s element", node->name);
        return NULL;
    }

    if (!moo_command_registered (type))
    {
        g_warning ("unknown command type %s", type);
        return NULL;
    }

    data = moo_command_data_new ();

    for (child = node->children; child != NULL; child = child->next)
    {
        if (!MOO_MARKUP_IS_ELEMENT (child))
            continue;

        if (moo_command_data_get (data, child->name))
        {
            g_warning ("duplicated %s element", child->name);
            moo_command_data_unref (data);
            return NULL;
        }

        moo_command_data_set (data, child->name, moo_markup_get_content (child));
    }

    cmd = moo_command_create (type, data);

    moo_command_data_unref (data);
    return cmd;
}


MooCommandData *
moo_command_data_ref (MooCommandData *data)
{
    g_return_val_if_fail (data != NULL, NULL);
    data->ref_count++;
    return data;
}


void
moo_command_data_unref (MooCommandData *data)
{
    g_return_if_fail (data != NULL);

    if (!--data->ref_count)
    {
        g_hash_table_destroy (data->hash);
        g_free (data);
    }
}


void
moo_command_data_set (MooCommandData *data,
                      const char     *key,
                      const char     *value)
{
    g_return_if_fail (data != NULL);
    g_return_if_fail (key != NULL);
    g_return_if_fail (value != NULL);

    g_hash_table_insert (data->hash, g_strdup (key), g_strdup (value));
}


const char *
moo_command_data_get (MooCommandData *data,
                      const char     *key)
{
    g_return_val_if_fail (data != NULL, NULL);
    g_return_val_if_fail (key != NULL, NULL);

    return g_hash_table_lookup (data->hash, key);
}


void
moo_command_data_unset (MooCommandData *data,
                        const char     *key)
{
    g_return_if_fail (data != NULL);
    g_return_if_fail (key != NULL);

    g_hash_table_remove (data->hash, key);
}


GType
moo_command_data_get_type (void)
{
    static GType type;

    if (!type)
        type = g_boxed_type_register_static ("MooCommandData",
                                             (GBoxedCopyFunc) moo_command_data_ref,
                                             (GBoxedFreeFunc) moo_command_data_unref);

    return type;
}


void
_moo_command_init (void)
{
    static gboolean been_here = FALSE;

    if (!been_here)
    {
        g_type_class_ref (MOO_TYPE_COMMAND_SCRIPT);
//         g_type_class_ref (MOO_TYPE_COMMAND_EXE);
        been_here = TRUE;
    }
}
