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
#include "mooedit/mooeditor.h"
#include "mooedit/mooedit-actions.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/mooaccel.h"
#include "mooutils/mooi18n.h"
#include "mooutils/mooaction-private.h"
#include <string.h>


#ifdef __WIN32__
#if MOO_USER_TOOL_THIS_OS != MOO_USER_TOOL_WIN32
#error "oops"
#endif
#else
#if MOO_USER_TOOL_THIS_OS != MOO_USER_TOOL_UNIX
#error "oops"
#endif
#endif

#define N_TOOLS 2

#define ELEMENT_TOOLS       "tools"
#define ELEMENT_TOOL        "tool"
#define ELEMENT_ACCEL       "accel"
#define ELEMENT_MENU        "menu"
#define ELEMENT_LANGS       "langs"
#define ELEMENT_FILTER      "filter"
#define ELEMENT_POSITION    "position"
#define ELEMENT_COMMAND     "command"
#define PROP_NAME           "name"
#define PROP_ENABLED        "enabled"
#define PROP_OS             "os"

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


static const char *FILENAMES[N_TOOLS] = {"menu.xml", "context.xml"};
static ToolStore *tools_stores[N_TOOLS];


static MooCommandContext   *create_command_context  (gpointer        window,
                                                     gpointer        doc);


static void
unload_user_tools (int type)
{
    ToolStore *store = tools_stores[type];
    GSList *list;
    gpointer klass;

    g_assert (type < N_TOOLS);

    if (!store)
        return;

    list = store->tools;
    store->tools = NULL;

    if (type == MOO_USER_TOOL_MENU)
        klass = g_type_class_peek (MOO_TYPE_EDIT_WINDOW);
    else
        klass = g_type_class_peek (MOO_TYPE_EDIT);

    while (list)
    {
        ToolInfo *info = list->data;

        if (info->xml)
        {
            moo_ui_xml_remove_ui (info->xml, info->merge_id);
            g_object_unref (info->xml);
        }

        if (type == MOO_USER_TOOL_MENU)
            moo_window_class_remove_action (klass, info->id);
        else
            moo_edit_class_remove_action (klass, info->id);

        g_free (info->id);
        g_free (info);

        list = g_slist_delete_link (list, list);
    }

    g_free (store);
    tools_stores[type] = NULL;
}


static ToolInfo *
tools_store_add (MooUserToolType type,
                 char           *id)
{
    ToolInfo *info;

    g_assert (type < N_TOOLS);

    info = g_new0 (ToolInfo, 1);
    info->id = g_strdup (id);
    info->xml = moo_editor_get_ui_xml (moo_editor_instance ());

    if (info->xml)
    {
        g_object_ref (info->xml);
        info->merge_id = moo_ui_xml_new_merge_id (info->xml);
    }

    if (!tools_stores[type])
        tools_stores[type] = g_new0 (ToolStore, 1);

    tools_stores[type]->tools = g_slist_prepend (tools_stores[type]->tools, info);

    return info;
}


static char *
find_user_tools_file (int type)
{
    char **files;
    guint n_files;
    char *filename = NULL;
    int i;

    files = moo_get_data_files (FILENAMES[type], MOO_DATA_SHARE, &n_files);

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
mangle_name (const char *label,
             char      **id,
             char      **name)
{
    char *underscore;

    *id = g_strdup (label);
    g_strcanon (*id, G_CSET_A_2_Z G_CSET_a_2_z G_CSET_DIGITS "_", '_');

    *name = g_strdup (label);
    underscore = strchr (*name, '_');

    if (underscore)
        memmove (underscore, underscore + 1, strlen (underscore + 1) + 1);
}


static gboolean
check_info (MooUserToolInfo *info,
            MooUserToolType  type)
{
    if (!info->name || !info->name[0])
    {
        g_warning ("tool name missing in file %s", info->file);
        return FALSE;
    }

    if (info->filter && type != MOO_USER_TOOL_CONTEXT)
        g_warning ("filter specified in tool %s in file %s",
                   info->name, info->file);

    if (info->position != MOO_USER_TOOL_POS_END &&
        type != MOO_USER_TOOL_CONTEXT)
    {
        g_warning ("filter specified in tool %s in file %s",
                   info->name, info->file);
    }

    if (info->menu != NULL &&  type != MOO_USER_TOOL_MENU)
        g_warning ("menu specified in tool %s in file %s",
                   info->name, info->file);

    if (!info->cmd_data)
        info->cmd_data = moo_command_data_new ();

    if (!info->cmd_type)
    {
        g_warning ("command typ missing in tool '%s' in file %s",
                   info->name, info->file);
        return FALSE;
    }

    return TRUE;
}

static void
load_tool_func (MooUserToolInfo *info,
                G_GNUC_UNUSED gpointer user_data)
{
    ToolInfo *tool_info;
    gpointer klass = NULL;
    MooCommand *cmd;
    char *id, *name;

    g_return_if_fail (info != NULL);
    g_return_if_fail (info->file != NULL);

    if (!check_info (info, info->type))
        return;

    if (!info->enabled)
        return;

#ifdef __WIN32__
    if (info->os_type != MOO_USER_TOOL_WIN32)
        return;
#else
    if (info->os_type != MOO_USER_TOOL_UNIX)
        return;
#endif

    g_return_if_fail (MOO_IS_COMMAND_TYPE (info->cmd_type));

    cmd = moo_command_create (info->cmd_type->name,
                              info->options,
                              info->cmd_data);

    if (!cmd)
    {
        g_warning ("could not get command for tool '%s' in file %s",
                   info->name, info->file);
        return;
    }

    mangle_name (info->name, &id, &name);

    switch (info->type)
    {
        case MOO_USER_TOOL_MENU:
            klass = g_type_class_peek (MOO_TYPE_EDIT_WINDOW);

            if (!moo_window_class_find_group (klass, "Tools"))
                moo_window_class_new_group (klass, "Tools", _("Tools"));

            moo_window_class_new_action (klass, id, "Tools",
                                         "action-type::", MOO_TYPE_TOOL_ACTION,
                                         "display-name", name,
                                         "label", info->name,
                                         "accel", info->accel,
                                         "command", cmd,
                                         NULL);

            moo_edit_window_set_action_check (id, MOO_ACTION_CHECK_SENSITIVE,
                                              check_sensitive_func,
                                              NULL, NULL);

            if (info->langs)
                moo_edit_window_set_action_langs (id, MOO_ACTION_CHECK_ACTIVE, info->langs);

            break;

        case MOO_USER_TOOL_CONTEXT:
            klass = g_type_class_peek (MOO_TYPE_EDIT);
            moo_edit_class_new_action (klass, id,
                                       "action-type::", MOO_TYPE_TOOL_ACTION,
                                       "display-name", name,
                                       "label", info->name,
                                       "accel", info->accel,
                                       "command", cmd,
                                       "langs", info->langs,
                                       NULL);
            break;
    }

    tool_info = tools_store_add (info->type, id);

    if (tool_info->xml)
    {
        const char *ui_path;
        char *freeme = NULL;
        char *markup;

        markup = g_markup_printf_escaped ("<item action=\"%s\"/>", id);

        if (info->type == MOO_USER_TOOL_CONTEXT)
        {

            if (info->position == MOO_USER_TOOL_POS_START)
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

        moo_ui_xml_insert_markup (tool_info->xml,
                                  tool_info->merge_id,
                                  ui_path, -1, markup);

        g_free (markup);
        g_free (freeme);
    }

    g_free (id);
    g_free (name);
    g_object_unref (cmd);
}


void
_moo_edit_load_user_tools (MooUserToolType type)
{
    unload_user_tools (type);
    _moo_edit_parse_user_tools (type, load_tool_func, NULL);
}


static void
parse_element (MooMarkupNode       *node,
               MooUserToolType      type,
               MooToolFileParseFunc func,
               gpointer             data,
               const char          *file)
{
    const char *os;
    const char *position = NULL;
    MooMarkupNode *cmd_node, *child;
    MooUserToolInfo *info;

    info = _moo_user_tool_info_new ();
    info->type = type;
    info->file = g_strdup (file);
    info->position = MOO_USER_TOOL_POS_END;

    if (strcmp (node->name, ELEMENT_TOOL))
    {
        g_warning ("invalid element %s in file %s", node->name, file);
        return;
    }

    info->name = g_strdup (moo_markup_get_prop (node, PROP_NAME));

    info->enabled = moo_markup_get_bool_prop (node, PROP_ENABLED, TRUE);
    os = moo_markup_get_prop (node, PROP_OS);

#ifdef __WIN32__
    info->os_type = (!os || !g_ascii_strncasecmp (os, "win", 3)) ? MOO_USER_TOOL_WIN32 : MOO_USER_TOOL_UNIX;
#else
    info->os_type = (!os || !g_ascii_strcasecmp (os, "unix")) ? MOO_USER_TOOL_UNIX : MOO_USER_TOOL_WIN32;
#endif

    for (child = node->children; child != NULL; child = child->next)
    {
        if (!MOO_MARKUP_IS_ELEMENT (child) || !strcmp (child->name, ELEMENT_COMMAND))
            continue;

        if (!strcmp (child->name, ELEMENT_FILTER))
        {
            if (info->filter)
            {
                g_warning ("duplicated element '%s' in tool %s in file %s",
                           child->name, info->name, file);
                _moo_user_tool_info_unref (info);
                return;
            }

            info->filter = g_strdup (moo_markup_get_content (child));
        }
        else if (!strcmp (child->name, ELEMENT_ACCEL))
        {
            if (info->accel)
            {
                g_warning ("duplicated element '%s' in tool %s in file %s",
                           child->name, info->name, file);
                _moo_user_tool_info_unref (info);
                return;
            }

            info->accel = g_strdup (moo_markup_get_content (child));
        }
        else if (!strcmp (child->name, ELEMENT_POSITION))
        {
            position = moo_markup_get_content (child);
        }
        else if (!strcmp (child->name, ELEMENT_MENU))
        {
            if (info->menu)
            {
                g_warning ("duplicated element '%s' in tool %s in file %s",
                           child->name, info->name, file);
                _moo_user_tool_info_unref (info);
                return;
            }

            info->menu = g_strdup (moo_markup_get_content (child));
        }
        else if (!strcmp (child->name, ELEMENT_LANGS))
        {
            if (info->langs)
            {
                g_warning ("duplicated element '%s' in tool %s in file %s",
                           child->name, info->name, file);
                _moo_user_tool_info_unref (info);
                return;
            }

            info->langs = g_strdup (moo_markup_get_content (child));
        }
        else
        {
            g_warning ("unknown element %s in tool %s in file %s",
                       child->name, info->name, file);
        }
    }

    if (position)
    {
        if (!g_ascii_strcasecmp (position, "end"))
            info->position = MOO_USER_TOOL_POS_END;
        else if (!g_ascii_strcasecmp (position, "start"))
            info->position = MOO_USER_TOOL_POS_START;
        else
            g_warning ("unknown position type '%s' for tool %s in file %s",
                       position, info->name, file);
    }

    cmd_node = moo_markup_get_element (node, ELEMENT_COMMAND);

    if (!cmd_node)
    {
        g_warning ("command missing for tool '%s' in file %s", info->name, file);
        _moo_user_tool_info_unref (info);
        return;
    }

    info->cmd_data = _moo_command_parse_markup (cmd_node, &info->cmd_type, &info->options);

    if (!info->cmd_data)
    {
        g_warning ("could not get command data for tool '%s' in file %s", info->name, file);
        _moo_user_tool_info_unref (info);
        return;
    }

    func (info, data);

    _moo_user_tool_info_unref (info);
}


static void
parse_doc (MooMarkupDoc        *doc,
           MooUserToolType      type,
           MooToolFileParseFunc func,
           gpointer             data)
{
    MooMarkupNode *root, *child;

    root = moo_markup_get_root_element (doc, ELEMENT_TOOLS);

    if (!root)
    {
        g_warning ("no '" ELEMENT_TOOLS "' element in file '%s'", doc->name);
        return;
    }

    for (child = root->children; child != NULL; child = child->next)
    {
        if (MOO_MARKUP_IS_ELEMENT (child))
            parse_element (child, type, func, data, doc->name);
    }
}


void
_moo_edit_parse_user_tools (MooUserToolType        type,
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


void
_moo_edit_save_user_tools (MooUserToolType  type,
                           const GSList    *list)
{
    MooMarkupDoc *doc;
    MooMarkupNode *root;
    GError *error = NULL;
    char *string;

    g_return_if_fail (type < N_TOOLS);

    doc = moo_markup_doc_new ("tools");
    root = moo_markup_create_root_element (doc, ELEMENT_TOOLS);

    while (list)
    {
        MooMarkupNode *node;
        const MooUserToolInfo *info = list->data;

        node = moo_markup_create_element (root, ELEMENT_TOOL);

        if (info->name && info->name[0])
            moo_markup_set_prop (node, PROP_NAME, info->name);
        if (info->accel && info->accel[0])
            moo_markup_create_text_element (node, ELEMENT_ACCEL, info->accel);
        if (info->menu && info->menu[0])
            moo_markup_create_text_element (node, ELEMENT_MENU, info->menu);
        if (info->langs && info->langs[0])
            moo_markup_create_text_element (node, ELEMENT_LANGS, info->langs);
        if (info->filter && info->filter[0])
            moo_markup_create_text_element (node, ELEMENT_FILTER, info->filter);
        if (!info->enabled)
            moo_markup_set_bool_prop (node, PROP_ENABLED, info->enabled);
        if (info->position != MOO_USER_TOOL_POS_END)
            moo_markup_create_text_element (node, ELEMENT_POSITION, "start");

        _moo_command_format_markup (node, info->cmd_data,
                                    info->cmd_type, info->options);

        list = list->next;
    }

    string = moo_markup_node_get_pretty_string (MOO_MARKUP_NODE (doc), 2);

    g_message ("saving file %s", FILENAMES[type]);

    if (!moo_save_user_data_file (FILENAMES[type], string, -1, &error))
    {
        g_critical ("could not save tools file: %s", error->message);
        g_error_free (error);
    }

    g_message ("done");

    g_free (string);
    moo_markup_doc_unref (doc);
}


MooUserToolInfo *
_moo_user_tool_info_new (void)
{
    MooUserToolInfo *info = g_new0 (MooUserToolInfo, 1);
    info->ref_count = 1;
    return info;
}


MooUserToolInfo *
_moo_user_tool_info_ref (MooUserToolInfo *info)
{
    g_return_val_if_fail (info != NULL, NULL);
    info->ref_count++;
    return info;
}


void
_moo_user_tool_info_unref (MooUserToolInfo *info)
{
    g_return_if_fail (info != NULL);

    if (--info->ref_count)
        return;

    g_free (info->name);
    g_free (info->accel);
    g_free (info->menu);
    g_free (info->langs);
    g_free (info->options);
    g_free (info->filter);
    g_free (info->file);

    if (info->cmd_data)
        moo_command_data_unref (info->cmd_data);

    g_free (info);
}


GType
_moo_user_tool_info_get_type (void)
{
    static GType type = 0;

    if (!type)
        type = g_boxed_type_register_static ("MooUserToolInfo",
                                             (GBoxedCopyFunc) _moo_user_tool_info_ref,
                                             (GBoxedFreeFunc) _moo_user_tool_info_unref);

    return type;
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
