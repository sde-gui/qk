/*
 *   mooedit.c
 *
 *   Copyright (C) 2004-2010 by Yevgen Muntyan <emuntyan@users.sourceforge.net>
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

/**
 * class:MooEdit: (parent GObject)
 **/

#define MOOEDIT_COMPILATION
#include "mooedit/mooeditaction-factory.h"
#include "mooedit/mooedit-private.h"
#include "mooedit/mooeditview-impl.h"
#include "mooedit/mooeditbookmark.h"
#include "mooedit/mooeditdialogs.h"
#include "mooedit/mooeditprefs.h"
#include "mooedit/mootextbuffer.h"
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
#include "mooedit/mooeditprogress-gxml.h"
#include <string.h>
#include <stdlib.h>

#define KEY_ENCODING "encoding"
#define KEY_LINE "line"

MOO_DEFINE_OBJECT_ARRAY (MooEdit, moo_edit)

MooEditList *_moo_edit_instances = NULL;
static guint moo_edit_apply_config_all_idle;

static GObject *moo_edit_constructor            (GType           type,
                                                 guint           n_construct_properties,
                                                 GObjectConstructParam *construct_param);
static void     moo_edit_finalize               (GObject        *object);
static void     moo_edit_dispose                (GObject        *object);

static void     moo_edit_set_property           (GObject        *object,
                                                 guint           prop_id,
                                                 const GValue   *value,
                                                 GParamSpec     *pspec);
static void     moo_edit_get_property           (GObject        *object,
                                                 guint           prop_id,
                                                 GValue         *value,
                                                 GParamSpec     *pspec);

static MooEditSaveResponse moo_edit_before_save (MooEdit        *doc,
                                                 GFile          *file);

static void     config_changed                  (MooEdit        *edit);
static void     update_config_from_mode_lines   (MooEdit        *doc);
static void     moo_edit_recheck_config         (MooEdit        *doc);

static void     modified_changed_cb             (GtkTextBuffer  *buffer,
                                                 MooEdit        *edit);

static MooLang *moo_edit_get_lang               (MooEdit        *doc);


enum {
    DOC_STATUS_CHANGED,
    FILENAME_CHANGED,
    BEFORE_SAVE,
    AFTER_SAVE,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

enum {
    PROP_0,
    PROP_EDITOR,
    PROP_ENABLE_BOOKMARKS,
    PROP_HAS_COMMENTS,
    PROP_LINE_END_TYPE,
    PROP_LANG,
    PROP_ENCODING
};

G_DEFINE_TYPE (MooEdit, moo_edit, G_TYPE_OBJECT)


static void
moo_edit_class_init (MooEditClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->set_property = moo_edit_set_property;
    gobject_class->get_property = moo_edit_get_property;
    gobject_class->constructor = moo_edit_constructor;
    gobject_class->finalize = moo_edit_finalize;
    gobject_class->dispose = moo_edit_dispose;

    klass->before_save = moo_edit_before_save;

    g_type_class_add_private (klass, sizeof (MooEditPrivate));

    g_object_class_install_property (gobject_class, PROP_EDITOR,
        g_param_spec_object ("editor", "editor", "editor",
                             MOO_TYPE_EDITOR, (GParamFlags) (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY)));

    g_object_class_install_property (gobject_class, PROP_ENABLE_BOOKMARKS,
        g_param_spec_boolean ("enable-bookmarks", "enable-bookmarks", "enable-bookmarks",
                              TRUE, (GParamFlags) (G_PARAM_READWRITE | G_PARAM_CONSTRUCT)));

    g_object_class_install_property (gobject_class, PROP_HAS_COMMENTS,
        g_param_spec_boolean ("has-comments", "has-comments", "has-comments",
                              FALSE, G_PARAM_READABLE));

    g_object_class_install_property (gobject_class, PROP_LINE_END_TYPE,
        g_param_spec_enum ("line-end-type", "line-end-type", "line-end-type",
                           MOO_TYPE_LINE_END_TYPE, MOO_LE_NONE,
                           (GParamFlags) G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class, PROP_ENCODING,
        g_param_spec_string ("encoding", "encoding", "encoding",
                             NULL, (GParamFlags) G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class, PROP_LANG,
        g_param_spec_object ("lang", "lang", "lang",
                             MOO_TYPE_LANG, G_PARAM_READABLE));

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
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[BEFORE_SAVE] =
            g_signal_new ("before-save",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooEditClass, before_save),
                          (GSignalAccumulator) _moo_signal_accumulator_save_response, NULL,
                          _moo_marshal_ENUM__OBJECT,
                          MOO_TYPE_EDIT_SAVE_RESPONSE, 1,
                          G_TYPE_FILE);

    signals[AFTER_SAVE] =
            g_signal_new ("after-save",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_FIRST,
                          G_STRUCT_OFFSET (MooEditClass, after_save),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    _moo_edit_init_config ();
    _moo_edit_class_init_actions (klass);
}


static void
moo_edit_init (MooEdit *edit)
{
    edit->priv = G_TYPE_INSTANCE_GET_PRIVATE (edit, MOO_TYPE_EDIT, MooEditPrivate);

    edit->config = moo_edit_config_new ();
    g_signal_connect_swapped (edit->config, "notify",
                              G_CALLBACK (config_changed),
                              edit);

    edit->priv->actions = moo_action_collection_new ("MooEdit", "MooEdit");

    edit->priv->line_end_type = MOO_LE_NONE;
}


static GObject *
moo_edit_constructor (GType                  type,
                      guint                  n_construct_properties,
                      GObjectConstructParam *construct_param)
{
    GObject *object;
    MooEdit *edit;
    GtkTextBuffer *buffer;
    MooIndenter *indent;

    object = G_OBJECT_CLASS (moo_edit_parent_class)->constructor (
        type, n_construct_properties, construct_param);

    edit = MOO_EDIT (object);

    edit->priv->view = g_object_new (MOO_TYPE_EDIT_VIEW, NULL);
    g_object_ref_sink (edit->priv->view);
    _moo_edit_view_set_doc (edit->priv->view, edit);

    _moo_edit_add_class_actions (edit);
    _moo_edit_instances = moo_edit_list_prepend (_moo_edit_instances, edit);

    indent = moo_indenter_new (edit);
    moo_text_view_set_indenter (MOO_TEXT_VIEW (edit->priv->view), indent);
    g_object_unref (indent);

    buffer = moo_edit_get_buffer (edit);

    edit->priv->modified_changed_handler_id =
            g_signal_connect (buffer,
                              "modified-changed",
                              G_CALLBACK (modified_changed_cb),
                              edit);

    _moo_edit_set_file (edit, NULL, NULL);

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

    moo_file_free (edit->priv->file);
    g_free (edit->priv->filename);
    g_free (edit->priv->norm_name);
    g_free (edit->priv->display_filename);
    g_free (edit->priv->display_basename);
    g_free (edit->priv->encoding);

    G_OBJECT_CLASS (moo_edit_parent_class)->finalize (object);
}

void
_moo_edit_closed (MooEdit *doc)
{
    moo_return_if_fail (MOO_IS_EDIT (doc));

    _moo_edit_remove_untitled (doc);
    _moo_edit_instances = moo_edit_list_remove (_moo_edit_instances, doc);

    if (doc->priv->view)
    {
        g_object_unref (doc->priv->view);
        doc->priv->view = NULL;
    }

    if (doc->config)
    {
        g_signal_handlers_disconnect_by_func (doc->config,
                                              (gpointer) config_changed,
                                              doc);
        g_object_unref (doc->config);
        doc->config = NULL;
    }

    if (doc->priv->apply_config_idle)
    {
        g_source_remove (doc->priv->apply_config_idle);
        doc->priv->apply_config_idle = 0;
    }

    if (doc->priv->file_monitor_id)
    {
        _moo_edit_stop_file_watch (doc);
        doc->priv->file_monitor_id = 0;
    }

    if (doc->priv->update_bookmarks_idle)
    {
        g_source_remove (doc->priv->update_bookmarks_idle);
        doc->priv->update_bookmarks_idle = 0;
    }

    _moo_edit_delete_bookmarks (doc, TRUE);

    if (doc->priv->actions)
    {
        g_object_unref (doc->priv->actions);
        doc->priv->actions = NULL;
    }
}

static void
moo_edit_dispose (GObject *object)
{
    _moo_edit_closed (MOO_EDIT (object));
    G_OBJECT_CLASS (moo_edit_parent_class)->dispose (object);
}


MooActionCollection *
_moo_edit_get_actions (MooEdit *doc)
{
    moo_return_val_if_fail (MOO_IS_EDIT (doc), NULL);
    return doc->priv->actions;
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

/**
 * moo_edit_set_modified:
 **/
void
moo_edit_set_modified (MooEdit            *edit,
                       gboolean            modified)
{
    gboolean buf_modified;
    GtkTextBuffer *buffer;

    g_return_if_fail (MOO_IS_EDIT (edit));

    buffer = moo_edit_get_buffer (edit);

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

    _moo_edit_status_changed (edit);
}


/**
 * moo_edit_set_clean:
 **/
void
moo_edit_set_clean (MooEdit  *edit,
                    gboolean  clean)
{
    g_return_if_fail (MOO_IS_EDIT (edit));
    modify_status (edit, MOO_EDIT_CLEAN, clean);
    _moo_edit_status_changed (edit);
}

/**
 * moo_edit_get_clean:
 **/
gboolean
moo_edit_get_clean (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    return (edit->priv->status & MOO_EDIT_CLEAN) ? TRUE : FALSE;
}


void
_moo_edit_status_changed (MooEdit *edit)
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
        _moo_edit_status_changed (edit);
    }
}


/**
 * moo_edit_is_empty:
 *
 * This function returns whether the document is "empty", i.e. is not modified,
 * is untitled, and contains no text.
 **/
gboolean
moo_edit_is_empty (MooEdit *edit)
{
    GtkTextIter start, end;

    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);

    if (MOO_EDIT_IS_BUSY (edit) || MOO_EDIT_IS_MODIFIED (edit) || !MOO_EDIT_IS_UNTITLED (edit))
        return FALSE;

    gtk_text_buffer_get_bounds (moo_edit_get_buffer (edit), &start, &end);

    return !gtk_text_iter_compare (&start, &end);
}

/**
 * moo_edit_is_untitled:
 **/
gboolean
moo_edit_is_untitled (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    return MOO_EDIT_IS_UNTITLED (edit);
}

/**
 * moo_edit_is_modified:
 **/
gboolean
moo_edit_is_modified (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    return MOO_EDIT_IS_MODIFIED (edit);
}

/**
 * moo_edit_get_status:
 **/
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

        case PROP_LANG:
            g_value_set_object (value, moo_edit_get_lang (edit));
            break;

        case PROP_LINE_END_TYPE:
            g_value_set_enum (value, edit->priv->line_end_type);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


/**
 * moo_edit_get_file:
 *
 * Returns: (transfer full)
 **/
GFile *
moo_edit_get_file (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    return edit->priv->file ? g_file_dup (edit->priv->file) : NULL;
}

/**
 * moo_edit_get_filename:
 **/
char *
moo_edit_get_filename (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    return g_strdup (edit->priv->filename);
}

char *
_moo_edit_get_normalized_name (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    return g_strdup (edit->priv->norm_name);
}

char *
_moo_edit_get_utf8_filename (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    return edit->priv->filename ? g_strdup (edit->priv->display_filename) : NULL;
}

/**
 * moo_edit_get_display_name:
 **/
const char *
moo_edit_get_display_name (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    return edit->priv->display_filename;
}

/**
 * moo_edit_get_display_basename:
 **/
const char *
moo_edit_get_display_basename (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    return edit->priv->display_basename;
}

/**
 * moo_edit_get_uri:
 **/
char *
moo_edit_get_uri (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    return edit->priv->file ? g_file_get_uri (edit->priv->file) : NULL;
}

/**
 * moo_edit_get_encoding:
 **/
const char *
moo_edit_get_encoding (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    return edit->priv->encoding;
}

/**
 * moo_edit_set_encoding:
 **/
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


/**
 * moo_edit_get_editor:
 **/
MooEditor *
moo_edit_get_editor (MooEdit *doc)
{
    g_return_val_if_fail (MOO_IS_EDIT (doc), NULL);
    return doc->priv->editor;
}

/**
 * moo_edit_get_view:
 **/
MooEditView *
moo_edit_get_view (MooEdit *doc)
{
    g_return_val_if_fail (MOO_IS_EDIT (doc), NULL);
    return doc->priv->view;
}

/**
 * moo_edit_get_window:
 **/
MooEditWindow *
moo_edit_get_window (MooEdit *doc)
{
    g_return_val_if_fail (MOO_IS_EDIT (doc), NULL);
    return moo_edit_view_get_window (doc->priv->view);
}

/**
 * moo_edit_get_buffer:
 **/
GtkTextBuffer *
moo_edit_get_buffer (MooEdit *doc)
{
    g_return_val_if_fail (MOO_IS_EDIT (doc), NULL);
    return gtk_text_view_get_buffer (GTK_TEXT_VIEW (doc->priv->view));
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
update_config_from_mode_lines (MooEdit *edit)
{
    GtkTextBuffer *buffer = moo_edit_get_buffer (edit);
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


static MooLang *
moo_edit_get_lang (MooEdit *doc)
{
    g_return_val_if_fail (MOO_IS_EDIT (doc), NULL);
    moo_assert (!doc->priv->in_recheck_config);
    return moo_text_view_get_lang (MOO_TEXT_VIEW (moo_edit_get_view (doc)));
}

/**
 * moo_edit_get_lang_id:
 *
 * Returns: id of language currently used in the document. If no language
 * is used, then string "none" is returned.
 */
char *
moo_edit_get_lang_id (MooEdit *doc)
{
    MooLang *lang = NULL;

    moo_return_val_if_fail (MOO_IS_EDIT (doc), NULL);

    if (!doc->priv->in_recheck_config)
    {
        lang = moo_edit_get_lang (doc);
    }
    else
    {
        const char *lang_id = moo_edit_config_get_string (doc->config, "lang");
        lang = lang_id ? _moo_lang_mgr_find_lang (moo_lang_mgr_default (), lang_id) : NULL;
    }

    return g_strdup (_moo_lang_id (lang));
}

static gboolean
moo_edit_apply_config_all_in_idle (void)
{
    MooEditList *l;

    moo_edit_apply_config_all_idle = 0;

    for (l = _moo_edit_instances; l != NULL; l = l->next)
        moo_edit_recheck_config (l->data);

    return FALSE;
}

void
_moo_edit_queue_recheck_config_all (void)
{
    if (!moo_edit_apply_config_all_idle)
        moo_edit_apply_config_all_idle =
            moo_idle_add ((GSourceFunc) moo_edit_apply_config_all_in_idle, NULL);
}


static void
update_lang_config_from_lang_globs (MooEdit *doc)
{
    const char *lang_id = NULL;

    if (doc->priv->file)
    {
        MooLangMgr *mgr = moo_lang_mgr_default ();
        MooLang *lang = moo_lang_mgr_get_lang_for_file (mgr, doc->priv->file);
        lang_id = lang ? _moo_lang_id (lang) : NULL;
    }

    moo_edit_config_set (doc->config, MOO_EDIT_CONFIG_SOURCE_FILENAME,
                         "lang", lang_id, (char*) NULL);
}

static void
update_config_from_filter_settings (MooEdit *doc)
{
    char *filter_config = NULL;

    filter_config = _moo_edit_filter_settings_get_for_doc (doc);

    if (filter_config)
        moo_edit_config_parse (doc->config, filter_config,
                               MOO_EDIT_CONFIG_SOURCE_FILENAME);

    g_free (filter_config);
}

static void
update_config_from_lang (MooEdit *doc)
{
    char *lang_id = moo_edit_get_lang_id (doc);
    _moo_lang_mgr_update_config (moo_lang_mgr_default (), doc->config, lang_id);
    g_free (lang_id);
}

static void
moo_edit_apply_config (MooEdit *doc)
{
    const char *lang_id = moo_edit_config_get_string (doc->config, "lang");
    MooLangMgr *mgr = moo_lang_mgr_default ();
    MooLang *lang = lang_id ? _moo_lang_mgr_find_lang (mgr, lang_id) : NULL;
    moo_text_view_set_lang (MOO_TEXT_VIEW (moo_edit_get_view (doc)), lang);
    g_object_notify (G_OBJECT (doc), "has-comments");
    g_object_notify (G_OBJECT (doc), "lang");
    _moo_edit_view_apply_config (moo_edit_get_view (doc));
}

static void
moo_edit_recheck_config (MooEdit *doc)
{
    moo_return_if_fail (!doc->priv->in_recheck_config);

    if (doc->priv->apply_config_idle)
    {
        g_source_remove (doc->priv->apply_config_idle);
        doc->priv->apply_config_idle = 0;
    }

    g_object_freeze_notify (G_OBJECT (doc));
    g_object_freeze_notify (G_OBJECT (moo_edit_get_view (doc)));
    doc->priv->in_recheck_config = TRUE;

    // this resets settings from global config
    moo_edit_config_unset_by_source (doc->config, MOO_EDIT_CONFIG_SOURCE_FILE);

    // First global settings
    _moo_edit_apply_prefs (doc);

    // then language from globs
    update_lang_config_from_lang_globs (doc);

    // then settings from mode lines, these may change lang
    update_config_from_mode_lines (doc);

    // then filter settings, these also may change lang
    update_config_from_filter_settings (doc);

    // update config for lang
    update_config_from_lang (doc);

    // finally apply config
    moo_edit_apply_config (doc);

    doc->priv->in_recheck_config = FALSE;

    g_object_thaw_notify (G_OBJECT (moo_edit_get_view (doc)));
    g_object_thaw_notify (G_OBJECT (doc));
}

static gboolean
moo_edit_recheck_config_in_idle (MooEdit *edit)
{
    edit->priv->apply_config_idle = 0;
    moo_edit_recheck_config (edit);
    return FALSE;
}

void
_moo_edit_queue_recheck_config (MooEdit *doc)
{
    moo_return_if_fail (!doc->priv->in_recheck_config);
    if (!doc->priv->apply_config_idle)
        doc->priv->apply_config_idle =
                moo_idle_add_full (G_PRIORITY_HIGH,
                                   (GSourceFunc) moo_edit_recheck_config_in_idle,
                                   doc, NULL);
}

static void
config_changed (MooEdit *doc)
{
    if (!doc->priv->in_recheck_config)
        _moo_edit_queue_recheck_config (doc);
}


/**
 * moo_edit_reload:
 *
 * @edit:
 * @info: (allow-none) (default NULL):
 * @error:
 *
 * Reload document from disk
 *
 * Returns: whether document was successfully reloaded
 **/
gboolean
moo_edit_reload (MooEdit            *doc,
                 MooEditReloadInfo  *info,
                 GError            **error)
{
    moo_return_error_if_fail (MOO_IS_EDIT (doc));
    return moo_editor_reload (doc->priv->editor, doc, info, error);
}

/**
 * moo_edit_close:
 *
 * @edit:
 * @ask_confirm: (default TRUE): whether user should be asked if he wants
 * to save changes before closing if the document is modified.
 *
 * Returns: whether document was closed
 **/
gboolean
moo_edit_close (MooEdit        *edit,
                gboolean        ask_confirm)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    return moo_editor_close_doc (edit->priv->editor, edit, ask_confirm);
}

static MooEditSaveResponse
moo_edit_before_save (G_GNUC_UNUSED MooEdit *doc,
                      G_GNUC_UNUSED GFile *file)
{
    return MOO_EDIT_SAVE_RESPONSE_CONTINUE;
}

/**
 * moo_edit_save:
 **/
gboolean
moo_edit_save (MooEdit *doc,
               GError **error)
{
    moo_return_error_if_fail (MOO_IS_EDIT (doc));
    return moo_editor_save (doc->priv->editor, doc, error);
}

/**
 * moo_edit_save_as:
 **/
gboolean
moo_edit_save_as (MooEdit          *doc,
                  MooEditSaveInfo  *info,
                  GError          **error)
{
    moo_return_error_if_fail (MOO_IS_EDIT (doc));
    return moo_editor_save_as (doc->priv->editor, doc, info, error);
}

/**
 * moo_edit_save_copy:
 **/
gboolean
moo_edit_save_copy (MooEdit          *doc,
                    MooEditSaveInfo  *info,
                    GError          **error)
{
    moo_return_error_if_fail (MOO_IS_EDIT (doc));
    return moo_editor_save_copy (doc->priv->editor, doc, info, error);
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

    lang = moo_edit_get_lang (edit);

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
_moo_edit_comment (MooEdit *edit)
{
    MooLang *lang;
    GtkTextIter start, end;
    GtkTextBuffer *buffer;
    gboolean has_selection, single_line, multi_line;
    gboolean adjust_selection = FALSE, move_insert = FALSE;
    int sel_start_line = 0, sel_start_offset = 0;

    g_return_if_fail (MOO_IS_EDIT (edit));

    lang = moo_edit_get_lang (edit);

    if (!_moo_edit_has_comments (edit, &single_line, &multi_line))
        return;

    buffer = moo_edit_get_buffer (edit);
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
_moo_edit_uncomment (MooEdit *edit)
{
    MooLang *lang;
    GtkTextIter start, end;
    GtkTextBuffer *buffer;
    gboolean single_line, multi_line;

    g_return_if_fail (MOO_IS_EDIT (edit));

    lang = moo_edit_get_lang (edit);

    if (!_moo_edit_has_comments (edit, &single_line, &multi_line))
        return;

    buffer = moo_edit_get_buffer (edit);
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

    buffer = moo_edit_get_buffer (edit);
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

    buffer = moo_edit_get_buffer (doc);
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
