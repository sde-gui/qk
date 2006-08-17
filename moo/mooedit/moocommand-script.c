/*
 *   moocommand-script.c
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
#include "mooedit/mooedit-script.h"
#include "mooedit/mooedittools-glade.h"
#include "mooedit/mootextview.h"
#include "mooscript/mooscript-parser.h"
#include "mooutils/mooi18n.h"
#include "mooutils/mooglade.h"
#include <string.h>


struct _MooCommandScriptPrivate {
    MSNode *script;
};


G_DEFINE_TYPE (MooCommandScript, moo_command_script, MOO_TYPE_COMMAND)


static void
set_variable (const char   *name,
              const GValue *value,
              gpointer      data)
{
    MSContext *ctx = data;
    MSValue *ms_value;

    ms_value = ms_value_from_gvalue (value);
    g_return_if_fail (ms_value != NULL);

    ms_context_assign_variable (ctx, name, ms_value);
    ms_value_unref (ms_value);
}


static void
moo_command_script_run (MooCommand        *cmd_base,
                        MooCommandContext *ctx)
{
    MSValue *ret;
    MSContext *script_ctx;
    MooCommandScript *cmd = MOO_COMMAND_SCRIPT (cmd_base);

    g_return_if_fail (cmd->priv->script != NULL);

    script_ctx = moo_edit_script_context_new (moo_command_context_get_doc (ctx),
                                              moo_command_context_get_window (ctx));
    g_return_if_fail (script_ctx != NULL);

    moo_command_context_foreach (ctx, set_variable, script_ctx);

    ret = ms_top_node_eval (cmd->priv->script, script_ctx);

    if (!ret)
    {
        g_print ("%s\n", ms_context_get_error_msg (script_ctx));
        ms_context_clear_error (script_ctx);
    }

    ms_value_unref (ret);
    g_object_unref (script_ctx);
}


static void
moo_command_script_dispose (GObject *object)
{
    MooCommandScript *cmd = MOO_COMMAND_SCRIPT (object);

    if (cmd->priv)
    {
        ms_node_unref (cmd->priv->script);
        g_free (cmd->priv);
        cmd->priv = NULL;
    }

    G_OBJECT_CLASS(moo_command_script_parent_class)->dispose (object);
}


static MooCommand *
factory_func (MooCommandData *data,
              const char     *options,
              G_GNUC_UNUSED gpointer user_data)
{
    MooCommand *cmd;
    const char *code;

    code = moo_command_data_get (data, "code");

    g_return_val_if_fail (code && *code, NULL);

    cmd = moo_command_script_new (code, moo_command_options_parse (options));
    g_return_val_if_fail (cmd != NULL, NULL);

    return cmd;
}


static GtkWidget *
create_widget (G_GNUC_UNUSED gpointer data)
{
    GtkWidget *page;
    MooGladeXML *xml;

    xml = moo_glade_xml_new_empty (GETTEXT_PACKAGE);
    moo_glade_xml_parse_memory (xml, MOO_EDIT_TOOLS_GLADE_XML, -1, "moo_script_page", NULL);
    page = moo_glade_xml_get_widget (xml, "moo_script_page");
    g_return_val_if_fail (page != NULL, NULL);

    g_object_set_data_full (G_OBJECT (page), "moo-glade-xml", xml, g_object_unref);
    return page;
}


static void
load_data (GtkWidget      *page,
           MooCommandData *data,
           G_GNUC_UNUSED gpointer user_data)
{
    MooGladeXML *xml;
    GtkTextView *textview;
    GtkTextBuffer *buffer;
    const char *code;

    xml = g_object_get_data (G_OBJECT (page), "moo-glade-xml");
    textview = moo_glade_xml_get_widget (xml, "textview");
    buffer = gtk_text_view_get_buffer (textview);

    code = moo_command_data_get (data, "code");
    gtk_text_buffer_set_text (buffer, code ? code : "", -1);
}


static gboolean
save_data (GtkWidget      *page,
           MooCommandData *data,
           G_GNUC_UNUSED gpointer user_data)
{
    MooGladeXML *xml;
    GtkTextView *textview;
    const char *code;
    char *new_code;
    gboolean changed = FALSE;

    xml = g_object_get_data (G_OBJECT (page), "moo-glade-xml");
    textview = moo_glade_xml_get_widget (xml, "textview");
    g_assert (GTK_IS_TEXT_VIEW (textview));

    new_code = moo_text_view_get_text (textview);
    code = moo_command_data_get (data, "code");
    code = code ? code : "";

    if (strcmp (code, new_code) != 0)
    {
        moo_command_data_set (data, "code", new_code);
        changed = TRUE;
    }

    g_free (new_code);
    return changed;
}


static void
moo_command_script_class_init (MooCommandScriptClass *klass)
{
    G_OBJECT_CLASS(klass)->dispose = moo_command_script_dispose;
    MOO_COMMAND_CLASS(klass)->run = moo_command_script_run;

    moo_command_type_register ("MooScript", _("MooScript"),
                               factory_func,
                               create_widget,
                               load_data,
                               save_data,
                               NULL, NULL);
}


static void
moo_command_script_init (MooCommandScript *cmd)
{
    cmd->priv = g_new0 (MooCommandScriptPrivate, 1);
}


MooCommand *
moo_command_script_new (const char       *script,
                        MooCommandOptions options)
{
    MooCommandScript *cmd;
    MSNode *node;

    g_return_val_if_fail (script != NULL, NULL);

    node = ms_script_parse (script);
    g_return_val_if_fail (node != NULL, NULL);

    cmd = g_object_new (MOO_TYPE_COMMAND_SCRIPT, "options", options, NULL);
    cmd->priv->script = node;

    return MOO_COMMAND (cmd);
}
