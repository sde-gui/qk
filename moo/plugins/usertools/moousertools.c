/*
 *   moousertools.c
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
#include "moousertools.h"
#include "moousertools-prefs.h"
#include "moocommand-private.h"
#include "plugins/mooplugin-builtin.h"
#include "mooedit/mooeditor.h"
#include "mooedit/mooeditaction.h"
#include "mooedit/mooeditaction-factory.h"
#include "mooedit/mooplugin-macro.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/mooutils-debug.h"
#include "mooutils/mooaccel.h"
#include "mooutils/mooi18n.h"
#include "mooutils/mooaction-private.h"
#include "mooutils/mootype-macros.h"
#include <string.h>
#include <stdlib.h>


#define N_TOOLS 2

#define ITEM_TOOL       "tool"
#define KEY_ACCEL       "accel"
#define KEY_MENU        "menu"
#define KEY_LANGS       "langs"
#define KEY_FILTER      "file-filter"
#define KEY_POSITION    "position"
#define KEY_COMMAND     "command"
#define KEY_NAME        "name"
#define KEY_ENABLED     "enabled"
#define KEY_OS          "os"
#define KEY_ID          "id"
#define KEY_DELETED     "deleted"
#define KEY_BUILTIN     "builtin"
#define KEY_OPTIONS     "options"

#define NAME_PREFIX_LEN 3

enum {
    PROP_0,
    PROP_COMMAND
};

typedef struct {
    GSList *tools;
} ToolStore;


typedef struct {
    char *id;
    MooUiXml *xml;
    guint merge_id;
} ToolInfo;

typedef struct {
    MooEditAction parent;
    MooCommand *cmd;
} MooToolAction;

typedef MooEditActionClass MooToolActionClass;

static MooCommandContext   *create_command_context  (gpointer        window,
                                                     gpointer        doc);
static MooUserToolInfo     *_moo_user_tool_info_ref (MooUserToolInfo *info);
static void                 add_info                (MooUserToolInfo *info,
                                                     GSList        **list,
                                                     GHashTable     *ids);

MOO_DEFINE_BOXED_TYPE_R (MooUserToolInfo, _moo_user_tool_info)
MOO_DEFINE_TYPE_STATIC (MooToolAction, _moo_tool_action, MOO_TYPE_EDIT_ACTION)
#define MOO_TYPE_TOOL_ACTION    (_moo_tool_action_get_type())
#define MOO_IS_TOOL_ACTION(obj) (G_TYPE_CHECK_INSTANCE_TYPE (obj, MOO_TYPE_TOOL_ACTION))
#define MOO_TOOL_ACTION(obj)    (G_TYPE_CHECK_INSTANCE_CAST (obj, MOO_TYPE_TOOL_ACTION, MooToolAction))

static const char *FILENAMES[N_TOOLS] = {"menu.cfg", "context.cfg"};
static ToolStore *tools_stores[N_TOOLS];

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

    switch (type)
    {
        case MOO_USER_TOOL_MENU:
            info->xml = moo_editor_get_ui_xml (moo_editor_instance ());
            break;
        case MOO_USER_TOOL_CONTEXT:
            info->xml = moo_editor_get_doc_ui_xml (moo_editor_instance ());
            break;
    }

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


static void
find_user_tools_file (int     type,
                      char ***sys_files_p,
                      char  **user_file_p)
{
    char **files;
    guint n_files;
    int i;
    GPtrArray *sys_files = NULL;

    *sys_files_p = NULL;
    *user_file_p = NULL;

    files = _moo_strv_reverse (moo_get_data_files (FILENAMES[type]));

    if (!files || !*files)
        return;

    n_files = g_strv_length (files);

    if (g_file_test (files[n_files - 1], G_FILE_TEST_EXISTS))
        *user_file_p = g_strdup (files[n_files - 1]);

    for (i = 0; i < (int) n_files - 1; ++i)
    {
        if (g_file_test (files[i], G_FILE_TEST_EXISTS))
        {
            if (!sys_files)
                sys_files = g_ptr_array_new ();
            g_ptr_array_add (sys_files, g_strdup (files[i]));
        }
    }

    if (sys_files)
    {
        g_ptr_array_add (sys_files, NULL);
        *sys_files_p = (char**) g_ptr_array_free (sys_files, FALSE);
    }

    g_strfreev (files);
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


static gboolean
check_info (MooUserToolInfo *info,
            MooUserToolType  type)
{
    if (!info->id || !info->id[0])
    {
        g_warning ("tool id missing in file %s", info->file);
        return FALSE;
    }

    if (!info->name || !info->name[0])
    {
        g_warning ("tool name missing in file %s", info->file);
        return FALSE;
    }

    if (info->position != MOO_USER_TOOL_POS_END &&
        type != MOO_USER_TOOL_CONTEXT)
    {
        g_warning ("position specified in tool %s in file %s",
                   info->name, info->file);
    }

    if (info->menu != NULL &&  type != MOO_USER_TOOL_MENU)
        g_warning ("menu specified in tool %s in file %s",
                   info->name, info->file);

    if (!info->cmd_factory)
    {
        g_warning ("command type missing in tool '%s' in file %s",
                   info->name, info->file);
        return FALSE;
    }

    if (!info->cmd_data)
        info->cmd_data = moo_command_data_new (info->cmd_factory->n_keys);

    return TRUE;
}

static void
parse_name (MooUserToolInfo  *info,
            char            **name,
            char            **label,
            char            **ui_path,
            MooUiXml         *ui_xml,
            guint             merge_id)
{
    char *ph;
    char *prefix;
    char *underscore;
    const char *slash;

    g_return_if_fail (info->name != NULL);

    *name = g_strdup (info->name);
    if ((underscore = strchr (*name, '_')))
        memmove (underscore, underscore + 1, strlen (underscore + 1) + 1);

    if (info->type == MOO_USER_TOOL_CONTEXT)
    {
        if (info->position == MOO_USER_TOOL_POS_START)
            *ui_path = g_strdup ("Editor/Popup/PopupStart");
        else
            *ui_path = g_strdup ("Editor/Popup/PopupEnd");

        *label = g_strdup (info->name);

        return;
    }

    ph = g_strdup_printf ("Editor/Menubar/%s/UserMenu",
                          info->menu ? info->menu : "Tools");

    if (!(slash = strrchr (info->name, '/')))
    {
        *label = g_strdup (info->name);
        *ui_path = ph;
        return;
    }

    *label = g_strdup (slash + 1);
    prefix = g_strndup (info->name, slash - info->name);
    *ui_path = g_strdup_printf ("%s/%s", ph, prefix);

    if (!moo_ui_xml_get_node (ui_xml, *ui_path))
    {
        MooUiNode *parent;
        GString *markup;
        char **comps, **p;
        int i;

        parent = moo_ui_xml_get_node (ui_xml, ph);
        g_return_if_fail (parent != NULL);

        comps = g_strsplit (prefix, "/", 0);
        markup = g_string_new (NULL);

        for (p = comps, i = 0; p && *p; ++p)
        {
            if (**p)
            {
                char *tmp = g_markup_printf_escaped ("<item name=\"%s\" label=\"%s\">", *p, *p);
                g_string_append (markup, tmp);
                ++i;
            }
        }

        while (i--)
            g_string_append (markup, "</item>");

        moo_ui_xml_insert (ui_xml, merge_id, parent, -1, markup->str);

        g_strfreev (comps);
        g_string_free (markup, TRUE);
    }

    g_free (prefix);
    g_free (ph);
}

static void
load_tool (MooUserToolInfo *info)
{
    ToolInfo *tool_info;
    gpointer klass = NULL;
    MooCommand *cmd;
    char *name = NULL;
    char *label = NULL;
    char *ui_path = NULL;

    g_return_if_fail (info != NULL);
    g_return_if_fail (info->file != NULL);

    if (!check_info (info, info->type))
        return;

    if (!info->enabled)
        return;

    g_return_if_fail (MOO_IS_COMMAND_FACTORY (info->cmd_factory));

    cmd = moo_command_create (info->cmd_factory->name,
                              info->options,
                              info->cmd_data);

    if (!cmd)
    {
        g_warning ("could not get command for tool '%s' in file %s",
                   info->name, info->file);
        return;
    }

    tool_info = tools_store_add (info->type, info->id);
    parse_name (info, &name, &label, &ui_path, tool_info->xml, tool_info->merge_id);

    switch (info->type)
    {
        case MOO_USER_TOOL_MENU:
            klass = g_type_class_peek (MOO_TYPE_EDIT_WINDOW);

            if (!moo_window_class_find_group (klass, "Tools"))
                moo_window_class_new_group (klass, "Tools", _("Tools"));

            moo_window_class_new_action (klass, info->id, "Tools",
                                         "action-type::", MOO_TYPE_TOOL_ACTION,
                                         "display-name", name,
                                         "label", label,
                                         "default-accel", info->accel,
                                         "command", cmd,
                                         NULL);

            moo_edit_window_set_action_check (info->id, MOO_ACTION_CHECK_SENSITIVE,
                                              check_sensitive_func,
                                              NULL, NULL);
            moo_edit_window_set_action_filter (info->id, MOO_ACTION_CHECK_ACTIVE, info->filter);

            break;

        case MOO_USER_TOOL_CONTEXT:
            klass = g_type_class_peek (MOO_TYPE_EDIT);
            moo_edit_class_new_action (klass, info->id,
                                       "action-type::", MOO_TYPE_TOOL_ACTION,
                                       "display-name", name,
                                       "label", label,
                                       "default-accel", info->accel,
                                       "command", cmd,
                                       "file-filter", info->filter,
                                       NULL);
            break;
    }

    if (tool_info->xml)
    {
        char *markup;
        markup = g_markup_printf_escaped ("<item action=\"%s\"/>", info->id);
        moo_ui_xml_insert_markup (tool_info->xml,
                                  tool_info->merge_id,
                                  ui_path, -1, markup);
        g_free (markup);
    }

    g_free (name);
    g_free (ui_path);
    g_free (label);
    g_object_unref (cmd);
}


static void
init_tools_environment (void)
{
    static gboolean done;

    if (!done)
    {
        char **script_dirs;
        const char *old_path;
        char *new_path, *my_path;

        done = TRUE;

        script_dirs = moo_get_data_subdirs ("scripts");
        g_return_if_fail (script_dirs != NULL);

        old_path = g_getenv ("PATH");
        g_return_if_fail (old_path != NULL);

        my_path = g_strjoinv (G_SEARCHPATH_SEPARATOR_S, script_dirs);
        new_path = g_strdup_printf ("%s%s%s", my_path, G_SEARCHPATH_SEPARATOR_S, old_path);

        g_setenv ("PATH", new_path, TRUE);

        g_free (my_path);
        g_free (new_path);
        g_strfreev (script_dirs);
    }
}


void
_moo_edit_load_user_tools_type (MooUserToolType type)
{
    GSList *list;

    init_tools_environment ();

    unload_user_tools (type);

    list = _moo_edit_parse_user_tools (type, TRUE);

    while (list)
    {
        load_tool (list->data);
        _moo_user_tool_info_unref (list->data);
        list = g_slist_delete_link (list, list);
    }
}

void
_moo_edit_load_user_tools (void)
{
    _moo_edit_load_user_tools_type (MOO_USER_TOOL_MENU);
    _moo_edit_load_user_tools_type (MOO_USER_TOOL_CONTEXT);
}


static void
parse_params (MooUserToolInfo  *info,
              char            **params,
              const char       *filename)
{
    while (params && *params)
    {
        char *key = *params++;
        char *value = *params++;

        if (!strcmp (key, KEY_POSITION))
        {
            if (!g_ascii_strcasecmp (value, "end"))
                info->position = MOO_USER_TOOL_POS_END;
            else if (!g_ascii_strcasecmp (value, "start"))
                info->position = MOO_USER_TOOL_POS_START;
            else
                g_warning ("unknown position type '%s' for tool %s in file %s",
                           value, info->name, filename);
        }
        else if (!strcmp (key, KEY_ID))
        {
            g_free (info->id);
            info->id = g_strdup (value);
        }
        else if (!strcmp (key, KEY_NAME))
        {
            g_free (info->name);
            info->name = g_strdup (value);
        }
        else if (!strcmp (key, KEY_OS))
        {
            /* ignore */
        }
        else if (!strcmp (key, KEY_ACCEL))
        {
            g_free (info->accel);
            info->accel = g_strdup (value);
        }
        else if (!strcmp (key, KEY_MENU))
        {
            g_free (info->menu);
            info->menu = g_strdup (value);
        }
        else if (!strcmp (key, KEY_LANGS))
        {
            g_free (info->filter);
            info->filter = g_strdup_printf ("langs: %s", value);
        }
        else if (!strcmp (key, KEY_FILTER))
        {
            g_free (info->filter);
            info->filter = g_strdup (value);
        }
        else if (!strcmp (key, KEY_OPTIONS))
        {
            g_free (info->options);
            info->options = g_strdup (value);
        }
        else
        {
            g_warning ("%s: unknown option %s=%s in file %s",
                       G_STRLOC, key, value, filename);
        }
    }
}

static void
load_file (const char      *filename,
           const char      *basename,
           MooUserToolType  type,
           GSList         **list,
           GHashTable      *ids)
{
    MooUserToolInfo *info;
    MooCommandData *cmd_data;
    MooCommandFactory *cmd_factory;
    char **params;
    guint suffix_len = 0;

    cmd_data = _moo_command_parse_file (filename, &cmd_factory, &params);
    if (!cmd_data)
        return;

    if (cmd_factory->extension && g_str_has_suffix (basename, cmd_factory->extension))
        suffix_len = strlen (cmd_factory->extension);

    info = _moo_user_tool_info_new ();
    info->type = type;
    info->file = g_strdup (filename);
    info->id = g_strndup (basename + NAME_PREFIX_LEN,
                          strlen (basename + NAME_PREFIX_LEN - suffix_len));
    info->name = g_strdup (info->id);
    info->enabled = TRUE;
    info->cmd_data = cmd_data;
    info->cmd_factory = cmd_factory;

    parse_params (info, params, filename);
    add_info (info, list, ids);

    g_strfreev (params);
}

static int
cmp_filenames (const void *p1,
               const void *p2)
{
    const char *const *sp1 = p1;
    const char *const *sp2 = p2;
    return strcmp (*sp1, *sp2);
}

static void
load_directory (const char       *path,
                MooUserToolType   type,
                GSList          **list,
                GHashTable       *ids)
{
    GDir *dir;
    GError *error = NULL;
    const char *name;
    GPtrArray *names;
    guint i;

    dir = g_dir_open (path, 0, &error);

    if (!dir)
    {
        g_warning ("%s: %s", G_STRLOC, error->message);
        g_error_free (error);
        return;
    }

    names = g_ptr_array_new ();

    while ((name = g_dir_read_name (dir)))
        if (strlen (name) > NAME_PREFIX_LEN)
            g_ptr_array_add (names, g_strdup (name));

    g_dir_close (dir);

    qsort (names->pdata, names->len, sizeof (char*), cmp_filenames);

    for (i = 0; i < names->len; ++i)
    {
        char *filename;

        filename = g_build_filename (path, names->pdata[i], NULL);
        load_file (filename, names->pdata[i], type, list, ids);

        g_free (filename);
        g_free (names->pdata[i]);
    }

    g_ptr_array_free (names, TRUE);
}

static void
load_directories (MooUserToolType   type,
                  GSList          **list,
                  GHashTable       *ids)
{
    char **dirs, **p;

    dirs = moo_get_data_subdirs (type == MOO_USER_TOOL_MENU ? "tools" : "tools-context");
    dirs = _moo_strv_reverse (dirs);

    for (p = dirs; p && *p; ++p)
        if (g_file_test (*p, G_FILE_TEST_IS_DIR))
            load_directory (*p, type, list, ids);

    g_strfreev (dirs);
}


static MooUserToolInfo *
parse_item (MooKeyFileItem  *item,
            MooUserToolType  type,
            const char      *file)
{
    char *position = NULL;
    MooUserToolInfo *info;
    char *langs;

    if (strcmp (moo_key_file_item_name (item), ITEM_TOOL))
    {
        g_warning ("invalid group %s in file %s", moo_key_file_item_name (item), file);
        return NULL;
    }

    info = _moo_user_tool_info_new ();
    info->type = type;
    info->file = g_strdup (file);
    info->position = MOO_USER_TOOL_POS_END;
    info->id = moo_key_file_item_steal (item, KEY_ID);
    info->name = moo_key_file_item_steal (item, KEY_NAME);
    info->enabled = moo_key_file_item_steal_bool (item, KEY_ENABLED, TRUE);
    info->deleted = moo_key_file_item_steal_bool (item, KEY_DELETED, FALSE);
    info->builtin = moo_key_file_item_steal_bool (item, KEY_BUILTIN, FALSE);

    if (!info->id)
    {
        g_warning ("tool id missing in file %s", file);
        _moo_user_tool_info_unref (info);
        return NULL;
    }

    if (info->deleted || info->builtin)
        return info;

    info->accel = moo_key_file_item_steal (item, KEY_ACCEL);
    info->menu = moo_key_file_item_steal (item, KEY_MENU);
    position = moo_key_file_item_steal (item, KEY_POSITION);

    if (position)
    {
        if (!g_ascii_strcasecmp (position, "end"))
            info->position = MOO_USER_TOOL_POS_END;
        else if (!g_ascii_strcasecmp (position, "start"))
            info->position = MOO_USER_TOOL_POS_START;
        else
            g_warning ("unknown position type '%s' for tool %s in file %s",
                       position, info->name, file);

        g_free (position);
    }

    if ((langs = moo_key_file_item_steal (item, KEY_LANGS)))
    {
        info->filter = g_strdup_printf ("langs: %s", langs);
        g_free (langs);
    }
    else
    {
        info->filter = moo_key_file_item_steal (item, KEY_FILTER);
    }

    info->cmd_data = _moo_command_parse_item (item, info->name, file,
                                              &info->cmd_factory,
                                              &info->options);

    if (!info->cmd_data)
    {
        _moo_user_tool_info_unref (info);
        return NULL;
    }

    return info;
}


static GSList *
parse_key_file (MooKeyFile      *key_file,
                MooUserToolType  type,
                const char      *filename)
{
    guint n_items, i;
    GSList *list = NULL;

    n_items = moo_key_file_n_items (key_file);

    for (i = 0; i < n_items; ++i)
    {
        MooKeyFileItem *item = moo_key_file_nth_item (key_file, i);
        MooUserToolInfo *info = parse_item (item, type, filename);
        if (info)
            list = g_slist_prepend (list, info);
    }

    return g_slist_reverse (list);
}


static GSList *
parse_file_simple (const char     *filename,
                   MooUserToolType type)
{
    MooKeyFile *key_file;
    GError *error = NULL;
    GSList *list = NULL;

    g_return_val_if_fail (filename != NULL, NULL);
    g_return_val_if_fail (type < N_TOOLS, NULL);

    key_file = moo_key_file_new_from_file (filename, &error);

    if (key_file)
    {
        list = parse_key_file (key_file, type, filename);
        moo_key_file_unref (key_file);
    }
    else
    {
        g_warning ("could not load file '%s': %s", filename, error->message);
        g_error_free (error);
    }

    return list;
}

static void
add_info (MooUserToolInfo *info,
          GSList        **list,
          GHashTable     *ids)
{
    MooUserToolInfo *old_info;
    GSList *old_link;

    old_link = g_hash_table_lookup (ids, info->id);
    old_info = old_link ? old_link->data : NULL;

    if (old_link)
    {
        *list = g_slist_delete_link (*list, old_link);
        g_hash_table_remove (ids, info->id);
    }

    if (info->deleted)
    {
        _moo_user_tool_info_unref (info);
        info = NULL;
    }
    else if (info->builtin)
    {
        _moo_user_tool_info_unref (info);
        info = old_info ? _moo_user_tool_info_ref (old_info) : NULL;
    }

    if (info)
    {
        *list = g_slist_prepend (*list, info);
        g_hash_table_insert (ids, g_strdup (info->id), *list);
    }

    if (old_info)
        _moo_user_tool_info_unref (old_info);
}

static void
parse_file (const char     *filename,
            MooUserToolType type,
            GSList        **list,
            GHashTable     *ids)
{
    GSList *new_list;

    new_list = parse_file_simple (filename, type);

    if (!new_list)
        return;

    while (new_list)
    {
        MooUserToolInfo *info;

        info = new_list->data;
        g_return_if_fail (info->id != NULL);

        add_info (info, list, ids);
        new_list = g_slist_delete_link (new_list, new_list);
    }
}

static GSList *
parse_user_tools (MooUserToolType type,
                  GHashTable    **ids_p,
                  gboolean        sys_only,
                  gboolean        all)
{
    char **sys_files, *user_file;
    GSList *list = NULL;
    GHashTable *ids = NULL;
    char **p;

    g_return_val_if_fail (type < N_TOOLS, NULL);

    _moo_command_init ();

    find_user_tools_file (type, &sys_files, &user_file);
    ids = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

    for (p = sys_files; p && *p; ++p)
        parse_file (*p, type, &list, ids);

    if (!sys_only && user_file)
        parse_file (user_file, type, &list, ids);

    if (all)
        load_directories (type, &list, ids);

    if (ids_p)
        *ids_p = ids;
    else
        g_hash_table_destroy (ids);

    g_strfreev (sys_files);
    g_free (user_file);

    return g_slist_reverse (list);
}

GSList *
_moo_edit_parse_user_tools (MooUserToolType type,
                            gboolean        all)
{
    g_return_val_if_fail (type < N_TOOLS, NULL);
    return parse_user_tools (type, NULL, FALSE, all);
}


static void
generate_id (MooUserToolInfo *info,
             GHashTable      *ids)
{
    guint i;

    g_return_if_fail (info->id == NULL);

    for (i = 0; ; ++i)
    {
        char *id = g_strdup_printf ("MooUserTool%u", i);

        if (!g_hash_table_lookup (ids, id))
        {
            info->id = id;
            break;
        }

        g_free (id);
    }
}

static void
assign_missing_ids (GSList *list)
{
    GSList *l;
    GHashTable *ids;

    ids = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

    for (l = list; l != NULL; l = l->next)
    {
        MooUserToolInfo *info = l->data;

        if (info->id)
            g_hash_table_insert (ids, g_strdup (info->id), l);
    }

    for (l = list; l != NULL; l = l->next)
    {
        MooUserToolInfo *info = l->data;

        if (!info->id)
        {
            generate_id (info, ids);
            g_hash_table_insert (ids, g_strdup (info->id), l);
        }
    }

    g_hash_table_destroy (ids);
}

static void
add_deleted (const char *id,
             G_GNUC_UNUSED GSList *link,
             GSList    **list)
{
    MooUserToolInfo *info;

    g_assert (!strcmp (((MooUserToolInfo*)link->data)->id, id));

    info = _moo_user_tool_info_new ();
    info->id = g_strdup (id);
    info->deleted = TRUE;

    *list = g_slist_prepend (*list, info);
}

static gboolean
info_equal (MooUserToolInfo *info1,
            MooUserToolInfo *info2)
{
    if (info1 == info2)
        return TRUE;

    if (!info1->enabled != !info2->enabled)
        return FALSE;

    return info1->position == info2->position &&
           info1->position == info2->position &&
           info1->position == info2->position &&
           info1->cmd_factory == info2->cmd_factory &&
           _moo_str_equal (info1->name, info2->name) &&
           _moo_str_equal (info1->accel, info2->accel) &&
           _moo_str_equal (info1->menu, info2->menu) &&
           _moo_str_equal (info1->filter, info2->filter) &&
           _moo_str_equal (info1->options, info2->options) &&
           _moo_command_factory_data_equal (info1->cmd_factory, info1->cmd_data, info2->cmd_data);
}

static GSList *
generate_real_list (MooUserToolType  type,
                    GSList          *list)
{
    GSList *real_list = NULL;
    GSList *sys_list;
    GHashTable *sys_ids;

    sys_list = parse_user_tools (type, &sys_ids, TRUE, FALSE);
    assign_missing_ids (list);

    while (list)
    {
        MooUserToolInfo *new_info = NULL;
        MooUserToolInfo *info = list->data;
        GSList *sys_link;

        sys_link = g_hash_table_lookup (sys_ids, info->id);

        if (sys_link)
        {
            MooUserToolInfo *sys_info = sys_link->data;

            if (info_equal (sys_info, info))
            {
                new_info = _moo_user_tool_info_new ();
                new_info->id = g_strdup (sys_info->id);
                new_info->builtin = TRUE;
            }
            else
            {
                new_info = _moo_user_tool_info_ref (info);
            }

            g_hash_table_remove (sys_ids, info->id);
        }
        else
        {
            new_info = _moo_user_tool_info_ref (info);
        }

        real_list = g_slist_prepend (real_list, new_info);
        list = list->next;
    }

    g_hash_table_foreach (sys_ids, (GHFunc) add_deleted, &real_list);

    g_hash_table_destroy (sys_ids);
    g_slist_foreach (sys_list, (GFunc) _moo_user_tool_info_unref, NULL);
    g_slist_free (sys_list);

    return g_slist_reverse (real_list);
}

void
_moo_edit_save_user_tools (MooUserToolType  type,
                           GSList          *user_list)
{
    MooKeyFile *key_file;
    GError *error = NULL;
    char *string;
    GSList *list;

    g_return_if_fail (type < N_TOOLS);

    key_file = moo_key_file_new ();
    list = generate_real_list (type, user_list);

    while (list)
    {
        MooKeyFileItem *item;
        MooUserToolInfo *info = list->data;

        item = moo_key_file_new_item (key_file, ITEM_TOOL);

        moo_key_file_item_set (item, KEY_ID, info->id);

        if (info->deleted)
        {
            moo_key_file_item_set_bool (item, KEY_DELETED, TRUE);
        }
        else if (info->builtin)
        {
            moo_key_file_item_set_bool (item, KEY_BUILTIN, TRUE);
        }
        else
        {
            if (info->name && info->name[0])
                moo_key_file_item_set (item, KEY_NAME, info->name);
            if (info->accel && info->accel[0])
                moo_key_file_item_set (item, KEY_ACCEL, info->accel);
            if (info->menu && info->menu[0])
                moo_key_file_item_set (item, KEY_MENU, info->menu);
            if (info->filter && info->filter[0])
                moo_key_file_item_set (item, KEY_FILTER, info->filter);
            if (!info->enabled)
                moo_key_file_item_set_bool (item, KEY_ENABLED, info->enabled);
            if (info->position != MOO_USER_TOOL_POS_END)
                moo_key_file_item_set (item, KEY_POSITION, "start");

            _moo_command_format_item (item,
                                      info->cmd_data,
                                      info->cmd_factory,
                                      info->options);
        }

        _moo_user_tool_info_unref (info);
        list = g_slist_delete_link (list, list);
    }

    string = moo_key_file_format (key_file, "This file is autogenerated, do not edit\n"
                                            "-%- strip: false -%-", 2);

    if (!moo_save_user_data_file (FILENAMES[type], string, -1, &error))
    {
        g_critical ("could not save tools file: %s", error->message);
        g_error_free (error);
    }

    g_free (string);
    moo_key_file_unref (key_file);
}


MooUserToolInfo *
_moo_user_tool_info_new (void)
{
    MooUserToolInfo *info = g_new0 (MooUserToolInfo, 1);
    info->ref_count = 1;
    return info;
}


static MooUserToolInfo *
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

    g_free (info->id);
    g_free (info->name);
    g_free (info->accel);
    g_free (info->menu);
    g_free (info->filter);
    g_free (info->options);
    g_free (info->file);

    if (info->cmd_data)
        moo_command_data_unref (info->cmd_data);

    g_free (info);
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

    doc = moo_edit_action_get_doc (edit_action);

    if (doc)
    {
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
    MooEdit *doc;

    g_return_if_fail (action->cmd != NULL);

    MOO_EDIT_ACTION_CLASS (_moo_tool_action_parent_class)->check_state (edit_action);

    if (!gtk_action_is_visible (GTK_ACTION (action)))
        return;

    doc = moo_edit_action_get_doc (edit_action);
    sensitive = moo_command_check_sensitive (action->cmd, doc, moo_edit_get_window (doc));

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
                                                          (GParamFlags) G_PARAM_READWRITE));
}


static void
get_extension (const char *string,
               char      **base,
               char      **ext)
{
    char *dot;

    g_return_if_fail (string != NULL);

    dot = strrchr (string, '.');

    if (dot)
    {
        *base = g_strndup (string, dot - string);
        *ext = g_strdup (dot);
    }
    else
    {
        *base = g_strdup (string);
        *ext = g_strdup ("");
    }
}

static MooCommandContext *
create_command_context (gpointer window,
                        gpointer doc)
{
    MooCommandContext *ctx;
    char *user_dir;

    ctx = moo_command_context_new (doc, window);

    if (MOO_IS_EDIT (doc) && !moo_edit_is_untitled (doc))
    {
        char *filename, *basename;
        char *dirname, *base = NULL, *extension = NULL;

        filename = moo_edit_get_filename (doc);
        basename = filename ? g_path_get_basename (filename) : NULL;
        dirname = g_path_get_dirname (filename);
        get_extension (basename, &base, &extension);

        moo_command_context_set_string (ctx, "DOC", basename);
        moo_command_context_set_string (ctx, "DOC_DIR", dirname);
        moo_command_context_set_string (ctx, "DOC_BASE", base);
        moo_command_context_set_string (ctx, "DOC_EXT", extension);

        g_free (dirname);
        g_free (base);
        g_free (extension);
        g_free (basename);
        g_free (filename);
    }

    if (MOO_IS_EDIT (doc))
    {
        GValue val;
        val.g_type = 0;
        g_value_init (&val, G_TYPE_INT);
        g_value_set_int (&val, moo_text_view_get_cursor_line (doc));
        moo_command_context_set (ctx, "LINE0", &val);
        g_value_set_int (&val, moo_text_view_get_cursor_line (doc) + 1);
        moo_command_context_set (ctx, "LINE", &val);
        g_value_unset (&val);
    }

    user_dir = moo_get_user_data_dir ();

    moo_command_context_set_string (ctx, "DATA_DIR", user_dir);
    moo_command_context_set_string (ctx, "APP_PID", _moo_get_pid_string ());
    /* XXX */
    moo_command_context_set_string (ctx, "MEDIT_PID", _moo_get_pid_string ());

    g_free (user_dir);
    return ctx;
}

static GtkWidget *
user_tools_plugin_prefs_page (G_GNUC_UNUSED MooPlugin *plugin)
{
    return moo_user_tools_prefs_page_new ();
}


typedef MooPlugin UserToolsPlugin;

static gboolean
user_tools_plugin_init (G_GNUC_UNUSED UserToolsPlugin *plugin)
{
    _moo_edit_load_user_tools ();
    return TRUE;
}

static void
user_tools_plugin_deinit (G_GNUC_UNUSED UserToolsPlugin *plugin)
{
}

MOO_PLUGIN_DEFINE_INFO (user_tools,
                        N_("User Tools"),
                        N_("Configurable tools"),
                        "Yevgen Muntyan <emuntyan@sourceforge.net>",
                        MOO_VERSION)

MOO_PLUGIN_DEFINE (UserTools, user_tools,
                   NULL, NULL, NULL, NULL,
                   user_tools_plugin_prefs_page,
                   0, 0)

gboolean
_moo_user_tools_plugin_init (void)
{
    MooPluginParams params = { TRUE, FALSE };
    return moo_plugin_register (MOO_USER_TOOLS_PLUGIN_ID,
                                user_tools_plugin_get_type (),
                                &user_tools_plugin_info,
                                &params);
}
