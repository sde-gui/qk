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
#include "mooedit/mootextfind.h"
#include "mooutils/moomarshals.h"
#include "mooutils/mooutils-gobject.h"
#include "mooutils/mooundomanager.h"
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>

#define LIGHT_BLUE "#EEF6FF"
#define BOOL_CMP(b1,b2) ((b1 && b2) || (!b1 && !b2))

static GObject *moo_text_view_constructor   (GType               type,
                                             guint               n_construct_properties,
                                             GObjectConstructParam *construct_param);
static void     moo_text_view_finalize      (GObject            *object);

static void     moo_text_view_set_property  (GObject            *object,
                                             guint               prop_id,
                                             const GValue       *value,
                                             GParamSpec         *pspec);
static void     moo_text_view_get_property  (GObject            *object,
                                             guint               prop_id,
                                             GValue             *value,
                                             GParamSpec         *pspec);

static void     moo_text_view_realize       (GtkWidget          *widget);
static void     moo_text_view_unrealize     (GtkWidget          *widget);
static gboolean moo_text_view_expose        (GtkWidget          *widget,
                                             GdkEventExpose     *event);

static void     moo_text_view_cut_clipboard (GtkTextView        *text_view);
static void     moo_text_view_paste_clipboard (GtkTextView      *text_view);
static void     moo_text_view_populate_popup(GtkTextView        *text_view,
                                             GtkMenu            *menu);

static void     create_current_line_gc      (MooTextView        *view);

static GtkTextBuffer *get_buffer            (MooTextView        *view);
static MooTextBuffer *get_moo_buffer        (MooTextView        *view);
static GtkTextMark *get_insert              (MooTextView        *view);

static void     cursor_moved                (MooTextView        *view,
                                             GtkTextIter        *where);
static void     proxy_prop_notify           (MooTextView        *view,
                                             GParamSpec         *pspec);

static void     find_interactive            (MooTextView        *view);
static void     replace_interactive         (MooTextView        *view);
static void     find_next_interactive       (MooTextView        *view);
static void     find_prev_interactive       (MooTextView        *view);
static void     goto_line_interactive       (MooTextView        *view);

static void     insert_text_cb              (MooTextView        *view,
                                             GtkTextIter        *iter,
                                             gchar              *text,
                                             gint                len);
static gboolean moo_text_view_char_inserted (MooTextView        *view,
                                             GtkTextIter        *where,
                                             guint               character);

static void     set_draw_tabs               (MooTextView        *view,
                                             gboolean            draw);
static void     set_draw_trailing_spaces    (MooTextView        *view,
                                             gboolean            draw);
static void     set_manage_clipboard        (MooTextView        *view,
                                             gboolean            manage);
static void     selection_changed           (MooTextView        *view,
                                             MooTextBuffer      *buffer);
static void     highlighting_changed        (GtkTextView        *view,
                                             const GtkTextIter  *start,
                                             const GtkTextIter  *end);


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
    PROP_DRAW_TABS,
    PROP_DRAW_TRAILING_SPACES,
    PROP_HAS_TEXT,
    PROP_HAS_SELECTION,
    PROP_CAN_UNDO,
    PROP_CAN_REDO,
    PROP_MANAGE_CLIPBOARD
};


/* MOO_TYPE_TEXT_VIEW */
G_DEFINE_TYPE (MooTextView, moo_text_view, GTK_TYPE_TEXT_VIEW)


static void moo_text_view_class_init (MooTextViewClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
    GtkTextViewClass *text_view_class = GTK_TEXT_VIEW_CLASS (klass);
    GtkBindingSet *binding_set;

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
#if 0
    widget_class->drag_data_received = _moo_text_view_drag_data_received;
    widget_class->drag_drop = _moo_text_view_drag_drop;
    widget_class->drag_leave = _moo_text_view_drag_leave;
    widget_class->drag_motion = _moo_text_view_drag_motion;
#endif

    text_view_class->move_cursor = _moo_text_view_move_cursor;
    text_view_class->delete_from_cursor = _moo_text_view_delete_from_cursor;
    text_view_class->cut_clipboard = moo_text_view_cut_clipboard;
    text_view_class->paste_clipboard = moo_text_view_paste_clipboard;
    text_view_class->populate_popup = moo_text_view_populate_popup;

    klass->delete_selection = moo_text_view_delete_selection;
    klass->extend_selection = _moo_text_view_extend_selection;
    klass->find_interactive = find_interactive;
    klass->find_next_interactive = find_next_interactive;
    klass->find_prev_interactive = find_prev_interactive;
    klass->replace_interactive = replace_interactive;
    klass->goto_line_interactive = goto_line_interactive;
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
                                     PROP_DRAW_TABS,
                                     g_param_spec_boolean ("draw-tabs",
                                             "draw-tabs",
                                             "draw-tabs",
                                             FALSE,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_DRAW_TRAILING_SPACES,
                                     g_param_spec_boolean ("draw-trailing-spaces",
                                             "draw-trailing-spaces",
                                             "draw-trailing-spaces",
                                             FALSE,
                                             G_PARAM_READWRITE));

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

    g_object_class_install_property (gobject_class,
                                     PROP_MANAGE_CLIPBOARD,
                                     g_param_spec_boolean ("manage-clipboard",
                                             "manage-clipboard",
                                             "manage-clipboard",
                                             TRUE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    signals[UNDO] =
            g_signal_new ("undo",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                          G_STRUCT_OFFSET (MooTextViewClass, undo),
                          g_signal_accumulator_true_handled, NULL,
                          _moo_marshal_BOOLEAN__VOID,
                          G_TYPE_BOOLEAN, 0);

    signals[REDO] =
            g_signal_new ("redo",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                          G_STRUCT_OFFSET (MooTextViewClass, redo),
                          g_signal_accumulator_true_handled, NULL,
                          _moo_marshal_BOOLEAN__VOID,
                          G_TYPE_BOOLEAN, 0);

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
                               GTK_TYPE_TEXT_ITER | G_SIGNAL_TYPE_STATIC_SCOPE);

    binding_set = gtk_binding_set_by_class (klass);
    gtk_binding_entry_add_signal (binding_set, GDK_z, GDK_CONTROL_MASK,
                                  "undo", 0);
    gtk_binding_entry_add_signal (binding_set, GDK_z, GDK_CONTROL_MASK | GDK_SHIFT_MASK,
                                  "redo", 0);
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

#if 0
    gtk_drag_dest_unset (GTK_WIDGET (view));
    gtk_drag_dest_set (GTK_WIDGET (view), 0, NULL, 0, GDK_ACTION_DEFAULT);
    view->priv->targets = gtk_target_list_new (NULL, 0);
    gtk_target_list_add_text_targets (view->priv->targets, DND_TARGET_TEXT);
#endif

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
    MooUndoMgr *undo_mgr;
    GtkTextIter iter;

    object = G_OBJECT_CLASS (moo_text_view_parent_class)->constructor (
        type, n_construct_properties, construct_param);

    view = MOO_TEXT_VIEW (object);

    view->priv->constructed = TRUE;

    g_object_set (get_buffer (view), "check-brackets",
                  view->priv->check_brackets, NULL);

    g_signal_connect_swapped (get_buffer (view), "cursor_moved",
                              G_CALLBACK (cursor_moved), view);
    g_signal_connect_swapped (get_buffer (view), "selection-changed",
                              G_CALLBACK (selection_changed), view);
    g_signal_connect_swapped (get_buffer (view), "highlighting-changed",
                              G_CALLBACK (highlighting_changed), view);
    g_signal_connect_swapped (get_buffer (view), "notify::has-selection",
                              G_CALLBACK (proxy_prop_notify), view);
    g_signal_connect_swapped (get_buffer (view), "notify::has-text",
                              G_CALLBACK (proxy_prop_notify), view);

    undo_mgr = moo_text_buffer_get_undo_mgr (get_moo_buffer (view));
    g_signal_connect_swapped (undo_mgr, "notify::can-undo",
                              G_CALLBACK (proxy_prop_notify), view);
    g_signal_connect_swapped (undo_mgr, "notify::can-redo",
                              G_CALLBACK (proxy_prop_notify), view);

    g_signal_connect_data (get_buffer (view), "insert-text",
                           G_CALLBACK (insert_text_cb), view,
                           NULL, G_CONNECT_AFTER | G_CONNECT_SWAPPED);

    gtk_text_buffer_get_start_iter (get_buffer (view), &iter);
    view->priv->dnd_mark = gtk_text_buffer_create_mark (get_buffer (view), NULL, &iter, FALSE);
    gtk_text_mark_set_visible (view->priv->dnd_mark, FALSE);

    return object;
}


static void
moo_text_view_finalize (GObject *object)
{
    MooTextView *view = MOO_TEXT_VIEW (object);

    if (view->priv->indenter)
        g_object_unref (view->priv->indenter);

    /* XXX free view->priv->targets */

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


static void
find_interactive (MooTextView *view)
{
    moo_text_view_run_find (GTK_TEXT_VIEW (view));
}

static void
replace_interactive (MooTextView *view)
{
    moo_text_view_run_replace (GTK_TEXT_VIEW (view));
}

static void
find_next_interactive (MooTextView *view)
{
    moo_text_view_run_find_next (GTK_TEXT_VIEW (view));
}

static void
find_prev_interactive (MooTextView *view)
{
    moo_text_view_run_find_prev (GTK_TEXT_VIEW (view));
}

static void
goto_line_interactive (MooTextView *view)
{
    moo_text_view_run_goto_line (GTK_TEXT_VIEW (view));
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


static MooUndoMgr *
get_undo_mgr (MooTextView *view)
{
    return moo_text_buffer_get_undo_mgr (get_moo_buffer (view));
}


gboolean
moo_text_view_can_redo (MooTextView *view)
{
    g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), FALSE);
    return moo_undo_mgr_can_redo (get_undo_mgr (view));
}


gboolean
moo_text_view_can_undo (MooTextView *view)
{
    g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), FALSE);
    return moo_undo_mgr_can_undo (get_undo_mgr (view));
}


void
moo_text_view_begin_not_undoable_action (MooTextView *view)
{
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));
    moo_undo_mgr_freeze (get_undo_mgr (view));
    gtk_text_buffer_begin_user_action (get_buffer (view));
}


void
moo_text_view_end_not_undoable_action (MooTextView *view)
{
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));
    gtk_text_buffer_end_user_action (get_buffer (view));
    moo_undo_mgr_thaw (get_undo_mgr (view));
}


gboolean
moo_text_view_redo (MooTextView    *view)
{
    g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), FALSE);

    if (moo_undo_mgr_can_redo (get_undo_mgr (view)))
    {
        moo_text_buffer_freeze (get_moo_buffer (view));
        moo_undo_mgr_redo (get_undo_mgr (view));
        gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (view),
                                      get_insert (view),
                                      0, FALSE, 0, 0);
        moo_text_buffer_thaw (get_moo_buffer (view));
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


gboolean
moo_text_view_undo (MooTextView    *view)
{
    g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), FALSE);

    if (moo_undo_mgr_can_undo (get_undo_mgr (view)))
    {
        moo_text_buffer_freeze (get_moo_buffer (view));
        moo_undo_mgr_undo (get_undo_mgr (view));
        gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (view),
                                      get_insert (view),
                                      0, FALSE, 0, 0);
        moo_text_buffer_thaw (get_moo_buffer (view));
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


static void
moo_text_view_cut_clipboard (GtkTextView *text_view)
{
    MooTextBuffer *buffer = get_moo_buffer (MOO_TEXT_VIEW (text_view));
    moo_text_buffer_begin_interactive_action (buffer);
    GTK_TEXT_VIEW_CLASS(moo_text_view_parent_class)->cut_clipboard (text_view);
    moo_text_buffer_end_interactive_action (buffer);
}


static void
moo_text_view_paste_clipboard (GtkTextView  *text_view)
{
    MooTextBuffer *buffer = get_moo_buffer (MOO_TEXT_VIEW (text_view));
    moo_text_buffer_begin_interactive_action (buffer);
    GTK_TEXT_VIEW_CLASS(moo_text_view_parent_class)->paste_clipboard (text_view);
    moo_text_buffer_end_interactive_action (buffer);
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

        case PROP_DRAW_TABS:
            set_draw_tabs (view, g_value_get_boolean (value));
            break;
        case PROP_DRAW_TRAILING_SPACES:
            set_draw_trailing_spaces (view, g_value_get_boolean (value));
            break;

        case PROP_MANAGE_CLIPBOARD:
            set_manage_clipboard (view, g_value_get_boolean (value));
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

        case PROP_DRAW_TABS:
            g_value_set_boolean (value, view->priv->draw_tabs != 0);
            break;
        case PROP_DRAW_TRAILING_SPACES:
            g_value_set_boolean (value, view->priv->draw_trailing_spaces != 0);
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

        case PROP_MANAGE_CLIPBOARD:
            g_value_set_boolean (value, view->priv->manage_clipboard != 0);
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
add_selection_clipboard (MooTextView *view)
{
    GtkClipboard *clipboard;
    clipboard = gtk_widget_get_clipboard (GTK_WIDGET (view), GDK_SELECTION_PRIMARY);
    gtk_text_buffer_add_selection_clipboard (get_buffer (view), clipboard);
}

static void
remove_selection_clipboard (MooTextView *view)
{
    GtkClipboard *clipboard;
    clipboard = gtk_widget_get_clipboard (GTK_WIDGET (view), GDK_SELECTION_PRIMARY);
    gtk_text_buffer_remove_selection_clipboard (get_buffer (view), clipboard);
}


static void
clipboard_get_selection (G_GNUC_UNUSED GtkClipboard *clipboard,
                         GtkSelectionData *selection_data,
                         G_GNUC_UNUSED guint info,
                         gpointer          data)
{
    MooTextView *view = data;
    GtkTextIter start, end;

    if (gtk_text_buffer_get_selection_bounds (get_buffer (view), &start, &end))
    {
        char *text = gtk_text_iter_get_text (&start, &end);
        gtk_selection_data_set_text (selection_data, text, -1);
        g_free (text);
    }
}


static const GtkTargetEntry targets[] = {
    { (char*) "STRING", 0, 0 },
    { (char*) "TEXT",   0, 0 },
    { (char*) "COMPOUND_TEXT", 0, 0 },
    { (char*) "UTF8_STRING", 0, 0 }
};

static void
clear_clipboard (MooTextView *view)
{
    GtkClipboard *clipboard;

    clipboard = gtk_widget_get_clipboard (GTK_WIDGET (view),
                                          GDK_SELECTION_PRIMARY);

    if (gtk_clipboard_get_owner (clipboard) == G_OBJECT (view))
        gtk_clipboard_clear (clipboard);
}


static void
selection_changed (MooTextView   *view,
                   MooTextBuffer *buffer)
{
    GtkClipboard *clipboard;

    if (!view->priv->manage_clipboard)
        return;

    clipboard = gtk_widget_get_clipboard (GTK_WIDGET (view),
                                          GDK_SELECTION_PRIMARY);

    if (moo_text_buffer_has_selection (buffer))
        gtk_clipboard_set_with_owner (clipboard, targets,
                                      G_N_ELEMENTS (targets),
                                      clipboard_get_selection,
                                      NULL, G_OBJECT (view));
    else
        clear_clipboard (view);
}


static void
set_manage_clipboard (MooTextView    *view,
                      gboolean        manage)
{
    if (BOOL_CMP (view->priv->manage_clipboard, manage))
        return;

    view->priv->manage_clipboard = manage;

    if (GTK_WIDGET_REALIZED (view))
    {
        if (manage)
        {
            remove_selection_clipboard (view);
            selection_changed (view, get_moo_buffer (view));
        }
        else
        {
            clear_clipboard (view);
            add_selection_clipboard (view);
        }
    }

    g_object_notify (G_OBJECT (view), "manage-clipboard");
}


static void
moo_text_view_realize (GtkWidget *widget)
{
    MooTextView *view = MOO_TEXT_VIEW (widget);

    GTK_WIDGET_CLASS(moo_text_view_parent_class)->realize (widget);

    create_current_line_gc (view);

    if (view->priv->manage_clipboard)
    {
        remove_selection_clipboard (view);
        selection_changed (view, get_moo_buffer (view));
    }
}


static void
moo_text_view_unrealize (GtkWidget *widget)
{
    MooTextView *view = MOO_TEXT_VIEW (widget);

    if (view->priv->current_line_gc)
        g_object_unref (view->priv->current_line_gc);
    view->priv->current_line_gc = NULL;

    if (view->priv->manage_clipboard)
    {
        clear_clipboard (view);
        add_selection_clipboard (view);
    }

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
moo_text_view_draw_tabs (GtkTextView       *text_view,
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
moo_text_view_draw_trailing_spaces (GtkTextView       *text_view,
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

        /* it reports bogus values sometimes, like on opening huge file */
        if (last_line - first_line < 2000)
            _moo_text_buffer_ensure_highlight (get_moo_buffer (view),
                                               first_line, last_line);
    }

    handled = GTK_WIDGET_CLASS(moo_text_view_parent_class)->expose_event (widget, event);

    if (event->window == text_window && view->priv->draw_tabs)
        moo_text_view_draw_tabs (text_view, event, &start, &end);

    if (event->window == text_window && view->priv->draw_trailing_spaces)
        moo_text_view_draw_trailing_spaces (text_view, event, &start, &end);

    return handled;
}


static void
highlighting_changed (GtkTextView        *view,
                      const GtkTextIter  *start,
                      const GtkTextIter  *end)
{
    GdkRectangle visible, redraw;
    int y, height;

    if (!GTK_WIDGET_DRAWABLE (view))
        return;

    gtk_text_view_get_visible_rect (view, &visible);

    gtk_text_view_get_line_yrange (view, start, &redraw.y, &redraw.height);
    gtk_text_view_get_line_yrange (view, end, &y, &height);
    redraw.height += y + redraw.y;
    redraw.x = visible.x;
    redraw.width = visible.width;

    if (gdk_rectangle_intersect (&redraw, &visible, &redraw))
    {
        GdkWindow *window = gtk_text_view_get_window (view, GTK_TEXT_WINDOW_TEXT);
        gtk_text_view_buffer_to_window_coords (view,
                                               GTK_TEXT_WINDOW_TEXT,
                                               redraw.x,
                                               redraw.y,
                                               &redraw.x,
                                               &redraw.y);
        gdk_window_invalidate_rect (window, &redraw, FALSE);
    }
}


static void
set_draw_tabs (MooTextView    *view,
               gboolean        draw)
{
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));

    if (BOOL_CMP (view->priv->draw_tabs, draw))
        return;

    view->priv->draw_tabs = draw != 0;
    g_object_notify (G_OBJECT (view), "draw-tabs");

    if (GTK_WIDGET_DRAWABLE (view))
        gtk_widget_queue_draw (GTK_WIDGET (view));
}


static void
set_draw_trailing_spaces (MooTextView    *view,
                          gboolean        draw)
{
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));

    if (BOOL_CMP (view->priv->draw_trailing_spaces, draw))
        return;

    view->priv->draw_trailing_spaces = draw != 0;
    g_object_notify (G_OBJECT (view), "draw-trailing-spaces");

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
    gtk_widget_queue_draw (GTK_WIDGET (view));
}


MooLang*
moo_text_view_get_lang (MooTextView *view)
{
    g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), NULL);
    return moo_text_buffer_get_lang (get_moo_buffer (view));
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


static void
moo_text_view_populate_popup (GtkTextView    *text_view,
                              GtkMenu        *menu)
{
    MooTextView *view = MOO_TEXT_VIEW (text_view);
    GtkWidget *item;

    item = gtk_separator_menu_item_new ();
    gtk_widget_show (item);
    gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), item);

    item = gtk_image_menu_item_new_from_stock (GTK_STOCK_REDO, NULL);
    gtk_widget_show (item);
    gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), item);
    g_signal_connect_swapped (item, "activate", G_CALLBACK (moo_text_view_redo), view);
    gtk_widget_set_sensitive (item, moo_text_view_can_redo (view));

    item = gtk_image_menu_item_new_from_stock (GTK_STOCK_UNDO, NULL);
    gtk_widget_show (item);
    gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), item);
    g_signal_connect_swapped (item, "activate", G_CALLBACK (moo_text_view_undo), view);
    gtk_widget_set_sensitive (item, moo_text_view_can_undo (view));
}
