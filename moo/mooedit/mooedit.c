/*
 *   mooedit.c
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
#include "mooedit/mooeditaction-factory.h"
#include "mooedit/mooedit-private.h"
#include "mooedit/mooeditbookmark.h"
#include "mooedit/mooeditdialogs.h"
#include "mooedit/mooeditprefs.h"
#include "mooedit/mooeditbuffer-impl.h"
#include "mooedit/mooeditview-impl.h"
#include "mooedit/mooeditfiltersettings.h"
#include "mooedit/mooeditor-impl.h"
#include "mooedit/moolangmgr.h"
#include "marshals.h"
#include "mooutils/mooutils-fs.h"
#include "mooutils/mooi18n.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/mootype-macros.h"
#include "mooutils/mooatom.h"
#include "mooutils/moocompat.h"
#include <string.h>
#include <stdlib.h>

#define KEY_ENCODING "encoding"
#define KEY_LINE "line"

MOO_DEFINE_OBJECT_ARRAY (MooEditArray, moo_edit_array, MooEdit)

MooEditList *_moo_edit_instances = NULL;

static void     moo_edit_finalize           (GObject        *object);

static void     moo_edit_set_property       (GObject        *object,
                                             guint           prop_id,
                                             const GValue   *value,
                                             GParamSpec     *pspec);
static void     moo_edit_get_property       (GObject        *object,
                                             guint           prop_id,
                                             GValue         *value,
                                             GParamSpec     *pspec);

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
    PROP_HAS_COMMENTS,
    PROP_LINE_END_TYPE,
    PROP_ENCODING
};

G_DEFINE_TYPE (MooEdit, moo_edit, G_TYPE_OBJECT)

static void
moo_edit_class_init (MooEditClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->set_property = moo_edit_set_property;
    gobject_class->get_property = moo_edit_get_property;
    gobject_class->finalize = moo_edit_finalize;

    klass->filename_changed = moo_edit_filename_changed;
    klass->config_notify = moo_edit_config_notify;

    g_type_class_add_private (klass, sizeof (MooEditPrivate));

    g_object_class_install_property (gobject_class,
                                     PROP_EDITOR,
                                     g_param_spec_object ("editor",
                                             "editor",
                                             "editor",
                                             MOO_TYPE_EDITOR,
                                             (GParamFlags) (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY)));

    g_object_class_install_property (gobject_class,
                                     PROP_ENABLE_BOOKMARKS,
                                     g_param_spec_boolean ("enable-bookmarks",
                                             "enable-bookmarks",
                                             "enable-bookmarks",
                                             TRUE,
                                             (GParamFlags) (G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));

    g_object_class_install_property (gobject_class,
                                     PROP_HAS_COMMENTS,
                                     g_param_spec_boolean ("has-comments",
                                             "has-comments",
                                             "has-comments",
                                             FALSE,
                                             G_PARAM_READABLE));

    g_object_class_install_property (gobject_class, PROP_LINE_END_TYPE,
        g_param_spec_enum ("line-end-type", "line-end-type", "line-end-type",
                           MOO_TYPE_LINE_END_TYPE, MOO_LE_NONE,
                           (GParamFlags) G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_ENCODING,
                                     g_param_spec_string ("encoding",
                                             "encoding",
                                             "encoding",
                                             NULL,
                                             (GParamFlags) G_PARAM_READWRITE));

    signals[CONFIG_NOTIFY] =
            g_signal_new ("config-notify",
                          G_OBJECT_CLASS_TYPE (klass),
                          (GSignalFlags) (G_SIGNAL_RUN_FIRST | G_SIGNAL_DETAILED),
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
                                (GSignalFlags) (G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION),
                                G_CALLBACK (moo_edit_comment),
                                NULL, NULL,
                                _moo_marshal_VOID__VOID,
                                G_TYPE_NONE, 0);

    signals[UNCOMMENT] =
            _moo_signal_new_cb ("uncomment",
                                G_OBJECT_CLASS_TYPE (klass),
                                (GSignalFlags) (G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION),
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
    GtkTextBuffer *buffer;

    edit->config = moo_edit_config_new ();
    g_signal_connect_swapped (edit->config, "notify",
                              G_CALLBACK (config_changed), edit);

    edit->priv = G_TYPE_INSTANCE_GET_PRIVATE (edit, MOO_TYPE_EDIT, MooEditPrivate);

    edit->priv->actions = moo_action_collection_new ("MooEdit", "MooEdit");
    edit->priv->line_end_type = MOO_LE_NONE;

    edit->priv->view = g_object_new (MOO_TYPE_EDIT_VIEW, NULL);
    g_object_ref_sink (edit->priv->view);
    _moo_edit_view_set_doc (edit->priv->view, edit);

    indent = moo_indenter_new (edit);
    moo_text_view_set_indenter (MOO_TEXT_VIEW (edit->priv->view), indent);
    g_object_unref (indent);

    _moo_edit_add_class_actions (edit);
    _moo_edit_instances = moo_edit_list_prepend (_moo_edit_instances, edit);

    edit->priv->modified_changed_handler_id =
            g_signal_connect (get_buffer (edit),
                              "modified-changed",
                              G_CALLBACK (modified_changed_cb),
                              edit);

    _moo_edit_set_file (edit, NULL, NULL);

    buffer = get_buffer (edit);
    g_signal_connect_swapped (buffer, "line-mark-moved",
                              G_CALLBACK (_moo_edit_line_mark_moved),
                              edit);
    g_signal_connect_swapped (buffer, "line-mark-deleted",
                              G_CALLBACK (_moo_edit_line_mark_deleted),
                              edit);
}


static void
moo_edit_finalize (GObject *object)
{
    MooEdit *edit = MOO_EDIT (object);

    moo_file_free (edit->priv->file);
    g_free (edit->priv->filename);
    g_free (edit->priv->norm_filename);
    g_free (edit->priv->display_filename);
    g_free (edit->priv->display_basename);
    g_free (edit->priv->encoding);

    moo_assert (!edit->config);
    moo_assert (!edit->priv->view);

    G_OBJECT_CLASS (moo_edit_parent_class)->finalize (object);
}


static void
moo_edit_unset_view (MooEdit *edit)
{
    if (edit->priv->view)
    {
        MooEditView *view = edit->priv->view;
        edit->priv->view = NULL;
        _moo_edit_view_set_doc (view, NULL);
        g_object_unref (view);
    }

    if (edit->priv->apply_config_idle)
    {
        g_source_remove (edit->priv->apply_config_idle);
        edit->priv->apply_config_idle = 0;
    }

    if (edit->priv->update_bookmarks_idle)
    {
        g_source_remove (edit->priv->update_bookmarks_idle);
        edit->priv->update_bookmarks_idle = 0;
    }

    _moo_edit_delete_bookmarks (edit, TRUE);
}

void
_moo_edit_closed (MooEdit *doc)
{
    moo_return_if_fail (MOO_IS_EDIT (doc));

    _moo_edit_instances = moo_edit_list_remove (_moo_edit_instances, doc);

    moo_edit_unset_view (doc);

    if (doc->config)
    {
        g_signal_handlers_disconnect_by_func (doc->config,
                                              (gpointer) config_changed,
                                              doc);
        g_object_unref (doc->config);
        doc->config = NULL;
    }

    if (doc->priv->file_monitor_id)
    {
        _moo_edit_stop_file_watch (doc);
        doc->priv->file_monitor_id = 0;
    }

    if (doc->priv->actions)
    {
        g_object_unref (doc->priv->actions);
        doc->priv->actions = NULL;
    }
}

void
_moo_edit_view_destroyed (MooEdit *doc)
{
    moo_return_if_fail (MOO_IS_EDIT (doc));
    moo_edit_unset_view (doc);
}


MooActionCollection *
_moo_edit_get_action_collection (MooEdit *edit)
{
    moo_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    return edit->priv->actions;
}


static void
modified_changed_cb (GtkTextBuffer      *buffer,
                     MooEdit            *edit)
{
    moo_edit_set_modified (edit, gtk_text_buffer_get_modified (buffer));
}


static void
modify_status (MooEdit       *edit,
               MooEditStatus  status,
               gboolean       add)
{
    if (add)
        edit->priv->status = (MooEditStatus) (edit->priv->status | status);
    else
        edit->priv->status = (MooEditStatus) (edit->priv->status & ~status);
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

    modify_status (edit, MOO_EDIT_MODIFIED, modified);

    moo_edit_status_changed (edit);
}


void
moo_edit_set_clean (MooEdit            *edit,
                    gboolean            clean)
{
    g_return_if_fail (MOO_IS_EDIT (edit));
    modify_status (edit, MOO_EDIT_CLEAN, clean);
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
    g_return_val_if_fail (MOO_IS_EDIT (edit), (MooEditStatus) 0);
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
            edit->priv->editor = (MooEditor*) g_value_get_object (value);
            break;

        case PROP_ENABLE_BOOKMARKS:
            moo_edit_set_enable_bookmarks (edit, g_value_get_boolean (value));
            break;

        case PROP_ENCODING:
            moo_edit_set_encoding (edit, g_value_get_string (value));
            break;

        case PROP_LINE_END_TYPE:
            moo_edit_set_line_end_type (edit, (MooLineEndType) g_value_get_enum (value));
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

        case PROP_ENCODING:
            g_value_set_string (value, edit->priv->encoding);
            break;

        case PROP_LINE_END_TYPE:
            g_value_set_enum (value, edit->priv->line_end_type);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


GFile *
moo_edit_get_file (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    return edit->priv->file ? g_file_dup (edit->priv->file) : NULL;
}

char *
moo_edit_get_filename (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    return g_strdup (edit->priv->filename);
}

char *
moo_edit_get_norm_filename (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    return g_strdup (edit->priv->norm_filename);
}

char *
moo_edit_get_utf8_filename (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    return edit->priv->filename ? g_strdup (edit->priv->display_filename) : NULL;
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
    return edit->priv->file ? g_file_get_uri (edit->priv->file) : NULL;
}

const char *
moo_edit_get_encoding (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    return edit->priv->encoding;
}

void
moo_edit_set_encoding (MooEdit    *edit,
                       const char *encoding)
{
    g_return_if_fail (MOO_IS_EDIT (edit));
    g_return_if_fail (encoding != NULL);

    if (!_moo_str_equal (encoding, edit->priv->encoding))
    {
        MOO_ASSIGN_STRING (edit->priv->encoding, encoding);
        g_object_notify (G_OBJECT (edit), "encoding");
    }
}


static GtkTextBuffer *
get_buffer (MooEdit *edit)
{
    moo_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    return GTK_TEXT_BUFFER (moo_edit_view_get_buffer (edit->priv->view));
}


MooEditor *
moo_edit_get_editor (MooEdit *doc)
{
    g_return_val_if_fail (MOO_IS_EDIT (doc), NULL);
    return doc->priv->editor;
}

MooEditView *
moo_edit_get_view (MooEdit *doc)
{
    g_return_val_if_fail (MOO_IS_EDIT (doc), NULL);
    return doc->priv->view;
}

MooEditBuffer *
moo_edit_get_buffer (MooEdit *doc)
{
    return moo_edit_view_get_buffer (moo_edit_get_view (doc));
}

MooEditWindow *
moo_edit_get_window (MooEdit *doc)
{
    return moo_edit_view_get_window (moo_edit_get_view (doc));
}


void
_moo_edit_history_item_set_encoding (MdHistoryItem *item,
                                     const char    *encoding)
{
    g_return_if_fail (item != NULL);
    md_history_item_set (item, KEY_ENCODING, encoding);
}

void
_moo_edit_history_item_set_line (MdHistoryItem *item,
                                 int            line)
{
    char *value = NULL;

    g_return_if_fail (item != NULL);

    if (line >= 0)
        value = g_strdup_printf ("%d", line + 1);

    md_history_item_set (item, KEY_LINE, value);
    g_free (value);
}

const char *
_moo_edit_history_item_get_encoding (MdHistoryItem *item)
{
    g_return_val_if_fail (item != NULL, NULL);
    return md_history_item_get (item, KEY_ENCODING);
}

int
_moo_edit_history_item_get_line (MdHistoryItem *item)
{
    const char *strval;

    g_return_val_if_fail (item != NULL, -1);

    strval = md_history_item_get (item, KEY_LINE);

    if (strval && strval[0])
        return strtol (strval, NULL, 10) - 1;
    else
        return -1;
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

    mgr = moo_lang_mgr_default ();

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


MooLang *
moo_edit_get_lang (MooEdit *doc)
{
    g_return_val_if_fail (MOO_IS_EDIT (doc), NULL);
    return moo_text_view_get_lang (MOO_TEXT_VIEW (moo_edit_get_view (doc)));
}

static void
moo_edit_set_lang (MooEdit *doc,
                   MooLang *lang)
{
    MooLang *old_lang;
    MooEditView *view = moo_edit_get_view (doc);

    old_lang = moo_text_view_get_lang (MOO_TEXT_VIEW (view));

    if (old_lang != lang)
    {
        moo_text_view_set_lang (MOO_TEXT_VIEW (view), lang);
        _moo_lang_mgr_update_config (moo_lang_mgr_default (),
                                     doc->config,
                                     _moo_lang_id (lang));
        _moo_edit_update_config_from_global (doc);
        g_object_notify (G_OBJECT (doc), "has-comments");
    }
}


static void
moo_edit_apply_lang_config (MooEdit *edit)
{
    const char *lang_id = moo_edit_config_get_string (edit->config, "lang");
    MooLangMgr *mgr = moo_lang_mgr_default ();
    MooLang *lang = lang_id ? _moo_lang_mgr_find_lang (mgr, lang_id) : NULL;
    moo_edit_set_lang (edit, lang);
}

static void
moo_edit_apply_config (MooEdit *edit)
{
    MooEditView *view;
    GtkWrapMode wrap_mode;
    gboolean line_numbers;
    guint tab_width;
    char *word_chars;

    moo_edit_apply_lang_config (edit);

    moo_edit_config_get (edit->config,
                         "wrap-mode", &wrap_mode,
                         "show-line-numbers", &line_numbers,
                         "tab-width", &tab_width,
                         "word-chars", &word_chars,
                         (char*) 0);

    view = moo_edit_get_view (edit);
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (view), wrap_mode);
    moo_text_view_set_show_line_numbers (MOO_TEXT_VIEW (view), line_numbers);
    moo_text_view_set_tab_width (MOO_TEXT_VIEW (view), tab_width);
    moo_text_view_set_word_chars (MOO_TEXT_VIEW (view), word_chars);

    g_free (word_chars);
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
                moo_idle_add_full (G_PRIORITY_HIGH,
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
    MooEditList *l;

    for (l = _moo_edit_instances; l != NULL; l = l->next)
    {
        MooEdit *edit = l->data;
        _moo_lang_mgr_update_config (moo_lang_mgr_default (), edit->config,
            _moo_lang_id (moo_text_view_get_lang (MOO_TEXT_VIEW (moo_edit_get_view (edit)))));
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

    old_lang = moo_text_view_get_lang (MOO_TEXT_VIEW (moo_edit_get_view (edit)));

    _moo_edit_freeze_config_notify (edit);

    moo_edit_config_unset_by_source (edit->config, MOO_EDIT_CONFIG_SOURCE_FILE);
    _moo_edit_update_config_from_global (edit);

    if (filename)
    {
        MooLangMgr *mgr = moo_lang_mgr_default ();
        lang = moo_lang_mgr_get_lang_for_file (mgr, filename);
        lang_id = lang ? _moo_lang_id (lang) : NULL;
    }

    moo_edit_config_set (edit->config, MOO_EDIT_CONFIG_SOURCE_FILENAME,
                         "lang", lang_id, "indent", (void*) NULL, (char*) NULL);

    filter_config = _moo_edit_filter_settings_get_for_doc (edit);

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
        _moo_lang_mgr_update_config (moo_lang_mgr_default (),
                                     edit->config,
                                     _moo_lang_id (lang));
        _moo_edit_update_config_from_global (edit);
    }

    _moo_edit_thaw_config_notify (edit);

    g_free (filter_config);
}


void
moo_edit_reload (MooEdit     *edit,
                 const char  *encoding,
                 GError     **error)
{
    _moo_editor_reload (edit->priv->editor, edit, encoding, error);
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

    lang = moo_text_view_get_lang (MOO_TEXT_VIEW (moo_edit_get_view (edit)));

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
    found = moo_text_search_forward (start, comment_start, (MooTextSearchFlags) 0,
                                     &start1, &start2,
                                     &limit);

    if (!found)
    {
        gtk_text_iter_set_line_offset (&limit, 0);
        found = gtk_text_iter_backward_search (start, comment_start, (GtkTextSearchFlags) 0,
                                               &start1, &start2,
                                               &limit);
    }

    if (!found)
        return;

    limit = start2;
    found = gtk_text_iter_backward_search (end, comment_end, (GtkTextSearchFlags) 0,
                                           &end1, &end2, &limit);

    if (!found)
    {
        limit = *end;
        iter_to_line_end (&limit);
        found = moo_text_search_forward (end, comment_end, (MooTextSearchFlags) 0,
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

    lang = moo_text_view_get_lang (MOO_TEXT_VIEW (moo_edit_get_view (edit)));

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

    lang = moo_text_view_get_lang (MOO_TEXT_VIEW (moo_edit_get_view (edit)));

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


void
_moo_edit_ensure_newline (MooEdit *edit)
{
    GtkTextBuffer *buffer;
    GtkTextIter iter;

    g_return_if_fail (MOO_IS_EDIT (edit));

    buffer = get_buffer (edit);
    gtk_text_buffer_get_end_iter (buffer, &iter);

    if (!gtk_text_iter_starts_line (&iter))
        gtk_text_buffer_insert (buffer, &iter, "\n", -1);
}

void
_moo_edit_strip_whitespace (MooEdit *doc)
{
    GtkTextBuffer *buffer;
    GtkTextIter iter;

    g_return_if_fail (MOO_IS_EDIT (doc));

    buffer = get_buffer (doc);
    gtk_text_buffer_begin_user_action (buffer);

    for (gtk_text_buffer_get_start_iter (buffer, &iter);
         !gtk_text_iter_is_end (&iter);
         gtk_text_iter_forward_line (&iter))
    {
        GtkTextIter end;
        char *slice, *p;
        int len;

        if (gtk_text_iter_ends_line (&iter))
            continue;

        end = iter;
        gtk_text_iter_forward_to_line_end (&end);

        slice = gtk_text_buffer_get_slice (buffer, &iter, &end, TRUE);
        len = strlen (slice);
        g_assert (len > 0);

        for (p = slice + len; p > slice && (p[-1] == ' ' || p[-1] == '\t'); --p) ;

        if (*p)
        {
            gtk_text_iter_forward_chars (&iter, g_utf8_pointer_to_offset (slice, p));
            gtk_text_buffer_delete (buffer, &iter, &end);
        }

        g_free (slice);
    }

    gtk_text_buffer_end_user_action (buffer);
}


/*****************************************************************************/
/* progress dialog
 */

MooEditState
moo_edit_get_state (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), MOO_EDIT_STATE_NORMAL);
    return edit->priv->state;
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

    if (state != edit->priv->state)
    {
        edit->priv->state = state;
        _moo_edit_view_set_state (edit->priv->view, state, text, cancel, data);
    }
}
