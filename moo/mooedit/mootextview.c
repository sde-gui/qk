/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mootextview.c
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
#include "mooedit/mootextview-private.h"
#include "mooedit/mootextview.h"
#include "mooedit/mootextbuffer.h"
#include "mooutils/moomarshals.h"
#include "mooutils/mooutils-gobject.h"
#include <string.h>

#define LIGHT_BLUE "#EEF6FF"


MooTextSearchParams *_moo_text_search_params;
static MooTextSearchParams search_params;


static GObject *moo_text_view_constructor   (GType                  type,
                                             guint                  n_construct_properties,
                                             GObjectConstructParam *construct_param);
static void     moo_text_view_finalize      (GObject        *object);

static void     moo_text_view_set_property  (GObject        *object,
                                             guint           prop_id,
                                             const GValue   *value,
                                             GParamSpec     *pspec);
static void     moo_text_view_get_property  (GObject        *object,
                                             guint           prop_id,
                                             GValue         *value,
                                             GParamSpec     *pspec);

static void     moo_text_view_realize       (GtkWidget      *widget);
static void     moo_text_view_unrealize     (GtkWidget      *widget);
static gboolean moo_text_view_expose        (GtkWidget      *widget,
                                             GdkEventExpose *event);

static void     create_current_line_gc      (MooTextView    *view);

static void     emit_can_undo_changed       (MooTextView    *view);
static void     emit_can_redo_changed       (MooTextView    *view);

static GtkTextBuffer *get_buffer            (MooTextView    *view);
static MooTextBuffer *get_moo_buffer        (MooTextView    *view);
static GtkTextMark *get_insert              (MooTextView    *view);

static void     cursor_moved                (MooTextView    *view,
                                             GtkTextIter    *where);
static void     proxy_prop_notify           (MooTextView    *view,
                                             GParamSpec     *pspec);

static void     goto_line                   (MooTextView    *view);

static void     insert_text_cb              (MooTextView    *view,
                                             GtkTextIter    *iter,
                                             gchar          *text,
                                             gint            len);
static gboolean moo_text_view_char_inserted (MooTextView    *view,
                                             GtkTextIter    *where,
                                             guint           character);


enum {
    DELETE_SELECTION,
    FIND_INTERACTIVE,
    FIND_NEXT_INTERACTIVE,
    FIND_PREV_INTERACTIVE,
    REPLACE_INTERACTIVE,
    GOTO_LINE_INTERACTIVE,
    CURSOR_MOVED,
    CHAR_INSERTED,
    UNDO,
    REDO,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];


enum {
    PROP_0,
    PROP_BUFFER,
    PROP_INDENTER,
    PROP_HIGHLIGHT_CURRENT_LINE,
    PROP_CHECK_BRACKETS,
    PROP_CURRENT_LINE_COLOR,
    PROP_CURRENT_LINE_COLOR_GDK,
    PROP_SHOW_TABS,
    PROP_HAS_TEXT,
    PROP_HAS_SELECTION,
    PROP_CAN_UNDO,
    PROP_CAN_REDO
};


/* MOO_TYPE_TEXT_VIEW */
G_DEFINE_TYPE (MooTextView, moo_text_view, GTK_TYPE_TEXT_VIEW)


static void moo_text_view_class_init (MooTextViewClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
    GtkTextViewClass *text_view_class = GTK_TEXT_VIEW_CLASS (klass);

    _moo_text_search_params = &search_params;
    search_params.last_search_stamp = -1;
    search_params.text_to_find_history = moo_history_list_new (NULL);
    search_params.replacement_history = moo_history_list_new (NULL);

    gobject_class->set_property = moo_text_view_set_property;
    gobject_class->get_property = moo_text_view_get_property;
    gobject_class->constructor = moo_text_view_constructor;
    gobject_class->finalize = moo_text_view_finalize;

    widget_class->key_press_event = _moo_text_view_key_press_event;
    widget_class->button_press_event = _moo_text_view_button_press_event;
    widget_class->button_release_event = _moo_text_view_button_release_event;
    widget_class->motion_notify_event = _moo_text_view_motion_event;
    widget_class->realize = moo_text_view_realize;
    widget_class->unrealize = moo_text_view_unrealize;
    widget_class->expose_event = moo_text_view_expose;

    text_view_class->move_cursor = _moo_text_view_move_cursor;
    text_view_class->delete_from_cursor = _moo_text_view_delete_from_cursor;

    klass->delete_selection = moo_text_view_delete_selection;
    klass->extend_selection = _moo_text_view_extend_selection;
    klass->find_interactive = _moo_text_view_find;
    klass->find_next_interactive = _moo_text_view_find_next;
    klass->find_prev_interactive = _moo_text_view_find_previous;
    klass->replace_interactive = _moo_text_view_replace;
    klass->goto_line_interactive = goto_line;
    klass->undo = moo_text_view_undo;
    klass->redo = moo_text_view_redo;
    klass->char_inserted = moo_text_view_char_inserted;

    g_object_class_install_property (gobject_class,
                                     PROP_BUFFER,
                                     g_param_spec_object ("buffer",
                                             "buffer",
                                             "buffer",
                                             MOO_TYPE_TEXT_BUFFER,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (gobject_class,
                                     PROP_HIGHLIGHT_CURRENT_LINE,
                                     g_param_spec_boolean ("highlight-current-line",
                                             "highlight-current-line",
                                             "highlight-current-line",
                                             TRUE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_CHECK_BRACKETS,
                                     g_param_spec_boolean ("check-brackets",
                                             "check-brackets",
                                             "check-brackets",
                                             TRUE,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_CURRENT_LINE_COLOR_GDK,
                                     g_param_spec_boxed ("current-line-color-gdk",
                                             "current-line-color-gdk",
                                             "current-line-color-gdk",
                                             GDK_TYPE_COLOR,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_CURRENT_LINE_COLOR,
                                     g_param_spec_string ("current-line-color",
                                             "current-line-color",
                                             "current-line-color",
                                             LIGHT_BLUE,
                                             G_PARAM_WRITABLE));

    g_object_class_install_property (gobject_class,
                                     PROP_SHOW_TABS,
                                     g_param_spec_boolean ("show-tabs",
                                             "show-tabs",
                                             "show-tabs",
                                             TRUE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_INDENTER,
                                     g_param_spec_object ("indenter",
                                             "indenter",
                                             "indenter",
                                             MOO_TYPE_INDENTER,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_HAS_TEXT,
                                     g_param_spec_boolean ("has-text",
                                             "has-text",
                                             "has-text",
                                             FALSE,
                                             G_PARAM_READABLE));

    g_object_class_install_property (gobject_class,
                                     PROP_HAS_SELECTION,
                                     g_param_spec_boolean ("has-selection",
                                             "has-selection",
                                             "has-selection",
                                             FALSE,
                                             G_PARAM_READABLE));

    g_object_class_install_property (gobject_class,
                                     PROP_CAN_UNDO,
                                     g_param_spec_boolean ("can-undo",
                                             "can-undo",
                                             "can-undo",
                                             FALSE,
                                             G_PARAM_READABLE));

    g_object_class_install_property (gobject_class,
                                     PROP_CAN_REDO,
                                     g_param_spec_boolean ("can-redo",
                                             "can-redo",
                                             "can-redo",
                                             FALSE,
                                             G_PARAM_READABLE));

    signals[UNDO] =
            g_signal_new ("undo",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                          G_STRUCT_OFFSET (MooTextViewClass, undo),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[REDO] =
            g_signal_new ("redo",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                          G_STRUCT_OFFSET (MooTextViewClass, redo),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[DELETE_SELECTION] =
            g_signal_new ("delete-selection",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                          G_STRUCT_OFFSET (MooTextViewClass, delete_selection),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[FIND_INTERACTIVE] =
            g_signal_new ("find-interactive",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                          G_STRUCT_OFFSET (MooTextViewClass, find_interactive),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[FIND_NEXT_INTERACTIVE] =
            g_signal_new ("find-next-interactive",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                          G_STRUCT_OFFSET (MooTextViewClass, find_next_interactive),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[FIND_PREV_INTERACTIVE] =
            g_signal_new ("find-prev-interactive",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                          G_STRUCT_OFFSET (MooTextViewClass, find_prev_interactive),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[REPLACE_INTERACTIVE] =
            g_signal_new ("replace-interactive",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                          G_STRUCT_OFFSET (MooTextViewClass, replace_interactive),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[GOTO_LINE_INTERACTIVE] =
            g_signal_new ("goto-line-interactive",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                          G_STRUCT_OFFSET (MooTextViewClass, goto_line_interactive),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[CHAR_INSERTED] =
            g_signal_new ("char-inserted",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTextViewClass, char_inserted),
                          g_signal_accumulator_true_handled, NULL,
                          _moo_marshal_BOOLEAN__BOXED_UINT,
                          G_TYPE_BOOLEAN, 2,
                          GTK_TYPE_TEXT_ITER | G_SIGNAL_TYPE_STATIC_SCOPE,
                          G_TYPE_UINT);

    signals[CURSOR_MOVED] =
            moo_signal_new_cb ("cursor-moved",
                               G_OBJECT_CLASS_TYPE (klass),
                               G_SIGNAL_RUN_LAST,
                               NULL, NULL, NULL,
                               _moo_marshal_VOID__BOXED,
                               G_TYPE_NONE, 1,
                               GTK_TYPE_TEXT_ITER);
}


static void moo_text_view_init (MooTextView *view)
{
    char *name;

    view->priv = g_new0 (MooTextViewPrivate, 1);

    gdk_color_parse (LIGHT_BLUE, &view->priv->current_line_color);

    view->priv->drag_button = GDK_BUTTON_RELEASE;
    view->priv->drag_type = MOO_TEXT_VIEW_DRAG_NONE;
    view->priv->drag_start_x = -1;
    view->priv->drag_start_y = -1;

    view->priv->last_search_stamp = -1;

    view->priv->tab_indents = TRUE;
    view->priv->shift_tab_unindents = TRUE;
    view->priv->backspace_indents = TRUE;
    view->priv->enter_indents = TRUE;
    view->priv->ctrl_up_down_scrolls = TRUE;
    view->priv->ctrl_page_up_down_scrolls = TRUE;
    view->priv->smart_home_end = TRUE;

    view->priv->check_brackets = TRUE;

    name = g_strdup_printf ("moo-text-view-%p", view);
    gtk_widget_set_name (GTK_WIDGET (view), name);
    g_free (name);
}


static GObject*
moo_text_view_constructor (GType                  type,
                           guint                  n_construct_properties,
                           GObjectConstructParam *construct_param)
{
    GObject *object;
    MooTextView *view;

    object = G_OBJECT_CLASS (moo_text_view_parent_class)->constructor (
        type, n_construct_properties, construct_param);

    view = MOO_TEXT_VIEW (object);

    view->priv->constructed = TRUE;

    g_object_set (get_buffer (view), "check-brackets",
                  view->priv->check_brackets, NULL);

    g_signal_connect_swapped (get_buffer (view), "cursor_moved",
                              G_CALLBACK (cursor_moved), view);
    g_signal_connect_swapped (get_buffer (view), "notify::has-selection",
                              G_CALLBACK (proxy_prop_notify), view);
    g_signal_connect_swapped (get_buffer (view), "notify::has-text",
                              G_CALLBACK (proxy_prop_notify), view);
    g_signal_connect_swapped (get_buffer (view), "notify::can-undo",
                              G_CALLBACK (proxy_prop_notify), view);
    g_signal_connect_swapped (get_buffer (view), "notify::can-redo",
                              G_CALLBACK (proxy_prop_notify), view);

    g_signal_connect_data (get_buffer (view), "insert-text",
                           G_CALLBACK (insert_text_cb), view,
                           NULL, G_CONNECT_AFTER | G_CONNECT_SWAPPED);

    view->priv->undo_mgr = gtk_source_undo_manager_new (get_buffer (view));
    gtk_source_undo_manager_set_max_undo_levels (view->priv->undo_mgr, -1);

    g_signal_connect_swapped (view->priv->undo_mgr, "can-undo",
                              G_CALLBACK (emit_can_undo_changed), view);
    g_signal_connect_swapped (view->priv->undo_mgr, "can-redo",
                              G_CALLBACK (emit_can_redo_changed), view);

    return object;
}


static void
moo_text_view_finalize (GObject *object)
{
    MooTextView *view = MOO_TEXT_VIEW (object);

    if (view->priv->indenter)
        g_object_unref (view->priv->indenter);

    g_free (view->priv);
    view->priv = NULL;

    G_OBJECT_CLASS (moo_text_view_parent_class)->finalize (object);
}


MooTextView*
moo_text_view_new (void)
{
    return g_object_new (MOO_TYPE_TEXT_VIEW, NULL);
}


void
moo_text_view_delete_selection (MooTextView *view)
{
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));
    gtk_text_buffer_delete_selection (get_buffer (view), TRUE, TRUE);
}


void
moo_text_view_find_interactive (MooTextView *view)
{
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));
    g_signal_emit (view, signals[FIND_INTERACTIVE], 0, NULL);
}

void
moo_text_view_replace_interactive (MooTextView *view)
{
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));
    g_signal_emit (view, signals[REPLACE_INTERACTIVE], 0, NULL);
}

void
moo_text_view_find_next_interactive (MooTextView *view)
{
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));
    g_signal_emit (view, signals[FIND_NEXT_INTERACTIVE], 0, NULL);
}

void
moo_text_view_find_prev_interactive (MooTextView *view)
{
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));
    g_signal_emit (view, signals[FIND_PREV_INTERACTIVE], 0, NULL);
}

static void
goto_line (MooTextView *view)
{
    moo_text_view_goto_line (view, -1);
}


void
moo_text_view_set_font_from_string (MooTextView *view,
                                    const char  *font)
{
    PangoFontDescription *font_desc = NULL;

    g_return_if_fail (MOO_IS_TEXT_VIEW (view));

    if (font)
        font_desc = pango_font_description_from_string (font);

    gtk_widget_modify_font (GTK_WIDGET (view), font_desc);

    if (font_desc)
        pango_font_description_free (font_desc);
}


gboolean
moo_text_view_can_redo (MooTextView *view)
{
    g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), FALSE);
    return gtk_source_undo_manager_can_redo (view->priv->undo_mgr);
}


gboolean
moo_text_view_can_undo (MooTextView *view)
{
    g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), FALSE);
    return gtk_source_undo_manager_can_undo (view->priv->undo_mgr);
}


static void
emit_can_undo_changed (MooTextView *view)
{
    g_object_notify (G_OBJECT (view), "can-undo");
}


static void
emit_can_redo_changed (MooTextView *view)
{
    g_object_notify (G_OBJECT (view), "can-redo");
}


void
moo_text_view_start_not_undoable_action (MooTextView *view)
{
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));
    gtk_source_undo_manager_begin_not_undoable_action (view->priv->undo_mgr);
    gtk_text_buffer_begin_user_action (get_buffer (view));
}


void
moo_text_view_end_not_undoable_action (MooTextView *view)
{
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));
    gtk_text_buffer_end_user_action (get_buffer (view));
    gtk_source_undo_manager_end_not_undoable_action (view->priv->undo_mgr);
}


void
moo_text_view_redo (MooTextView    *view)
{
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));
//     _moo_text_buffer_freeze_highlight (get_moo_buffer (view));
    gtk_source_undo_manager_redo (view->priv->undo_mgr);
    gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (view),
                                  get_insert (view),
                                  0, FALSE, 0, 0);
//     _moo_text_buffer_thaw_highlight (get_moo_buffer (view));
}


void
moo_text_view_undo (MooTextView    *view)
{
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));
//     _moo_text_buffer_freeze_highlight (get_moo_buffer (view));
    gtk_source_undo_manager_undo (view->priv->undo_mgr);
    gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (view),
                                  get_insert (view),
                                  0, FALSE, 0, 0);
//     _moo_text_buffer_thaw_highlight (get_moo_buffer (view));
}


void
moo_text_view_select_all (MooTextView *view)
{
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));
    g_signal_emit_by_name (view, "select-all", TRUE, NULL);
}


static void
moo_text_view_set_property (GObject        *object,
                            guint           prop_id,
                            const GValue   *value,
                            GParamSpec     *pspec)
{
    MooTextView *view = MOO_TEXT_VIEW (object);
    GtkTextBuffer *buffer;
    GdkColor color;

    switch (prop_id)
    {
        case PROP_BUFFER:
            buffer = g_value_get_object (value);

            if (buffer && !MOO_IS_TEXT_BUFFER (buffer))
            {
                g_warning ("%s: ignoring buffer not of type MooTextBuffer", G_STRLOC);
                buffer = moo_text_buffer_new (NULL);
            }
            else if (!buffer)
            {
                buffer = moo_text_buffer_new (NULL);
            }
            else
            {
                g_object_ref (buffer);
            }

            gtk_text_view_set_buffer (GTK_TEXT_VIEW (view), buffer);

            g_object_unref (buffer);
            break;

        case PROP_INDENTER:
            moo_text_view_set_indenter (view, g_value_get_object (value));
            break;

        case PROP_CHECK_BRACKETS:
            view->priv->check_brackets = g_value_get_boolean (value);
            if (view->priv->constructed)
                g_object_set (get_buffer (view), "check-brackets",
                              view->priv->check_brackets, NULL);
            break;

        case PROP_HIGHLIGHT_CURRENT_LINE:
            moo_text_view_set_highlight_current_line (view, g_value_get_boolean (value));
            break;

        case PROP_CURRENT_LINE_COLOR_GDK:
            moo_text_view_set_current_line_color (view, g_value_get_boxed (value));
            break;

        case PROP_CURRENT_LINE_COLOR:
            g_return_if_fail (g_value_get_string (value) != NULL);
            gdk_color_parse (g_value_get_string (value), &color);
            moo_text_view_set_current_line_color (view, &color);
            break;

        case PROP_SHOW_TABS:
            moo_text_view_set_show_tabs (view, g_value_get_boolean (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
moo_text_view_get_property (GObject        *object,
                            guint           prop_id,
                            GValue         *value,
                            GParamSpec     *pspec)
{
    MooTextView *view = MOO_TEXT_VIEW (object);
    gboolean val;

    switch (prop_id)
    {
        case PROP_BUFFER:
            g_value_set_object (value, get_buffer (view));
            break;

        case PROP_INDENTER:
            g_value_set_object (value, view->priv->indenter);
            break;

        case PROP_HIGHLIGHT_CURRENT_LINE:
            g_value_set_boolean (value, view->priv->highlight_current_line);
            break;

        case PROP_CHECK_BRACKETS:
            g_object_get (get_buffer (view), "check-brackets", &val, NULL);
            g_value_set_boolean (value, val);
            break;

        case PROP_CURRENT_LINE_COLOR_GDK:
            g_value_set_boxed (value, &view->priv->current_line_color);
            break;

        case PROP_SHOW_TABS:
            g_value_set_boolean (value, view->priv->show_tabs);
            break;

        case PROP_HAS_TEXT:
            g_value_set_boolean (value, moo_text_view_has_text (view));
            break;

        case PROP_HAS_SELECTION:
            g_value_set_boolean (value, moo_text_view_has_selection (view));
            break;

        case PROP_CAN_UNDO:
            g_value_set_boolean (value, moo_text_view_can_undo (view));
            break;

        case PROP_CAN_REDO:
            g_value_set_boolean (value, moo_text_view_can_redo (view));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


GType
moo_text_selection_type_get_type (void)
{
    static GType type = 0;

    if (!type)
    {
        static const GEnumValue values[] = {
            { MOO_TEXT_SELECT_CHARS, (char*)"MOO_TEXT_SELECT_CHARS", (char*)"select-chars" },
            { MOO_TEXT_SELECT_WORDS, (char*)"MOO_TEXT_SELECT_WORDS", (char*)"select-words" },
            { MOO_TEXT_SELECT_LINES, (char*)"MOO_TEXT_SELECT_LINES", (char*)"select-lines" },
            { 0, NULL, NULL }
        };
        type = g_enum_register_static ("MooTextSelectionType", values);
    }

    return type;
}


char*
moo_text_view_get_selection (MooTextView *view)
{
    GtkTextBuffer *buf;
    GtkTextIter start, end;

    g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), NULL);

    buf = get_buffer (view);

    if (gtk_text_buffer_get_selection_bounds (buf, &start, &end))
        return gtk_text_buffer_get_text (buf, &start, &end, TRUE);
    else
        return NULL;
}


gboolean
moo_text_view_has_selection (MooTextView *view)
{
    g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), FALSE);
    return moo_text_buffer_has_selection (get_moo_buffer (view));
}


gboolean
moo_text_view_has_text (MooTextView *view)
{
    g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), FALSE);
    return moo_text_buffer_has_text (get_moo_buffer (view));
}


char*
moo_text_view_get_text (MooTextView *view)
{
    GtkTextBuffer *buf;
    GtkTextIter start, end;
    char *text;

    g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), NULL);

    buf = get_buffer (view);
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


static void
insert_text_cb (MooTextView        *view,
                GtkTextIter        *iter,
                gchar              *text,
                gint                len)
{
    if (view->priv->in_key_press && g_utf8_strlen (text, len) == 1)
    {
        view->priv->in_key_press = FALSE;
        view->priv->char_inserted = g_utf8_get_char (text);
        view->priv->char_inserted_pos = *iter;
    }
}


void
_moo_text_view_check_char_inserted (MooTextView        *view)
{
    if (view->priv->char_inserted)
    {
        gboolean result;
        g_signal_emit (view, signals[CHAR_INSERTED], 0,
                       &view->priv->char_inserted_pos,
                       (guint) view->priv->char_inserted,
                       &result);
        view->priv->char_inserted = 0;
    }
}


static gboolean
moo_text_view_char_inserted (MooTextView    *view,
                             GtkTextIter    *where,
                             guint           character)
{
    if (view->priv->indenter)
    {
        GtkTextBuffer *buffer = get_buffer (view);
        gtk_text_buffer_begin_user_action (buffer);
        moo_indenter_character (view->priv->indenter, buffer,
                                character, where);
        gtk_text_buffer_end_user_action (buffer);
        return TRUE;
    }

    return FALSE;
}


static void
cursor_moved (MooTextView    *view,
              GtkTextIter    *where)
{
    g_signal_emit (view, signals[CURSOR_MOVED], 0, where);
}


static void
proxy_prop_notify (MooTextView *view,
                   GParamSpec  *pspec)
{
    g_object_notify (G_OBJECT (view), pspec->name);
}


MooIndenter*
moo_text_view_get_indenter (MooTextView *view)
{
    g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), NULL);
    return view->priv->indenter;
}


void
moo_text_view_set_indenter (MooTextView *view,
                            MooIndenter *indenter)
{
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));
    g_return_if_fail (!indenter || MOO_IS_INDENTER (indenter));

    if (view->priv->indenter == indenter)
        return;

    if (view->priv->indenter)
        g_object_unref (view->priv->indenter);
    view->priv->indenter = indenter;
    if (view->priv->indenter)
        g_object_ref (view->priv->indenter);

//     if (view->priv->indenter)
//         gtk_source_view_set_tabs_width (GTK_SOURCE_VIEW (view),
//                                         view->priv->indenter->tab_width);
//     else
//         gtk_source_view_set_tabs_width (GTK_SOURCE_VIEW (view), 8);

    g_object_notify (G_OBJECT (view), "indenter");
}


typedef struct {
    MooTextView *view;
    int      line;
    int      character;
} Scroll;


static gboolean
do_move_cursor (Scroll *scroll)
{
    GtkTextBuffer *buffer;
    GtkTextIter iter;

    g_return_val_if_fail (scroll != NULL, FALSE);
    g_return_val_if_fail (MOO_IS_TEXT_VIEW (scroll->view), FALSE);
    g_return_val_if_fail (scroll->line >= 0, FALSE);

    buffer = get_buffer (scroll->view);

    if (scroll->line >= gtk_text_buffer_get_line_count (buffer))
        scroll->line = gtk_text_buffer_get_line_count (buffer) - 1;

    gtk_text_buffer_get_iter_at_line (buffer, &iter, scroll->line);

    if (scroll->character >= 0)
    {
        if (scroll->character > gtk_text_iter_get_chars_in_line (&iter))
            scroll->character = gtk_text_iter_get_chars_in_line (&iter);
        if (scroll->character < 0)
            scroll->character = 0;
        gtk_text_iter_set_line_offset (&iter, scroll->character);
    }

    gtk_text_buffer_place_cursor (buffer, &iter);
    gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (scroll->view),
                                  gtk_text_buffer_get_insert (buffer),
                                  0.2, FALSE, 0, 0);

    g_free (scroll);
    return FALSE;
}


void
moo_text_view_move_cursor (MooTextView  *view,
                           int           line,
                           int           character,
                           gboolean      in_idle)
{
    Scroll *scroll;

    g_return_if_fail (MOO_IS_TEXT_VIEW (view));

    scroll = g_new (Scroll, 1);
    scroll->view = view;
    scroll->line = line;
    scroll->character = character;

    if (in_idle)
        g_idle_add ((GSourceFunc) do_move_cursor,
                     scroll);
    else
        do_move_cursor (scroll);
}


void
moo_text_view_get_cursor (MooTextView *view,
                          GtkTextIter *iter)
{
    GtkTextBuffer *buffer;

    g_return_if_fail (MOO_IS_TEXT_VIEW (view));
    g_return_if_fail (iter != NULL);

    buffer = get_buffer (view);
    gtk_text_buffer_get_iter_at_mark (buffer, iter,
                                      gtk_text_buffer_get_insert (buffer));
}


static GtkTextBuffer*
get_buffer (MooTextView *view)
{
    return gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
}


static MooTextBuffer*
get_moo_buffer (MooTextView *view)
{
    return MOO_TEXT_BUFFER (get_buffer (view));
}


static GtkTextMark*
get_insert (MooTextView    *view)
{
    return gtk_text_buffer_get_insert (get_buffer (view));
}


void
moo_text_view_set_highlight_current_line (MooTextView    *view,
                                          gboolean        highlight)
{
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));

    if (view->priv->highlight_current_line == highlight)
        return;

    view->priv->highlight_current_line = highlight;
    g_object_notify (G_OBJECT (view), "highlight-current-line");

    if (GTK_WIDGET_DRAWABLE (view))
        gtk_widget_queue_draw (GTK_WIDGET (view));
}


void
moo_text_view_set_current_line_color (MooTextView    *view,
                                      const GdkColor *color)
{
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));

    if (color)
    {
        view->priv->current_line_color = *color;
        view->priv->highlight_current_line = TRUE;

        if (view->priv->current_line_gc)
        {
            g_object_unref (view->priv->current_line_gc);
            view->priv->current_line_gc = NULL;
            create_current_line_gc (view);
            if (GTK_WIDGET_DRAWABLE (view))
                gtk_widget_queue_draw (GTK_WIDGET (view));
        }

        g_object_freeze_notify (G_OBJECT (view));
        g_object_notify (G_OBJECT (view), "current-line-color-gdk");
        g_object_notify (G_OBJECT (view), "current-line-color");
        g_object_notify (G_OBJECT (view), "highlight-current-line");
        g_object_thaw_notify (G_OBJECT (view));
    }
    else
    {
        view->priv->highlight_current_line = FALSE;

        if (view->priv->current_line_gc)
        {
            g_object_unref (view->priv->current_line_gc);
            view->priv->current_line_gc = NULL;
        }

        if (GTK_WIDGET_DRAWABLE (view))
            gtk_widget_queue_draw (GTK_WIDGET (view));

        g_object_notify (G_OBJECT (view), "highlight-current-line");
    }
}


static void
moo_text_view_realize (GtkWidget *widget)
{
    GTK_WIDGET_CLASS(moo_text_view_parent_class)->realize (widget);
    create_current_line_gc (MOO_TEXT_VIEW (widget));
}


static void
moo_text_view_unrealize (GtkWidget *widget)
{
    MooTextView *view = MOO_TEXT_VIEW (widget);
    if (view->priv->current_line_gc)
        g_object_unref (view->priv->current_line_gc);
    view->priv->current_line_gc = NULL;
    GTK_WIDGET_CLASS(moo_text_view_parent_class)->unrealize (widget);
}


static void
create_current_line_gc (MooTextView *view)
{
    GtkWidget *widget = GTK_WIDGET (view);
    GdkColormap *colormap;
    gboolean success;
    GdkWindow *window;

    g_return_if_fail (GTK_WIDGET_REALIZED (widget));
    g_return_if_fail (view->priv->current_line_gc == NULL);

    colormap = gtk_widget_get_colormap (widget);
    g_return_if_fail (colormap != NULL);

    window = gtk_text_view_get_window (GTK_TEXT_VIEW (view),
                                       GTK_TEXT_WINDOW_TEXT);
    g_return_if_fail (window != NULL);

    success = gdk_colormap_alloc_color (colormap,
                                        &view->priv->current_line_color,
                                        FALSE, TRUE);

    if (!success)
    {
        g_warning ("%s: failed to allocate color", G_STRLOC);
        view->priv->current_line_color = widget->style->bg[GTK_STATE_NORMAL];
    }

    view->priv->current_line_gc = gdk_gc_new (window);
    gdk_gc_set_foreground (view->priv->current_line_gc,
                           &view->priv->current_line_color);
}


static void
moo_text_view_draw_current_line (GtkTextView    *text_view,
                                 GdkEventExpose *event)
{
    GdkRectangle visible_rect;
    GdkRectangle redraw_rect;
    GtkTextIter cur;
    gint y;
    gint height;
    gint win_y;

    gtk_text_buffer_get_iter_at_mark (text_view->buffer,
                                      &cur,
                                      gtk_text_buffer_get_insert (text_view->buffer));

    gtk_text_view_get_line_yrange (text_view, &cur, &y, &height);

    gtk_text_view_get_visible_rect (text_view, &visible_rect);

    gtk_text_view_buffer_to_window_coords (text_view,
                                           GTK_TEXT_WINDOW_TEXT,
                                           visible_rect.x,
                                           visible_rect.y,
                                           &redraw_rect.x,
                                           &redraw_rect.y);

    gtk_text_view_buffer_to_window_coords (text_view,
                                           GTK_TEXT_WINDOW_TEXT,
                                           0,
                                           y,
                                           NULL,
                                           &win_y);

    redraw_rect.width = visible_rect.width;
    redraw_rect.height = visible_rect.height;

    gdk_draw_rectangle (event->window,
                        MOO_TEXT_VIEW(text_view)->priv->current_line_gc,
                        TRUE,
                        redraw_rect.x + MAX (0, gtk_text_view_get_left_margin (text_view) - 1),
                        win_y,
                        redraw_rect.width,
                        height);
}


static void
draw_tab_at_iter (GtkTextView    *text_view,
                  GdkEventExpose *event,
                  GtkTextIter    *iter)
{
    GdkRectangle rect;
    GdkPoint points[3];

    gtk_text_view_get_iter_location (text_view, iter, &rect);
    gtk_text_view_buffer_to_window_coords (text_view, GTK_TEXT_WINDOW_TEXT,
                                           rect.x, rect.y + rect.height - 2,
                                           &points[0].x, &points[0].y);
    points[1] = points[0];
    points[2] = points[0];
    points[1].y += 1;
    points[2].x += 1;
    points[2].y += 1;
    gdk_draw_polygon (event->window,
                      GTK_WIDGET(text_view)->style->text_gc[GTK_STATE_NORMAL],
                      FALSE, points, 3);
}


static void
moo_text_view_draw_tab_markers (GtkTextView       *text_view,
                                GdkEventExpose    *event,
                                const GtkTextIter *start,
                                const GtkTextIter *end)
{
    GtkTextIter iter = *start;

    while (gtk_text_iter_compare (&iter, end) < 0)
    {
        if (gtk_text_iter_get_char (&iter) == '\t')
            draw_tab_at_iter (text_view, event, &iter);
        if (!gtk_text_iter_forward_char (&iter))
            break;
    }
}


static void
moo_text_view_draw_trailing_space (GtkTextView       *text_view,
                                   GdkEventExpose    *event,
                                   const GtkTextIter *start,
                                   const GtkTextIter *end)
{
    GtkTextIter iter = *start;

    do
    {
        if (!gtk_text_iter_ends_line (&iter))
            gtk_text_iter_forward_to_line_end (&iter);

        while (!gtk_text_iter_starts_line (&iter))
        {
            gunichar c;
            gtk_text_iter_backward_char (&iter);
            c = gtk_text_iter_get_char (&iter);

            if (g_unichar_isspace (c))
                draw_tab_at_iter (text_view, event, &iter);
            else
                break;
        }

        gtk_text_iter_forward_line (&iter);
    }
    while (gtk_text_iter_compare (&iter, end) < 0);
}


static gboolean
moo_text_view_expose (GtkWidget      *widget,
                      GdkEventExpose *event)
{
    gboolean handled;
    MooTextView *view = MOO_TEXT_VIEW (widget);
    GtkTextView *text_view = GTK_TEXT_VIEW (widget);
    GdkWindow *text_window = gtk_text_view_get_window (text_view, GTK_TEXT_WINDOW_TEXT);
    GtkTextIter start, end;
    int first_line, last_line;

    if (view->priv->highlight_current_line &&
        event->window == text_window && view->priv->current_line_gc)
            moo_text_view_draw_current_line (text_view, event);

    if (event->window == text_window)
    {
        GdkRectangle visible_rect;

        /* XXX do correct math here */

        gtk_text_view_get_visible_rect (text_view, &visible_rect);
        gtk_text_view_get_line_at_y (text_view, &start,
                                     visible_rect.y, NULL);
        gtk_text_iter_backward_line (&start);
        gtk_text_view_get_line_at_y (text_view, &end,
                                     visible_rect.y
                                             + visible_rect.height, NULL);
        gtk_text_iter_forward_line (&end);

        first_line = gtk_text_iter_get_line (&start);
        last_line = gtk_text_iter_get_line (&end);

        _moo_text_buffer_ensure_highlight (get_moo_buffer (view),
                                           first_line, last_line);
    }

    handled = GTK_WIDGET_CLASS(moo_text_view_parent_class)->expose_event (widget, event);

    if (event->window == text_window && view->priv->show_tabs)
        moo_text_view_draw_tab_markers (text_view, event, &start, &end);

    if (event->window == text_window && view->priv->show_trailing_space)
        moo_text_view_draw_trailing_space (text_view, event, &start, &end);

    return handled;
}


void
moo_text_view_set_show_tabs (MooTextView    *view,
                             gboolean        show)
{
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));

    if (view->priv->show_tabs == show)
        return;

    view->priv->show_tabs = show;
    g_object_notify (G_OBJECT (view), "show-tabs");

    if (GTK_WIDGET_DRAWABLE (view))
        gtk_widget_queue_draw (GTK_WIDGET (view));
}


GtkTextTag*
moo_text_view_lookup_tag (MooTextView    *view,
                          const char     *name)
{
    GtkTextBuffer *buffer;
    GtkTextTagTable *table;

    g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), NULL);
    g_return_val_if_fail (name != NULL, NULL);

    buffer = get_buffer (view);
    table = gtk_text_buffer_get_tag_table (buffer);

    return gtk_text_tag_table_lookup (table, name);
}


void
moo_text_view_set_cursor_color (MooTextView    *view,
                                const GdkColor *color)
{
    char *rc_string;

    g_return_if_fail (MOO_IS_TEXT_VIEW (view));

    if (!color)
    {
        gtk_widget_ensure_style (GTK_WIDGET (view));
        g_return_if_fail (GTK_WIDGET (view)->style != NULL);
        color = &GTK_WIDGET(view)->style->text[GTK_STATE_NORMAL];
    }

    g_return_if_fail (color != NULL);

    rc_string = g_strdup_printf (
        "style \"%p\"\n"
        "{\n"
        "   GtkWidget::cursor-color = \"#%02x%02x%02x\"\n"
        "}\n"
        "widget \"*.%s\" style \"%p\"\n",
        view,
        color->red >> 8, color->green >> 8, color->blue >> 8,
        gtk_widget_get_name (GTK_WIDGET (view)), view
    );

    gtk_rc_parse_string (rc_string);

    g_free (rc_string);
}


void
moo_text_view_set_lang (MooTextView    *view,
                        MooLang        *lang)
{
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));
    moo_text_buffer_set_lang (get_moo_buffer (view), lang);
    /* GtkTextView does not want to properly redraw when
       tags are changed */
    gtk_widget_queue_draw (GTK_WIDGET (view));
}


void
moo_text_view_apply_scheme (MooTextView        *view,
                            MooTextStyleScheme *scheme)
{
    GdkColor color;
    GdkColor *color_ptr;
    MooTextBuffer *buffer;
    GtkWidget *widget;

    g_return_if_fail (scheme != NULL);
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));

    widget = GTK_WIDGET (view);
    buffer = get_moo_buffer (view);
    gtk_widget_ensure_style (widget);

    color_ptr = NULL;
    if (scheme->text_colors[MOO_TEXT_COLOR_FG])
    {
        if (gdk_color_parse (scheme->text_colors[MOO_TEXT_COLOR_FG], &color))
            color_ptr = &color;
        else
            g_warning ("%s: could not parse color '%s'", G_STRLOC,
                       scheme->text_colors[MOO_TEXT_COLOR_FG]);
    }
    gtk_widget_modify_text (widget, GTK_STATE_NORMAL, color_ptr);
    gtk_widget_modify_text (widget, GTK_STATE_ACTIVE, color_ptr);
    gtk_widget_modify_text (widget, GTK_STATE_PRELIGHT, color_ptr);
    gtk_widget_modify_text (widget, GTK_STATE_INSENSITIVE, color_ptr);
    moo_text_view_set_cursor_color (view, color_ptr);

    color_ptr = NULL;
    if (scheme->text_colors[MOO_TEXT_COLOR_BG])
    {
        if (gdk_color_parse (scheme->text_colors[MOO_TEXT_COLOR_BG], &color))
            color_ptr = &color;
        else
            g_warning ("%s: could not parse color '%s'", G_STRLOC,
                       scheme->text_colors[MOO_TEXT_COLOR_BG]);
    }
    gtk_widget_modify_base (widget, GTK_STATE_NORMAL, color_ptr);
    gtk_widget_modify_base (widget, GTK_STATE_ACTIVE, color_ptr);
    gtk_widget_modify_base (widget, GTK_STATE_PRELIGHT, color_ptr);
    gtk_widget_modify_base (widget, GTK_STATE_INSENSITIVE, color_ptr);

    color_ptr = NULL;
    if (scheme->text_colors[MOO_TEXT_COLOR_SEL_FG])
    {
        if (gdk_color_parse (scheme->text_colors[MOO_TEXT_COLOR_SEL_FG], &color))
            color_ptr = &color;
        else
            g_warning ("%s: could not parse color '%s'", G_STRLOC,
                       scheme->text_colors[MOO_TEXT_COLOR_SEL_FG]);
    }
    gtk_widget_modify_text (widget, GTK_STATE_SELECTED, color_ptr);

    color_ptr = NULL;
    if (scheme->text_colors[MOO_TEXT_COLOR_SEL_BG])
    {
        if (gdk_color_parse (scheme->text_colors[MOO_TEXT_COLOR_SEL_BG], &color))
            color_ptr = &color;
        else
            g_warning ("%s: could not parse color '%s'", G_STRLOC,
                       scheme->text_colors[MOO_TEXT_COLOR_SEL_BG]);
    }
    gtk_widget_modify_base (widget, GTK_STATE_SELECTED, color_ptr);

    color_ptr = NULL;
    if (scheme->text_colors[MOO_TEXT_COLOR_CUR_LINE])
    {
        if (gdk_color_parse (scheme->text_colors[MOO_TEXT_COLOR_CUR_LINE], &color))
            color_ptr = &color;
        else
            g_warning ("%s: could not parse color '%s'", G_STRLOC,
                       scheme->text_colors[MOO_TEXT_COLOR_CUR_LINE]);
    }
    moo_text_view_set_current_line_color (view, color_ptr);
    moo_text_view_set_highlight_current_line (view, color_ptr != NULL);

    moo_text_buffer_apply_scheme (buffer, scheme);
}


void
moo_text_view_strip_whitespace (MooTextView *view)
{
    GtkTextBuffer *buffer;
    GtkTextIter iter;

    g_return_if_fail (MOO_IS_TEXT_VIEW (view));

    buffer = get_buffer (view);
    gtk_text_buffer_begin_user_action (buffer);

    for (gtk_text_buffer_get_start_iter (buffer, &iter);
         !gtk_text_iter_is_end (&iter);
         gtk_text_iter_forward_line (&iter))
    {
        GtkTextIter end;

        if (gtk_text_iter_ends_line (&iter))
            continue;

        gtk_text_iter_forward_to_line_end (&iter);
        end = iter;

        do
        {
            gunichar c;

            gtk_text_iter_backward_char (&iter);
            c = gtk_text_iter_get_char (&iter);

            if (!g_unichar_isspace (c))
            {
                gtk_text_iter_forward_char (&iter);
                break;
            }
        }
        while (!gtk_text_iter_starts_line (&iter));

        gtk_text_buffer_delete (buffer, &iter, &end);
    }

    gtk_text_buffer_end_user_action (buffer);
}
