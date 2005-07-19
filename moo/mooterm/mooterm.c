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
#include "mooterm/mooterm-selection.h"
#include "mooterm/mootermparser.h"
#include "mooterm/mootermpt.h"
#include "mooterm/mootermbuffer-private.h"
#include "mooui/mootext.h"
#include "mooutils/moocompat.h"
#include "mooutils/moomarshals.h"


static void moo_term_class_init             (MooTermClass   *klass);
static void moo_term_init                   (MooTerm        *term);
static void moo_term_finalize               (GObject        *object);

static void moo_term_realize                (GtkWidget          *widget);
static void moo_term_size_allocate          (GtkWidget          *widget,
                                             GtkAllocation      *allocation);

static gboolean moo_term_popup_menu         (GtkWidget          *widget);
static gboolean moo_term_scroll             (GtkWidget          *widget,
                                             GdkEventScroll     *event);

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

static void     im_preedit_start                (MooTerm        *term);
static void     im_preedit_end                  (MooTerm        *term);

static void     child_died                      (MooTerm        *term);

static void     clear_saved_cursor              (MooTerm        *term);


enum {
    SET_SCROLL_ADJUSTMENTS,
    SET_WINDOW_TITLE,
    SET_ICON_NAME,
    BELL,
    CHILD_DIED,
    POPULATE_POPUP,
    LAST_SIGNAL
};

enum {
    PROP_0 = 0,
    LAST_PROP
};


/* MOO_TYPE_TERM */
static gpointer moo_term_parent_class = NULL;
GType moo_term_get_type (void)
{
    static GType type = 0;

    if (!type)
    {
        static const GTypeInfo info = {
            sizeof (MooTermClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) moo_term_class_init,
            (GClassFinalizeFunc) NULL,
            NULL,   /* class_data */
            sizeof (MooTerm),
            0,      /* n_preallocs */
            (GInstanceInitFunc) moo_term_init,
            NULL    /* value_table */
        };

        static const GInterfaceInfo iface_info = {
            (GInterfaceInitFunc) moo_term_text_iface_init,
            NULL,
            NULL
        };

        type = g_type_register_static (GTK_TYPE_WIDGET, "MooTerm",
                                       &info, (GTypeFlags) 0);

        g_type_add_interface_static (type,
                                     MOO_TYPE_TEXT,
                                     &iface_info);
    }

    return type;
}


static guint signals[LAST_SIGNAL];


static void moo_term_class_init (MooTermClass *klass)
{
    GObjectClass   *gobject_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    moo_term_parent_class = g_type_class_peek_parent (klass);

    gobject_class->finalize = moo_term_finalize;

    widget_class->realize = moo_term_realize;
    widget_class->size_allocate = moo_term_size_allocate;
    widget_class->expose_event = moo_term_expose_event;
    widget_class->key_press_event = moo_term_key_press;
    widget_class->key_release_event = moo_term_key_release;
    widget_class->popup_menu = moo_term_popup_menu;
    widget_class->scroll_event = moo_term_scroll;
    widget_class->button_press_event = moo_term_button_press;
    widget_class->button_release_event = moo_term_button_release;
    widget_class->motion_notify_event = moo_term_motion_notify;

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

    signals[CHILD_DIED] =
            g_signal_new ("child-died",
                          G_OBJECT_CLASS_TYPE (gobject_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTermClass, child_died),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[POPULATE_POPUP] =
            g_signal_new ("populate-popup",
                          G_OBJECT_CLASS_TYPE (gobject_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTermClass, populate_popup),
                          NULL, NULL,
                          _moo_marshal_VOID__OBJECT,
                          G_TYPE_NONE, 1,
                          GTK_TYPE_MENU);
}


static void moo_term_init                   (MooTerm        *term)
{
    term->priv = g_new0 (MooTermPrivate, 1);

    term->priv->pt = moo_term_pt_new (term);
    g_signal_connect_swapped (term->priv->pt, "child-died",
                              G_CALLBACK (child_died), term);

    term->priv->parser = moo_term_parser_new (term);

    term->priv->primary_buffer = moo_term_buffer_new (80, 24);
    term->priv->alternate_buffer = moo_term_buffer_new (80, 24);
    term->priv->buffer = term->priv->primary_buffer;

    clear_saved_cursor (term);

    term->priv->width = 80;
    term->priv->height = 24;

    term->priv->selection = term_selection_new (term);

    term->priv->cursor_visible = TRUE;

    term->priv->settings.hide_pointer_on_keypress = TRUE;
    term->priv->settings.meta_sends_escape = TRUE;
    term->priv->settings.scroll_on_keystroke = TRUE;
    term->priv->settings.backspace_binding = MOO_TERM_ERASE_AUTO;
    term->priv->settings.delete_binding = MOO_TERM_ERASE_AUTO;

    term->priv->pointer_visible = TRUE;

    set_default_modes (term->priv->modes);
    set_default_modes (term->priv->saved_modes);

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

    g_free (term->priv->selection);
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

    for (i = 0; i < POINTERS_NUM; ++i)
        if (term->priv->pointer[i])
            gdk_cursor_unref (term->priv->pointer[i]);

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
    term->priv->pointer[POINTER_NONE] =
            gdk_cursor_new_from_pixmap (empty_bitmap,
                                        empty_bitmap,
                                        &useless,
                                        &useless, 0, 0);

    display = gtk_widget_get_display (widget);
    term->priv->pointer[POINTER_TEXT] =
            gdk_cursor_new_for_display (display, GDK_XTERM);
    term->priv->pointer[POINTER_NORMAL] = NULL;

    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.x = widget->allocation.x;
    attributes.y = widget->allocation.y;
    attributes.width = widget->allocation.width;
    attributes.height = widget->allocation.height;
    attributes.wclass = GDK_INPUT_OUTPUT;
    attributes.visual = gtk_widget_get_visual (widget);
    attributes.colormap = gtk_widget_get_colormap (widget);
    attributes.cursor = term->priv->pointer[POINTER_TEXT];
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
    g_signal_connect_swapped (term->priv->im, "preedit-start",
                              G_CALLBACK (im_preedit_start), term);
    g_signal_connect_swapped (term->priv->im, "preedit-end",
                              G_CALLBACK (im_preedit_end), term);
    gtk_im_context_focus_in (term->priv->im);
}


static void     im_preedit_start    (MooTerm        *term)
{
    term->priv->im_preedit_active = TRUE;
}


static void     im_preedit_end      (MooTerm        *term)
{
    term->priv->im_preedit_active = FALSE;
}


static void     child_died                      (MooTerm        *term)
{
    g_signal_emit (term, signals[CHILD_DIED], 0);
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
        {
            moo_term_selection_clear (term);
            scroll_to_bottom (term, TRUE);
        }
        else
        {
            update_adjustment (term);
        }
    }
}


static void     width_changed                   (MooTerm        *term,
                                                 G_GNUC_UNUSED GParamSpec *pspec)
{
    g_assert (!strcmp (pspec->name, "screen-width"));

    term->priv->width = buf_screen_width (term->priv->buffer);

    if (GTK_WIDGET_REALIZED (term))
        moo_term_resize_back_pixmap (term);

    moo_term_selection_clear (term);
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

    moo_term_selection_clear (term);
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

    moo_term_invalidate_all (term);

    if (update_adj)
        update_adjustment_value (term);
}


static void     scroll_to_bottom                (MooTerm        *term,
                                                 gboolean        update_adj)
{
    guint scrollback = buf_scrollback (term->priv->buffer);
    gboolean update_full = FALSE;

    if (!term->priv->_scrolled && term->priv->_top_line <= scrollback)
    {
        if (update_adj)
            update_adjustment_value (term);
        return;
    }

    if (term->priv->_top_line > scrollback)
    {
        term->priv->_top_line = scrollback;
        update_full = TRUE;
    }

    term->priv->_scrolled = FALSE;

    moo_term_invalidate_all (term);

    if (update_full)
        update_adjustment (term);
    else if (update_adj)
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

    width = CLAMP (width, MIN_TERMINAL_WIDTH, MAX_TERMINAL_WIDTH);
    height = height < MIN_TERMINAL_HEIGHT ? MIN_TERMINAL_HEIGHT : height;

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
    g_return_if_fail (MOO_IS_TERM (term) && string != NULL);
    moo_term_pt_write (term->priv->pt, string, len);
}


void             moo_term_copy_clipboard    (MooTerm        *term,
                                             GdkAtom         selection)
{
    GtkClipboard *cb;
    char *text;

    text = moo_term_selection_get_text (term);

    if (text && *text)
    {
        cb = gtk_clipboard_get (selection);
        gtk_clipboard_set_text (cb, text, -1);
    }

    if (text)
    {
        g_assert (*text);
        g_free (text);
    }
}


void             moo_term_ctrl_c            (MooTerm        *term)
{
    moo_term_pt_send_intr (term->priv->pt);
}


void             moo_term_paste_clipboard   (MooTerm        *term,
                                             GdkAtom         selection)
{
    GtkClipboard *cb;
    char *text;

    g_return_if_fail (MOO_IS_TERM (term));

    cb = gtk_clipboard_get (selection);
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
}


void        moo_term_decid                  (MooTerm    *term)
{
    moo_term_feed_child (term, VT_DECID_, -1);
}


void        moo_term_set_dec_modes          (MooTerm    *term,
                                             int        *modes,
                                             guint       n_modes,
                                             gboolean    set)
{
    guint i;

    g_return_if_fail (n_modes != 0); /* ??? */

    for (i = 0; i < n_modes; ++i)
    {
        int mode = -1;

        switch (modes[i])
        {
            case 47:
                moo_term_set_alternate_buffer (term, set);
                break;

            default:
                GET_DEC_MODE (modes[i], mode);

                if (mode >= 0)
                    moo_term_set_mode (term, mode, set);
        }
    }
}


void        moo_term_save_dec_modes         (MooTerm    *term,
                                             int        *modes,
                                             guint       n_modes)
{
    guint i;

    g_return_if_fail (n_modes != 0); /* ??? */

    for (i = 0; i < n_modes; ++i)
    {
        int mode = -1;

        GET_DEC_MODE (modes[i], mode);

        if (mode >= 0)
            term->priv->saved_modes[mode] = term->priv->modes[mode];
    }
}


void        moo_term_restore_dec_modes      (MooTerm    *term,
                                             int        *modes,
                                             guint       n_modes)
{
    guint i;

    g_return_if_fail (n_modes != 0); /* ??? */

    for (i = 0; i < n_modes; ++i)
    {
        int mode = -1;

        GET_DEC_MODE (modes[i], mode);

        if (mode >= 0)
            moo_term_set_mode (term, mode, term->priv->saved_modes[mode]);
    }
}


void        moo_term_set_ansi_modes         (MooTerm    *term,
                                             int        *modes,
                                             guint       n_modes,
                                             gboolean    set)
{
    guint i;

    g_return_if_fail (n_modes != 0); /* ??? */

    for (i = 0; i < n_modes; ++i)
    {
        int mode = -1;

        GET_ANSI_MODE (modes[i], mode);

        if (mode >= 0)
            moo_term_set_mode (term, mode, set);
    }
}


void        moo_term_set_mode               (MooTerm    *term,
                                             int         mode,
                                             gboolean    set)
{
#if 1
    switch (mode)
    {
        case MODE_DECSCNM:
            g_message ("set MODE_DECSCNM %s", set ? "TRUE" : "FALSE");
            break;
#if 0
        case MODE_DECTCEM:
            g_message ("set MODE_DECTCEM %s", set ? "TRUE" : "FALSE");
            break;
#endif
        case MODE_CA:
            g_message ("set MODE_CA %s", set ? "TRUE" : "FALSE");
            break;
        case MODE_REVERSE_WRAPAROUND:
            g_message ("set MODE_REVERSE_WRAPAROUND %s", set ? "TRUE" : "FALSE");
            break;
        case MODE_PRESS_TRACKING:
            g_message ("set MODE_PRESS_TRACKING %s", set ? "TRUE" : "FALSE");
            break;
        case MODE_PRESS_AND_RELEASE_TRACKING:
            g_message ("set MODE_PRESS_AND_RELEASE_TRACKING %s", set ? "TRUE" : "FALSE");
            break;
        case MODE_HILITE_MOUSE_TRACKING:
            g_message ("set MODE_HILITE_MOUSE_TRACKING %s", set ? "TRUE" : "FALSE");
            break;
        case MODE_SRM:
            g_message ("set MODE_SRM %s", set ? "TRUE" : "FALSE");
            break;
        case MODE_LNM:
            g_message ("set MODE_LNM %s", set ? "TRUE" : "FALSE");
            break;
#if 0
        case MODE_DECNKM:
            g_message ("set MODE_DECNKM %s", set ? "TRUE" : "FALSE");
            break;
        case MODE_DECCKM:
            g_message ("set MODE_DECCKM %s", set ? "TRUE" : "FALSE");
            break;
#endif
        case MODE_DECANM:
            g_message ("set MODE_DECANM %s", set ? "TRUE" : "FALSE");
            break;
        case MODE_DECBKM:
            g_message ("set MODE_DECBKM %s", set ? "TRUE" : "FALSE");
            break;
        case MODE_DECKPM:
            g_message ("set MODE_DECKPM %s", set ? "TRUE" : "FALSE");
            break;
        case MODE_IRM:
            g_message ("set MODE_IRM %s", set ? "TRUE" : "FALSE");
            break;
        case MODE_DECOM:
            g_message ("set MODE_DECOM %s", set ? "TRUE" : "FALSE");
            break;
        case MODE_DECAWM:
            g_message ("set MODE_DECAWM %s", set ? "TRUE" : "FALSE");
            break;
    }
#endif

    switch (mode)
    {
        case MODE_DECSCNM:
            term->priv->modes[mode] = set;
            moo_term_buffer_set_mode (term->priv->buffer, mode, set);
            moo_term_invert_colors (term, set);
            break;

        case MODE_DECTCEM:
            term->priv->modes[mode] = set;
            moo_term_buffer_set_mode (term->priv->buffer, mode, set);
            moo_term_set_cursor_visible (term, set);
            break;

        case MODE_CA:
            term->priv->modes[mode] = set;
            moo_term_buffer_set_mode (term->priv->buffer, mode, set);
            moo_term_set_ca_mode (term, set);
            break;

        case MODE_PRESS_TRACKING:
        case MODE_PRESS_AND_RELEASE_TRACKING:
        case MODE_HILITE_MOUSE_TRACKING:
            if (set)
                moo_term_set_mouse_tracking (term, mode);
            else
                moo_term_set_mouse_tracking (term, -1);
            term->priv->modes[mode] = set;
            moo_term_buffer_set_mode (term->priv->buffer, mode, set);
            break;

        /* these do not require anything special, just record the value */
        case MODE_SRM:
        case MODE_LNM:
        case MODE_DECNKM:
        case MODE_DECCKM:
        case MODE_DECANM:
        case MODE_DECBKM:
        case MODE_DECKPM:
        case MODE_REVERSE_WRAPAROUND:
            term->priv->modes[mode] = set;
            moo_term_buffer_set_mode (term->priv->buffer, mode, set);
            break;

        /* these are ignored */
        case MODE_IRM:
        case MODE_DECOM:
        case MODE_DECAWM:
            term->priv->modes[mode] = set;
            moo_term_buffer_set_mode (term->priv->buffer, mode, set);
            break;

        default:
            g_warning ("%s: unknown mode %d", G_STRFUNC, mode);
    }
}


static void clear_saved_cursor              (MooTerm        *term)
{
    term->priv->saved_cursor.cursor_row = 0;
    term->priv->saved_cursor.cursor_col = 0;
    term->priv->saved_cursor.attr.mask = 0;
    term->priv->saved_cursor.GL = 0;
    term->priv->saved_cursor.GR = 4;
    term->priv->saved_cursor.autowrap = DEFAULT_MODE_DECAWM;
    term->priv->saved_cursor.decom = DEFAULT_MODE_DECOM;
    term->priv->saved_cursor.top_margin = 0,
    term->priv->saved_cursor.bottom_margin = 999;
    term->priv->saved_cursor.single_shift = -1;
}


void        moo_term_decsc                  (MooTerm    *term)
{
    MooTermBuffer *buf = term->priv->buffer;

    term->priv->saved_cursor.cursor_row = term->priv->cursor_row;
    term->priv->saved_cursor.cursor_col = term->priv->cursor_col;
    term->priv->saved_cursor.attr = buf->priv->current_attr;
    term->priv->saved_cursor.GL = buf->priv->GL[0];
    term->priv->saved_cursor.GR = buf->priv->GL[1];
    term->priv->saved_cursor.autowrap = term_get_mode (MODE_DECAWM);
    term->priv->saved_cursor.decom = term_get_mode (MODE_DECOM);
    term->priv->saved_cursor.top_margin = buf->priv->top_margin;
    term->priv->saved_cursor.bottom_margin = buf->priv->bottom_margin;
    term->priv->saved_cursor.single_shift = buf->priv->single_shift;
}


void        moo_term_decrc                  (MooTerm    *term)
{
    MooTermBuffer *buf = term->priv->buffer;

    moo_term_buffer_cursor_move_to (term->priv->buffer,
                                    term->priv->saved_cursor.cursor_row,
                                    term->priv->saved_cursor.cursor_col);
    buf->priv->current_attr = term->priv->saved_cursor.attr;
    buf->priv->GL[0] = term->priv->saved_cursor.GL;
    buf->priv->GL[1] = term->priv->saved_cursor.GR;
    moo_term_set_mode (term, MODE_DECAWM, term->priv->saved_cursor.autowrap);
    moo_term_set_mode (term, MODE_DECOM, term->priv->saved_cursor.decom);
    moo_term_buffer_set_scrolling_region (buf,
                                          term->priv->saved_cursor.top_margin,
                                          term->priv->saved_cursor.bottom_margin);
    buf->priv->single_shift = term->priv->saved_cursor.single_shift;
}


void        moo_term_set_ca_mode            (MooTerm    *term,
                                             gboolean    set)
{
    if (set)
    {
        moo_term_decsc (term);
        moo_term_set_alternate_buffer (term, TRUE);
        moo_term_buffer_erase_in_display (term->priv->buffer, 2);
        moo_term_buffer_set_ca_mode (term->priv->buffer, TRUE);
    }
    else
    {
        moo_term_set_alternate_buffer (term, FALSE);
        moo_term_buffer_set_ca_mode (term->priv->buffer, FALSE);
        moo_term_decrc (term);
    }
}


void        moo_term_set_alternate_buffer   (MooTerm        *term,
                                             gboolean        alternate)
{
    g_message ("set alternate buffer %s", alternate ? "TRUE" : "FALSE");

    if ((alternate && term->priv->buffer == term->priv->alternate_buffer) ||
         (!alternate && term->priv->buffer == term->priv->primary_buffer))
        return;

    if (alternate)
        term->priv->buffer = term->priv->alternate_buffer;
    else
        term->priv->buffer = term->priv->primary_buffer;

    moo_term_invalidate_all (term);
    moo_term_buffer_scrollback_changed (term->priv->buffer);
}


void        moo_term_da1                    (MooTerm    *term)
{
    moo_term_feed_child (term, VT_DA1_, -1);
}

void        moo_term_da2                    (MooTerm    *term)
{
    /* TODO */
    moo_term_feed_child (term, VT_DA2_, -1);
}

void        moo_term_da3                    (MooTerm    *term)
{
    /* TODO */
    moo_term_feed_child (term, VT_DA3_, -1);
}


#define make_DECRQSS(c)                                  \
    answer = g_strdup_printf (VT_DCS_ "%s$r" FINAL_##c VT_ST_, ps)

void        moo_term_setting_request        (MooTerm    *term,
                                             int         setting)
{
    DECRQSSCode code = setting;
    char *ps = NULL, *answer = NULL;

    switch (code)
    {
        case CODE_DECSASD:       /* Select Active Status Display*/
            ps = g_strdup ("0");
            make_DECRQSS (DECSASD);
            break;
        case CODE_DECSCL:        /* Set Conformance Level */
            ps = g_strdup ("61");
            make_DECRQSS (DECSCL);
            break;
        case CODE_DECSCPP:       /* Set Columns Per Page */
            ps = g_strdup_printf ("%d", term->priv->width);
            make_DECRQSS (DECSCPP);
            break;
        case CODE_DECSLPP:       /* Set Lines Per Page */
            ps = g_strdup_printf ("%d", term->priv->height);
            make_DECRQSS (DECSLPP);
            break;
        case CODE_DECSNLS:       /* Set Number of Lines per Screen */
            ps = g_strdup_printf ("%d", term->priv->height);
            make_DECRQSS (DECSNLS);
            break;
        case CODE_DECSTBM:       /* Set Top and Bottom Margins */
            ps = g_strdup_printf ("%d;%d",
                                  term->priv->buffer->priv->top_margin + 1,
                                  term->priv->buffer->priv->bottom_margin + 1);
            make_DECRQSS (DECSTBM);
            break;
    }

    moo_term_feed_child (term, answer, -1);
    g_free (answer);
    g_free (ps);
}


void        moo_term_reset                  (MooTerm    *term)
{
    moo_term_buffer_freeze_changed_notify (term->priv->primary_buffer);
    moo_term_buffer_freeze_cursor_notify (term->priv->primary_buffer);

    term->priv->buffer = term->priv->primary_buffer;
    moo_term_buffer_reset (term->priv->primary_buffer);
    moo_term_buffer_reset (term->priv->alternate_buffer);
    set_default_modes (term->priv->modes);
    set_default_modes (term->priv->saved_modes);
    clear_saved_cursor (term);

    moo_term_buffer_thaw_changed_notify (term->priv->primary_buffer);
    moo_term_buffer_thaw_cursor_notify (term->priv->primary_buffer);
    moo_term_buffer_changed (term->priv->primary_buffer);
    moo_term_buffer_cursor_moved (term->priv->primary_buffer);
}


void        moo_term_soft_reset             (MooTerm    *term)
{
    moo_term_buffer_freeze_changed_notify (term->priv->buffer);
    moo_term_buffer_freeze_cursor_notify (term->priv->buffer);

    moo_term_buffer_soft_reset (term->priv->buffer);
    set_default_modes (term->priv->modes);
    set_default_modes (term->priv->saved_modes);

    moo_term_buffer_thaw_changed_notify (term->priv->primary_buffer);
    moo_term_buffer_thaw_cursor_notify (term->priv->primary_buffer);
    moo_term_buffer_changed (term->priv->primary_buffer);
    moo_term_buffer_cursor_moved (term->priv->primary_buffer);
}


void        moo_term_dsr                    (MooTerm    *term,
                                             int         type,
                                             int         arg,
                                             gboolean    extended)
{
    char *answer = NULL;
    MooTermBuffer *buf = term->priv->buffer;

    switch (type)
    {
        case 6:
            if (extended)
                answer = g_strdup_printf (VT_CSI_ "%d;%d;0R",
                                          buf_cursor_row (buf) + 1,
                                          buf_cursor_col (buf) + 1);
            else
                answer = g_strdup_printf (VT_CSI_ "%d;%dR",
                                          buf_cursor_row (buf) + 1,
                                          buf_cursor_col (buf) + 1);
            break;

            break;
        case 75:
            answer = g_strdup (VT_CSI_ "?70n");
            break;
        case 26:
            answer = g_strdup (VT_CSI_ "?27;1;0;5n");
            break;
        case 62:
            answer = g_strdup (VT_CSI_ "0*{");
            break;
        case 63:
            if (arg > 0)
                answer = g_strdup_printf (VT_DCS_ "%d!~30303030" VT_ST_, arg);
            else
                answer = g_strdup (VT_DCS_ "!~30303030" VT_ST_);
            break;
        case 5:
            answer = g_strdup (VT_CSI_ "0n");
            break;
        case 15:
            answer = g_strdup (VT_CSI_ "?13n");
            break;
        case 25:
            answer = g_strdup (VT_CSI_ "?21n");
            break;

        default:
            g_warning ("%s: unknown request", G_STRFUNC);
    }

    if (answer)
    {
        moo_term_feed_child (term, answer, -1);
        g_free (answer);
    }
}


void        moo_term_update_pointer         (MooTerm        *term)
{
    if (term->priv->pointer_visible)
    {
        if (term->priv->tracking_mouse)
            gdk_window_set_cursor (GTK_WIDGET(term)->window,
                                   term->priv->pointer[POINTER_NORMAL]);
        else
            gdk_window_set_cursor (GTK_WIDGET(term)->window,
                                   term->priv->pointer[POINTER_TEXT]);
    }
    else
    {
        gdk_window_set_cursor (GTK_WIDGET(term)->window,
                               term->priv->pointer[POINTER_NONE]);
    }
}


void        moo_term_set_pointer_visible    (MooTerm        *term,
                                             gboolean        visible)
{
    g_return_if_fail (GTK_WIDGET_REALIZED (term));

    if (visible != term->priv->pointer_visible)
    {
        term->priv->pointer_visible = visible;
        moo_term_update_pointer (term);
    }
}


static gboolean moo_term_popup_menu         (GtkWidget      *widget)
{
    moo_term_do_popup_menu (MOO_TERM (widget), NULL);
    return TRUE;
}


static void menu_position_func (G_GNUC_UNUSED GtkMenu     *menu,
                                gint        *px,
                                gint        *py,
                                gboolean    *push_in,
                                MooTerm     *term)
{
    guint cursor_row, cursor_col;
    GdkWindow *window;

    window = GTK_WIDGET(term)->window;
    gdk_window_get_origin (window, px, py);

    cursor_col = buf_cursor_col (term->priv->buffer);
    cursor_row = buf_cursor_row (term->priv->buffer);
    cursor_row += buf_scrollback (term->priv->buffer);

    if (cursor_row >= term_top_line (term))
    {
        cursor_row -= term_top_line (term);
        *px += (cursor_col + 1) * term_char_width (term);
        *py += (cursor_row + 1) * term_char_height (term);
    }
    else
    {
        int x, y, width, height;
        GdkModifierType mask;
        gdk_window_get_pointer (window, &x, &y, &mask);
        gdk_drawable_get_size (GDK_DRAWABLE (window), &width, &height);
        *px += CLAMP (x, 2, width - 2);
        *py += CLAMP (x, 2, height - 2);
    }

    *push_in = TRUE;
}


static void menu_copy (MooTerm *term)
{
    moo_term_copy_clipboard (term, GDK_SELECTION_CLIPBOARD);
}

static void menu_paste (MooTerm *term)
{
    moo_term_paste_clipboard (term, GDK_SELECTION_CLIPBOARD);
}

static void destroy_menu (GtkWidget *menu)
{
    g_idle_add ((GSourceFunc)gtk_widget_destroy, menu);
}

void        moo_term_do_popup_menu          (MooTerm        *term,
                                             GdkEventButton *event)
{
    GtkWidget *menu;
    GtkWidget *item;

    menu = gtk_menu_new ();
    /* TODO: wtf? */
    g_signal_connect (menu, "deactivate",
                      G_CALLBACK (destroy_menu), NULL);

    item = gtk_image_menu_item_new_from_stock (GTK_STOCK_COPY, NULL);
    gtk_widget_show (item);
    g_signal_connect_swapped (item, "activate",
                              G_CALLBACK (menu_copy), term);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

    item = gtk_image_menu_item_new_from_stock (GTK_STOCK_PASTE, NULL);
    gtk_widget_show (item);
    g_signal_connect_swapped (item, "activate",
                              G_CALLBACK (menu_paste), term);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

    g_signal_emit (term, signals[POPULATE_POPUP], 0, menu);

    if (event)
        gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL,
                        event->button, event->time);
    else
        gtk_menu_popup (GTK_MENU (menu), NULL, NULL,
                        (GtkMenuPositionFunc) menu_position_func,
                        term, 0, gtk_get_current_event_time ());
}


static gboolean moo_term_scroll             (GtkWidget      *widget,
                                             GdkEventScroll *event)
{
    switch (event->direction)
    {
        case GDK_SCROLL_UP:
            moo_term_scroll_lines (MOO_TERM (widget), -SCROLL_GRANULARITY);
            return TRUE;

        case GDK_SCROLL_DOWN:
            moo_term_scroll_lines (MOO_TERM (widget), SCROLL_GRANULARITY);
            return TRUE;

        default:
            return FALSE;
    }
}
