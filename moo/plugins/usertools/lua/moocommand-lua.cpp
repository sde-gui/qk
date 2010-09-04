/*
 *   moocommand-lua.cpp
 *
 *   Copyright (C) 2004-2010 by Yevgen Muntyan <emuntyan@sourceforge.net>
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
#include "moocommand-lua.h"
#include "mooedit-lua1.h"
#include "lua2-tool-setup.h"
#include "script/momscript-lua.h"
#include "mooedit/mooeditor.h"
#include "mooedit/mootext-private.h"
#include "mooutils/mooi18n.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/mootype-macros.h"
#include "glade/mooedittools-lua-gxml.h"
#include "moolua/moolua.h"
#include <string.h>


enum {
    KEY_VERSION,
    N_KEYS
};

static const char *data_keys[N_KEYS+1] = {
    "version", NULL
};

enum {
    MOO_COMMAND_LUA1,
    MOO_COMMAND_LUA2
};

struct _MooCommandLuaPrivate {
    char *code;
    int version;
};

G_DEFINE_TYPE (MooCommandLua, _moo_command_lua, MOO_TYPE_COMMAND)

typedef MooCommandFactory MooCommandFactoryLua;
typedef MooCommandFactoryClass MooCommandFactoryLuaClass;
MOO_DEFINE_TYPE_STATIC (MooCommandFactoryLua, _moo_command_factory_lua, MOO_TYPE_COMMAND_FACTORY)

static MooCommand  *moo_command_lua_new (int               version,
                                         const char       *code,
                                         MooCommandOptions options);


static void
add_path (lua_State *L, const char *dir)
{
    char **dirs;

    dirs = moo_get_data_subdirs (dir, MOO_DATA_SHARE, NULL);
    lua_addpath (L, dirs, g_strv_length (dirs));

    g_strfreev (dirs);
}

static lua_State *
create_lua1 (void)
{
    lua_State *L;

    L = lua_open ();
    luaL_openlibs (L);
    _moo_edit_lua1_add_api (L);
    add_path (L, "lua");

    return L;
}

static void
moo_command_lua1_run (MooCommandLua     *cmd,
                      MooCommandContext *ctx)
{
    GtkTextBuffer *buffer = NULL;
    lua_State *L;

    g_return_if_fail (cmd->priv->code != NULL);

    L = create_lua1 ();
    g_return_if_fail (L != NULL);

    if (luaL_loadstring (L, cmd->priv->code) != 0)
    {
        const char *msg = lua_tostring (L, -1);
        g_critical ("%s: %s", G_STRLOC, msg ? msg : "ERROR");
        lua_pop (L, 1);
        return;
    }

    _moo_edit_lua1_set_doc (L, GTK_TEXT_VIEW (moo_command_context_get_doc (ctx)));

    if (moo_command_context_get_doc (ctx))
        buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (moo_command_context_get_doc (ctx)));

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

    _moo_edit_lua1_cleanup (L);
    lua_close (L);
}

static void
moo_command_lua2_run (MooCommandLua     *cmd,
                      MooCommandContext *ctx)
{
    GtkTextBuffer *buffer = NULL;
    lua_State *L;

    g_return_if_fail (cmd->priv->code != NULL);

    L = lua_open ();
    g_return_if_fail (L != NULL);

    luaL_openlibs (L);
    add_path (L, "lua2");

    if (!mom::lua_setup (L))
    {
        lua_close (L);
        return;
    }

    if (luaL_loadstring (L, LUA2_SETUP_CODE) != 0)
    {
        const char *msg = lua_tostring (L, -1);
        g_critical ("%s: %s", G_STRLOC, msg ? msg : "ERROR");
        lua_close (L);
        return;
    }

    if (lua_pcall (L, 0, 0, 0) != 0)
    {
        const char *msg = lua_tostring (L, -1);
        g_critical ("%s: %s", G_STRLOC, msg ? msg : "ERROR");
        lua_close (L);
        return;
    }

    if (luaL_loadstring (L, cmd->priv->code) != 0)
    {
        const char *msg = lua_tostring (L, -1);
        g_critical ("%s: %s", G_STRLOC, msg ? msg : "ERROR");
        lua_close (L);
        return;
    }

    if (moo_command_context_get_doc (ctx))
        buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (moo_command_context_get_doc (ctx)));

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

    mom::lua_cleanup (L);
    lua_close (L);
}

static void
moo_command_lua_run (MooCommand        *cmd_base,
                     MooCommandContext *ctx)
{
    MooCommandLua *cmd = MOO_COMMAND_LUA (cmd_base);

    switch (cmd->priv->version)
    {
        case MOO_COMMAND_LUA1:
            moo_command_lua1_run (cmd, ctx);
            break;
        case MOO_COMMAND_LUA2:
            moo_command_lua2_run (cmd, ctx);
            break;
    }
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


static gboolean
parse_version (const char *string,
               int        *version)
{
    *version = MOO_COMMAND_LUA1;

    if (!string || !string[0])
        return TRUE;

    if (!strcmp (string, "1"))
        *version = MOO_COMMAND_LUA1;
    else if (!strcmp (string, "2"))
        *version = MOO_COMMAND_LUA2;
    else
    {
        moo_warning ("unknown version type %s", string);
        return FALSE;
    }

    return TRUE;
}

static MooCommand *
lua_factory_create_command (G_GNUC_UNUSED MooCommandFactory *factory,
                            MooCommandData *data,
                            const char     *options)
{
    MooCommand *cmd;
    int version = MOO_COMMAND_LUA1;
    const char *code;

    if (!parse_version (moo_command_data_get (data, KEY_VERSION), &version))
        return NULL;

    code = moo_command_data_get_code (data);
    g_return_val_if_fail (code && *code, NULL);

    cmd = moo_command_lua_new (version, code, moo_command_options_parse (options));
    g_return_val_if_fail (cmd != NULL, NULL);

    return cmd;
}


static void
init_combo (GtkComboBox *combo,
            const char **items,
            guint        n_items)
{
    GtkListStore *store;
    GtkCellRenderer *cell;
    guint i;

    cell = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), cell, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo), cell, "text", 0, (char*) NULL);

    store = gtk_list_store_new (1, G_TYPE_STRING);

    for (i = 0; i < n_items; ++i)
    {
        GtkTreeIter iter;
        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter, 0, Q_(items[i]), -1);
    }

    gtk_combo_box_set_model (combo, GTK_TREE_MODEL (store));

    g_object_unref (store);
}

static GtkWidget *
lua_factory_create_widget (G_GNUC_UNUSED MooCommandFactory *factory)
{
    LuaPageXml *xml;

    xml = lua_page_xml_new ();

    moo_text_view_set_font_from_string (xml->textview, "Monospace");
    moo_text_view_set_lang_by_id (xml->textview, "lua");

    const char *version_names[] = { N_("1"), N_("2") };
    init_combo (xml->version, version_names, G_N_ELEMENTS (version_names));

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
    int version;

    xml = lua_page_xml_get (page);
    g_return_if_fail (xml != NULL);

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (xml->textview));

    code = moo_command_data_get_code (data);
    gtk_text_buffer_set_text (buffer, code ? code : "", -1);

    parse_version (moo_command_data_get (data, KEY_VERSION), &version);
    gtk_combo_box_set_active (xml->version, version);
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
    int version, old_version;
    const char *version_strings[] = { "1", "2" };

    xml = lua_page_xml_get (page);
    g_return_val_if_fail (xml != NULL, FALSE);

    new_code = moo_text_view_get_text (xml->textview);
    code = moo_command_data_get_code (data);

    if (!_moo_str_equal (code, new_code))
    {
        moo_command_data_set_code (data, new_code);
        changed = TRUE;
    }

    version = gtk_combo_box_get_active (xml->version);
    parse_version (moo_command_data_get (data, KEY_VERSION), &old_version);
    g_assert (0 <= version && version <= MOO_COMMAND_LUA2);
    if (version != old_version)
    {
        moo_command_data_set (data, KEY_VERSION, version_strings[version]);
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

    factory = MOO_COMMAND_FACTORY (g_object_new (_moo_command_factory_lua_get_type (), (const char*) NULL));
    moo_command_factory_register ("lua", _("Lua script"), factory, (char**) data_keys, ".lua");
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
moo_command_lua_new (int                version,
                     const char        *code,
                     MooCommandOptions  options)
{
    MooCommandLua *cmd;

    g_return_val_if_fail (code != NULL, NULL);

    cmd = MOO_COMMAND_LUA (g_object_new (MOO_TYPE_COMMAND_LUA, "options", options, (const char*) NULL));
    cmd->priv->code = g_strdup (code);
    cmd->priv->version = version;

    return MOO_COMMAND (cmd);
}
