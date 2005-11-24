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
#include "mooedit/mootextview-private.h"
#include "mooedit/mooeditdialogs.h"
#include "mooedit/mooeditprefs.h"
#include "mooedit/mootextbuffer.h"
#include "mooedit/mooedit-vars.h"
#include "mooutils/moomarshals.h"
#include "mooutils/moocompat.h"
#include "mooutils/mooutils-gobject.h"
#include <string.h>


static VarPool *moo_edit_var_pool = NULL;
static void     moo_edit_init_var_pool      (void);

static GObject *moo_edit_constructor        (GType                  type,
                                             guint                  n_construct_properties,
                                             GObjectConstructParam *construct_param);
static void     moo_edit_finalize           (GObject        *object);

static void     moo_edit_set_property       (GObject        *object,
                                             guint           prop_id,
                                             const GValue   *value,
                                             GParamSpec     *pspec);
static void     moo_edit_get_property       (GObject        *object,
                                             guint           prop_id,
                                             GValue         *value,
                                             GParamSpec     *pspec);

static void     moo_edit_variable_changed   (MooEdit        *edit,
                                             const char     *name);
static void     moo_edit_filename_changed   (MooEdit        *edit,
                                             const char     *new_filename);

static GtkTextBuffer *get_buffer            (MooEdit        *edit);
static MooTextBuffer *get_moo_buffer        (MooEdit        *edit);

static void     modified_changed_cb         (GtkTextBuffer  *buffer,
                                             MooEdit        *edit);


enum {
    DOC_STATUS_CHANGED,
    FILENAME_CHANGED,
    COMMENT,
    UNCOMMENT,
    VARIABLE_CHANGED,
    SAVE_BEFORE,
    SAVE_AFTER,
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

    moo_edit_register_var (g_param_spec_string (MOO_EDIT_VAR_LANG, MOO_EDIT_VAR_LANG,
                           MOO_EDIT_VAR_LANG, NULL, 0));
    moo_edit_register_var (g_param_spec_string (MOO_EDIT_VAR_INDENT, MOO_EDIT_VAR_INDENT,
                           MOO_EDIT_VAR_INDENT, NULL, 0));
    moo_edit_register_var (g_param_spec_boolean (MOO_EDIT_VAR_STRIP, MOO_EDIT_VAR_STRIP,
                           MOO_EDIT_VAR_STRIP, FALSE, 0));

    gobject_class->set_property = moo_edit_set_property;
    gobject_class->get_property = moo_edit_get_property;
    gobject_class->constructor = moo_edit_constructor;
    gobject_class->finalize = moo_edit_finalize;

    klass->variable_changed = moo_edit_variable_changed;
    klass->filename_changed = moo_edit_filename_changed;

    g_object_class_install_property (gobject_class,
                                     PROP_EDITOR,
                                     g_param_spec_object ("editor",
                                             "editor",
                                             "editor",
                                             MOO_TYPE_EDITOR,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

    signals[VARIABLE_CHANGED] =
            g_signal_new ("variable-changed",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_FIRST,
                          G_STRUCT_OFFSET (MooEditClass, variable_changed),
                          NULL, NULL,
                          _moo_marshal_VOID__STRING,
                          G_TYPE_NONE, 1,
                          G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE);

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
                               NULL, /* G_CALLBACK (moo_edit_comment), */
                               NULL, NULL,
                               _moo_marshal_VOID__VOID,
                               G_TYPE_NONE, 0);

    signals[UNCOMMENT] =
            moo_signal_new_cb ("uncomment",
                               G_OBJECT_CLASS_TYPE (klass),
                               G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                               NULL, /* G_CALLBACK (moo_edit_uncomment), */
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

    /* TODO: this is wrong */
    _moo_edit_set_default_settings ();
}


static void
moo_edit_init (MooEdit *edit)
{
    MooIndenter *indent;

    edit->priv = g_new0 (MooEditPrivate, 1);

    edit->priv->file_watch_policy = MOO_EDIT_RELOAD_IF_SAFE;

    edit->priv->vars = var_table_new (moo_edit_var_pool);

    edit->priv->enable_indentation = TRUE;
    indent = moo_indenter_new (edit, NULL);
    moo_text_view_set_indenter (MOO_TEXT_VIEW (edit), indent);
    g_object_unref (indent);

    /* XXX this is stupid */
#if defined(__WIN32__)
    edit->priv->line_end_type = MOO_EDIT_LINE_END_WIN32;
#elif defined(MOO_OS_DARWIN)
    edit->priv->line_end_type = MOO_EDIT_LINE_END_MAC;
#else
    edit->priv->line_end_type = MOO_EDIT_LINE_END_UNIX;
#endif

    g_object_set (edit, "draw-tabs", TRUE, "draw-trailing-spaces", TRUE, NULL);
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

    var_table_free (edit->priv->vars);

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
moo_edit_var_dep_get_type (void)
{
    static GType type = 0;

    if (!type)
    {
        static const GEnumValue values[] = {
            { MOO_EDIT_VAR_DEP_NONE, (char*)"MOO_EDIT_VAR_DEP_NONE", (char*)"none" },
            { MOO_EDIT_VAR_DEP_FILENAME, (char*)"MOO_EDIT_VAR_DEP_FILENAME", (char*)"filename" },
            { MOO_EDIT_VAR_DEP_AUTO, (char*)"MOO_EDIT_VAR_DEP_AUTO", (char*)"auto" },
            { 0, NULL, NULL }
        };
        type = g_enum_register_static ("MooEditVarDep", values);
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


static void
moo_edit_set_lang (MooEdit        *edit,
                   MooLang        *lang)
{
    g_return_if_fail (MOO_IS_EDIT (edit));
    moo_text_view_set_lang (MOO_TEXT_VIEW (edit), lang);
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
parse_kate_mode_string (G_GNUC_UNUSED MooEdit *edit,
                        G_GNUC_UNUSED char    *string)
{
}


static gboolean
moo_edit_set_var_from_string (MooEdit       *edit,
                              const char    *name,
                              const char    *strval)
{
    char *normname, *normval;
    GParamSpec *pspec;
    gboolean result = FALSE;
    GValue value;

    g_return_val_if_fail (name != NULL, FALSE);
    g_return_val_if_fail (strval != NULL, FALSE);

    normname = g_strdelimit (g_ascii_strdown (name, -1), "_", '-');
    normval = g_ascii_strdown (strval, -1);

    pspec = var_pool_get_pspec (moo_edit_var_pool, normname);
    value.g_type = 0;

    if (!pspec)
        goto out;

    if (G_PARAM_SPEC_VALUE_TYPE (pspec) == G_TYPE_BOOLEAN)
    {
        if (!strcmp (normval, "yes") || !strcmp (normval, "on") ||
             !strcmp (normval, "true") || !strcmp (normval, "1"))
        {
            g_value_init (&value, G_TYPE_BOOLEAN);
            g_value_set_boolean (&value, TRUE);
        }
        else if (!strcmp (normval, "no") || !strcmp (normval, "off") ||
                  !strcmp (normval, "false") || !strcmp (normval, "0"))
        {
            g_value_init (&value, G_TYPE_BOOLEAN);
            g_value_set_boolean (&value, FALSE);
        }
        else
        {
            goto out;
        }
    }
    else
    {
        g_value_init (&value, G_PARAM_SPEC_VALUE_TYPE (pspec));

        if (!moo_value_convert_from_string (normval, &value))
        {
            g_value_unset (&value);
            goto out;
        }
    }

    moo_edit_set_var (edit, normname, &value, MOO_EDIT_VAR_DEP_FILENAME);
    result = TRUE;

out:

#if 1
    if (!result)
        g_warning ("%s: could not set '%s' to '%s'", G_STRLOC, name, strval);
    else
        g_message ("%s: set '%s' to '%s'", G_STRLOC, normname, normval);
#endif

    g_free (normname);
    g_free (normval);

    if (G_IS_VALUE (&value))
        g_value_unset (&value);

    return result;
}


static void
set_emacs_var (MooEdit *edit,
               char    *name,
               char    *val)
{
    if (!g_ascii_strcasecmp (name, "mode"))
    {
        moo_edit_set_var_from_string (edit, "lang", val);
    }
    else if (!g_ascii_strcasecmp (name, "tab-width"))
    {
        moo_edit_set_var_from_string (edit, "indent-tab-width", val);
    }
    else if (!g_ascii_strcasecmp (name, "c-basic-offset"))
    {
        moo_edit_set_var_from_string (edit, "indent-width", val);
    }
    else if (!g_ascii_strcasecmp (name, "indent-tabs-mode"))
    {
        if (!g_ascii_strcasecmp (val, "nil"))
            moo_edit_set_var_from_string (edit, "indent-use-tabs", "false");
        else
            moo_edit_set_var_from_string (edit, "indent-use-tabs", "true");
    }
}

static void
parse_emacs_mode_string (MooEdit *edit,
                         char    *string)
{
    parse_mode_string (edit, string, ":", set_emacs_var);
}


static void
parse_moo_mode_string (MooEdit *edit,
                       char    *string)
{
    parse_mode_string (edit, string, " ", (SetVarFunc) moo_edit_set_var_from_string);
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
moo_edit_variable_changed (MooEdit        *edit,
                           const char     *name)
{
    if (!strcmp (name, MOO_EDIT_VAR_LANG))
    {
        char *value = moo_edit_get_string (edit, name);
        MooLangMgr *mgr = moo_editor_get_lang_mgr (edit->priv->editor);
        MooLang *lang = value ? moo_lang_mgr_get_lang (mgr, value) : NULL;
        moo_edit_set_lang (edit, lang);
        g_free (value);
    }
}


static void
moo_edit_filename_changed (MooEdit    *edit,
                           const char *filename)
{
    MooLang *lang = NULL;

    _moo_edit_freeze_var_notify (edit);

    if (!edit->priv->changed_vars)
        edit->priv->changed_vars = string_list_new ();

    var_table_remove_by_dep (edit->priv->vars,
                             MOO_EDIT_VAR_DEP_FILENAME,
                             edit->priv->changed_vars);

    if (filename)
    {
        MooLangMgr *mgr = moo_editor_get_lang_mgr (edit->priv->editor);
        lang = moo_lang_mgr_get_lang_for_file (mgr, filename);
    }

    moo_edit_set_string (edit, MOO_EDIT_VAR_LANG, lang ? lang->id : NULL,
                         MOO_EDIT_VAR_DEP_FILENAME);
    moo_edit_set_string (edit, MOO_EDIT_VAR_INDENT, NULL,
                         MOO_EDIT_VAR_DEP_FILENAME);
    try_mode_strings (edit);

    _moo_edit_thaw_var_notify (edit);
}


gboolean
moo_edit_save (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    return _moo_editor_save (edit->priv->editor, edit);
}


gboolean
moo_edit_save_as (MooEdit        *edit,
                  const char     *filename,
                  const char     *encoding)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    return _moo_editor_save_as (edit->priv->editor, edit, filename, encoding);
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


/*********************************************************************/
/* Variables
 */

void
moo_edit_set_var (MooEdit        *edit,
                  const char     *name,
                  const GValue   *value,
                  MooEditVarDep   dep)
{
    Var *v;
    GParamSpec *pspec;

    g_return_if_fail (MOO_IS_EDIT (edit));
    g_return_if_fail (name != NULL);
    g_return_if_fail (!value || G_IS_VALUE (value));

    pspec = var_pool_get_pspec (moo_edit_var_pool, name);
    g_return_if_fail (pspec != NULL);
    g_return_if_fail (!value || G_PARAM_SPEC_VALUE_TYPE (pspec) == G_VALUE_TYPE (value));

    name = g_param_spec_get_name (pspec);
    v = var_table_get (edit->priv->vars, name);

    if (v && v->dep < dep)
        return;

    if (value)
    {
        if (!v)
        {
            v = var_new (dep, G_VALUE_TYPE (value));
            var_table_insert (edit->priv->vars, name, v);
        }

        v->dep = dep;
        g_value_copy (value, &v->value);
    }
    else
    {
        var_table_remove (edit->priv->vars, name);
    }

    _moo_edit_var_notify (edit, name);
}


gboolean
moo_edit_get_var (MooEdit    *edit,
                  const char *name,
                  GValue     *value)
{
    Var *v;

    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    g_return_val_if_fail (name != NULL, FALSE);
    g_return_val_if_fail (!value || G_IS_VALUE (value), FALSE);

    name = var_pool_find_name (moo_edit_var_pool, name);
    g_return_val_if_fail (name != NULL, FALSE);

    v = var_table_get (edit->priv->vars, name);

    if (v && value)
    {
        g_return_val_if_fail (G_VALUE_TYPE (&v->value) == G_VALUE_TYPE (value), FALSE);
        g_value_copy (&v->value, value);
    }

    return v != NULL;
}


char*
moo_edit_get_string (MooEdit    *edit,
                     const char *name)
{
    GValue val;
    char *strval;

    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    g_return_val_if_fail (name != NULL, NULL);

    val.g_type = 0;
    g_value_init (&val, G_TYPE_STRING);

    if (!moo_edit_get_var (edit, name, &val))
        return NULL;

    strval = g_value_dup_string (&val);
    g_value_unset (&val);

    return strval;
}


gboolean
moo_edit_get_bool (MooEdit        *edit,
                   const char     *name,
                   gboolean        default_val)
{
    GValue val;

    g_return_val_if_fail (MOO_IS_EDIT (edit), default_val);
    g_return_val_if_fail (name != NULL, default_val);

    val.g_type = 0;
    g_value_init (&val, G_TYPE_BOOLEAN);

    if (!moo_edit_get_var (edit, name, &val))
        return default_val;

    return g_value_get_boolean (&val);
}


int
moo_edit_get_int (MooEdit        *edit,
                  const char     *name,
                  int             default_val)
{
    GValue val;

    g_return_val_if_fail (MOO_IS_EDIT (edit), default_val);
    g_return_val_if_fail (name != NULL, default_val);

    val.g_type = 0;
    g_value_init (&val, G_TYPE_INT);

    if (!moo_edit_get_var (edit, name, &val))
        return default_val;

    return g_value_get_int (&val);
}


guint
moo_edit_get_uint (MooEdit        *edit,
                   const char     *name,
                   guint           default_val)
{
    GValue val;

    g_return_val_if_fail (MOO_IS_EDIT (edit), default_val);
    g_return_val_if_fail (name != NULL, default_val);

    val.g_type = 0;
    g_value_init (&val, G_TYPE_UINT);

    if (!moo_edit_get_var (edit, name, &val))
        return default_val;

    return g_value_get_uint (&val);
}


void
moo_edit_set_string (MooEdit        *edit,
                     const char     *name,
                     const char     *value,
                     MooEditVarDep   dep)
{
    g_return_if_fail (MOO_IS_EDIT (edit));
    g_return_if_fail (name != NULL);

    if (value)
    {
        GValue gval;
        gval.g_type = 0;
        g_value_init (&gval, G_TYPE_STRING);
        g_value_set_static_string (&gval, value);
        moo_edit_set_var (edit, name, &gval, dep);
        g_value_unset (&gval);
    }
    else
    {
        moo_edit_set_var (edit, name, NULL, dep);
    }
}


static void
emit_var_notify (MooEdit        *edit,
                 const char     *name)
{
    g_signal_emit (edit, signals[VARIABLE_CHANGED], 0, name);
}


void
_moo_edit_var_notify (MooEdit        *edit,
                      const char     *name)
{
    g_return_if_fail (MOO_IS_EDIT (edit));

    if (edit->priv->freeze_var_notify)
    {
        if (!edit->priv->changed_vars)
            edit->priv->changed_vars = string_list_new ();
        string_list_add (edit->priv->changed_vars, name);
    }
    else
    {
        emit_var_notify (edit, name);
    }
}


void
_moo_edit_freeze_var_notify (MooEdit *edit)
{
    g_return_if_fail (MOO_IS_EDIT (edit));
    edit->priv->freeze_var_notify++;
}


static void
emit_var_notify_cb (const char *varname,
                    G_GNUC_UNUSED gpointer dummy,
                    MooEdit    *edit)
{
    emit_var_notify (edit, varname);
}

void
_moo_edit_thaw_var_notify (MooEdit *edit)
{
    g_return_if_fail (MOO_IS_EDIT (edit));
    g_return_if_fail (edit->priv->freeze_var_notify > 0);

    if (!--edit->priv->freeze_var_notify && edit->priv->changed_vars)
    {
        StringList *vars = edit->priv->changed_vars;
        edit->priv->changed_vars = NULL;
        g_object_ref (edit);
        g_hash_table_foreach (vars->hash, (GHFunc) emit_var_notify_cb, edit);
        g_object_unref (edit);
        string_list_free (vars);
    }
}


void
moo_edit_register_var (GParamSpec     *pspec)
{
    g_return_if_fail (G_IS_PARAM_SPEC (pspec));

    moo_edit_init_var_pool ();

    if (var_pool_get_pspec (moo_edit_var_pool, g_param_spec_get_name (pspec)))
    {
        g_critical ("%s: variable '%s' already registered",
                    G_STRLOC, g_param_spec_get_name (pspec));
        return;
    }

    var_pool_add (moo_edit_var_pool, pspec);
}


void
moo_edit_register_var_alias (const char     *name,
                             const char     *alias)
{
    g_return_if_fail (name != NULL);
    g_return_if_fail (alias != NULL);

    moo_edit_init_var_pool ();

    if (!var_pool_get_pspec (moo_edit_var_pool, name))
    {
        g_critical ("%s: no variable '%s'", G_STRLOC, name);
        return;
    }

    if (var_pool_get_pspec (moo_edit_var_pool, alias))
    {
        g_critical ("%s: variable '%s' already registered",
                    G_STRLOC, alias);
        return;
    }

    var_pool_add_alias (moo_edit_var_pool, name, alias);
}


static void
moo_edit_init_var_pool (void)
{
    if (!moo_edit_var_pool)
        moo_edit_var_pool = var_pool_new ();
}
