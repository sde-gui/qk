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
#include "mooedit/mooeditlangmgr.h"
#include "mooedit/mootextbuffer.h"
#include "mooutils/moomarshals.h"
#include "mooutils/moocompat.h"
#include "mooutils/moofileutils.h"
#include "mooutils/moosignal.h"
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

static void modified_changed_cb         (GtkTextBuffer      *buffer,
                                         MooEdit            *edit);


enum {
    DOC_STATUS_CHANGED,
    FILENAME_CHANGED,
    LANG_CHANGED,
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
    klass->lang_changed = NULL;

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

    signals[LANG_CHANGED] =
            g_signal_new ("lang-changed",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooEditClass, lang_changed),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    /* TODO: this is wrong */
    _moo_edit_set_default_settings ();
}


static void
moo_edit_init (MooEdit *edit)
{
    edit->priv = _moo_edit_private_new ();
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

    edit->priv->constructed = TRUE;

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
    if (edit->priv->file_watch_id)
        g_source_remove (edit->priv->file_watch_id);

    g_free (edit->priv->filename);
    g_free (edit->priv->basename);
    g_free (edit->priv->display_filename);
    g_free (edit->priv->display_basename);
    g_free (edit->priv->encoding);

    if (edit->priv->lang)
        g_object_unref (edit->priv->lang);

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


MooEditPrivate*
_moo_edit_private_new (void)
{
    MooEditPrivate *priv = g_new0 (MooEditPrivate, 1);

//     priv->status = MOO_EDIT_DOC_NEW;

//     priv->timestamp = MOO_MTIME_EINVAL;
    priv->file_watch_timeout = 5000;
    priv->file_watch_policy = MOO_EDIT_RELOAD_IF_SAFE;

    priv->enable_indentation = TRUE;

#if defined(__WIN32__)
    priv->line_end_type = MOO_EDIT_LINE_END_WIN32;
#elif defined(OS_DARWIN)
    priv->line_end_type = MOO_EDIT_LINE_END_MAC;
#else
    priv->line_end_type = MOO_EDIT_LINE_END_UNIX;
#endif

    return priv;
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


void
moo_edit_set_lang (MooEdit            *edit,
                   MooEditLang        *lang)
{
    g_return_if_fail (MOO_IS_EDIT (edit));

    if (lang == edit->priv->lang)
        return;

    if (edit->priv->lang)
    {
        g_object_unref (edit->priv->lang);
        edit->priv->lang = NULL;
    }

    moo_text_buffer_set_lang (get_moo_buffer (edit), lang);

    if (lang)
    {
        edit->priv->lang = lang;
        g_object_ref (edit->priv->lang);
    }

    g_signal_emit (edit, signals[LANG_CHANGED], 0, NULL);
}


void
moo_edit_set_highlight (MooEdit            *edit,
                        gboolean            highlight)
{
    g_return_if_fail (MOO_IS_EDIT (edit));
    gtk_source_buffer_set_highlight (GTK_SOURCE_BUFFER (get_buffer (edit)),
                                     highlight);
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


MooEditLangMgr*
_moo_edit_get_lang_mgr (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);

    if (edit->priv->editor)
        return moo_editor_get_lang_mgr (edit->priv->editor);
    else
        return NULL;
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


MooEditLang*
moo_edit_get_lang (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    return edit->priv->lang;
}


GType
moo_edit_doc_status_get_type (void)
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


#define MODE_STRING "-*-"

static MooIndenter*
get_indenter_for_mode_string (MooEdit *edit)
{
    GtkTextBuffer *buffer = get_buffer (edit);
    GtkTextIter line_start, line_end;
    char *text, *start, *end, *mode_string;
    char **vars, **p;
    MooIndenter *indenter;

    mode_string = NULL;
    vars = NULL;
    indenter = NULL;

    gtk_text_buffer_get_start_iter (buffer, &line_start);
    line_end = line_start;
    gtk_text_iter_forward_to_line_end (&line_end);

    text = gtk_text_buffer_get_text (buffer, &line_start, &line_end, FALSE);

    start = strstr (text, MODE_STRING);

    if (!start || !start[strlen (MODE_STRING)])
        goto out;

    start += strlen (MODE_STRING);

    end = strstr (start, MODE_STRING);

    if (!end)
        goto out;

    mode_string = g_strndup (start, end - start);
    g_strstrip (mode_string);

    vars = g_strsplit (mode_string, ";", 0);

    if (!vars)
        goto out;

    for (p = vars; *p != NULL; p++)
    {
        char *colon, *var, *value;

        colon = g_strrstr (*p, ":");
        if (!colon || colon == *p || !colon[1])
            goto out;

        var = g_strndup (*p, colon - *p);
        g_strstrip (var);
        if (!var[0])
        {
            g_free (var);
            goto out;
        }

        value = colon + 1;
        g_strstrip (value);
        if (!value)
        {
            g_free (var);
            goto out;
        }

        if (!indenter)
        {
            if (!g_ascii_strcasecmp (var, "mode"))
                indenter = moo_indenter_get_for_mode (value);

            if (!indenter)
            {
                g_free (var);
                goto out;
            }
        }
        else
        {
            moo_indenter_set_value (indenter, var, value);
        }

        g_free (var);
    }

out:
    g_free (text);
    g_free (mode_string);
    g_strfreev (vars);
    return indenter;
}


void
_moo_edit_choose_indenter (MooEdit *edit)
{
    MooIndenter *indenter;

    if (!edit->priv->enable_indentation)
        return;

    indenter = get_indenter_for_mode_string (edit);

    if (!indenter && edit->priv->lang)
        indenter = moo_indenter_get_for_lang (moo_edit_lang_get_id (edit->priv->lang));

    if (!indenter)
        indenter = moo_indenter_default_new ();

    moo_text_view_set_indenter (MOO_TEXT_VIEW (edit), indenter);
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
