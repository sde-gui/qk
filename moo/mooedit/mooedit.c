/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
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

#include <gdk/gdkkeysyms.h>
#include "mooedit/mooedit-private.h"
#include "mooedit/mooeditdialogs.h"
#include "mooedit/mooeditprefs.h"
#include "mooedit/mooeditlangmgr.h"
#include "mooutils/moomarshals.h"
#include "mooutils/moocompat.h"
#include "mooutils/moofileutils.h"
#include "mooutils/moosignal.h"
#include <string.h>


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

static GtkTextBuffer *get_buffer        (MooEdit            *edit);

static void goto_line                   (MooEdit            *edit);

static void can_redo_cb                 (GtkSourceBuffer    *buffer,
                                         gboolean            arg,
                                         MooEdit            *edit);
static void can_undo_cb                 (GtkSourceBuffer    *buffer,
                                         gboolean            arg,
                                         MooEdit            *edit);
static void modified_changed_cb         (GtkTextBuffer      *buffer,
                                         MooEdit            *edit);

static void mark_set_cb                 (GtkTextBuffer      *buffer,
                                         GtkTextIter        *iter,
                                         GtkTextMark        *mark,
                                         MooEdit            *edit);
static void insert_text_cb              (GtkTextBuffer      *buffer,
                                         GtkTextIter        *iter,
                                         gchar              *text,
                                         gint                len,
                                         MooEdit            *edit);
static void delete_range_cb             (GtkTextBuffer      *textbuffer,
                                         GtkTextIter        *iter1,
                                         GtkTextIter        *iter2,
                                         MooEdit            *edit);


enum {
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
    CURSOR_MOVED,
    HAS_SELECTION,
    HAS_TEXT,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];


enum {
    PROP_0,
    PROP_EDITOR,
    PROP_INDENTER
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

    klass->doc_status_changed = NULL;
    klass->filename_changed = NULL;
    klass->lang_changed = NULL;

    klass->delete_selection = moo_edit_delete_selection;
    klass->can_redo = NULL;
    klass->can_undo = NULL;
    klass->has_selection = NULL;
    klass->has_text = NULL;
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

    g_object_class_install_property (gobject_class,
                                     PROP_INDENTER,
                                     g_param_spec_object ("indenter",
                                             "indenter",
                                             "indenter",
                                             MOO_TYPE_INDENTER,
                                             G_PARAM_READWRITE));

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

    signals[HAS_SELECTION] =
            g_signal_new ("has-selection",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooEditClass, has_selection),
                          NULL, NULL,
                          _moo_marshal_VOID__BOOLEAN,
                          G_TYPE_NONE, 1, G_TYPE_BOOLEAN);

    signals[HAS_TEXT] =
            g_signal_new ("has-text",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooEditClass, has_text),
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

    signals[CURSOR_MOVED] =
            moo_signal_new_cb ("cursor-moved",
                               G_OBJECT_CLASS_TYPE (klass),
                               G_SIGNAL_RUN_LAST,
                               NULL, NULL, NULL,
                               _moo_marshal_VOID__BOXED,
                               G_TYPE_NONE, 1,
                               GTK_TYPE_TEXT_ITER);

    /* TODO: this is wrong */
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

    /* XXX do something about this */
    buffer = gtk_source_buffer_new (NULL);
    gtk_text_view_set_buffer (GTK_TEXT_VIEW (object), GTK_TEXT_BUFFER (buffer));
    g_object_unref (G_OBJECT (buffer));

    edit = MOO_EDIT (object);

    edit->priv->constructed = TRUE;

    moo_edit_instances = g_slist_prepend (moo_edit_instances, edit);

    edit->priv->can_undo_handler_id =
            g_signal_connect (buffer,
                              "can-undo",
                              G_CALLBACK (can_undo_cb),
                              edit);
    edit->priv->can_redo_handler_id =
            g_signal_connect (buffer,
                              "can-redo",
                              G_CALLBACK (can_redo_cb),
                              edit);
    edit->priv->modified_changed_handler_id =
            g_signal_connect (buffer,
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

    g_signal_connect_after (buffer, "mark-set",
                            G_CALLBACK (mark_set_cb), edit);
    g_signal_connect_after (buffer, "insert-text",
                            G_CALLBACK (insert_text_cb), edit);
    g_signal_connect_after (buffer, "delete-range",
                            G_CALLBACK (delete_range_cb), edit);

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

    if (edit->priv->indenter)
        g_object_unref (edit->priv->indenter);

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


MooEdit        *moo_edit_new                   (void)
{
    return g_object_new (MOO_TYPE_EDIT, NULL);
}


void moo_edit_delete_selection   (MooEdit    *edit)
{
    g_return_if_fail (MOO_IS_EDIT (edit));
    gtk_text_buffer_delete_selection (get_buffer (edit), TRUE, TRUE);
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


void        moo_edit_set_modified           (MooEdit            *edit,
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


void        moo_edit_set_clean              (MooEdit            *edit,
                                             gboolean            clean)
{
    g_return_if_fail (MOO_IS_EDIT (edit));
    if (clean)
        edit->priv->status |= MOO_EDIT_CLEAN;
    else
        edit->priv->status &= ~MOO_EDIT_CLEAN;
    moo_edit_status_changed (edit);
}


void        moo_edit_status_changed     (MooEdit            *edit)
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
    priv->file_watch_policy = MOO_EDIT_RELOAD_IF_SAFE;

#if defined(__WIN32__)
    priv->line_end_type = MOO_EDIT_LINE_END_WIN32;
#elif defined(OS_DARWIN)
    priv->line_end_type = MOO_EDIT_LINE_END_MAC;
#else
    priv->line_end_type = MOO_EDIT_LINE_END_UNIX;
#endif

    priv->last_search_stamp = -1;

    priv->enable_indentation = TRUE;
    priv->tab_indents = TRUE;
    priv->shift_tab_unindents = TRUE;
    priv->backspace_indents = TRUE;
    priv->enter_indents = TRUE;
    priv->ctrl_up_down_scrolls = TRUE;
    priv->ctrl_page_up_down_scrolls = TRUE;

    return priv;
}


MooEditFileInfo *moo_edit_file_info_new     (const char         *filename,
                                             const char         *encoding)
{
    MooEditFileInfo *info = g_new0 (MooEditFileInfo, 1);
    info->filename = g_strdup (filename);
    info->encoding = g_strdup (encoding);
    return info;
}

MooEditFileInfo *moo_edit_file_info_copy    (const MooEditFileInfo  *info)
{
    MooEditFileInfo *copy;
    g_return_val_if_fail (info != NULL, NULL);
    copy = g_new (MooEditFileInfo, 1);
    copy->encoding = g_strdup (info->encoding);
    copy->filename = g_strdup (info->filename);
    return copy;
}

void        moo_edit_file_info_free         (MooEditFileInfo    *info)
{
    if (info)
    {
        g_free (info->encoding);
        g_free (info->filename);
        g_free (info);
    }
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
    GtkTextBuffer *buffer;

    g_return_if_fail (MOO_IS_EDIT (edit));

    if (lang == edit->priv->lang)
        return;

    buffer = get_buffer (edit);
    table = gtk_text_buffer_get_tag_table (buffer);
    gtk_source_tag_table_remove_source_tags (GTK_SOURCE_TAG_TABLE (table));

    if (edit->priv->lang)
    {
        g_object_unref (edit->priv->lang);
        edit->priv->lang = NULL;
    }

    if (lang)
    {
        GSList *tags;
        gunichar escape_char;

        edit->priv->lang = lang;
        g_object_ref (edit->priv->lang);

        tags = moo_edit_lang_get_tags (lang);
        gtk_source_tag_table_add_tags (GTK_SOURCE_TAG_TABLE (table), tags);
        g_slist_foreach (tags, (GFunc) g_object_unref, NULL);
        g_slist_free (tags);

        escape_char = moo_edit_lang_get_escape_char (lang);
        gtk_source_buffer_set_escape_char (GTK_SOURCE_BUFFER (buffer),
                                           escape_char);

        gtk_source_buffer_set_brackets (GTK_SOURCE_BUFFER (buffer),
                                        moo_edit_lang_get_brackets (lang));
    }
    else
    {
        gtk_source_buffer_set_escape_char (GTK_SOURCE_BUFFER (buffer), '\\');
        gtk_source_buffer_set_brackets (GTK_SOURCE_BUFFER (buffer), "{}()[]");
    }

    g_signal_emit (edit, signals[LANG_CHANGED], 0, NULL);
}


void        moo_edit_set_highlight          (MooEdit            *edit,
                                             gboolean            highlight)
{
    g_return_if_fail (MOO_IS_EDIT (edit));
    gtk_source_buffer_set_highlight (GTK_SOURCE_BUFFER (get_buffer (edit)),
                                     highlight);
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
    moo_edit_status_changed (edit);
}


gboolean    moo_edit_is_empty               (MooEdit            *edit)
{
    GtkTextIter start, end;

    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);

    if (MOO_EDIT_IS_MODIFIED (edit) || edit->priv->filename)
        return FALSE;

    gtk_text_buffer_get_bounds (get_buffer (edit), &start, &end);

    return !gtk_text_iter_compare (&start, &end);
}


gboolean    moo_edit_can_redo               (MooEdit            *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    return gtk_source_buffer_can_redo (GTK_SOURCE_BUFFER (get_buffer (edit)));
}


gboolean    moo_edit_can_undo               (MooEdit            *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    return gtk_source_buffer_can_undo (GTK_SOURCE_BUFFER (get_buffer (edit)));
}


void        moo_edit_select_all             (MooEdit            *edit)
{
    g_return_if_fail (MOO_IS_EDIT (edit));
    g_signal_emit_by_name (edit, "select-all", TRUE, NULL);
}


MooEditStatus moo_edit_get_status    (MooEdit            *edit)
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

    switch (prop_id)
    {
        case PROP_EDITOR:
            edit->priv->editor = g_value_get_object (value);
            break;

        case PROP_INDENTER:
            moo_edit_set_indenter (edit, g_value_get_object (value));
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

    switch (prop_id)
    {
        case PROP_EDITOR:
            g_value_set_object (value, edit->priv->editor);
            break;

        case PROP_INDENTER:
            g_value_set_object (value, edit->priv->indenter);
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

    buf = get_buffer (edit);

    if (gtk_text_buffer_get_selection_bounds (buf, &start, &end))
        return gtk_text_buffer_get_text (buf, &start, &end, TRUE);
    else
        return NULL;
}


gboolean    moo_edit_has_selection          (MooEdit            *edit)
{
    GtkTextIter start, end;
    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    return gtk_text_buffer_get_selection_bounds (get_buffer (edit), &start, &end);
}


gboolean    moo_edit_has_text               (MooEdit            *edit)
{
    GtkTextIter start, end;

    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);

    gtk_text_buffer_get_bounds (get_buffer (edit), &start, &end);
    return gtk_text_iter_compare (&start, &end) ? TRUE : FALSE;
}


char       *moo_edit_get_text               (MooEdit            *edit)
{
    GtkTextBuffer *buf;
    GtkTextIter start, end;
    char *text;

    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);

    buf = get_buffer (edit);
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


static void mark_set_cb                 (GtkTextBuffer      *buffer,
                                         GtkTextIter        *iter,
                                         GtkTextMark        *mark,
                                         MooEdit            *edit)
{
    GtkTextMark *insert = gtk_text_buffer_get_insert (buffer);
    GtkTextMark *sel_bound = gtk_text_buffer_get_selection_bound (buffer);
    gboolean has_selection = edit->priv->has_selection;

    if (mark == insert)
    {
        GtkTextIter iter2;
        gtk_text_buffer_get_iter_at_mark (buffer, &iter2, sel_bound);
        has_selection = gtk_text_iter_compare (iter, &iter2) ? TRUE : FALSE;
    }
    else if (mark == sel_bound)
    {
        GtkTextIter iter2;
        gtk_text_buffer_get_iter_at_mark (buffer, &iter2, insert);
        has_selection = gtk_text_iter_compare (iter, &iter2) ? TRUE : FALSE;
    }

    if (has_selection != edit->priv->has_selection)
    {
        edit->priv->has_selection = has_selection;
        g_signal_emit (edit, signals[HAS_SELECTION], 0, has_selection);
    }

    if (mark == insert)
        g_signal_emit (edit, signals[CURSOR_MOVED], 0, iter);
}


static void insert_text_cb              (GtkTextBuffer      *buffer,
                                         GtkTextIter        *iter,
                                         gchar              *text,
                                         gint                len,
                                         MooEdit            *edit)
{
    g_signal_emit (edit, signals[CURSOR_MOVED], 0, iter);

    if (!edit->priv->has_text)
    {
        edit->priv->has_text = TRUE;
        g_signal_emit (edit, signals[HAS_TEXT], 0, TRUE);
    }

    if (edit->priv->in_key_press && edit->priv->indenter &&
        g_utf8_strlen (text, len) == 1)
    {
        gtk_text_buffer_begin_user_action (buffer);
        moo_indenter_character (edit->priv->indenter, buffer,
                                g_utf8_get_char (text), iter);
        gtk_text_buffer_end_user_action (buffer);
    }
}


static void delete_range_cb             (GtkTextBuffer *buffer,
                                         GtkTextIter   *iter1,
                                         G_GNUC_UNUSED GtkTextIter *iter2,
                                         MooEdit       *edit)
{
    g_signal_emit (edit, signals[CURSOR_MOVED], 0, iter1);

    if (edit->priv->has_text)
    {
        GtkTextIter start, end;
        gtk_text_buffer_get_bounds (buffer, &start, &end);
        if (!gtk_text_iter_compare (&start, &end))
        {
            edit->priv->has_text = FALSE;
            g_signal_emit (edit, signals[HAS_TEXT], 0, FALSE);
        }
    }
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

const char *moo_edit_get_encoding           (MooEdit            *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    return edit->priv->encoding;
}


MooIndenter *moo_edit_get_indenter          (MooEdit            *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);
    return edit->priv->indenter;
}


void         moo_edit_set_indenter          (MooEdit            *edit,
                                             MooIndenter        *indenter)
{
    g_return_if_fail (MOO_IS_EDIT (edit));
    g_return_if_fail (!indenter || MOO_IS_INDENTER (indenter));

    if (edit->priv->indenter == indenter)
        return;

    if (edit->priv->indenter)
        g_object_unref (edit->priv->indenter);
    edit->priv->indenter = indenter;
    if (edit->priv->indenter)
        g_object_ref (edit->priv->indenter);

    g_object_notify (G_OBJECT (edit), "indenter");
}


#define MODE_STRING "-*-"

static MooIndenter *get_indenter_for_mode_string (MooEdit *edit)
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


void        _moo_edit_choose_indenter       (MooEdit            *edit)
{
    MooIndenter *indenter;

    if (!edit->priv->enable_indentation)
        return;

    indenter = get_indenter_for_mode_string (edit);

    if (!indenter && edit->priv->lang)
        indenter = moo_indenter_get_for_lang (moo_edit_lang_get_id (edit->priv->lang));

    if (!indenter)
        indenter = moo_indenter_default_new ();

    moo_edit_set_indenter (edit, indenter);
}


void
moo_edit_move_cursor (MooEdit *edit,
                      int      line,
                      int      character)
{
    GtkTextBuffer *buffer;
    GtkTextIter iter;

    g_return_if_fail (MOO_IS_EDIT (edit));
    g_return_if_fail (line >= 0);

    buffer = get_buffer (edit);

    if (line >= gtk_text_buffer_get_line_count (buffer))
        line = gtk_text_buffer_get_line_count (buffer) - 1;

    gtk_text_buffer_get_iter_at_line (buffer, &iter, line);

    if (character >= 0)
    {
        if (character >= gtk_text_iter_get_chars_in_line (&iter) - 1)
            character = gtk_text_iter_get_chars_in_line (&iter) - 2;
        if (character < 0)
            character = 0;
        gtk_text_iter_set_line_offset (&iter, character);
    }

    gtk_text_buffer_place_cursor (buffer, &iter);
    gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (edit),
                                  gtk_text_buffer_get_insert (buffer),
                                  0.2, FALSE, 0, 0);
}


static GtkTextBuffer*
get_buffer (MooEdit *edit)
{
    return gtk_text_view_get_buffer (GTK_TEXT_VIEW (edit));
}
