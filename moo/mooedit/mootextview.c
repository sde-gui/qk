/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 * kate: space-indent on; indent-width 4; replace-tabs on;
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
#include "mooedit/mooedit-private.h"
#include "mooedit/mootextview.h"
#include "mooedit/mootextbuffer.h"
#include "mooutils/moomarshals.h"
#include "mooutils/moosignal.h"
#include <string.h>

#define LIGHT_BLUE "#EEF6FF"


MooTextSearchParams *_moo_text_search_params;
static MooTextSearchParams search_params = {-1, NULL, NULL, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};


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

static GtkTextBuffer *get_buffer (MooTextView *view);
static GtkSourceBuffer *get_source_buffer (MooTextView *view);
static MooTextBuffer *get_moo_buffer (MooTextView *view);

static void cursor_moved            (MooTextView    *view,
                                     GtkTextIter    *where);
static void has_selection_notify    (MooTextView    *view);
static void has_text_notify         (MooTextView    *view);

static void goto_line               (MooTextView    *view);

static void can_redo_cb             (MooTextView    *view,
                                     gboolean        arg);
static void can_undo_cb             (MooTextView    *view,
                                     gboolean        arg);

static void insert_text_cb          (GtkTextBuffer  *buffer,
                                     GtkTextIter    *iter,
                                     gchar          *text,
                                     gint            len,
                                     MooTextView    *view);


enum {
    DELETE_SELECTION,
    CAN_UNDO,
    CAN_REDO,
    FIND,
    FIND_NEXT,
    FIND_PREVIOUS,
    REPLACE,
    GOTO_LINE,
    CURSOR_MOVED,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];


enum {
    PROP_0,
    PROP_BUFFER,
    PROP_INDENTER,
    PROP_HIGHLIGHT_CURRENT_LINE,
    PROP_CURRENT_LINE_COLOR,
    PROP_CURRENT_LINE_COLOR_GDK,
    PROP_SHOW_TABS,
    PROP_HAS_TEXT,
    PROP_HAS_SELECTION
};


/* MOO_TYPE_TEXT_VIEW */
G_DEFINE_TYPE (MooTextView, moo_text_view, GTK_TYPE_SOURCE_VIEW)


static void moo_text_view_class_init (MooTextViewClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
    GtkTextViewClass *text_view_class = GTK_TEXT_VIEW_CLASS (klass);

    _moo_text_search_params = &search_params;

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
    klass->find = _moo_text_view_find;
    klass->find_next = _moo_text_view_find_next;
    klass->find_previous = _moo_text_view_find_previous;
    klass->replace = _moo_text_view_replace;
    klass->goto_line = goto_line;

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

    signals[DELETE_SELECTION] =
            g_signal_new ("delete-selection",
                          G_OBJECT_CLASS_TYPE (klass),
                          (GSignalFlags) (G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION),
                          G_STRUCT_OFFSET (MooTextViewClass, delete_selection),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[CAN_UNDO] =
            g_signal_new ("can-undo",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTextViewClass, can_undo),
                          NULL, NULL,
                          _moo_marshal_VOID__BOOLEAN,
                          G_TYPE_NONE, 1, G_TYPE_BOOLEAN);

    signals[CAN_REDO] =
            g_signal_new ("can-redo",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTextViewClass, can_redo),
                          NULL, NULL,
                          _moo_marshal_VOID__BOOLEAN,
                          G_TYPE_NONE, 1, G_TYPE_BOOLEAN);

    signals[FIND] =
            g_signal_new ("find",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                          G_STRUCT_OFFSET (MooTextViewClass, find),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[FIND_NEXT] =
            g_signal_new ("find-next",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                          G_STRUCT_OFFSET (MooTextViewClass, find_next),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[FIND_PREVIOUS] =
            g_signal_new ("find-previous",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                          G_STRUCT_OFFSET (MooTextViewClass, find_previous),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[REPLACE] =
            g_signal_new ("replace",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                          G_STRUCT_OFFSET (MooTextViewClass, replace),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[GOTO_LINE] =
            g_signal_new ("goto-line",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                          G_STRUCT_OFFSET (MooTextViewClass, goto_line),
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
}


static void moo_text_view_init (MooTextView *view)
{
    view->priv = _moo_text_view_private_new ();
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

    view->priv->can_undo_handler_id =
            g_signal_connect_swapped (get_buffer (view), "can-undo",
                                      G_CALLBACK (can_undo_cb),
                                      view);
    view->priv->can_redo_handler_id =
            g_signal_connect_swapped (get_buffer (view), "can-redo",
                                      G_CALLBACK (can_redo_cb),
                                      view);

    g_signal_connect_swapped (get_buffer (view), "cursor_moved",
                              G_CALLBACK (cursor_moved), view);
    g_signal_connect_swapped (get_buffer (view), "notify::has-selection",
                              G_CALLBACK (has_selection_notify), view);
    g_signal_connect_swapped (get_buffer (view), "notify::has-text",
                              G_CALLBACK (has_text_notify), view);

    g_signal_connect_after (get_buffer (view), "insert-text",
                            G_CALLBACK (insert_text_cb), view);

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


static void 
can_redo_cb (MooTextView *view,
             gboolean     arg)
{
    g_signal_emit (view, signals[CAN_REDO], 0, arg, NULL);
}

static void 
can_undo_cb (MooTextView *view,
             gboolean     arg)
{
    g_signal_emit (view, signals[CAN_UNDO], 0, arg, NULL);
}


MooTextViewPrivate*
_moo_text_view_private_new (void)
{
    MooTextViewPrivate *priv = g_new0 (MooTextViewPrivate, 1);

    gdk_color_parse (LIGHT_BLUE, &priv->current_line_color);

    priv->drag_button = GDK_BUTTON_RELEASE;
    priv->drag_type = MOO_TEXT_VIEW_DRAG_NONE;
    priv->drag_start_x = -1;
    priv->drag_start_y = -1;

    priv->last_search_stamp = -1;

    priv->tab_indents = TRUE;
    priv->shift_tab_unindents = TRUE;
    priv->backspace_indents = TRUE;
    priv->enter_indents = TRUE;
    priv->ctrl_up_down_scrolls = TRUE;
    priv->ctrl_page_up_down_scrolls = TRUE;

    return priv;
}


void        
moo_text_view_find (MooTextView *view)
{
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));
    g_signal_emit (view, signals[FIND], 0, NULL);
}

void        
moo_text_view_replace (MooTextView *view)
{
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));
    g_signal_emit (view, signals[REPLACE], 0, NULL);
}

void        
moo_text_view_find_next (MooTextView *view)
{
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));
    g_signal_emit (view, signals[FIND_NEXT], 0, NULL);
}

void        
moo_text_view_find_previous (MooTextView *view)
{
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));
    g_signal_emit (view, signals[FIND_PREVIOUS], 0, NULL);
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
    return gtk_source_buffer_can_redo (get_source_buffer (view));
}


gboolean    
moo_text_view_can_undo (MooTextView *view)
{
    g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), FALSE);
    return gtk_source_buffer_can_undo (get_source_buffer (view));
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
                buffer = g_object_new (MOO_TYPE_TEXT_BUFFER, NULL);
            }
            else if (!buffer)
            {
                buffer = g_object_new (MOO_TYPE_TEXT_BUFFER, NULL);
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

        case PROP_CURRENT_LINE_COLOR_GDK:
            g_value_set_boxed (value, &view->priv->current_line_color);
            break;

        case PROP_SHOW_TABS:
            g_value_set_boolean (value, view->priv->show_tabs);
            break;

        case PROP_HAS_TEXT:
            g_value_set_boolean (value, 
                                 moo_text_view_has_text (view));
            break;

        case PROP_HAS_SELECTION:
            g_value_set_boolean (value, 
                                 moo_text_view_has_selection (view));
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
insert_text_cb (GtkTextBuffer      *buffer,
                GtkTextIter        *iter,
                gchar              *text,
                gint                len,
                MooTextView        *view)
{
    if (view->priv->in_key_press && view->priv->indenter &&
        g_utf8_strlen (text, len) == 1)
    {
        gtk_text_buffer_begin_user_action (buffer);
        moo_indenter_character (view->priv->indenter, buffer,
                                g_utf8_get_char (text), iter);
        gtk_text_buffer_end_user_action (buffer);
    }
}


static void 
cursor_moved (MooTextView    *view,
              GtkTextIter    *where)
{
    g_signal_emit (view, signals[CURSOR_MOVED], 0, where);
}


static void 
has_selection_notify (MooTextView *view)
{
    g_object_notify (G_OBJECT (view), "has-selection");
}


static void 
has_text_notify (MooTextView *view)
{
    g_object_notify (G_OBJECT (view), "has-text");
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
        if (scroll->character >= gtk_text_iter_get_chars_in_line (&iter) - 1)
            scroll->character = gtk_text_iter_get_chars_in_line (&iter) - 2;
        if (scroll->character < 0)
            scroll->character = 0;
        gtk_text_iter_set_line_offset (&iter, scroll->character);
    }

    gtk_text_buffer_place_cursor (buffer, &iter);
    gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (scroll->view),
                                  gtk_text_buffer_get_insert (buffer),
                                  0.2, FALSE, 0, 0);

    return FALSE;
}


void
moo_text_view_move_cursor (MooTextView  *view,
                           int           line,
                           int           character,
                           gboolean      in_idle)
{
    Scroll *scroll = g_new (Scroll, 1);
    scroll->view = view;
    scroll->line = line;
    scroll->character = character;

    if (in_idle)
        g_idle_add ((GSourceFunc) do_move_cursor,
                     scroll);
    else
        do_move_cursor (scroll);
}


static GtkTextBuffer*
get_buffer (MooTextView *view)
{
    return gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
}


static GtkSourceBuffer*
get_source_buffer (MooTextView *view)
{
    return GTK_SOURCE_BUFFER (get_buffer (view));
}


static MooTextBuffer*
get_moo_buffer (MooTextView *view)
{
    return MOO_TEXT_BUFFER (get_buffer (view));
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
    g_return_if_fail (color != NULL);

    view->priv->current_line_color = *color;

    if (view->priv->current_line_gc)
    {
        g_object_unref (view->priv->current_line_gc);
        view->priv->current_line_gc = NULL;
        create_current_line_gc (view);
        if (GTK_WIDGET_DRAWABLE (view))
            gtk_widget_queue_draw (GTK_WIDGET (view));
    }

    g_object_notify (G_OBJECT (view), "current-line-color-gdk");
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
moo_text_view_draw_tab_markers (GtkTextView     *text_view, 
                                GdkEventExpose  *event,
                                GtkTextIter     *start,
                                GtkTextIter     *end)
{
    
    
    while (gtk_text_iter_compare (start, end) < 0)
    {
        if (gtk_text_iter_get_char (start) == '\t')
            draw_tab_at_iter (text_view, event, start);
        if (!gtk_text_iter_forward_char (start))
            break;
    }
}


static gboolean 
moo_text_view_expose (GtkWidget      *widget,
                      GdkEventExpose *event)
{
    gboolean handled;
    MooTextView *view = MOO_TEXT_VIEW (widget);
    GtkTextView *text_view = GTK_TEXT_VIEW (widget);
    GdkWindow *text_window = gtk_text_view_get_window (text_view, GTK_TEXT_WINDOW_TEXT);
    
    if (view->priv->highlight_current_line && event->window == text_window)
    {
        moo_text_view_draw_current_line (text_view, event);
    }

    handled = GTK_WIDGET_CLASS(moo_text_view_parent_class)->expose_event (widget, event);

    if (event->window == text_window && view->priv->show_tabs)
    {
        GdkRectangle visible_rect;
        GtkTextIter start, end;

        gtk_text_view_get_visible_rect (text_view, &visible_rect);
        gtk_text_view_get_line_at_y (text_view, &start,
                                     visible_rect.y, NULL);
        gtk_text_iter_backward_line (&start);
        gtk_text_view_get_line_at_y (text_view, &end,
                                     visible_rect.y
                                             + visible_rect.height, NULL);
        gtk_text_iter_forward_line (&end);

        moo_text_view_draw_tab_markers (text_view, event, &start, &end);
    }

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
