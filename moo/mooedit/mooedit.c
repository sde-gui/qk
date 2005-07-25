/*
 *   mooedit/mooedit.c
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

#include <gdk/gdkkeysyms.h>
#include "mooedit/mooedit-private.h"
#include "mooedit/mooeditdialogs.h"
#include "mooedit/mooeditprefs.h"
#include "mooedit/mooeditlangmgr.h"
#include "mooutils/moomarshals.h"
#include "mooutils/moocompat.h"
#include "mooutils/moofileutils.h"


MooEditSearchParams *_moo_edit_search_params;
static MooEditSearchParams search_params = {-1, NULL, NULL, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};


static void moo_edit_class_init     (MooEditClass   *klass);

static GObject *moo_edit_constructor(GType                  type,
                                     guint                  n_construct_properties,
                                     GObjectConstructParam *construct_param);
static void moo_edit_init           (MooEdit        *edit);
static void moo_edit_finalize       (GObject        *object);

static void moo_edit_set_property   (GObject        *object,
                                     guint           prop_id,
                                     const GValue   *value,
                                     GParamSpec     *pspec);
static void moo_edit_get_property   (GObject        *object,
                                     guint           prop_id,
                                     GValue         *value,
                                     GParamSpec     *pspec);

static void open_interactive            (MooEdit            *edit);
static void save_as_interactive         (MooEdit            *edit);
static void reload_interactive          (MooEdit            *edit);
static void goto_line                   (MooEdit            *edit);

static void can_redo_cb                 (GtkSourceBuffer    *buffer,
                                         gboolean            arg,
                                         MooEdit            *edit);
static void can_undo_cb                 (GtkSourceBuffer    *buffer,
                                         gboolean            arg,
                                         MooEdit            *edit);
static void modified_changed_cb         (GtkTextBuffer      *buffer,
                                         MooEdit            *edit);


enum {
    OPEN,
    OPEN_INTERACTIVE,
    SAVE,
    SAVE_AS,
    SAVE_AS_INTERACTIVE,
    CLOSE,
    LOAD,
    RELOAD,
    RELOAD_INTERACTIVE,
    WRITE,
    DOC_STATUS_CHANGED,
    FILENAME_CHANGED,
    LANG_CHANGED,
    DELETE_SELECTION,
    CAN_UNDO,
    CAN_REDO,
    FIND,
    FIND_NEXT,
    FIND_PREVIOUS,
    REPLACE,
    GOTO_LINE,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];


enum {
    PROP_0,
    PROP_EDITOR
};


static GSList *moo_edit_instances = NULL;


/* MOO_TYPE_EDIT */
G_DEFINE_TYPE (MooEdit, moo_edit, GTK_TYPE_SOURCE_VIEW)


static void moo_edit_class_init (MooEditClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
    GtkTextViewClass *text_view_class = GTK_TEXT_VIEW_CLASS (klass);

    _moo_edit_search_params = &search_params;

    gobject_class->set_property = moo_edit_set_property;
    gobject_class->get_property = moo_edit_get_property;
    gobject_class->constructor = moo_edit_constructor;
    gobject_class->finalize = moo_edit_finalize;

    widget_class->key_press_event = _moo_edit_key_press_event;
//     widget_class->key_release_event = _moo_edit_key_release_event;
    widget_class->button_press_event = _moo_edit_button_press_event;
    widget_class->button_release_event = _moo_edit_button_release_event;
    widget_class->motion_notify_event = _moo_edit_motion_event;

    text_view_class->move_cursor = _moo_edit_move_cursor;
    text_view_class->delete_from_cursor = _moo_edit_delete_from_cursor;

    klass->open = _moo_edit_open;
    klass->save = _moo_edit_save;
    klass->save_as = _moo_edit_save_as;
    klass->close = _moo_edit_close;
    klass->open_interactive = open_interactive;
    klass->save_as_interactive = save_as_interactive;
    klass->reload_interactive = reload_interactive;
    klass->load = _moo_edit_load;
    klass->reload = _moo_edit_reload;
    klass->write = _moo_edit_write;
    klass->doc_status_changed = NULL;
    klass->filename_changed = NULL;
    klass->lang_changed = NULL;

    klass->delete_selection = moo_edit_delete_selection;
    klass->can_redo = NULL;
    klass->can_undo = NULL;
    klass->extend_selection = _moo_edit_extend_selection;
    klass->find = _moo_edit_find;
    klass->find_next = _moo_edit_find_next;
    klass->find_previous = _moo_edit_find_previous;
    klass->replace = _moo_edit_replace;
    klass->goto_line = goto_line;

    g_object_class_install_property (gobject_class,
                                     PROP_EDITOR,
                                     g_param_spec_object ("editor",
                                             "editor",
                                             "editor",
                                             MOO_TYPE_EDITOR,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

    signals[OPEN] =
            g_signal_new ("open",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                          G_STRUCT_OFFSET (MooEditClass, open),
                          NULL, NULL,
                          _moo_marshal_BOOLEAN__STRING_STRING,
                          G_TYPE_BOOLEAN, 2,
                          G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE,
                          G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE);

    signals[OPEN_INTERACTIVE] =
            g_signal_new ("open-interactive",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                          G_STRUCT_OFFSET (MooEditClass, open_interactive),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[SAVE] =
        g_signal_new ("save",
                      G_OBJECT_CLASS_TYPE (klass),
                      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                      G_STRUCT_OFFSET (MooEditClass, save),
                      NULL, NULL,
                      _moo_marshal_BOOLEAN__VOID,
                      G_TYPE_BOOLEAN, 0);

    signals[SAVE_AS] =
        g_signal_new ("save_as",
                      G_OBJECT_CLASS_TYPE (klass),
                      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                      G_STRUCT_OFFSET (MooEditClass, save_as),
                      NULL, NULL,
                      _moo_marshal_BOOLEAN__STRING_STRING,
                      G_TYPE_BOOLEAN, 2,
                      G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE,
                      G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE);

    signals[SAVE_AS_INTERACTIVE] =
            g_signal_new ("save-as-interactive",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                          G_STRUCT_OFFSET (MooEditClass, save_as_interactive),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[CLOSE] =
        g_signal_new ("close",
                      G_OBJECT_CLASS_TYPE (klass),
                      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                      G_STRUCT_OFFSET (MooEditClass, close),
                      NULL, NULL,
                      _moo_marshal_BOOLEAN__VOID,
                      G_TYPE_BOOLEAN, 0);

    signals[LOAD] =
        g_signal_new ("load",
                      G_OBJECT_CLASS_TYPE (klass),
                      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                      G_STRUCT_OFFSET (MooEditClass, load),
                      NULL, NULL,
                      _moo_marshal_BOOLEAN__STRING_STRING_POINTER,
                      G_TYPE_BOOLEAN, 3,
                      G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE,
                      G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE,
                      G_TYPE_POINTER);

    signals[RELOAD] =
            g_signal_new ("reload",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                          G_STRUCT_OFFSET (MooEditClass, reload),
                          NULL, NULL,
                          _moo_marshal_BOOLEAN__POINTER,
                          G_TYPE_BOOLEAN, 1, G_TYPE_POINTER);

    signals[RELOAD_INTERACTIVE] =
            g_signal_new ("reload-interactive",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                          G_STRUCT_OFFSET (MooEditClass, reload_interactive),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[WRITE] =
        g_signal_new ("write",
                      G_OBJECT_CLASS_TYPE (klass),
                      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                      G_STRUCT_OFFSET (MooEditClass, write),
                      NULL, NULL,
                      _moo_marshal_BOOLEAN__STRING_STRING_POINTER,
                      G_TYPE_BOOLEAN, 3,
                      G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE,
                      G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE,
                      G_TYPE_POINTER);

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

    signals[DELETE_SELECTION] =
        g_signal_new ("delete-selection",
                      G_OBJECT_CLASS_TYPE (klass),
                      (GSignalFlags) (G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION),
                      G_STRUCT_OFFSET (MooEditClass, delete_selection),
                      NULL, NULL,
                      _moo_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    signals[LANG_CHANGED] =
            g_signal_new ("lang-changed",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooEditClass, lang_changed),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[CAN_UNDO] =
        g_signal_new ("can-undo",
                      G_OBJECT_CLASS_TYPE (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (MooEditClass, can_undo),
                      NULL, NULL,
                      _moo_marshal_VOID__BOOLEAN,
                      G_TYPE_NONE, 1, G_TYPE_BOOLEAN);

    signals[CAN_REDO] =
        g_signal_new ("can-redo",
                      G_OBJECT_CLASS_TYPE (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (MooEditClass, can_redo),
                      NULL, NULL,
                      _moo_marshal_VOID__BOOLEAN,
                      G_TYPE_NONE, 1, G_TYPE_BOOLEAN);

    signals[FIND] =
        g_signal_new ("find",
                      G_OBJECT_CLASS_TYPE (klass),
                      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                      G_STRUCT_OFFSET (MooEditClass, find),
                      NULL, NULL,
                      _moo_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    signals[FIND_NEXT] =
        g_signal_new ("find-next",
                      G_OBJECT_CLASS_TYPE (klass),
                      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                      G_STRUCT_OFFSET (MooEditClass, find_next),
                      NULL, NULL,
                      _moo_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    signals[FIND_PREVIOUS] =
        g_signal_new ("find-previous",
                      G_OBJECT_CLASS_TYPE (klass),
                      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                      G_STRUCT_OFFSET (MooEditClass, find_previous),
                      NULL, NULL,
                      _moo_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    signals[REPLACE] =
        g_signal_new ("replace",
                      G_OBJECT_CLASS_TYPE (klass),
                      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                      G_STRUCT_OFFSET (MooEditClass, replace),
                      NULL, NULL,
                      _moo_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    signals[GOTO_LINE] =
        g_signal_new ("goto-line",
                      G_OBJECT_CLASS_TYPE (klass),
                      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                      G_STRUCT_OFFSET (MooEditClass, goto_line),
                      NULL, NULL,
                      _moo_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    _moo_edit_set_default_settings ();
}


static void moo_edit_init (MooEdit *edit)
{
    edit->priv = moo_edit_private_new ();
}


static GObject *moo_edit_constructor (GType                  type,
                                      guint                  n_construct_properties,
                                      GObjectConstructParam *construct_param)
{
    GObject *object;
    MooEdit *edit;
    GtkSourceBuffer *buffer;

    object = G_OBJECT_CLASS (moo_edit_parent_class)->constructor (
        type, n_construct_properties, construct_param);

    buffer = gtk_source_buffer_new (NULL);
    gtk_text_view_set_buffer (GTK_TEXT_VIEW (object), GTK_TEXT_BUFFER (buffer));
    g_object_unref (G_OBJECT (buffer));

    edit = MOO_EDIT (object);

    edit->priv->constructed = TRUE;
    edit->priv->text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (edit));
    edit->priv->source_buffer = GTK_SOURCE_BUFFER (edit->priv->text_buffer);

    moo_edit_instances = g_slist_prepend (moo_edit_instances, edit);

    edit->priv->can_undo_handler_id =
            g_signal_connect (edit->priv->source_buffer,
                              "can-undo",
                              G_CALLBACK (can_undo_cb),
                              edit);
    edit->priv->can_redo_handler_id =
            g_signal_connect (edit->priv->source_buffer,
                              "can-redo",
                              G_CALLBACK (can_redo_cb),
                              edit);
    edit->priv->modified_changed_handler_id =
            g_signal_connect (edit->priv->text_buffer,
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


static void moo_edit_finalize       (GObject      *object)
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

    moo_edit_instances = g_slist_remove (moo_edit_instances, edit);

    G_OBJECT_CLASS (moo_edit_parent_class)->finalize (object);
}


MooEdit        *_moo_edit_new               (MooEditor  *editor)
{
    MooEdit *edit = MOO_EDIT (g_object_new (MOO_TYPE_EDIT,
                                            "editor", editor,
                                            NULL));
    return edit;
}


GtkWidget   *moo_edit_new                   (void)
{
    return GTK_WIDGET (g_object_new (MOO_TYPE_EDIT, NULL));
}


void moo_edit_delete_selection   (MooEdit    *edit)
{
    g_return_if_fail (MOO_IS_EDIT (edit));
    gtk_text_buffer_delete_selection (edit->priv->text_buffer, TRUE, TRUE);
}


gboolean    moo_edit_open                   (MooEdit            *edit,
                                             const char         *file,
                                             const char         *encoding)
{
    gboolean result;
    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    if (file && !file[0]) file = NULL;
    g_signal_emit (edit, signals[OPEN], 0, file, encoding, &result);
    return result;
}

static void open_interactive            (MooEdit            *edit)
{
    gboolean result;
    g_signal_emit (edit, signals[OPEN], 0, NULL, NULL, &result);
}

static void save_as_interactive         (MooEdit            *edit)
{
    gboolean result;
    g_signal_emit (edit, signals[SAVE_AS], 0, NULL, NULL, &result);
}

static void reload_interactive          (MooEdit            *edit)
{
    gboolean result;
    if (edit->priv->filename)
        g_signal_emit (edit, signals[RELOAD], 0, NULL, &result);
}

gboolean    moo_edit_save_as                (MooEdit            *edit,
                                             const char         *file,
                                             const char         *encoding)
{
    gboolean result;
    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    if (file && !file[0]) file = NULL;
    g_signal_emit (edit, signals[SAVE_AS], 0, file, encoding, &result);
    return result;
}

gboolean    moo_edit_save                   (MooEdit            *edit)
{
    gboolean result;
    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    g_signal_emit (edit, signals[SAVE], 0, &result);
    return result;
}

gboolean    moo_edit_close                  (MooEdit            *edit)
{
    gboolean result;
    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    g_signal_emit (edit, signals[CLOSE], 0, &result);
    return result;
}


gboolean    moo_edit_load                   (MooEdit            *edit,
                                             const char         *file,
                                             const char         *encoding,
                                             GError            **error)
{
    gboolean result;
    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    if (file && !file[0]) file = NULL;
    g_signal_emit (edit, signals[LOAD], 0, file, encoding, error, &result);
    return result;
}

gboolean    moo_edit_reload                 (MooEdit            *edit,
                                             GError            **error)
{
    gboolean result;
    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    g_signal_emit (edit, signals[RELOAD], 0, error, &result);
    return result;
}

gboolean    moo_edit_write                  (MooEdit            *edit,
                                             const char         *file,
                                             const char         *encoding,
                                             GError            **error)
{
    gboolean result;
    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    if (file && !file[0]) file = NULL;
    g_signal_emit (edit, signals[WRITE], 0, file, encoding, error, &result);
    return result;
}


static void can_redo_cb                 (G_GNUC_UNUSED GtkSourceBuffer    *buffer,
                                         gboolean            arg,
                                         MooEdit            *edit)
{
    g_signal_emit (edit, signals[CAN_REDO], 0, arg, NULL);
}

static void can_undo_cb                 (G_GNUC_UNUSED GtkSourceBuffer    *buffer,
                                         gboolean            arg,
                                         MooEdit            *edit)
{
    g_signal_emit (edit, signals[CAN_UNDO], 0, arg, NULL);
}

static void modified_changed_cb         (GtkTextBuffer      *buffer,
                                         MooEdit            *edit)
{
    moo_edit_set_modified (edit, gtk_text_buffer_get_modified (buffer));
}


gboolean    moo_edit_get_modified           (MooEdit            *edit)
{
    return edit->priv->status & MOO_EDIT_DOC_MODIFIED;
}


void        moo_edit_set_modified           (MooEdit            *edit,
                                             gboolean            modified)
{
    gboolean buf_modified;

    g_return_if_fail (MOO_IS_EDIT (edit));

    buf_modified =
            gtk_text_buffer_get_modified (edit->priv->text_buffer);

    if (buf_modified != modified)
    {
        g_signal_handler_block (edit->priv->text_buffer,
                                edit->priv->modified_changed_handler_id);
        gtk_text_buffer_set_modified (edit->priv->text_buffer, modified);
        g_signal_handler_unblock (edit->priv->text_buffer,
                                  edit->priv->modified_changed_handler_id);
    }

    if (modified)
        edit->priv->status |= MOO_EDIT_DOC_MODIFIED;
    else
        edit->priv->status &= ~MOO_EDIT_DOC_MODIFIED;

    moo_edit_doc_status_changed (edit);
}


gboolean    moo_edit_get_clean              (MooEdit            *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    return edit->priv->status & MOO_EDIT_DOC_CLEAN;
}

void        moo_edit_set_clean              (MooEdit            *edit,
                                             gboolean            clean)
{
    g_return_if_fail (MOO_IS_EDIT (edit));
    if (clean)
        edit->priv->status |= MOO_EDIT_DOC_CLEAN;
    else
        edit->priv->status &= ~MOO_EDIT_DOC_CLEAN;
    moo_edit_doc_status_changed (edit);
}


void        moo_edit_doc_status_changed     (MooEdit            *edit)
{
    g_return_if_fail (MOO_IS_EDIT (edit));
    g_signal_emit (edit, signals[DOC_STATUS_CHANGED], 0, NULL);
}


MooEditPrivate *moo_edit_private_new (void)
{
    MooEditPrivate *priv = g_new0 (MooEditPrivate, 1);

//     priv->status = MOO_EDIT_DOC_NEW;
    priv->drag_button = GDK_BUTTON_RELEASE;
    priv->drag_type = MOO_EDIT_DRAG_NONE;
    priv->drag_start_x = -1;
    priv->drag_start_y = -1;

//     priv->timestamp = MOO_MTIME_EINVAL;
    priv->file_watch_timeout = 5000;
    priv->file_watch_policy = MOO_EDIT_ALWAYS_ALERT;

#if defined(__WIN32__)
    priv->line_end_type = MOO_EDIT_LINE_END_WIN32;
#elif defined(OS_DARWIN)
    priv->line_end_type = MOO_EDIT_LINE_END_MAC;
#else
    priv->line_end_type = MOO_EDIT_LINE_END_UNIX;
#endif

    priv->last_search_stamp = -1;

    priv->tab_indents = TRUE;
    priv->shift_tab_unindents = TRUE;
    priv->backspace_indents = TRUE;
    priv->auto_indent = TRUE;
    priv->ctrl_up_down_scrolls = TRUE;
    priv->ctrl_page_up_down_scrolls = TRUE;

    return priv;
}


MooEditFileInfo *moo_edit_file_info_new     (const char         *filename,
                                             const char         *encoding)
{
    MooEditFileInfo *info = g_new (MooEditFileInfo, 1);
    info->filename = g_strdup (filename);
    info->encoding = g_strdup (encoding);
    return info;
}

MooEditFileInfo *moo_edit_file_info_copy    (MooEditFileInfo    *info)
{
    MooEditFileInfo *copy;
    g_return_val_if_fail (info != NULL, NULL);
    copy = g_new (MooEditFileInfo, 1);
    copy->filename = g_strdup (info->filename);
    copy->encoding = g_strdup (info->encoding);
    return copy;
}

void        moo_edit_file_info_free         (MooEditFileInfo    *info)
{
    if (!info) return;
    g_free (info->filename);
    g_free (info->encoding);
    g_free (info);
}


const char *moo_edit_get_filename           (MooEdit            *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    return edit->priv->filename;
}

const char *moo_edit_get_basename           (MooEdit            *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    return edit->priv->basename;
}

const char *moo_edit_get_display_filename   (MooEdit            *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    return edit->priv->display_filename;
}

const char *moo_edit_get_display_basename   (MooEdit            *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    return edit->priv->display_basename;
}


void        moo_edit_find                   (MooEdit            *edit)
{
    g_return_if_fail (MOO_IS_EDIT (edit));
    g_signal_emit (edit, signals[FIND], 0, NULL);
}

void        moo_edit_replace                (MooEdit            *edit)
{
    g_return_if_fail (MOO_IS_EDIT (edit));
    g_signal_emit (edit, signals[REPLACE], 0, NULL);
}

void        moo_edit_find_next              (MooEdit            *edit)
{
    g_return_if_fail (MOO_IS_EDIT (edit));
    g_signal_emit (edit, signals[FIND_NEXT], 0, NULL);
}

void        moo_edit_find_previous          (MooEdit            *edit)
{
    g_return_if_fail (MOO_IS_EDIT (edit));
    g_signal_emit (edit, signals[FIND_PREVIOUS], 0, NULL);
}

static void goto_line                       (MooEdit            *edit)
{
    moo_edit_goto_line (edit, -1);
}


void        moo_edit_set_lang               (MooEdit            *edit,
                                             MooEditLang        *lang)
{
    GtkTextTagTable *table;

    g_return_if_fail (MOO_IS_EDIT (edit));

    if (lang == edit->priv->lang) return;

    table = gtk_text_buffer_get_tag_table (edit->priv->text_buffer);
    gtk_source_tag_table_remove_source_tags (GTK_SOURCE_TAG_TABLE (table));

    if (edit->priv->lang) {
        g_object_unref (edit->priv->lang);
        edit->priv->lang = NULL;
    }

    if (lang) {
        GSList *tags;
        gunichar escape_char;

        edit->priv->lang = lang;
        g_object_ref (edit->priv->lang);

        tags = moo_edit_lang_get_tags (lang);
        gtk_source_tag_table_add_tags (GTK_SOURCE_TAG_TABLE (table), tags);
        g_slist_foreach (tags, (GFunc) g_object_unref, NULL);
        g_slist_free (tags);

        escape_char = moo_edit_lang_get_escape_char (lang);
        gtk_source_buffer_set_escape_char (edit->priv->source_buffer,
                                           escape_char);

        gtk_source_buffer_set_brackets (edit->priv->source_buffer,
                                        moo_edit_lang_get_brackets (lang));
    }
    else {
        gtk_source_buffer_set_escape_char (edit->priv->source_buffer, '\\');
        gtk_source_buffer_set_brackets (edit->priv->source_buffer, "{}()[]");
    }

    g_signal_emit (edit, signals[LANG_CHANGED], 0, NULL);
}


void        moo_edit_set_highlight          (MooEdit            *edit,
                                             gboolean            highlight)
{
    g_return_if_fail (MOO_IS_EDIT (edit));
    gtk_source_buffer_set_highlight (edit->priv->source_buffer, highlight);
}


void        moo_edit_set_font_from_string   (MooEdit            *edit,
                                             const char         *font)
{
    PangoFontDescription *font_desc = NULL;

    g_return_if_fail (MOO_IS_EDIT (edit));

    if (font) font_desc = pango_font_description_from_string (font);
    gtk_widget_modify_font (GTK_WIDGET (edit), font_desc);
    if (font_desc) pango_font_description_free (font_desc);
}


gboolean    moo_edit_get_read_only          (MooEdit            *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), TRUE);
    return edit->priv->readonly;
}


void        moo_edit_set_read_only          (MooEdit            *edit,
                                             gboolean            readonly)
{
    g_return_if_fail (MOO_IS_EDIT (edit));
    edit->priv->readonly = readonly;
    gtk_text_view_set_editable (GTK_TEXT_VIEW (edit), !readonly);
    moo_edit_doc_status_changed (edit);
}


gboolean    moo_edit_is_empty               (MooEdit            *edit)
{
    GtkTextIter start, end;

    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);

    if (moo_edit_get_modified (edit) || edit->priv->filename)
        return FALSE;

    gtk_text_buffer_get_bounds (edit->priv->text_buffer, &start, &end);
    return !gtk_text_iter_compare (&start, &end);
}


gboolean    moo_edit_can_redo               (MooEdit            *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    return gtk_source_buffer_can_redo (edit->priv->source_buffer);
}


gboolean    moo_edit_can_undo               (MooEdit            *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    return gtk_source_buffer_can_undo (edit->priv->source_buffer);
}


void        moo_edit_select_all             (MooEdit            *edit)
{
    g_return_if_fail (MOO_IS_EDIT (edit));
    g_signal_emit_by_name (edit, "select-all", TRUE, NULL);
}


MooEditDocStatus moo_edit_get_doc_status    (MooEdit            *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), 0);
    return edit->priv->status;
}


MooEditLangMgr *_moo_edit_get_lang_mgr      (MooEdit    *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    if (edit->priv->editor)
        return moo_editor_get_lang_mgr (edit->priv->editor);
    else
        return NULL;
}


static void     moo_edit_set_property       (GObject        *object,
                                             guint           prop_id,
                                             const GValue   *value,
                                             GParamSpec     *pspec)
{
    MooEdit *edit = MOO_EDIT (object);
    switch (prop_id) {
        case PROP_EDITOR:
            edit->priv->editor = MOO_EDITOR (g_value_get_object (value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void     moo_edit_get_property       (GObject        *object,
                                             guint           prop_id,
                                             GValue         *value,
                                             GParamSpec     *pspec)
{
    MooEdit *edit = MOO_EDIT (object);
    switch (prop_id) {
        case PROP_EDITOR:
            g_value_set_object (value, edit->priv->editor);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


MooEditLang*moo_edit_get_lang               (MooEdit            *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    return edit->priv->lang;
}


GType       moo_edit_doc_status_get_type            (void)
{
    static GType type = 0;

    if (!type)
    {
        static const GFlagsValue values[] = {
            { MOO_EDIT_DOC_MODIFIED_ON_DISK, (char*)"MOO_EDIT_DOC_MODIFIED_ON_DISK", (char*)"modified-on-disk" },
            { MOO_EDIT_DOC_DELETED, (char*)"MOO_EDIT_DOC_DELETED", (char*)"deleted" },
            { MOO_EDIT_DOC_CHANGED_ON_DISK, (char*)"MOO_EDIT_DOC_CHANGED_ON_DISK", (char*)"changed-on-disk" },
            { MOO_EDIT_DOC_MODIFIED, (char*)"MOO_EDIT_DOC_MODIFIED", (char*)"modified" },
            { MOO_EDIT_DOC_CLEAN, (char*)"MOO_EDIT_DOC_CLEAN", (char*)"clean" },
            { 0, NULL, NULL }
        };
        type = g_flags_register_static ("MooEditDocStatus", values);
    }

    return type;
}

GType       moo_edit_selection_type_get_type        (void)
{
    static GType type = 0;

    if (!type)
    {
        static const GEnumValue values[] = {
            { MOO_EDIT_SELECT_CHARS, (char*)"MOO_EDIT_SELECT_CHARS", (char*)"select-chars" },
            { MOO_EDIT_SELECT_WORDS, (char*)"MOO_EDIT_SELECT_WORDS", (char*)"select-words" },
            { MOO_EDIT_SELECT_LINES, (char*)"MOO_EDIT_SELECT_LINES", (char*)"select-lines" },
            { 0, NULL, NULL }
        };
        type = g_enum_register_static ("MooEditSelectionType", values);
    }

    return type;
}

GType       moo_edit_on_external_changes_get_type   (void)
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

GType       moo_edit_file_info_get_type             (void)
{
    static GType type = 0;
    if (!type)
        type = g_boxed_type_register_static ("MooEditFileInfo",
                                             (GBoxedCopyFunc) moo_edit_file_info_copy,
                                             (GBoxedFreeFunc)moo_edit_file_info_free);
    return type;
}


char       *moo_edit_get_selection          (MooEdit            *edit)
{
    GtkTextBuffer *buf;
    GtkTextIter start, end;

    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);

    buf = gtk_text_view_get_buffer (GTK_TEXT_VIEW (edit));

    if (gtk_text_buffer_get_selection_bounds (buf, &start, &end))
        return gtk_text_buffer_get_text (buf, &start, &end, TRUE);
    else
        return NULL;
}


char       *moo_edit_get_text               (MooEdit            *edit)
{
    GtkTextBuffer *buf;
    GtkTextIter start, end;
    char *text;

    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);

    buf = gtk_text_view_get_buffer (GTK_TEXT_VIEW (edit));
    gtk_text_buffer_get_bounds (buf, &start, &end);
    text = gtk_text_buffer_get_text (buf, &start, &end, TRUE);

    if (text && *text)
    {
        return text;
    }
    else
    {
        g_free (text);
        return NULL;
    }
}
