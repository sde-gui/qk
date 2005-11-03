/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mooedit.c
 *
 *   Copyright (C) 2004-2005 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#define MOOEDIT_COMPILATION

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mooedit/mooedit-private.h"
#include "mooedit/mooeditdialogs.h"
#include "mooedit/mooeditprefs.h"
#include "mooedit/mootextbuffer.h"
#include "mooutils/moomarshals.h"
#include "mooutils/moocompat.h"
#include "mooutils/mooutils-gobject.h"
#include <string.h>


static GObject *moo_edit_constructor    (GType                  type,
                                         guint                  n_construct_properties,
                                         GObjectConstructParam *construct_param);
static void     moo_edit_finalize       (GObject        *object);

static void     moo_edit_set_property   (GObject        *object,
                                         guint           prop_id,
                                         const GValue   *value,
                                         GParamSpec     *pspec);
static void     moo_edit_get_property   (GObject        *object,
                                         guint           prop_id,
                                         GValue         *value,
                                         GParamSpec     *pspec);

static GtkTextBuffer *get_buffer        (MooEdit            *edit);
static MooTextBuffer *get_moo_buffer    (MooEdit            *edit);

static void     modified_changed_cb     (GtkTextBuffer      *buffer,
                                         MooEdit            *edit);

static void     moo_edit_comment        (MooEdit            *edit);
static void     moo_edit_uncomment      (MooEdit            *edit);


enum {
    DOC_STATUS_CHANGED,
    FILENAME_CHANGED,
    COMMENT,
    UNCOMMENT,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];


enum {
    PROP_0,
    PROP_EDITOR
};


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

    klass->doc_status_changed = NULL;
    klass->filename_changed = NULL;

    g_object_class_install_property (gobject_class,
                                     PROP_EDITOR,
                                     g_param_spec_object ("editor",
                                             "editor",
                                             "editor",
                                             MOO_TYPE_EDITOR,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

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
                          G_SIGNAL_RUN_LAST,
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

    /* TODO: this is wrong */
    _moo_edit_set_default_settings ();
}


static void
moo_edit_init (MooEdit *edit)
{
    edit->priv = g_new0 (MooEditPrivate, 1);

    edit->priv->file_watch_policy = MOO_EDIT_RELOAD_IF_SAFE;

    edit->priv->enable_indentation = TRUE;

    /* XXX this is stupid */
#if defined(__WIN32__)
    edit->priv->line_end_type = MOO_EDIT_LINE_END_WIN32;
#elif defined(MOO_OS_DARWIN)
    edit->priv->line_end_type = MOO_EDIT_LINE_END_MAC;
#else
    edit->priv->line_end_type = MOO_EDIT_LINE_END_UNIX;
#endif

    edit->priv->vars = g_hash_table_new_full (g_str_hash, g_str_equal,
                                              g_free, g_free);
}


static GObject*
moo_edit_constructor (GType                  type,
                      guint                  n_construct_properties,
                      GObjectConstructParam *construct_param)
{
    GObject *object;
    MooEdit *edit;

    object = G_OBJECT_CLASS (moo_edit_parent_class)->constructor (
        type, n_construct_properties, construct_param);

    edit = MOO_EDIT (object);

    edit->priv->modified_changed_handler_id =
            g_signal_connect (get_buffer (edit),
                              "modified-changed",
                              G_CALLBACK (modified_changed_cb),
                              edit);

    _moo_edit_apply_settings (edit);
    edit->priv->prefs_notify =
        moo_prefs_notify_connect (MOO_EDIT_PREFS_PREFIX,
                                  MOO_PREFS_MATCH_PREFIX,
                                  (MooPrefsNotify) _moo_edit_settings_changed,
                                  edit);

    g_signal_connect (edit, "realize", G_CALLBACK (_moo_edit_apply_style_settings), NULL);

    _moo_edit_set_filename (edit, NULL, NULL);

    return object;
}


static void
moo_edit_finalize (GObject *object)
{
    MooEdit *edit = MOO_EDIT (object);

    moo_prefs_notify_disconnect (edit->priv->prefs_notify);

    edit->priv->focus_in_handler_id = 0;
    if (edit->priv->file_monitor_id)
        _moo_edit_stop_file_watch (edit);

    g_free (edit->priv->filename);
    g_free (edit->priv->basename);
    g_free (edit->priv->display_filename);
    g_free (edit->priv->display_basename);
    g_free (edit->priv->encoding);

    g_hash_table_destroy (edit->priv->vars);

    g_free (edit->priv);
    edit->priv = NULL;

    G_OBJECT_CLASS (moo_edit_parent_class)->finalize (object);
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
moo_edit_get_readonly (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), TRUE);
    return edit->priv->readonly;
}


void
moo_edit_set_readonly (MooEdit            *edit,
                       gboolean            readonly)
{
    g_return_if_fail (MOO_IS_EDIT (edit));
    edit->priv->readonly = readonly;
    gtk_text_view_set_editable (GTK_TEXT_VIEW (edit), !readonly);
    moo_edit_status_changed (edit);
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
                                             (GBoxedFreeFunc)moo_edit_file_info_free);
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


static GtkTextBuffer*
get_buffer (MooEdit *edit)
{
    return gtk_text_view_get_buffer (GTK_TEXT_VIEW (edit));
}


static MooTextBuffer*
get_moo_buffer (MooEdit *edit)
{
    return MOO_TEXT_BUFFER (get_buffer (edit));
}


void
moo_edit_set_lang (MooEdit        *edit,
                   MooLang        *lang)
{
    g_return_if_fail (MOO_IS_EDIT (edit));
    moo_text_view_set_lang (MOO_TEXT_VIEW (edit), lang);
}


MooLang*
moo_edit_get_lang (MooEdit        *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    return moo_text_buffer_get_lang (get_moo_buffer (edit));
}


void
moo_edit_set_highlight (MooEdit        *edit,
                        gboolean        highlight)
{
    g_return_if_fail (MOO_IS_EDIT (edit));
    moo_text_buffer_set_highlight (get_moo_buffer (edit), highlight);
}


gboolean
moo_edit_get_highlight (MooEdit        *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    return moo_text_buffer_get_highlight (get_moo_buffer (edit));
}


static gboolean
is_ascii (const char *string)
{
    while (*string++)
        if ((guint8) *string > 127)
            return FALSE;
    return TRUE;
}


void
moo_edit_set_var (MooEdit        *edit,
                  const char     *name,
                  const char     *value)
{
    char *key = NULL;

    g_return_if_fail (MOO_IS_EDIT (edit));
    g_return_if_fail (name && name[0]);

    if (is_ascii (name))
        key = g_ascii_strdown (name, -1);
    else
        key = g_strdup (name);

    g_strdelimit (key, "-_", '-');

    g_hash_table_insert (edit->priv->vars, key, g_strdup (value));
}


const char*
moo_edit_get_var (MooEdit        *edit,
                  const char     *name)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    g_return_val_if_fail (name && name[0], NULL);
    return g_hash_table_lookup (edit->priv->vars, name);
}


static void
try_mode_string (MooEdit    *edit,
                 const char *text,
                 const char *mode_string_start,
                 const char *var_val_separator)
{
    char *start, *mode_string;
    char **vars, **p;

    mode_string = NULL;
    vars = NULL;

    start = strstr (text, mode_string_start);

    if (!start || !start[strlen (mode_string_start)])
        goto out;

    start += strlen (mode_string_start);

    mode_string = g_strdup (start);
    g_strstrip (mode_string);

    vars = g_strsplit (mode_string, ";", 0);

    if (!vars)
        goto out;

    for (p = vars; *p != NULL; p++)
    {
        char *sep, *var, *value;

        sep = g_strrstr (*p, var_val_separator);

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

        moo_edit_set_var (edit, var, value);

        g_free (var);
    }

out:
    g_free (mode_string);
    g_strfreev (vars);
}


#define KATE_MODE_STRING        "kate:"
#define KATE_VAR_VAL_SEPARATOR  " "
#define EMACS_MODE_STRING       "-*-"
#define EMACS_VAR_VAL_SEPARATOR ":"

static void
try_mode (MooEdit *edit)
{
    GtkTextBuffer *buffer = get_buffer (edit);
    GtkTextIter start, end;
    char *text;

    gtk_text_buffer_get_start_iter (buffer, &start);
    end = start;
    gtk_text_iter_forward_to_line_end (&end);
    text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
    try_mode_string (edit, text, KATE_MODE_STRING, KATE_VAR_VAL_SEPARATOR);
    try_mode_string (edit, text, EMACS_MODE_STRING, EMACS_VAR_VAL_SEPARATOR);
    g_free (text);

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

    text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
    try_mode_string (edit, text, KATE_MODE_STRING, KATE_VAR_VAL_SEPARATOR);
    g_free (text);
}


void
_moo_edit_reload_vars (MooEdit *edit)
{
    g_return_if_fail (MOO_IS_EDIT (edit));
    g_hash_table_foreach_remove (edit->priv->vars, (GHRFunc) gtk_true, NULL);
    try_mode (edit);
}


void
_moo_edit_choose_lang (MooEdit *edit)
{
    MooLang *lang = NULL;

    if (edit->priv->filename)
    {
        MooLangMgr *mgr = moo_editor_get_lang_mgr (edit->priv->editor);
        lang = moo_lang_mgr_get_lang_for_file (mgr, edit->priv->filename);
    }

    moo_edit_set_lang (edit, lang);
}


static void
indenter_set_var (const char  *var,
                  const char  *value,
                  MooIndenter *indent)
{
    moo_indenter_set_value (indent, var, value);
}

void
_moo_edit_choose_indenter (MooEdit *edit)
{
    MooIndenter *indenter = NULL;
    const char *mode;

    if (!edit->priv->enable_indentation)
        return;

    mode = moo_edit_get_var (edit, "mode");

    if (mode)
        indenter = moo_indenter_get_for_mode (mode);

    if (!indenter)
        indenter = moo_indenter_default_new ();

    g_hash_table_foreach (edit->priv->vars,
                          (GHFunc) indenter_set_var,
                          indenter);

    moo_text_view_set_indenter (MOO_TEXT_VIEW (edit), indenter);
}


static void
moo_edit_comment (MooEdit *edit)
{
    MooLang *lang = moo_edit_get_lang (edit);

    if (!lang || (!lang->single_line_comment && !lang->multi_line_comment_start))
        return;

    g_return_if_fail (!lang->multi_line_comment_start || lang->multi_line_comment_end);

    g_message ("comment");
}


static void
moo_edit_uncomment (MooEdit *edit)
{
    MooLang *lang = moo_edit_get_lang (edit);

    if (!lang || (!lang->single_line_comment && !lang->multi_line_comment_start))
        return;

    g_return_if_fail (!lang->multi_line_comment_start || lang->multi_line_comment_end);

    g_message ("uncomment");
}
