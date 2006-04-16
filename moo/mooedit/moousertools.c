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
#include "mooedit/mooeditwindow.h"
#include "mooedit/mooedit-script.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/mooconfig.h"
#include "mooutils/moocommand.h"
#include <string.h>


typedef enum {
    ACTION_NEED_DOC  = 1 << 0,
    ACTION_NEED_FILE = 1 << 1,
    ACTION_NEED_SAVE = 1 << 2,
} ActionOptions;

typedef struct {
    char *id;
    char *name;
    char *label;
    char *accel;
    GSList *langs;
    MooCommand *cmd;

    MooUIXML *xml;
    guint merge_id;

    ActionOptions options : 3;
} ActionData;

static GSList *actions;


static void         remove_actions      (void);
static void         load_file           (const char     *file,
                                         MooUIXML       *xml,
                                         const char     *ui_path);
static void         load_config_item    (MooConfigItem  *item,
                                         MooUIXML       *xml,
                                         const char     *ui_path);
static MooAction   *create_action       (MooWindow      *window,
                                         gpointer        user_data);

static void         check_visible_func  (MooAction      *action,
                                         MooEdit        *doc,
                                         GParamSpec     *pspec,
                                         GValue         *prop_value,
                                         gpointer        dummy);
static void         check_sensitive_func(MooAction      *action,
                                         MooEdit        *doc,
                                         GParamSpec     *pspec,
                                         GValue         *prop_value,
                                         gpointer        dummy);

static ActionData  *action_data_new     (const char     *name,
                                         const char     *label,
                                         const char     *accel,
                                         GSList         *langs,
                                         MooCommand     *cmd,
                                         ActionOptions   options);
static void         action_data_free    (ActionData     *data);


char **
moo_edit_get_user_tools_files (guint *n_files_p)
{
    guint n_files, i;
    char **files;
    GSList *list = NULL;

    files = moo_get_data_files (MOO_USER_TOOLS_FILE,
                                MOO_DATA_SHARE, &n_files);

    if (n_files)
    {
        int i;

        for (i = n_files - 1; i >= 0; --i)
        {
            if (g_file_test (files[i], G_FILE_TEST_EXISTS))
            {
                list = g_slist_prepend (list, g_strdup (files[i]));
                break;
            }
        }
    }

    g_strfreev (files);


    files = moo_get_data_files (MOO_USER_TOOLS_ADD_FILE,
                                MOO_DATA_SHARE, &n_files);

    if (n_files)
    {
        int i;

        for (i = n_files - 1; i >= 0; --i)
        {
            if (g_file_test (files[i], G_FILE_TEST_EXISTS))
                list = g_slist_prepend (list, g_strdup (files[i]));
        }
    }

    g_strfreev (files);


    list = g_slist_reverse (list);
    n_files = g_slist_length (list);
    files = g_new (char*, n_files + 1);
    files[n_files] = NULL;

    for (i = 0; i < n_files; ++i)
    {
        files[i] = list->data;
        list = g_slist_delete_link (list, list);
    }

    if (n_files_p)
        *n_files_p = n_files;

    return files;
}


void
moo_edit_load_user_tools (char      **files,
                          guint       n_files,
                          MooUIXML   *xml,
                          const char *ui_path)
{
    guint i;

    if (!n_files)
        return;

    g_return_if_fail (files || !n_files);

    remove_actions ();

    for (i = 0; i < n_files; ++i)
        load_file (files[i], xml, ui_path);
}


static void
load_file (const char *file,
           MooUIXML   *xml,
           const char *ui_path)
{
    MooConfig *config;
    guint n_items, i;

    g_return_if_fail (file != NULL);

    config = moo_config_parse_file (file);
    g_return_if_fail (config != NULL);

    n_items = moo_config_n_items (config);

    for (i = 0; i < n_items; ++i)
        load_config_item (moo_config_nth_item (config, i), xml, ui_path);

    moo_config_free (config);
}


static GSList *
config_item_get_langs (MooConfigItem *item)
{
    const char *string;
    char **pieces, **p;
    GSList *list = NULL;

    string = moo_config_item_get_value (item, "lang");

    if (!string)
        string = moo_config_item_get_value (item, "langs");

    if (!string)
        return NULL;

    pieces = g_strsplit_set (string, " \t\r\n;,", 0);

    if (!pieces)
        return NULL;

    for (p = pieces; p && *p; ++p)
        if (**p)
            list = g_slist_prepend (list, moo_lang_id_from_name (*p));

    g_strfreev (pieces);
    return g_slist_reverse (list);
}


static MooCommand *
config_item_get_command (MooConfigItem *item)
{
    const char *code, *type;
    MooCommandType cmd_type = MOO_COMMAND_SCRIPT;
    MooCommand *cmd;

    code = moo_config_item_get_content (item);
    g_return_val_if_fail (code != NULL, NULL);

    type = moo_config_item_get_value (item, "command");

    if (type)
        cmd_type = moo_command_type_parse (type);

    g_return_val_if_fail (cmd_type != 0, NULL);

    cmd = moo_command_new (cmd_type);
    moo_command_set_code (cmd, code);

    return cmd;
}


static ActionOptions
config_item_get_options (MooConfigItem *item)
{
    const char *string;
    char **pieces, **p;
    ActionOptions opts = 0;

    string = moo_config_item_get_value (item, "options");

    if (!string)
        return 0;

    pieces = g_strsplit_set (string, " \t\r\n;,", 0);

    if (!pieces)
        return 0;

    for (p = pieces; p && *p; ++p)
    {
        if (**p)
        {
            char *opt = g_ascii_strdown (g_strdelimit (*p, "_", '-'), -1);

            if (!strcmp (opt, "need-save"))
                opts |= ACTION_NEED_SAVE;
            else if (!strcmp (opt, "need-file"))
                opts |= ACTION_NEED_FILE;
            else if (!strcmp (opt, "need-doc"))
                opts |= ACTION_NEED_DOC;
            else
                g_warning ("%s: unknown option '%s'", G_STRLOC, opt);

            g_free (opt);
        }
    }

    g_strfreev (pieces);
    return opts;
}


static void
load_config_item (MooConfigItem *item,
                  MooUIXML      *xml,
                  const char    *ui_path)
{
    MooCommand *cmd;
    ActionData *data;
    ActionOptions options;
    GSList *langs;
    const char *name, *label, *accel;
    MooWindowClass *klass;

    g_return_if_fail (item != NULL);

    name = moo_config_item_get_value (item, "action");
    label = moo_config_item_get_value (item, "label");
    accel = moo_config_item_get_value (item, "accel");
    g_return_if_fail (name != NULL);

    cmd = config_item_get_command (item);
    g_return_if_fail (cmd != NULL);

    options = config_item_get_options (item);
    langs = config_item_get_langs (item);

    data = action_data_new (name, label, accel, langs, cmd, options);
    g_return_if_fail (data != NULL);

    klass = g_type_class_ref (MOO_TYPE_EDIT_WINDOW);

    moo_window_class_new_action_custom (klass, data->id,
                                        create_action, data,
                                        (GDestroyNotify) action_data_free);

    if (data->langs)
        moo_edit_window_add_action_check (data->id, "visible",
                                          check_visible_func,
                                          NULL, NULL);
    if (data->options)
        moo_edit_window_add_action_check (data->id, "sensitive",
                                          check_sensitive_func,
                                          NULL, NULL);

    if (xml)
    {
        char *markup = g_markup_printf_escaped ("<item action=\"%s\"/>",
                                                data->id);
        data->xml = g_object_ref (xml);
        data->merge_id = moo_ui_xml_new_merge_id (xml);
        moo_ui_xml_insert_markup (xml, data->merge_id, ui_path, -1, markup);
        g_free (markup);
    }

    g_type_class_unref (klass);
}


static char *
parse_accel (const char *string)
{
    char *accel;

    if (!string)
        return NULL;

    accel = g_strdup (string);
    g_strstrip (accel);

    if (*accel)
        return accel;

    g_free (accel);
    return NULL;
}


static ActionData *
action_data_new (const char     *name,
                 const char     *label,
                 const char     *accel,
                 GSList         *langs,
                 MooCommand     *cmd,
                 ActionOptions   options)
{
    ActionData *data;

    g_return_val_if_fail (name && name[0], NULL);
    g_return_val_if_fail (MOO_IS_COMMAND (cmd), NULL);

    data = g_new0 (ActionData, 1);

    data->id = g_strdup (name);
    data->name = g_strdup (name);
    data->label = label ? g_strdup (label) : g_strdup (name);
    data->accel = parse_accel (accel);
    data->langs = langs;
    data->cmd = cmd;
    data->options = options;

    actions = g_slist_prepend (actions, data);

    return data;
}


static void
action_data_free (ActionData *data)
{
    if (data)
    {
        MooWindowClass *klass = g_type_class_ref (MOO_TYPE_EDIT_WINDOW);
        /* XXX */
        if (data->langs)
            moo_edit_window_remove_action_check (data->id, "visible");
        if (data->options)
            moo_edit_window_remove_action_check (data->id, "sensitive");
        g_type_class_unref (klass);

        if (data->xml)
        {
            moo_ui_xml_remove_ui (data->xml, data->merge_id);
            g_object_unref (data->xml);
        }

        actions = g_slist_remove (actions, data);

        g_free (data->id);
        g_free (data->name);
        g_free (data->label);
        g_free (data->accel);
        g_slist_foreach (data->langs, (GFunc) g_free, NULL);
        g_slist_free (data->langs);
        g_object_unref (data->cmd);
        g_free (data);
    }
}


static void
add_id (ActionData *data,
        GSList    **list)
{
    g_return_if_fail (data != NULL);
    *list = g_slist_prepend (*list, g_strdup (data->id));
}

static void
remove_actions (void)
{
    GSList *names = NULL;
    MooWindowClass *klass = g_type_class_ref (MOO_TYPE_EDIT_WINDOW);

    g_slist_foreach (actions, (GFunc) add_id, &names);

    while (names)
    {
        moo_window_class_remove_action (klass, names->data);
        g_free (names->data);
        names = g_slist_delete_link (names, names);
    }

    g_type_class_unref (klass);
}


/****************************************************************************/
/* MooUserToolAction
 */

typedef struct {
    MooAction parent;
    MooEditWindow *window;
    ActionData *data;
} MooToolAction;

typedef MooActionClass MooToolActionClass;

GType _moo_tool_action_get_type (void) G_GNUC_CONST;

G_DEFINE_TYPE (MooToolAction, _moo_tool_action, MOO_TYPE_ACTION);
#define MOO_IS_TOOL_ACTION(obj) (G_TYPE_CHECK_INSTANCE_TYPE (obj, _moo_tool_action_get_type()))
#define MOO_TOOL_ACTION(obj)    (G_TYPE_CHECK_INSTANCE_CAST (obj, _moo_tool_action_get_type(), MooToolAction))


static void
moo_tool_action_activate (MooAction *_action)
{
    MooToolAction *action;
    MooEdit *doc;

    g_return_if_fail (MOO_IS_TOOL_ACTION (_action));
    action = MOO_TOOL_ACTION (_action);
    g_return_if_fail (action->data != NULL);

    doc = moo_edit_window_get_active_doc (action->window);

    if ((action->data->options & ACTION_NEED_DOC) && !doc)
        return;

    if (action->data->options & ACTION_NEED_SAVE)
        if (!moo_edit_save (doc, NULL))
            return;

    if (action->data->options & ACTION_NEED_FILE)
        if (!moo_edit_get_filename (doc))
            return;

    moo_edit_setup_command (action->data->cmd, doc, action->window);
    moo_command_run (action->data->cmd);
}


static void
_moo_tool_action_class_init (MooActionClass *klass)
{
    klass->activate = moo_tool_action_activate;
}

static void
_moo_tool_action_init (G_GNUC_UNUSED MooToolAction *action)
{
}


static MooAction *
create_action (MooWindow *window,
               gpointer   user_data)
{
    ActionData *data = user_data;
    MooToolAction *action;

    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), NULL);
    g_return_val_if_fail (data != NULL, NULL);

    action = g_object_new (_moo_tool_action_get_type(),
                           "name", data->name,
                           "label", data->label,
                           "accel", data->accel,
                           NULL);
    action->window = MOO_EDIT_WINDOW (window);
    action->data = data;

    return MOO_ACTION (action);
}


static void
check_visible_func (MooAction      *_action,
                    MooEdit        *doc,
                    G_GNUC_UNUSED GParamSpec *pspec,
                    GValue         *prop_value,
                    G_GNUC_UNUSED gpointer dummy)
{
    MooToolAction *action;
    const char *lang_id;
    MooLang *lang;
    gboolean visible;

    g_return_if_fail (MOO_IS_TOOL_ACTION (_action));
    action = MOO_TOOL_ACTION (_action);
    g_return_if_fail (action->data != NULL);

    lang = moo_text_view_get_lang (MOO_TEXT_VIEW (doc));
    lang_id = lang ? lang->id : MOO_LANG_NONE;

    visible = NULL != g_slist_find_custom (action->data->langs,
                                           lang_id,
                                           (GCompareFunc) strcmp);

    g_value_set_boolean (prop_value, visible);
}


static void
check_sensitive_func (MooAction      *_action,
                      G_GNUC_UNUSED MooEdit *doc,
                      G_GNUC_UNUSED GParamSpec *pspec,
                      GValue         *prop_value,
                      G_GNUC_UNUSED gpointer dummy)
{
    MooToolAction *action;
    gboolean sensitive = TRUE;

    g_return_if_fail (MOO_IS_TOOL_ACTION (_action));
    action = MOO_TOOL_ACTION (_action);
    g_return_if_fail (action->data != NULL);

    g_value_set_boolean (prop_value, sensitive);
}
