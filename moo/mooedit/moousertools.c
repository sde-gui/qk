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
#include "mooedit/moocmdview.h"
#include "mooedit/mooedit-actions.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/mooconfig.h"
#include "mooutils/moocommand.h"
#include "mooutils/mooaccel.h"
#include <string.h>


typedef enum {
    FILE_TOOLS,
    FILE_MENU
} FileType;

typedef enum {
    ACTION_NEED_DOC  = 1 << 0,
    ACTION_NEED_FILE = 1 << 1,
    ACTION_NEED_SAVE = 1 << 2,
    ACTION_SILENT    = 1 << 3
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

    FileType type;
    ActionOptions options;
} ActionData;

static GSList *tools_actions;
static GSList *menu_actions;


static void         remove_tools        (void);
static void         remove_menu_actions (void);

static void         load_config_item    (FileType        type,
                                         MooConfigItem  *item,
                                         MooUIXML       *xml,
                                         const char     *ui_path1,
                                         const char     *ui_path2);
static MooAction   *create_tool_action  (MooWindow      *window,
                                         gpointer        user_data);
static MooAction   *create_edit_action  (MooEdit        *edit,
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

static ActionData  *action_data_new     (FileType        type,
                                         const char     *name,
                                         const char     *label,
                                         const char     *accel,
                                         GSList         *langs,
                                         MooCommand     *cmd,
                                         ActionOptions   options);
static void         action_data_free    (ActionData     *data);


static void
get_files (FileType    type,
           char     ***files_p,
           guint      *n_files_p,
           char      **user_file_p)
{
    guint n_files = 0, i;
    char **files = NULL;
    char *user_file = NULL;
    GSList *list = NULL;

    switch (type)
    {
        case FILE_TOOLS:
            user_file = moo_get_user_data_file (MOO_USER_TOOLS_FILE);
            files = moo_get_data_files (MOO_USER_TOOLS_FILE,
                                        MOO_DATA_SHARE, &n_files);
            break;
        case FILE_MENU:
            user_file = moo_get_user_data_file (MOO_USER_MENU_FILE);
            files = moo_get_data_files (MOO_USER_MENU_FILE,
                                        MOO_DATA_SHARE, &n_files);
            break;
    }

    if (n_files)
    {
        int i;

        for (i = n_files - 1; i >= 0; --i)
            if (!user_file || strcmp (user_file, files[i]))
                if (g_file_test (files[i], G_FILE_TEST_EXISTS))
                    list = g_slist_prepend (list, g_strdup (files[i]));
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

    *user_file_p = user_file;
    *files_p = files;
    *n_files_p = n_files;
}


void
moo_edit_get_user_tools_files (char     ***default_files,
                               guint      *n_files,
                               char      **user_file)
{
    return get_files (FILE_TOOLS, default_files, n_files, user_file);
}


void
moo_edit_get_user_menu_files (char     ***default_files,
                              guint      *n_files,
                              char      **user_file)
{
    return get_files (FILE_MENU, default_files, n_files, user_file);
}


static void
moo_edit_load_tools (FileType    type,
                     char      **default_files,
                     guint       n_files,
                     char       *user_file,
                     MooUIXML   *xml,
                     const char *ui_path1,
                     const char *ui_path2)
{
    guint i, n_items;
    MooConfig *config;

    if (!n_files && !user_file)
        return;

    switch (type)
    {
        case FILE_TOOLS:
            remove_tools ();
            break;
        case FILE_MENU:
            remove_menu_actions ();
            break;
    }

    config = moo_config_new ();

    for (i = 0; i < n_files; ++i)
        moo_config_parse_file (config, default_files[i], FALSE, NULL);

    if (user_file && g_file_test (user_file, G_FILE_TEST_EXISTS))
        moo_config_parse_file (config, user_file, FALSE, NULL);

    n_items = moo_config_n_items (config);

    for (i = 0; i < n_items; ++i)
        load_config_item (type, moo_config_nth_item (config, i),
                          xml, ui_path1, ui_path2);

    g_object_unref (config);
}


void
moo_edit_load_user_tools (char      **default_files,
                          guint       n_files,
                          char       *user_file,
                          MooUIXML   *xml,
                          const char *ui_path)
{
    moo_edit_load_tools (FILE_TOOLS, default_files, n_files,
                         user_file, xml, ui_path, NULL);
}


void
moo_edit_load_user_menu (char      **default_files,
                         guint       n_files,
                         char       *user_file,
                         MooUIXML   *xml,
                         const char *start_path,
                         const char *end_path)
{
    moo_edit_load_tools (FILE_MENU, default_files, n_files,
                         user_file, xml, start_path, end_path);
}


static GSList *
config_item_get_langs (MooConfigItem *item)
{
    const char *string;
    char **pieces, **p;
    GSList *list = NULL;

    string = moo_config_item_get_value (item, MOO_USER_TOOL_KEY_LANG);

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

    type = moo_config_item_get_value (item, MOO_USER_TOOL_KEY_COMMAND);

    if (type)
        cmd_type = moo_command_type_parse (type);

    g_return_val_if_fail (cmd_type != 0, NULL);

    cmd = moo_command_new (cmd_type, code);

    return cmd;
}


static ActionOptions
config_item_get_options (MooConfigItem *item)
{
    const char *string;
    char **pieces, **p;
    ActionOptions opts = 0;

    string = moo_config_item_get_value (item, MOO_USER_TOOL_KEY_OPTIONS);

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

            if (!strcmp (opt, MOO_USER_TOOL_OPTION_NEED_SAVE))
                opts |= ACTION_NEED_SAVE;
            else if (!strcmp (opt, MOO_USER_TOOL_OPTION_NEED_FILE))
                opts |= ACTION_NEED_FILE;
            else if (!strcmp (opt, MOO_USER_TOOL_OPTION_NEED_DOC))
                opts |= ACTION_NEED_DOC;
            else if (!strcmp (opt, MOO_USER_TOOL_OPTION_SILENT))
                opts |= ACTION_SILENT;
            else
                g_warning ("%s: unknown option '%s'", G_STRLOC, opt);

            g_free (opt);
        }
    }

    g_strfreev (pieces);
    return opts;
}


static void
load_config_item (FileType       type,
                  MooConfigItem *item,
                  MooUIXML      *xml,
                  const char     *ui_path1,
                  const char     *ui_path2)
{
    MooCommand *cmd;
    ActionData *data;
    ActionOptions options;
    GSList *langs;
    const char *name, *label, *accel, *pos, *os;
    gpointer klass;

    g_return_if_fail (item != NULL);

    os = moo_config_item_get_value (item, MOO_USER_TOOL_KEY_OS);

    if (os)
    {
        char *norm = g_ascii_strdown (os, -1);

#ifdef __WIN32__
        if (!strcmp (norm, "unix"))
            return;
#else
        if (!strncmp (norm, "win", 3))
            return;
#endif

        g_free (norm);
    }

    name = moo_config_item_get_value (item, MOO_USER_TOOL_KEY_ACTION);
    label = moo_config_item_get_value (item, MOO_USER_TOOL_KEY_LABEL);
    accel = moo_config_item_get_value (item, MOO_USER_TOOL_KEY_ACCEL);
    pos = moo_config_item_get_value (item, MOO_USER_TOOL_KEY_POSITION);
    g_return_if_fail (name != NULL);

    cmd = config_item_get_command (item);
    g_return_if_fail (cmd != NULL);

    options = config_item_get_options (item);
    langs = config_item_get_langs (item);

    data = action_data_new (type, name, label, accel, langs, cmd, options);
    g_return_if_fail (data != NULL);

    switch (type)
    {
        case FILE_TOOLS:
            klass = g_type_class_ref (MOO_TYPE_EDIT_WINDOW);

            moo_window_class_new_action_custom (klass, data->id,
                                                create_tool_action, data,
                                                (GDestroyNotify) action_data_free);

            if (data->langs)
                moo_edit_window_add_action_check (data->id, "visible",
                                                  check_visible_func,
                                                  NULL, NULL);
            if (data->options)
                moo_edit_window_add_action_check (data->id, "sensitive",
                                                  check_sensitive_func,
                                                  NULL, NULL);

            g_type_class_unref (klass);
            break;

        case FILE_MENU:
            klass = g_type_class_ref (MOO_TYPE_EDIT);
            moo_edit_class_new_action_custom (klass, data->id,
                                              create_edit_action, data,
                                              (GDestroyNotify) action_data_free);
            g_type_class_unref (klass);
            break;
    }

    if (xml)
    {
        const char *ui_path = ui_path1;
        char *markup = g_markup_printf_escaped ("<item action=\"%s\"/>",
                                                data->id);
        data->xml = g_object_ref (xml);
        data->merge_id = moo_ui_xml_new_merge_id (xml);

        if (type == FILE_MENU)
        {
            if (pos)
            {
                char *c = g_ascii_strdown (pos, -1);

                if (!strcmp (c, MOO_USER_TOOL_POSITION_END))
                    ui_path = ui_path2;
                else if (!strcmp (c, MOO_USER_TOOL_POSITION_START))
                    ui_path = ui_path1;
                else
                    g_warning ("%s: unknown position type '%s'",
                               G_STRLOC, c);

                g_free (c);
            }
            else
            {
                ui_path = ui_path2;
            }
        }

        moo_ui_xml_insert_markup (xml, data->merge_id, ui_path, -1, markup);
        g_free (markup);
    }

}


static ActionData *
action_data_new (FileType        type,
                 const char     *name,
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

    data->type = type;
    data->id = g_strdup (name);
    data->name = g_strdup (name);
    data->label = label ? g_strdup (label) : g_strdup (name);
    data->accel = moo_accel_normalize (accel);
    data->langs = langs;
    data->cmd = cmd;
    data->options = options;

    if (options & ACTION_SILENT)
        moo_command_add_flags (cmd, MOO_COMMAND_SILENT);

    switch (type)
    {
        case FILE_TOOLS:
            tools_actions = g_slist_prepend (tools_actions, data);
            break;
        case FILE_MENU:
            menu_actions = g_slist_prepend (menu_actions, data);
            break;
    }

    return data;
}


static void
action_data_free (ActionData *data)
{
    if (data)
    {
        gpointer *klass;

        if (data->xml)
        {
            moo_ui_xml_remove_ui (data->xml, data->merge_id);
            g_object_unref (data->xml);
        }

        switch (data->type)
        {
            case FILE_TOOLS:
                tools_actions = g_slist_remove (tools_actions, data);

                klass = g_type_class_ref (MOO_TYPE_EDIT_WINDOW);
                if (data->langs)
                    moo_edit_window_remove_action_check (data->id, "visible");
                if (data->options)
                    moo_edit_window_remove_action_check (data->id, "sensitive");
                g_type_class_unref (klass);
                break;

            case FILE_MENU:
                menu_actions = g_slist_remove (menu_actions, data);
                break;
        }

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
remove_tools (void)
{
    GSList *names = NULL;
    MooWindowClass *klass = g_type_class_ref (MOO_TYPE_EDIT_WINDOW);

    g_slist_foreach (tools_actions, (GFunc) add_id, &names);

    while (names)
    {
        moo_window_class_remove_action (klass, names->data);
        g_free (names->data);
        names = g_slist_delete_link (names, names);
    }

    g_type_class_unref (klass);
}


static void
remove_menu_actions (void)
{
    GSList *names = NULL;
    MooEditClass *klass = g_type_class_ref (MOO_TYPE_EDIT);

    g_slist_foreach (menu_actions, (GFunc) add_id, &names);

    while (names)
    {
        moo_edit_class_remove_action (klass, names->data);
        g_free (names->data);
        names = g_slist_delete_link (names, names);
    }

    g_type_class_unref (klass);
}


/****************************************************************************/
/* MooUserToolAction
 */

typedef struct {
    MooEditAction parent;
    MooEditWindow *window;
    ActionData *data;
} MooToolAction;

typedef MooEditActionClass MooToolActionClass;

GType _moo_tool_action_get_type (void) G_GNUC_CONST;

G_DEFINE_TYPE (MooToolAction, _moo_tool_action, MOO_TYPE_EDIT_ACTION);
#define MOO_IS_TOOL_ACTION(obj) (G_TYPE_CHECK_INSTANCE_TYPE (obj, _moo_tool_action_get_type()))
#define MOO_TOOL_ACTION(obj)    (G_TYPE_CHECK_INSTANCE_CAST (obj, _moo_tool_action_get_type(), MooToolAction))


static gboolean
run_exe (MooToolAction *action,
         const char    *cmd_line)
{
    GtkWidget *cmd_view;

    g_return_val_if_fail (MOO_IS_TOOL_ACTION (action), FALSE);
    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (action->window), FALSE);
    g_return_val_if_fail (cmd_line != NULL, FALSE);

    cmd_view = moo_edit_window_get_output (action->window);
    g_return_val_if_fail (MOO_IS_CMD_VIEW (cmd_view), FALSE);

    moo_line_view_clear (MOO_LINE_VIEW (cmd_view));
    moo_big_paned_present_pane (action->window->paned,
                                moo_edit_window_get_output_pane (action->window));

    return moo_cmd_view_run_command (MOO_CMD_VIEW (cmd_view), cmd_line,
                                     action->data->cmd->working_dir,
                                     moo_action_get_name (MOO_ACTION (action)));
}


static void
moo_tool_action_activate (MooAction *_action)
{
    MooToolAction *action;
    MooEdit *doc;
    gboolean silent;

    g_return_if_fail (MOO_IS_TOOL_ACTION (_action));
    action = MOO_TOOL_ACTION (_action);
    g_return_if_fail (action->data != NULL);

    doc = MOO_EDIT_ACTION(action)->doc;
    doc = doc ? doc : moo_edit_window_get_active_doc (action->window);

    if ((action->data->options & ACTION_NEED_DOC) && !doc)
        return;

    if (action->data->options & ACTION_NEED_SAVE)
        if (!doc || !moo_edit_save (doc, NULL))
            return;

    if (action->data->options & ACTION_NEED_FILE)
        if (!doc || !moo_edit_get_filename (doc))
            return;

    moo_edit_setup_command (action->data->cmd, doc, action->window);
    silent = action->data->cmd->flags & MOO_COMMAND_SILENT;

    if (action->window && !silent)
        g_signal_connect_swapped (action->data->cmd, "run-exe",
                                  G_CALLBACK (run_exe), action);

    if (doc && moo_edit_get_filename (doc))
    {
        char *dir = g_path_get_dirname (moo_edit_get_filename (doc));
        moo_command_set_working_dir (action->data->cmd, dir);
        g_free (dir);
    }

    moo_command_run (action->data->cmd);

    if (action->window && !silent)
        g_signal_handlers_disconnect_by_func (action->data->cmd,
                                              (gpointer) run_exe,
                                              action);
}


static void
_moo_tool_action_class_init (MooToolActionClass *klass)
{
    MOO_ACTION_CLASS(klass)->activate = moo_tool_action_activate;
}

static void
_moo_tool_action_init (G_GNUC_UNUSED MooToolAction *action)
{
}


static MooAction *
create_tool_action (MooWindow *window,
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


static MooAction *
create_edit_action (MooEdit *edit,
                    gpointer user_data)
{
    ActionData *data = user_data;
    MooToolAction *action;
    MooEditActionFlags flags = 0;

    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    g_return_val_if_fail (data != NULL, NULL);

    if (data->options & ACTION_NEED_FILE)
        flags |= MOO_EDIT_ACTION_NEED_FILE;

    action = g_object_new (_moo_tool_action_get_type(),
                           "name", data->name,
                           "label", data->label,
                           "accel", data->accel,
                           "doc", edit,
                           "langs", data->langs,
                           "flags", flags,
                           NULL);
    action->window = moo_edit_get_window (edit);
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
