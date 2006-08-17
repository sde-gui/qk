/*
 *   moousertools.c
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

#include "mooedit/moousertools.h"
#include "mooedit/moocommand.h"
#include "mooedit/mooeditwindow.h"
#include "mooedit/mooedit-actions.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/mooaccel.h"
#include "mooutils/mooi18n.h"
#include "mooutils/mooaction-private.h"
#include <string.h>


#define TOOLS_FILE  "tools.xml"
#define MENU_FILE   "menu.xml"

#define N_TOOLS     2

enum {
    PROP_0,
    PROP_COMMAND
};

typedef struct {
    GSList *tools;
} ToolStore;

typedef struct {
    char *id;
    MooUIXML *xml;
    guint merge_id;
} ToolInfo;

typedef struct {
    MooEditAction parent;
    MooCommand *cmd;
} MooToolAction;

typedef MooEditActionClass MooToolActionClass;

GType _moo_tool_action_get_type (void) G_GNUC_CONST;
G_DEFINE_TYPE (MooToolAction, _moo_tool_action, MOO_TYPE_EDIT_ACTION);
#define MOO_TYPE_TOOL_ACTION    (_moo_tool_action_get_type())
#define MOO_IS_TOOL_ACTION(obj) (G_TYPE_CHECK_INSTANCE_TYPE (obj, MOO_TYPE_TOOL_ACTION))
#define MOO_TOOL_ACTION(obj)    (G_TYPE_CHECK_INSTANCE_CAST (obj, MOO_TYPE_TOOL_ACTION, MooToolAction))


static ToolStore *tools_stores[N_TOOLS];


static MooCommandContext   *create_command_context  (gpointer        window,
                                                     gpointer        doc);


static void
unload_user_tools (int type)
{
    ToolStore *store = tools_stores[type];
    GSList *list;
    gpointer klass;

    if (!store)
        return;

    list = store->tools;
    store->tools = NULL;

    if (type == MOO_TOOL_FILE_MENU)
        klass = g_type_class_peek (MOO_TYPE_EDIT_WINDOW);
    else
        klass = g_type_class_peek (MOO_TYPE_EDIT);

    while (list)
    {
        gpointer klass;
        ToolInfo *info = list->data;
        list = g_slist_delete_link (list, list);

        if (info->xml)
        {
            moo_ui_xml_remove_ui (info->xml, info->merge_id);
            g_object_unref (info->xml);
        }

        if (type == MOO_TOOL_FILE_MENU)
            moo_window_class_remove_action (klass, info->id);
        else
            moo_edit_class_remove_action (klass, info->id);

        g_free (info->id);
        g_free (info);
    }

    g_free (store);
    tools_stores[type] = NULL;
}


static char *
find_user_tools_file (int type)
{
    char **files;
    guint n_files;
    char *filename = NULL;
    int i;

    files = moo_get_data_files (type == MOO_TOOL_FILE_TOOLS ? TOOLS_FILE : MENU_FILE,
                                MOO_DATA_SHARE, &n_files);

    if (!n_files)
        return NULL;

    for (i = n_files - 1; i >= 0; --i)
    {
        if (g_file_test (files[i], G_FILE_TEST_EXISTS))
        {
            filename = g_strdup (files[i]);
            break;
        }
    }

    g_strfreev (files);
    return filename;
}


void
_moo_edit_save_user_tools (MooToolFileType  type,
                           MooMarkupDoc    *doc)
{
    char *contents;
    GError *error = NULL;

    g_return_if_fail (type < N_TOOLS);
    g_return_if_fail (doc != NULL);

    contents = moo_markup_node_get_pretty_string (MOO_MARKUP_NODE (doc), 2);

    if (!moo_save_user_data_file (type == MOO_TOOL_FILE_TOOLS ? TOOLS_FILE : MENU_FILE,
                                  contents, -1, &error))
    {
        g_critical ("could not save tools file: %s", error->message);
        g_error_free (error);
    }

    g_free (contents);
}


static gboolean
check_sensitive_func (GtkAction      *gtkaction,
                      MooEditWindow  *window,
                      MooEdit        *doc,
                      G_GNUC_UNUSED gpointer data)
{
    MooToolAction *action;

    g_return_val_if_fail (MOO_IS_TOOL_ACTION (gtkaction), FALSE);
    action = MOO_TOOL_ACTION (gtkaction);

    return moo_command_check_sensitive (action->cmd, doc, window);
}


static void
load_tool_func (MooToolLoadInfo *info,
                gpointer         user_data)
{
    ToolStore **store;
    ToolInfo *tool_info;
    gpointer klass = NULL;
    MooCommand *cmd;
    MooUIXML *xml = user_data;

    if (!info->enabled)
        return;

#ifdef __WIN32__
    if (info->os_type != MOO_TOOL_WINDOWS)
        return;
#else
    if (info->os_type != MOO_TOOL_UNIX)
        return;
#endif

    cmd = moo_command_create (info->cmd_type,
                              info->options,
                              info->cmd_data);

    if (!cmd)
    {
        g_warning ("could not get command for tool '%s' in file %s",
                   info->id, info->file);
        return;
    }

    switch (info->type)
    {
        case MOO_TOOL_FILE_TOOLS:
            klass = g_type_class_peek (MOO_TYPE_EDIT_WINDOW);

            if (!moo_window_class_find_group (klass, "Tools"))
                moo_window_class_new_group (klass, "Tools", _("Tools"));

            moo_window_class_new_action (klass, info->id, "Tools",
                                         "action-type::", MOO_TYPE_TOOL_ACTION,
                                         "display-name", info->name,
                                         "label", info->label,
                                         "accel", info->accel,
                                         "command", cmd,
                                         NULL);

            moo_edit_window_set_action_check (info->id, MOO_ACTION_CHECK_SENSITIVE,
                                              check_sensitive_func,
                                              NULL, NULL);

            if (info->langs)
                moo_edit_window_set_action_langs (info->id, MOO_ACTION_CHECK_ACTIVE, info->langs);

            break;

        case MOO_TOOL_FILE_MENU:
            klass = g_type_class_peek (MOO_TYPE_EDIT);
            moo_edit_class_new_action (klass, info->id,
                                       "action-type::", MOO_TYPE_TOOL_ACTION,
                                       "display-name", info->name,
                                       "label", info->label,
                                       "accel", info->accel,
                                       "command", cmd,
                                       "langs", info->langs,
                                       NULL);
            break;
    }

    tool_info = g_new0 (ToolInfo, 1);
    tool_info->id = g_strdup (info->id);

    store = &tools_stores[info->type];

    if (!*store)
        *store = g_new (ToolStore, 1);

    (*store)->tools = g_slist_prepend ((*store)->tools, tool_info);

    if (xml)
    {
        const char *ui_path;
        char *freeme = NULL;
        char *markup;

        markup = g_markup_printf_escaped ("<item action=\"%s\"/>", info->id);
        tool_info->xml = g_object_ref (xml);
        tool_info->merge_id = moo_ui_xml_new_merge_id (xml);

        if (info->type == MOO_TOOL_FILE_MENU)
        {

            if (info->position == MOO_TOOL_POS_START)
                ui_path = "Editor/Popup/PopupStart";
            else
                ui_path = "Editor/Popup/PopupEnd";
        }
        else
        {
            freeme = g_strdup_printf ("Editor/Menubar/%s/UserMenu",
                                      info->menu ? info->menu : "Tools");
            ui_path = freeme;
        }

        moo_ui_xml_insert_markup (xml, tool_info->merge_id, ui_path, -1, markup);

        g_free (markup);
        g_free (freeme);
    }

    g_object_unref (cmd);
}


void
_moo_edit_load_user_tools (MooToolFileType type,
                           MooUIXML       *xml)
{
    unload_user_tools (type);
    _moo_edit_parse_user_tools (type, load_tool_func, xml);
}


static void
parse_element (MooMarkupNode       *node,
               MooToolFileType      type,
               MooToolFileParseFunc func,
               gpointer             data,
               const char          *file)
{
    const char *os;
    const char *position = NULL;
    MooMarkupNode *cmd_node, *child;
    MooCommandData *cmd_data;
    MooToolLoadInfo info;

    memset (&info, 0, sizeof (info));
    info.type = type;
    info.file = file;
    info.position = MOO_TOOL_POS_END;

    if (strcmp (node->name, "tool"))
    {
        g_warning ("invalid element %s in file %s", node->name, file);
        return;
    }

    info.id = moo_markup_get_prop (node, "id");

    if (!info.id || !info.id[0])
    {
        g_warning ("tool id missing in file %s", file);
        return;
    }

    info.enabled = moo_markup_get_bool_prop (node, "enabled", TRUE);
    os = moo_markup_get_prop (node, "os");

#ifdef __WIN32__
    info.os_type = (!os || !g_ascii_strncasecmp (os, "win", 3)) ? MOO_TOOL_WINDOWS : MOO_TOOL_UNIX;
#else
    info.os_type = (!os || !g_ascii_strcasecmp (os, "unix")) ? MOO_TOOL_UNIX : MOO_TOOL_WINDOWS;
#endif

    for (child = node->children; child != NULL; child = child->next)
    {
        if (!MOO_MARKUP_IS_ELEMENT (child) || !strcmp (child->name, "command"))
            continue;

        if (!strcmp (child->name, "name"))
            info.name = moo_markup_get_content (child);
        else if (!strcmp (child->name, "_name"))
            info.name = _(moo_markup_get_content (child));
        else if (!strcmp (child->name, "label"))
            info.label = moo_markup_get_content (child);
        else if (!strcmp (child->name, "_label"))
            info.label = _(moo_markup_get_content (child));
        else if (!strcmp (child->name, "accel"))
            info.accel = moo_markup_get_content (child);
        else if (!strcmp (child->name, "position"))
            position = moo_markup_get_content (child);
        else if (!strcmp (child->name, "menu"))
            info.menu = moo_markup_get_content (child);
        else if (!strcmp (child->name, "langs"))
            info.langs = moo_markup_get_content (child);
        else
            g_warning ("unknown element %s in tool %s in file %s",
                       child->name, info.id, file);
    }

    if (!info.name)
    {
        g_warning ("name missing for tool '%s' in file %s", info.id, file);
        return;
    }

    if (position && type != MOO_TOOL_FILE_MENU)
    {
        g_warning ("invalid element 'position' in tool '%s' in file %s", info.id, file);
        return;
    }
    else if (position)
    {
        if (!g_ascii_strcasecmp (position, "end"))
            info.position = MOO_TOOL_POS_END;
        else if (!g_ascii_strcasecmp (position, "start"))
            info.position = MOO_TOOL_POS_START;
        else
            g_warning ("unknown position type '%s' for tool %s in file %s",
                       position, info.id, file);
    }

    if (!info.label)
        info.label = info.name;

    cmd_node = moo_markup_get_element (node, "command");

    if (!cmd_node)
    {
        g_warning ("command missing for tool '%s' in file %s", info.id, file);
        return;
    }

    info.cmd_data = _moo_command_parse_markup (cmd_node,
                                               (char**) &info.cmd_type,
                                               (char**) &info.options);

    if (!info.cmd_data)
    {
        g_warning ("could not get command data for tool '%s' in file %s", info.id, file);
        return;
    }

    func (&info, data);

    moo_command_data_unref (cmd_data);
    g_free ((char*) info.cmd_type);
    g_free ((char*) info.options);
}


static void
parse_doc (MooMarkupDoc        *doc,
           MooToolFileType      type,
           MooToolFileParseFunc func,
           gpointer             data)
{
    MooMarkupNode *root, *child;

    root = moo_markup_get_root_element (doc, "tools");

    if (!root)
    {
        g_warning ("no 'tools' element in file '%s'", doc->name);
        return;
    }

    for (child = root->children; child != NULL; child = child->next)
    {
        if (MOO_MARKUP_IS_ELEMENT (child))
            parse_element (child, type, func, data, doc->name);
    }
}


void
_moo_edit_parse_user_tools (MooToolFileType        type,
                            MooToolFileParseFunc   func,
                            gpointer               data)
{
    char *file;
    MooMarkupDoc *doc;
    GError *error = NULL;

    g_return_if_fail (type < N_TOOLS);
    g_return_if_fail (func != NULL);

    _moo_command_init ();
    file = find_user_tools_file (type);

    if (!file)
        return;

    doc = moo_markup_parse_file (file, &error);

    if (doc)
    {
        parse_doc (doc, type, func, data);
        moo_markup_doc_unref (doc);
    }
    else
    {
        g_warning ("could not load file '%s': %s", file, error->message);
        g_error_free (error);
    }

    g_free (file);
}


static void
moo_tool_action_set_property (GObject      *object,
                              guint         property_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
    MooToolAction *action = MOO_TOOL_ACTION (object);

    switch (property_id)
    {
        case PROP_COMMAND:
            if (action->cmd)
                g_object_unref (action->cmd);
            action->cmd = g_value_get_object (value);
            if (action->cmd)
                g_object_ref (action->cmd);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}


static void
moo_tool_action_get_property (GObject    *object,
                              guint       property_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
    MooToolAction *action = MOO_TOOL_ACTION (object);

    switch (property_id)
    {
        case PROP_COMMAND:
            g_value_set_object (value, action->cmd);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}


static void
moo_tool_action_finalize (GObject *object)
{
    MooToolAction *action = MOO_TOOL_ACTION (object);

    if (action->cmd)
        g_object_unref (action->cmd);

    G_OBJECT_CLASS (_moo_tool_action_parent_class)->finalize (object);
}


static void
moo_tool_action_activate (GtkAction *gtkaction)
{
    MooEditWindow *window;
    MooCommandContext *ctx = NULL;
    MooEdit *doc;
    MooToolAction *action = MOO_TOOL_ACTION (gtkaction);
    MooEditAction *edit_action = MOO_EDIT_ACTION (gtkaction);

    g_return_if_fail (action->cmd != NULL);

    if (edit_action->doc)
    {
        doc = edit_action->doc;
        window = moo_edit_get_window (doc);
    }
    else
    {
        window = _moo_action_get_window (action);
        g_return_if_fail (MOO_IS_EDIT_WINDOW (window));
        doc = moo_edit_window_get_active_doc (window);
    }

    ctx = create_command_context (window, doc);

    moo_command_run (action->cmd, ctx);

    g_object_unref (ctx);
}


static void
moo_tool_action_check_state (MooEditAction *edit_action)
{
    gboolean sensitive;
    MooToolAction *action = MOO_TOOL_ACTION (edit_action);

    g_return_if_fail (action->cmd != NULL);

    MOO_EDIT_ACTION_CLASS (_moo_tool_action_parent_class)->check_state (edit_action);

    if (!gtk_action_is_visible (GTK_ACTION (action)))
        return;

    sensitive = moo_command_check_sensitive (action->cmd,
                                             edit_action->doc,
                                             moo_edit_get_window (edit_action->doc));

    g_object_set (action, "sensitive", sensitive, NULL);
}


static void
_moo_tool_action_init (G_GNUC_UNUSED MooToolAction *action)
{
}


static void
_moo_tool_action_class_init (MooToolActionClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GtkActionClass *gtkaction_class = GTK_ACTION_CLASS (klass);
    MooEditActionClass *action_class = MOO_EDIT_ACTION_CLASS (klass);

    object_class->set_property = moo_tool_action_set_property;
    object_class->get_property = moo_tool_action_get_property;
    object_class->finalize = moo_tool_action_finalize;
    gtkaction_class->activate = moo_tool_action_activate;
    action_class->check_state = moo_tool_action_check_state;

    g_object_class_install_property (object_class, PROP_COMMAND,
                                     g_param_spec_object ("command", "command", "command",
                                                          MOO_TYPE_COMMAND,
                                                          G_PARAM_READWRITE));
}


static MooCommandContext *
create_command_context (gpointer window,
                        gpointer doc)
{
    MooCommandContext *ctx;

    ctx = moo_command_context_new (doc, window);

    return ctx;
}
