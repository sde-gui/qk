/*
 *   moocommand.c
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

#define MOOEDIT_COMPILATION
#include "mooedit/moocommand-private.h"
#include "mooedit/moocommand-script.h"
#include "mooedit/moocommand-exe.h"
#include "mooedit/moocommand-builtin.h"
#include "mooedit/mooeditwindow.h"
#include "mooedit/moooutputfilterregex.h"
#include "mooedit/mooedit-enums.h"
#include "mooutils/mooutils-misc.h"
#include <gtk/gtkwindow.h>
#include <gtk/gtktextview.h>
#include <string.h>


#define KEY_TYPE    "type"
#define KEY_OPTIONS "options"


G_DEFINE_TYPE (MooCommand, moo_command, G_TYPE_OBJECT)
G_DEFINE_TYPE (MooCommandFactory, moo_command_factory, G_TYPE_OBJECT)
G_DEFINE_TYPE (MooCommandContext, moo_command_context, G_TYPE_OBJECT)

enum {
    CTX_PROP_0,
    CTX_PROP_DOC,
    CTX_PROP_WINDOW
};

enum {
    CMD_PROP_0,
    CMD_PROP_OPTIONS
};

typedef struct {
    GValue value;
} Variable;

struct _MooCommandContextPrivate {
    GHashTable *vars;
    gpointer window;
    gpointer doc;
};

struct _MooCommandData {
    guint ref_count;
    char **data;
    guint len;
    char *code;
};

typedef struct {
    char *name;
    MooCommandFilterFactory factory_func;
    gpointer data;
    GDestroyNotify data_notify;
} FilterInfo;


static GHashTable *registered_factories;
static GHashTable *registered_filters;


static Variable    *variable_new                        (const GValue       *value);
static void         variable_free                       (Variable           *var);
static void         moo_command_data_take_code          (MooCommandData     *data,
                                                         char               *code);

static MooCommand  *_moo_command_factory_create_command (MooCommandFactory  *factory,
                                                         MooCommandData     *data,
                                                         const char         *options);


MooCommand *
moo_command_create (const char     *name,
                    const char     *options,
                    MooCommandData *data)
{
    MooCommandFactory *factory;

    g_return_val_if_fail (name != NULL, NULL);

    factory = moo_command_factory_lookup (name);
    g_return_val_if_fail (factory != NULL, NULL);

    return _moo_command_factory_create_command (factory, data, options);
}


void
moo_command_factory_register (const char        *name,
                              const char        *display_name,
                              MooCommandFactory *factory,
                              char             **keys)
{
    MooCommandFactoryClass *klass;

    g_return_if_fail (name != NULL);
    g_return_if_fail (display_name != NULL);
    g_return_if_fail (MOO_IS_COMMAND_FACTORY (factory));

    klass = MOO_COMMAND_FACTORY_GET_CLASS (factory);
    g_return_if_fail (klass->create_command != NULL);
    g_return_if_fail (klass->create_widget != NULL);
    g_return_if_fail (klass->load_data != NULL);
    g_return_if_fail (klass->save_data != NULL);
    g_return_if_fail (klass->data_equal != NULL);

    if (registered_factories != NULL)
    {
        MooCommandFactory *old = g_hash_table_lookup (registered_factories, name);

        if (old)
        {
            _moo_message ("command factory '%s' already registered", name);
            g_hash_table_remove (registered_factories, name);
            g_object_unref (old);
        }
    }

    if (!registered_factories)
        registered_factories = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

    factory->name = g_strdup (name);
    factory->display_name = g_strdup (display_name);
    factory->keys = g_strdupv (keys);
    factory->n_keys = keys ? g_strv_length (keys) : 0;

    g_hash_table_insert (registered_factories, g_strdup (name), g_object_ref (factory));
}


MooCommandFactory *
moo_command_factory_lookup (const char *name)
{
    MooCommandFactory *factory = NULL;

    g_return_val_if_fail (name != NULL, NULL);

    if (registered_factories != NULL)
        factory = g_hash_table_lookup (registered_factories, name);

    return factory;
}


static void
add_factory_hash_cb (G_GNUC_UNUSED const char *name,
                     MooCommandFactory *factory,
                     GSList **list)
{
    *list = g_slist_prepend (*list, factory);
}

GSList *
moo_command_list_factories (void)
{
    GSList *list = NULL;

    if (registered_factories)
        g_hash_table_foreach (registered_factories, (GHFunc) add_factory_hash_cb, &list);

    return g_slist_reverse (list);
}


static void
moo_command_factory_init (G_GNUC_UNUSED MooCommandFactory *factory)
{
}

static void
moo_command_factory_finalize (GObject *object)
{
    MooCommandFactory *factory = MOO_COMMAND_FACTORY (object);
    g_free (factory->name);
    g_free (factory->display_name);
    g_strfreev (factory->keys);
    G_OBJECT_CLASS (moo_command_factory_parent_class)->finalize (object);
}

static GtkWidget *
dummy_create_widget (G_GNUC_UNUSED MooCommandFactory *factory)
{
    return gtk_vbox_new (FALSE, FALSE);
}

static void
dummy_load_data (G_GNUC_UNUSED MooCommandFactory *factory,
                 G_GNUC_UNUSED GtkWidget *page,
                 G_GNUC_UNUSED MooCommandData *data)
{
}

static gboolean
dummy_save_data (G_GNUC_UNUSED MooCommandFactory *factory,
                 G_GNUC_UNUSED GtkWidget *page,
                 G_GNUC_UNUSED MooCommandData *data)
{
    return FALSE;
}

static void
moo_command_factory_class_init (MooCommandFactoryClass *klass)
{
    G_OBJECT_CLASS (klass)->finalize = moo_command_factory_finalize;
    klass->create_widget = dummy_create_widget;
    klass->load_data = dummy_load_data;
    klass->save_data = dummy_save_data;
}


static MooCommand *
_moo_command_factory_create_command (MooCommandFactory *factory,
                                     MooCommandData    *data,
                                     const char        *options)
{
    MooCommand *cmd;

    g_return_val_if_fail (MOO_IS_COMMAND_FACTORY (factory), NULL);
    g_return_val_if_fail (MOO_COMMAND_FACTORY_GET_CLASS(factory)->create_command != NULL, NULL);

    if (data)
        moo_command_data_ref (data);
    else
        data = moo_command_data_new (factory->n_keys);

    cmd = MOO_COMMAND_FACTORY_GET_CLASS (factory)->create_command (factory, data, options);

    moo_command_data_unref (data);
    return cmd;
}

GtkWidget *
_moo_command_factory_create_widget (MooCommandFactory *factory)
{
    g_return_val_if_fail (MOO_IS_COMMAND_FACTORY (factory), NULL);
    g_return_val_if_fail (MOO_COMMAND_FACTORY_GET_CLASS(factory)->create_widget != NULL, NULL);
    return MOO_COMMAND_FACTORY_GET_CLASS (factory)->create_widget (factory);
}

void
_moo_command_factory_load_data (MooCommandFactory *factory,
                                GtkWidget         *widget,
                                MooCommandData    *data)
{
    g_return_if_fail (MOO_IS_COMMAND_FACTORY (factory));
    g_return_if_fail (MOO_COMMAND_FACTORY_GET_CLASS(factory)->load_data != NULL);
    g_return_if_fail (GTK_IS_WIDGET (widget));
    g_return_if_fail (data != NULL);
    MOO_COMMAND_FACTORY_GET_CLASS(factory)->load_data (factory, widget, data);
}

gboolean
_moo_command_factory_save_data (MooCommandFactory *factory,
                                GtkWidget         *widget,
                                MooCommandData    *data)
{
    g_return_val_if_fail (MOO_IS_COMMAND_FACTORY (factory), FALSE);
    g_return_val_if_fail (MOO_COMMAND_FACTORY_GET_CLASS(factory)->save_data != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);
    g_return_val_if_fail (data != NULL, FALSE);
    return MOO_COMMAND_FACTORY_GET_CLASS (factory)->save_data (factory, widget, data);
}

gboolean
_moo_command_factory_data_equal (MooCommandFactory *factory,
                                 MooCommandData    *data1,
                                 MooCommandData    *data2)
{
    g_return_val_if_fail (MOO_IS_COMMAND_FACTORY (factory), FALSE);
    g_return_val_if_fail (MOO_COMMAND_FACTORY_GET_CLASS(factory)->data_equal != NULL, FALSE);
    g_return_val_if_fail (data1 != NULL, FALSE);
    g_return_val_if_fail (data2 != NULL, FALSE);
    return MOO_COMMAND_FACTORY_GET_CLASS (factory)->data_equal (factory, data1, data2);
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
        case CMD_PROP_OPTIONS:
            moo_command_set_options (cmd, g_value_get_flags (value));
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
        case CMD_PROP_OPTIONS:
            g_value_set_flags (value, cmd->options);
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
    if ((cmd->options & MOO_COMMAND_NEED_WINDOW) && !MOO_IS_EDIT_WINDOW (window))
        return FALSE;

    if ((cmd->options & MOO_COMMAND_NEED_DOC) && !doc)
        return FALSE;

    if ((cmd->options & MOO_COMMAND_NEED_FILE) && (!MOO_IS_EDIT (doc) || moo_edit_is_untitled (doc)))
        return FALSE;

    return TRUE;
}


static void
moo_command_class_init (MooCommandClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->set_property = moo_command_set_property;
    object_class->get_property = moo_command_get_property;

    klass->check_sensitive = moo_command_check_sensitive_real;

    g_object_class_install_property (object_class, CMD_PROP_OPTIONS,
                                     g_param_spec_flags ("options", "options", "options",
                                                         MOO_TYPE_COMMAND_OPTIONS,
                                                         0,
                                                         G_PARAM_READWRITE));
}


static void
moo_command_init (G_GNUC_UNUSED MooCommand *cmd)
{
}


static MooCommandOptions
check_options (MooCommandOptions options)
{
    MooCommandOptions checked = options;

    if (options & MOO_COMMAND_NEED_SAVE_ALL)
        checked |= MOO_COMMAND_NEED_SAVE;
    if (options & MOO_COMMAND_NEED_FILE)
        checked |= MOO_COMMAND_NEED_DOC;
    if (options & MOO_COMMAND_NEED_SAVE)
        checked |= MOO_COMMAND_NEED_DOC;

    return checked;
}


static gboolean
save_one (MooEdit *doc)
{
    g_return_val_if_fail (MOO_IS_EDIT (doc), FALSE);

    if (MOO_EDIT_IS_MODIFIED (doc) && !moo_edit_save (doc, NULL))
        return FALSE;

    return TRUE;
}

static gboolean
save_all (MooEdit *doc)
{
    MooEditWindow *window;
    GSList *list, *l;
    gboolean result = TRUE;

    g_return_val_if_fail (MOO_IS_EDIT (doc), FALSE);

    window = moo_edit_get_window (doc);
    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), FALSE);

    list = moo_edit_window_list_docs (window);

    for (l = list; l != NULL; l = l->next)
    {
        if (!save_one (l->data))
        {
            result = FALSE;
            break;
        }
    }

    g_slist_free (list);
    return result;
}


static gboolean
check_context (MooCommandOptions options,
               gpointer          doc,
               gpointer          window)
{
    if ((options & MOO_COMMAND_NEED_WINDOW) &&
        !MOO_IS_EDIT_WINDOW (window))
            return FALSE;

    if ((options & MOO_COMMAND_NEED_DOC) && doc == NULL)
        return FALSE;

    if ((options & MOO_COMMAND_NEED_FILE) &&
        !(MOO_IS_EDIT (doc) && !moo_edit_is_untitled (doc)))
            return FALSE;

    if ((options & MOO_COMMAND_NEED_SAVE) && !MOO_IS_EDIT (doc))
        return FALSE;

    return TRUE;
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

    g_return_if_fail (check_context (cmd->options, doc, window));

    if (cmd->options & MOO_COMMAND_NEED_SAVE_ALL)
    {
        if (!save_all (doc))
            return;
    }
    else if (cmd->options & MOO_COMMAND_NEED_SAVE)
    {
        if (!save_one (doc) || moo_edit_is_untitled (doc))
            return;
    }

    MOO_COMMAND_GET_CLASS(cmd)->run (cmd, ctx);
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
moo_command_set_options (MooCommand       *cmd,
                         MooCommandOptions options)
{
    g_return_if_fail (MOO_IS_COMMAND (cmd));

    options = check_options (options);

    if (options != cmd->options)
    {
        cmd->options = options;
        g_object_notify (G_OBJECT (cmd), "options");
    }
}


MooCommandOptions
moo_command_get_options (MooCommand *cmd)
{
    g_return_val_if_fail (MOO_IS_COMMAND (cmd), 0);
    return cmd->options;
}


MooCommandOptions
moo_command_options_parse (const char *string)
{
    MooCommandOptions options = 0;
    char **pieces, **p;

    if (!string)
        return 0;

    pieces = g_strsplit_set (string, " \t\r\n;,", 0);

    if (!pieces || !pieces[0])
        goto out;

    for (p = pieces; *p != NULL; ++p)
    {
        char *s = *p;

        g_strdelimit (g_strstrip (s), "_", '-');

        if (!*s)
            continue;

        if (!strcmp (s, "need-doc"))
            options |= MOO_COMMAND_NEED_DOC;
        else if (!strcmp (s, "need-file"))
            options |= MOO_COMMAND_NEED_FILE;
        else if (!strcmp (s, "need-save") || !strcmp (s, "save"))
            options |= MOO_COMMAND_NEED_SAVE;
        else if (!strcmp (s, "need-save-all") || !strcmp (s, "save-all"))
            options |= MOO_COMMAND_NEED_SAVE_ALL;
        else if (!strcmp (s, "need-window"))
            options |= MOO_COMMAND_NEED_WINDOW;
        else
            g_warning ("unknown flag '%s'", s);
    }

out:
    g_strfreev (pieces);
    return options;
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


void
moo_command_context_set_string (MooCommandContext *ctx,
                                const char        *name,
                                const char        *value)
{
    if (value)
    {
        GValue gval;
        gval.g_type = 0;
        g_value_init (&gval, G_TYPE_STRING);
        g_value_set_static_string (&gval, value);
        moo_command_context_set (ctx, name, &gval);
        g_value_unset (&gval);
    }
    else
    {
        moo_command_context_unset (ctx, name);
    }
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
    g_return_val_if_fail (G_VALUE_TYPE (value) == 0, FALSE);

    var = g_hash_table_lookup (ctx->priv->vars, name);

    if (!var)
        return FALSE;

    g_value_init (value, G_VALUE_TYPE (&var->value));
    g_value_copy (&var->value, value);
    return TRUE;
}


const char *
moo_command_context_get_string (MooCommandContext  *ctx,
                                const char         *name)
{
    Variable *var;

    g_return_val_if_fail (MOO_IS_COMMAND_CONTEXT (ctx), NULL);
    g_return_val_if_fail (name != NULL, NULL);

    var = g_hash_table_lookup (ctx->priv->vars, name);

    if (!var)
        return NULL;

    g_return_val_if_fail (G_VALUE_TYPE (&var->value) == G_TYPE_STRING, NULL);
    return g_value_get_string (&var->value);
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

    g_type_class_add_private (klass, sizeof (MooCommandContextPrivate));

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
    ctx->priv = G_TYPE_INSTANCE_GET_PRIVATE (ctx, MOO_TYPE_COMMAND_CONTEXT, MooCommandContextPrivate);
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


static int
find_key (MooCommandFactory *factory,
          const char        *key)
{
    guint i;

    for (i = 0; i < factory->n_keys; ++i)
        if (!strcmp (factory->keys[i], key))
            return i;

    return -1;
}

static void
item_foreach_func (const char *key,
                   const char *val,
                   gpointer    user_data)
{
    int index;

    struct {
        MooCommandFactory *factory;
        MooCommandData *cmd_data;
        gboolean error;
        const char *filename;
        const char *name;
    } *data = user_data;

    if (data->error)
        return;

    index = find_key (data->factory, key);

    if (index < 0)
    {
        g_warning ("unknown key %s in item %s in file %s",
                   key, data->name, data->filename);
        data->error = TRUE;
        return;
    }

    moo_command_data_set (data->cmd_data, index, val);
}

MooCommandData *
_moo_command_parse_item (MooKeyFileItem     *item,
                         const char         *name,
                         const char         *filename,
                         MooCommandFactory **factory_p,
                         char              **options_p)
{
    MooCommandData *data;
    MooCommandFactory *factory;
    char *factory_name, *options;
    struct {
        MooCommandFactory *factory;
        MooCommandData *cmd_data;
        gboolean error;
        const char *filename;
        const char *name;
    } parse_data;

    g_return_val_if_fail (item != NULL, NULL);

    factory_name = moo_key_file_item_steal (item, KEY_TYPE);
    options = moo_key_file_item_steal (item, KEY_OPTIONS);

    if (!factory_name)
    {
        g_warning ("no type attribute in item %s in file %s", name, filename);
        goto error;
    }

    factory = moo_command_factory_lookup (factory_name);

    if (!factory)
    {
        g_warning ("unknown command type %s in item %s in file %s",
                   factory_name, name, filename);
        goto error;
    }

    data = moo_command_data_new (factory->n_keys);

    parse_data.factory = factory;
    parse_data.cmd_data = data;
    parse_data.error = FALSE;
    parse_data.filename = filename;
    parse_data.name = name;

    moo_key_file_item_foreach (item, (GHFunc) item_foreach_func, &parse_data);

    if (parse_data.error)
    {
        moo_command_data_unref (data);
        goto error;
    }

    moo_command_data_take_code (data, moo_key_file_item_steal_content (item));

    if (factory_p)
        *factory_p = factory;

    if (options_p)
        *options_p = options;
    else
        g_free (options);

    g_free (factory_name);

    return data;

error:
    g_free (factory_name);
    g_free (options);
    return NULL;
}


void
_moo_command_format_item (MooKeyFileItem    *item,
                          MooCommandData    *data,
                          MooCommandFactory *factory,
                          char              *options)
{
    g_return_if_fail (item != NULL);
    g_return_if_fail (MOO_IS_COMMAND_FACTORY (factory));

    moo_key_file_item_set (item, KEY_TYPE, factory->name);

    if (options && options[0])
        moo_key_file_item_set (item, KEY_OPTIONS, options);

    if (data)
    {
        guint i;
        for (i = 0; i < factory->n_keys; ++i)
            moo_key_file_item_set (item, factory->keys[i], moo_command_data_get (data, i));
        moo_key_file_item_set_content (item, moo_command_data_get_code (data));
    }
}


MooCommandData *
moo_command_data_new (guint len)
{
    MooCommandData *data = g_new0 (MooCommandData, 1);
    data->ref_count = 1;
    data->data = len ? g_new0 (char*, len) : NULL;
    data->len = len;
    return data;
}


#if 0
void
moo_command_data_clear (MooCommandData *data)
{
    guint i;

    g_return_if_fail (data != NULL);

    for (i = 0; i < data->len; ++i)
    {
        g_free (data->data[i]);
        data->data[i] = NULL;
    }
}
#endif


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
        guint i;
        for (i = 0; i < data->len; ++i)
            g_free (data->data[i]);
        g_free (data->data);
        g_free (data->code);
        g_free (data);
    }
}


void
moo_command_data_set (MooCommandData *data,
                      guint           index,
                      const char     *value)
{
    char *tmp;

    g_return_if_fail (data != NULL);
    g_return_if_fail (index < data->len);

    tmp = data->data[index];
    data->data[index] = g_strdup (value);
    g_free (tmp);
}


const char *
moo_command_data_get (MooCommandData *data,
                      guint           index)
{
    g_return_val_if_fail (data != NULL, NULL);
    g_return_val_if_fail (index < data->len, NULL);
    return data->data[index];
}


static void
moo_command_data_take_code (MooCommandData *data,
                            char           *code)
{
    char *tmp;

    g_return_if_fail (data != NULL);

    tmp = data->code;
    data->code = code;
    g_free (tmp);
}


void
moo_command_data_set_code (MooCommandData *data,
                           const char     *code)
{
    moo_command_data_take_code (data, g_strdup (code));
}


const char *
moo_command_data_get_code (MooCommandData *data)
{
    g_return_val_if_fail (data != NULL, NULL);
    return data->code;
}


GType
moo_command_data_get_type (void)
{
    static GType type;

    if (G_UNLIKELY (!type))
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
        g_type_class_unref (g_type_class_ref (MOO_TYPE_COMMAND_BUILTIN));
        g_type_class_unref (g_type_class_ref (MOO_TYPE_COMMAND_SCRIPT));
#ifndef __WIN32__
        g_type_class_unref (g_type_class_ref (MOO_TYPE_COMMAND_EXE));
#endif
        _moo_command_filter_regex_load ();
        been_here = TRUE;
    }
}


static void
filters_init (void)
{
    if (!registered_filters)
        registered_filters = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
}


static FilterInfo *
filter_lookup (const char *id)
{
    g_return_val_if_fail (id != NULL, NULL);

    if (registered_filters)
        return g_hash_table_lookup (registered_filters, id);
    else
        return NULL;
}


void
moo_command_filter_register (const char             *id,
                             const char             *name,
                             MooCommandFilterFactory factory_func,
                             gpointer                data,
                             GDestroyNotify          data_notify)
{
    FilterInfo *info;

    g_return_if_fail (id != NULL);
    g_return_if_fail (name != NULL);
    g_return_if_fail (factory_func != NULL);

    filters_init ();

    if (filter_lookup (id))
    {
        _moo_message ("reregistering filter '%s'", id);
        moo_command_filter_unregister (id);
    }

    info = g_new0 (FilterInfo, 1);
    info->name = g_strdup (name);
    info->factory_func = factory_func;
    info->data = data;
    info->data_notify = data_notify;

    g_hash_table_insert (registered_filters, g_strdup (id), info);
}


void
moo_command_filter_unregister (const char *id)
{
    FilterInfo *info;

    g_return_if_fail (id != NULL);

    info = filter_lookup (id);

    if (!info)
    {
        g_warning ("filter '%s' not registered", id);
        return;
    }

    g_hash_table_remove (registered_filters, id);

    if (info->data_notify)
        info->data_notify (info->data);

    g_free (info->name);
    g_free (info);
}


const char *
moo_command_filter_lookup (const char *id)
{
    FilterInfo *info;

    g_return_val_if_fail (id != NULL, 0);

    info = filter_lookup (id);

    return info ? info->name : NULL;
}


static void
prepend_filter_id (const char *id,
                   G_GNUC_UNUSED gpointer info,
                   GSList    **list)
{
    *list = g_slist_prepend (*list, g_strdup (id));
}

GSList *
moo_command_filter_list (void)
{
    GSList *list = NULL;

    if (registered_filters)
        g_hash_table_foreach (registered_filters, (GHFunc) prepend_filter_id, &list);

    return list;
}


MooOutputFilter *
moo_command_filter_create (const char *id)
{
    FilterInfo *info;

    g_return_val_if_fail (id != NULL, NULL);

    info = filter_lookup (id);
    g_return_val_if_fail (info != NULL, NULL);

    return info->factory_func (id, info->data);
}
