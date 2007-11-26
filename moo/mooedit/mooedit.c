/*
 *   mooedit.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#define MOOEDIT_COMPILATION
#include "mooedit/mooeditaction-factory.h"
#include "mooedit/mooedit-private.h"
#include "mooedit/mooedit-bookmarks.h"
#include "mooedit/mootextview-private.h"
#include "mooedit/mooeditdialogs.h"
#include "mooedit/mooeditprefs.h"
#include "mooedit/mootextbuffer.h"
#include "mooedit/mooeditprogress-glade.h"
#include "mooedit/mooeditfiltersettings.h"
#include "mooedit/mooeditor-private.h"
#include "mooutils/moomarshals.h"
#include "mooutils/mooutils-gobject.h"
#include "mooutils/mooglade.h"
#include "mooutils/mooi18n.h"
#include "mooutils/mooutils-misc.h"
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

static gboolean moo_edit_focus_in           (GtkWidget      *widget,
                                             GdkEventFocus  *event);
static gboolean moo_edit_focus_out          (GtkWidget      *widget,
                                             GdkEventFocus  *event);
static gboolean moo_edit_popup_menu         (GtkWidget      *widget);
static gboolean moo_edit_drag_motion        (GtkWidget      *widget,
                                             GdkDragContext *context,
                                             gint            x,
                                             gint            y,
                                             guint           time);
static gboolean moo_edit_drag_drop          (GtkWidget      *widget,
                                             GdkDragContext *context,
                                             gint            x,
                                             gint            y,
                                             guint           time);

static void     moo_edit_filename_changed       (MooEdit        *edit,
                                                 const char     *new_filename);

static void     config_changed                  (MooEdit        *edit,
                                                 GParamSpec     *pspec);
static void     moo_edit_config_notify          (MooEdit        *edit,
                                                 guint           var_id,
                                                 GParamSpec     *pspec);
static void     _moo_edit_freeze_config_notify  (MooEdit        *edit);
static void     _moo_edit_thaw_config_notify    (MooEdit        *edit);
static void     _moo_edit_update_config_from_global (MooEdit    *edit);

static GtkTextBuffer *get_buffer                (MooEdit        *edit);

static void     modified_changed_cb             (GtkTextBuffer  *buffer,
                                                 MooEdit        *edit);


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

/* MOO_TYPE_EDIT */
G_DEFINE_TYPE (MooEdit, moo_edit, MOO_TYPE_TEXT_VIEW)


static void
moo_edit_class_init (MooEditClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
    MooTextViewClass *textview_class = MOO_TEXT_VIEW_CLASS (klass);

    gobject_class->set_property = moo_edit_set_property;
    gobject_class->get_property = moo_edit_get_property;
    gobject_class->constructor = moo_edit_constructor;
    gobject_class->finalize = moo_edit_finalize;
    gobject_class->dispose = moo_edit_dispose;

    widget_class->popup_menu = moo_edit_popup_menu;
    widget_class->drag_motion = moo_edit_drag_motion;
    widget_class->drag_drop = moo_edit_drag_drop;
    widget_class->focus_in_event = moo_edit_focus_in;
    widget_class->focus_out_event = moo_edit_focus_out;

    textview_class->line_mark_clicked = _moo_edit_line_mark_clicked;

    klass->filename_changed = moo_edit_filename_changed;
    klass->config_notify = moo_edit_config_notify;

    g_type_class_add_private (klass, sizeof (MooEditPrivate));

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
                                             TRUE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_HAS_COMMENTS,
                                     g_param_spec_boolean ("has-comments",
                                             "has-comments",
                                             "has-comments",
                                             FALSE,
                                             G_PARAM_READABLE));

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
            _moo_signal_new_cb ("comment",
                                G_OBJECT_CLASS_TYPE (klass),
                                G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                                G_CALLBACK (moo_edit_comment),
                                NULL, NULL,
                                _moo_marshal_VOID__VOID,
                                G_TYPE_NONE, 0);

    signals[UNCOMMENT] =
            _moo_signal_new_cb ("uncomment",
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

    _moo_edit_init_config ();
    _moo_edit_class_init_actions (klass);
}


static void
moo_edit_init (MooEdit *edit)
{
    MooIndenter *indent;

    edit->config = moo_edit_config_new ();
    g_signal_connect_swapped (edit->config, "notify",
                              G_CALLBACK (config_changed), edit);

    edit->priv = G_TYPE_INSTANCE_GET_PRIVATE (edit, MOO_TYPE_EDIT, MooEditPrivate);

    edit->priv->actions = moo_action_collection_new ("MooEdit", "MooEdit");

    indent = moo_indenter_new (edit, NULL);
    moo_text_view_set_indenter (MOO_TEXT_VIEW (edit), indent);
    g_object_unref (indent);
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
                              G_CALLBACK (_moo_edit_line_mark_moved),
                              edit);
    g_signal_connect_swapped (buffer, "line-mark-deleted",
                              G_CALLBACK (_moo_edit_line_mark_deleted),
                              edit);

    return object;
}


static void
moo_edit_finalize (GObject *object)
{
    MooEdit *edit = MOO_EDIT (object);

    g_free (edit->priv->filename);
    g_free (edit->priv->display_filename);
    g_free (edit->priv->display_basename);
    g_free (edit->priv->encoding);
    g_free (edit->priv->progress_text);

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

    if (edit->priv->progress)
    {
        g_critical ("%s: oops", G_STRLOC);
        edit->priv->progress = NULL;
        edit->priv->progressbar = NULL;
    }

    if (edit->priv->progress_timeout)
    {
        g_critical ("%s: oops", G_STRLOC);
        g_source_remove (edit->priv->progress_timeout);
        edit->priv->progress_timeout = 0;
    }

    if (edit->priv->update_bookmarks_idle)
    {
        g_source_remove (edit->priv->update_bookmarks_idle);
        edit->priv->update_bookmarks_idle = 0;
    }

    _moo_edit_delete_bookmarks (edit, TRUE);

    if (edit->priv->actions)
    {
        g_object_unref (edit->priv->actions);
        edit->priv->actions = NULL;
    }

    G_OBJECT_CLASS (moo_edit_parent_class)->dispose (object);
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


#if 0
void
_moo_edit_set_status (MooEdit        *edit,
                      MooEditStatus   status)
{
    g_return_if_fail (MOO_IS_EDIT (edit));

    if (edit->priv->status != status)
    {
        edit->priv->status = status;
        moo_edit_status_changed (edit);
    }
}
#endif


MooEditFileInfo*
moo_edit_file_info_new (const char *filename,
                        const char *encoding)
{
    MooEditFileInfo *info = g_new0 (MooEditFileInfo, 1);
    info->filename = g_strdup (filename);
    info->encoding = g_strdup (encoding);
    return info;
}


MooEditFileInfo*
moo_edit_file_info_copy (const MooEditFileInfo *info)
{
    MooEditFileInfo *copy;
    g_return_val_if_fail (info != NULL, NULL);
    copy = g_new (MooEditFileInfo, 1);
    copy->encoding = g_strdup (info->encoding);
    copy->filename = g_strdup (info->filename);
    return copy;
}

void
moo_edit_file_info_free (MooEditFileInfo *info)
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

    if (MOO_EDIT_IS_BUSY (edit) || MOO_EDIT_IS_MODIFIED (edit) || !MOO_EDIT_IS_UNTITLED (edit))
        return FALSE;

    gtk_text_buffer_get_bounds (get_buffer (edit), &start, &end);

    return !gtk_text_iter_compare (&start, &end);
}

gboolean
moo_edit_is_untitled (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    return MOO_EDIT_IS_UNTITLED (edit);
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


static gboolean
moo_edit_focus_in (GtkWidget     *widget,
                   GdkEventFocus *event)
{
    gboolean retval = FALSE;
    MooEdit *doc = MOO_EDIT (widget);

    _moo_editor_set_focused_doc (doc->priv->editor, doc);

    if (GTK_WIDGET_CLASS(moo_edit_parent_class)->focus_in_event)
        retval = GTK_WIDGET_CLASS(moo_edit_parent_class)->focus_in_event (widget, event);

    return retval;
}


static gboolean
moo_edit_focus_out (GtkWidget     *widget,
                    GdkEventFocus *event)
{
    gboolean retval = FALSE;
    MooEdit *doc = MOO_EDIT (widget);

    _moo_editor_unset_focused_doc (doc->priv->editor, doc);

    if (GTK_WIDGET_CLASS(moo_edit_parent_class)->focus_out_event)
        retval = GTK_WIDGET_CLASS(moo_edit_parent_class)->focus_out_event (widget, event);

    return retval;
}


GType
moo_edit_file_info_get_type (void)
{
    static GType type = 0;
    if (G_UNLIKELY (!type))
        type = g_boxed_type_register_static ("MooEditFileInfo",
                                             (GBoxedCopyFunc) moo_edit_file_info_copy,
                                             (GBoxedFreeFunc) moo_edit_file_info_free);
    return type;
}


char *
moo_edit_get_filename (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    return g_strdup (edit->priv->filename);
}

const char *
moo_edit_get_display_name (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    return edit->priv->display_filename;
}

const char *
moo_edit_get_display_basename (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    return edit->priv->display_basename;
}

char *
moo_edit_get_uri (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);

    if (edit->priv->filename)
        return g_filename_to_uri (edit->priv->filename, NULL, NULL);
    else
        return NULL;
}

const char *
moo_edit_get_encoding (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    return edit->priv->encoding;
}

void
_moo_edit_set_encoding (MooEdit    *edit,
                        const char *encoding)
{
    g_return_if_fail (MOO_IS_EDIT (edit));
    g_return_if_fail (encoding != NULL);
    g_free (edit->priv->encoding);
    edit->priv->encoding = g_strdup (encoding);
}


static GtkTextBuffer *
get_buffer (MooEdit *edit)
{
    return gtk_text_view_get_buffer (GTK_TEXT_VIEW (edit));
}


MooEditor *
moo_edit_get_editor (MooEdit *doc)
{
    g_return_val_if_fail (MOO_IS_EDIT (doc), NULL);
    return doc->priv->editor;
}


typedef void (*SetVarFunc) (MooEdit    *edit,
                            const char *name,
                            char       *val);

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
            moo_edit_config_parse_one (edit->config, "indent-use-tabs",
                                       spaces ? "false" : "true",
                                       MOO_EDIT_CONFIG_SOURCE_FILE);
    }
    else
    {
        moo_edit_config_parse_one (edit->config, name, val,
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
set_emacs_var (MooEdit    *edit,
               const char *name,
               char       *val)
{
    if (!g_ascii_strcasecmp (name, "mode"))
    {
        moo_edit_config_parse_one (edit->config, "lang", val,
                                   MOO_EDIT_CONFIG_SOURCE_FILE);
    }
    else if (!g_ascii_strcasecmp (name, "tab-width"))
    {
        moo_edit_config_parse_one (edit->config, "tab-width", val,
                                   MOO_EDIT_CONFIG_SOURCE_FILE);
    }
    else if (!g_ascii_strcasecmp (name, "c-basic-offset"))
    {
        moo_edit_config_parse_one (edit->config, "indent-width", val,
                                   MOO_EDIT_CONFIG_SOURCE_FILE);
    }
    else if (!g_ascii_strcasecmp (name, "indent-tabs-mode"))
    {
        if (!g_ascii_strcasecmp (val, "nil"))
            moo_edit_config_parse_one (edit->config, "indent-use-tabs", "false",
                                       MOO_EDIT_CONFIG_SOURCE_FILE);
        else
            moo_edit_config_parse_one (edit->config, "indent-use-tabs", "true",
                                       MOO_EDIT_CONFIG_SOURCE_FILE);
    }
}

static void
parse_emacs_mode_string (MooEdit *edit,
                         char    *string)
{
    MooLangMgr *mgr;

    g_strstrip (string);

    mgr = moo_editor_get_lang_mgr (edit->priv->editor);

    if (_moo_lang_mgr_find_lang (mgr, string))
        set_emacs_var (edit, "mode", string);
    else
        parse_mode_string (edit, string, ":", set_emacs_var);
}


static void
parse_moo_mode_string (MooEdit *edit,
                       char    *string)
{
    moo_edit_config_parse (edit->config, string, MOO_EDIT_CONFIG_SOURCE_FILE);
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
        parse_kate_mode_string (edit, start);
        return;
    }

    if ((start = strstr (string, EMACS_MODE_STRING)))
    {
        start += strlen (EMACS_MODE_STRING);

        if ((end = strstr (start, EMACS_MODE_STRING)) && end > start)
        {
            end[0] = 0;
            parse_emacs_mode_string (edit, start);
            return;
        }
    }

    if ((start = strstr (string, MOO_MODE_STRING)))
    {
        start += strlen (MOO_MODE_STRING);

        if ((end = strstr (start, MOO_MODE_STRING)) && end > start)
        {
            end[0] = 0;
            parse_moo_mode_string (edit, start);
            return;
        }
    }
}

static void
try_mode_strings (MooEdit *edit)
{
    GtkTextBuffer *buffer = get_buffer (edit);
    GtkTextIter start, end;
    char *first = NULL, *second = NULL, *last = NULL;

    gtk_text_buffer_get_start_iter (buffer, &start);

    if (!gtk_text_iter_ends_line (&start))
    {
        end = start;
        gtk_text_iter_forward_to_line_end (&end);
        first = gtk_text_buffer_get_slice (buffer, &start, &end, TRUE);
    }

    if (gtk_text_iter_forward_line (&start))
    {
        if (!gtk_text_iter_ends_line (&start))
        {
            end = start;
            gtk_text_iter_forward_to_line_end (&end);
            second = gtk_text_buffer_get_slice (buffer, &start, &end, TRUE);
        }
    }

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

    if (gtk_text_iter_get_line (&start) != 1 &&
        gtk_text_iter_get_line (&start) != 2)
            last = gtk_text_buffer_get_slice (buffer, &start, &end, TRUE);

    if (first)
        try_mode_string (edit, first);
    if (second)
        try_mode_string (edit, second);
    if (last)
        try_mode_string (edit, last);

    g_free (first);
    g_free (second);
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

    old_lang = moo_text_view_get_lang (MOO_TEXT_VIEW (edit));

    if (old_lang != lang)
    {
        moo_text_view_set_lang (MOO_TEXT_VIEW (edit), lang);
        _moo_lang_mgr_update_config (moo_editor_get_lang_mgr (edit->priv->editor),
                                     edit->config, _moo_lang_id (lang));
        _moo_edit_update_config_from_global (edit);
        g_object_notify (G_OBJECT (edit), "has-comments");
    }
}


static void
moo_edit_apply_lang_config (MooEdit *edit)
{
    const char *lang_id = moo_edit_config_get_string (edit->config, "lang");
    MooLangMgr *mgr = moo_editor_get_lang_mgr (edit->priv->editor);
    MooLang *lang = lang_id ? _moo_lang_mgr_find_lang (mgr, lang_id) : NULL;
    moo_edit_set_lang (edit, lang);
}

static void
moo_edit_apply_config (MooEdit *edit)
{
    GtkWrapMode wrap_mode;
    gboolean line_numbers;
    guint tab_width;

    moo_edit_apply_lang_config (edit);

    moo_edit_config_get (edit->config,
                         "wrap-mode", &wrap_mode,
                         "show-line-numbers", &line_numbers,
                         "tab-width", &tab_width,
                         NULL);

    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (edit), wrap_mode);
    moo_text_view_set_show_line_numbers (MOO_TEXT_VIEW (edit), line_numbers);
    moo_text_view_set_tab_width (MOO_TEXT_VIEW (edit), tab_width);
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
                _moo_idle_add_full (G_PRIORITY_HIGH,
                                    (GSourceFunc) do_apply_config,
                                    edit, NULL);
}


static void
moo_edit_config_notify (MooEdit        *edit,
                        guint           var_id,
                        G_GNUC_UNUSED GParamSpec *pspec)
{
    if (var_id == _moo_edit_settings[MOO_EDIT_SETTING_LANG])
        moo_edit_apply_lang_config (edit);
    else
        moo_edit_queue_apply_config (edit);
}


static void
_moo_edit_update_config_from_global (MooEdit *edit)
{
    g_return_if_fail (MOO_IS_EDIT (edit));
    /* XXX */
    moo_edit_config_unset_by_source (edit->config,
                                     MOO_EDIT_CONFIG_SOURCE_AUTO);
}


void
_moo_edit_update_lang_config (void)
{
    GSList *l;

    for (l = _moo_edit_instances; l != NULL; l = l->next)
    {
        MooEdit *edit = l->data;
        _moo_lang_mgr_update_config (moo_editor_get_lang_mgr (edit->priv->editor), edit->config,
                                     _moo_lang_id (moo_text_view_get_lang (MOO_TEXT_VIEW (edit))));
    }
}


static void
moo_edit_filename_changed (MooEdit    *edit,
                           const char *filename)
{
    gboolean lang_changed = FALSE;
    MooLang *lang = NULL, *old_lang = NULL;
    const char *lang_id = NULL;
    char *filter_config = NULL;

    old_lang = moo_text_view_get_lang (MOO_TEXT_VIEW (edit));

    _moo_edit_freeze_config_notify (edit);

    moo_edit_config_unset_by_source (edit->config, MOO_EDIT_CONFIG_SOURCE_FILE);
    _moo_edit_update_config_from_global (edit);

    if (filename)
    {
        MooLangMgr *mgr = moo_editor_get_lang_mgr (edit->priv->editor);
        lang = moo_lang_mgr_get_lang_for_file (mgr, filename);
        lang_id = lang ? _moo_lang_id (lang) : NULL;
        filter_config = _moo_edit_filter_settings_get_for_file (filename);
    }

    moo_edit_config_set (edit->config, MOO_EDIT_CONFIG_SOURCE_FILENAME,
                         "lang", lang_id, "indent", NULL, NULL);

    if (filter_config)
        moo_edit_config_parse (edit->config, filter_config,
                               MOO_EDIT_CONFIG_SOURCE_FILENAME);

    try_mode_strings (edit);

    lang_id = moo_edit_config_get_string (edit->config, "lang");

    if (!lang_id)
        lang_id = MOO_LANG_NONE;

    lang_changed = strcmp (lang_id, _moo_lang_id (old_lang)) != 0;

    if (!lang_changed)
    {
        _moo_lang_mgr_update_config (moo_editor_get_lang_mgr (edit->priv->editor),
                                     edit->config, _moo_lang_id (lang));
        _moo_edit_update_config_from_global (edit);
    }

    _moo_edit_thaw_config_notify (edit);

    g_free (filter_config);
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


static void
_moo_edit_freeze_config_notify (MooEdit *edit)
{
    g_return_if_fail (MOO_IS_EDIT (edit));
    g_object_freeze_notify (G_OBJECT (edit->config));
}

static void
_moo_edit_thaw_config_notify (MooEdit *edit)
{
    g_return_if_fail (MOO_IS_EDIT (edit));
    g_object_thaw_notify (G_OBJECT (edit->config));
}


static gboolean
find_uri_atom (GdkDragContext *context)
{
    GList *targets;
    GdkAtom atom;

    atom = gdk_atom_intern ("text/uri-list", FALSE);
    targets = context->targets;

    while (targets)
    {
        if (targets->data == GUINT_TO_POINTER (atom))
            return TRUE;
        targets = targets->next;
    }

    return FALSE;
}


static gboolean
moo_edit_drag_motion (GtkWidget      *widget,
                      GdkDragContext *context,
                      gint            x,
                      gint            y,
                      guint           time)
{
    if (find_uri_atom (context))
        return FALSE;

    return GTK_WIDGET_CLASS(moo_edit_parent_class)->drag_motion (widget, context, x, y, time);
}


static gboolean
moo_edit_drag_drop (GtkWidget      *widget,
                    GdkDragContext *context,
                    gint            x,
                    gint            y,
                    guint           time)
{
    if (find_uri_atom (context))
        return FALSE;

    return GTK_WIDGET_CLASS(moo_edit_parent_class)->drag_drop (widget, context, x, y, time);
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

    single = _moo_lang_get_line_comment (lang) != NULL;
    multi = _moo_lang_get_block_comment_start (lang) && _moo_lang_get_block_comment_end (lang);

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

    gtk_text_buffer_begin_user_action (buffer);

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
        line_comment (buffer, _moo_lang_get_line_comment (lang), &start, &end);
    else
        block_comment (buffer,
                       _moo_lang_get_block_comment_start (lang),
                       _moo_lang_get_block_comment_end (lang),
                       &start, &end);


    if (adjust_selection)
    {
        const char *mark = move_insert ? "insert" : "selection_bound";
        gtk_text_buffer_get_iter_at_line_offset (buffer, &start,
                                                 sel_start_line,
                                                 sel_start_offset);
        gtk_text_buffer_move_mark_by_name (buffer, mark, &start);
    }

    gtk_text_buffer_end_user_action (buffer);
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

    gtk_text_buffer_begin_user_action (buffer);

    /* FIXME */
    if (single_line)
        line_uncomment (buffer, _moo_lang_get_line_comment (lang), &start, &end);
    else
        block_uncomment (buffer,
                         _moo_lang_get_block_comment_start (lang),
                         _moo_lang_get_block_comment_end (lang),
                         &start, &end);

    gtk_text_buffer_end_user_action (buffer);
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

    gtk_widget_size_request (GTK_WIDGET (menu), &req);

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
    GtkMenu *menu;

    window = moo_edit_get_window (edit);
    xml = moo_editor_get_doc_ui_xml (edit->priv->editor);
    g_return_if_fail (xml != NULL);

    menu = moo_ui_xml_create_widget (xml, MOO_UI_MENU, "Editor/Popup", edit->priv->actions,
                                     window ? MOO_WINDOW(window)->accel_group : NULL);
    g_return_if_fail (menu != NULL);
    MOO_OBJECT_REF_SINK (menu);

    _moo_edit_check_actions (edit);

    if (event)
    {
        gtk_menu_popup (menu, NULL, NULL, NULL, NULL,
                        event->button, event->time);
    }
    else
    {
        gtk_menu_popup (menu, NULL, NULL,
                        popup_position_func, edit,
                        0, gtk_get_current_event_time ());
        gtk_menu_shell_select_first (GTK_MENU_SHELL (menu), FALSE);
    }

    g_object_unref (menu);
}


static gboolean
moo_edit_popup_menu (GtkWidget *widget)
{
    _moo_edit_do_popup (MOO_EDIT (widget), NULL);
    return TRUE;
}


/*****************************************************************************/
/* progress dialogs and stuff
 */

MooEditState
moo_edit_get_state (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), MOO_EDIT_STATE_NORMAL);
    return edit->priv->state;
}


static void
position_progress (MooEdit *edit)
{
    GtkAllocation *allocation;
    int x, y;

    g_return_if_fail (MOO_IS_EDIT (edit));
    g_return_if_fail (GTK_IS_WIDGET (edit->priv->progress));

    if (!GTK_WIDGET_REALIZED (edit))
        return;

    allocation = &GTK_WIDGET(edit)->allocation;

    x = allocation->width/2 - PROGRESS_WIDTH/2;
    y = allocation->height/2 - PROGRESS_HEIGHT/2;
    gtk_text_view_move_child (GTK_TEXT_VIEW (edit),
                              edit->priv->progress,
                              x, y);
}


static void
update_progress (MooEdit *edit)
{
    g_return_if_fail (MOO_IS_EDIT (edit));
    g_return_if_fail (edit->priv->progress_text != NULL);
    g_return_if_fail (edit->priv->state != MOO_EDIT_STATE_NORMAL);

    if (edit->priv->progressbar)
        gtk_progress_bar_set_text (GTK_PROGRESS_BAR (edit->priv->progressbar),
                                   edit->priv->progress_text);
}


void
_moo_edit_set_progress_text (MooEdit    *edit,
                             const char *text)
{
    g_free (edit->priv->progress_text);
    edit->priv->progress_text = g_strdup (text);
    update_progress (edit);
}


static gboolean
pulse_progress (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    g_return_val_if_fail (GTK_IS_WIDGET (edit->priv->progressbar), FALSE);
    gtk_progress_bar_pulse (GTK_PROGRESS_BAR (edit->priv->progressbar));
    update_progress (edit);
    return TRUE;
}


static void
progress_cancel_clicked (MooEdit *doc)
{
    g_return_if_fail (MOO_IS_EDIT (doc));
    if (doc->priv->state && doc->priv->cancel_op)
        doc->priv->cancel_op (doc->priv->cancel_data);
}


static gboolean
show_progress (MooEdit *edit)
{
    MooGladeXML *xml;
    GtkButton *cancel;

    edit->priv->progress_timeout = 0;

    g_return_val_if_fail (!edit->priv->progress, FALSE);

    xml = moo_glade_xml_new_from_buf (mooeditprogress_glade_xml, -1,
                                      "eventbox", GETTEXT_PACKAGE, NULL);
    g_return_val_if_fail (xml != NULL, FALSE);

    edit->priv->progress = moo_glade_xml_get_widget (xml, "eventbox");
    edit->priv->progressbar = moo_glade_xml_get_widget (xml, "progressbar");
    g_assert (GTK_IS_WIDGET (edit->priv->progressbar));

    cancel = moo_glade_xml_get_widget (xml, "cancel");
    g_signal_connect_swapped (cancel, "clicked",
                              G_CALLBACK (progress_cancel_clicked),
                              MOO_EDIT (edit));

    gtk_text_view_add_child_in_window (GTK_TEXT_VIEW (edit),
                                       edit->priv->progress,
                                       GTK_TEXT_WINDOW_WIDGET,
                                       0, 0);
    position_progress (edit);
    update_progress (edit);

    edit->priv->progress_timeout =
            _moo_timeout_add (PROGRESS_TIMEOUT,
                              (GSourceFunc) pulse_progress,
                              edit);

    g_object_unref (xml);
    return FALSE;
}


void
_moo_edit_set_state (MooEdit        *edit,
                     MooEditState    state,
                     const char     *text,
                     GDestroyNotify  cancel,
                     gpointer        data)
{
    g_return_if_fail (state == MOO_EDIT_STATE_NORMAL ||
                      edit->priv->state == MOO_EDIT_STATE_NORMAL);

    edit->priv->cancel_op = cancel;
    edit->priv->cancel_data = data;

    if (state == edit->priv->state)
        return;

    edit->priv->state = state;
    gtk_text_view_set_editable (GTK_TEXT_VIEW (edit), !state);

    if (!state)
    {
        if (edit->priv->progress)
        {
            GtkWidget *tmp = edit->priv->progress;
            edit->priv->progress = NULL;
            edit->priv->progressbar = NULL;
            gtk_widget_destroy (tmp);
        }

        g_free (edit->priv->progress_text);
        edit->priv->progress_text = NULL;

        if (edit->priv->progress_timeout)
            g_source_remove (edit->priv->progress_timeout);
        edit->priv->progress_timeout = 0;
    }
    else
    {
        if (!edit->priv->progress_timeout)
            edit->priv->progress_timeout =
                    _moo_timeout_add (PROGRESS_TIMEOUT,
                                      (GSourceFunc) show_progress,
                                      edit);
        edit->priv->progress_text = g_strdup (text);
    }
}
