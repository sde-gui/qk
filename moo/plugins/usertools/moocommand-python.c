/*
 *   moocommand-python.cpp
 *
 *   Copyright (C) 2004-2010 by Yevgen Muntyan <emuntyan@users.sourceforge.net>
 *
 *   This file is part of medit.  medit is free software; you can
 *   redistribute it and/or modify it under the terms of the
 *   GNU Lesser General Public License as published by the
 *   Free Software Foundation; either version 2.1 of the License,
 *   or (at your option) any later version.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with medit.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "moocommand-python.h"
#include "plugins/usertools/python-tool-setup.h"
#include "mooedit/mooeditor.h"
#include "mooutils/mooi18n.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/mootype-macros.h"
#include "plugins/usertools/mooedittools-script-gxml.h"
#include "moopython/medit-python.h"
#include <string.h>

G_DEFINE_TYPE (MooCommandPython, _moo_command_python, MOO_TYPE_COMMAND)

typedef MooCommandFactory MooCommandFactoryPython;
typedef MooCommandFactoryClass MooCommandFactoryPythonClass;
MOO_DEFINE_TYPE_STATIC (MooCommandFactoryPython, _moo_command_factory_python, MOO_TYPE_COMMAND_FACTORY)

static MooCommand  *moo_command_python_new (const char       *code,
                                            MooCommandOptions options);

static void
moo_command_python_run (MooCommand        *cmd_base,
                        MooCommandContext *ctx)
{
    MooCommandPython *cmd = MOO_COMMAND_PYTHON (cmd_base);
    GtkTextBuffer *buffer = NULL;
    MooPythonState *state;

    g_return_if_fail (cmd->code != NULL);

    state = moo_python_state_new (TRUE);
    moo_return_if_fail (state != NULL);

    if (!moo_python_run_string (state, PYTHON_TOOL_SETUP_PY))
    {
        moo_python_state_free (state);
        return;
    }

    if (moo_command_context_get_doc (ctx))
        buffer = moo_edit_get_buffer (moo_command_context_get_doc (ctx));

    if (buffer)
        gtk_text_buffer_begin_user_action (buffer);

    moo_python_run_string (state, cmd->code);

    if (buffer)
        gtk_text_buffer_end_user_action (buffer);

    moo_python_state_free (state);
}

static void
moo_command_python_dispose (GObject *object)
{
    MooCommandPython *cmd = MOO_COMMAND_PYTHON (object);

    g_free (cmd->code);
    cmd->code = NULL;

    G_OBJECT_CLASS (_moo_command_python_parent_class)->dispose (object);
}

static MooCommand *
python_factory_create_command (G_GNUC_UNUSED MooCommandFactory *factory,
                               MooCommandData *data,
                               const char     *options)
{
    MooCommand *cmd;
    const char *code;

    code = moo_command_data_get_code (data);
    moo_return_val_if_fail (code && *code, NULL);

    cmd = moo_command_python_new (code, moo_parse_command_options (options));
    moo_return_val_if_fail (cmd != NULL, NULL);

    return cmd;
}

static GtkWidget *
python_factory_create_widget (G_GNUC_UNUSED MooCommandFactory *factory)
{
    ScriptPageXml *xml;

    xml = script_page_xml_new ();

    moo_text_view_set_font_from_string (xml->textview, "Monospace");
    moo_text_view_set_lang_by_id (xml->textview, "python");

    return GTK_WIDGET (xml->ScriptPage);
}

static void
python_factory_load_data (G_GNUC_UNUSED MooCommandFactory *factory,
                          GtkWidget      *page,
                          MooCommandData *data)
{
    ScriptPageXml *xml;
    GtkTextBuffer *buffer;
    const char *code;

    xml = script_page_xml_get (page);
    g_return_if_fail (xml != NULL);

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (xml->textview));

    code = moo_command_data_get_code (data);
    gtk_text_buffer_set_text (buffer, code ? code : "", -1);
}


static gboolean
python_factory_save_data (G_GNUC_UNUSED MooCommandFactory *factory,
                          GtkWidget      *page,
                          MooCommandData *data)
{
    ScriptPageXml *xml;
    const char *code;
    char *new_code;
    gboolean changed = FALSE;

    xml = script_page_xml_get (page);
    moo_return_val_if_fail (xml != NULL, FALSE);

    new_code = moo_text_view_get_text (GTK_TEXT_VIEW (xml->textview));
    code = moo_command_data_get_code (data);

    if (!_moo_str_equal (code, new_code))
    {
        moo_command_data_set_code (data, new_code);
        changed = TRUE;
    }

    g_free (new_code);
    return changed;
}

static void
_moo_command_factory_python_init (G_GNUC_UNUSED MooCommandFactory *factory)
{
}

static void
_moo_command_factory_python_class_init (MooCommandFactoryClass *klass)
{
    klass->create_command = python_factory_create_command;
    klass->create_widget = python_factory_create_widget;
    klass->load_data = python_factory_load_data;
    klass->save_data = python_factory_save_data;
}


static void
_moo_command_python_class_init (MooCommandPythonClass *klass)
{
    MooCommandFactory *factory;

    G_OBJECT_CLASS (klass)->dispose = moo_command_python_dispose;
    MOO_COMMAND_CLASS (klass)->run = moo_command_python_run;

    factory = MOO_COMMAND_FACTORY (g_object_new (_moo_command_factory_python_get_type (), (const char*) NULL));
    moo_command_factory_register ("python", _("Python script"), factory, NULL, ".py");
    g_object_unref (factory);
}

static void
_moo_command_python_init (G_GNUC_UNUSED MooCommandPython *cmd)
{
}

static MooCommand *
moo_command_python_new (const char        *code,
                        MooCommandOptions  options)
{
    MooCommandPython *cmd;

    moo_return_val_if_fail (code != NULL, NULL);

    cmd = MOO_COMMAND_PYTHON (g_object_new (MOO_TYPE_COMMAND_PYTHON, "options", options, (const char*) NULL));
    cmd->code = g_strdup (code);

    return MOO_COMMAND (cmd);
}
