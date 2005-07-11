/*
 *   mooterm/mooterm.c
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

#define MOOTERM_COMPILATION
#include "mooterm/mooterm-private.h"
#include "mooterm/mootermparser.h"
#include "mooterm/mootermpt.h"
#include "mooterm/mootermbuffer-private.h"
#include "mooutils/moocompat.h"
#include "mooutils/moomarshals.h"


static void moo_term_finalize               (GObject        *object);

static void moo_term_realize                (GtkWidget          *widget);
static void moo_term_size_allocate          (GtkWidget          *widget,
                                             GtkAllocation      *allocation);

// static gboolean moo_term_popup_menu         (GtkWidget          *widget);
// static gboolean moo_term_scroll             (GtkWidget          *widget,
//                                              GdkEventScroll     *event);
// static gboolean moo_term_motion_notify      (GtkWidget          *widget,
//                                              GdkEventMotion     *event);

static void     moo_term_set_scroll_adjustments (GtkWidget      *widget,
                                                 GtkAdjustment  *hadj,
                                                 GtkAdjustment  *vadj);

static void     update_adjustment               (MooTerm        *term);
static void     update_adjustment_value         (MooTerm        *term);
static void     adjustment_value_changed        (MooTerm        *term);
static void     queue_adjustment_changed        (MooTerm        *term,
                                                 gboolean        now);
static void     queue_adjustment_value_changed  (MooTerm        *term,
                                                 gboolean        now);
static gboolean emit_adjustment_changed         (MooTerm        *term);
static gboolean emit_adjustment_value_changed   (MooTerm        *term);
static void     scroll_abs                      (MooTerm        *term,
                                                 guint           line,
                                                 gboolean        update_adj);
static void     scroll_to_bottom                (MooTerm        *term,
                                                 gboolean        update_adj);

static void     scrollback_changed              (MooTerm        *term,
                                                 GParamSpec     *pspec,
                                                 MooTermBuffer  *buf);
static void     width_changed                   (MooTerm        *term,
                                                 GParamSpec     *pspec);
static void     height_changed                  (MooTerm        *term,
                                                 GParamSpec     *pspec);


enum {
    SET_SCROLL_ADJUSTMENTS,
    SET_WINDOW_TITLE,
    SET_ICON_NAME,
    BELL,
    LAST_SIGNAL
};

enum {
    PROP_0 = 0,
    LAST_PROP
};


/* MOO_TYPE_TERM */
G_DEFINE_TYPE (MooTerm, moo_term, GTK_TYPE_WIDGET)
static guint signals[LAST_SIGNAL];


static void moo_term_class_init (MooTermClass *klass)
{
    GObjectClass   *gobject_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    gobject_class->finalize = moo_term_finalize;

    widget_class->realize = moo_term_realize;
    widget_class->size_allocate = moo_term_size_allocate;
    widget_class->expose_event = moo_term_expose_event;
//     widget_class->button_press_event = moo_term_button_press;
//     widget_class->button_release_event = moo_term_button_release;
    widget_class->key_press_event = moo_term_key_press;
    widget_class->key_release_event = moo_term_key_release;
//     widget_class->popup_menu = moo_term_popup_menu;
//     widget_class->scroll_event = moo_term_scroll;
//     widget_class->motion_notify_event = moo_term_motion_notify;

    klass->set_scroll_adjustments = moo_term_set_scroll_adjustments;

    signals[SET_SCROLL_ADJUSTMENTS] =
            g_signal_new ("set-scroll-adjustments",
                          G_OBJECT_CLASS_TYPE (gobject_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTermClass, set_scroll_adjustments),
                          NULL, NULL,
                          _moo_marshal_VOID__OBJECT_OBJECT,
                          G_TYPE_NONE, 2,
                          GTK_TYPE_ADJUSTMENT,
                          GTK_TYPE_ADJUSTMENT);
    widget_class->set_scroll_adjustments_signal = signals[SET_SCROLL_ADJUSTMENTS];

    signals[SET_WINDOW_TITLE] =
            g_signal_new ("set-window-title",
                          G_OBJECT_CLASS_TYPE (gobject_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTermClass, set_window_title),
                          NULL, NULL,
                          _moo_marshal_VOID__STRING,
                          G_TYPE_NONE, 1,
                          G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE);

    signals[SET_ICON_NAME] =
            g_signal_new ("set-icon-name",
                          G_OBJECT_CLASS_TYPE (gobject_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTermClass, set_icon_name),
                          NULL, NULL,
                          _moo_marshal_VOID__STRING,
                          G_TYPE_NONE, 1,
                          G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE);

    signals[BELL] =
            g_signal_new ("bell",
                          G_OBJECT_CLASS_TYPE (gobject_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTermClass, bell),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);
}


static void moo_term_init                   (MooTerm        *term)
{
    term->priv = g_new0 (MooTermPrivate, 1);

    term->priv->pt = moo_term_pt_new (term);
    term->priv->parser = moo_term_parser_new (term);

    term->priv->primary_buffer = moo_term_buffer_new (80, 24);
    term->priv->alternate_buffer = moo_term_buffer_new (80, 24);
    term->priv->buffer = term->priv->primary_buffer;

    term->priv->width = 80;
    term->priv->height = 24;

    term->priv->selection = term_selection_new ();

    term->priv->cursor_visible = TRUE;
    term->priv->scroll_on_keystroke = TRUE;

    set_default_modes (term->priv->modes);

    g_signal_connect_swapped (term->priv->primary_buffer,
                              "notify::scrollback",
                              G_CALLBACK (scrollback_changed),
                              term);
    g_signal_connect_swapped (term->priv->alternate_buffer,
                              "notify::scrollback",
                              G_CALLBACK (scrollback_changed),
                              term);

    g_signal_connect_swapped (term->priv->buffer,
                              "notify::screen-width",
                              G_CALLBACK (width_changed),
                              term);
    g_signal_connect_swapped (term->priv->buffer,
                              "notify::screen-height",
                              G_CALLBACK (height_changed),
                              term);

    g_signal_connect_swapped (term->priv->primary_buffer,
                              "changed",
                              G_CALLBACK (moo_term_buf_content_changed),
                              term);
    g_signal_connect_swapped (term->priv->alternate_buffer,
                              "changed",
                              G_CALLBACK (moo_term_buf_content_changed),
                              term);

    g_signal_connect_swapped (term->priv->primary_buffer,
                              "cursor-moved",
                              G_CALLBACK (moo_term_cursor_moved),
                              term);
    g_signal_connect_swapped (term->priv->alternate_buffer,
                              "cursor-moved",
                              G_CALLBACK (moo_term_cursor_moved),
                              term);

    g_signal_connect_swapped (term->priv->primary_buffer,
                              "feed-child",
                              G_CALLBACK (moo_term_feed_child),
                              term);
    g_signal_connect_swapped (term->priv->alternate_buffer,
                              "feed-child",
                              G_CALLBACK (moo_term_feed_child),
                              term);
}


static void moo_term_finalize               (GObject        *object)
{
    guint i, j;
    MooTerm *term = MOO_TERM (object);

    g_object_unref (term->priv->pt);
    moo_term_parser_free (term->priv->parser);

    g_object_unref (term->priv->primary_buffer);
    g_object_unref (term->priv->alternate_buffer);

    term_selection_free (term->priv->selection);
    term_font_info_free (term->priv->font_info);

    g_object_unref (term->priv->back_pixmap);

    if (term->priv->changed_content)
        gdk_region_destroy (term->priv->changed_content);

    if (term->priv->pending_expose)
        g_source_remove (term->priv->pending_expose);

    g_object_unref (term->priv->clip);
    g_object_unref (term->priv->layout);

    g_object_unref (term->priv->im);

    if (term->priv->adjustment)
        g_object_unref (term->priv->adjustment);

    for (i = 0; i < CURSORS_NUM; ++i)
        if (term->priv->cursor[i])
            gdk_cursor_unref (term->priv->cursor[i]);

    /* TODO TODO TODO */
    for (i = 0; i < 1; ++i)
        for (j = 0; j <= MOO_TERM_COLOR_MAX; ++j)
    {
        g_object_unref (term->priv->fg[i][j]);
        g_object_unref (term->priv->bg[i][j]);
    }
    g_object_unref (term->priv->fg[1][MOO_TERM_COLOR_MAX]);
    g_object_unref (term->priv->bg[1][MOO_TERM_COLOR_MAX]);
    g_object_unref (term->priv->fg[2][MOO_TERM_COLOR_MAX]);
    g_object_unref (term->priv->bg[2][MOO_TERM_COLOR_MAX]);

    g_free (term->priv);
    G_OBJECT_CLASS (moo_term_parent_class)->finalize (object);
}


static void moo_term_size_allocate          (GtkWidget          *widget,
                                             GtkAllocation      *allocation)
{
    int old_width, old_height;
    MooTerm *term = MOO_TERM (widget);

    old_width = widget->allocation.width;
    old_height = widget->allocation.height;
    widget->allocation = *allocation;

    if (GTK_WIDGET_REALIZED (widget))
    {
        gdk_window_move_resize (widget->window,
                                allocation->x, allocation->y,
                                allocation->width, allocation->height);

        if (old_width != allocation->width || old_height != allocation->height)
            moo_term_size_changed (term);
    }
}


static void moo_term_realize                (GtkWidget          *widget)
{
    MooTerm *term;
    GdkWindowAttr attributes;
    gint attributes_mask;
    GdkDisplay *display;
    GdkBitmap *empty_bitmap;
    GdkColor useless = {0, 0, 0, 0};
    char invisible_cursor_bits[] = { 0x0 };

    term = MOO_TERM (widget);

    empty_bitmap = gdk_bitmap_create_from_data (NULL, invisible_cursor_bits, 1, 1);
    term->priv->cursor[CURSOR_NONE] =
            gdk_cursor_new_from_pixmap (empty_bitmap,
                                        empty_bitmap,
                                        &useless,
                                        &useless, 0, 0);

    display = gtk_widget_get_display (widget);
    term->priv->cursor[CURSOR_TEXT] =
            gdk_cursor_new_for_display (display, GDK_XTERM);
    term->priv->cursor[CURSOR_POINTER] = NULL;

    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.x = widget->allocation.x;
    attributes.y = widget->allocation.y;
    attributes.width = widget->allocation.width;
    attributes.height = widget->allocation.height;
    attributes.wclass = GDK_INPUT_OUTPUT;
    attributes.visual = gtk_widget_get_visual (widget);
    attributes.colormap = gtk_widget_get_colormap (widget);
    attributes.cursor = term->priv->cursor[CURSOR_TEXT];
    attributes.event_mask = gtk_widget_get_events (widget) |
            GDK_EXPOSURE_MASK |
            GDK_BUTTON_PRESS_MASK |
            GDK_BUTTON_RELEASE_MASK |
            GDK_POINTER_MOTION_HINT_MASK |
            GDK_POINTER_MOTION_MASK |
            GDK_KEY_PRESS_MASK |
            GDK_KEY_RELEASE_MASK;
    attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL |
            GDK_WA_COLORMAP | GDK_WA_CURSOR;

    GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED | GTK_CAN_FOCUS | GTK_CAN_DEFAULT);
    widget->window = gdk_window_new (gtk_widget_get_parent_window (widget),
                                     &attributes, attributes_mask);
    gdk_window_set_user_data (widget->window, widget);

    widget->style = gtk_style_attach (widget->style, widget->window);

    gtk_widget_set_double_buffered (widget, FALSE);

    moo_term_setup_palette (term);
    moo_term_init_font_stuff (term);
    moo_term_init_back_pixmap (term);
    moo_term_size_changed (term);

    gdk_window_set_background (widget->window, &(widget->style->white));

    term->priv->im = gtk_im_multicontext_new ();
    gtk_im_context_set_client_window (term->priv->im, widget->window);
    gtk_im_context_set_use_preedit (term->priv->im, FALSE);
    g_signal_connect (term->priv->im, "commit",
                      G_CALLBACK (moo_term_im_commit), term);
    gtk_im_context_focus_in (term->priv->im);
}


static void     moo_term_set_scroll_adjustments (GtkWidget      *widget,
                                                 G_GNUC_UNUSED GtkAdjustment  *hadj,
                                                 GtkAdjustment  *vadj)
{
    moo_term_set_adjustment (MOO_TERM (widget), vadj);
}


void             moo_term_set_adjustment    (MooTerm        *term,
                                             GtkAdjustment  *vadj)
{
    if (term->priv->adjustment == vadj)
        return;

    if (term->priv->adjustment)
    {
        g_signal_handlers_disconnect_by_func (term->priv->adjustment,
                                              (gpointer) adjustment_value_changed,
                                              term);
        g_object_unref (term->priv->adjustment);
    }

    term->priv->adjustment = vadj;

    if (term->priv->adjustment)
    {
        g_object_ref (term->priv->adjustment);
        gtk_object_sink (GTK_OBJECT (term->priv->adjustment));

        update_adjustment (term);
        g_signal_connect_swapped (term->priv->adjustment,
                                  "value-changed",
                                  G_CALLBACK (adjustment_value_changed),
                                  term);
    }
}


static void     scrollback_changed              (MooTerm        *term,
                                                 G_GNUC_UNUSED GParamSpec *pspec,
                                                 MooTermBuffer  *buf)
{
    g_assert (!strcmp (pspec->name, "scrollback"));

    if (buf == term->priv->buffer)
    {
        guint scrollback = buf_scrollback (term->priv->buffer);

        if (term->priv->_scrolled && term->priv->_top_line > scrollback)
            scroll_to_bottom (term, TRUE);
        else
            update_adjustment (term);
    }
}


static void     width_changed                   (MooTerm        *term,
                                                 G_GNUC_UNUSED GParamSpec *pspec)
{
    g_assert (!strcmp (pspec->name, "screen-width"));

    term->priv->width = buf_screen_width (term->priv->buffer);

    if (GTK_WIDGET_REALIZED (term))
        moo_term_resize_back_pixmap (term);

    term_selection_set_width (term, term->priv->width);
    term_selection_clear (term);
}


static void     height_changed                  (MooTerm        *term,
                                                 G_GNUC_UNUSED GParamSpec *pspec)
{
    guint scrollback = buf_scrollback (term->priv->buffer);

    g_assert (!strcmp (pspec->name, "screen-height"));

    term->priv->height = buf_screen_height (term->priv->buffer);

    if (!term->priv->_scrolled || term->priv->_top_line > scrollback)
        scroll_to_bottom (term, TRUE);
    else
        update_adjustment (term);

    if (GTK_WIDGET_REALIZED (term))
        moo_term_resize_back_pixmap (term);

    term_selection_clear (term);
    moo_term_invalidate_all (term);
}


#define equal(a, b) (ABS((a)-(b)) < 0.4)

static void     update_adjustment               (MooTerm        *term)
{
    double upper, value, page_size;
    GtkAdjustment *adj = term->priv->adjustment;
    gboolean now = FALSE;

    if (!adj)
        return;

    upper = buf_total_height (term->priv->buffer);
    value = term_top_line (term);
    page_size = term->priv->height;

    if ((ABS (upper - term->priv->adjustment->upper) >
         ADJUSTMENT_DELTA * page_size) ||
         (ABS (value - term->priv->adjustment->value) >
         ADJUSTMENT_DELTA * page_size))
    {
        now = TRUE;
    }

    if (!equal (adj->lower, 0.0) || !equal (adj->upper, upper) ||
         !equal (adj->value, value) || !equal (adj->page_size, page_size) ||
         !equal (adj->page_increment, page_size) || !equal (adj->step_increment, 1.0))
    {
        adj->lower = 0.0;
        adj->upper = upper;
        adj->value = value;
        adj->page_size = page_size;
        adj->page_increment = page_size;
        adj->step_increment = 1.0;

        queue_adjustment_changed (term, now);
    }
}


static void     update_adjustment_value         (MooTerm        *term)
{
    double value = term_top_line (term);
    gboolean now = FALSE;

    if (!term->priv->adjustment)
        return;

    if (ABS (value - term->priv->adjustment->value) >
         ADJUSTMENT_DELTA * term->priv->height)
    {
        now = TRUE;
    }

    if (!equal (term->priv->adjustment->value, value))
    {
        term->priv->adjustment->value = value;
        queue_adjustment_value_changed (term, now);
    }
}


static void     adjustment_value_changed        (MooTerm        *term)
{
    guint val, real_val;

    g_assert (term->priv->adjustment != NULL);

    val = term->priv->adjustment->value;
    real_val = term_top_line (term);

    if (val == real_val)
        return;

    g_return_if_fail (val <= buf_scrollback (term->priv->buffer));

    scroll_abs (term, val, FALSE);
}


static void     queue_adjustment_changed        (MooTerm        *term,
                                                 gboolean        now)
{
    if (now)
    {
        if (term->priv->pending_adjustment_changed)
            g_source_remove (term->priv->pending_adjustment_changed);
        term->priv->pending_adjustment_changed = 0;
        emit_adjustment_changed (term);
    }
    else if (!term->priv->pending_adjustment_changed)
    {
        term->priv->pending_adjustment_changed =
                g_idle_add_full (ADJUSTMENT_PRIORITY,
                                 (GSourceFunc) emit_adjustment_changed,
                                 term,
                                 NULL);
    }
}


static void     queue_adjustment_value_changed  (MooTerm        *term,
                                                 gboolean        now)
{
    if (now)
    {
        if (term->priv->pending_adjustment_value_changed)
            g_source_remove (term->priv->pending_adjustment_value_changed);
        term->priv->pending_adjustment_value_changed = 0;
        emit_adjustment_value_changed (term);
    }
    else if (!term->priv->pending_adjustment_value_changed)
    {
        term->priv->pending_adjustment_value_changed =
                g_idle_add_full(ADJUSTMENT_PRIORITY,
                                (GSourceFunc) emit_adjustment_value_changed,
                                term,
                                NULL);
    }
}


static gboolean emit_adjustment_changed         (MooTerm        *term)
{
    term->priv->pending_adjustment_changed = 0;
    gtk_adjustment_changed (term->priv->adjustment);
    return FALSE;
}


static gboolean emit_adjustment_value_changed   (MooTerm        *term)
{
    term->priv->pending_adjustment_value_changed = 0;
    gtk_adjustment_value_changed (term->priv->adjustment);
    return FALSE;
}


static void     scroll_abs                      (MooTerm        *term,
                                                 guint           line,
                                                 gboolean        update_adj)
{
    if (term_top_line (term) == line)
    {
        if (update_adj)
            update_adjustment_value (term);

        return;
    }

    if (line >= buf_scrollback (term->priv->buffer))
        return scroll_to_bottom (term, update_adj);

    term->priv->_top_line = line;
    term->priv->_scrolled = TRUE;

    moo_term_invalidate_content_all (term);
    moo_term_invalidate_all (term);

    if (update_adj)
        update_adjustment_value (term);
}


static void     scroll_to_bottom                (MooTerm        *term,
                                                 gboolean        update_adj)
{
    if (!term->priv->_scrolled)
    {
        if (update_adj)
            update_adjustment_value (term);
        return;
    }

    term->priv->_scrolled = FALSE;

    moo_term_invalidate_content_all (term);
    moo_term_invalidate_all (term);

    if (update_adj)
        update_adjustment_value (term);
}


void        moo_term_size_changed       (MooTerm        *term)
{
    GtkWidget *widget = GTK_WIDGET (term);
    TermFontInfo *font_info = term->priv->font_info;
    guint width, height;
    guint old_width, old_height;

    width = widget->allocation.width / font_info->width;
    height = widget->allocation.height / font_info->height;
    old_width = term->priv->width;
    old_height = term->priv->height;

    if (width == old_width && height == old_height)
        return;

    moo_term_buffer_set_screen_size (term->priv->primary_buffer,
                                     width, height);
    moo_term_buffer_set_screen_size (term->priv->alternate_buffer,
                                     width, height);
    moo_term_pt_set_size (term->priv->pt, width, height);
}


gboolean         moo_term_fork_command      (MooTerm        *term,
                                             const char     *cmd,
                                             const char     *working_dir,
                                             char          **envp)
{
    g_return_val_if_fail (MOO_IS_TERM (term), FALSE);

    return moo_term_pt_fork_command (term->priv->pt,
                                     cmd, working_dir, envp);
}


void             moo_term_feed_child        (MooTerm        *term,
                                             const char     *string,
                                             int             len)
{
    g_return_if_fail (MOO_IS_TERM (term));
    moo_term_pt_write (term->priv->pt, string, len);
}


void             moo_term_copy_clipboard    (MooTerm        *term)
{
    g_message ("%s: implement me", G_STRFUNC);
}

void             moo_term_ctrl_c            (MooTerm        *term)
{
    g_message ("%s: implement me", G_STRFUNC);
}


void             moo_term_paste_clipboard   (MooTerm        *term)
{
    GtkClipboard *cb;
    char *text;

    g_return_if_fail (MOO_IS_TERM (term));

    cb = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
    text = gtk_clipboard_wait_for_text (cb);

    if (text)
    {
        moo_term_pt_write (term->priv->pt, text, -1);
        g_free (text);
    }
}


void            moo_term_feed               (MooTerm        *term,
                                             const char     *data,
                                             int             len)
{
    if (!len)
        return;

    g_return_if_fail (data != NULL);

    if (len < 0)
        len = strlen (data);

    moo_term_parser_parse (term->priv->parser, data, len);
}


void             moo_term_scroll_to_top     (MooTerm        *term)
{
    scroll_abs (term, 0, TRUE);
}

void             moo_term_scroll_to_bottom  (MooTerm        *term)
{
    scroll_to_bottom (term, TRUE);
}

void             moo_term_scroll_lines      (MooTerm        *term,
                                             int             lines)
{
    int top = term_top_line (term);

    top += lines;

    if (top < 0)
        top = 0;

    scroll_abs (term, top, TRUE);
}

void             moo_term_scroll_pages      (MooTerm        *term,
                                             int             pages)
{
    int top = term_top_line (term);

    top += pages * term->priv->height;

    if (top < 0)
        top = 0;

    scroll_abs (term, top, TRUE);
}


void        moo_term_set_window_title       (MooTerm        *term,
                                             const char     *title)
{
    g_signal_emit (term, signals[SET_WINDOW_TITLE], 0, title);
}


void        moo_term_set_icon_name          (MooTerm        *term,
                                             const char     *title)
{
    g_signal_emit (term, signals[SET_ICON_NAME], 0, title);
}


void        moo_term_bell                   (MooTerm    *term)
{
    g_signal_emit (term, signals[BELL], 0);
    g_message ("BELL");
}


void        moo_term_decid                  (MooTerm    *term)
{
    moo_term_feed_child (term, TERM_VT_DECID_STRING, -1);
}
