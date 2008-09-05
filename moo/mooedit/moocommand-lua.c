/*
 *   moocommand-lua.c
 *
 *   Copyright (C) 2004-2008 by Yevgen Muntyan <muntyan@tamu.edu>
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

#define MOOEDIT_COMPILATION
#include "mooedit/moocommand-lua.h"
#include "mooedit/mooedit-lua.h"
#include "mooedit/mooeditor.h"
#include "mooedit/mootext-private.h"
#include "mooutils/mooi18n.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/mootype-macros.h"
#include "glade/mooedittools-lua-gxml.h"
#include "moolua/moolua.h"
#include <string.h>


struct _MooCommandLuaPrivate {
    char *code;
};

G_DEFINE_TYPE (MooCommandLua, _moo_command_lua, MOO_TYPE_COMMAND)

typedef MooCommandFactory MooCommandFactoryLua;
typedef MooCommandFactoryClass MooCommandFactoryLuaClass;
MOO_DEFINE_TYPE_STATIC (MooCommandFactoryLua, _moo_command_factory_lua, MOO_TYPE_COMMAND_FACTORY)

static MooCommand  *moo_command_lua_new (const char        *code,
                                         MooCommandOptions  options);


static void
set_variable (const char   *name,
              const GValue *value,
              gpointer      data)
{
#if 0
    lua_State *L = data;
    MSContext *ctx = data;
    MSValue *ms_value;

    ms_value = ms_value_from_gvalue (value);
    g_return_if_fail (ms_value != NULL);

    [ctx assignVariable:name :ms_value];
    ms_value_unref (ms_value);
#endif
}

static void
add_path (lua_State *L)
{
    char **dirs;

    dirs = moo_get_data_subdirs ("lua", MOO_DATA_SHARE, NULL);
    lua_addpath (L, dirs, g_strv_length (dirs));

    g_strfreev (dirs);
}

static lua_State *
create_lua (void)
{
    lua_State *L;

    L = lua_open ();
    luaL_openlibs (L);
    _moo_edit_lua_add_api (L);
    add_path (L);

    return L;
}

static void
moo_command_lua_run (MooCommand        *cmd_base,
                     MooCommandContext *ctx)
{
    MooCommandLua *cmd = MOO_COMMAND_LUA (cmd_base);
    GtkTextBuffer *buffer = NULL;
    lua_State *L;

    g_return_if_fail (cmd->priv->code != NULL);

    L = create_lua ();
    g_return_if_fail (L != NULL);

    moo_command_context_foreach (ctx, set_variable, L);

    if (luaL_loadstring (L, cmd->priv->code) != 0)
    {
        const char *msg = lua_tostring (L, -1);
        g_critical ("%s: %s", G_STRLOC, msg ? msg : "ERROR");
        lua_pop (L, 1);
        return;
    }

    _moo_edit_lua_set_doc (L, moo_command_context_get_doc (ctx));

    if (moo_command_context_get_doc (ctx))
        buffer = gtk_text_view_get_buffer (moo_command_context_get_doc (ctx));

    if (buffer)
        gtk_text_buffer_begin_user_action (buffer);

    if (lua_pcall (L, 0, 0, 0) != 0)
    {
        const char *msg = lua_tostring (L, -1);
        g_critical ("%s: %s", G_STRLOC, msg ? msg : "ERROR");
        lua_pop (L, 1);
    }

    if (buffer)
        gtk_text_buffer_end_user_action (buffer);

    _moo_edit_lua_cleanup (L);
    lua_close (L);
}


static void
moo_command_lua_dispose (GObject *object)
{
    MooCommandLua *cmd = MOO_COMMAND_LUA (object);

    if (cmd->priv)
    {
        g_free (cmd->priv->code);
        cmd->priv = NULL;
    }

    G_OBJECT_CLASS(_moo_command_lua_parent_class)->dispose (object);
}


static MooCommand *
lua_factory_create_command (G_GNUC_UNUSED MooCommandFactory *factory,
                            MooCommandData *data,
                            const char     *options)
{
    MooCommand *cmd;
    const char *code;

    code = moo_command_data_get_code (data);

    g_return_val_if_fail (code && *code, NULL);

    cmd = moo_command_lua_new (code, moo_command_options_parse (options));
    g_return_val_if_fail (cmd != NULL, NULL);

    return cmd;
}


static GtkWidget *
lua_factory_create_widget (G_GNUC_UNUSED MooCommandFactory *factory)
{
    LuaPageXml *xml;

    xml = lua_page_xml_new ();

    moo_text_view_set_font_from_string (xml->textview, "Monospace");
    moo_text_view_set_lang_by_id (xml->textview, "lua");

    return GTK_WIDGET (xml->LuaPage);
}


static void
lua_factory_load_data (G_GNUC_UNUSED MooCommandFactory *factory,
                       GtkWidget      *page,
                       MooCommandData *data)
{
    LuaPageXml *xml;
    GtkTextBuffer *buffer;
    const char *code;

    xml = lua_page_xml_get (page);
    g_return_if_fail (xml != NULL);

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (xml->textview));

    code = moo_command_data_get_code (data);
    gtk_text_buffer_set_text (buffer, code ? code : "", -1);
}


static gboolean
lua_factory_save_data (G_GNUC_UNUSED MooCommandFactory *factory,
                       GtkWidget      *page,
                       MooCommandData *data)
{
    LuaPageXml *xml;
    const char *code;
    char *new_code;
    gboolean changed = FALSE;

    xml = lua_page_xml_get (page);
    g_return_val_if_fail (xml != NULL, FALSE);

    new_code = moo_text_view_get_text (xml->textview);
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
_moo_command_factory_lua_init (G_GNUC_UNUSED MooCommandFactory *factory)
{
}

static void
_moo_command_factory_lua_class_init (MooCommandFactoryClass *klass)
{
    klass->create_command = lua_factory_create_command;
    klass->create_widget = lua_factory_create_widget;
    klass->load_data = lua_factory_load_data;
    klass->save_data = lua_factory_save_data;
}


static void
_moo_command_lua_class_init (MooCommandLuaClass *klass)
{
    MooCommandFactory *factory;

    G_OBJECT_CLASS(klass)->dispose = moo_command_lua_dispose;
    MOO_COMMAND_CLASS(klass)->run = moo_command_lua_run;

    g_type_class_add_private (klass, sizeof (MooCommandLuaPrivate));

    factory = g_object_new (_moo_command_factory_lua_get_type (), NULL);
    moo_command_factory_register ("lua", _("Lua script"), factory, NULL, ".lua");
    g_object_unref (factory);
}


static void
_moo_command_lua_init (MooCommandLua *cmd)
{
    cmd->priv = G_TYPE_INSTANCE_GET_PRIVATE (cmd,
                                             MOO_TYPE_COMMAND_LUA,
                                             MooCommandLuaPrivate);
}


static MooCommand *
moo_command_lua_new (const char       *code,
                     MooCommandOptions options)
{
    MooCommandLua *cmd;

    g_return_val_if_fail (code != NULL, NULL);

    cmd = g_object_new (MOO_TYPE_COMMAND_LUA, "options", options, NULL);
    cmd->priv->code = g_strdup (code);

    return MOO_COMMAND (cmd);
}
