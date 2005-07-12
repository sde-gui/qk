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
#include "mooterm/mooterm-private.h"
#include "mooterm/mootermbuffer-graph.h"
#include "mooutils/moocompat.h"
#include "mooutils/moomarshals.h"


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
    FEED_CHILD,
    FULL_RESET,
    LAST_SIGNAL
};

enum {
    PROP_0,
    PROP_SCREEN_WIDTH,
    PROP_SCREEN_HEIGHT,
    PROP_SCROLLBACK,
    PROP_SCROLLING_REGION_SET
};

static guint signals[LAST_SIGNAL];


static void moo_term_buffer_class_init (MooTermBufferClass *klass)
{
    GObjectClass   *gobject_class = G_OBJECT_CLASS (klass);

    init_drawing_sets ();

    gobject_class->set_property = moo_term_buffer_set_property;
    gobject_class->get_property = moo_term_buffer_get_property;
    gobject_class->constructor = moo_term_buffer_constructor;
    gobject_class->finalize = moo_term_buffer_finalize;

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
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[FEED_CHILD] =
            g_signal_new ("feed-child",
                          G_OBJECT_CLASS_TYPE (gobject_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTermBufferClass, feed_child),
                          NULL, NULL,
                          _moo_marshal_VOID__STRING_INT,
                          G_TYPE_NONE, 2,
                          G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE,
                          G_TYPE_INT);

    signals[FULL_RESET] =
            g_signal_new ("full-reset",
                          G_OBJECT_CLASS_TYPE (gobject_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTermBufferClass, full_reset),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    g_object_class_install_property (gobject_class,
                                     PROP_SCREEN_WIDTH,
                                     g_param_spec_uint ("screen-width",
                                             "screen-width",
                                             "screen-width",
                                             0, 1000000, 80,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_SCREEN_HEIGHT,
                                     g_param_spec_uint ("screen-height",
                                             "screen-height",
                                             "screen-height",
                                             0, 1000000, 24,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_SCROLLBACK,
                                     g_param_spec_uint ("scrollback",
                                             "scrollback",
                                             "scrollback",
                                             0, 1000000, 0,
                                             G_PARAM_READABLE));

    g_object_class_install_property (gobject_class,
                                     PROP_SCROLLING_REGION_SET,
                                     g_param_spec_boolean ("scrolling-region-set",
                                             "scrolling-region-set",
                                             "scrolling-region-set",
                                             FALSE,
                                             G_PARAM_READABLE));
}


static void     moo_term_buffer_init            (MooTermBuffer      *buf)
{
    buf->priv = g_new0 (MooTermBufferPrivate, 1);

    buf->priv->lines = g_ptr_array_new ();
    buf->priv->changed = NULL;
    buf->priv->changed_all = FALSE;

    buf->priv->single_shift = -1;
    buf->priv->graph_sets[0] = buf->priv->graph_sets[1] =
            buf->priv->graph_sets[2] = buf->priv->graph_sets[3] = NULL;
    buf->priv->current_graph_set = NULL;

    set_default_modes (buf->priv->modes);
    moo_term_buffer_clear_saved (buf);
}


static void     moo_term_buffer_finalize        (GObject            *object)
{
    guint i;
    MooTermBuffer *buf = MOO_TERM_BUFFER (object);

    for (i = 0; i < buf->priv->lines->len; ++i)
        term_line_free (g_ptr_array_index (buf->priv->lines, i));
    g_ptr_array_free (buf->priv->lines, TRUE);

    g_list_free (buf->priv->tab_stops);

    if (buf->priv->changed)
        gdk_region_destroy (buf->priv->changed);

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
                moo_term_buffer_set_screen_width (buf, g_value_get_uint (value));
            else
                buf->priv->screen_width = g_value_get_uint (value);
            break;

        case PROP_SCREEN_HEIGHT:
            if (buf->priv->constructed)
                moo_term_buffer_set_screen_height (buf, g_value_get_uint (value));
            else
                buf->priv->screen_height = g_value_get_uint (value);
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
            g_value_set_uint (value, buf->priv->screen_width);
            break;

        case PROP_SCREEN_HEIGHT:
            g_value_set_uint (value, buf->priv->screen_height);
            break;

        case PROP_SCROLLBACK:
            g_value_set_uint (value, buf_scrollback (buf));
            break;

        case PROP_SCROLLING_REGION_SET:
            g_value_set_boolean (value, buf->priv->scrolling_region_set);
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

    buf->priv->current_attr.mask = 0;

    if (buf->priv->screen_width < MIN_TERMINAL_WIDTH)
        buf->priv->screen_width = MIN_TERMINAL_WIDTH;
    if (buf->priv->screen_height < MIN_TERMINAL_HEIGHT)
        buf->priv->screen_height = MIN_TERMINAL_HEIGHT;
    buf->priv->_screen_offset = 0;

    buf->priv->top_margin = 0;
    buf->priv->bottom_margin = buf->priv->screen_height - 1;
    buf->priv->scrolling_region_set = FALSE;

    for (i = 0; i < buf->priv->screen_height; ++i)
    {
        g_ptr_array_add (buf->priv->lines,
                         term_line_new (buf->priv->screen_width));
    }

    return object;
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


void    moo_term_buffer_set_screen_width    (MooTermBuffer  *buf,
                                             guint           width)
{
    guint old_width;

    g_return_if_fail (width >= MIN_TERMINAL_WIDTH);

    old_width = buf->priv->screen_width;
    buf->priv->screen_width = width;

    if (old_width != width)
    {
        if (buf->priv->cursor_col >= width)
            moo_term_buffer_cursor_move_to (buf, -1, width - 1);

        g_object_notify (G_OBJECT (buf), "screen-width");

        if (old_width < width)
        {
            GdkRectangle changed = {
                0, old_width,
                buf->priv->screen_height,
                width - old_width
            };

            buf_changed_add_rect (changed);
            moo_term_buffer_changed (buf);
        }
        else
        {
            moo_term_buffer_reset_tab_stops (buf);
        }
    }
}


void    moo_term_buffer_set_screen_height   (MooTermBuffer  *buf,
                                             guint           height)
{
    guint old_height = buf->priv->screen_height;
    guint width = buf->priv->screen_width;
    gboolean scrollback_changed = FALSE;
    gboolean content_changed = FALSE;
    gboolean cursor_moved = FALSE;

    if (old_height == height)
        return;

    buf->priv->screen_height = height;

    if (buf_get_mode (MODE_CA))
    {
        /* in CA mode do not scroll or something, just resize the screeen */

        guint i;

        if (height > old_height)
        {
            guint add = height - old_height;
            GdkRectangle changed = {0, old_height, width, add};

            for (i = 0; i < height - old_height; ++i)
                g_ptr_array_add (buf->priv->lines,
                                 term_line_new (width));

            buf_changed_add_rect (changed);
            content_changed = TRUE;
        }
        else /* height < old_height */
        {
            guint remove = old_height - height;

            for (i = 1; i <= remove; ++i)
                term_line_free (g_ptr_array_index (buf->priv->lines,
                                                   buf->priv->lines->len - i));
            g_ptr_array_remove_range (buf->priv->lines,
                                      buf->priv->lines->len - remove,
                                      remove);

            if (buf->priv->cursor_row >= height)
            {
                buf->priv->cursor_row = height - 1;
                cursor_moved = TRUE;
            }
        }
    }
    else /* ! MODE_CA */
    {
        /* if height increases, just add new lines;
           othewise remove lines below cursor, then increment scrollback */

        guint i;

        if (height > old_height)
        {
            guint add = height - old_height;
            GdkRectangle changed = {0, old_height, width, add};

            for (i = 0; i < height - old_height; ++i)
                g_ptr_array_add (buf->priv->lines,
                                 term_line_new (width));

            buf_changed_add_rect (changed);
            content_changed = TRUE;
        }
        else /* height < old_height */
        {
            guint remove = old_height - height;

            if (remove + buf->priv->cursor_row < height)
            {
                for (i = 1; i <= remove; ++i)
                    term_line_free (g_ptr_array_index (buf->priv->lines,
                                    buf->priv->lines->len - i));
                g_ptr_array_remove_range (buf->priv->lines,
                                          buf->priv->lines->len - remove,
                                          remove);
            }
            else
            {
                if (buf->priv->cursor_row < old_height - 1)
                {
                    guint del = old_height - 1 - buf->priv->cursor_row;

                    for (i = 1; i <= del; ++i)
                        term_line_free (g_ptr_array_index (buf->priv->lines,
                                                           buf->priv->lines->len - i));
                    g_ptr_array_remove_range (buf->priv->lines,
                                              buf->priv->lines->len - del,
                                              del);

                    remove -= del;
                }

                buf->priv->_screen_offset += remove;
                buf_changed_set_all ();
                scrollback_changed = TRUE;
                content_changed = TRUE;
            }

            if (buf->priv->cursor_row >= height)
            {
                buf->priv->cursor_row = height - 1;
                cursor_moved = TRUE;
            }
        }
    }

    moo_term_buffer_set_scrolling_region (buf, 0, height - 1);

    if (scrollback_changed)
        moo_term_buffer_scrollback_changed (buf);
    g_object_notify (G_OBJECT (buf), "screen-height");
    if (content_changed)
        moo_term_buffer_changed (buf);
    if (cursor_moved)
        moo_term_buffer_cursor_moved (buf);
}


void    moo_term_buffer_set_screen_size (MooTermBuffer  *buf,
                                         guint           columns,
                                         guint           rows)
{
    moo_term_buffer_set_screen_height (buf, rows);
    moo_term_buffer_set_screen_width (buf, columns);
}


void    moo_term_buffer_cursor_move     (MooTermBuffer  *buf,
                                         int             rows,
                                         int             cols)
{
    int width = buf_screen_width (buf);
    int height = buf_screen_height (buf);
    int cursor_row = buf_cursor_row (buf);
    int cursor_col = buf_cursor_col (buf);

    cursor_col += cols;
    cursor_row += rows;

    if (cursor_row < 0)
        cursor_row = 0;
    else if (cursor_row >= height)
        cursor_row = height - 1;

    if (cursor_col < 0)
        cursor_col = 0;
    else if (cursor_col >= width)
        cursor_col = width - 1;

    moo_term_buffer_cursor_move_to (buf, cursor_row, cursor_col);
}


void    moo_term_buffer_cursor_moved (MooTermBuffer  *buf)
{
    if (!buf->priv->freeze_cursor_notify)
        g_signal_emit (buf, signals[CURSOR_MOVED], 0);
}


void    moo_term_buffer_cursor_move_to  (MooTermBuffer  *buf,
                                         int             row,
                                         int             col)
{
    int width = buf_screen_width (buf);
    int height = buf_screen_height (buf);
    guint old_row = buf_cursor_row (buf);
    guint old_col = buf_cursor_col (buf);

    g_return_if_fail (row < (int) buf_screen_height (buf));
    g_return_if_fail (col < (int) buf_screen_width (buf));

    if (row < 0)
        row = old_row;
    else if (row >= height)
        row = height - 1;

    if (col < 0)
        col = old_col;
    else if (col >= width)
        col = width - 1;

    buf->priv->cursor_row = row;
    buf->priv->cursor_col = col;

    moo_term_buffer_cursor_moved (buf);
}


/* chars must be valid unicode string */
static void buf_print_unichar_real  (MooTermBuffer  *buf,
                                     gunichar        c)
{
    guint width = buf_screen_width (buf);
    guint cursor_row = buf_cursor_row (buf);

    MooTermTextAttr *attr =
            buf->priv->current_attr.mask ? &buf->priv->current_attr : NULL;

    if (c <= MAX_GRAPH)
    {
        if (buf->priv->single_shift >= 0)
        {
            if (buf->priv->graph_sets[buf->priv->single_shift])
            {
                if (buf->priv->graph_sets[buf->priv->single_shift][c])
                    c = buf->priv->graph_sets[buf->priv->single_shift][c];
                else
                    g_warning ("%s: using regular character while in "
                            "graphics mode", G_STRLOC);
            }
            buf->priv->single_shift = -1;
        }
        else if (buf->priv->current_graph_set)
        {
            if (buf->priv->current_graph_set[c])
                c = buf->priv->current_graph_set[c];
            else
                g_warning ("%s: using regular character while in "
                           "graphics mode", G_STRLOC);
        }
    }

    if (buf_get_mode (MODE_IRM))
    {
        term_line_insert_unichar (buf_screen_line (buf, cursor_row),
                                    buf->priv->cursor_col++,
                                    c, 1, attr, width);
        buf_changed_add_range (cursor_row,
                                buf->priv->cursor_col - 1,
                                width - buf->priv->cursor_col + 1);
    }
    else
    {
        term_line_set_unichar (buf_screen_line (buf, cursor_row),
                                buf->priv->cursor_col++,
                                c, 1, attr, width);
        buf_changed_add_range (cursor_row,
                                buf->priv->cursor_col - 1, 1);
    }

    if (buf->priv->cursor_col == width)
    {
        buf->priv->cursor_col--;
        if (buf_get_mode (MODE_DECAWM))
            moo_term_buffer_new_line (buf);
    }
}


/* chars must be valid unicode string */
void    moo_term_buffer_print_chars     (MooTermBuffer  *buf,
                                         const char     *chars,
                                         int             len)
{
    const char *p = chars;
    const char *s;

    g_return_if_fail (len != 0 && chars != NULL);

    moo_term_buffer_freeze_changed_notify (buf);
    moo_term_buffer_freeze_cursor_notify (buf);

    while ((len > 0 && p != chars + len) || (len < 0 && *p != 0))
    {
        for (s = p; ((len > 0 && s != chars + len) || (len < 0 && *s != 0))
             && *s && (*s & 0x80 || (' ' <= *s && *s <= '~')); ++s) ;

        if (s != p)
        {
            const char *i = p;

            for (i = p; i < s; i = g_utf8_next_char (i))
                buf_print_unichar_real (buf, g_utf8_get_char (i));

            p = s;
        }
        else if (!*s)
        {
            ++p;
        }
        else if (*s == 0x7F)
        {
            g_warning ("passed DEL to %s", G_STRFUNC);
            ++p;
        }
        else
        {
            buf_print_unichar_real (buf, '^');
            buf_print_unichar_real (buf, *s + 0x40);
        }
    }

    moo_term_buffer_thaw_changed_notify (buf);
    moo_term_buffer_thaw_cursor_notify (buf);
    moo_term_buffer_changed (buf);
    moo_term_buffer_cursor_moved (buf);
}


void    moo_term_buffer_print_unichar       (MooTermBuffer  *buf,
                                             gunichar        c)
{
    moo_term_buffer_freeze_changed_notify (buf);
    moo_term_buffer_freeze_cursor_notify (buf);

    buf_print_unichar_real (buf, c);

    moo_term_buffer_thaw_changed_notify (buf);
    moo_term_buffer_thaw_cursor_notify (buf);
    moo_term_buffer_changed (buf);
    moo_term_buffer_cursor_moved (buf);
}


void    moo_term_buffer_feed_child      (MooTermBuffer  *buf,
                                         const char     *string,
                                         int             len)
{
    g_signal_emit (buf, signals[FEED_CHILD], 0, string, len);
}


void    moo_term_buffer_reset_tab_stops     (MooTermBuffer  *buf)
{
    guint i;
    guint width = buf_screen_width (buf);

    g_list_free (buf->priv->tab_stops);
    buf->priv->tab_stops = NULL;

    for (i = 0; i < (width + 7) / 8; ++i)
        buf->priv->tab_stops = g_list_append (buf->priv->tab_stops,
                                              GUINT_TO_POINTER (8 * i));
}

guint   moo_term_buffer_next_tab_stop       (MooTermBuffer  *buf,
                                             guint           current)
{
    GList *l;

    for (l = buf->priv->tab_stops;
         l != NULL && GPOINTER_TO_UINT (l->data) <= current;
         l = l->next) ;

    if (l && GPOINTER_TO_UINT (l->data) > current)
        return GPOINTER_TO_UINT (l->data);
    else
        return buf_screen_width (buf) - 1;
}

guint   moo_term_buffer_prev_tab_stop       (MooTermBuffer  *buf,
                                             guint           current)
{
    GList *l;

    for (l = buf->priv->tab_stops;
         l != NULL && GPOINTER_TO_UINT (l->data) < current;
         l = l->next) ;

    if (l && l->prev && GPOINTER_TO_UINT (l->prev->data) < current)
        return GPOINTER_TO_UINT (l->prev->data);
    else
        return 0;
}

void    moo_term_buffer_clear_tab_stop      (MooTermBuffer  *buf)
{
    buf->priv->tab_stops =
            g_list_remove (buf->priv->tab_stops,
                           GUINT_TO_POINTER (buf_cursor_col (buf)));
}

static int cmp_guints (gconstpointer a, gconstpointer b)
{
    if (GPOINTER_TO_UINT (a) < GPOINTER_TO_UINT (b))
        return -1;
    else if (a == b)
        return 0;
    else
        return 1;
}

void    moo_term_buffer_set_tab_stop        (MooTermBuffer  *buf)
{
    guint cursor = buf_cursor_col (buf);

    if (!g_list_find (buf->priv->tab_stops, GUINT_TO_POINTER (cursor)))
        buf->priv->tab_stops =
                g_list_insert_sorted (buf->priv->tab_stops,
                                      GUINT_TO_POINTER (cursor),
                                      cmp_guints);
}


void    moo_term_buffer_select_charset  (MooTermBuffer  *buf,
                                         guint           set_num,
                                         guint           charset)
{
    g_return_if_fail (set_num < 4 && charset < 5);

    switch (charset)
    {
        case 0:
            if (buf->priv->use_ascii_graphics)
                buf->priv->graph_sets[set_num] = ASCII_DRAWING_SET;
            else
                buf->priv->graph_sets[set_num] = DRAWING_SET;
            break;

        case 2:
            g_warning ("%s: choosing graphics instead of"
                       "Alternate Character ROM Special Set", G_STRLOC);
            if (buf->priv->use_ascii_graphics)
                buf->priv->graph_sets[set_num] = ASCII_DRAWING_SET;
            else
                buf->priv->graph_sets[set_num] = DRAWING_SET;
            break;

        case 1:
            g_warning ("%s: choosing regular charset instead of"
                    "Alternate Character ROM Standard Set", G_STRLOC);
            buf->priv->graph_sets[set_num] = NULL;
            break;

        case 3:
            g_warning ("%s: choosing regular charset instead of"
                       "United Kingdom", G_STRLOC);
            buf->priv->graph_sets[set_num] = NULL;
            break;

        case 4:
            buf->priv->graph_sets[set_num] = NULL;
            break;
    }
}


void    moo_term_buffer_shift           (MooTermBuffer  *buf,
                                         guint           set)
{
    g_return_if_fail (set < 4);
    buf->priv->current_graph_set = buf->priv->graph_sets[set];
}


void    moo_term_buffer_single_shift    (MooTermBuffer  *buf,
                                         guint           set)
{
    g_return_if_fail (set < 4);
    buf->priv->single_shift = set;
}


MooTermBuffer  *moo_term_buffer_new         (guint width,
                                             guint height)
{
    return g_object_new (MOO_TYPE_TERM_BUFFER,
                         "screen-width", width,
                         "screen-height", height,
                         NULL);
}


void    moo_term_buffer_changed_clear           (MooTermBuffer  *buf)
{
    if (buf->priv->changed)
        gdk_region_destroy (buf->priv->changed);

    buf->priv->changed = NULL;
    buf->priv->changed_all = FALSE;
}


void    moo_term_buffer_set_scrolling_region    (MooTermBuffer  *buf,
                                                 guint           top_margin,
                                                 guint           bottom_margin)
{
    if (top_margin >= bottom_margin ||
        top_margin >= buf->priv->screen_height ||
        bottom_margin >= buf->priv->screen_height)
    {
        top_margin = 0;
        bottom_margin = buf->priv->screen_height - 1;
    }

    buf->priv->top_margin = top_margin;
    buf->priv->bottom_margin = bottom_margin;
    buf->priv->scrolling_region_set =
            top_margin != 0 || bottom_margin != buf->priv->screen_height - 1;

    g_object_notify (G_OBJECT (buf), "scrolling-region-set");
}


void    moo_term_buffer_freeze_changed_notify   (MooTermBuffer  *buf)
{
    buf->priv->freeze_changed_notify++;
}

void    moo_term_buffer_thaw_changed_notify     (MooTermBuffer  *buf)
{
    if (buf->priv->freeze_changed_notify)
        buf->priv->freeze_changed_notify--;
}

void    moo_term_buffer_freeze_cursor_notify    (MooTermBuffer  *buf)
{
    buf->priv->freeze_cursor_notify++;
}

void    moo_term_buffer_thaw_cursor_notify      (MooTermBuffer  *buf)
{
    if (buf->priv->freeze_cursor_notify)
        buf->priv->freeze_cursor_notify--;
}


#define freeze_notify()                             \
{                                                   \
    moo_term_buffer_freeze_changed_notify (buf);    \
    moo_term_buffer_freeze_cursor_notify (buf);     \
}

#define notify()                                    \
{                                                   \
    moo_term_buffer_changed (buf);                  \
    moo_term_buffer_cursor_moved (buf);             \
}

#define thaw_notify()                               \
{                                                   \
    moo_term_buffer_thaw_changed_notify (buf);      \
    moo_term_buffer_thaw_cursor_notify (buf);       \
}

#define thaw_and_notify()                           \
{                                                   \
    thaw_notify ();                                 \
    notify();                                       \
}

#define notify_changed()    moo_term_buffer_changed (buf)
#define freeze_changed()    moo_term_buffer_freeze_changed_notify (buf);

#define thaw_and_notify_changed()                   \
{                                                   \
    moo_term_buffer_thaw_changed_notify (buf);      \
    moo_term_buffer_changed (buf);                  \
}


/*****************************************************************************/
/* Terminal stuff
 */

void    moo_term_buffer_new_line        (MooTermBuffer  *buf)
{
    freeze_notify ();
    moo_term_buffer_index (buf);
    moo_term_buffer_cursor_move_to (buf, -1, 0);
    thaw_and_notify ();
}


void    moo_term_buffer_index           (MooTermBuffer  *buf)
{
    guint cursor_row = buf_cursor_row (buf);
    guint screen_height = buf_screen_height (buf);
    guint width = buf_screen_width (buf);

    g_assert (cursor_row < screen_height);

    if (buf_get_mode (MODE_CA) || buf->priv->scrolling_region_set)
    {
        guint top = buf->priv->top_margin + buf->priv->_screen_offset;
        guint bottom = buf->priv->bottom_margin + buf->priv->_screen_offset;
        guint cursor = cursor_row + buf->priv->_screen_offset;

        g_assert (cursor <= bottom && cursor >= top);

        if (cursor == bottom)
        {
            GdkRectangle changed = {
                0, buf->priv->top_margin, width, bottom - top + 1
            };

            term_line_free (g_ptr_array_index (buf->priv->lines, top));

            memmove (&buf->priv->lines->pdata[top],
                     &buf->priv->lines->pdata[top+1],
                     (bottom - top) * sizeof(gpointer));

            /* TODO: attributes */
            buf->priv->lines->pdata[bottom] = term_line_new (width);

            buf_changed_add_rect (changed);
        }
        else
        {
            buf->priv->cursor_row += 1;
        }
    }
    else
    {
        g_assert (cursor_row < screen_height);

        if (cursor_row == screen_height - 1)
        {
            g_ptr_array_add (buf->priv->lines,
                             term_line_new (width));

            buf->priv->_screen_offset += 1;
            moo_term_buffer_scrollback_changed (buf);

            buf_changed_set_all ();
        }
        else
        {
            buf->priv->cursor_row += 1;
        }
    }

    notify ();
}


void    moo_term_buffer_reverse_index           (MooTermBuffer  *buf)
{
    guint width = buf_screen_width (buf);
    guint top = buf->priv->top_margin + buf->priv->_screen_offset;
    guint bottom = buf->priv->bottom_margin + buf->priv->_screen_offset;
    guint cursor = buf_cursor_row (buf) + buf->priv->_screen_offset;

    g_assert (cursor <= bottom && cursor >= top);

    if (cursor == top)
    {
        GdkRectangle changed = {
            0, buf->priv->top_margin, width, bottom - top + 1
        };

        term_line_free (g_ptr_array_index (buf->priv->lines, bottom));

        memmove (&buf->priv->lines->pdata[top+1],
                 &buf->priv->lines->pdata[top],
                 (bottom - top) * sizeof(gpointer));

        /* TODO: attributes */
        buf->priv->lines->pdata[top] = term_line_new (width);

        buf_changed_add_rect (changed);
    }
    else
    {
        buf->priv->cursor_row -= 1;
    }

    notify ();
}


void    moo_term_buffer_backspace               (MooTermBuffer  *buf)
{
    moo_term_buffer_cursor_move (buf, 0, -1);
}


void    moo_term_buffer_tab                     (MooTermBuffer  *buf)
{
    moo_term_buffer_cursor_move_to (buf, -1,
        moo_term_buffer_next_tab_stop (buf, buf->priv->cursor_col));
}


void    moo_term_buffer_linefeed                (MooTermBuffer  *buf)
{
    if (buf_get_mode (MODE_LNM))
        moo_term_buffer_new_line (buf);
    else
        moo_term_buffer_index (buf);
}


void    moo_term_buffer_carriage_return         (MooTermBuffer  *buf)
{
    moo_term_buffer_cursor_move_to (buf, -1, 0);
}


void    moo_term_buffer_sgr                     (MooTermBuffer  *buf,
                                                 int            *params,
                                                 guint           num_params)
{
    guint i;

    if (!num_params)
    {
        buf_set_attrs_mask (0);
        return;
    }

    for (i = 0; i < num_params; ++i)
    {
        switch (params[i])
        {
            case ANSI_ALL_ATTRIBUTES_OFF:
                buf_set_attrs_mask (0);
                break;
            case ANSI_BOLD:
                buf_add_attrs_mask (MOO_TERM_TEXT_BOLD);
                break;
            case ANSI_UNDERLINE:
                buf_add_attrs_mask (MOO_TERM_TEXT_UNDERLINE);
                break;
            case ANSI_BLINKING:
                g_warning ("%s: ignoring blink", G_STRLOC);
                break;
            case ANSI_NEGATIVE:
                buf_add_attrs_mask (MOO_TERM_TEXT_REVERSE);
                break;
            case ANSI_BOLD_OFF:
                buf_remove_attrs_mask (MOO_TERM_TEXT_BOLD);
                break;
            case ANSI_UNDERLINE_OFF:
                buf_remove_attrs_mask (MOO_TERM_TEXT_UNDERLINE);
                break;
            case ANSI_BLINKING_OFF:
                g_warning ("%s: ignoring blink", G_STRLOC);
                break;
            case ANSI_NEGATIVE_OFF:
                buf_remove_attrs_mask (MOO_TERM_TEXT_REVERSE);
                break;

            default:
                if (30 <= params[i] && params[i] <= 37)
                {
                    buf_set_ansi_foreground (params[i] - 30);
                }
                else if (40 <= params[i] && params[i] <= 47)
                {
                    buf_set_ansi_background (params[i] - 40);
                }
                else if (39 == params[i])
                {
                    buf_set_ansi_foreground (8);
                }
                else if (49 == params[i])
                {
                    buf_set_ansi_background (8);
                }
                else
                    g_warning ("%s: unknown text attribute %d",
                               G_STRLOC, params[i]);
        }
    }
}


void    moo_term_buffer_delete_char             (MooTermBuffer  *buf,
                                                 guint           n)
{
    guint cursor_col = buf_cursor_col (buf);
    guint cursor_row = buf_cursor_row (buf);

    g_assert (cursor_col < buf_screen_width (buf));

    if (!n || cursor_row > buf->priv->bottom_margin ||
         cursor_row < buf->priv->top_margin)
    {
        return;
    }

    term_line_delete_range (buf_screen_line (buf, cursor_row),
                            cursor_col, n);
    buf_changed_add_range(cursor_row, cursor_col,
                          buf_screen_width (buf) - cursor_col);
    notify_changed ();
}


void    moo_term_buffer_erase_range             (MooTermBuffer  *buf,
                                                 guint           row,
                                                 guint           col,
                                                 guint           len)
{
    if (row >= buf_screen_height (buf) ||
        col >= buf_screen_width (buf) ||
        !len)
    {
        return;
    }

    term_line_erase_range (buf_screen_line (buf, row),
                           col, len);
    buf_changed_add_range(row, col, len);
    notify_changed ();
}


void    moo_term_buffer_erase_char              (MooTermBuffer  *buf,
                                                 guint           n)
{
    guint cursor_col = buf_cursor_col (buf);
    guint cursor_row = buf_cursor_row (buf);

    g_assert (cursor_col < buf_screen_width (buf));

    moo_term_buffer_erase_range (buf, cursor_row,
                                 cursor_col, n);
}


void    moo_term_buffer_erase_in_display        (MooTermBuffer  *buf,
                                                 guint           what)
{
    guint i;
    guint cursor_col = buf_cursor_col (buf);
    guint cursor_row = buf_cursor_row (buf);
    guint width = buf_screen_width (buf);
    guint height = buf_screen_height (buf);

    g_return_if_fail (what == 0 || what == 1 || what == 2);

    freeze_changed ();

    switch (what)
    {
        case 0:
            moo_term_buffer_erase_range (buf, cursor_row,
                                         cursor_col, width);
            for (i = cursor_row + 1; i < height; ++i)
                moo_term_buffer_erase_range (buf, i, 0, width);
            break;

        case 1:
            for (i = 0; i < cursor_row; ++i)
                moo_term_buffer_erase_range (buf, i, 0, width);
            moo_term_buffer_erase_range (buf, cursor_row,
                                         0, cursor_col + 1);
            break;

        case 2:
            for (i = 0; i < height; ++i)
                moo_term_buffer_erase_range (buf, i, 0, width);
            break;
    }

    thaw_and_notify_changed ();
}


void    moo_term_buffer_erase_in_line           (MooTermBuffer  *buf,
                                                 guint           what)
{
    guint cursor_col = buf_cursor_col (buf);
    guint cursor_row = buf_cursor_row (buf);
    guint width = buf_screen_width (buf);

    g_return_if_fail (what == 0 || what == 1 || what == 2);

    switch (what)
    {
        case 0:
            moo_term_buffer_erase_range (buf, cursor_row,
                                         cursor_col, width);
            break;

        case 1:
            moo_term_buffer_erase_range (buf, cursor_row,
                                         0, cursor_col + 1);
            break;

        case 2:
            moo_term_buffer_erase_range (buf, cursor_row,
                                         0, width);
            break;
    }
}


void    moo_term_buffer_insert_char             (MooTermBuffer  *buf,
                                                 guint           n)
{
    guint cursor_col = buf_cursor_col (buf);
    guint cursor_row = buf_cursor_row (buf);

    g_assert (cursor_col < buf_screen_width (buf));

    if (!n || cursor_row > buf->priv->bottom_margin ||
         cursor_row < buf->priv->top_margin)
    {
        return;
    }

    term_line_insert_unichar (buf_screen_line (buf, cursor_row),
                              cursor_col, EMPTY_CHAR, n,
                              &ZERO_ATTR, buf_screen_width (buf));
    buf_changed_add_range(cursor_row, cursor_col,
                          buf_screen_width (buf) - cursor_col);
    notify_changed ();
}


void    moo_term_buffer_cup                     (MooTermBuffer  *buf,
                                                 guint           row,
                                                 guint           col)
{
    if (col >= buf_screen_width (buf))
        col = buf_screen_width (buf) - 1;

    if (buf_get_mode (MODE_DECOM))
        moo_term_buffer_cursor_move_to (buf, row + buf->priv->top_margin, col);
    else
        moo_term_buffer_cursor_move_to (buf, row, col);
}


void    moo_term_buffer_delete_line             (MooTermBuffer  *buf,
                                                 guint           n)
{
    guint cursor = buf->priv->cursor_row + buf->priv->_screen_offset;
    guint top = buf->priv->top_margin + buf->priv->_screen_offset;
    guint bottom = buf->priv->bottom_margin + buf->priv->_screen_offset;
    guint i;

    GdkRectangle changed = {
        0, buf->priv->cursor_row,
        buf->priv->screen_width,
        bottom - cursor + 1
    };

    g_return_if_fail (n != 0);
    g_return_if_fail (top <= cursor && cursor <= bottom);

    /* TODO: scroll the window */

    if (n > bottom - cursor + 1)
        n = bottom - cursor + 1;

    for (i = cursor; i < cursor + n; ++i)
        term_line_free (g_ptr_array_index (buf->priv->lines, i));

    if (n < bottom - cursor + 1)
        memmove (&g_ptr_array_index (buf->priv->lines, cursor),
                  &g_ptr_array_index (buf->priv->lines, cursor + n),
                  (bottom - cursor + 1 - n) * sizeof(gpointer));

    for (i = bottom + 1 - n; i <= bottom; ++i)
        g_ptr_array_index (buf->priv->lines, i) =
                term_line_new (buf->priv->screen_width);

    buf_changed_add_rect (changed);
    moo_term_buffer_changed (buf);
}


void    moo_term_buffer_insert_line             (MooTermBuffer  *buf,
                                                 guint           n)
{
    guint cursor = buf->priv->cursor_row + buf->priv->_screen_offset;
    guint top = buf->priv->top_margin + buf->priv->_screen_offset;
    guint bottom = buf->priv->bottom_margin + buf->priv->_screen_offset;
    guint i;

    GdkRectangle changed = {
        0, buf->priv->cursor_row,
        buf->priv->screen_width,
        buf->priv->bottom_margin - buf->priv->cursor_row + 1
    };

    g_return_if_fail (n != 0);
    g_return_if_fail (top <= cursor && cursor <= bottom);

    /* TODO: scroll the window */

    if (n > bottom - cursor + 1)
        n = bottom - cursor + 1;

    for (i = bottom - n + 1; i <= bottom; ++i)
        term_line_free (g_ptr_array_index (buf->priv->lines, i));

    if (n < bottom - cursor + 1)
        memmove (&g_ptr_array_index (buf->priv->lines, cursor + n),
                 &g_ptr_array_index (buf->priv->lines, cursor),
                 (bottom - cursor + 1 - n) * sizeof(gpointer));

    for (i = cursor; i < cursor + n; ++i)
        g_ptr_array_index (buf->priv->lines, i) =
                term_line_new (buf->priv->screen_width);

    buf_changed_add_rect (changed);
    moo_term_buffer_changed (buf);
}


void    moo_term_buffer_decsc                   (MooTermBuffer  *buf)
{
    buf->priv->saved.cursor_row = buf->priv->cursor_row;
    buf->priv->saved.cursor_col = buf->priv->cursor_col;
    buf->priv->saved.attr = buf->priv->current_attr;
    buf->priv->saved.GL = buf->priv->graph_sets[0];
    buf->priv->saved.GR = buf->priv->graph_sets[1];
    buf->priv->saved.autowrap = buf_get_mode (MODE_DECAWM);
    buf->priv->saved.decom = buf_get_mode (MODE_DECOM);
    buf->priv->saved.top_margin = buf->priv->top_margin;
    buf->priv->saved.bottom_margin = buf->priv->bottom_margin;
    buf->priv->saved.single_shift = buf->priv->single_shift;
}


void    moo_term_buffer_decrc                   (MooTermBuffer  *buf)
{
    buf->priv->cursor_row = buf->priv->saved.cursor_row;
    buf->priv->cursor_col = buf->priv->saved.cursor_col;
    buf->priv->current_attr = buf->priv->saved.attr;
    buf->priv->graph_sets[0] = buf->priv->saved.GL;
    buf->priv->graph_sets[1] = buf->priv->saved.GR;
    moo_term_buffer_set_mode (buf, MODE_DECAWM, buf->priv->saved.autowrap);
    moo_term_buffer_set_mode (buf, MODE_DECOM, buf->priv->saved.decom);
    moo_term_buffer_set_scrolling_region (buf, buf->priv->top_margin,
                                          buf->priv->bottom_margin);
    buf->priv->single_shift = buf->priv->saved.single_shift;
}


void    moo_term_buffer_clear_saved             (MooTermBuffer  *buf)
{
    buf->priv->saved.cursor_row = 0;
    buf->priv->saved.cursor_col = 0;
    buf->priv->saved.attr.mask = 0;
    buf->priv->saved.autowrap = DEFAULT_MODE_DECAWM;
    buf->priv->saved.decom = DEFAULT_MODE_DECOM;
    buf->priv->saved.top_margin = 0;
    buf->priv->saved.bottom_margin = buf_screen_height (buf) - 1;
    buf->priv->saved.single_shift = -1;
    buf->priv->saved.GL = NULL;

    if (buf->priv->use_ascii_graphics)
        buf->priv->saved.GR = ASCII_DRAWING_SET;
    else
        buf->priv->saved.GR = DRAWING_SET;
}


void    moo_term_buffer_set_mode                (MooTermBuffer  *buf,
                                                 guint           mode,
                                                 gboolean        set)
{
    switch (mode)
    {
        case MODE_CA:
            moo_term_buffer_set_ca_mode (buf, set);
            break;

        /* these do not require anything special, just record the value */
        case MODE_IRM:
        case MODE_LNM:
        case MODE_DECANM:
        case MODE_DECOM:
        case MODE_DECAWM:
            buf->priv->modes[mode] = set;
            break;

        /* these are ignored in the buffer */
        case MODE_SRM:
        case MODE_DECCKM:
        case MODE_DECSCNM:
        case MODE_DECTCEM:
        case MODE_DECNKM:
        case MODE_DECBKM:
        case MODE_DECKPM:
        case MODE_PRESS_TRACKING:
        case MODE_PRESS_AND_RELEASE_TRACKING:
        case MODE_HILITE_MOUSE_TRACKING:
            buf->priv->modes[mode] = set;
            break;

        default:
            g_warning ("%s: unknown mode %d", G_STRFUNC, mode);
    }
}


void    moo_term_buffer_set_ca_mode             (MooTermBuffer  *buf,
                                                 gboolean        set)
{
    buf->priv->modes[MODE_CA] = set;
    moo_term_buffer_scrollback_changed (buf);
}
