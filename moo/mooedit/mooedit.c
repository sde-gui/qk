/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mooedit.c
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

#define MOOEDIT_COMPILATION
#include "mooedit/mooedit-actions.h"
#include "mooedit/mooedit-private.h"
#include "mooedit/mootextview-private.h"
#include "mooedit/mooeditdialogs.h"
#include "mooedit/mooeditprefs.h"
#include "mooedit/mootextbuffer.h"
#include "mooutils/moomarshals.h"
#include "mooutils/moocompat.h"
#include "mooutils/mooutils-gobject.h"
#include <string.h>


GSList *_moo_edit_instances = NULL;


static GObject *moo_edit_constructor        (GType                  type,
                                             guint                  n_construct_properties,
                                             GObjectConstructParam *construct_param);
static void     moo_edit_finalize           (GObject        *object);
static void     moo_edit_dispose            (GObject        *object);

static void     moo_edit_set_property       (GObject        *object,
                                             guint           prop_id,
                                             const GValue   *value,
                                             GParamSpec     *pspec);
static void     moo_edit_get_property       (GObject        *object,
                                             guint           prop_id,
                                             GValue         *value,
                                             GParamSpec     *pspec);

static gboolean moo_edit_popup_menu         (GtkWidget      *widget);

static void     moo_edit_filename_changed   (MooEdit        *edit,
                                             const char     *new_filename);
static gboolean moo_edit_line_mark_clicked  (MooTextView    *view,
                                             int             line);

static void     config_changed              (MooEdit        *edit,
                                             GParamSpec     *pspec);
static void     moo_edit_config_notify      (MooEdit        *edit,
                                             guint           var_id,
                                             GParamSpec     *pspec);

static GtkTextBuffer *get_buffer            (MooEdit        *edit);
static MooTextBuffer *get_moo_buffer        (MooEdit        *edit);

static void     modified_changed_cb         (GtkTextBuffer  *buffer,
                                             MooEdit        *edit);

static void     disconnect_bookmark         (MooEditBookmark *bk);
static void     line_mark_moved             (MooEdit        *edit,
                                             MooLineMark    *mark);
static void     line_mark_deleted           (MooEdit        *edit,
                                             MooLineMark    *mark);


enum {
    DOC_STATUS_CHANGED,
    FILENAME_CHANGED,
    COMMENT,
    UNCOMMENT,
    CONFIG_NOTIFY,
    SAVE_BEFORE,
    SAVE_AFTER,
    BOOKMARKS_CHANGED,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

enum {
    PROP_0,
    PROP_EDITOR,
    PROP_ENABLE_BOOKMARKS,
    PROP_HAS_COMMENTS
};

enum {
    SETTING_LANG,
    SETTING_INDENT,
    SETTING_STRIP,
    LAST_SETTING
};

static guint settings[LAST_SETTING];

/* MOO_TYPE_EDIT */
G_DEFINE_TYPE (MooEdit, moo_edit, MOO_TYPE_TEXT_VIEW)


static void
moo_edit_class_init (MooEditClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->set_property = moo_edit_set_property;
    gobject_class->get_property = moo_edit_get_property;
    gobject_class->constructor = moo_edit_constructor;
    gobject_class->finalize = moo_edit_finalize;
    gobject_class->dispose = moo_edit_dispose;

    MOO_TEXT_VIEW_CLASS(klass)->line_mark_clicked = moo_edit_line_mark_clicked;
    GTK_WIDGET_CLASS(klass)->popup_menu = moo_edit_popup_menu;

    klass->filename_changed = moo_edit_filename_changed;
    klass->config_notify = moo_edit_config_notify;

    g_object_class_install_property (gobject_class,
                                     PROP_EDITOR,
                                     g_param_spec_object ("editor",
                                             "editor",
                                             "editor",
                                             MOO_TYPE_EDITOR,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (gobject_class,
                                     PROP_ENABLE_BOOKMARKS,
                                     g_param_spec_boolean ("enable-bookmarks",
                                             "enable-bookmarks",
                                             "enable-bookmarks",
                                             FALSE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_HAS_COMMENTS,
                                     g_param_spec_boolean ("has-comments",
                                             "has-comments",
                                             "has-comments",
                                             FALSE,
                                             G_PARAM_READABLE));

    settings[SETTING_LANG] = moo_edit_config_install_setting (
            g_param_spec_string ("lang", "lang", "lang",
                                 NULL,
                                 G_PARAM_READWRITE));
    settings[SETTING_INDENT] = moo_edit_config_install_setting (
            g_param_spec_string ("indent", "indent", "indent",
                                 NULL,
                                 G_PARAM_READWRITE));
    settings[SETTING_STRIP] = moo_edit_config_install_setting (
            g_param_spec_boolean ("strip", "strip", "strip",
                                  FALSE,
                                  G_PARAM_READWRITE));

    _moo_edit_class_init_actions (klass);

    signals[CONFIG_NOTIFY] =
            g_signal_new ("config-notify",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_FIRST | G_SIGNAL_DETAILED,
                          G_STRUCT_OFFSET (MooEditClass, config_notify),
                          NULL, NULL,
                          _moo_marshal_VOID__UINT_POINTER,
                          G_TYPE_NONE, 2,
                          G_TYPE_UINT, G_TYPE_POINTER);

    signals[DOC_STATUS_CHANGED] =
            g_signal_new ("doc-status-changed",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooEditClass, doc_status_changed),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[FILENAME_CHANGED] =
            g_signal_new ("filename-changed",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_FIRST,
                          G_STRUCT_OFFSET (MooEditClass, filename_changed),
                          NULL, NULL,
                          _moo_marshal_VOID__STRING,
                          G_TYPE_NONE, 1,
                          G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE);

    signals[COMMENT] =
            moo_signal_new_cb ("comment",
                               G_OBJECT_CLASS_TYPE (klass),
                               G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                               G_CALLBACK (moo_edit_comment),
                               NULL, NULL,
                               _moo_marshal_VOID__VOID,
                               G_TYPE_NONE, 0);

    signals[UNCOMMENT] =
            moo_signal_new_cb ("uncomment",
                               G_OBJECT_CLASS_TYPE (klass),
                               G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                               G_CALLBACK (moo_edit_uncomment),
                               NULL, NULL,
                               _moo_marshal_VOID__VOID,
                               G_TYPE_NONE, 0);

    signals[SAVE_BEFORE] =
            g_signal_new ("save-before",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_FIRST,
                          G_STRUCT_OFFSET (MooEditClass, save_before),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[SAVE_AFTER] =
            g_signal_new ("save-after",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_FIRST,
                          G_STRUCT_OFFSET (MooEditClass, save_after),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[BOOKMARKS_CHANGED] =
            g_signal_new ("bookmarks-changed",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooEditClass, bookmarks_changed),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    _moo_edit_init_settings ();
}


static void
moo_edit_init (MooEdit *edit)
{
    MooIndenter *indent;

    edit->config = moo_edit_config_new ();
    g_signal_connect_swapped (edit->config, "notify",
                              G_CALLBACK (config_changed), edit);

    edit->priv = g_new0 (MooEditPrivate, 1);

    edit->priv->file_watch_policy = MOO_EDIT_RELOAD_IF_SAFE;

    edit->priv->actions = moo_action_group_new ("MooEdit");

    indent = moo_indenter_new (edit, NULL);
    moo_text_view_set_indenter (MOO_TEXT_VIEW (edit), indent);
    g_object_unref (indent);

    g_object_set (edit, "draw-tabs", TRUE, "draw-trailing-spaces", TRUE, NULL);
}


static GObject*
moo_edit_constructor (GType                  type,
                      guint                  n_construct_properties,
                      GObjectConstructParam *construct_param)
{
    GObject *object;
    MooEdit *edit;
    GtkTextBuffer *buffer;

    object = G_OBJECT_CLASS (moo_edit_parent_class)->constructor (
        type, n_construct_properties, construct_param);

    edit = MOO_EDIT (object);

    _moo_edit_add_class_actions (edit);
    _moo_edit_instances = g_slist_prepend (_moo_edit_instances, edit);

    edit->priv->modified_changed_handler_id =
            g_signal_connect (get_buffer (edit),
                              "modified-changed",
                              G_CALLBACK (modified_changed_cb),
                              edit);

    _moo_edit_set_filename (edit, NULL, NULL);

    buffer = get_buffer (edit);
    g_signal_connect_swapped (buffer, "line-mark-moved",
                              G_CALLBACK (line_mark_moved),
                              edit);
    g_signal_connect_swapped (buffer, "line-mark-deleted",
                              G_CALLBACK (line_mark_deleted),
                              edit);

//     g_object_set (edit, "enable-folding", TRUE, NULL);

    return object;
}


static void
moo_edit_finalize (GObject *object)
{
    MooEdit *edit = MOO_EDIT (object);

    g_free (edit->priv->filename);
    g_free (edit->priv->basename);
    g_free (edit->priv->display_filename);
    g_free (edit->priv->display_basename);
    g_free (edit->priv->encoding);
    g_free (edit->priv);
    edit->priv = NULL;

    G_OBJECT_CLASS (moo_edit_parent_class)->finalize (object);
}


static void
moo_edit_dispose (GObject *object)
{
    MooEdit *edit = MOO_EDIT (object);

    _moo_edit_instances = g_slist_remove (_moo_edit_instances, edit);

    if (edit->config)
    {
        g_signal_handlers_disconnect_by_func (edit->config,
                                              (gpointer) config_changed,
                                              edit);
        g_object_unref (edit->config);
        edit->config = NULL;
    }

    if (edit->priv->apply_config_idle)
    {
        g_source_remove (edit->priv->apply_config_idle);
        edit->priv->apply_config_idle = 0;
    }

    edit->priv->focus_in_handler_id = 0;

    if (edit->priv->file_monitor_id)
    {
        _moo_edit_stop_file_watch (edit);
        edit->priv->file_monitor_id = 0;
    }

    if (edit->priv->update_bookmarks_idle)
    {
        g_source_remove (edit->priv->update_bookmarks_idle);
        edit->priv->update_bookmarks_idle = 0;
    }

    if (edit->priv->bookmarks)
    {
        g_slist_foreach (edit->priv->bookmarks, (GFunc) disconnect_bookmark, NULL);
        g_slist_foreach (edit->priv->bookmarks, (GFunc) g_object_unref, NULL);
        g_slist_free (edit->priv->bookmarks);
        edit->priv->bookmarks = NULL;
    }

    if (edit->priv->menu)
    {
        g_object_unref (edit->priv->menu);
        edit->priv->menu = NULL;
    }

    if (edit->priv->actions)
    {
        g_object_unref (edit->priv->actions);
        edit->priv->actions = NULL;
    }

    G_OBJECT_CLASS (moo_edit_parent_class)->dispose (object);
}


MooEdit*
_moo_edit_new (MooEditor  *editor)
{
    MooEdit *edit = MOO_EDIT (g_object_new (MOO_TYPE_EDIT,
                                            "editor", editor,
                                            NULL));
    return edit;
}


static void
modified_changed_cb (GtkTextBuffer      *buffer,
                     MooEdit            *edit)
{
    moo_edit_set_modified (edit, gtk_text_buffer_get_modified (buffer));
}


void
moo_edit_set_modified (MooEdit            *edit,
                       gboolean            modified)
{
    gboolean buf_modified;
    GtkTextBuffer *buffer;

    g_return_if_fail (MOO_IS_EDIT (edit));

    buffer = get_buffer (edit);

    buf_modified =
            gtk_text_buffer_get_modified (buffer);

    if (buf_modified != modified)
    {
        g_signal_handler_block (buffer,
                                edit->priv->modified_changed_handler_id);
        gtk_text_buffer_set_modified (buffer, modified);
        g_signal_handler_unblock (buffer,
                                  edit->priv->modified_changed_handler_id);
    }

    if (modified)
        edit->priv->status |= MOO_EDIT_MODIFIED;
    else
        edit->priv->status &= ~MOO_EDIT_MODIFIED;

    moo_edit_status_changed (edit);
}


void
moo_edit_set_clean (MooEdit            *edit,
                    gboolean            clean)
{
    g_return_if_fail (MOO_IS_EDIT (edit));
    if (clean)
        edit->priv->status |= MOO_EDIT_CLEAN;
    else
        edit->priv->status &= ~MOO_EDIT_CLEAN;
    moo_edit_status_changed (edit);
}


gboolean
moo_edit_get_clean (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    return (edit->priv->status & MOO_EDIT_CLEAN) ? TRUE : FALSE;
}


void
moo_edit_status_changed (MooEdit *edit)
{
    g_return_if_fail (MOO_IS_EDIT (edit));
    g_signal_emit (edit, signals[DOC_STATUS_CHANGED], 0, NULL);
}


MooEditFileInfo*
moo_edit_file_info_new (const char         *filename,
                        const char         *encoding)
{
    MooEditFileInfo *info = g_new0 (MooEditFileInfo, 1);
    info->filename = g_strdup (filename);
    info->encoding = g_strdup (encoding);
    return info;
}


MooEditFileInfo*
moo_edit_file_info_copy (const MooEditFileInfo  *info)
{
    MooEditFileInfo *copy;
    g_return_val_if_fail (info != NULL, NULL);
    copy = g_new (MooEditFileInfo, 1);
    copy->encoding = g_strdup (info->encoding);
    copy->filename = g_strdup (info->filename);
    return copy;
}

void
moo_edit_file_info_free (MooEditFileInfo    *info)
{
    if (info)
    {
        g_free (info->encoding);
        g_free (info->filename);
        g_free (info);
    }
}


gboolean
moo_edit_is_empty (MooEdit *edit)
{
    GtkTextIter start, end;

    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);

    if (MOO_EDIT_IS_MODIFIED (edit) || edit->priv->filename)
        return FALSE;

    gtk_text_buffer_get_bounds (get_buffer (edit), &start, &end);

    return !gtk_text_iter_compare (&start, &end);
}


MooEditStatus
moo_edit_get_status (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), 0);
    return edit->priv->status;
}


static void
moo_edit_set_property (GObject        *object,
                       guint           prop_id,
                       const GValue   *value,
                       GParamSpec     *pspec)
{
    MooEdit *edit = MOO_EDIT (object);

    switch (prop_id)
    {
        case PROP_EDITOR:
            edit->priv->editor = g_value_get_object (value);
            break;

        case PROP_ENABLE_BOOKMARKS:
            moo_edit_set_enable_bookmarks (edit, g_value_get_boolean (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static void
moo_edit_get_property (GObject        *object,
                       guint           prop_id,
                       GValue         *value,
                       GParamSpec     *pspec)
{
    MooEdit *edit = MOO_EDIT (object);

    switch (prop_id)
    {
        case PROP_EDITOR:
            g_value_set_object (value, edit->priv->editor);
            break;

        case PROP_ENABLE_BOOKMARKS:
            g_value_set_boolean (value, edit->priv->enable_bookmarks);
            break;

        case PROP_HAS_COMMENTS:
            g_value_set_boolean (value, _moo_edit_has_comments (edit, NULL, NULL));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


GType
moo_edit_status_get_type (void)
{
    static GType type = 0;

    if (!type)
    {
        static const GFlagsValue values[] = {
            { MOO_EDIT_MODIFIED_ON_DISK, (char*)"MOO_EDIT_MODIFIED_ON_DISK", (char*)"modified-on-disk" },
            { MOO_EDIT_DELETED, (char*)"MOO_EDIT_DELETED", (char*)"deleted" },
            { MOO_EDIT_CHANGED_ON_DISK, (char*)"MOO_EDIT_CHANGED_ON_DISK", (char*)"changed-on-disk" },
            { MOO_EDIT_MODIFIED, (char*)"MOO_EDIT_MODIFIED", (char*)"modified" },
            { MOO_EDIT_CLEAN, (char*)"MOO_EDIT_CLEAN", (char*)"clean" },
            { 0, NULL, NULL }
        };
        type = g_flags_register_static ("MooEditStatus", values);
    }

    return type;
}


GType
moo_edit_on_external_changes_get_type (void)
{
    static GType type = 0;

    if (!type)
    {
        static const GEnumValue values[] = {
            { MOO_EDIT_DONT_WATCH_FILE, (char*)"MOO_EDIT_DONT_WATCH_FILE", (char*)"dont-watch-file" },
            { MOO_EDIT_ALWAYS_ALERT, (char*)"MOO_EDIT_ALWAYS_ALERT", (char*)"always-alert" },
            { MOO_EDIT_ALWAYS_RELOAD, (char*)"MOO_EDIT_ALWAYS_RELOAD", (char*)"always-reload" },
            { MOO_EDIT_RELOAD_IF_SAFE, (char*)"MOO_EDIT_RELOAD_IF_SAFE", (char*)"reload-if-safe" },
            { 0, NULL, NULL }
        };
        type = g_enum_register_static ("MooEditOnExternalChanges", values);
    }

    return type;
}


GType
moo_edit_file_info_get_type (void)
{
    static GType type = 0;
    if (!type)
        type = g_boxed_type_register_static ("MooEditFileInfo",
                                             (GBoxedCopyFunc) moo_edit_file_info_copy,
                                             (GBoxedFreeFunc) moo_edit_file_info_free);
    return type;
}


const char*
moo_edit_get_filename (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    return edit->priv->filename;
}

const char*
moo_edit_get_basename (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    return edit->priv->basename;
}

const char*
moo_edit_get_display_filename (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    return edit->priv->display_filename;
}

const char*
moo_edit_get_display_basename (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    return edit->priv->display_basename;
}

const char*
moo_edit_get_encoding (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    return edit->priv->encoding;
}


char*
moo_edit_get_uri (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);

    if (edit->priv->filename)
        return g_filename_to_uri (edit->priv->filename, NULL, NULL);
    else
        return NULL;
}


static GtkTextBuffer*
get_buffer (MooEdit *edit)
{
    return gtk_text_view_get_buffer (GTK_TEXT_VIEW (edit));
}


static MooTextBuffer *
get_moo_buffer (MooEdit *edit)
{
    return MOO_TEXT_BUFFER (get_buffer (edit));
}


MooEditor *
moo_edit_get_editor (MooEdit *doc)
{
    g_return_val_if_fail (MOO_IS_EDIT (doc), NULL);
    return doc->priv->editor;
}


typedef void (*SetVarFunc) (MooEdit *edit,
                            char    *name,
                            char    *val);

static void
parse_mode_string (MooEdit    *edit,
                   char       *string,
                   const char *var_val_separator,
                   SetVarFunc  func)
{
    char **vars, **p;

    vars = NULL;

    g_strstrip (string);

    vars = g_strsplit (string, ";", 0);

    if (!vars)
        goto out;

    for (p = vars; *p != NULL; p++)
    {
        char *sep, *var, *value;

        g_strstrip (*p);
        sep = strstr (*p, var_val_separator);

        if (!sep || sep == *p || !sep[1])
            goto out;

        var = g_strndup (*p, sep - *p);
        g_strstrip (var);

        if (!var[0])
        {
            g_free (var);
            goto out;
        }

        value = sep + 1;
        g_strstrip (value);

        if (!value)
        {
            g_free (var);
            goto out;
        }

        func (edit, var, value);

        g_free (var);
    }

out:
    g_strfreev (vars);
}


static void
set_kate_var (MooEdit    *edit,
              const char *name,
              const char *val)
{
    if (!g_ascii_strcasecmp (name, "space-indent"))
    {
        gboolean spaces = FALSE;

        if (moo_edit_config_parse_bool (val, &spaces))
            moo_edit_config_parse (edit->config, "indent-use-tabs",
                                   spaces ? "false" : "true",
                                   MOO_EDIT_CONFIG_SOURCE_FILE);
    }
    else
    {
        moo_edit_config_parse (edit->config, name, val,
                               MOO_EDIT_CONFIG_SOURCE_FILE);
    }
}

static void
parse_kate_mode_string (MooEdit *edit,
                        char    *string)
{
    parse_mode_string (edit, string, " ", (SetVarFunc) set_kate_var);
}


static void
set_emacs_var (MooEdit *edit,
               char    *name,
               char    *val)
{
    if (!g_ascii_strcasecmp (name, "mode"))
    {
        moo_edit_config_parse (edit->config, "lang", val,
                               MOO_EDIT_CONFIG_SOURCE_FILE);
    }
    else if (!g_ascii_strcasecmp (name, "tab-width"))
    {
        moo_edit_config_parse (edit->config, "indent-tab-width", val,
                               MOO_EDIT_CONFIG_SOURCE_FILE);
    }
    else if (!g_ascii_strcasecmp (name, "c-basic-offset"))
    {
        moo_edit_config_parse (edit->config, "indent-width", val,
                               MOO_EDIT_CONFIG_SOURCE_FILE);
    }
    else if (!g_ascii_strcasecmp (name, "indent-tabs-mode"))
    {
        if (!g_ascii_strcasecmp (val, "nil"))
            moo_edit_config_parse (edit->config, "indent-use-tabs", "false",
                                   MOO_EDIT_CONFIG_SOURCE_FILE);
        else
            moo_edit_config_parse (edit->config, "indent-use-tabs", "true",
                                   MOO_EDIT_CONFIG_SOURCE_FILE);
    }
}

static void
parse_emacs_mode_string (MooEdit *edit,
                         char    *string)
{
    parse_mode_string (edit, string, ":", set_emacs_var);
}


static void
set_moo_var (MooEdit *edit,
             char    *name,
             char    *val)
{
    moo_edit_config_parse (edit->config, name, val,
                           MOO_EDIT_CONFIG_SOURCE_FILE);
}

static void
parse_moo_mode_string (MooEdit *edit,
                       char    *string)
{
    parse_mode_string (edit, string, " ", (SetVarFunc) set_moo_var);
}


#define KATE_MODE_STRING        "kate:"
#define EMACS_MODE_STRING       "-*-"
#define MOO_MODE_STRING         "-%-"

static void
try_mode_string (MooEdit    *edit,
                 char       *string)
{
    char *start, *end;

    if ((start = strstr (string, KATE_MODE_STRING)))
    {
        start += strlen (KATE_MODE_STRING);
        return parse_kate_mode_string (edit, start);
    }

    if ((start = strstr (string, EMACS_MODE_STRING)))
    {
        start += strlen (EMACS_MODE_STRING);

        if ((end = strstr (start, EMACS_MODE_STRING)) && end > start)
        {
            end[0] = 0;
            return parse_emacs_mode_string (edit, start);
        }
    }

    if ((start = strstr (string, MOO_MODE_STRING)))
    {
        start += strlen (MOO_MODE_STRING);

        if ((end = strstr (start, MOO_MODE_STRING)) && end > start)
        {
            end[0] = 0;
            return parse_moo_mode_string (edit, start);
        }
    }
}

static void
try_mode_strings (MooEdit *edit)
{
    GtkTextBuffer *buffer = get_buffer (edit);
    GtkTextIter start, end;
    char *first, *last;

    gtk_text_buffer_get_start_iter (buffer, &start);
    end = start;
    gtk_text_iter_forward_to_line_end (&end);
    first = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);

    gtk_text_buffer_get_end_iter (buffer, &end);

    if (gtk_text_iter_starts_line (&end))
    {
        gtk_text_iter_backward_line (&end);
        start = end;
        gtk_text_iter_forward_to_line_end (&end);
    }
    else
    {
        start = end;
        gtk_text_iter_set_line_offset (&start, 0);
    }

    last = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);

    try_mode_string (edit, first);
    try_mode_string (edit, last);

    g_free (first);
    g_free (last);
}


static void
config_changed (MooEdit        *edit,
                GParamSpec     *pspec)
{
    guint id = moo_edit_config_get_setting_id (pspec);
    GQuark detail = g_quark_from_string (pspec->name);
    g_return_if_fail (id != 0);
    g_signal_emit (edit, signals[CONFIG_NOTIFY], detail, id, pspec);
}


static void
moo_edit_set_lang (MooEdit *edit,
                   MooLang *lang)
{
    MooLang *old_lang;
    MooEditConfig *config;

    old_lang = moo_text_view_get_lang (MOO_TEXT_VIEW (edit));

    if (old_lang == lang)
        return;

    _moo_edit_freeze_config_notify (edit);

    moo_edit_config_unset_by_source (edit->config, MOO_EDIT_CONFIG_SOURCE_LANG);
    config = moo_edit_config_get_for_lang (moo_lang_id (lang));
    moo_edit_config_compose (edit->config, config);

    moo_text_view_set_lang (MOO_TEXT_VIEW (edit), lang);

    _moo_edit_thaw_config_notify (edit);

    g_object_notify (G_OBJECT (edit), "has-comments");
}


static void
moo_edit_apply_config (MooEdit *edit)
{
    const char *lang_id = moo_edit_config_get_string (edit->config, "lang");
    MooLangMgr *mgr = moo_editor_get_lang_mgr (edit->priv->editor);
    MooLang *lang = lang_id ? moo_lang_mgr_get_lang (mgr, lang_id) : NULL;
    moo_edit_set_lang (edit, lang);
}


static gboolean
do_apply_config (MooEdit *edit)
{
    edit->priv->apply_config_idle = 0;
    moo_edit_apply_config (edit);
    return FALSE;
}

static void
moo_edit_queue_apply_config (MooEdit *edit)
{
    if (!edit->priv->apply_config_idle)
        edit->priv->apply_config_idle =
                g_idle_add ((GSourceFunc) do_apply_config, edit);
}


static void
moo_edit_config_notify (MooEdit        *edit,
                        guint           var_id,
                        G_GNUC_UNUSED GParamSpec *pspec)
{
    if (var_id == settings[SETTING_LANG])
        moo_edit_apply_config (edit);
    else
        moo_edit_queue_apply_config (edit);
}


static void
moo_edit_filename_changed (MooEdit    *edit,
                           const char *filename)
{
    MooLang *lang = NULL;
    const char *lang_id = NULL;

    _moo_edit_freeze_config_notify (edit);

    moo_edit_config_unset_by_source (edit->config, MOO_EDIT_CONFIG_SOURCE_FILE);

    if (filename)
    {
        MooLangMgr *mgr = moo_editor_get_lang_mgr (edit->priv->editor);
        lang = moo_lang_mgr_get_lang_for_file (mgr, filename);
        lang_id = lang ? lang->id : NULL;
    }

    moo_edit_config_set (edit->config, "lang", MOO_EDIT_CONFIG_SOURCE_FILENAME, lang_id, NULL);
    moo_edit_config_set (edit->config, "indent", MOO_EDIT_CONFIG_SOURCE_FILENAME, NULL, NULL);

    try_mode_strings (edit);

    _moo_edit_thaw_config_notify (edit);
}


gboolean
moo_edit_close (MooEdit        *edit,
                gboolean        ask_confirm)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    return moo_editor_close_doc (edit->priv->editor, edit, ask_confirm);
}


gboolean
moo_edit_save (MooEdit *edit,
               GError **error)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    return _moo_editor_save (edit->priv->editor, edit, error);
}


gboolean
moo_edit_save_as (MooEdit        *edit,
                  const char     *filename,
                  const char     *encoding,
                  GError        **error)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    return _moo_editor_save_as (edit->priv->editor, edit, filename, encoding, error);
}


gboolean
moo_edit_save_copy (MooEdit        *edit,
                    const char     *filename,
                    const char     *encoding,
                    GError        **error)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    return moo_editor_save_copy (edit->priv->editor, edit,
                                 filename, encoding, error);
}


void
_moo_edit_freeze_config_notify (MooEdit *edit)
{
    g_return_if_fail (MOO_IS_EDIT (edit));
    g_object_freeze_notify (G_OBJECT (edit->config));
}


void
_moo_edit_thaw_config_notify (MooEdit *edit)
{
    g_return_if_fail (MOO_IS_EDIT (edit));
    g_object_thaw_notify (G_OBJECT (edit->config));
}


/***********************************************************************/
/* Bookmarks
 */

G_DEFINE_TYPE(MooEditBookmark, moo_edit_bookmark, MOO_TYPE_LINE_MARK)


static void
moo_edit_bookmark_finalize (GObject *object)
{
    MooEditBookmark *bk = MOO_EDIT_BOOKMARK (object);
    g_free (bk->text);
    G_OBJECT_CLASS(moo_edit_bookmark_parent_class)->finalize (object);
}


static void
moo_edit_bookmark_class_init (MooEditBookmarkClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    object_class->finalize = moo_edit_bookmark_finalize;
}


static void
moo_edit_bookmark_init (MooEditBookmark *bk)
{
    g_object_set (bk,
                  "visible", TRUE,
                  "background", "#E5E5FF",
                  "stock-id", GTK_STOCK_ABOUT,
                  NULL);
}


static guint
get_line_count (MooEdit *edit)
{
    return gtk_text_buffer_get_line_count (get_buffer (edit));
}


static void
bookmarks_changed (MooEdit *edit)
{
    g_signal_emit (edit, signals[BOOKMARKS_CHANGED], 0);
}


void
moo_edit_set_enable_bookmarks (MooEdit  *edit,
                               gboolean  enable)
{
    MooTextBuffer *buffer;

    g_return_if_fail (MOO_IS_EDIT (edit));

    enable = enable != 0;

    if (enable == edit->priv->enable_bookmarks)
        return;

    edit->priv->enable_bookmarks = enable;
    moo_text_view_set_show_line_marks (MOO_TEXT_VIEW (edit), enable);
    buffer = get_moo_buffer (edit);

    if (!enable && edit->priv->bookmarks)
    {
        GSList *tmp = edit->priv->bookmarks;
        edit->priv->bookmarks = NULL;
        g_slist_foreach (tmp, (GFunc) disconnect_bookmark, NULL);

        while (tmp)
        {
            moo_text_buffer_delete_line_mark (buffer, tmp->data);
            g_object_unref (tmp->data);
            tmp = g_slist_delete_link (tmp, tmp);
        }

        bookmarks_changed (edit);
    }

    g_object_notify (G_OBJECT (edit), "enable-bookmarks");
}


gboolean
moo_edit_get_enable_bookmarks (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    return edit->priv->enable_bookmarks;
}


static int cmp_bookmarks (MooLineMark *a,
                          MooLineMark *b)
{
    int line_a = moo_line_mark_get_line (a);
    int line_b = moo_line_mark_get_line (b);
    return line_a < line_b ? -1 : (line_a > line_b ? 1 : 0);
}

static gboolean
update_bookmarks (MooEdit *edit)
{
    GSList *deleted, *dup, *old, *new, *l;

    edit->priv->update_bookmarks_idle = 0;
    old = edit->priv->bookmarks;
    edit->priv->bookmarks = NULL;

    for (deleted = NULL, new = NULL, l = old; l != NULL; l = l->next)
        if (moo_line_mark_get_deleted (MOO_LINE_MARK (l->data)))
            deleted = g_slist_prepend (deleted, l->data);
        else
            new = g_slist_prepend (new, l->data);

    g_slist_foreach (deleted, (GFunc) disconnect_bookmark, NULL);
    g_slist_foreach (deleted, (GFunc) g_object_unref, NULL);
    g_slist_free (deleted);

    new = g_slist_sort (new, (GCompareFunc) cmp_bookmarks);
    old = new;
    new = NULL;
    dup = NULL;

    for (l = old; l != NULL; l = l->next)
        if (new && moo_line_mark_get_line (new->data) == moo_line_mark_get_line (l->data))
            dup = g_slist_prepend (dup, l->data);
        else
            new = g_slist_prepend (new, l->data);

    while (dup)
    {
        disconnect_bookmark (dup->data);
        moo_text_buffer_delete_line_mark (get_moo_buffer (edit), dup->data);
        g_object_unref (dup->data);
        dup = g_slist_delete_link (dup, dup);
    }

    edit->priv->bookmarks = g_slist_reverse (new);

    return FALSE;
}


static void
update_bookmarks_now (MooEdit *edit)
{
    if (edit->priv->update_bookmarks_idle)
    {
        g_source_remove (edit->priv->update_bookmarks_idle);
        edit->priv->update_bookmarks_idle = 0;
        update_bookmarks (edit);
    }
}


const GSList *
moo_edit_list_bookmarks (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    update_bookmarks_now (edit);
    return edit->priv->bookmarks;
}


void
moo_edit_toggle_bookmark (MooEdit *edit,
                          guint    line)
{
    MooEditBookmark *bk;

    g_return_if_fail (MOO_IS_EDIT (edit));
    g_return_if_fail (line < get_line_count (edit));

    bk = moo_edit_get_bookmark_at_line (edit, line);

    if (bk)
        moo_edit_remove_bookmark (edit, bk);
    else
        moo_edit_add_bookmark (edit, line);
}


MooEditBookmark *
moo_edit_get_bookmark_at_line (MooEdit *edit,
                               guint    line)
{
    GSList *list, *l;
    MooEditBookmark *bk;

    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    g_return_val_if_fail (line < get_line_count (edit), NULL);

    bk = NULL;
    list = moo_text_buffer_get_line_marks_at_line (get_moo_buffer (edit), line);

    for (l = list; l != NULL; l = l->next)
    {
        if (MOO_IS_EDIT_BOOKMARK (l->data) && g_slist_find (edit->priv->bookmarks, l->data))
        {
            bk = l->data;
            break;
        }
    }

    g_slist_free (list);
    return bk;
}

void
moo_edit_remove_bookmark (MooEdit         *edit,
                          MooEditBookmark *bookmark)
{
    g_return_if_fail (MOO_IS_EDIT (edit));
    g_return_if_fail (MOO_IS_EDIT_BOOKMARK (bookmark));
    g_return_if_fail (g_slist_find (edit->priv->bookmarks, bookmark));

    disconnect_bookmark (bookmark);
    edit->priv->bookmarks = g_slist_remove (edit->priv->bookmarks, bookmark);
    moo_text_buffer_delete_line_mark (get_moo_buffer (edit), MOO_LINE_MARK (bookmark));

    g_object_unref (bookmark);
    bookmarks_changed (edit);
}


void
moo_edit_add_bookmark (MooEdit *edit,
                       guint    line)
{
    MooEditBookmark *bk;

    g_return_if_fail (MOO_IS_EDIT (edit));
    g_return_if_fail (line < get_line_count (edit));
    g_return_if_fail (moo_edit_get_bookmark_at_line (edit, line) == NULL);

    bk = g_object_new (MOO_TYPE_EDIT_BOOKMARK, NULL);
    moo_text_buffer_add_line_mark (get_moo_buffer (edit), MOO_LINE_MARK (bk), line);
    g_object_set_data (G_OBJECT (bk), "moo-edit-bookmark", GINT_TO_POINTER (TRUE));

    // bk->text = ???
    // background

    if (!edit->priv->update_bookmarks_idle)
        edit->priv->bookmarks =
                g_slist_insert_sorted (edit->priv->bookmarks, bk,
                                       (GCompareFunc) cmp_bookmarks);
    else
        edit->priv->bookmarks = g_slist_prepend (edit->priv->bookmarks, bk);

    bookmarks_changed (edit);
}


static void
disconnect_bookmark (MooEditBookmark *bk)
{
    g_object_set_data (G_OBJECT (bk), "moo-edit-bookmark", NULL);
}


static void
line_mark_moved (MooEdit        *edit,
                 MooLineMark    *mark)
{
    if (MOO_IS_EDIT_BOOKMARK (mark) &&
        g_object_get_data (G_OBJECT (mark), "moo-edit-bookmark") &&
        !edit->priv->update_bookmarks_idle)
    {
        edit->priv->update_bookmarks_idle =
                g_idle_add ((GSourceFunc) update_bookmarks, edit);
        bookmarks_changed (edit);
    }
}


static void
line_mark_deleted (MooEdit        *edit,
                   MooLineMark    *mark)
{
    if (MOO_IS_EDIT_BOOKMARK (mark) &&
        g_object_get_data (G_OBJECT (mark), "moo-edit-bookmark") &&
        g_slist_find (edit->priv->bookmarks, mark))
    {
        disconnect_bookmark (MOO_EDIT_BOOKMARK (mark));
        edit->priv->bookmarks = g_slist_remove (edit->priv->bookmarks, mark);
        g_object_unref (mark);
        bookmarks_changed (edit);
    }
}


static gboolean
moo_edit_line_mark_clicked (MooTextView *view,
                            int          line)
{
    moo_edit_toggle_bookmark (MOO_EDIT (view), line);
    return TRUE;
}


GSList*
moo_edit_get_bookmarks_in_range (MooEdit *edit,
                                 int      first_line,
                                 int      last_line)
{
    GSList *all, *range, *l;

    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    g_return_val_if_fail (first_line >= 0, NULL);

    if (last_line < 0 || last_line >= (int) get_line_count (edit))
        last_line = get_line_count (edit) - 1;

    if (first_line > last_line)
        return NULL;

    all = (GSList*) moo_edit_list_bookmarks (edit);

    for (l = all, range = NULL; l != NULL; l = l->next)
    {
        int line = moo_line_mark_get_line (l->data);

        if (line < first_line)
            continue;
        else if (line > last_line)
            break;
        else
            range = g_slist_prepend (range, l->data);
    }

    return g_slist_reverse (range);
}


/*****************************************************************************/
/* Comment/uncomment
 */

/* TODO: all this stuff, it's pretty lame */

gboolean
_moo_edit_has_comments (MooEdit  *edit,
                        gboolean *single_line,
                        gboolean *multi_line)
{
    MooLang *lang;
    gboolean single, multi;

    lang = moo_text_view_get_lang (MOO_TEXT_VIEW (edit));

    if (!lang)
        return FALSE;

    single = lang->line_comment != NULL;
    multi = lang->block_comment_start && lang->block_comment_end;

    if (single_line)
        *single_line = single;
    if (multi_line)
        *multi_line = multi;

    return single || multi;
}


static void
line_comment (GtkTextBuffer *buffer,
              const char    *comment_string,
              GtkTextIter   *start,
              GtkTextIter   *end)
{
    int first, last, i;
    GtkTextIter iter;
    char *comment_and_space;

    g_return_if_fail (comment_string && comment_string[0]);

    first = gtk_text_iter_get_line (start);
    last = gtk_text_iter_get_line (end);

    if (!gtk_text_iter_equal (start, end) && gtk_text_iter_starts_line (end))
        last -= 1;

    comment_and_space = g_strdup_printf ("%s ", comment_string);

    for (i = first; i <= last; ++i)
    {
        gtk_text_buffer_get_iter_at_line (buffer, &iter, i);

        if (gtk_text_iter_ends_line (&iter))
            gtk_text_buffer_insert (buffer, &iter, comment_string, -1);
        else
            gtk_text_buffer_insert (buffer, &iter, comment_and_space, -1);
    }

    g_free (comment_and_space);
}


static void
line_uncomment (GtkTextBuffer *buffer,
                const char    *comment_string,
                GtkTextIter   *start,
                GtkTextIter   *end)
{
    int first, last, i;
    guint chars;

    g_return_if_fail (comment_string && comment_string[0]);

    first = gtk_text_iter_get_line (start);
    last = gtk_text_iter_get_line (end);

    if (!gtk_text_iter_equal (start, end) && gtk_text_iter_starts_line (end))
        last -= 1;

    chars = g_utf8_strlen (comment_string, -1);

    for (i = first; i <= last; ++i)
    {
        char *text;

        gtk_text_buffer_get_iter_at_line (buffer, start, i);
        *end = *start;
        gtk_text_iter_forward_chars (end, chars);
        text = gtk_text_iter_get_slice (start, end);

        if (!strcmp (comment_string, text))
        {
            if (gtk_text_iter_get_char (end) == ' ')
                gtk_text_iter_forward_char (end);
            gtk_text_buffer_delete (buffer, start, end);
        }

        g_free (text);
    }
}


static void
iter_to_line_end (GtkTextIter *iter)
{
    if (!gtk_text_iter_ends_line (iter))
        gtk_text_iter_forward_to_line_end (iter);
}

static void
block_comment (GtkTextBuffer *buffer,
               const char    *comment_start,
               const char    *comment_end,
               GtkTextIter   *start,
               GtkTextIter   *end)
{
    GtkTextMark *end_mark;

    g_return_if_fail (comment_start && comment_start[0]);
    g_return_if_fail (comment_end && comment_end[0]);

    if (gtk_text_iter_equal (start, end))
    {
        gtk_text_iter_set_line_offset (start, 0);
        iter_to_line_end (end);
    }
    else
    {
        if (gtk_text_iter_starts_line (end))
        {
            gtk_text_iter_backward_line (end);
            iter_to_line_end (end);
        }
    }

    end_mark = gtk_text_buffer_create_mark (buffer, NULL, end, FALSE);
    gtk_text_buffer_insert (buffer, start, comment_start, -1);
    gtk_text_buffer_get_iter_at_mark (buffer, start, end_mark);
    gtk_text_buffer_insert (buffer, start, comment_end, -1);
    gtk_text_buffer_delete_mark (buffer, end_mark);
}


static void
block_uncomment (GtkTextBuffer *buffer,
                 const char    *comment_start,
                 const char    *comment_end,
                 GtkTextIter   *start,
                 GtkTextIter   *end)
{
    GtkTextIter start1, start2, end1, end2;
    GtkTextMark *mark1, *mark2;
    GtkTextIter limit;
    gboolean found;

    g_return_if_fail (comment_start && comment_start[0]);
    g_return_if_fail (comment_end && comment_end[0]);

    if (!gtk_text_iter_equal (start, end) && gtk_text_iter_starts_line (end))
    {
        gtk_text_iter_backward_line (end);
        iter_to_line_end (end);
    }

    limit = *end;
    found = moo_text_search_forward (start, comment_start, 0,
                                     &start1, &start2,
                                     &limit);

    if (!found)
    {
        gtk_text_iter_set_line_offset (&limit, 0);
        found = gtk_text_iter_backward_search (start, comment_start, 0,
                                               &start1, &start2,
                                               &limit);
    }

    if (!found)
        return;

    limit = start2;
    found = gtk_text_iter_backward_search (end, comment_end, 0,
                                           &end1, &end2, &limit);

    if (!found)
    {
        limit = *end;
        iter_to_line_end (&limit);
        found = moo_text_search_forward (end, comment_end, 0,
                                         &end1, &end2, &limit);
    }

    if (!found)
        return;

    g_assert (gtk_text_iter_compare (&start2, &end1) < 0);

    mark1 = gtk_text_buffer_create_mark (buffer, NULL, &end1, FALSE);
    mark2 = gtk_text_buffer_create_mark (buffer, NULL, &end2, FALSE);
    gtk_text_buffer_delete (buffer, &start1, &start2);
    gtk_text_buffer_get_iter_at_mark (buffer, &end1, mark1);
    gtk_text_buffer_get_iter_at_mark (buffer, &end2, mark2);
    gtk_text_buffer_delete (buffer, &end1, &end2);
    gtk_text_buffer_delete_mark (buffer, mark1);
    gtk_text_buffer_delete_mark (buffer, mark2);
}


static void
begin_comment_action (MooEdit *edit)
{
    gtk_text_buffer_begin_user_action (get_buffer (edit));
    moo_text_buffer_begin_interactive_action (get_moo_buffer (edit));
}

static void
end_comment_action (MooEdit *edit)
{
    gtk_text_buffer_end_user_action (get_buffer (edit));
    moo_text_buffer_end_interactive_action (get_moo_buffer (edit));
}


void
moo_edit_comment (MooEdit *edit)
{
    MooLang *lang;
    GtkTextIter start, end;
    GtkTextBuffer *buffer;
    gboolean has_selection, single_line, multi_line;
    gboolean adjust_selection = FALSE, move_insert = FALSE;
    int sel_start_line = 0, sel_start_offset = 0;

    g_return_if_fail (MOO_IS_EDIT (edit));

    lang = moo_text_view_get_lang (MOO_TEXT_VIEW (edit));

    if (!_moo_edit_has_comments (edit, &single_line, &multi_line))
        return;

    buffer = get_buffer (edit);
    has_selection = gtk_text_buffer_get_selection_bounds (buffer, &start, &end);

    begin_comment_action (edit);

    if (has_selection)
    {
        GtkTextIter iter;
        adjust_selection = TRUE;
        gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                          gtk_text_buffer_get_insert (buffer));
        move_insert = gtk_text_iter_equal (&iter, &start);
        sel_start_line = gtk_text_iter_get_line (&start);
        sel_start_offset = gtk_text_iter_get_line_offset (&start);
    }

    /* FIXME */
    if (single_line)
        line_comment (buffer, lang->line_comment, &start, &end);
    else
        block_comment (buffer, lang->block_comment_start,
                       lang->block_comment_end, &start, &end);


    if (adjust_selection)
    {
        const char *mark = move_insert ? "insert" : "selection_bound";
        gtk_text_buffer_get_iter_at_line_offset (buffer, &start,
                                                 sel_start_line,
                                                 sel_start_offset);
        gtk_text_buffer_move_mark_by_name (buffer, mark, &start);
    }

    end_comment_action (edit);
}


void
moo_edit_uncomment (MooEdit *edit)
{
    MooLang *lang;
    GtkTextIter start, end;
    GtkTextBuffer *buffer;
    gboolean single_line, multi_line;

    g_return_if_fail (MOO_IS_EDIT (edit));

    lang = moo_text_view_get_lang (MOO_TEXT_VIEW (edit));

    if (!_moo_edit_has_comments (edit, &single_line, &multi_line))
        return;

    buffer = get_buffer (edit);
    gtk_text_buffer_get_selection_bounds (buffer, &start, &end);

    begin_comment_action (edit);

    /* FIXME */
    if (single_line)
        line_uncomment (buffer, lang->line_comment, &start, &end);
    else
        block_uncomment (buffer, lang->block_comment_start,
                         lang->block_comment_end, &start, &end);

    end_comment_action (edit);
}


/*****************************************************************************/
/* popup menu
 */

/* gtktextview.c */
static void
popup_position_func (GtkMenu   *menu,
                     gint      *x,
                     gint      *y,
                     gboolean  *push_in,
                     gpointer   user_data)
{
    GtkTextView *text_view;
    GtkWidget *widget;
    GdkRectangle cursor_rect;
    GdkRectangle onscreen_rect;
    gint root_x, root_y;
    GtkTextIter iter;
    GtkRequisition req;
    GdkScreen *screen;
    gint monitor_num;
    GdkRectangle monitor;

    text_view = GTK_TEXT_VIEW (user_data);
    widget = GTK_WIDGET (text_view);

    g_return_if_fail (GTK_WIDGET_REALIZED (text_view));

    screen = gtk_widget_get_screen (widget);

    gdk_window_get_origin (widget->window, &root_x, &root_y);

    gtk_text_buffer_get_iter_at_mark (gtk_text_view_get_buffer (text_view),
                                      &iter,
                                      gtk_text_buffer_get_insert (gtk_text_view_get_buffer (text_view)));

    gtk_text_view_get_iter_location (text_view,
                                     &iter,
                                     &cursor_rect);

    gtk_text_view_get_visible_rect (text_view, &onscreen_rect);

    gtk_widget_size_request (text_view->popup_menu, &req);

    /* can't use rectangle_intersect since cursor rect can have 0 width */
    if (cursor_rect.x >= onscreen_rect.x &&
        cursor_rect.x < onscreen_rect.x + onscreen_rect.width &&
        cursor_rect.y >= onscreen_rect.y &&
        cursor_rect.y < onscreen_rect.y + onscreen_rect.height)
    {
        gtk_text_view_buffer_to_window_coords (text_view,
                                               GTK_TEXT_WINDOW_WIDGET,
                                               cursor_rect.x, cursor_rect.y,
                                               &cursor_rect.x, &cursor_rect.y);

        *x = root_x + cursor_rect.x + cursor_rect.width;
        *y = root_y + cursor_rect.y + cursor_rect.height;
    }
    else
    {
        /* Just center the menu, since cursor is offscreen. */
        *x = root_x + (widget->allocation.width / 2 - req.width / 2);
        *y = root_y + (widget->allocation.height / 2 - req.height / 2);
    }

    /* Ensure sanity */
    *x = CLAMP (*x, root_x, (root_x + widget->allocation.width));
    *y = CLAMP (*y, root_y, (root_y + widget->allocation.height));

    monitor_num = gdk_screen_get_monitor_at_point (screen, *x, *y);
    gtk_menu_set_monitor (menu, monitor_num);
    gdk_screen_get_monitor_geometry (screen, monitor_num, &monitor);

    *x = CLAMP (*x, monitor.x, monitor.x + MAX (0, monitor.width - req.width));
    *y = CLAMP (*y, monitor.y, monitor.y + MAX (0, monitor.height - req.height));

    *push_in = FALSE;
}

void
_moo_edit_do_popup (MooEdit        *edit,
                    GdkEventButton *event)
{
    MooUIXML *xml;
    MooEditWindow *window;

    xml = moo_editor_get_ui_xml (edit->priv->editor);
    g_return_if_fail (xml != NULL);

    if (!edit->priv->menu)
    {
        gboolean show_im_menu = TRUE;

        window = moo_edit_get_window (edit);
        edit->priv->menu =
                moo_ui_xml_create_widget (xml, MOO_UI_MENU, "Editor/Popup",
                                          moo_edit_get_actions (edit),
                                          window ? MOO_WINDOW(window)->accel_group : NULL);
        gtk_object_sink (g_object_ref (edit->priv->menu));

        if (show_im_menu)
        {
            GtkWidget *item, *submenu;

            item = gtk_separator_menu_item_new ();
            gtk_widget_show (item);
            gtk_menu_shell_append (GTK_MENU_SHELL (edit->priv->menu), item);

            item = gtk_menu_item_new_with_label ("Input Methods");
            gtk_widget_show (item);

            submenu = gtk_menu_new ();
            gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), submenu);
            gtk_menu_shell_append (GTK_MENU_SHELL (edit->priv->menu), item);

            gtk_im_multicontext_append_menuitems (GTK_IM_MULTICONTEXT (GTK_TEXT_VIEW (edit)->im_context),
                                                  GTK_MENU_SHELL (submenu));
        }
    }

    g_return_if_fail (edit->priv->menu != NULL);

    _moo_edit_check_actions (edit);

    if (event)
    {
        gtk_menu_popup (GTK_MENU (edit->priv->menu),
                        NULL, NULL, NULL, NULL,
                        event->button, event->time);
    }
    else
    {
        gtk_menu_popup (GTK_MENU (edit->priv->menu), NULL, NULL,
                        popup_position_func, edit,
                        0, gtk_get_current_event_time ());
        gtk_menu_shell_select_first (GTK_MENU_SHELL (edit->priv->menu), FALSE);
    }
}


static gboolean
moo_edit_popup_menu (GtkWidget *widget)
{
    _moo_edit_do_popup (MOO_EDIT (widget), NULL);
    return TRUE;
}
