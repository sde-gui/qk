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

enum {
    FILE_TOOLS,
    FILE_MENU,
    N_FILES
};

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


static ToolStore *tools_stores[N_FILES];


static void                 parse_user_tools        (MooMarkupDoc   *doc,
                                                     MooUIXML       *xml,
                                                     int             type);
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

    if (type == FILE_MENU)
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

        if (type == FILE_MENU)
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

    files = moo_get_data_files (type == FILE_TOOLS ? TOOLS_FILE : MENU_FILE,
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


static void
load_user_tools (const char *file,
                 MooUIXML   *xml,
                 int         type)
{
    MooMarkupDoc *doc;
    GError *error = NULL;
    char *freeme = NULL;

    g_return_if_fail (!xml || MOO_IS_UI_XML (xml));
    g_return_if_fail (type < N_FILES);

    unload_user_tools (type);
    _moo_command_init ();

    if (!file)
    {
        freeme = find_user_tools_file (type);
        file = freeme;
    }

    if (!file)
        return;

    doc = moo_markup_parse_file (file, &error);

    if (doc)
    {
        parse_user_tools (doc, xml, type);
        moo_markup_doc_unref (doc);
    }
    else
    {
        g_warning ("could not load file '%s': %s", file, error->message);
        g_error_free (error);
    }

    g_free (freeme);
}


void
moo_edit_load_user_tools (const char *file,
                          MooUIXML   *xml)
{
    return load_user_tools (file, xml, FILE_TOOLS);
}


void
moo_edit_load_user_menu (const char *file,
                         MooUIXML   *xml)
{
    return load_user_tools (file, xml, FILE_MENU);
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
parse_element (MooMarkupNode *node,
               MooUIXML      *xml,
               int            type,
               const char    *file)
{
    const char *os, *id;
    const char *position = NULL, *name = NULL, *label = NULL;
    const char *accel = NULL, *menu = NULL, *langs = NULL;
    MooMarkupNode *cmd_node, *child;
    MooCommand *cmd;
    gboolean enabled;
    ToolStore **store;
    ToolInfo *tool_info;
    gpointer klass = NULL;

    if (strcmp (node->name, "tool"))
    {
        g_warning ("invalid element %s in file %s", node->name, file);
        return;
    }

    id = moo_markup_get_prop (node, "id");

    if (!id || !id[0])
    {
        g_warning ("tool id attribute missing in file %s", file);
        return;
    }

    enabled = moo_markup_get_bool_prop (node, "enabled", TRUE);
    os = moo_markup_get_prop (node, "os");
    position = moo_markup_get_prop (node, "position");

    if (!enabled)
        return;

    if (os)
    {
#ifdef __WIN32__
        if (g_ascii_strncasecmp (os, "win", 3);
            return;
#else
        if (g_ascii_strcasecmp (os, "unix"))
            return;
#endif
    }

    for (child = node->children; child != NULL; child = child->next)
    {
        if (!MOO_MARKUP_IS_ELEMENT (child) || !strcmp (child->name, "command"))
            continue;

        if (!strcmp (child->name, "name"))
            name = moo_markup_get_content (child);
        else if (!strcmp (child->name, "_name"))
            name = _(moo_markup_get_content (child));
        else if (!strcmp (child->name, "label"))
            label = moo_markup_get_content (child);
        else if (!strcmp (child->name, "_label"))
            label = _(moo_markup_get_content (child));
        else if (!strcmp (child->name, "accel"))
            accel = moo_markup_get_content (child);
        else if (!strcmp (child->name, "position"))
            position = moo_markup_get_content (child);
        else if (!strcmp (child->name, "menu"))
            menu = moo_markup_get_content (child);
        else if (!strcmp (child->name, "langs"))
            langs = moo_markup_get_content (child);
        else
            g_warning ("unknown element %s in tool %s in file %s",
                       child->name, id, file);
    }

    if (!name)
    {
        g_warning ("name missing for tool '%s' in file %s", id, file);
        return;
    }

    if (!label)
        label = name;

    cmd_node = moo_markup_get_element (node, "command");

    if (!cmd_node)
    {
        g_warning ("command missing for tool '%s' in file %s", id, file);
        return;
    }

    cmd = _moo_command_parse_markup (cmd_node);

    if (!cmd)
    {
        g_warning ("could not get command for tool '%s' in file %s", id, file);
        return;
    }

    switch (type)
    {
        case FILE_TOOLS:
            klass = g_type_class_peek (MOO_TYPE_EDIT_WINDOW);

            if (!moo_window_class_find_group (klass, "Tools"))
                moo_window_class_new_group (klass, "Tools", _("Tools"));

            moo_window_class_new_action (klass, id, "Tools",
                                         "action-type::", MOO_TYPE_TOOL_ACTION,
                                         "display-name", name,
                                         "label", label,
                                         "accel", accel,
                                         "command", cmd,
                                         NULL);

            moo_edit_window_set_action_check (id, MOO_ACTION_CHECK_SENSITIVE,
                                              check_sensitive_func,
                                              NULL, NULL);

            if (langs)
                moo_edit_window_set_action_langs (id, MOO_ACTION_CHECK_ACTIVE, langs);

            break;

        case FILE_MENU:
            klass = g_type_class_peek (MOO_TYPE_EDIT);
            moo_edit_class_new_action (klass, id,
                                       "action-type::", MOO_TYPE_TOOL_ACTION,
                                       "display-name", name,
                                       "label", label,
                                       "accel", accel,
                                       "command", cmd,
                                       "langs", langs,
                                       NULL);
            break;
    }

    tool_info = g_new0 (ToolInfo, 1);
    tool_info->id = g_strdup (id);

    store = &tools_stores[type];

    if (!*store)
        *store = g_new (ToolStore, 1);

    (*store)->tools = g_slist_prepend ((*store)->tools, tool_info);

    if (xml)
    {
        const char *ui_path;
        char *freeme = NULL;
        char *markup;

        markup = g_markup_printf_escaped ("<item action=\"%s\"/>", id);
        tool_info->xml = g_object_ref (xml);
        tool_info->merge_id = moo_ui_xml_new_merge_id (xml);

        if (type == FILE_MENU)
        {
            ui_path = "Editor/Popup/PopupEnd";

            if (position)
            {
                if (!g_ascii_strcasecmp (position, "end"))
                    ui_path = "Editor/Popup/PopupEnd";
                else if (!g_ascii_strcasecmp (position, "start"))
                    ui_path = "Editor/Popup/PopupStart";
                else
                    g_warning ("unknown position type '%s' for tool %s in file %s",
                               position, id, file);
            }
        }
        else
        {
            freeme = g_strdup_printf ("Editor/Menubar/%s/UserMenu",
                                      menu ? menu : "Tools");
            ui_path = freeme;
        }

        moo_ui_xml_insert_markup (xml, tool_info->merge_id, ui_path, -1, markup);

        g_free (markup);
        g_free (freeme);
    }

    g_object_unref (cmd);
}


static void
parse_user_tools (MooMarkupDoc *doc,
                  MooUIXML     *xml,
                  int           type)
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
            parse_element (child, xml, type, doc->name);
    }
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
