/*
 *   mooterm/mootermbuffer.c
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
#include "mooterm/mootermbuffer-private.h"
#include "mooutils/moocompat.h"
#include "mooutils/moomarshals.h"


#define MIN_WIDTH   (10L)
#define MIN_HEIGHT  (10L)

static void     moo_term_buffer_set_property    (GObject        *object,
                                                 guint           prop_id,
                                                 const GValue   *value,
                                                 GParamSpec     *pspec);
static void     moo_term_buffer_get_property    (GObject        *object,
                                                 guint           prop_id,
                                                 GValue         *value,
                                                 GParamSpec     *pspec);
static GObject *moo_term_buffer_constructor     (GType                  type,
                                                 guint                  n_construct_properties,
                                                 GObjectConstructParam *construct_param);
static void     moo_term_buffer_finalize        (GObject        *object);


/* MOO_TYPE_TERM_BUFFER */
G_DEFINE_TYPE (MooTermBuffer, moo_term_buffer, G_TYPE_OBJECT)

enum {
    CHANGED,
    CURSOR_MOVED,
    BELL,
    FLASH_SCREEN,
    SCREEN_SIZE_CHANGED,
    SET_WINDOW_TITLE,
    SET_ICON_NAME,
    LAST_SIGNAL
};

enum {
    PROP_0,
    PROP_SCREEN_WIDTH,
    PROP_SCREEN_HEIGHT,
    PROP_SCROLLBACK,
    PROP_MAX_SCROLLBACK,
    PROP_CURSOR_VISIBLE,
    PROP_AM_MODE,
    PROP_INSERT_MODE
};

static guint signals[LAST_SIGNAL];


static void moo_term_buffer_class_init (MooTermBufferClass *klass)
{
    GObjectClass   *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->set_property = moo_term_buffer_set_property;
    gobject_class->get_property = moo_term_buffer_get_property;
    gobject_class->constructor = moo_term_buffer_constructor;
    gobject_class->finalize = moo_term_buffer_finalize;

    klass->changed = buf_changed_clear;
    klass->screen_size_changed = NULL;
    klass->cursor_moved = NULL;
    klass->bell = NULL;
    klass->flash_screen = NULL;

    signals[CHANGED] =
            g_signal_new ("changed",
                          G_OBJECT_CLASS_TYPE (gobject_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTermBufferClass, changed),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[CURSOR_MOVED] =
            g_signal_new ("cursor-moved",
                          G_OBJECT_CLASS_TYPE (gobject_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTermBufferClass, cursor_moved),
                          NULL, NULL,
                          _moo_marshal_VOID__ULONG_ULONG,
                          G_TYPE_NONE, 2,
                          G_TYPE_ULONG, G_TYPE_ULONG);

    signals[BELL] =
            g_signal_new ("bell",
                          G_OBJECT_CLASS_TYPE (gobject_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTermBufferClass, bell),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[FLASH_SCREEN] =
            g_signal_new ("flash-screen",
                          G_OBJECT_CLASS_TYPE (gobject_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTermBufferClass, flash_screen),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[SCREEN_SIZE_CHANGED] =
            g_signal_new ("screen-size-changed",
                          G_OBJECT_CLASS_TYPE (gobject_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTermBufferClass, screen_size_changed),
                          NULL, NULL,
                          _moo_marshal_VOID__ULONG_ULONG,
                          G_TYPE_NONE, 2,
                          G_TYPE_ULONG, G_TYPE_ULONG);

    signals[SET_WINDOW_TITLE] =
            g_signal_new ("set-window-title",
                          G_OBJECT_CLASS_TYPE (gobject_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTermBufferClass, set_window_title),
                          NULL, NULL,
                          _moo_marshal_VOID__STRING,
                          G_TYPE_NONE, 1,
                          G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE);

    signals[SET_ICON_NAME] =
            g_signal_new ("set-icon-name",
                          G_OBJECT_CLASS_TYPE (gobject_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTermBufferClass, set_icon_name),
                          NULL, NULL,
                          _moo_marshal_VOID__STRING,
                          G_TYPE_NONE, 1,
                          G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE);

    g_object_class_install_property (gobject_class,
                                     PROP_SCREEN_WIDTH,
                                     g_param_spec_ulong ("screen-width",
                                             "screen-width",
                                             "screen-width",
                                             0, 1000000, 80,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_SCREEN_HEIGHT,
                                     g_param_spec_ulong ("screen-height",
                                             "screen-height",
                                             "screen-height",
                                             0, 1000000, 24,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_MAX_SCROLLBACK,
                                     g_param_spec_long ("max-scrollback",
                                             "max-scrollback",
                                             "max-scrollback",
                                             -1, 1000000, -1,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_SCROLLBACK,
                                     g_param_spec_ulong ("scrollback",
                                             "scrollback",
                                             "scrollback",
                                             0, 1000000, 0,
                                             G_PARAM_READABLE));

    g_object_class_install_property (gobject_class,
                                     PROP_CURSOR_VISIBLE,
                                     g_param_spec_boolean ("cursor-visible",
                                             "cursor-visible",
                                             "cursor-visible",
                                             TRUE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_AM_MODE,
                                     g_param_spec_boolean ("am-mode",
                                             "am-mode",
                                             "am-mode",
                                             TRUE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_INSERT_MODE,
                                     g_param_spec_boolean ("insert-mode",
                                             "insert-mode",
                                             "insert-mode",
                                             TRUE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
}


static void     moo_term_buffer_init            (MooTermBuffer      *buf)
{
    buf->priv = g_new0 (MooTermBufferPrivate, 1);

    buf->priv->lines = g_array_new (FALSE, FALSE, sizeof(MooTermLine));
    buf->priv->changed = NULL;
    buf->priv->changed_all = FALSE;
    buf->priv->parser = moo_term_parser_new (buf);
}


static void     moo_term_buffer_finalize        (GObject            *object)
{
    guint i;
    MooTermBuffer *buf = MOO_TERM_BUFFER (object);

    for (i = 0; i < buf->priv->lines->len; ++i)
        term_line_destroy (buf_line (buf, i));
    g_array_free (buf->priv->lines, TRUE);
    if (buf->priv->changed)
        buf_region_destroy (buf->priv->changed);
    moo_term_parser_free (buf->priv->parser);

    g_free (buf->priv);

    G_OBJECT_CLASS (moo_term_buffer_parent_class)->finalize (object);
}


static void     moo_term_buffer_set_property    (GObject        *object,
                                                 guint           prop_id,
                                                 const GValue   *value,
                                                 GParamSpec     *pspec)
{
    MooTermBuffer *buf = MOO_TERM_BUFFER (object);

    switch (prop_id) {
        case PROP_SCREEN_WIDTH:
            if (buf->priv->constructed)
                moo_term_buffer_set_screen_size (buf, g_value_get_ulong (value), 0);
            else
                buf->priv->screen_width = g_value_get_ulong (value);
            break;

        case PROP_SCREEN_HEIGHT:
            if (buf->priv->constructed)
                moo_term_buffer_set_screen_size (buf, 0, g_value_get_ulong (value));
            else
                buf->priv->screen_height = g_value_get_ulong (value);
            break;

        case PROP_MAX_SCROLLBACK:
            if (buf->priv->constructed)
                moo_term_buffer_set_max_scrollback (buf, g_value_get_long (value));
            else
                buf->priv->max_height = g_value_get_long (value);
            break;

        case PROP_CURSOR_VISIBLE:
            moo_term_buffer_set_cursor_visible (buf, g_value_get_boolean (value));
            break;

        case PROP_AM_MODE:
            moo_term_buffer_set_am_mode (buf, g_value_get_boolean (value));
            break;

        case PROP_INSERT_MODE:
            moo_term_buffer_set_insert_mode (buf, g_value_get_boolean (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static void     moo_term_buffer_get_property    (GObject        *object,
                                                 guint           prop_id,
                                                 GValue         *value,
                                                 GParamSpec     *pspec)
{
    MooTermBuffer *buf = MOO_TERM_BUFFER (object);

    switch (prop_id) {
        case PROP_SCREEN_WIDTH:
            g_value_set_ulong (value, buf->priv->screen_width);
            break;

        case PROP_SCREEN_HEIGHT:
            g_value_set_ulong (value, buf->priv->screen_height);
            break;

        case PROP_SCROLLBACK:
            g_value_set_ulong (value, buf_screen_offset (buf));
            break;

        case PROP_MAX_SCROLLBACK:
            g_value_set_long (value, buf->priv->max_height);
            break;

        case PROP_CURSOR_VISIBLE:
            g_value_set_boolean (value, buf->priv->cursor_visible);
            break;

        case PROP_AM_MODE:
            g_value_set_boolean (value, buf->priv->am_mode);
            break;

        case PROP_INSERT_MODE:
            g_value_set_boolean (value, buf->priv->insert_mode);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static GObject *moo_term_buffer_constructor     (GType                  type,
                                                 guint                  n_props,
                                                 GObjectConstructParam *props)
{
    MooTermBuffer *buf;
    GObject *object;
    guint i;

    object = G_OBJECT_CLASS(moo_term_buffer_parent_class)->constructor (type, n_props, props);
    buf = MOO_TERM_BUFFER (object);

    buf->priv->constructed = TRUE;

    if (buf->priv->max_height <= MIN_HEIGHT)
        buf->priv->max_height = -1;

    buf->priv->current_attr.mask = 0;

    if (buf->priv->screen_width < MIN_WIDTH)
        buf->priv->screen_width = MIN_WIDTH;
    if (buf->priv->screen_height < MIN_HEIGHT)
        buf->priv->screen_height = MIN_HEIGHT;
    buf->priv->screen_offset = 0;

    for (i = 0; i < buf->priv->screen_height; ++i)
    {
        MooTermLine line;
        term_line_init (&line, buf->priv->screen_width);
        g_array_append_val (buf->priv->lines, line);
    }

    return object;
}


void    moo_term_buffer_set_cursor_visible  (MooTermBuffer  *buf,
                                             gboolean        visible)
{
    buf->priv->cursor_visible = visible;
    g_object_notify (G_OBJECT (buf), "cursor-visible");
}


void    moo_term_buffer_bell            (MooTermBuffer  *buf)
{
    g_signal_emit (buf, signals[BELL], 0);
}


void    moo_term_buffer_flash_screen    (MooTermBuffer  *buf)
{
    g_signal_emit (buf, signals[FLASH_SCREEN], 0);
}


void    moo_term_buffer_changed         (MooTermBuffer  *buf)
{
    if (!buf->priv->freeze_changed_notify)
        g_signal_emit (buf, signals[CHANGED], 0);
}

void    moo_term_buffer_scrollback_changed  (MooTermBuffer  *buf)
{
    g_object_notify (G_OBJECT (buf), "scrollback");
}


void    moo_term_buffer_set_am_mode     (MooTermBuffer  *buf,
                                         gboolean        auto_margins)
{
    buf->priv->am_mode = auto_margins;
    g_object_notify (G_OBJECT (buf), "am-mode");
}


void    moo_term_buffer_set_insert_mode (MooTermBuffer  *buf,
                                         gboolean        insert_mode)
{
    buf->priv->insert_mode = insert_mode;
    g_object_notify (G_OBJECT (buf), "insert-mode");
}


void    moo_term_buffer_set_screen_size (MooTermBuffer  *buf,
                                         gulong          width,
                                         gulong          height)
{
    gulong old_width = buf_screen_width (buf);
    gulong old_height = buf_screen_height (buf);
    guint i;

    if (height >= MIN_HEIGHT && height != old_height)
    {
        buf->priv->screen_height = height;

        if (height > old_height)
        {
            if (buf->priv->screen_offset < height - old_height)
            {
                guint add = height - old_height - buf->priv->screen_offset;

                for (i = 0; i < add; ++i)
                {
                    MooTermLine new_line;
                    term_line_init (&new_line, old_width);
                    g_array_append_val (buf->priv->lines, new_line);
                }

                if (buf->priv->screen_offset)
                {
                    gulong cursor_row =
                            buf->priv->screen_offset + buf->priv->cursor_row;

                    buf->priv->screen_offset = 0;

                    buf_cursor_move_to (buf, cursor_row, -1);
                    moo_term_buffer_scrollback_changed (buf);
                }
            }
            else
            {
                buf->priv->screen_offset -= (height - old_height);
                buf_cursor_move_to (buf,
                                    buf->priv->cursor_row + height - old_height,
                                    -1);
                moo_term_buffer_scrollback_changed (buf);
            }
        }
        else
        {
            buf->priv->screen_offset += (old_height - height);

            if (buf->priv->cursor_row > old_height - height)
                buf_cursor_move_to (buf,
                                    buf->priv->cursor_row + height - old_height,
                                    -1);
            else
                buf_cursor_move_to (buf, 0, -1);

            moo_term_buffer_scrollback_changed (buf);
        }

        g_object_notify (G_OBJECT (buf), "screen-height");
    }

    if (width >= MIN_WIDTH && width != old_width)
    {
        height = buf_screen_height (buf);

        for (i = 0; i < height; ++i)
            term_line_set_len (buf_screen_line (buf, i), width);

        buf->priv->screen_width = width;

        if (buf->priv->cursor_col >= width)
            buf_cursor_move_to (buf, -1, width - 1);

        g_object_notify (G_OBJECT (buf), "screen-width");
    }

    g_signal_emit (buf, signals[SCREEN_SIZE_CHANGED], 0,
                   buf_screen_width (buf),
                   buf_screen_height (buf));
    buf_changed_set_all (buf);
    moo_term_buffer_changed (buf);
}


void    moo_term_buffer_set_max_scrollback      (G_GNUC_UNUSED MooTermBuffer  *buf,
                                                 G_GNUC_UNUSED glong           lines)
{
    g_warning ("%s: implement me", G_STRLOC);
}


void    moo_term_buffer_cursor_move     (MooTermBuffer  *buf,
                                         long            rows,
                                         long            cols)
{
    buf_cursor_move (buf, rows, cols);
}


static void moo_term_buffer_cursor_moved (MooTermBuffer  *buf,
                                          gulong          old_row,
                                          gulong          old_col)
{
    if (!buf->priv->freeze_cursor_notify)
        g_signal_emit (buf, signals[CURSOR_MOVED], 0,
                       old_row, old_col);
}


void    moo_term_buffer_cursor_move_to  (MooTermBuffer  *buf,
                                         long            row,
                                         long            col)
{
    gulong old_row = buf_cursor_row (buf);
    gulong old_col = buf_cursor_col (buf);

    g_return_if_fail (row < (long)buf_screen_height (buf));
    g_return_if_fail (col < (long)buf_screen_width (buf));

    if (row < 0)
        row = old_row;
    if (col < 0)
        col = old_col;

    buf->priv->cursor_row = row;
    buf->priv->cursor_col = col;

    moo_term_buffer_cursor_moved (buf, old_row, old_col);
}


void    moo_term_buffer_write           (MooTermBuffer  *buf,
                                         const char     *data,
                                         gssize          len)
{
    gulong old_cursor_row = buf_cursor_row (buf);
    gulong old_cursor_col = buf_cursor_col (buf);

    buf_freeze_changed_notify (buf);
    buf_freeze_cursor_notify (buf);

    moo_term_parser_parse (buf->priv->parser, data, len);

    buf_thaw_changed_notify (buf);
    buf_thaw_cursor_notify (buf);

    moo_term_buffer_changed (buf);
    moo_term_buffer_cursor_moved (buf, old_cursor_row, old_cursor_col);
}


static void buf_print_char (MooTermBuffer   *buf,
                            char            c)
{
    gulong width = buf_screen_width (buf);
    gulong cursor_row = buf_cursor_row (buf);

    MooTermTextAttr *attr =
            buf->priv->current_attr.mask ? &buf->priv->current_attr : NULL;

    if (buf->priv->insert_mode)
    {
        term_line_insert_char (buf_screen_line (buf, cursor_row),
                               buf->priv->cursor_col++,
                               c, attr, width);
        buf_changed_add_range (buf, cursor_row,
                               buf->priv->cursor_col - 1,
                               width - buf->priv->cursor_col + 1);
    }
    else
    {
        term_line_set_char (buf_screen_line (buf, cursor_row),
                            buf->priv->cursor_col++,
                            c, attr, width);
        buf_changed_add_range (buf, cursor_row,
                               buf->priv->cursor_col - 1, 1);
    }

    if (buf->priv->cursor_col == width)
    {
        if (buf->priv->am_mode)
        {
            buf->priv->cursor_col = 0;
            buf_line_feed (buf);
        }
        else
            buf->priv->cursor_col--;
    }
}


void    moo_term_buffer_print_chars     (MooTermBuffer  *buf,
                                         const char     *chars,
                                         gssize          len)
{
    gsize i;

    if (!len)
        return;

    for (i = 0; (len < 0 && chars[i]) || (len > 0 && i < (gsize)len); ++i)
    {
        char c = chars[i];

        if (c & '\200' || (' ' <= c && c <= '~'))
            buf_print_char (buf, c);
        else if (c == '\127')
            buf_print_char (buf, '~');
        else
        {
            buf_print_char (buf, '^');
            buf_print_char (buf, c + 0100);
        }
    }
}


MooTermBuffer  *moo_term_buffer_new         (gulong width,
                                             gulong height)
{
    return MOO_TERM_BUFFER (g_object_new (MOO_TYPE_TERM_BUFFER,
                                          "screen-width", width,
                                          "screen-height", height,
                                          NULL));
}


void moo_term_buffer_set_window_title   (MooTermBuffer  *buf,
                                         const char     *title)
{
    g_signal_emit (buf, signals[SET_WINDOW_TITLE], 0, title);
}

void moo_term_buffer_set_icon_name      (MooTermBuffer  *buf,
                                         const char     *icon)
{
    g_signal_emit (buf, signals[SET_ICON_NAME], 0, icon);
}
